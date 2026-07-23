using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace PkoProxyClient
{
    // ------------------------------------------------------------------
    // Main Proxy class
    // ------------------------------------------------------------------
    public class PkoProxy
    {
        public static readonly List<IProxyPlugin> Plugins = new List<IProxyPlugin>();
        public static readonly Dictionary<int, ProxySession> ActiveSessions = new Dictionary<int, ProxySession>();

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

            Plugins.Clear();
            Plugins.Add(new ConsoleColorPacketLoggerPlugin());
            Plugins.Add(new LogToFilePlugin(_logFilePath));
            Plugins.Add(new AutoBotPlugin());

            ProxyLog.Write("General", "Proxy initialized with plugins: ConsoleColorPacketLogger, LogToFile, AutoBot", ConsoleColor.Green);
        }

        public static async Task InjectClientPacketAsync(int connectionId, byte[] decryptedPacket)
        {
            ProxySession? session;
            lock (ActiveSessions) { ActiveSessions.TryGetValue(connectionId, out session); }
            if (session != null)
                await session.SendClientPacketAsync(decryptedPacket, isInjected: true);
            else
                ProxyLog.Write("General", $"Cannot inject packet: connection {connectionId} not found", ConsoleColor.Red);
        }

        public async Task StartAsync(CancellationToken cancellationToken)
        {
            _listener = new TcpListener(IPAddress.Any, _localPort);
            _listener.Start();

            ProxyLog.Write("General", $"Proxy started on port {_localPort}, forwarding to {_remoteHost}:{_remotePort}", ConsoleColor.Green);
            ProxyLog.Write("General", $"Protection: {(_protectionEnabled ? "ENABLED" : "DISABLED")}", ConsoleColor.Gray);
            ProxyLog.Write("General", $"Logging to: {Path.GetFullPath(_logFilePath)}", ConsoleColor.Gray);

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
                ProxyLog.Write("General", $"Listener error: {ex.Message}", ConsoleColor.Red);
            }
            finally
            {
                _listener?.Stop();
                ProxyLog.Write("General", "Proxy stopped.", ConsoleColor.Red);
            }
        }

        private async Task HandleConnectionAsync(TcpClient client, int connId, CancellationToken cancellationToken)
        {
            ProxyLog.Write("General", $"[Conn #{connId}] Client connected from {client.Client.RemoteEndPoint}", ConsoleColor.Gray);

            using (client)
            using (TcpClient server = new TcpClient())
            {
                try
                {
                    await server.ConnectAsync(_remoteHost, _remotePort, cancellationToken);
                    ProxyLog.Write("General", $"[Conn #{connId}] Connected to server {_remoteHost}:{_remotePort}", ConsoleColor.Gray);

                    using (NetworkStream clientStream = client.GetStream())
                    using (NetworkStream serverStream = server.GetStream())
                    using (var cts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken))
                    {
                        var sessionState = new ProxySession { ConnectionId = connId, ServerStream = serverStream };
                        lock (ActiveSessions) { ActiveSessions[connId] = sessionState; }

                        HandshakeState handshakeState = new HandshakeState { PlaintextPassword = _plaintextPassword };
                        PacketEncryptor encryptor = new PacketEncryptor();
                        sessionState.Encryptor = encryptor;

                        var c2s = ForwardClientToServerAsync(clientStream, serverStream, connId, encryptor, handshakeState, cts.Token);
                        var s2c = ForwardServerToClientAsync(serverStream, clientStream, connId, encryptor, handshakeState, cts.Token);
                        await Task.WhenAny(c2s, s2c);
                        cts.Cancel();
                    }
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("General", $"[Conn #{connId}] Connection error: {ex.Message}", ConsoleColor.Red);
                }
                finally
                {
                    lock (ActiveSessions) { ActiveSessions.Remove(connId); }
                    ProxyLog.Write("General", $"[Conn #{connId}] Disconnected.", ConsoleColor.Gray);
                }
            }
        }

        private async Task ForwardClientToServerAsync(NetworkStream clientStream, NetworkStream serverStream, int connId,
            PacketEncryptor encryptor, HandshakeState state, CancellationToken cancellationToken)
        {
            var reader = new TcpStreamPacketReader(clientStream);

            while (!cancellationToken.IsCancellationRequested)
            {
                byte[]? packet = await reader.ReadPacketAsync(cancellationToken);
                if (packet == null) break;

                // Heartbeat
                if (packet.Length == 2)
                {
                    await serverStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
                    continue;
                }

                byte[] copy = (byte[])packet.Clone();
                uint session = (uint)((copy[2] << 24) | (copy[3] << 16) | (copy[4] << 8) | copy[5]);

                if (encryptor.Enabled)
                {
                    byte[] payload = new byte[copy.Length - 6];
                    Array.Copy(copy, 6, payload, 0, payload.Length);
                    encryptor.Decrypt(payload, DecryptType.CS);
                    Array.Copy(payload, 0, copy, 6, payload.Length);
                }

                var pkoPacket = new PkoPacket(copy);
                ushort packetId = pkoPacket.Command;

                // Parse handshake info
                var pktReader = new PkoPacketReader(copy);
                pktReader.ReadUint16(); // size
                pktReader.ReadUint32(); // session
                pktReader.ReadUint16(); // packetId
                if (pkoPacket.HasPacketCount) pktReader.ReadUint32();

                if (packetId == 431) // AccountLogin
                {
                    try
                    {
                        _ = pktReader.ReadString(); // nobill
                        string loginVal = pktReader.ReadString();
                        ushort pwdLen = pktReader.ReadUint16();
                        byte[] pwdBytes = pktReader.ReadBytes(pwdLen);
                        string mac = pktReader.ReadString();
                        _ = pktReader.ReadUint16(); // flag
                        ushort versionVal = pktReader.ReadUint16();

                        lock (state)
                        {
                            state.Login = loginVal;
                            state.PasswordBytes = pwdBytes;
                            state.Version = versionVal;
                        }

                        ProxyLog.Write("Handshake", $"[Conn #{connId}] Intercepted Login: {loginVal}, Version: {versionVal}, Mac: {mac}", ConsoleColor.Magenta);
                    }
                    catch (Exception ex)
                    {
                        ProxyLog.Write("Handshake", $"[Conn #{connId}] Parse error ID 431: {ex.Message}", ConsoleColor.Red);
                    }
                }

                // Run plugins
                var context = new ProxyPacketContext(connId, "C -> S", pkoPacket);
                foreach (var plugin in Plugins)
                {
                    try
                    {
                        if (ProxyLog.LogDebug) ProxyLog.Write("Plugin", $"Calling plugin {plugin.Name} on C->S packet {packetId}", ConsoleColor.DarkGray);
                        plugin.OnPacket(context);
                    }
                    catch (Exception ex)
                    {
                        ProxyLog.Write("Plugin", $"Plugin {plugin.Name} threw: {ex.Message}", ConsoleColor.Red);
                    }
                }

                if (context.IsDropped)
                {
                    ProxyLog.Write("Packet", $"[Conn #{connId}] C->S {PkoCommandTranslator.GetCommandName(packetId)} DROPPED by plugin", ConsoleColor.Red);
                    continue;
                }

                if (ProxyLog.LogRawHex)
                    ProxyLog.HexDump("Raw", context.DecryptedPacket, $"C->S {packetId} (modified?)");

                // Send
                ProxySession? activeSess;
                lock (ActiveSessions) { ActiveSessions.TryGetValue(connId, out activeSess); }
                if (activeSess != null)
                    await activeSess.SendClientPacketAsync(context.DecryptedPacket);
                else
                    await serverStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
            }
        }

        private async Task ForwardServerToClientAsync(NetworkStream serverStream, NetworkStream clientStream, int connId,
            PacketEncryptor encryptor, HandshakeState state, CancellationToken cancellationToken)
        {
            var reader = new TcpStreamPacketReader(serverStream);

            while (!cancellationToken.IsCancellationRequested)
            {
                byte[]? packet = await reader.ReadPacketAsync(cancellationToken);
                if (packet == null) break;

                if (packet.Length == 2)
                {
                    await clientStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
                    continue;
                }

                byte[] copy = (byte[])packet.Clone();
                uint session = (uint)((copy[2] << 24) | (copy[3] << 16) | (copy[4] << 8) | copy[5]);

                ushort packetIdBeforeDec = 0;
                if (!encryptor.Enabled)
                    packetIdBeforeDec = (ushort)((copy[6] << 8) | copy[7]);

                if (encryptor.Enabled && packetIdBeforeDec != 931)
                {
                    byte[] payload = new byte[copy.Length - 6];
                    Array.Copy(copy, 6, payload, 0, payload.Length);
                    encryptor.Decrypt(payload, DecryptType.SC);
                    Array.Copy(payload, 0, copy, 6, payload.Length);
                }

                var pktReader = new PkoPacketReader(copy);
                pktReader.ReadUint16(); // size
                pktReader.ReadUint32(); // session
                ushort packetId = pktReader.ReadUint16();

                // Handshake packets
                if (packetId == 940) // ChapString
                {
                    try
                    {
                        string chapString = pktReader.ReadString();
                        lock (state) { state.ChapString = chapString; }
                        ProxyLog.Write("Handshake", $"[Conn #{connId}] CHAP challenge: \"{chapString}\"", ConsoleColor.Magenta);
                    }
                    catch (Exception ex)
                    {
                        ProxyLog.Write("Handshake", $"[Conn #{connId}] Parse error ID 940: {ex.Message}", ConsoleColor.Red);
                    }
                }
                else if (packetId == 931) // LoginResult
                {
                    try
                    {
                        ushort result = pktReader.ReadUint16();
                        ProxyLog.Write("Handshake", $"[Conn #{connId}] LoginResult: {result} ({(result == 0 ? "SUCCESS" : "FAIL")})", result == 0 ? ConsoleColor.Green : ConsoleColor.Red);

                        if (result == 0)
                        {
                            ushort keyLen = pktReader.ReadUint16();
                            byte[] encryptionKey = pktReader.ReadBytes(keyLen);
                            pktReader.SeekFromEnd(8);
                            uint commEncryptionVal = pktReader.ReadUint32();
                            bool commEncryption = commEncryptionVal != 0;

                            if (ProxyLog.LogDebug)
                            {
                                var keyHex = BitConverter.ToString(encryptionKey).Replace("-", " ");
                                ProxyLog.Write("Handshake", $"EncryptionKey: {keyHex}", ConsoleColor.DarkGray);
                            }

                            if (commEncryption)
                            {
                                lock (state)
                                {
                                    var candidates = new List<byte[]>();
                                    if (state.PasswordBytes.Length > 0) candidates.Add(state.PasswordBytes);
                                    if (state.PasswordBytes.Length > 0)
                                    {
                                        byte[] trunc = new byte[state.PasswordBytes.Length - 1];
                                        Array.Copy(state.PasswordBytes, trunc, trunc.Length);
                                        candidates.Add(trunc);
                                    }
                                    if (!string.IsNullOrEmpty(state.PlaintextPassword))
                                    {
                                        string md5 = GetMD5Hash(state.PlaintextPassword);
                                        candidates.Add(Encoding.ASCII.GetBytes(md5));
                                        candidates.Add(Encoding.ASCII.GetBytes(state.PlaintextPassword));
                                    }
                                    encryptor.SetCandidates(candidates, state.Version, state.ChapString, encryptionKey);
                                    ProxyLog.Write("Handshake", $"[Conn #{connId}] Encryption initialized with {candidates.Count} candidates", ConsoleColor.Green);
                                }
                            }
                        }
                    }
                    catch (Exception ex)
                    {
                        ProxyLog.Write("Handshake", $"[Conn #{connId}] Parse error ID 931: {ex.Message}", ConsoleColor.Red);
                    }
                }

                // Run plugins
                var context = new ProxyPacketContext(connId, "S -> C", packetId, session, copy);
                foreach (var plugin in Plugins)
                {
                    try
                    {
                        if (ProxyLog.LogDebug) ProxyLog.Write("Plugin", $"Calling plugin {plugin.Name} on S->C packet {packetId}", ConsoleColor.DarkGray);
                        plugin.OnPacket(context);
                    }
                    catch (Exception ex)
                    {
                        ProxyLog.Write("Plugin", $"Plugin {plugin.Name} threw: {ex.Message}", ConsoleColor.Red);
                    }
                }

                if (context.IsDropped)
                {
                    ProxyLog.Write("Packet", $"[Conn #{connId}] S->C {PkoCommandTranslator.GetCommandName(packetId)} DROPPED by plugin", ConsoleColor.Red);
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

                if (ProxyLog.LogRawHex)
                    ProxyLog.HexDump("Raw", packet, $"S->C {packetId}");

                await clientStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
            }
        }

        public static string GetMD5Hash(string input)
        {
            using (var md5 = System.Security.Cryptography.MD5.Create())
            {
                byte[] inputBytes = Encoding.ASCII.GetBytes(input);
                byte[] hashBytes = md5.ComputeHash(inputBytes);
                var sb = new StringBuilder();
                for (int i = 0; i < hashBytes.Length; i++)
                    sb.Append(hashBytes[i].ToString("X2"));
                return sb.ToString();
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
            var sb = new StringBuilder();
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
                    sb.Append(c >= 32 && c <= 126 ? c : '.');
                }
                sb.AppendLine();
            }
            return sb.ToString();
        }
    }
}