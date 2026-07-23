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
    public uint NextPacketCount { get; set; } = 1;
    public bool PacketCountInitialized { get; set; } = false;

    private readonly object _sendLock = new object();

    public async Task SendClientPacketAsync(byte[] decryptedPacket)
    {
        if (ServerStream == null) return;

        byte[] copy = (byte[])decryptedPacket.Clone();
        var pkt = new PkoPacket(copy);

        lock (_sendLock)
        {
            SessionId = pkt.Session;
            pkt.Size = (ushort)copy.Length;

            if (pkt.HasPacketCount)
            {
                if (!PacketCountInitialized)
                {
                    if (pkt.PacketCount > 0)
                    {
                        NextPacketCount = pkt.PacketCount;
                        PacketCountInitialized = true;
                    }
                }
                pkt.PacketCount = NextPacketCount++;
            }

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