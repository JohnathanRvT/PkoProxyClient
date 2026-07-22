using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace PkoProxyClient
{
    /// <summary>
    /// Tracks intermediate handshake parameters intercepted by the proxy for dynamic cryptography alignment.
    /// </summary>
    public class HandshakeState
    {
        public string ChapString { get; set; } = "";
        public string Login { get; set; } = "";
        public byte[] PasswordBytes { get; set; } = Array.Empty<byte>();
        public ushort Version { get; set; } = 0;
        public string PlaintextPassword { get; set; } = "";
    }

    /// <summary>
    /// Encapsulates decrypted packet context metadata passed to intercepting proxy plugins.
    /// </summary>
    public class ProxyPacketContext
    {
        public int ConnectionId { get; }
        public string Direction { get; } // "C -> S" or "S -> C"
        public ushort PacketId { get; set; }
        public uint Session { get; set; }
        public byte[] DecryptedPacket { get; set; }
        public bool IsDropped { get; set; } = false;

        public ProxyPacketContext(int connectionId, string direction, ushort packetId, uint session, byte[] decryptedPacket)
        {
            ConnectionId = connectionId;
            Direction = direction ?? throw new ArgumentNullException(nameof(direction));
            PacketId = packetId;
            Session = session;
            DecryptedPacket = decryptedPacket ?? throw new ArgumentNullException(nameof(decryptedPacket));
        }
    }

    /// <summary>
    /// Represents a plugin interface for inspecting and modifying unencrypted packets inside PkoProxy.
    /// </summary>
    public interface IProxyPlugin
    {
        /// <summary>
        /// Unique identify name of the plugin.
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Gets or sets whether the plugin is actively processing packets.
        /// </summary>
        bool Enabled { get; set; }

        /// <summary>
        /// Callback executed when a decrypted packet is intercepted by the proxy.
        /// </summary>
        /// <param name="context">The packet context, allowing inspection or mutation of the payload.</param>
        void OnPacket(ProxyPacketContext context);
    }

    /// <summary>
    /// Plugin responsible for logging detailed packet dumps to local .log and .bin (ImHex) files.
    /// </summary>
    public class LogToFilePlugin : IProxyPlugin
    {
        public string Name => "LogToFile";
        public bool Enabled { get; set; } = true;
        private readonly string _logFilePath;
        private readonly string _binFilePath;

        public LogToFilePlugin(string logFilePath)
        {
            _logFilePath = logFilePath ?? throw new ArgumentNullException(nameof(logFilePath));
            _binFilePath = Path.ChangeExtension(_logFilePath, ".bin");
        }

        public void OnPacket(ProxyPacketContext context)
        {
            if (!Enabled) return;

            ushort size = (ushort)((context.DecryptedPacket[0] << 8) | context.DecryptedPacket[1]);
            byte[] payload = new byte[context.DecryptedPacket.Length - 8];
            Array.Copy(context.DecryptedPacket, 8, payload, 0, payload.Length);

            string cmdName = PkoCommandTranslator.GetCommandName(context.PacketId);

            StringBuilder sb = new StringBuilder();
            sb.AppendLine($"=========================================================================");
            sb.AppendLine($"[Connection #{context.ConnectionId}] {context.Direction} | Packet ID: {cmdName} ({context.PacketId}) | Size: {size} | Session: 0x{context.Session:X8}");
            sb.AppendLine($"-------------------------------------------------------------------------");
            sb.AppendLine(PkoProxy.HexDump(payload));
            sb.AppendLine();

            string text = sb.ToString();

            // Append textual hex dump safely
            try
            {
                lock (_logFilePath)
                {
                    File.AppendAllText(_logFilePath, text);
                }
            }
            catch { }

            // Append structured binary format safely
            try
            {
                byte dirVal = context.Direction == "C -> S" ? (byte)0 : (byte)1;
                lock (_binFilePath)
                {
                    using (var fs = new FileStream(_binFilePath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite))
                    {
                        fs.WriteByte(dirVal);
                        fs.Write(context.DecryptedPacket, 0, context.DecryptedPacket.Length);
                    }
                }
            }
            catch { }
        }
    }

    /// <summary>
    /// Plugin responsible for printing beautiful colored, stylized console highlights for intercepted packets.
    /// </summary>
    public class ConsoleColorPacketLoggerPlugin : IProxyPlugin
    {
        public string Name => "ConsoleColorPacketLogger";
        public bool Enabled { get; set; } = true;

        private readonly object _consoleLock = new object();

        public void OnPacket(ProxyPacketContext context)
        {
            if (!Enabled) return;

            ushort size = (ushort)((context.DecryptedPacket[0] << 8) | context.DecryptedPacket[1]);
            string cmdName = PkoCommandTranslator.GetCommandName(context.PacketId);

            lock (_consoleLock)
            {
                var prevColor = Console.ForegroundColor;
                if (context.Direction == "C -> S")
                {
                    Console.ForegroundColor = ConsoleColor.Cyan;
                }
                else
                {
                    Console.ForegroundColor = ConsoleColor.Yellow;
                }

                // Highlight handshakes with magenta highlights
                if (context.PacketId == 940 || context.PacketId == 931 || context.PacketId == 431)
                {
                    Console.ForegroundColor = ConsoleColor.Magenta;
                    Console.Write($"[{DateTime.Now:HH:mm:ss.fff}] [HANDSHAKE] ");
                }
                else
                {
                    Console.Write($"[{DateTime.Now:HH:mm:ss.fff}] ");
                }

                Console.WriteLine($"[Connection #{context.ConnectionId}] {context.Direction} | {cmdName} ({context.PacketId}) | Size: {size,4} | Session: 0x{context.Session:X8}");
                Console.ForegroundColor = prevColor;
            }
        }
    }

    /// <summary>
    /// Proxy server designed to intercept PKO client-to-server and server-to-client traffic,
    /// decrypt packets in real-time, dump them for inspection, and optionally modify them using plugins.
    /// </summary>
    public class PkoProxy
    {
        public static readonly System.Collections.Generic.List<IProxyPlugin> Plugins = new System.Collections.Generic.List<IProxyPlugin>();

        private readonly int _localPort;
        private readonly string _remoteHost;
        private readonly int _remotePort;
        private readonly bool _protectionEnabled;
        private readonly string _logFilePath;
        private readonly string _plaintextPassword;
        private TcpListener? _listener;
        private int _connectionCounter = 0;

        public PkoProxy(int localPort, string remoteHost, int remotePort, bool protectionEnabled, string logFilePath = "proxy_packets.log", string plaintextPassword = "")
        {
            _localPort = localPort;
            _remoteHost = remoteHost ?? throw new ArgumentNullException(nameof(remoteHost));
            _remotePort = remotePort;
            _protectionEnabled = protectionEnabled;
            _logFilePath = logFilePath ?? throw new ArgumentNullException(nameof(logFilePath));
            _plaintextPassword = plaintextPassword ?? "";

            // Register default built-in plugins
            Plugins.Clear();
            Plugins.Add(new ConsoleColorPacketLoggerPlugin());
            Plugins.Add(new LogToFilePlugin(_logFilePath));
            Plugins.Add(new BotPlugin());
        }

        public async Task StartAsync(CancellationToken cancellationToken)
        {
            _listener = new TcpListener(IPAddress.Any, _localPort);
            _listener.Start();

            LogConsole($"Proxy started. Listening on port {_localPort}, forwarding to {_remoteHost}:{_remotePort}...");
            LogConsole($"Packet sequence protection: {(_protectionEnabled ? "ENABLED" : "DISABLED")}");
            LogConsole($"Logging unencrypted packets to: {Path.GetFullPath(_logFilePath)}\n");

            try
            {
                while (!cancellationToken.IsCancellationRequested)
                {
                    TcpClient client = await _listener.AcceptTcpClientAsync(cancellationToken);
                    int connId = Interlocked.Increment(ref _connectionCounter);
                    _ = HandleConnectionAsync(client, connId, cancellationToken);
                }
            }
            catch (Exception ex) when (ex is not OperationCanceledException)
            {
                LogConsole($"Proxy listener encountered an error: {ex.Message}");
            }
            finally
            {
                _listener?.Stop();
                LogConsole("Proxy stopped.");
            }
        }

        private async Task HandleConnectionAsync(TcpClient client, int connId, CancellationToken cancellationToken)
        {
            LogConsole($"[Connection #{connId}] Client connected from {client.Client.RemoteEndPoint}");

            using (client)
            using (TcpClient server = new TcpClient())
            {
                try
                {
                    await server.ConnectAsync(_remoteHost, _remotePort, cancellationToken);
                    LogConsole($"[Connection #{connId}] Connected to target server at {_remoteHost}:{_remotePort}");

                    using (NetworkStream clientStream = client.GetStream())
                    using (NetworkStream serverStream = server.GetStream())
                    {
                        using (var cts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken))
                        {
                            // Stateful crypto parameters for this connection
                            HandshakeState handshakeState = new HandshakeState();
                            handshakeState.PlaintextPassword = _plaintextPassword;
                            PacketEncryptor encryptor = new PacketEncryptor();

                            var serverWriteSemaphore = new SemaphoreSlim(1, 1);

                            // Task for Client to Server direction
                            var clientToServerTask = ForwardClientToServerAsync(clientStream, serverStream, connId, encryptor, handshakeState, serverWriteSemaphore, cts.Token);

                            // Task for Server to Client direction
                            var serverToClientTask = ForwardServerToClientAsync(serverStream, clientStream, connId, encryptor, handshakeState, cts.Token);

                            await Task.WhenAny(clientToServerTask, serverToClientTask);
                            cts.Cancel();
                        }
                    }
                }
                catch (Exception ex)
                {
                    LogConsole($"[Connection #{connId}] Connection closed due to error: {ex.Message}");
                }
                finally
                {
                    LogConsole($"[Connection #{connId}] Disconnected.");
                }
            }
        }

        private async Task ForwardClientToServerAsync(NetworkStream clientStream, NetworkStream serverStream, int connId, PacketEncryptor encryptor, HandshakeState state, SemaphoreSlim serverWriteSemaphore, CancellationToken cancellationToken)
        {
            var reader = new TcpStreamPacketReader(clientStream);

            // Register the BotPlugin send callback once before the connection forwarding starts
            foreach (var plugin in Plugins)
            {
                if (plugin is BotPlugin bot)
                {
                    bot.SetSendCallback(connId, (decryptedPkt) =>
                    {
                        try
                        {
                            byte[] encPkt = (byte[])decryptedPkt.Clone();
                            if (encryptor.Enabled)
                            {
                                byte[] payload = new byte[encPkt.Length - 6];
                                Array.Copy(encPkt, 6, payload, 0, payload.Length);
                                encryptor.Encrypt(payload, EncryptType.CS);
                                Array.Copy(payload, 0, encPkt, 6, payload.Length);
                            }

                            serverWriteSemaphore.Wait();
                            try
                            {
                                serverStream.Write(encPkt, 0, encPkt.Length);
                                serverStream.Flush();
                            }
                            finally
                            {
                                serverWriteSemaphore.Release();
                            }
                        }
                        catch (Exception ex)
                        {
                            LogConsole($"[BotPlugin Callback Error] {ex.Message}");
                        }
                    });
                }
            }

            while (!cancellationToken.IsCancellationRequested)
            {
                byte[]? packet = await reader.ReadPacketAsync(cancellationToken);
                if (packet == null) break;

                // Handle 2-byte heartbeat packet
                if (packet.Length == 2)
                {
                    try
                    {
                        await serverWriteSemaphore.WaitAsync(cancellationToken);
                        await serverStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
                    }
                    finally
                    {
                        serverWriteSemaphore.Release();
                    }
                    continue;
                }

                // Parse/Inspect packet copy before forwarding
                byte[] copy = (byte[])packet.Clone();
                uint session = (uint)((copy[2] << 24) | (copy[3] << 16) | (copy[4] << 8) | copy[5]);

                // Decrypt if encryption has been enabled
                if (encryptor.Enabled)
                {
                    // Copy bytes for decryption from index 6 onwards
                    byte[] payload = new byte[copy.Length - 6];
                    Array.Copy(copy, 6, payload, 0, payload.Length);

                    encryptor.Decrypt(payload, DecryptType.CS);

                    // Reconstruct decrypted copy
                    Array.Copy(payload, 0, copy, 6, payload.Length);
                }

                var pktReader = new PkoPacketReader(copy);
                pktReader.ReadUint16(); // Skip size
                pktReader.ReadUint32(); // Skip session
                ushort packetId = pktReader.ReadUint16();

                // Skip the 4-byte protection sequence number if it is present (which might be read by ReadUint32)
                _ = pktReader.ReadUint32();

                // Handle specific handshake packets to capture info
                if (packetId == 431) // AccountLogin
                {
                    try
                    {
                        // Structure of 431: string nobill, string login, ushort pwd_len, bytes password, string mac, ushort flag, ushort version
                        _ = pktReader.ReadString(); // nobill
                        string loginVal = pktReader.ReadString();
                        ushort pwdLen = pktReader.ReadUint16();
                        byte[] pwdBytes = pktReader.ReadBytes(pwdLen);
                        string mac = pktReader.ReadString();
                        _ = pktReader.ReadUint16(); // flag (always 911)
                        ushort versionVal = pktReader.ReadUint16();

                        lock (state)
                        {
                            state.Login = loginVal;
                            state.PasswordBytes = pwdBytes;
                            state.Version = versionVal;
                        }

                        LogConsole($"[Connection #{connId}] [C->S Handshake Info] Intercepted Login: {loginVal}, Version: {versionVal}, Mac: {mac}");
                    }
                    catch (Exception ex)
                    {
                        LogConsole($"[Connection #{connId}] [C->S Parse Error ID 431] {ex.Message}");
                    }
                }

                // Run plugins
                var context = new ProxyPacketContext(connId, "C -> S", packetId, session, copy);
                foreach (var plugin in Plugins)
                {
                    try { plugin.OnPacket(context); } catch { }
                }

                if (context.IsDropped)
                {
                    LogConsole($"[Connection #{connId}] C -> S | Packet ID: {packetId} DROPPED by plugin.");
                    continue;
                }

                bool wasModified = !ByteArrayCompare(context.DecryptedPacket, copy);
                if (wasModified)
                {
                    copy = context.DecryptedPacket;
                    ushort newSize = (ushort)copy.Length;
                    copy[0] = (byte)(newSize >> 8);
                    copy[1] = (byte)(newSize & 0xFF);

                    packet = (byte[])copy.Clone();
                    if (encryptor.Enabled)
                    {
                        byte[] payload = new byte[packet.Length - 6];
                        Array.Copy(packet, 6, payload, 0, payload.Length);
                        encryptor.Encrypt(payload, EncryptType.CS);
                        Array.Copy(payload, 0, packet, 6, payload.Length);
                    }
                }

                // Forward packet to server
                try
                {
                    await serverWriteSemaphore.WaitAsync(cancellationToken);
                    await serverStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
                }
                finally
                {
                    serverWriteSemaphore.Release();
                }
            }
        }

        private async Task ForwardServerToClientAsync(NetworkStream serverStream, NetworkStream clientStream, int connId, PacketEncryptor encryptor, HandshakeState state, CancellationToken cancellationToken)
        {
            var reader = new TcpStreamPacketReader(serverStream);

            while (!cancellationToken.IsCancellationRequested)
            {
                byte[]? packet = await reader.ReadPacketAsync(cancellationToken);
                if (packet == null) break;

                // Handle 2-byte heartbeat packet
                if (packet.Length == 2)
                {
                    await clientStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
                    continue;
                }

                // Parse/Inspect packet copy before forwarding
                byte[] copy = (byte[])packet.Clone();
                uint session = (uint)((copy[2] << 24) | (copy[3] << 16) | (copy[4] << 8) | copy[5]);

                // Decrypt if encryption has been enabled (excluding 931 which is not encrypted itself)
                ushort packetIdBeforeDec = 0;
                if (!encryptor.Enabled)
                {
                    packetIdBeforeDec = (ushort)((copy[6] << 8) | copy[7]);
                }

                if (encryptor.Enabled && packetIdBeforeDec != 931)
                {
                    // Copy bytes for decryption from index 6 onwards
                    byte[] payload = new byte[copy.Length - 6];
                    Array.Copy(copy, 6, payload, 0, payload.Length);

                    encryptor.Decrypt(payload, DecryptType.SC);

                    // Reconstruct decrypted copy
                    Array.Copy(payload, 0, copy, 6, payload.Length);
                }

                var pktReader = new PkoPacketReader(copy);
                pktReader.ReadUint16(); // Skip size
                pktReader.ReadUint32(); // Skip session
                ushort packetId = pktReader.ReadUint16();

                // Inspect Server Handshake Packets
                if (packetId == 940) // ChapString
                {
                    try
                    {
                        string chapString = pktReader.ReadString();
                        lock (state)
                        {
                            state.ChapString = chapString;
                        }
                        LogConsole($"[Connection #{connId}] [S->C ChapString] Intercepted Value: \"{chapString}\"");
                    }
                    catch (Exception ex)
                    {
                        LogConsole($"[Connection #{connId}] [S->C Parse Error ID 940] {ex.Message}");
                    }
                }
                else if (packetId == 931) // LoginResult (sent unencrypted)
                {
                    try
                    {
                        ushort result = pktReader.ReadUint16();
                        LogConsole($"[Connection #{connId}] [S->C LoginResult] Result Code: {result}");

                        if (result == 0) // result_success is 0 in PKO
                        {
                            ushort keyLen = pktReader.ReadUint16();
                            byte[] encryptionKey = pktReader.ReadBytes(keyLen);
                            pktReader.SeekFromEnd(8);
                            uint commEncryptionVal = pktReader.ReadUint32();
                            bool commEncryption = commEncryptionVal != 0;

                            StringBuilder keyHex = new StringBuilder();
                            foreach (byte b in encryptionKey) keyHex.Append($"{b:X2} ");
                            LogConsole($"[Connection #{connId}] [S->C Crypto Handshake] CommEncryption: {commEncryption}, Key ({encryptionKey.Length} bytes): {keyHex}");

                            if (commEncryption)
                            {
                                lock (state)
                                {
                                    System.Collections.Generic.List<byte[]> candidates = new System.Collections.Generic.List<byte[]>();

                                    // Candidate 1: pwdBytes from wire (24 bytes)
                                    if (state.PasswordBytes.Length > 0)
                                    {
                                        candidates.Add(state.PasswordBytes);
                                    }

                                    // Candidate 2: pwdBytes truncated (23 bytes)
                                    if (state.PasswordBytes.Length > 0)
                                    {
                                        byte[] trunc = new byte[state.PasswordBytes.Length - 1];
                                        Array.Copy(state.PasswordBytes, trunc, trunc.Length);
                                        candidates.Add(trunc);
                                    }

                                    // Candidate 3: plaintext password MD5 hash (32 bytes)
                                    if (!string.IsNullOrEmpty(state.PlaintextPassword))
                                    {
                                        string md5 = GetMD5Hash(state.PlaintextPassword);
                                        candidates.Add(Encoding.ASCII.GetBytes(md5));
                                    }

                                    // Candidate 4: plaintext password itself
                                    if (!string.IsNullOrEmpty(state.PlaintextPassword))
                                    {
                                        candidates.Add(Encoding.ASCII.GetBytes(state.PlaintextPassword));
                                    }

                                    encryptor.SetCandidates(candidates, state.Version, state.ChapString, encryptionKey);
                                    LogConsole($"[Connection #{connId}] [Success] Decryption engine initialized with {candidates.Count} candidates!");
                                }
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        LogConsole($"[Connection #{connId}] [S->C Parse Error ID 931] {ex.Message}");
                    }
                }

                // Run plugins
                var context = new ProxyPacketContext(connId, "S -> C", packetId, session, copy);
                foreach (var plugin in Plugins)
                {
                    try { plugin.OnPacket(context); } catch { }
                }

                if (context.IsDropped)
                {
                    LogConsole($"[Connection #{connId}] S -> C | Packet ID: {packetId} DROPPED by plugin.");
                    continue;
                }

                bool wasModified = !ByteArrayCompare(context.DecryptedPacket, copy);
                if (wasModified)
                {
                    copy = context.DecryptedPacket;
                    ushort newSize = (ushort)copy.Length;
                    copy[0] = (byte)(newSize >> 8);
                    copy[1] = (byte)(newSize & 0xFF);

                    packet = (byte[])copy.Clone();
                    if (encryptor.Enabled && packetId != 931)
                    {
                        byte[] payload = new byte[packet.Length - 6];
                        Array.Copy(packet, 6, payload, 0, payload.Length);
                        encryptor.Encrypt(payload, EncryptType.SC);
                        Array.Copy(payload, 0, packet, 6, payload.Length);
                    }
                }

                // Forward packet to client
                await clientStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
            }
        }

        public static string GetMD5Hash(string input)
        {
            using (System.Security.Cryptography.MD5 md5 = System.Security.Cryptography.MD5.Create())
            {
                byte[] inputBytes = Encoding.ASCII.GetBytes(input);
                byte[] hashBytes = md5.ComputeHash(inputBytes);
                StringBuilder sb = new StringBuilder();
                for (int i = 0; i < hashBytes.Length; i++)
                {
                    sb.Append(hashBytes[i].ToString("X2"));
                }
                return sb.ToString();
            }
        }

        private readonly object _consoleLock = new object();

        private void LogConsole(string message)
        {
            lock (_consoleLock)
            {
                Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] {message}");
            }
        }

        private static bool ByteArrayCompare(byte[]? a1, byte[]? a2)
        {
            if (a1 == null || a2 == null) return ReferenceEquals(a1, a2);
            if (a1.Length != a2.Length) return false;
            for (int i = 0; i < a1.Length; i++)
                if (a1[i] != a2[i]) return false;
            return true;
        }

        public static string HexDump(byte[] bytes, int bytesPerLine = 16)
        {
            if (bytes == null) return "<null>";
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < bytes.Length; i += bytesPerLine)
            {
                int chunkLength = Math.Min(bytesPerLine, bytes.Length - i);
                sb.Append($"{i:X4}: ");
                for (int j = 0; j < bytesPerLine; j++)
                {
                    if (j < chunkLength)
                        sb.Append($"{bytes[i + j]:X2} ");
                    else
                        sb.Append("   ");
                }
                sb.Append("  ");
                for (int j = 0; j < chunkLength; j++)
                {
                    char c = (char)bytes[i + j];
                    if (c >= 32 && c <= 126)
                        sb.Append(c);
                    else
                        sb.Append('.');
                }
                sb.AppendLine();
            }
            return sb.ToString();
        }
    }
}
