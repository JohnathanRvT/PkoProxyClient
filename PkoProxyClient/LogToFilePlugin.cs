using System.Text;

namespace PkoProxyClient;

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

        var sb = new StringBuilder();
        sb.AppendLine($"=========================================================================");
        sb.AppendLine($"[Connection #{context.ConnectionId}] {context.Direction} | Packet ID: {cmdName} ({context.PacketId}) | Size: {size} | Session: 0x{context.Session:X8}");
        sb.AppendLine($"-------------------------------------------------------------------------");
        sb.AppendLine(PkoProxy.HexDump(payload));
        sb.AppendLine();

        string text = sb.ToString();
        try { File.AppendAllText(_logFilePath, text); } catch { }

        try
        {
            byte dirVal = context.Direction == "C -> S" ? (byte)0 : (byte)1;
            using (var fs = new FileStream(_binFilePath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite))
            {
                fs.WriteByte(dirVal);
                fs.Write(context.DecryptedPacket, 0, context.DecryptedPacket.Length);
            }
        }
        catch { }
    }
}