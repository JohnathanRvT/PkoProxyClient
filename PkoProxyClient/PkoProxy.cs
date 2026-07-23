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
        public PkoPacket Packet { get; }
        public ushort PacketId
        {
            get => Packet.Command;
            set => Packet.Command = value;
        }
        public uint Session
        {
            get => Packet.Session;
            set => Packet.Session = value;
        }
        public byte[] DecryptedPacket
        {
            get => Packet.RawBytes;
            set => Packet.RawBytes = value;
        }
        public bool IsDropped { get; set; } = false;

        public ProxyPacketContext(int connectionId, string direction, PkoPacket packet)
        {
            ConnectionId = connectionId;
            Direction = direction ?? throw new ArgumentNullException(nameof(direction));
            Packet = packet ?? throw new ArgumentNullException(nameof(packet));
        }

        public ProxyPacketContext(int connectionId, string direction, ushort packetId, uint session, byte[] decryptedPacket)
        {
            ConnectionId = connectionId;
            Direction = direction ?? throw new ArgumentNullException(nameof(direction));
            Packet = new PkoPacket(decryptedPacket);
            PacketId = packetId;
            Session = session;
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
    /// Holds session-specific TCP stream reference and encryptor states for a specific client connection,
    /// enabling packet injection directly into the server stream.
    /// </summary>
    public class ProxySession
    {
        public int ConnectionId { get; set; }
        public NetworkStream? ServerStream { get; set; }
        public PacketEncryptor? Encryptor { get; set; }
        public uint SessionId { get; set; }
        public uint PlayerWorldId { get; set; }
        public uint NextPacketCount { get; set; } = 0;
        public bool PacketCountInitialized { get; set; } = false;

        private readonly object _sendLock = new object();

        /// <summary>
        /// Sequences, encrypts, and sends a decrypted client-to-server packet to the server stream.
        /// </summary>
        public async Task SendClientPacketAsync(byte[] decryptedPacket)
        {
            if (ServerStream == null) return;

            // Clone to avoid modifying original plugin context buffers
            byte[] copy = (byte[])decryptedPacket.Clone();
            var pkt = new PkoPacket(copy);

            lock (_sendLock)
            {
                // Keep the active SessionId updated
                SessionId = pkt.Session;

                // Sync packet length in header to reflect any plugin payload size modifications
                pkt.Size = (ushort)copy.Length;

                if (pkt.HasPacketCount)
                {
                    if (!PacketCountInitialized)
                    {
                        NextPacketCount = pkt.PacketCount;
                        PacketCountInitialized = true;
                    }

                    pkt.PacketCount = NextPacketCount++;
                }

                // Encrypt payload if encryptor has been enabled
                if (Encryptor != null && Encryptor.Enabled)
                {
                    byte[] payload = new byte[copy.Length - 6];
                    Array.Copy(copy, 6, payload, 0, payload.Length);
                    Encryptor.Encrypt(payload, EncryptType.CS);
                    Array.Copy(payload, 0, copy, 6, payload.Length);
                }
            }

            await ServerStream.WriteAsync(copy, 0, copy.Length);
        }
    }

    /// <summary>
    /// Represents a basic tracked monster/character entity visible to the client.
    /// </summary>
    public class MobInfo
    {
        public uint WorldId { get; set; }
        public string Name { get; set; } = "";
        public int X { get; set; }
        public int Y { get; set; }
    }

    /// <summary>
    /// Represents a tracked scene item on the ground visible to the client.
    /// </summary>
    public class ItemInfo
    {
        public uint WorldId { get; set; }
        public uint Handle { get; set; }
        public uint ItemId { get; set; }
        public int X { get; set; }
        public int Y { get; set; }
    }

    /// <summary>
    /// Intelligent automated farming bot proxy plugin.
    /// Intercepts chat commands starting with /bot, keeps track of nearby mobs and items by listening to
    /// see/endsee packets (504, 505, 506, 507), and runs a background farming loop injecting attack and loot packets.
    /// </summary>
    public class AutoBotPlugin : IProxyPlugin
    {
        public string Name => "AutoBot";
        public bool Enabled { get; set; } = false;

        private readonly System.Collections.Generic.Dictionary<uint, MobInfo> _mobs = new System.Collections.Generic.Dictionary<uint, MobInfo>();
        private readonly System.Collections.Generic.Dictionary<uint, ItemInfo> _items = new System.Collections.Generic.Dictionary<uint, ItemInfo>();
        private uint _playerWorldId = 0;
        private int _playerX = 0;
        private int _playerY = 0;
        private int _lastConnectionId = 0;
        private readonly object _lock = new object();
        private readonly System.Threading.Timer _loopTimer;

        public AutoBotPlugin()
        {
            _loopTimer = new System.Threading.Timer(OnBotLoop, null, 1500, 1500);
        }

        public void OnPacket(ProxyPacketContext context)
        {
            _lastConnectionId = context.ConnectionId;

            // Capture player world ID and track coordinates from CMD_CM_BEGINACTION (6)
            if (context.Direction == "C -> S" && context.PacketId == 6)
            {
                try
                {
                    var pktReader = new PkoPacketReader(context.DecryptedPacket);
                    _ = pktReader.ReadUint16(); // skip size
                    _ = pktReader.ReadUint32(); // skip session
                    _ = pktReader.ReadUint16(); // skip packetId (6)
                    _ = pktReader.ReadUint32(); // skip packetCount (4 bytes)
                    uint charWorldId = pktReader.ReadUint32();
                    lock (_lock)
                    {
                        _playerWorldId = charWorldId;
                    }

                    byte actionType = pktReader.ReadByte();
                    if (actionType == 1) // enumACTION_MOVE (1)
                    {
                        ushort turnNumBytes = pktReader.ReadUint16();
                        if (turnNumBytes >= 8)
                        {
                            // Skip to the last point (each point has size 8 bytes: X=4, Y=4)
                            int pointsToSkip = (turnNumBytes / 8) - 1;
                            if (pointsToSkip > 0)
                            {
                                _ = pktReader.ReadBytes(pointsToSkip * 8);
                            }
                            int lastX = (int)pktReader.ReadUint32();
                            int lastY = (int)pktReader.ReadUint32();
                            lock (_lock)
                            {
                                _playerX = lastX;
                                _playerY = lastY;
                            }
                        }
                    }
                }
                catch { }
            }

            // Capture /bot chat command inputs inside CMD_CM_SAY (1)
            if (context.Direction == "C -> S" && context.PacketId == 1)
            {
                var pktReader = new PkoPacketReader(context.DecryptedPacket);
                _ = pktReader.ReadUint16(); // skip size
                _ = pktReader.ReadUint32(); // skip session
                _ = pktReader.ReadUint16(); // skip packetId (1)

                try
                {
                    string chatMsg = pktReader.ReadString();
                    if (chatMsg.StartsWith("/bot_start"))
                    {
                        Enabled = true;
                        Console.ForegroundColor = ConsoleColor.Green;
                        Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] [AutoBot] Bot Loop STARTED!");
                        Console.ResetColor();
                        context.IsDropped = true; // Drop command packet so it is never sent to the server
                    }
                    else if (chatMsg.StartsWith("/bot_stop"))
                    {
                        Enabled = false;
                        Console.ForegroundColor = ConsoleColor.Red;
                        Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] [AutoBot] Bot Loop STOPPED!");
                        Console.ResetColor();
                        context.IsDropped = true; // Drop command packet
                    }
                    else if (chatMsg.StartsWith("/bot_status"))
                    {
                        lock (_lock)
                        {
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] [AutoBot] Status: Enabled={Enabled} | Known Mobs={_mobs.Count} | Known Items={_items.Count} | Player ID=0x{_playerWorldId:X} | Coords=({_playerX}, {_playerY})");
                            Console.ResetColor();
                        }
                        context.IsDropped = true; // Drop command packet
                    }
                }
                catch { }
            }

            // Track entities and items on the ground (S -> C packets)
            if (context.Direction == "S -> C")
            {
                // CMD_MC_CHABEGINSEE (504)
                if (context.PacketId == 504)
                {
                    try
                    {
                        var pktReader = new PkoPacketReader(context.DecryptedPacket);
                        _ = pktReader.ReadUint16(); // size
                        _ = pktReader.ReadUint32(); // session
                        _ = pktReader.ReadUint16(); // ID (504)
                        _ = pktReader.ReadByte();   // chSeeType
                        _ = pktReader.ReadUint32(); // ulChaID
                        uint ulWorldID = pktReader.ReadUint32();
                        _ = pktReader.ReadUint32(); // ulCommID
                        _ = pktReader.ReadString(); // szCommName
                        _ = pktReader.ReadByte();   // chGMLv
                        _ = pktReader.ReadUint32(); // lHandle
                        _ = pktReader.ReadByte();   // chCtrlType
                        string szName = pktReader.ReadString();
                        _ = pktReader.ReadString(); // szMotto
                        _ = pktReader.ReadUint16(); // icon
                        _ = pktReader.ReadUint32(); // guildID
                        _ = pktReader.ReadString(); // guildName
                        _ = pktReader.ReadString(); // guildMotto
                        _ = pktReader.ReadString(); // stallName
                        _ = pktReader.ReadUint16(); // existState
                        int x = (int)pktReader.ReadUint32();
                        int y = (int)pktReader.ReadUint32();

                        lock (_lock)
                        {
                            _mobs[ulWorldID] = new MobInfo { WorldId = ulWorldID, Name = szName, X = x, Y = y };
                            if (ulWorldID == _playerWorldId)
                            {
                                _playerX = x;
                                _playerY = y;
                            }
                        }
                    }
                    catch { }
                }

                // CMD_MC_CHAENDSEE (505)
                if (context.PacketId == 505)
                {
                    try
                    {
                        var pktReader = new PkoPacketReader(context.DecryptedPacket);
                        _ = pktReader.ReadUint16(); // size
                        _ = pktReader.ReadUint32(); // session
                        _ = pktReader.ReadUint16(); // ID (505)
                        _ = pktReader.ReadByte();   // chSeeType
                        uint lWorldID = pktReader.ReadUint32();

                        lock (_lock)
                        {
                            _mobs.Remove(lWorldID);
                        }
                    }
                    catch { }
                }

                // CMD_MC_ITEMBEGINSEE (506)
                if (context.PacketId == 506)
                {
                    try
                    {
                        var pktReader = new PkoPacketReader(context.DecryptedPacket);
                        _ = pktReader.ReadUint16(); // size
                        _ = pktReader.ReadUint32(); // session
                        _ = pktReader.ReadUint16(); // ID (506)
                        uint lWorldID = pktReader.ReadUint32();
                        uint lHandle = pktReader.ReadUint32();
                        uint lID = pktReader.ReadUint32();
                        int x = (int)pktReader.ReadUint32();
                        int y = (int)pktReader.ReadUint32();

                        lock (_lock)
                        {
                            _items[lWorldID] = new ItemInfo { WorldId = lWorldID, Handle = lHandle, ItemId = lID, X = x, Y = y };
                        }
                    }
                    catch { }
                }

                // CMD_MC_ITEMENDSEE (507)
                if (context.PacketId == 507)
                {
                    try
                    {
                        var pktReader = new PkoPacketReader(context.DecryptedPacket);
                        _ = pktReader.ReadUint16(); // size
                        _ = pktReader.ReadUint32(); // session
                        _ = pktReader.ReadUint16(); // ID (507)
                        uint lWorldID = pktReader.ReadUint32();

                        lock (_lock)
                        {
                            _items.Remove(lWorldID);
                        }
                    }
                    catch { }
                }
            }
        }

        private void OnBotLoop(object? state)
        {
            if (!Enabled || _lastConnectionId == 0) return;

            uint targetPlayerId = 0;
            uint targetSessionId = 0;

            lock (_lock)
            {
                targetPlayerId = _playerWorldId;
            }

            if (targetPlayerId == 0) return;

            // Retrieve proxy session metrics & packet count under lock
            lock (PkoProxy.ActiveSessions)
            {
                if (PkoProxy.ActiveSessions.TryGetValue(_lastConnectionId, out var session))
                {
                    targetSessionId = session.SessionId;
                    session.PlayerWorldId = targetPlayerId;
                }
            }

            if (targetSessionId == 0) return;

            // 1. Target picking up ground items first
            ItemInfo? pickItem = null;
            lock (_lock)
            {
                foreach (var item in _items.Values)
                {
                    pickItem = item;
                    break;
                }
            }

            if (pickItem != null)
            {
                bool hasCoords = false;
                int currentX = 0;
                int currentY = 0;
                lock (_lock)
                {
                    hasCoords = (_playerX != 0 || _playerY != 0);
                    currentX = _playerX;
                    currentY = _playerY;
                }

                double dist = hasCoords ? Math.Sqrt(Math.Pow(pickItem.X - currentX, 2) + Math.Pow(pickItem.Y - currentY, 2)) : 0;

                if (!hasCoords || dist <= 350)
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] [AutoBot] Looting Item (Dist={dist:F1}): WorldId={pickItem.WorldId}, Handle={pickItem.Handle}");
                    Console.ResetColor();

                    var writer = new PkoPacketWriter();
                    writer.WriteUint16(0); // size placeholder
                    writer.WriteUint32(targetSessionId);
                    writer.WriteUint16(6); // CMD_CM_BEGINACTION
                    writer.WriteUint32(0); // write sequence placeholder (rewritten centrally by ProxySession)
                    writer.WriteUint32(targetPlayerId);
                    writer.WriteByte(8); // enumACTION_ITEM_PICK
                    writer.WriteUint32(pickItem.WorldId);
                    writer.WriteUint32(pickItem.Handle);

                    _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
                }
                else
                {
                    Console.ForegroundColor = ConsoleColor.Yellow;
                    Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] [AutoBot] Item too far (Dist={dist:F1}). Moving to: ({pickItem.X}, {pickItem.Y})");
                    Console.ResetColor();

                    var writer = new PkoPacketWriter();
                    writer.WriteUint16(0); // size placeholder
                    writer.WriteUint32(targetSessionId);
                    writer.WriteUint16(6); // CMD_CM_BEGINACTION
                    writer.WriteUint32(0); // sequence placeholder
                    writer.WriteUint32(targetPlayerId);
                    writer.WriteByte(1); // actionType: enumACTION_MOVE (1)
                    writer.WriteUint16(8); // TurnNum (1 point = 8 bytes)
                    writer.WriteUint32((uint)pickItem.X);
                    writer.WriteUint32((uint)pickItem.Y);

                    lock (_lock)
                    {
                        _playerX = pickItem.X;
                        _playerY = pickItem.Y;
                    }

                    _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
                }
                return;
            }

            // 2. If no items are found, execute physical attack on visible mobs
            MobInfo? attackMob = null;
            lock (_lock)
            {
                foreach (var mob in _mobs.Values)
                {
                    if (mob.WorldId == targetPlayerId) continue;
                    attackMob = mob;
                    break;
                }
            }

            if (attackMob != null)
            {
                int targetX = 0;
                int targetY = 0;
                lock (_lock)
                {
                    targetX = (attackMob.X != 0 || attackMob.Y != 0) ? attackMob.X : _playerX;
                    targetY = (attackMob.X != 0 || attackMob.Y != 0) ? attackMob.Y : _playerY;
                }

                Console.ForegroundColor = ConsoleColor.Cyan;
                Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] [AutoBot] Attacking Mob: {attackMob.Name} (WorldId={attackMob.WorldId}) at ({targetX}, {targetY})");
                Console.ResetColor();

                var writer = new PkoPacketWriter();
                writer.WriteUint16(0); // size placeholder
                writer.WriteUint32(targetSessionId);
                writer.WriteUint16(6); // CMD_CM_BEGINACTION
                writer.WriteUint32(0); // write sequence placeholder (rewritten centrally by ProxySession)
                writer.WriteUint32(targetPlayerId);
                writer.WriteByte(2); // enumACTION_SKILL
                writer.WriteByte(2); // chMove = 2 (pre-movement)
                writer.WriteByte(0); // byFightID

                // Point sequence path
                writer.WriteUint16(8); // turnNumBytes: 8 (1 point)
                writer.WriteUint32((uint)targetX);
                writer.WriteUint32((uint)targetY);

                writer.WriteUint32(0); // lSkillID (0 = basic attack)
                writer.WriteUint32(attackMob.WorldId); // lTarInfo1 (target mob ID)
                writer.WriteUint32(0); // lTarInfo2

                lock (_lock)
                {
                    _playerX = targetX;
                    _playerY = targetY;
                }

                _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
            }
        }
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
        public static readonly System.Collections.Generic.Dictionary<int, ProxySession> ActiveSessions = new System.Collections.Generic.Dictionary<int, ProxySession>();

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
            Plugins.Add(new AutoBotPlugin());
        }

        /// <summary>
        /// Injects an unencrypted packet into an active connection server stream.
        /// It formats the Big-Endian length, encrypts the payload, and sends it directly.
        /// </summary>
        public static async Task InjectClientPacketAsync(int connectionId, byte[] decryptedPacket)
        {
            ProxySession? session;
            lock (ActiveSessions)
            {
                ActiveSessions.TryGetValue(connectionId, out session);
            }

            if (session != null)
            {
                await session.SendClientPacketAsync(decryptedPacket);
            }
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
                            // Register active session
                            var sessionState = new ProxySession
                            {
                                ConnectionId = connId,
                                ServerStream = serverStream
                            };
                            lock (ActiveSessions)
                            {
                                ActiveSessions[connId] = sessionState;
                            }

                            try
                            {
                                // Stateful crypto parameters for this connection
                                HandshakeState handshakeState = new HandshakeState();
                                handshakeState.PlaintextPassword = _plaintextPassword;
                                PacketEncryptor encryptor = new PacketEncryptor();

                                sessionState.Encryptor = encryptor;

                                // Task for Client to Server direction
                                var clientToServerTask = ForwardClientToServerAsync(clientStream, serverStream, connId, encryptor, handshakeState, cts.Token);

                                // Task for Server to Client direction
                                var serverToClientTask = ForwardServerToClientAsync(serverStream, clientStream, connId, encryptor, handshakeState, cts.Token);

                                await Task.WhenAny(clientToServerTask, serverToClientTask);
                                cts.Cancel();
                            }
                            finally
                            {
                                lock (ActiveSessions)
                                {
                                    ActiveSessions.Remove(connId);
                                }
                            }
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

        private async Task ForwardClientToServerAsync(NetworkStream clientStream, NetworkStream serverStream, int connId, PacketEncryptor encryptor, HandshakeState state, CancellationToken cancellationToken)
        {
            var reader = new TcpStreamPacketReader(clientStream);

            while (!cancellationToken.IsCancellationRequested)
            {
                byte[]? packet = await reader.ReadPacketAsync(cancellationToken);
                if (packet == null) break;

                // Handle 2-byte heartbeat packet
                if (packet.Length == 2)
                {
                    await serverStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
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

                var pkoPacket = new PkoPacket(copy);
                ushort packetId = pkoPacket.Command;

                // Handle specific handshake packets to capture info (requires original sequentially structured stream fields)
                var pktReader = new PkoPacketReader(copy);
                pktReader.ReadUint16(); // Skip size
                pktReader.ReadUint32(); // Skip session
                _ = pktReader.ReadUint16(); // Skip packetId
                if (pkoPacket.HasPacketCount)
                {
                    _ = pktReader.ReadUint32(); // Skip packetCount
                }

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
                var context = new ProxyPacketContext(connId, "C -> S", pkoPacket);
                foreach (var plugin in Plugins)
                {
                    try { plugin.OnPacket(context); } catch { }
                }

                if (context.IsDropped)
                {
                    LogConsole($"[Connection #{connId}] C -> S | {PkoCommandTranslator.GetCommandName(packetId)} ({packetId}) DROPPED by plugin.");
                    continue;
                }

                // Forward packet to server via centralized SendClientPacketAsync
                ProxySession? activeSess;
                lock (ActiveSessions)
                {
                    ActiveSessions.TryGetValue(connId, out activeSess);
                }

                if (activeSess != null)
                {
                    await activeSess.SendClientPacketAsync(context.DecryptedPacket);
                }
                else
                {
                    // Fallback to basic sending if session not registered yet
                    await serverStream.WriteAsync(packet, 0, packet.Length, cancellationToken);
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
                    LogConsole($"[Connection #{connId}] S -> C | {PkoCommandTranslator.GetCommandName(packetId)} ({packetId}) DROPPED by plugin.");
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
