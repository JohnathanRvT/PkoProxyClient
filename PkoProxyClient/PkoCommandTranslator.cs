using System;

namespace PkoProxyClient
{
    /// <summary>
    /// Represents the standard PKO protocol Packet ID (Opcode) commands.
    /// Derived from source/inc/Common/NetCommand.h.
    /// </summary>
    public enum PkoCommand : ushort
    {
        // ==========================================
        // Client -> GameServer (CM Base = 0)
        // ==========================================
        CMD_CM_SAY = 1,
        CMD_CM_BEGINACTION = 6,
        CMD_CM_ENDACTION = 7,
        CMD_CM_SYNATTR = 8,
        CMD_CM_SYNSKILLBAG = 9,
        CMD_CM_DIE_RETURN = 10,
        CMD_CM_SKILLUPGRADE = 11,
        CMD_CM_PING = 15,
        CMD_CM_REFRESH_DATA = 16,
        CMD_CM_CHECK_PING = 17,
        CMD_CM_MAP_MASK = 18,
        CMD_CM_UPDATEHAIR = 20,

        // Character select and login commands (CharBase = 430)
        CMD_CM_LOGIN = 431,
        CMD_CM_LOGOUT = 432,
        CMD_CM_BGNPLAY = 433,
        CMD_CM_ENDPLAY = 434,
        CMD_CM_NEWCHA = 435,
        CMD_CM_DELCHA = 436,

        // ==========================================
        // GameServer -> Client (MC Base = 500)
        // ==========================================
        CMD_MC_SAY = 501,
        CMD_MC_MAPCRASH = 503,
        CMD_MC_CHABEGINSEE = 504,
        CMD_MC_CHAENDSEE = 505,
        CMD_MC_ITEMBEGINSEE = 506,
        CMD_MC_ITEMENDSEE = 507,
        CMD_MC_NOTIACTION = 508,
        CMD_MC_SYNATTR = 509,
        CMD_MC_SYNSKILLBAG = 510,
        CMD_MC_SYNASKILLSTATE = 511,
        CMD_MC_CHECK_PING = 517, // Shared with CMD_MC_SYSINFO depending on version
        CMD_MC_ENTERMAP = 516,
        CMD_MC_SYSINFO = 517,
        CMD_MC_ALARM = 518,
        CMD_MC_TEAM = 519,
        CMD_MC_FAILEDACTION = 520,
        CMD_MC_MESSAGE = 521,
        CMD_MC_PING = 537,

        // Character select and login responses (CharBase = 930)
        CMD_MC_LOGIN = 931,
        CMD_MC_LOGOUT = 932,
        CMD_MC_BGNPLAY = 933,
        CMD_MC_ENDPLAY = 934,
        CMD_MC_NEWCHA = 935,
        CMD_MC_DELCHA = 936,
        CMD_MC_CHAPSTR = 940
    }

    /// <summary>
    /// Translates PKO command packet IDs to readable string macro names using the PkoCommand C# enum.
    /// </summary>
    public static class PkoCommandTranslator
    {
        /// <summary>
        /// Translates a Packet ID (Opcode) into its readable macro name.
        /// </summary>
        public static string GetCommandName(int packetId)
        {
            if (Enum.IsDefined(typeof(PkoCommand), (ushort)packetId))
            {
                return ((PkoCommand)packetId).ToString();
            }

            return $"CMD_UNKNOWN_{packetId}";
        }
    }
}
