namespace PkoProxyClient;

/// <summary>
/// Encapsulates decrypted packet context.
/// </summary>
public class ProxyPacketContext
{
    public int ConnectionId { get; }
    public string Direction { get; }
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