namespace PkoProxyClient;

public class AutoBotPlugin : IProxyPlugin
{
    public string Name => "AutoBot";
    public bool Enabled { get; set; } = false;

    private readonly Dictionary<uint, MobInfo> _mobs = new Dictionary<uint, MobInfo>();
    private readonly Dictionary<uint, ItemInfo> _items = new Dictionary<uint, ItemInfo>();
    private uint _playerWorldId = 0;
    private int _playerX = 0;
    private int _playerY = 0;
    private int _lastConnectionId = 0;
    private uint _packetID = 0;
    private readonly object _lock = new object();
    private readonly Timer _loopTimer;

    public AutoBotPlugin()
    {
        _loopTimer = new Timer(OnBotLoop, null, 1500, 1500);
        ProxyLog.Write("Bot", "AutoBot plugin initialized, timer set for 1500ms intervals", ConsoleColor.DarkCyan);
    }

    public void OnPacket(ProxyPacketContext context)
    {
        _lastConnectionId = context.ConnectionId;
        if (ProxyLog.LogDebug)
            ProxyLog.Write("Bot", $"OnPacket: Dir={context.Direction}, PktId={context.PacketId}, ConnId={context.ConnectionId}", ConsoleColor.DarkGray);

        // Capture player world ID and coordinates from CMD_CM_BEGINACTION (6)
        if (context.Direction == "C -> S" && context.PacketId == 6)
        {
            try
            {
                var pktReader = new PkoPacketReader(context.DecryptedPacket);
                _ = pktReader.ReadUint16(); // size
                _ = pktReader.ReadUint32(); // session
                _ = pktReader.ReadUint16(); // packetId
                _ = pktReader.ReadUint32(); // packetCount
                uint charWorldId = pktReader.ReadUint32();
                lock (_lock)
                {
                    _playerWorldId = charWorldId;
                    if (ProxyLog.LogDebug)
                        ProxyLog.Write("Bot", $"Player World ID captured: 0x{charWorldId:X}", ConsoleColor.DarkGray);
                }
                byte actionType = pktReader.ReadByte();
                if (actionType == 1) // Move
                {
                    ushort turnNumBytes = pktReader.ReadUint16();
                    if (turnNumBytes >= 8)
                    {
                        int pointsToSkip = (turnNumBytes / 8) - 1;
                        if (pointsToSkip > 0)
                            _ = pktReader.ReadBytes(pointsToSkip * 8);
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
            catch (Exception ex)
            {
                ProxyLog.Write("Bot", $"ERROR parsing CMD_CM_BEGINACTION: {ex.Message}", ConsoleColor.Red);
                if (ProxyLog.LogRawHex)
                    ProxyLog.HexDump("Bot", context.DecryptedPacket, "BeginAction raw:");
            }
        }

        // Handle /bot commands
        if (context.Direction == "C -> S" && context.PacketId == 1)
        {
            try
            {
                var pktReader = new PkoPacketReader(context.DecryptedPacket);
                _ = pktReader.ReadUint16(); // size
                _ = pktReader.ReadUint32(); // session
                _ = pktReader.ReadUint16(); // packetId
                _ = pktReader.ReadUint32(); // packetCount? skip
                string chatMsg = pktReader.ReadString();
                if (ProxyLog.LogDebug)
                    ProxyLog.Write("Bot", $"Chat message: '{chatMsg}'", ConsoleColor.DarkGray);

                if (chatMsg.StartsWith("/bot_start"))
                {
                    Enabled = true;
                    ProxyLog.Write("Bot", "Bot Loop STARTED!", ConsoleColor.Green);
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_stop"))
                {
                    Enabled = false;
                    ProxyLog.Write("Bot", "Bot Loop STOPPED!", ConsoleColor.Red);
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_status"))
                {
                    lock (_lock)
                    {
                        ProxyLog.Write("Bot", $"Status: Enabled={Enabled} | Mobs={_mobs.Count} | Items={_items.Count} | Player=0x{_playerWorldId:X} | Coords=({_playerX},{_playerY})", ConsoleColor.Cyan);
                        if (ProxyLog.LogDebug)
                        {
                            foreach (var mob in _mobs)
                                ProxyLog.Write("Bot", $"  Mob: 0x{mob.Key:X} Name={mob.Value.Name} Handle={mob.Value.Handle}", ConsoleColor.DarkGray);
                            foreach (var item in _items)
                                ProxyLog.Write("Bot", $"  Item: 0x{item.Key:X} ItemId={item.Value.ItemId} Pos=({item.Value.X},{item.Value.Y})", ConsoleColor.DarkGray);
                        }
                    }
                    context.IsDropped = true;
                }
            }
            catch (Exception ex)
            {
                ProxyLog.Write("Bot", $"ERROR parsing chat packet: {ex.Message}", ConsoleColor.Red);
            }
        }

        // Track entities and items (S -> C)
        if (context.Direction == "S -> C")
        {
            // CMD_MC_CHABEGINSEE (504) - Mob appears
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
                    _ = pktReader.ReadString(); // szCommName (may be empty)
                    _ = pktReader.ReadByte();   // chGMLv
                    uint ulHandle = pktReader.ReadUint32(); // lHandle
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
                        _mobs[ulWorldID] = new MobInfo { Handle = ulHandle, WorldId = ulWorldID, Name = szName, X = x, Y = y };
                        if (ulWorldID == _playerWorldId)
                        {
                            _playerX = x;
                            _playerY = y;
                        }
                        if (ProxyLog.LogDebug)
                            ProxyLog.Write("Bot", $"Mob appears: 0x{ulWorldID:X} '{szName}' at ({x},{y})", ConsoleColor.DarkGray);
                    }
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"ERROR parsing CMD_MC_CHABEGINSEE: {ex.Message}", ConsoleColor.Red);
                    if (ProxyLog.LogRawHex)
                        ProxyLog.HexDump("Bot", context.DecryptedPacket, "CHABEGINSEE raw:");
                }
            }

            // CMD_MC_CHAENDSEE (505) - Mob disappears
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
                        bool removed = _mobs.Remove(lWorldID);
                        if (ProxyLog.LogDebug)
                            ProxyLog.Write("Bot", $"Mob removed: 0x{lWorldID:X} (removed={removed})", ConsoleColor.DarkGray);
                    }
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"ERROR parsing CMD_MC_CHAENDSEE: {ex.Message}", ConsoleColor.Red);
                }
            }

            // CMD_MC_ITEMBEGINSEE (506) - Item appears
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
                        if (ProxyLog.LogDebug)
                            ProxyLog.Write("Bot", $"Item appears: 0x{lWorldID:X} ID={lID} at ({x},{y})", ConsoleColor.DarkGray);
                    }
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"ERROR parsing CMD_MC_ITEMBEGINSEE: {ex.Message}", ConsoleColor.Red);
                }
            }

            // CMD_MC_ITEMENDSEE (507) - Item disappears
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
                        bool removed = _items.Remove(lWorldID);
                        if (ProxyLog.LogDebug)
                            ProxyLog.Write("Bot", $"Item removed: 0x{lWorldID:X} (removed={removed})", ConsoleColor.DarkGray);
                    }
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"ERROR parsing CMD_MC_ITEMENDSEE: {ex.Message}", ConsoleColor.Red);
                }
            }
        }
    }

    private void OnBotLoop(object? state)
    {
        if (!Enabled) return;
        if (_lastConnectionId == 0) return;

        uint targetPlayerId = 0;
        uint targetSessionId = 0;
        lock (_lock) { targetPlayerId = _playerWorldId; }
        if (targetPlayerId == 0) return;

        // Get session
        ProxySession? session = null;
        lock (PkoProxy.ActiveSessions) { PkoProxy.ActiveSessions.TryGetValue(_lastConnectionId, out session); }
        if (session == null) return;
        targetSessionId = session.SessionId;
        session.PlayerWorldId = targetPlayerId;

        if (targetSessionId == 0) return;

        // 1. Pick up items
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
            int currentX = 0, currentY = 0;
            lock (_lock) { currentX = _playerX; currentY = _playerY; }
            double dist = Math.Sqrt(Math.Pow(pickItem.X - currentX, 2) + Math.Pow(pickItem.Y - currentY, 2));

            if (dist <= 350)
            {
                ProxyLog.Write("Bot", $"LOOTING item 0x{pickItem.WorldId:X} (dist={dist:F1})", ConsoleColor.Green);
                try
                {
                    var writer = new PkoPacketWriter();
                    writer.WriteUint16(0);
                    writer.WriteUint32(targetSessionId);
                    writer.WriteUint16(6);
                    writer.WriteUint32(0);
                    writer.WriteUint32(targetPlayerId);
                    writer.WriteUint32(_packetID++);
                    writer.WriteByte(8); // enumACTION_ITEM_PICK
                    writer.WriteUint32(pickItem.WorldId);
                    writer.WriteUint32(pickItem.Handle);
                    writer.WriteUint16(6332);
                    _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"Error sending loot packet: {ex.Message}", ConsoleColor.Red);
                }
            }
            else
            {
                ProxyLog.Write("Bot", $"Item too far (dist={dist:F1}), moving to ({pickItem.X},{pickItem.Y})", ConsoleColor.Yellow);
                try
                {
                    var writer = new PkoPacketWriter();
                    writer.WriteUint16(0);
                    writer.WriteUint32(targetSessionId);
                    writer.WriteUint16(6);
                    writer.WriteUint32(0);
                    writer.WriteUint32(targetPlayerId);
                    writer.WriteUint32(_packetID++);
                    writer.WriteByte(1); // move
                    writer.WriteUint16(8);
                    writer.WriteUint32((uint)pickItem.X);
                    writer.WriteUint32((uint)pickItem.Y);
                    writer.WriteUint16(6332);
                    lock (_lock) { _playerX = pickItem.X; _playerY = pickItem.Y; }
                    _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"Error sending move packet: {ex.Message}", ConsoleColor.Red);
                }
            }
            return;
        }

        // 2. Attack mobs
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
            ProxyLog.Write("Bot", $"ATTACKING mob '{attackMob.Name}' 0x{attackMob.WorldId:X}", ConsoleColor.Cyan);
            try
            {
                ProxyLog.Write(
                    "Bot", 
                    $"ATTACKING mob '{attackMob.Name}'" +
                    $" 0x{attackMob.WorldId:X} at {attackMob.X}, {attackMob.Y} from {_playerX}, {_playerY}",
                    ConsoleColor.Cyan);

                int targetX = attackMob.X != 0 ? attackMob.X : _playerX;
                int targetY = attackMob.Y != 0 ? attackMob.Y : _playerY;

                var writer = new PkoPacketWriter();
                writer.WriteUint16(0);
                writer.WriteUint32(targetSessionId);
                writer.WriteUint16(6);
                writer.WriteUint32(0);
                writer.WriteUint32(targetPlayerId);
                writer.WriteUint32(_packetID++);
                writer.WriteByte(2); // skill
                writer.WriteByte(2);
                writer.WriteByte(64);
                writer.WriteUint16(8);
                writer.WriteUint32((uint)targetX);
                writer.WriteUint32((uint)targetY);
                writer.WriteUint32(38); // basic attack
                writer.WriteUint32(attackMob.WorldId);
                writer.WriteUint32(attackMob.Handle);
                writer.WriteUint16(6332);
                _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
            }
            catch (Exception ex)
            {
                ProxyLog.Write("Bot", $"Error sending attack packet: {ex.Message}", ConsoleColor.Red);
            }
        }
    }
}