namespace PkoProxyClient;

/// <summary>
/// Plugin interface.
/// </summary>
public interface IProxyPlugin
{
    string Name { get; }
    bool Enabled { get; set; }
    void OnPacket(ProxyPacketContext context);
}