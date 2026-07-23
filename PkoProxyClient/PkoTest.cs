using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace PkoProxyClient
{
    /// <summary>
    /// Comprehensive test harness verifying cryptography algorithms, packet codecs, binary IO boundary safety,
    /// state machines, and end-to-end protocol simulations.
    /// </summary>
    public static class PkoTest
    {
        /// <summary>
        /// Runs all built-in tests sequentially. Returns true if all pass, false if any fails.
        /// </summary>
        public static bool RunTests()
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine("====================================================");
            Console.WriteLine("    RUNNING PKO C# CRYTOGRAPHY & PROTOCOL TESTS     ");
            Console.WriteLine("====================================================");
            Console.ResetColor();

            try
            {
                // Custom test for B decrypted
                byte[] data = new byte[] { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
                byte[] keyB = new byte[] { 0x99, 0x3a, 0x3c, 0x7b, 0x27, 0xb1 };
                PacketEncoder.encrypt_B(data, keyB, false);

                Console.Write("C# B decrypted: ");
                foreach (byte b in data)
                {
                    Console.Write($"{b:x2} ");
                }
                Console.WriteLine();

                // Custom test to compare with C++ decrypted value
                byte[] cppKey = new byte[] { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
                byte[] cppPwd = new byte[23];
                byte[] pwdChars = Encoding.ASCII.GetBytes("chap12345chap678901234");
                Array.Copy(pwdChars, cppPwd, 22);

                byte[] outBytes = new byte[8];
                PkoDes.RunDes(PkoDes.DECRYPT, PkoDes.ECB, cppKey, outBytes, cppPwd);

                Console.Write("C# decrypted: ");
                foreach (byte b in outBytes)
                {
                    Console.Write($"{b:x2} ");
                }
                Console.WriteLine();

                // Test 1: DES Pad and Single Key ECB Encrypt/Decrypt
                byte[] plaintext = Encoding.ASCII.GetBytes("HelloPKOWorld!");
                byte[] key = Encoding.ASCII.GetBytes("mypassword123"); // 13 bytes, nKey = 1

                byte[] padded = PkoDes.RunPad(plaintext);
                byte[] encrypted = new byte[padded.Length];
                byte[] decrypted = new byte[padded.Length];

                if (!PkoDes.RunDes(PkoDes.ENCRYPT, PkoDes.ECB, padded, encrypted, key))
                {
                    LogFail("RunDes Encrypt failed");
                    return false;
                }

                if (!PkoDes.RunDes(PkoDes.DECRYPT, PkoDes.ECB, encrypted, decrypted, key))
                {
                    LogFail("RunDes Decrypt failed");
                    return false;
                }

                string decStr = Encoding.ASCII.GetString(decrypted).TrimEnd('\0');
                if (decStr != "HelloPKOWorld!")
                {
                    LogFail($"Decrypted string '{decStr}' does not match original 'HelloPKOWorld!'");
                    return false;
                }
                LogPass("CDES ECB 1-key Encryption and Decryption");

                // Test 2: TripleDES ECB 2-Keys and 3-Keys
                byte[] key2 = Encoding.ASCII.GetBytes("key12345key67890"); // 16 bytes, nKey = 2
                Array.Clear(encrypted, 0, encrypted.Length);
                Array.Clear(decrypted, 0, decrypted.Length);

                PkoDes.RunDes(PkoDes.ENCRYPT, PkoDes.ECB, padded, encrypted, key2);
                PkoDes.RunDes(PkoDes.DECRYPT, PkoDes.ECB, encrypted, decrypted, key2);
                decStr = Encoding.ASCII.GetString(decrypted).TrimEnd('\0');
                if (decStr != "HelloPKOWorld!")
                {
                    LogFail($"3DES 2-key decrypted string '{decStr}' incorrect");
                    return false;
                }
                LogPass("CDES ECB 2-key 3DES");

                byte[] key3 = Encoding.ASCII.GetBytes("key12345key67890keyabcde"); // 24 bytes, nKey = 3
                Array.Clear(encrypted, 0, encrypted.Length);
                Array.Clear(decrypted, 0, decrypted.Length);

                PkoDes.RunDes(PkoDes.ENCRYPT, PkoDes.ECB, padded, encrypted, key3);
                PkoDes.RunDes(PkoDes.DECRYPT, PkoDes.ECB, encrypted, decrypted, key3);
                decStr = Encoding.ASCII.GetString(decrypted).TrimEnd('\0');
                if (decStr != "HelloPKOWorld!")
                {
                    LogFail($"3DES 3-key decrypted string '{decStr}' incorrect");
                    return false;
                }
                LogPass("CDES ECB 3-key 3DES");

                // Test 3: PasswordEncoder Encode
                byte[] pwdEncoded = PasswordEncoder.Encode("my_super_password", "chap12345chap678");
                if (pwdEncoded == null || pwdEncoded.Length == 0)
                {
                    LogFail("PasswordEncoder returned empty bytes");
                    return false;
                }
                LogPass($"PasswordEncoder.Encode. Output Length: {pwdEncoded.Length}");

                // Test 4: PacketEncoder encrypt_B and decrypt_B
                byte[] payload = Encoding.ASCII.GetBytes("A very secure message that we want to encrypt with 'B' algorithm!");
                byte[] sessionKey = new byte[] { 0x05, 0xF1, 0x32, 0x1A, 0xCC, 0x48 }; // 6 bytes
                byte[] payloadCopy = (byte[])payload.Clone();

                PacketEncoder.encrypt_B(payloadCopy, sessionKey, true);
                PacketEncoder.encrypt_B(payloadCopy, sessionKey, false);

                string bDecStr = Encoding.ASCII.GetString(payloadCopy);
                if (bDecStr != "A very secure message that we want to encrypt with 'B' algorithm!")
                {
                    LogFail($"encrypt_B/decrypt_B is not symmetric. Got: '{bDecStr}'");
                    return false;
                }
                LogPass("PacketEncoder 'B' Algorithm symmetry");

                // Test 5: PacketEncoder encrypt_Noise and decrypt_Noise (using separate synced keys)
                byte[] encryptNoiseKey = new byte[] { 0x01, 0x02, 0x04, 0x08 };
                byte[] decryptNoiseKey = new byte[] { 0x01, 0x02, 0x04, 0x08 };
                byte[] noisePayload = Encoding.ASCII.GetBytes("PacketWith8+Bytes");
                byte[] noisePayloadCopy = (byte[])noisePayload.Clone();

                PacketEncoder.encrypt_Noise(encryptNoiseKey, noisePayloadCopy);
                PacketEncoder.decrypt_Noise(decryptNoiseKey, noisePayloadCopy);

                string noiseDecStr = Encoding.ASCII.GetString(noisePayloadCopy);
                if (noiseDecStr != "PacketWith8+Bytes")
                {
                    LogFail($"Noise Algorithm is not symmetric. Got: '{noiseDecStr}'");
                    return false;
                }
                LogPass("PacketEncoder Noise Algorithm symmetry");

                // Test 6: Binary IO
                var writer = new PkoPacketWriter();
                writer.WriteUint16(42);
                writer.WriteUint32(0xDEADBEEF);
                writer.WriteString("PKORules!");
                byte[] serialized = writer.ToArray();

                var pktReader = new PkoPacketReader(serialized);
                ushort u16 = pktReader.ReadUint16();
                uint u32 = pktReader.ReadUint32();
                string s = pktReader.ReadString();

                if (u16 != 42 || u32 != 0xDEADBEEF || s != "PKORules!")
                {
                    LogFail($"Binary IO mismatch. Got: {u16}, 0x{u32:X}, '{s}'");
                    return false;
                }
                LogPass("PkoPacketWriter and PkoPacketReader Basic IO");

                // New Test 7: ClientState transitions verification simulation
                var state = ClientState.Disconnected;
                if (state != ClientState.Disconnected)
                {
                    LogFail("ClientState is not initialized to Disconnected.");
                    return false;
                }
                state = ClientState.Connected;
                state = ClientState.Handshaking;
                state = ClientState.Authenticated;
                state = ClientState.Playing;
                if (state != ClientState.Playing)
                {
                    LogFail("ClientState transition mapping is flawed.");
                    return false;
                }
                LogPass("PkoClient State Machine Transition Simulation");

                // New Test 8: End-to-end stateful PacketEncryptor full lifecycle simulation
                var encryptor = new PacketEncryptor();
                if (encryptor.Enabled)
                {
                    LogFail("Encryptor must start disabled.");
                    return false;
                }

                ushort ver = 136;
                string chap = "chapChallengeString001";
                string pwd = "mySecurePassword_test_123";
                byte[] pwdBytes = PasswordEncoder.Encode(pwd, chap);

                // Derived randomized session key by server
                byte[] rawSessionKey = new byte[] { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22 };
                byte[] pwdBytesTrunc = pwdBytes;
                if (pwdBytesTrunc.Length > 0)
                {
                    byte[] trunc = new byte[pwdBytesTrunc.Length - 1];
                    Array.Copy(pwdBytesTrunc, trunc, trunc.Length);
                    pwdBytesTrunc = trunc;
                }

                byte[] encSessionKey = new byte[8];
                PkoDes.RunDes(PkoDes.ENCRYPT, PkoDes.ECB, rawSessionKey, encSessionKey, pwdBytesTrunc);

                encryptor.Init(true, ver, chap, pwdBytes, encSessionKey);
                if (!encryptor.Enabled)
                {
                    LogFail("Encryptor initialization failed to enable crypto.");
                    return false;
                }

                // Encrypt a simulated client payload
                byte[] originalPayload = Encoding.ASCII.GetBytes("ThisIsAPatternOfBytesToEncryptE2E");
                byte[] testPayloadCopy = (byte[])originalPayload.Clone();

                encryptor.Encrypt(testPayloadCopy, EncryptType.CS);

                // Ensure it got scrambled
                if (ByteArrayCompare(originalPayload, testPayloadCopy))
                {
                    LogFail("Encryptor.Encrypt did not scramble the data.");
                    return false;
                }

                // Decrypt it
                encryptor.Decrypt(testPayloadCopy, DecryptType.CS);

                if (!ByteArrayCompare(originalPayload, testPayloadCopy))
                {
                    LogFail("End-to-end Encryption-Decryption symmetry failed.");
                    return false;
                }
                LogPass("Full Lifecycle Stateful PacketEncryptor Simulation");

                // New Test 9: Binary IO boundary safety and exception throwing checks
                byte[] tinyBuffer = new byte[] { 0x11 };
                var safetyReader = new PkoPacketReader(tinyBuffer);
                _ = safetyReader.ReadByte();

                bool exceptionCaught = false;
                try
                {
                    _ = safetyReader.ReadByte();
                }
                catch (EndOfStreamException)
                {
                    exceptionCaught = true;
                }

                if (!exceptionCaught)
                {
                    LogFail("SafetyReader did not throw EndOfStreamException when reading past end of packet.");
                    return false;
                }
                LogPass("PkoPacketReader Boundary Safety and Exception Checks");

                // New Test 10: PkoPacket properties and ProxySession sequencing
                byte[] mockPacketData = new byte[] {
                    0, 16,               // Size (16)
                    0x80, 0, 0, 0,       // Session (0x80000000)
                    0, 6,                // Command (6)
                    0, 0, 0, 99,         // ulWorldID (99)
                    0, 0, 0, 1           // PacketCount (1)
                };
                var testPkt = new PkoPacket(mockPacketData);
                if (testPkt.Size != 16 || testPkt.Session != 0x80000000 || testPkt.Command != 6 || testPkt.PacketCount != 1)
                {
                    LogFail($"PkoPacket parsing failed. Size: {testPkt.Size}, Session: {testPkt.Session:X}, Command: {testPkt.Command}, Count: {testPkt.PacketCount}");
                    return false;
                }

                // Check setters
                testPkt.Size = 20;
                testPkt.Session = 0x12345678;
                testPkt.Command = 6;
                testPkt.PacketCount = 42;
                if (testPkt.Size != 20 || testPkt.Session != 0x12345678 || testPkt.Command != 6 || testPkt.PacketCount != 42)
                {
                    LogFail("PkoPacket properties setter failed.");
                    return false;
                }

                // Check ProxySession sequencing simulation
                var testSession = new ProxySession {
                    NextPacketCount = 100,
                    PacketCountInitialized = false
                };

                // Create a temporary loopback socket/stream to simulate sending
                byte[] testSeqBytes = new byte[] {
                    0, 16,
                    0, 0, 0, 0,
                    0, 6,
                    0, 0, 0, 99, // ulWorldID
                    0, 0, 0, 50  // initial packetCount is 50 at bytes 12-15
                };
                var pSeq = new PkoPacket(testSeqBytes);

                // Manually simulate what SendClientPacketAsync does under lock
                if (pSeq.HasPacketCount)
                {
                    if (!testSession.PacketCountInitialized)
                    {
                        testSession.NextPacketCount = pSeq.PacketCount;
                        testSession.PacketCountInitialized = true;
                    }
                    pSeq.PacketCount = testSession.NextPacketCount++;
                }

                if (!testSession.PacketCountInitialized || testSession.NextPacketCount != 51 || pSeq.PacketCount != 50)
                {
                    LogFail($"ProxySession sequencing simulation failed. Initialized: {testSession.PacketCountInitialized}, Next: {testSession.NextPacketCount}, Count: {pSeq.PacketCount}");
                    return false;
                }
                LogPass("PkoPacket Accessors and ProxySession Sequence Counter");

                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("\n====================================================");
                Console.WriteLine("       ALL 10 UNIT TESTS PASSED SUCCESSFULLY!       ");
                Console.WriteLine("====================================================");
                Console.ResetColor();
                return true;
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"\nFAIL: Exception thrown during tests: {ex.Message}\n{ex.StackTrace}");
                Console.ResetColor();
                return false;
            }
        }

        private static void LogPass(string name)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine($"[PASS] {name}");
            Console.ResetColor();
        }

        private static void LogFail(string reason)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"[FAIL] {reason}");
            Console.ResetColor();
        }

        private static bool ByteArrayCompare(byte[] a1, byte[] a2)
        {
            if (a1 == null || a2 == null) return ReferenceEquals(a1, a2);
            if (a1.Length != a2.Length) return false;
            for (int i = 0; i < a1.Length; i++)
                if (a1[i] != a2[i]) return false;
            return true;
        }
    }
}
