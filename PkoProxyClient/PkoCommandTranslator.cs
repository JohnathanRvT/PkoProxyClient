using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

namespace PkoProxyClient
{
    /// <summary>
    /// Dynamically parses PKO's C++ protocol header (NetCommand.h) to translate
    /// Packet ID (Opcode) integers into human-readable macro names.
    /// Includes a robust fallback dictionary if the header file is not accessible.
    /// </summary>
    public static class PkoCommandTranslator
    {
        private static readonly Dictionary<int, string> _commands = new Dictionary<int, string>();
        private static readonly Dictionary<string, int> _constants = new Dictionary<string, int>();
        private static bool _initialized = false;

        static PkoCommandTranslator()
        {
            Initialize();
        }

        /// <summary>
        /// Translates a Packet ID (Opcode) into its readable macro name.
        /// </summary>
        public static string GetCommandName(int packetId)
        {
            if (!_initialized)
            {
                Initialize();
            }

            if (_commands.TryGetValue(packetId, out string? name))
            {
                return name;
            }

            return $"CMD_UNKNOWN_{packetId}";
        }

        /// <summary>
        /// Forces initialization/parsing of NetCommand.h or loads fallback defaults.
        /// </summary>
        public static void Initialize()
        {
            _commands.Clear();
            _constants.Clear();

            // 1. Load Standalone Defaults (Fallbacks in case NetCommand.h is missing)
            LoadDefaults();

            // 2. Locate and dynamically parse NetCommand.h
            string relativePath = Path.Combine("source", "inc", "Common", "NetCommand.h");
            string fullPath = Path.GetFullPath(relativePath);

            if (File.Exists(fullPath))
            {
                try
                {
                    ParseHeaderFile(fullPath);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[Warning] Failed to dynamically parse NetCommand.h: {ex.Message}. Fallbacks active.");
                }
            }
            else
            {
                // Try parent directories recursively
                string dir = AppDomain.CurrentDomain.BaseDirectory;
                bool found = false;
                for (int i = 0; i < 5; i++)
                {
                    string candidate = Path.Combine(dir, relativePath);
                    if (File.Exists(candidate))
                    {
                        try
                        {
                            ParseHeaderFile(candidate);
                            found = true;
                            break;
                        }
                        catch { }
                    }
                    var parent = Directory.GetParent(dir);
                    if (parent == null) break;
                    dir = parent.FullName;
                }

                if (!found)
                {
                    // Fallbacks are already loaded
                    _initialized = true;
                }
            }
        }

        private static void LoadDefaults()
        {
            // Core standard PKO commands
            _commands[17] = "CMD_CM_CHECK_PING";
            _commands[431] = "CMD_CM_LOGIN";
            _commands[432] = "CMD_CM_LOGOUT";
            _commands[433] = "CMD_CM_BGNPLAY";
            _commands[434] = "CMD_CM_ENDPLAY";
            _commands[435] = "CMD_CM_NEWCHA";
            _commands[436] = "CMD_CM_DELCHA";

            _commands[517] = "CMD_MC_CHECK_PING";
            _commands[537] = "CMD_MC_PING";
            _commands[931] = "CMD_MC_LOGIN";
            _commands[940] = "CMD_MC_CHAPSTR";
        }

        private static void ParseHeaderFile(string filePath)
        {
            string[] lines = File.ReadAllLines(filePath);

            // Regex patterns
            // Match "#define CMD_CM_BASE 0"
            var defineRegex = new Regex(@"^\s*#define\s+([A-Za-z0-9_]+)\s+([0-9]+)\b", RegexOptions.Compiled);

            // Match "MESSAGE_DEF(CMD_CM_SAY, CMD_CM_BASE + 1, ...)"
            var msgDefRegex = new Regex(@"^\s*MESSAGE_DEF\s*\(\s*([A-Za-z0-9_]+)\s*,\s*([A-Za-z0-9_\s\+\-]+)\s*,", RegexOptions.Compiled);

            foreach (string line in lines)
            {
                // 1. Check for standard integer definitions
                var defMatch = defineRegex.Match(line);
                if (defMatch.Success)
                {
                    string name = defMatch.Groups[1].Value;
                    if (int.TryParse(defMatch.Groups[2].Value, out int val))
                    {
                        _constants[name] = val;
                    }
                    continue;
                }

                // 2. Check for MESSAGE_DEF macros
                var msgMatch = msgDefRegex.Match(line);
                if (msgMatch.Success)
                {
                    string name = msgMatch.Groups[1].Value;
                    string expr = msgMatch.Groups[2].Value.Trim();

                    int resolvedValue = EvaluateExpression(expr);
                    if (resolvedValue >= 0)
                    {
                        _constants[name] = resolvedValue;
                        _commands[resolvedValue] = name;
                    }
                }
            }

            _initialized = true;
        }

        private static int EvaluateExpression(string expr)
        {
            // Simple expression evaluator supporting "NAME" or "NAME + OFFSET" or integers
            expr = expr.Replace(" ", "");

            if (int.TryParse(expr, out int directVal))
            {
                return directVal;
            }

            string[] tokens = expr.Split('+');
            int total = 0;

            foreach (string t in tokens)
            {
                string token = t.Trim();
                if (string.IsNullOrEmpty(token)) continue;

                if (int.TryParse(token, out int val))
                {
                    total += val;
                }
                else if (_constants.TryGetValue(token, out int constVal))
                {
                    total += constVal;
                }
                else
                {
                    // Unresolved variable token
                    return -1;
                }
            }

            return total;
        }
    }
}
