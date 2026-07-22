using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace PkoProxyClient
{
    public enum PkoActionType : byte
    {
        enumACTION_MOVE = 1,
        enumACTION_SKILL = 2,
        enumACTION_ITEM_PICK = 8,
        enumACTION_TOTAL_ITEM_PICK = 54
    }

    public class TrackedMob
    {
        public uint WorldId { get; set; }
        public uint Handle { get; set; }
        public string Name { get; set; } = "";
        public byte CtrlType { get; set; }
        public uint X { get; set; }
        public uint Y { get; set; }
        public DateTime LastSeen { get; set; } = DateTime.Now;
    }

    public class TrackedItem
    {
        public uint WorldId { get; set; }
        public uint Handle { get; set; }
        public uint ItemId { get; set; }
        public uint X { get; set; }
        public uint Y { get; set; }
        public DateTime LastSeen { get; set; } = DateTime.Now;
    }

    /// <summary>
    /// Custom proxy plugin designed to track mobs and items within see range,
    /// intercept chat commands via CMD_CM_SAY, and automate attack/pickup loops.
    /// Manages client-to-server packetCount (sequence number) state to remain
    /// aligned with the server's protection mechanism.
    /// </summary>
    public class BotPlugin : IProxyPlugin
    {
        public string Name => "BotPlugin";
        public bool Enabled { get; set; } = true;

        private readonly Dictionary<uint, TrackedMob> _mobs = new Dictionary<uint, TrackedMob>();
        private readonly Dictionary<uint, TrackedItem> _items = new Dictionary<uint, TrackedItem>();
        private readonly object _lock = new object();

        private Thread? _botThread;
        private Action<byte[]>? _sendToServer;
        private int _connId = -1;
        private uint _session = 0x80000000;
        private uint _currentPacketCount = 0;
        private uint _clientLastSeenPacketCount = 0;
        private uint _packetCountOffset = 0;

        public IReadOnlyDictionary<uint, TrackedMob> Mobs
        {
            get
            {
                lock (_lock)
                {
                    return new Dictionary<uint, TrackedMob>(_mobs);
                }
            }
        }

        public IReadOnlyDictionary<uint, TrackedItem> Items
        {
            get
            {
                lock (_lock)
                {
                    return new Dictionary<uint, TrackedItem>(_items);
                }
            }
        }

        public bool IsBotActive { get; private set; } = false;

        public uint CurrentPacketCount
        {
            get
            {
                lock (_lock)
                {
                    return _currentPacketCount;
                }
            }
            set
            {
                lock (_lock)
                {
                    _currentPacketCount = value;
                }
            }
        }

        public uint ClientLastSeenPacketCount
        {
            get
            {
                lock (_lock)
                {
                    return _clientLastSeenPacketCount;
                }
            }
            set
            {
                lock (_lock)
                {
                    _clientLastSeenPacketCount = value;
                }
            }
        }

        public uint PacketCountOffset
        {
            get
            {
                lock (_lock)
                {
                    return _packetCountOffset;
                }
            }
            set
            {
                lock (_lock)
                {
                    _packetCountOffset = value;
                }
            }
        }

        public void StartBot()
        {
            lock (_lock)
            {
                if (IsBotActive) return;
                IsBotActive = true;
                Console.WriteLine("[BotPlugin] Bot turned ON.");
                EnsureLoopRunning();
            }
        }

        public void StopBot()
        {
            lock (_lock)
            {
                if (!IsBotActive) return;
                IsBotActive = false;
                Console.WriteLine("[BotPlugin] Bot turned OFF.");
            }
        }

        public void SetSendCallback(int connId, Action<byte[]> sendCallback)
        {
            lock (_lock)
            {
                _connId = connId;
                _sendToServer = sendCallback;
                EnsureLoopRunning();
            }
        }

        private void EnsureLoopRunning()
        {
            if (_botThread == null || !_botThread.IsAlive)
            {
                _botThread = new Thread(BotLoop)
                {
                    IsBackground = true,
                    Name = "BotPluginLoop"
                };
                _botThread.Start();
            }
        }

        private void BotLoop()
        {
            Console.WriteLine("[BotPlugin] Background bot thread started.");
            while (true)
            {
                try
                {
                    bool active = false;
                    Action<byte[]>? send = null;
                    uint session = 0x80000000;

                    lock (_lock)
                    {
                        active = IsBotActive;
                        send = _sendToServer;
                        session = _session;
                    }

                    if (!active || send == null)
                    {
                        Thread.Sleep(500);
                        continue;
                    }

                    TickBot(send, session);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[BotPlugin] Exception in BotLoop: {ex.Message}");
                }

                Thread.Sleep(1000); // 1 second interval between bot actions
            }
        }

        public void TickBot(Action<byte[]> send, uint session)
        {
            // 1. Prioritize picking up nearby items
            TrackedItem? targetItem = null;
            lock (_lock)
            {
                foreach (var item in _items.Values)
                {
                    targetItem = item;
                    break;
                }
            }

            if (targetItem != null)
            {
                Console.WriteLine($"[BotPlugin] Picking up item {targetItem.ItemId} (WorldID: {targetItem.WorldId})");
                SendPickupPacket(send, session, targetItem.WorldId, targetItem.Handle);
                return;
            }

            // 2. Attack mobs if no items to pick up
            TrackedMob? targetMob = null;
            lock (_lock)
            {
                foreach (var mob in _mobs.Values)
                {
                    targetMob = mob;
                    break;
                }
            }

            if (targetMob != null)
            {
                Console.WriteLine($"[BotPlugin] Attacking mob '{targetMob.Name}' (WorldID: {targetMob.WorldId})");
                SendAttackPacket(send, session, targetMob.WorldId, targetMob.Handle, targetMob.X, targetMob.Y);
                return;
            }
        }

        private uint GetNextInjectedPacketCount()
        {
            lock (_lock)
            {
                _packetCountOffset++;
                _currentPacketCount = _clientLastSeenPacketCount + _packetCountOffset;
                return _currentPacketCount;
            }
        }

        private void SendPickupPacket(Action<byte[]> send, uint session, uint worldId, uint handle)
        {
            uint nextPktCount = GetNextInjectedPacketCount();

            var writer = new PkoPacketWriter();
            writer.WriteUint16(0); // Size placeholder
            writer.WriteUint32(session);
            writer.WriteUint16((ushort)PkoCommand.CMD_CM_BEGINACTION);

            // Write 4-byte sequence number
            writer.WriteUint32(nextPktCount);

            // Action Type (1 byte): enumACTION_ITEM_PICK = 8
            writer.WriteByte((byte)PkoActionType.enumACTION_ITEM_PICK);

            // Payload: WorldID (4 bytes), Handle (4 bytes)
            writer.WriteUint32(worldId);
            writer.WriteUint32(handle);

            byte[] data = writer.ToArray();
            ushort size = (ushort)data.Length;
            data[0] = (byte)(size >> 8);
            data[1] = (byte)(size & 0xFF);

            send(data);
        }

        private void SendAttackPacket(Action<byte[]> send, uint session, uint worldId, uint handle, uint mobX, uint mobY)
        {
            uint nextPktCount = GetNextInjectedPacketCount();

            var writer = new PkoPacketWriter();
            writer.WriteUint16(0); // Size placeholder
            writer.WriteUint32(session);
            writer.WriteUint16((ushort)PkoCommand.CMD_CM_BEGINACTION);

            // Write 4-byte sequence number
            writer.WriteUint32(nextPktCount);

            // Action Type (1 byte): enumACTION_SKILL = 2
            writer.WriteByte((byte)PkoActionType.enumACTION_SKILL);

            // chMove (1 byte): must be 2
            writer.WriteByte(2);

            // byFightID (1 byte): 1
            writer.WriteByte(1);

            // Path sequence (2 bytes length + data)
            writer.WriteUint16(8);
            writer.WriteUint32(mobX);
            writer.WriteUint32(mobY);

            // Skill ID (4 bytes): 1
            writer.WriteUint32(1);

            // Target World ID (4 bytes)
            writer.WriteUint32(worldId);

            // Target Handle (4 bytes)
            writer.WriteUint32(handle);

            byte[] data = writer.ToArray();
            ushort size = (ushort)data.Length;
            data[0] = (byte)(size >> 8);
            data[1] = (byte)(size & 0xFF);

            send(data);
        }

        public void OnPacket(ProxyPacketContext context)
        {
            if (!Enabled) return;

            if (context.Direction == "C -> S")
            {
                lock (_lock)
                {
                    _session = context.Session;
                }

                var reader = new PkoPacketReader(context.DecryptedPacket);
                try
                {
                    reader.ReadUint16(); // Skip size
                    reader.ReadUint32(); // Skip session
                    ushort packetId = reader.ReadUint16();

                    // Read/Capture the 4-byte protection sequence number if present in client stream
                    uint packetCount = reader.ReadUint32();
                    uint rewrittenCount = packetCount;

                    lock (_lock)
                    {
                        _clientLastSeenPacketCount = packetCount;
                        rewrittenCount = packetCount + _packetCountOffset;
                        _currentPacketCount = rewrittenCount;
                    }

                    // Dynamically rewrite client-to-server packetCount if offset has been introduced
                    if (rewrittenCount != packetCount)
                    {
                        context.DecryptedPacket[8] = (byte)(rewrittenCount >> 24);
                        context.DecryptedPacket[9] = (byte)(rewrittenCount >> 16);
                        context.DecryptedPacket[10] = (byte)(rewrittenCount >> 8);
                        context.DecryptedPacket[11] = (byte)(rewrittenCount & 0xFF);
                    }

                    if (packetId == (ushort)PkoCommand.CMD_CM_SAY)
                    {
                        string msg = reader.ReadString();
                        if (msg == "!bot start")
                        {
                            StartBot();
                        }
                        else if (msg == "!bot stop")
                        {
                            StopBot();
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[BotPlugin] General parsing error on C->S packet: {ex.Message}");
                }
            }
            else if (context.Direction == "S -> C")
            {
                var reader = new PkoPacketReader(context.DecryptedPacket);
                try
                {
                    reader.ReadUint16(); // Skip size
                    reader.ReadUint32(); // Skip session
                    ushort packetId = reader.ReadUint16();

                    if (packetId == (ushort)PkoCommand.CMD_MC_CHABEGINSEE)
                    {
                        try
                        {
                            byte seeType = reader.ReadByte();
                            uint chaId = reader.ReadUint32();
                            uint worldId = reader.ReadUint32();
                            uint commId = reader.ReadUint32();
                            string commName = reader.ReadString();
                            byte gmLevel = reader.ReadByte();
                            uint handle = reader.ReadUint32();
                            byte ctrlType = reader.ReadByte();
                            string name = reader.ReadString();
                            string motto = reader.ReadString();
                            ushort icon = reader.ReadUint16();
                            uint guildId = reader.ReadUint32();
                            string guildName = reader.ReadString();
                            string guildMotto = reader.ReadString();
                            string stallName = reader.ReadString();
                            ushort state = reader.ReadUint16();
                            uint x = reader.ReadUint32();
                            uint y = reader.ReadUint32();

                            var mob = new TrackedMob
                            {
                                WorldId = worldId,
                                Handle = handle,
                                Name = name,
                                CtrlType = ctrlType,
                                X = x,
                                Y = y,
                                LastSeen = DateTime.Now
                            };

                            lock (_lock)
                            {
                                _mobs[worldId] = mob;
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"[BotPlugin] Error parsing CMD_MC_CHABEGINSEE: {ex.Message}");
                        }
                    }
                    else if (packetId == (ushort)PkoCommand.CMD_MC_CHAENDSEE)
                    {
                        try
                        {
                            byte seeType = reader.ReadByte();
                            uint worldId = reader.ReadUint32();

                            lock (_lock)
                            {
                                _mobs.Remove(worldId);
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"[BotPlugin] Error parsing CMD_MC_CHAENDSEE: {ex.Message}");
                        }
                    }
                    else if (packetId == (ushort)PkoCommand.CMD_MC_ITEMBEGINSEE)
                    {
                        try
                        {
                            uint worldId = reader.ReadUint32();
                            uint handle = reader.ReadUint32();
                            uint itemId = reader.ReadUint32();
                            uint x = reader.ReadUint32();
                            uint y = reader.ReadUint32();
                            ushort angle = reader.ReadUint16();
                            ushort num = reader.ReadUint16();
                            byte appeType = reader.ReadByte();
                            uint fromId = reader.ReadUint32();

                            var item = new TrackedItem
                            {
                                WorldId = worldId,
                                Handle = handle,
                                ItemId = itemId,
                                X = x,
                                Y = y,
                                LastSeen = DateTime.Now
                            };

                            lock (_lock)
                            {
                                _items[worldId] = item;
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"[BotPlugin] Error parsing CMD_MC_ITEMBEGINSEE: {ex.Message}");
                        }
                    }
                    else if (packetId == (ushort)PkoCommand.CMD_MC_ITEMENDSEE)
                    {
                        try
                        {
                            uint worldId = reader.ReadUint32();

                            lock (_lock)
                            {
                                _items.Remove(worldId);
                            }
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"[BotPlugin] Error parsing CMD_MC_ITEMENDSEE: {ex.Message}");
                        }
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[BotPlugin] General parsing error on S->C packet: {ex.Message}");
                }
            }
        }
    }
}
