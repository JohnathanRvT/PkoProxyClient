using System.Text;

namespace PkoProxyClient;
// ===================================================================
// Logging system – enable/disable categories globally
// ===================================================================
public static class ProxyLog
{
    // Enable/disable entire logging
    public static bool Enabled { get; set; } = true;

    // Category switches
    public static bool LogGeneral { get; set; } = true;
    public static bool LogPacket { get; set; } = true;          // packet direction, ID, size
    public static bool LogHandshake { get; set; } = true;       // CHAP, login, encryption init
    public static bool LogPlugin { get; set; } = true;          // plugin OnPacket calls
    public static bool LogBot { get; set; } = true;             // AutoBot plugin internal
    public static bool LogRawHex { get; set; } = false;         // full hex dump of every packet (off by default)
    public static bool LogDebug { get; set; } = true;          // extra verbose debug

    private static readonly object _lock = new object();

    public static void Write(string category, string message, ConsoleColor color = ConsoleColor.White)
    {
        if (!Enabled) return;
        bool categoryEnabled = category switch
        {
            "General" => LogGeneral,
            "Packet"  => LogPacket,
            "Handshake" => LogHandshake,
            "Plugin"  => LogPlugin,
            "Bot"     => LogBot,
            "Raw"     => LogRawHex,
            "Debug"   => LogDebug,
            _ => true
        };
        if (!categoryEnabled) return;

        lock (_lock)
        {
            var prev = Console.ForegroundColor;
            Console.ForegroundColor = color;
            Console.WriteLine($"[{DateTime.Now:HH:mm:ss.fff}] [{category}] {message}");
            Console.ForegroundColor = prev;
        }
    }

    public static void HexDump(string category, byte[] data, string prefix = "")
    {
        if (!Enabled || !LogRawHex) return;
        if (data == null || data.Length == 0) return;

        var sb = new StringBuilder();
        if (!string.IsNullOrEmpty(prefix)) sb.Append(prefix).Append("\n");
        for (int i = 0; i < data.Length; i += 16)
        {
            int len = Math.Min(16, data.Length - i);
            sb.Append($"{i:X4}: ");
            for (int j = 0; j < 16; j++)
            {
                if (j < len) sb.Append($"{data[i + j]:X2} ");
                else sb.Append("   ");
            }
            sb.Append("  ");
            for (int j = 0; j < len; j++)
            {
                char c = (char)data[i + j];
                sb.Append(c >= 32 && c <= 126 ? c : '.');
            }
            sb.AppendLine();
        }
        Write(category, sb.ToString().TrimEnd(), ConsoleColor.DarkGray);
    }
}