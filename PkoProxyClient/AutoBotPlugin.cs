namespace PkoProxyClient;

public class AutoBotPlugin : IProxyPlugin
{
    public string Name => "AutoBot";
    public bool Enabled { get; set; } = false;

    public enum BotState
    {
        Idle,
        MovingToMob,
        Attacking,
        Looting,
        Patrolling
    }

    private BotState _currentState = BotState.Idle;

    private readonly Dictionary<uint, MobInfo> _mobs = new Dictionary<uint, MobInfo>();
    private readonly Dictionary<uint, ItemInfo> _items = new Dictionary<uint, ItemInfo>();
    private uint _playerWorldId = 0;
    private int _playerX = 0;
    private int _playerY = 0;
    private int _lastConnectionId = 0;
    private readonly object _lock = new object();
    private readonly Timer _loopTimer;

    private int _homeX = 0;
    private int _homeY = 0;
    private int _patrolIndex = 0;
    private readonly HashSet<uint> _itemBlacklist = new HashSet<uint>();
    private readonly HashSet<string> _mobBlacklist = new HashSet<string>();

    public AutoBotPlugin()
    {
        _loopTimer = new Timer(OnBotLoop, null, 1500, 1500);
        ProxyLog.Write("Bot", "AutoBot plugin initialized, timer set for 1500ms intervals", ConsoleColor.DarkCyan);
    }

    private void SendEndActionPacket(uint sessionId)
    {
        try
        {
            var writer = new PkoPacketWriter();
            writer.WriteUint16(0);                     // Size placeholder
            writer.WriteUint32(sessionId);             // Session ID
            writer.WriteUint16(7);                     // CMD_CM_ENDACTION
            writer.WriteUint32(0);                     // Main Packet Count placeholder
            _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
            if (ProxyLog.LogDebug)
                ProxyLog.Write("Bot", "Sent CMD_CM_ENDACTION (7)", ConsoleColor.DarkYellow);
        }
        catch (Exception ex)
        {
            ProxyLog.Write("Bot", $"Error sending ENDACTION packet: {ex.Message}", ConsoleColor.Red);
        }
    }

