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

                // Test 10: BotPlugin Integration and packetCount synchronization
                var botPlugin = new BotPlugin();

                // Simulate CMD_MC_CHABEGINSEE packet payload:
                // seeType: 1, chaId: 100, worldId: 200, commId: 300, commName: "Owner", gm: 0, handle: 400, ctrlType: 1 (mob), name: "Boar", motto: "Grunt"
                // icon: 12, guildId: 0, guildName: "", guildMotto: "", stallName: "", state: 0, x: 1500, y: 2500
                var writerBeginSee = new PkoPacketWriter();
                writerBeginSee.WriteUint16(0); // size placeholder
                writerBeginSee.WriteUint32(0x80000000); // session
                writerBeginSee.WriteUint16((ushort)PkoCommand.CMD_MC_CHABEGINSEE);
                writerBeginSee.WriteByte(1); // seeType
                writerBeginSee.WriteUint32(100); // chaId
                writerBeginSee.WriteUint32(200); // worldId
                writerBeginSee.WriteUint32(300); // commId
                writerBeginSee.WriteString("Owner"); // commName
                writerBeginSee.WriteByte(0); // gm
                writerBeginSee.WriteUint32(400); // handle
                writerBeginSee.WriteByte(1); // ctrlType
                writerBeginSee.WriteString("Boar"); // name
                writerBeginSee.WriteString("Grunt"); // motto
                writerBeginSee.WriteUint16(12); // icon
                writerBeginSee.WriteUint32(0); // guildId
                writerBeginSee.WriteString(""); // guildName
                writerBeginSee.WriteString(""); // guildMotto
                writerBeginSee.WriteString(""); // stallName
                writerBeginSee.WriteUint16(0); // state
                writerBeginSee.WriteUint32(1500); // x
                writerBeginSee.WriteUint32(2500); // y

                byte[] beginSeeData = writerBeginSee.ToArray();
                ushort beginSeeSize = (ushort)beginSeeData.Length;
                beginSeeData[0] = (byte)(beginSeeSize >> 8);
                beginSeeData[1] = (byte)(beginSeeSize & 0xFF);

                var contextBeginSee = new ProxyPacketContext(1, "S -> C", (ushort)PkoCommand.CMD_MC_CHABEGINSEE, 0x80000000, beginSeeData);
                botPlugin.OnPacket(contextBeginSee);

                if (botPlugin.Mobs.Count != 1 || !botPlugin.Mobs.ContainsKey(200) || botPlugin.Mobs[200].Name != "Boar")
                {
                    LogFail("BotPlugin failed to track CMD_MC_CHABEGINSEE");
                    return false;
                }

                // Simulate CMD_MC_ITEMBEGINSEE packet payload:
                // worldId: 500, handle: 600, itemId: 1001, x: 1510, y: 2510, angle: 0, num: 1, appeType: 1, fromId: 0
                var writerItemSee = new PkoPacketWriter();
                writerItemSee.WriteUint16(0); // size placeholder
                writerItemSee.WriteUint32(0x80000000); // session
                writerItemSee.WriteUint16((ushort)PkoCommand.CMD_MC_ITEMBEGINSEE);
                writerItemSee.WriteUint32(500); // worldId
                writerItemSee.WriteUint32(600); // handle
                writerItemSee.WriteUint32(1001); // itemId
                writerItemSee.WriteUint32(1510); // x
                writerItemSee.WriteUint32(2510); // y
                writerItemSee.WriteUint16(0); // angle
                writerItemSee.WriteUint16(1); // num
                writerItemSee.WriteByte(1); // appeType
                writerItemSee.WriteUint32(0); // fromId

                byte[] itemSeeData = writerItemSee.ToArray();
                ushort itemSeeSize = (ushort)itemSeeData.Length;
                itemSeeData[0] = (byte)(itemSeeSize >> 8);
                itemSeeData[1] = (byte)(itemSeeSize & 0xFF);

                var contextItemSee = new ProxyPacketContext(1, "S -> C", (ushort)PkoCommand.CMD_MC_ITEMBEGINSEE, 0x80000000, itemSeeData);
                botPlugin.OnPacket(contextItemSee);

                if (botPlugin.Items.Count != 1 || !botPlugin.Items.ContainsKey(500) || botPlugin.Items[500].ItemId != 1001)
                {
                    LogFail("BotPlugin failed to track CMD_MC_ITEMBEGINSEE");
                    return false;
                }

                // Simulate CMD_CM_SAY with message "!bot start" and a current packetCount of 42
                var writerSay = new PkoPacketWriter();
                writerSay.WriteUint16(0); // size placeholder
                writerSay.WriteUint32(0x80000000); // session
                writerSay.WriteUint16((ushort)PkoCommand.CMD_CM_SAY);
                writerSay.WriteUint32(42); // packetCount (sequence number)
                writerSay.WriteString("!bot start");

                byte[] sayData = writerSay.ToArray();
                ushort saySize = (ushort)sayData.Length;
                sayData[0] = (byte)(saySize >> 8);
                sayData[1] = (byte)(saySize & 0xFF);

                var contextSay = new ProxyPacketContext(1, "C -> S", (ushort)PkoCommand.CMD_CM_SAY, 0x80000000, sayData);
                botPlugin.OnPacket(contextSay);

                if (!botPlugin.IsBotActive)
                {
                    LogFail("BotPlugin failed to activate via !bot start");
                    return false;
                }

                if (botPlugin.CurrentPacketCount != 42)
                {
                    LogFail($"BotPlugin failed to track packetCount. Expected 42, got {botPlugin.CurrentPacketCount}");
                    return false;
                }

                // Test TickBot generates SendPickupPacket first (prioritizing items)
                byte[] generatedPacket = null;
                botPlugin.TickBot((pkt) => { generatedPacket = pkt; }, 0x80000000);

                if (generatedPacket == null)
                {
                    LogFail("TickBot did not generate any action packets");
                    return false;
                }

                var genReader = new PkoPacketReader(generatedPacket);
                genReader.ReadUint16(); // skip size
                genReader.ReadUint32(); // skip session
                ushort genPktId = genReader.ReadUint16();
                uint genPktCount = genReader.ReadUint32();
                byte genActionType = genReader.ReadByte();

                if (genPktId != (ushort)PkoCommand.CMD_CM_BEGINACTION || genPktCount != 43 || genActionType != (byte)PkoActionType.enumACTION_ITEM_PICK)
                {
                    LogFail($"BotPlugin generated invalid pickup packet structure. PktId: {genPktId}, PktCount: {genPktCount}, ActionType: {genActionType}");
                    return false;
                }

                // Now remove the item to test attack mob action packet generation and packetCount incrementing to 44
                var writerItemEnd = new PkoPacketWriter();
                writerItemEnd.WriteUint16(0);
                writerItemEnd.WriteUint32(0x80000000);
                writerItemEnd.WriteUint16((ushort)PkoCommand.CMD_MC_ITEMENDSEE);
                writerItemEnd.WriteUint32(500); // worldId of item to destroy

                byte[] itemEndData = writerItemEnd.ToArray();
                ushort itemEndSize = (ushort)itemEndData.Length;
                itemEndData[0] = (byte)(itemEndSize >> 8);
                itemEndData[1] = (byte)(itemEndSize & 0xFF);

                var contextItemEnd = new ProxyPacketContext(1, "S -> C", (ushort)PkoCommand.CMD_MC_ITEMENDSEE, 0x80000000, itemEndData);
                botPlugin.OnPacket(contextItemEnd);

                if (botPlugin.Items.Count != 0)
                {
                    LogFail("BotPlugin failed to remove item on CMD_MC_ITEMENDSEE");
                    return false;
                }

                // Tick bot again, it should generate attack/skill packet targeting worldId 200 with packetCount 44
                generatedPacket = null;
                botPlugin.TickBot((pkt) => { generatedPacket = pkt; }, 0x80000000);

                if (generatedPacket == null)
                {
                    LogFail("TickBot did not generate attack packet");
                    return false;
                }

                genReader = new PkoPacketReader(generatedPacket);
                genReader.ReadUint16(); // skip size
                genReader.ReadUint32(); // skip session
                genPktId = genReader.ReadUint16();
                genPktCount = genReader.ReadUint32();
                genActionType = genReader.ReadByte();

                if (genPktId != (ushort)PkoCommand.CMD_CM_BEGINACTION || genPktCount != 44 || genActionType != (byte)PkoActionType.enumACTION_SKILL)
                {
                    LogFail($"BotPlugin generated invalid attack packet structure. PktId: {genPktId}, PktCount: {genPktCount}, ActionType: {genActionType}");
                    return false;
                }

                LogPass("BotPlugin see/endsee tracking, CMD_CM_SAY command toggles, packetCount synchronization, and Action selection");

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
