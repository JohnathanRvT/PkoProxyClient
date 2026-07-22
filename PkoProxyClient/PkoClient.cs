using System;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace PkoProxyClient
{
    /// <summary>
    /// Tracks the connection and authentication lifecycle of the headless PKO client.
    /// </summary>
    public enum ClientState
    {
        Disconnected,
        Connected,
        Handshaking,    // Received CHAP challenge, ready to login
        Authenticated,  // Authenticated successfully via credentials
        Playing         // Selected a character, actively playing/simulating game loop
    }

    /// <summary>
    /// A standalone C# headless playable client designed for connection, handshake, authentication,
    /// character select, and active session gameplay simulation with the PKO servers.
    /// </summary>
    public class PkoClient
    {
        private readonly string _host;
        private readonly int _port;
        private readonly ushort _version;
        private TcpClient? _client;
        private NetworkStream? _stream;
        private readonly PacketEncryptor _encryptor;
        private string _chapString = "";
        private string _login = "";
        private byte[] _passwordBytes = Array.Empty<byte>();
        private bool _connected = false;
        private readonly uint _session = 0x80000000;
        private CancellationTokenSource? _cts;
        private ClientState _state = ClientState.Disconnected;

        /// <summary>
        /// Gets the current state of the client in the connection/authentication state machine.
        /// </summary>
        public ClientState State => _state;

        public PkoClient(string host, int port, ushort version = 100)
        {
            _host = host ?? throw new ArgumentNullException(nameof(host));
            _port = port;
            _version = version;
            _encryptor = new PacketEncryptor();
            _state = ClientState.Disconnected;
        }

        public async Task RunAsync()
        {
            _client = new TcpClient();
            try
            {
                LogConsole($"Connecting to target server at {_host}:{_port}...", ConsoleColor.Gray);
                await _client.ConnectAsync(_host, _port);
                _stream = _client.GetStream();
                _connected = true;
                _state = ClientState.Connected;
                _cts = new CancellationTokenSource();

                LogConsole("Connected! Starting background packet receiver...", ConsoleColor.Green);
                _ = ReceivePacketsAsync(_cts.Token);

                // Start CLI loop
                await CommandLoopAsync();
            }
            catch (Exception ex)
            {
                LogConsole($"Error: {ex.Message}", ConsoleColor.Red);
            }
            finally
            {
                Disconnect();
            }
        }

        private void Disconnect()
        {
            _connected = false;
            _state = ClientState.Disconnected;
            _cts?.Cancel();
            _stream?.Close();
            _client?.Close();
            LogConsole("Disconnected from server.", ConsoleColor.Red);
        }

        private async Task ReceivePacketsAsync(CancellationToken cancellationToken)
        {
            if (_stream == null) return;
            var reader = new TcpStreamPacketReader(_stream);

            try
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    byte[]? packet = await reader.ReadPacketAsync(cancellationToken);
                    if (packet == null) break;

                    // Handle 2-byte TCP heartbeat
                    if (packet.Length == 2)
                    {
                        // Reply with 2-byte heartbeat 0x0002
                        byte[] heartbeat = new byte[] { 0, 2 };
                        if (_stream == null) break;
                        await _stream.WriteAsync(heartbeat, 0, 2, cancellationToken);
                        LogConsole("[S->C] TCP Heartbeat received & answered.", ConsoleColor.DarkYellow);
                        continue;
                    }

                    // Parse normal packet
                    byte[] copy = (byte[])packet.Clone();
                    ushort packetSize = (ushort)((copy[0] << 8) | copy[1]);

                    // Decrypt if encryption is enabled (excluding unencrypted 931)
                    ushort packetIdBeforeDec = 0;
                    if (!_encryptor.Enabled)
                    {
                        packetIdBeforeDec = (ushort)((copy[6] << 8) | copy[7]);
                    }

                    if (_encryptor.Enabled && packetIdBeforeDec != 931)
                    {
                        byte[] payload = new byte[copy.Length - 6];
                        Array.Copy(copy, 6, payload, 0, payload.Length);
                        _encryptor.Decrypt(payload, DecryptType.SC);
                        Array.Copy(payload, 0, copy, 6, payload.Length);
                    }

                    var pktReader = new PkoPacketReader(copy);
                    _ = pktReader.ReadUint16(); // size
                    _ = pktReader.ReadUint32(); // session
                    ushort packetId = pktReader.ReadUint16();

                    LogConsole($"[S->C] Packet ID: {packetId} | Size: {packetSize}", ConsoleColor.Yellow);

                    if (packetId == 940) // ChapString
                    {
                        _chapString = pktReader.ReadString();
                        _state = ClientState.Handshaking;
                        LogConsole($"[Handshake] Received CHAP Challenge: \"{_chapString}\". Ready to login.", ConsoleColor.Green);
                    }
                    else if (packetId == 931) // LoginResult
                    {
                        ushort result = pktReader.ReadUint16();
                        LogConsole($"[Login] Result Code: {result} ({(result == 0 ? "SUCCESS" : "FAILED")})", result == 0 ? ConsoleColor.Green : ConsoleColor.Red);

                        if (result == 0) // result_success is 0 in PKO
                        {
                            _state = ClientState.Authenticated;
                            ushort keyLen = pktReader.ReadUint16();
                            byte[] encryptionKey = pktReader.ReadBytes(keyLen);
                            pktReader.SeekFromEnd(8);
                            bool commEncryption = pktReader.ReadUint32() != 0;

                            if (commEncryption)
                            {
                                _encryptor.Init(true, _version, _chapString, _passwordBytes, encryptionKey);
                                LogConsole("[Crypto] Stateful session encryption initialized and active!", ConsoleColor.Green);
                            }
                        }
                    }
                    else if (packetId == 537) // Ping from Server
                    {
                        LogConsole("[S->C] Ping from Server. Replying automatically with CPingPacket...", ConsoleColor.DarkYellow);
                        await SendPingAsync(cancellationToken);
                    }
                }
            }
            catch (Exception ex) when (ex is not OperationCanceledException)
            {
                LogConsole($"Receiver loop stopped: {ex.Message}", ConsoleColor.Red);
            }
            finally
            {
                Disconnect();
            }
        }

        private async Task CommandLoopAsync()
        {
            PrintInteractiveMenu();

            while (_connected)
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write("pko-client> ");
                Console.ResetColor();

                string? line = Console.ReadLine();
                if (string.IsNullOrWhiteSpace(line)) continue;

                string[] parts = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
                string cmd = parts[0].ToLower();

                if (cmd == "exit")
                {
                    break;
                }
                else if (cmd == "help")
                {
                    PrintHelp();
                }
                else if (cmd == "info")
                {
                    PrintInfo();
                }
                else if (cmd == "status")
                {
                    PrintStatus();
                }
                else if (cmd == "login")
                {
                    if (parts.Length < 3)
                    {
                        Console.WriteLine("Usage: login <user> <password>");
                        continue;
                    }
                    if (_state == ClientState.Disconnected || _state == ClientState.Connected)
                    {
                        Console.WriteLine("Error: Waiting for CHAP handshake challenge from server. Please wait or reconnect.");
                        continue;
                    }
                    _login = parts[1];
                    string rawPassword = parts[2];

                    // Cache password in the encoded form used inside proxy server
                    _passwordBytes = PasswordEncoder.Encode(rawPassword, _chapString);

                    await SendLoginAsync(_login, rawPassword);
                }
                else if (cmd == "select")
                {
                    if (parts.Length < 2)
                    {
                        Console.WriteLine("Usage: select <character_name>");
                        continue;
                    }
                    if (_state != ClientState.Authenticated)
                    {
                        Console.WriteLine("Error: You must login successfully before selecting a character.");
                        continue;
                    }
                    await SendBeginPlayAsync(parts[1]);
                }
                else if (cmd == "logout")
                {
                    if (_state != ClientState.Authenticated && _state != ClientState.Playing)
                    {
                        Console.WriteLine("Error: You are not logged in.");
                        continue;
                    }
                    await SendLogoutAsync();
                }
                else if (cmd == "ping")
                {
                    if (_cts != null)
                    {
                        await SendPingAsync(_cts.Token);
                    }
                    else
                    {
                        Console.WriteLine("Error: Client session is inactive.");
                    }
                }
                else
                {
                    Console.WriteLine("Unknown command. Type 'help' to view available commands.");
                }
            }
        }

        private void PrintInteractiveMenu()
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine(@"
=========================================================
      ____  _  ______    ____ _     ___ _____ _   _ _____
     |  _ \| |/ / ___|  / ___| |   |_ _| ____| \ | |_   _|
     | |_) | ' / |     | |   | |    | ||  _| |  \| | | |
     |  __/| . \ |___  | |___| |___ | || |___| |\  | | |
     |_|   |_|\_\____|  \____|_____|___|_____|_| \_| |_|

      [ PKO Standalone C# Playable Headless Client ]
=========================================================");
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.WriteLine("Type 'help' to see list of available commands.");
            Console.WriteLine("Type 'info' to read about the PKO cryptographic protocol.");
            Console.WriteLine("Type 'status' to print connection and session metrics.");
            Console.WriteLine("=========================================================\n");
            Console.ResetColor();
        }

        private void PrintHelp()
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("\n--- Available Interactive Commands ---");
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("  help                        - Displays this commands list.");
            Console.WriteLine("  info                        - Prints detailed technical background on PKO security.");
            Console.WriteLine("  status                      - Prints current connection metrics and session keys.");
            Console.WriteLine("  login <user> <password>     - Handshakes, authenticates and initializes encryption.");
            Console.WriteLine("  select <character_name>     - Selects the player character to play in-game.");
            Console.WriteLine("  logout                      - Gracefully logs out the active user session.");
            Console.WriteLine("  ping                        - Dispatches a keep-alive ping packet manually.");
            Console.WriteLine("  exit                        - Closes the connection and exits the application.");
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("---------------------------------------\n");
            Console.ResetColor();
        }

        private void PrintInfo()
        {
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine("\n--- PKO Security & Network Protocol Information ---");
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("  PKO uses a custom TCP-based binary protocol.");
            Console.WriteLine("  * Framing: Every packet starts with a 2-byte Big-Endian length followed");
            Console.WriteLine("    by a 4-byte session/connection sequence header.");
            Console.WriteLine("  * Challenge: The login handshake begins with a 940 challenge challenge string.");
            Console.WriteLine("  * Password: The password is encrypted with the CHAP string using DES ECB.");
            Console.WriteLine("  * Key Exchange: On successful authentication, the server generates a random");
            Console.WriteLine("    unencrypted key. Both sides derive a 6-byte session key from this.");
            Console.WriteLine("  * Encryption: Subsequent traffic is dynamically encrypted with a sliding");
            Console.WriteLine("    XOR-with-bit-rotation ('B' algorithm) and a 4-byte dynamic noise generator.");
            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine("----------------------------------------------------\n");
            Console.ResetColor();
        }

        private void PrintStatus()
        {
            Console.ForegroundColor = ConsoleColor.DarkCyan;
            Console.WriteLine("\n--- Connection and Session Status ---");
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine($"  * Server Host      : {_host}:{_port}");
            Console.WriteLine($"  * Client Version   : {_version}");
            Console.WriteLine($"  * Connection State : {_state}");
            Console.WriteLine($"  * Session Active   : {_connected}");
            Console.WriteLine($"  * Account Login    : {(string.IsNullOrEmpty(_login) ? "<none>" : _login)}");
            Console.WriteLine($"  * CHAP Challenge   : {(string.IsNullOrEmpty(_chapString) ? "<none>" : $"\"{_chapString}\"")}");
            Console.WriteLine($"  * Packet Session ID: 0x{_session:X8}");
            Console.WriteLine($"  * Encryption State : {(_encryptor.Enabled ? "ENABLED (Stateful B + Noise Active)" : "DISABLED (Plaintext Headers)")}");
            Console.ForegroundColor = ConsoleColor.DarkCyan;
            Console.WriteLine("--------------------------------------\n");
            Console.ResetColor();
        }

        private async Task SendLoginAsync(string user, string rawPassword)
        {
            if (_stream == null) return;

            var writer = new PkoPacketWriter();
            writer.WriteUint16(0); // Size placeholder
            writer.WriteUint32(_session);
            writer.WriteUint16(431); // Packet ID (AccountLogin)

            writer.WriteString(""); // m_nobill
            writer.WriteString(user); // m_login
            writer.WriteUint16((ushort)_passwordBytes.Length);
            writer.WriteBytes(_passwordBytes);
            writer.WriteString("00:11:22:33:44:55"); // mac address
            writer.WriteUint32(0x0100007F); // localhost IP (127.0.0.1)
            writer.WriteUint16(0); // flag
            writer.WriteUint16(_version); // version

            // Add extra 2 bytes protection version if target is local proxy or matches protect version
            writer.WriteByte(0);
            writer.WriteByte(0);

            byte[] data = writer.ToArray();
            ushort actualSize = (ushort)data.Length;
            data[0] = (byte)(actualSize >> 8);
            data[1] = (byte)(actualSize & 0xFF);

            // Encryption
            if (_encryptor.Enabled)
            {
                byte[] payload = new byte[data.Length - 6];
                Array.Copy(data, 6, payload, 0, payload.Length);
                _encryptor.Encrypt(payload, EncryptType.CS);
                Array.Copy(payload, 0, data, 6, payload.Length);
            }

            await _stream.WriteAsync(data, 0, data.Length);
            LogConsole($"[C->S] Sent AccountLogin (431) for user '{user}'", ConsoleColor.Cyan);
        }

        private async Task SendBeginPlayAsync(string charName)
        {
            if (_stream == null) return;

            var writer = new PkoPacketWriter();
            writer.WriteUint16(0); // Size placeholder
            writer.WriteUint32(_session);
            writer.WriteUint16(433); // Packet ID (BeginPlay)
            writer.WriteString(charName);

            byte[] data = writer.ToArray();
            ushort actualSize = (ushort)data.Length;
            data[0] = (byte)(actualSize >> 8);
            data[1] = (byte)(actualSize & 0xFF);

            // Encryption
            if (_encryptor.Enabled)
            {
                byte[] payload = new byte[data.Length - 6];
                Array.Copy(data, 6, payload, 0, payload.Length);
                _encryptor.Encrypt(payload, EncryptType.CS);
                Array.Copy(payload, 0, data, 6, payload.Length);
            }

            await _stream.WriteAsync(data, 0, data.Length);
            LogConsole($"[C->S] Sent BeginPlay (433) selecting character '{charName}'", ConsoleColor.Cyan);
            _state = ClientState.Playing;
        }

        private async Task SendLogoutAsync()
        {
            if (_stream == null) return;

            var writer = new PkoPacketWriter();
            writer.WriteUint16(0); // Size placeholder
            writer.WriteUint32(_session);
            writer.WriteUint16(432); // Packet ID (Logout)

            byte[] data = writer.ToArray();
            ushort actualSize = (ushort)data.Length;
            data[0] = (byte)(actualSize >> 8);
            data[1] = (byte)(actualSize & 0xFF);

            // Encryption
            if (_encryptor.Enabled)
            {
                byte[] payload = new byte[data.Length - 6];
                Array.Copy(data, 6, payload, 0, payload.Length);
                _encryptor.Encrypt(payload, EncryptType.CS);
                Array.Copy(payload, 0, data, 6, payload.Length);
            }

            await _stream.WriteAsync(data, 0, data.Length);
            LogConsole("[C->S] Sent AccountLogout (432)", ConsoleColor.Cyan);
            _state = ClientState.Connected;
        }

        private async Task SendPingAsync(CancellationToken cancellationToken)
        {
            if (_stream == null) return;

            var writer = new PkoPacketWriter();
            writer.WriteUint16(0); // Size placeholder
            writer.WriteUint32(_session);
            writer.WriteUint16(17); // Packet ID (Ping)

            byte[] data = writer.ToArray();
            ushort actualSize = (ushort)data.Length;
            data[0] = (byte)(actualSize >> 8);
            data[1] = (byte)(actualSize & 0xFF);

            // Encryption
            if (_encryptor.Enabled)
            {
                byte[] payload = new byte[data.Length - 6];
                Array.Copy(data, 6, payload, 0, payload.Length);
                _encryptor.Encrypt(payload, EncryptType.CS);
                Array.Copy(payload, 0, data, 6, payload.Length);
            }

            await _stream.WriteAsync(data, 0, data.Length, cancellationToken);
            LogConsole("[C->S] Sent PingPacket (17)", ConsoleColor.Cyan);
        }

        private void LogConsole(string message, ConsoleColor color = ConsoleColor.White)
        {
            lock (this)
            {
                var prevColor = Console.ForegroundColor;
                Console.ForegroundColor = color;
                Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] {message}");
                Console.ForegroundColor = prevColor;
            }
        }
    }
}