    public void OnPacket(ProxyPacketContext context)
    {
        if (context.IsInjected) return;
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
                uint charWorldId = pktReader.ReadUint32(); // Player World ID (bytes 8-11)
                _ = pktReader.ReadUint32(); // Main Packet Count (bytes 12-15)
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
                        int lastX = (int)pktReader.ReadUint32LE();
                        int lastY = (int)pktReader.ReadUint32LE();
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
                    lock (_lock)
                    {
                        _homeX = _playerX;
                        _homeY = _playerY;
                        _patrolIndex = 0;
                    }
                    ProxyLog.Write("Bot", $"Bot Loop STARTED! Home position set to ({_homeX},{_homeY})", ConsoleColor.Green);
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_stop"))
                {
                    Enabled = false;
                    ProxyLog.Write("Bot", "Bot Loop STOPPED!", ConsoleColor.Red);
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_blacklist_item"))
                {
                    var parts = chatMsg.Split(' ', 2);
                    if (parts.Length > 1 && uint.TryParse(parts[1].Trim(), out uint itemId))
                    {
                        lock (_lock) { _itemBlacklist.Add(itemId); }
                        ProxyLog.Write("Bot", $"Item ID {itemId} added to blacklist", ConsoleColor.Yellow);
                    }
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_whitelist_item"))
                {
                    var parts = chatMsg.Split(' ', 2);
                    if (parts.Length > 1 && uint.TryParse(parts[1].Trim(), out uint itemId))
                    {
                        lock (_lock) { _itemBlacklist.Remove(itemId); }
                        ProxyLog.Write("Bot", $"Item ID {itemId} removed from blacklist (whitelisted)", ConsoleColor.Green);
                    }
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_blacklist_mob"))
                {
                    var parts = chatMsg.Split(' ', 2);
                    if (parts.Length > 1)
                    {
                        string mobName = parts[1].Trim();
                        lock (_lock) { _mobBlacklist.Add(mobName); }
                        ProxyLog.Write("Bot", $"Mob name '{mobName}' added to blacklist", ConsoleColor.Yellow);
                    }
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_whitelist_mob"))
                {
                    var parts = chatMsg.Split(' ', 2);
                    if (parts.Length > 1)
                    {
                        string mobName = parts[1].Trim();
                        lock (_lock) { _mobBlacklist.Remove(mobName); }
                        ProxyLog.Write("Bot", $"Mob name '{mobName}' removed from blacklist (whitelisted)", ConsoleColor.Green);
                    }
                    context.IsDropped = true;
                }
                else if (chatMsg.StartsWith("/bot_status"))
                {
                    lock (_lock)
                    {
                        ProxyLog.Write("Bot", $"Status: Enabled={Enabled} | State={_currentState} | Mobs={_mobs.Count} | Items={_items.Count} | Player=0x{_playerWorldId:X} | Coords=({_playerX},{_playerY}) | Home=({_homeX},{_homeY})", ConsoleColor.Cyan);
                        ProxyLog.Write("Bot", $"Blacklisted Items: {string.Join(", ", _itemBlacklist)}", ConsoleColor.DarkYellow);
                        ProxyLog.Write("Bot", $"Blacklisted Mobs: {string.Join(", ", _mobBlacklist)}", ConsoleColor.DarkYellow);
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
            // CMD_MC_ENTERMAP (516) - Map enter, player character base info
            if (context.PacketId == 516)
            {
                try
                {
                    var pktReader = new PkoPacketReader(context.DecryptedPacket);
                    _ = pktReader.ReadUint16(); // size
                    _ = pktReader.ReadUint32(); // session
                    _ = pktReader.ReadUint16(); // 516

                    ushort sEnterRet = pktReader.ReadUint16();
                    if (sEnterRet == 0) // ERR_SUCCESS is 0
                    {
                        _ = pktReader.ReadByte();   // bAutoLock
                        _ = pktReader.ReadByte();   // bKitbagLock
                        _ = pktReader.ReadByte();   // chEnterType
                        _ = pktReader.ReadByte();   // bIsNewCha
                        _ = pktReader.ReadString(); // szMapName
                        _ = pktReader.ReadByte();   // bCanTeam

                        // Start of ReadChaBasePacket payload:
                        _ = pktReader.ReadUint32(); // ulChaID
                        uint ulWorldID = pktReader.ReadUint32(); // Player World ID!

                        _ = pktReader.ReadUint32(); // ulCommID
                        _ = pktReader.ReadString(); // szCommName
                        _ = pktReader.ReadByte();   // chGMLv
                        _ = pktReader.ReadUint32(); // lHandle
                        _ = pktReader.ReadByte();   // chCtrlType
                        _ = pktReader.ReadString(); // szName
                        _ = pktReader.ReadString(); // strMottoName
                        _ = pktReader.ReadUint16(); // sIcon
                        _ = pktReader.ReadUint32(); // lGuildID
                        _ = pktReader.ReadString(); // strGuildName
                        _ = pktReader.ReadString(); // strGuildMotto
                        _ = pktReader.ReadString(); // strStallName
                        _ = pktReader.ReadUint16(); // sState
                        int x = (int)pktReader.ReadUint32(); // Player X (Big-Endian)
                        int y = (int)pktReader.ReadUint32(); // Player Y (Big-Endian)

                        lock (_lock)
                        {
                            _playerWorldId = ulWorldID;
                            _playerX = x;
                            _playerY = y;
                        }

                        ProxyLog.Write("Bot", $"[MapEnter] Player World ID captured: 0x{ulWorldID:X}, Coords set to ({x},{y})", ConsoleColor.Green);
                    }
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"ERROR parsing CMD_MC_ENTERMAP: {ex.Message}", ConsoleColor.Red);
                }
            }

            // CMD_MC_NOTIACTION (508) - Position sync & movement updates
            if (context.PacketId == 508)
            {
                try
                {
                    var reader = new PkoPacketReader(context.DecryptedPacket);
                    _ = reader.ReadUint16(); // size
                    _ = reader.ReadUint32(); // session
                    _ = reader.ReadUint16(); // 508
                    uint entityId = reader.ReadUint32();
                    _ = reader.ReadUint32(); // packetId
                    byte actionType = reader.ReadByte();

                    if (actionType == 1) // Move action
                    {
                        ushort sState = reader.ReadUint16();
                        if (sState != 0) // enumMSTATE_ON = 0. If stopped, stop-state is written
                        {
                            _ = reader.ReadUint16(); // sStopState
                        }

                        ushort pathNumBytes = reader.ReadUint16();
                        if (pathNumBytes >= 8)
                        {
                            // Skip to the last point of the path
                            reader.ReadBytes(pathNumBytes - 8);
                            int finalX = (int)reader.ReadUint32LE();
                            int finalY = (int)reader.ReadUint32LE();

                            lock (_lock)
                            {
                                if (entityId == _playerWorldId && _playerWorldId != 0)
                                {
                                    _playerX = finalX;
                                    _playerY = finalY;
                                    if (ProxyLog.LogDebug)
                                        ProxyLog.Write("Bot", $"[Sync] Player coordinate synced via 508 to ({_playerX},{_playerY})", ConsoleColor.DarkGray);
                                }
                                else if (_mobs.TryGetValue(entityId, out var mob))
                                {
                                    mob.X = finalX;
                                    mob.Y = finalY;
                                    if (ProxyLog.LogDebug)
                                        ProxyLog.Write("Bot", $"[Sync] Mob 0x{entityId:X} coordinate synced via 508 to ({mob.X},{mob.Y})", ConsoleColor.DarkGray);
                                }
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Bot", $"ERROR parsing CMD_MC_NOTIACTION: {ex.Message}", ConsoleColor.Red);
                }
            }

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

        // 1. Pick up items (with Mass Looting)
        int currentX = 0, currentY = 0;
        lock (_lock) { currentX = _playerX; currentY = _playerY; }

        List<ItemInfo> closeItems = new List<ItemInfo>();
        ItemInfo? closestFarItem = null;
        double minFarDist = double.MaxValue;

        lock (_lock)
        {
            foreach (var item in _items.Values)
            {
                if (_itemBlacklist.Contains(item.ItemId))
                    continue;

                double dist = Math.Sqrt(Math.Pow(item.X - currentX, 2) + Math.Pow(item.Y - currentY, 2));
                if (dist <= 350)
                {
                    closeItems.Add(item);
                }
                else
                {
                    if (dist < minFarDist)
                    {
                        minFarDist = dist;
                        closestFarItem = item;
                    }
                }
            }
        }

        if (closeItems.Count > 0)
        {
            if (_currentState != BotState.Looting)
            {
                SendEndActionPacket(targetSessionId);
                _currentState = BotState.Looting;
            }

            // Cap mass loot at 48 items
            int countToLoot = Math.Min(closeItems.Count, 48);
            var itemsToLoot = closeItems.GetRange(0, countToLoot);

            ProxyLog.Write("Bot", $"LOOTING {itemsToLoot.Count} items via mass pick", ConsoleColor.Green);
            try
            {
                var writer = new PkoPacketWriter();
                writer.WriteUint16(0);                     // Size placeholder
                writer.WriteUint32(targetSessionId);       // Session
                writer.WriteUint16(6);                     // CMD_CM_BEGINACTION
                writer.WriteUint32(targetPlayerId);        // Bytes 8-11: Player ID
                writer.WriteUint32(0);                     // Bytes 12-15: Main Packet Count placeholder
                writer.WriteByte(54);                      // Byte 16: Action Type (54 = TotalPick)
                writer.WriteUint16((ushort)itemsToLoot.Count);

                foreach (var item in itemsToLoot)
                {
                    writer.WriteUint32LE(item.WorldId);
                    writer.WriteUint32LE(item.Handle);
                }

                _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
            }
            catch (Exception ex)
            {
                ProxyLog.Write("Bot", $"Error sending mass loot packet: {ex.Message}", ConsoleColor.Red);
            }
            return;
        }
        else if (closestFarItem != null)
        {
            ProxyLog.Write("Bot", $"Item too far (dist={minFarDist:F1}), moving to ({closestFarItem.X},{closestFarItem.Y})", ConsoleColor.Yellow);
            if (_currentState != BotState.Looting)
            {
                SendEndActionPacket(targetSessionId);
                _currentState = BotState.Looting;
            }

            try
            {
                var writer = new PkoPacketWriter();
                writer.WriteUint16(0);                     // Size placeholder
                writer.WriteUint32(targetSessionId);       // Session
                writer.WriteUint16(6);                     // CMD_CM_BEGINACTION
                writer.WriteUint32(targetPlayerId);        // Bytes 8-11: Player ID
                writer.WriteUint32(0);                     // Bytes 12-15: Main Packet Count placeholder
                writer.WriteByte(1);                       // Byte 16: Move action type
                writer.WriteUint16(8);                     // Path bytes (1 point * 8 bytes)
                writer.WriteUint32LE((uint)closestFarItem.X);
                writer.WriteUint32LE((uint)closestFarItem.Y);

                lock (_lock) { _playerX = closestFarItem.X; _playerY = closestFarItem.Y; }
                _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
            }
            catch (Exception ex)
            {
                ProxyLog.Write("Bot", $"Error sending move packet: {ex.Message}", ConsoleColor.Red);
            }
            return;
        }

        // 2. Attack mobs (Euclidean Closet Mob Targeting)
        MobInfo? attackMob = null;
        double minMobDist = double.MaxValue;
        lock (_lock)
        {
            foreach (var mob in _mobs.Values)
            {
                if (mob.WorldId == targetPlayerId) continue;

                // Check mob blacklist
                bool isBlacklisted = false;
                foreach (var blackMob in _mobBlacklist)
                {
                    if (mob.Name.Contains(blackMob, StringComparison.OrdinalIgnoreCase))
                    {
                        isBlacklisted = true;
                        break;
                    }
                }
                if (isBlacklisted)
                    continue;

                double dist = Math.Sqrt(Math.Pow(mob.X - currentX, 2) + Math.Pow(mob.Y - currentY, 2));
                if (dist < minMobDist)
                {
                    minMobDist = dist;
                    attackMob = mob;
                }
            }
        }

        if (attackMob != null)
        {
            if (_currentState != BotState.Attacking)
            {
                SendEndActionPacket(targetSessionId);
                _currentState = BotState.Attacking;
            }

            ProxyLog.Write("Bot", $"ATTACKING mob '{attackMob.Name}' 0x{attackMob.WorldId:X} at ({attackMob.X},{attackMob.Y}) from ({currentX},{currentY})", ConsoleColor.Cyan);
            try
            {
                int targetX = attackMob.X != 0 ? attackMob.X : currentX;
                int targetY = attackMob.Y != 0 ? attackMob.Y : currentY;

                var writer = new PkoPacketWriter();
                writer.WriteUint16(0);                     // Size placeholder (Bytes 0-1)
                writer.WriteUint32(targetSessionId);       // Session (Bytes 2-5)
                writer.WriteUint16(6);                     // Command ID = 6 (Bytes 6-7)
                writer.WriteUint32(targetPlayerId);        // Player World ID (Bytes 8-11)
                writer.WriteUint32(0);                     // Bytes 12-15: Main Packet Count placeholder
                writer.WriteByte(2);                       // Action Type = 2 (enumACTION_SKILL) (Byte 16)
                writer.WriteByte(2);                       // chMove = 2 (Byte 17)
                writer.WriteByte(64);                      // byFightID = 64 (Byte 18)
                writer.WriteUint16(8);                     // Path length in bytes = 8 (Bytes 19-20)
                writer.WriteUint32LE((uint)targetX);       // Target X (Bytes 21-24)
                writer.WriteUint32LE((uint)targetY);       // Target Y (Bytes 25-28)
                writer.WriteUint32LE(1);                   // ulSkillID = 1 (Bytes 29-32)
                writer.WriteUint32LE(attackMob.WorldId);   // Target World ID (Bytes 33-36)
                writer.WriteUint32LE(attackMob.Handle);    // Target Handle (Bytes 37-40)

                _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
            }
            catch (Exception ex)
            {
                ProxyLog.Write("Bot", $"Error sending attack packet: {ex.Message}", ConsoleColor.Red);
            }
        }
        else
        {
            // 3. Patrolling Route falling back
            if (_homeX == 0 && _homeY == 0)
            {
                lock (_lock)
                {
                    _homeX = currentX;
                    _homeY = currentY;
                }
            }

            if (_currentState != BotState.Patrolling)
            {
                SendEndActionPacket(targetSessionId);
                _currentState = BotState.Patrolling;
            }

            int wx = _homeX;
            int wy = _homeY;

            lock (_lock)
            {
                if (_patrolIndex == 0) { wx = _homeX + 500; wy = _homeY; }
                else if (_patrolIndex == 1) { wx = _homeX + 500; wy = _homeY + 500; }
                else if (_patrolIndex == 2) { wx = _homeX; wy = _homeY + 500; }
                else { wx = _homeX; wy = _homeY; }
            }

            double dist = Math.Sqrt(Math.Pow(wx - currentX, 2) + Math.Pow(wy - currentY, 2));
            if (dist <= 100)
            {
                lock (_lock)
                {
                    _patrolIndex = (_patrolIndex + 1) % 4;
                    if (_patrolIndex == 0) { wx = _homeX + 500; wy = _homeY; }
                    else if (_patrolIndex == 1) { wx = _homeX + 500; wy = _homeY + 500; }
                    else if (_patrolIndex == 2) { wx = _homeX; wy = _homeY + 500; }
                    else { wx = _homeX; wy = _homeY; }
                }
            }

            ProxyLog.Write("Bot", $"Patrolling to waypoint {_patrolIndex} ({wx},{wy}) from ({currentX},{currentY}) (dist={dist:F1})", ConsoleColor.Blue);
            try
            {
                var writer = new PkoPacketWriter();
                writer.WriteUint16(0);                     // Size placeholder
                writer.WriteUint32(targetSessionId);       // Session
                writer.WriteUint16(6);                     // CMD_CM_BEGINACTION
                writer.WriteUint32(targetPlayerId);        // Bytes 8-11: Player ID
                writer.WriteUint32(0);                     // Bytes 12-15: Main Packet Count placeholder
                writer.WriteByte(1);                       // Byte 16: Move action type
                writer.WriteUint16(8);                     // Path bytes (1 point * 8 bytes)
                writer.WriteUint32LE((uint)wx);
                writer.WriteUint32LE((uint)wy);

                lock (_lock) { _playerX = wx; _playerY = wy; }
                _ = PkoProxy.InjectClientPacketAsync(_lastConnectionId, writer.ToArray());
            }
            catch (Exception ex)
            {
                ProxyLog.Write("Bot", $"Error sending patrol move packet: {ex.Message}", ConsoleColor.Red);
            }
        }
    }
}