namespace PkoProxyClient;

public class ConsoleColorPacketLoggerPlugin : IProxyPlugin
{
    public string Name => "ConsoleColorPacketLogger";
    public bool Enabled { get; set; } = true;

    public void OnPacket(ProxyPacketContext context)
    {
        if (!Enabled || !ProxyLog.LogPacket) return;

        ushort size = (ushort)((context.DecryptedPacket[0] << 8) | context.DecryptedPacket[1]);
        string cmdName = PkoCommandTranslator.GetCommandName(context.PacketId);

        ConsoleColor color = context.Direction == "C -> S" ? ConsoleColor.Cyan : ConsoleColor.Yellow;
        if (context.PacketId == 940 || context.PacketId == 931 || context.PacketId == 431)
            color = ConsoleColor.Magenta;

        ProxyLog.Write("Packet", $"[Conn #{context.ConnectionId}] {context.Direction} | {cmdName} ({context.PacketId}) | Size: {size,4} | Session: 0x{context.Session:X8}", color);
    }
}