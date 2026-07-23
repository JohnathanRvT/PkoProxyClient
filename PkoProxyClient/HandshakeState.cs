namespace PkoProxyClient;

/// <summary>
/// Tracks intermediate handshake parameters intercepted by the proxy.
/// </summary>
public class HandshakeState
{
    public string ChapString { get; set; } = "";
    public string Login { get; set; } = "";
    public byte[] PasswordBytes { get; set; } = Array.Empty<byte>();
    public ushort Version { get; set; } = 0;
    public string PlaintextPassword { get; set; } = "";
}