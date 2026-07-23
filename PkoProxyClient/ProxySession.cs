using System.Net.Sockets;

namespace PkoProxyClient;

/// <summary>
/// Holds session-specific TCP stream and encryptor states.
/// </summary>
public class ProxySession
{
    public int ConnectionId { get; set; }
    public NetworkStream? ServerStream { get; set; }
    public PacketEncryptor? Encryptor { get; set; }
    public uint SessionId { get; set; }
    public uint PlayerWorldId { get; set; }
    public uint NextMainPacketCount { get; set; } = 0;
    public bool MainPacketCountInitialized { get; set; } = false;

    public uint NextSecondaryPacketCount { get; set; } = 1;
    public bool SecondaryPacketCountInitialized { get; set; } = false;

    public uint NextPacketCount
    {
        get => NextMainPacketCount;
        set => NextMainPacketCount = value;
    }
    public bool PacketCountInitialized
    {
        get => MainPacketCountInitialized;
        set => MainPacketCountInitialized = value;
    }

    private readonly object _sendLock = new object();

    public async Task SendClientPacketAsync(byte[] decryptedPacket, bool isInjected = false)
    {
        if (ServerStream == null) return;

        byte[] copy = (byte[])decryptedPacket.Clone();
        var pkt = new PkoPacket(copy);

        lock (_sendLock)
        {
            SessionId = pkt.Session;
            pkt.Size = (ushort)copy.Length;

            if (pkt.HasMainPacketCount)
            {
                if (!MainPacketCountInitialized)
                {
                    if (pkt.MainPacketCount > 0)
                    {
                        NextMainPacketCount = pkt.MainPacketCount;
                        MainPacketCountInitialized = true;
                    }
                }
                pkt.MainPacketCount = NextMainPacketCount++;
            }

            if (pkt.HasSecondaryPacketCount)
            {
                if (!SecondaryPacketCountInitialized)
                {
                    if (pkt.SecondaryPacketCount > 0)
                    {
                        NextSecondaryPacketCount = pkt.SecondaryPacketCount;
                        SecondaryPacketCountInitialized = true;
                    }
                }
                pkt.SecondaryPacketCount = NextSecondaryPacketCount++;
            }
        }

        // Run plugins on injected packets so they appear in logs and hex dumps
        if (isInjected)
        {
            var context = new ProxyPacketContext(ConnectionId, "C -> S", pkt) { IsInjected = true };
            foreach (var plugin in PkoProxy.Plugins)
            {
                try
                {
                    plugin.OnPacket(context);
                }
                catch (Exception ex)
                {
                    ProxyLog.Write("Plugin", $"Plugin {plugin.Name} threw on injected packet: {ex.Message}", ConsoleColor.Red);
                }
            }

            if (ProxyLog.LogRawHex)
                ProxyLog.HexDump("Raw", context.DecryptedPacket, $"C -> S Injected {pkt.Command}");

            copy = context.DecryptedPacket;
            // update size in case plugins modified the injected packet
            copy[0] = (byte)(copy.Length >> 8);
            copy[1] = (byte)(copy.Length & 0xFF);
        }

        lock (_sendLock)
        {
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