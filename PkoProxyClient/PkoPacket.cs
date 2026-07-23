using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace PkoProxyClient
{
    /// <summary>
    /// Represents a high-level PKO packet with easy accessors for header fields like
    /// Size, Session, Command (opcode), and optional PacketCount.
    /// </summary>
    public class PkoPacket
    {
        public byte[] RawBytes { get; set; }

        public ushort Size
        {
            get => (ushort)((RawBytes[0] << 8) | RawBytes[1]);
            set
            {
                RawBytes[0] = (byte)(value >> 8);
                RawBytes[1] = (byte)(value & 0xFF);
            }
        }

        public uint Session
        {
            get => (uint)((RawBytes[2] << 24) | (RawBytes[3] << 16) | (RawBytes[4] << 8) | RawBytes[5]);
            set
            {
                RawBytes[2] = (byte)(value >> 24);
                RawBytes[3] = (byte)(value >> 16);
                RawBytes[4] = (byte)(value >> 8);
                RawBytes[5] = (byte)(value & 0xFF);
            }
        }

        public ushort Command
        {
            get => (ushort)((RawBytes[6] << 8) | RawBytes[7]);
            set
            {
                RawBytes[6] = (byte)(value >> 8);
                RawBytes[7] = (byte)(value & 0xFF);
            }
        }

        public bool HasPacketCount => RawBytes.Length >= 12;

        public uint PacketCount
        {
            get
            {
                if (RawBytes.Length < 12) return 0;
                return (uint)((RawBytes[8] << 24) | (RawBytes[9] << 16) | (RawBytes[10] << 8) | RawBytes[11]);
            }
            set
            {
                if (RawBytes.Length >= 12)
                {
                    RawBytes[8] = (byte)(value >> 24);
                    RawBytes[9] = (byte)(value >> 16);
                    RawBytes[10] = (byte)(value >> 8);
                    RawBytes[11] = (byte)(value & 0xFF);
                }
            }
        }

        public PkoPacket(byte[] bytes)
        {
            RawBytes = bytes ?? throw new ArgumentNullException(nameof(bytes));
        }
    }

    /// <summary>
    /// Decodes structured bytes from a PKO packet payload.
    /// PKO payloads are typically structured sequentially as:
    /// - 2-byte Length (Big-Endian)
    /// - 4-byte Session ID / Sequence ID
    /// - 2-byte Packet ID (Opcode)
    /// - Sequential types (bytes, strings with ushort length prefix and null terminator, integers, etc.)
    /// </summary>
    public class PkoPacketReader
    {
        private readonly byte[] _data;
        private int _pos;

        /// <summary>
        /// Initializes a new instance of <see cref="PkoPacketReader"/> with the specified byte buffer.
        /// </summary>
        /// <param name="data">The byte array representing the packet content.</param>
        public PkoPacketReader(byte[] data)
        {
            _data = data ?? throw new ArgumentNullException(nameof(data));
            _pos = 0;
        }

        /// <summary>
        /// Reads a single byte from the current position and advances the position by 1.
        /// </summary>
        public byte ReadByte()
        {
            if (_pos >= _data.Length)
                throw new EndOfStreamException("Attempted to read past the end of the packet.");
            return _data[_pos++];
        }

        /// <summary>
        /// Reads a 2-byte unsigned integer (Big-Endian) and advances the position by 2.
        /// </summary>
        public ushort ReadUint16()
        {
            if (_pos + 2 > _data.Length)
                throw new EndOfStreamException("Attempted to read past the end of the packet.");
            ushort val = (ushort)((_data[_pos] << 8) | _data[_pos + 1]);
            _pos += 2;
            return val;
        }

        /// <summary>
        /// Reads a 4-byte unsigned integer (Big-Endian) and advances the position by 4.
        /// </summary>
        public uint ReadUint32()
        {
            if (_pos + 4 > _data.Length)
                throw new EndOfStreamException("Attempted to read past the end of the packet.");
            uint val = (uint)((_data[_pos] << 24) | (_data[_pos + 1] << 16) | (_data[_pos + 2] << 8) | _data[_pos + 3]);
            _pos += 4;
            return val;
        }

        /// <summary>
        /// Reads an ASCII string prefixed by its 2-byte length (which includes the null terminator)
        /// and advances the position accordingly.
        /// </summary>
        public string ReadString()
        {
            ushort len = ReadUint16();
            if (len <= 1)
                return "";

            if (_pos + len - 1 > _data.Length)
                throw new EndOfStreamException("Attempted to read past the end of the packet.");

            string s = Encoding.ASCII.GetString(_data, _pos, len - 1);
            _pos += len;
            return s;
        }

        /// <summary>
        /// Reads a specific number of bytes from the current position and advances the position.
        /// </summary>
        public byte[] ReadBytes(int count)
        {
            if (count < 0)
                throw new ArgumentException("Count cannot be negative.", nameof(count));
            if (_pos + count > _data.Length)
                throw new EndOfStreamException("Attempted to read past the end of the packet.");

            byte[] buf = new byte[count];
            Array.Copy(_data, _pos, buf, 0, count);
            _pos += count;
            return buf;
        }

        /// <summary>
        /// Shifts the current read position relative to the end of the data buffer.
        /// </summary>
        public void SeekFromEnd(int offset)
        {
            if (offset < 0 || offset > _data.Length)
                throw new ArgumentOutOfRangeException(nameof(offset), "Offset is out of bounds.");
            _pos = _data.Length - offset;
        }

        /// <summary>
        /// Gets the number of bytes remaining to be read in the buffer.
        /// </summary>
        public int Remaining => _data.Length - _pos;
    }

    /// <summary>
    /// Formats and builds structured binary payloads for sending to PKO server/client.
    /// Handles length prefixes, string framing, null-termination, and Big-Endian integer writing.
    /// </summary>
    public class PkoPacketWriter
    {
        private readonly List<byte> _data = new List<byte>();

        /// <summary>
        /// Writes a single byte into the buffer.
        /// </summary>
        public void WriteByte(byte b)
        {
            _data.Add(b);
        }

        /// <summary>
        /// Writes a 2-byte unsigned integer (Big-Endian) into the buffer.
        /// </summary>
        public void WriteUint16(ushort val)
        {
            _data.Add((byte)(val >> 8));
            _data.Add((byte)(val & 0xFF));
        }

        /// <summary>
        /// Writes a 4-byte unsigned integer (Big-Endian) into the buffer.
        /// </summary>
        public void WriteUint32(uint val)
        {
            _data.Add((byte)(val >> 24));
            _data.Add((byte)(val >> 16));
            _data.Add((byte)(val >> 8));
            _data.Add((byte)(val & 0xFF));
        }

        /// <summary>
        /// Writes an ASCII string prefixed by its 2-byte length (which includes the null-terminator)
        /// and appends the null terminator.
        /// </summary>
        public void WriteString(string s)
        {
            s ??= "";
            byte[] bytes = Encoding.ASCII.GetBytes(s);
            ushort len = (ushort)(bytes.Length + 1);
            WriteUint16(len);
            _data.AddRange(bytes);
            _data.Add(0); // null terminator
        }

        /// <summary>
        /// Writes an arbitrary array of bytes into the buffer.
        /// </summary>
        public void WriteBytes(byte[] buf)
        {
            if (buf != null)
            {
                _data.AddRange(buf);
            }
        }

        /// <summary>
        /// Serializes the compiled structured payload to a byte array.
        /// </summary>
        public byte[] ToArray()
        {
            return _data.ToArray();
        }
    }

    /// <summary>
    /// Reads structured TCP packets from a NetworkStream.
    /// In PKO, every TCP packet has a 2-byte Big-Endian length header.
    /// Heartbeat/Keep-alive packets are exactly 2 bytes in length.
    /// </summary>
    public class TcpStreamPacketReader
    {
        private readonly NetworkStream _stream;
        private readonly byte[] _headerBuffer = new byte[2];

        /// <summary>
        /// Initializes a new instance of <see cref="TcpStreamPacketReader"/> using the specified network stream.
        /// </summary>
        /// <param name="stream">The open network stream to read packets from.</param>
        public TcpStreamPacketReader(NetworkStream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        /// <summary>
        /// Asynchronously reads a single packet from the network stream.
        /// Returns null if the end of the stream is reached (EOF).
        /// </summary>
        public async Task<byte[]?> ReadPacketAsync(CancellationToken cancellationToken)
        {
            int bytesRead = 0;
            while (bytesRead < 2)
            {
                int read = await _stream.ReadAsync(_headerBuffer, bytesRead, 2 - bytesRead, cancellationToken);
                if (read == 0) return null; // EOF
                bytesRead += read;
            }

            ushort size = (ushort)((_headerBuffer[0] << 8) | _headerBuffer[1]);
            if (size < 2)
                throw new InvalidDataException($"Invalid packet size: {size}");

            byte[] packet = new byte[size];
            packet[0] = _headerBuffer[0];
            packet[1] = _headerBuffer[1];

            int target = size - 2;
            int offset = 2;
            while (target > 0)
            {
                int read = await _stream.ReadAsync(packet, offset, target, cancellationToken);
                if (read == 0)
                    throw new IOException("Connection closed while reading packet body.");
                offset += read;
                target -= read;
            }

            return packet;
        }
    }
}
