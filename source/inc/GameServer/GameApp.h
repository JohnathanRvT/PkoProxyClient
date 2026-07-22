//=============================================================================
// FileName: GameApp.h
// Creater: ZhangXuedong
// Date: 2004.11.04
// Comment: CChaMgr class
//=============================================================================

#ifndef GAMEAPP_H
#define GAMEAPP_H

//#define DISABLE_GM_CMD				//	关闭gm命令

#include "GameAppNet.h"
#include "point.h"
#include "RoleData.h"
#include "Config.h"
#include "Identity.h"
#include "Player.h"
#include "SkillStateRecord.h"
#include "Script.h"
#include "AreaRecord.h"
#include "SkillTemp.h"
#include "StateCell.h"
#include "Parser.h"
#include "CommFunc.h"
#include "EntityAlloc.h"
#include "MapRes.h"
#include "PicSet.h"
#include "atltime.h"
#include "NoLockrecord.h"

#include "ProfileMonitor.h" // 测试用
#include "MapCopyRecord.h"

class CCharacter;
class CPlayer;
class CItem;
class CPassengerMgr;

namespace mission {
class CTalkNpc;
class CNpc;
class CTradeData;
class CStallData;
} // namespace mission

class SubMap;
class CCharacter;
class CItemRecordAttr;

class CChaRecordSet;
class CSkillRecordSet;
class CItemRecordSet;
class CLevelRecordSet;
class CSailLvRecordSet;
class CLifeLvRecordSet;
class CJobEquipRecordSet;
class CForgeRecordSet;
class CHairRecordSet;
class CNoLockRecordSet;  //Add by sunny.sun 20090710
class CMapCopyRecordSet; //Add by sunny.sun 20090811

class GateServer;

#define MAX_DBLOG_POOL 100

struct SDBLogData {
	int nLoc;		  // 在pool中的位置
	char szLog[8192]; // 字符串内容
	SDBLogData() : nLoc(0) {}
};

class CDBLogMgr // 管理即将写入db的log
{

public:
	CDBLogMgr()
		: _nPerLogCnt(5), _nLogLeft(0), _nPoolUseLoc(0) {
		for (int i = 0; i < MAX_DBLOG_POOL; i++) {
			_LogPool[i].nLoc = i;
		}
	}

	// 可以Log 5个字符串字段, 最后一个长度为8000字符以内
	void Log(const char* type, const char* c1, const char* c2, const char* c3, const char* c4, const char* p, BOOL bAddToList = TRUE);

	// Add by lark.li 20080324 begin
	void TradeLog(const char* action, const char* pszChaFrom, const char* pszChaTo, const char* pszTrade);
	// End

	void HandleLogList();
	void FlushLogList(); // GameServer关闭的时候, 保证剩下的log都写入DB

	int GetLogLeft() { return _nLogLeft; }			   // 还剩下的没有处理的log数量, 可以输出到监控界面
	int SetPerLogCnt(int nCnt) { _nPerLogCnt = nCnt; } // 设置每次写入的数量, 可以动态调整
	int GetPerLogCnt() { return _nPerLogCnt; }		   // 用来监控输出

protected:
	std::list<SDBLogData*> _LogList;
	int _nPerLogCnt;					 // 每次写入的最大log数量
	int _nLogLeft;						 // 还剩下的log数量
	SDBLogData _LogPool[MAX_DBLOG_POOL]; // 存放要被log的数据, 该pool被循环使用
	int _nPoolUseLoc;					 // 当前正被使用的pool的位置
};

struct SVolunteer {
	char szName[defENTITY_NAME_LEN]; // 姓名
	unsigned long ulID;				 // ID
	long lLevel;					 // 等级
	long lJob;						 // 职业
	char szMapName[256];			 // 地图
};

class CGameApp : public CDBLogMgr {
public:
	CGameApp();
	~CGameApp();

	BOOL Init();
	BOOL InitMap();
	void Run(DWORD dwCurTime);

	// 与网络消息有关的4个处理接口函数
	void ProcessNetMsg(int nMsgType, GateServer* pGate, RPACKET pkt);
	void OnGateConnected(GateServer* pGate, RPACKET pkt);
	void OnGateDisconnect(GateServer* pGate, RPACKET pkt);
	void ProcessPacket(GateServer* pGate, RPACKET pkt);
	void ProcessTeamMsg(GateServer* pGate, RPACKET pkt);
	void ProcessGuildMsg(GateServer* pGate, RPACKET pkt);
	void ProcessGuildChallMoney(GateServer* pGate, RPACKET pkt);
	void ProcessGuildChallPrizeMoney(GateServer* pGate, RPACKET pkt);
	void ProcessDynMapEntry(GateServer* pGate, RPACKET pkt);

	void ProcessMapAdmin(GateServer* pGate, RPACKET pkt);

	void ProcessInterGameMsg(unsigned short usCmd, GateServer* pGate, RPACKET pkt);
	void ProcessGroupBroadcast(unsigned short usCmd, GateServer* pGate, RPACKET pkt);
	void ProcessGarner2Update(RPACKET pkt); //乱斗白银城
	// 处理TradeServer消息
	void ProcessInfoMsg(pNetMessage msg, short sType, TradeServer* pInfo);
	void ProcessMsg(pNetMessage msg, TradeServer* pInfo);
	void OnInfoConnected(TradeServer* pInfo);
	void OnInfoDisconnected(TradeServer* pInfo);

	//------------------------------------

	CPlayer* CreateGamePlayer(const char szPassword[], uLong ulChaDBId, uLong ulWorldId, const char* cszMapName, char chType);
	void ReleaseGamePlayer(CPlayer*);
	void GoOutGame(CPlayer* pPlayer, bool bOffLine);
	CPlayer* GetNewPlayer();
	CPlayer* GetPlayer(long lHandle);
	CPlayer* IsValidPlayer(long lID, long lHandle);
	CCharacter* GetNewCharacter();
	CItem* GetNewItem();
	mission::CTalkNpc* GetNewTNpc();
	Entity* GetEntity(long lHandle);
	Entity* IsValidEntity(unsigned long ulID, long lHandle);
	Entity* IsLiveingEntity(unsigned long ulID, long lHandle);
	Entity* IsMapEntity(unsigned long ulID, long lHandle);
	Entity* IsLifeEntity(unsigned long ulID, long lHandle);
	void BeginGetTNpc();
	mission::CTalkNpc* GetNextTNpc();

	int LogPlayer();

	void AddPlayerIdx(DWORD dwDBID, CPlayer* pPlayer);
	void DelPlayerIdx(DWORD dwDBID);
	CPlayer* GetPlayerByDBID(DWORD dwDBID);
	CPlayer* GetPlayerByMainChaName(const char* sMainChaName);
	void AfterPlayerLogin(const char* cszPlyName);
	void NoticePlayerLogin(CPlayer* pCPlayer);

	CMapRes* GetMap(int no);
	CMapRes* FindMapByName(const char* mapname, bool bIncUnRun = false);
	void LoadAllTable();
	void LoadCharacterInfo();
	void LoadSkillInfo();
	void LoadItemInfo();

	// 重载所有npc脚本信息
	BOOL ReloadNpcInfo(CCharacter& character);
	mission::CNpc* FindNpc(const char szName[]);

	// 召唤NPC出现一段时间
	BOOL SummonNpc(BYTE byMapID, USHORT sAreaID, const char szNpc[], USHORT sTime);

	// 创建实体
	mission::CEventEntity* CreateEntity(BYTE byType) { return m_pCEntSpace->GetEventEntity(byType); }

	void NotiGameReset(unsigned long ulLeftSec);
	void SaveAllPlayer();

	void SetEntityEnableLog(bool bValid = true);
	CSkillTempData* GetSkillTData(short sSkillNo, char chSkillLv);
	void SetSkillTDataRange(short* psRange);
	void SetSkillTDataState(short* psState);
	void InitSStateTraOnTime();
	long GetSStateTraOnTime(unsigned char uchStateID, unsigned char uchStateLv);

	void DataStatistic(); // 信息统计
	CCharacter* FindPlayerChaByName(const char* cszChaName);
	CCharacter* FindPlayerChaByID(unsigned long ulChaID);
	CCharacter* FindMainPlayerChaByID(unsigned long ulChaID);
	CCharacter* FindChaByID(unsigned long ulChaID);
	CCharacter* FindChaByName(const char* cszChaName);
	CPlayer* FindPlayerByDBChaID(unsigned long ulDBChaID);

	void WorldNotice(const char* szString);								  // 通知本组所有GameServer上的玩家
	void ScrollNotice(const char* szString, int SetNum);				  //Add by sunny.sun20080804
	void GMNotice(const char* szString);								  //add by sunny.sun 20080821
	void LocalNotice(const char* szString);								  // 通知本GameServer上的玩家
	void ChaNotice(const char* szNotiString, const char* szChaName = ""); // 通知指定玩家

	bool IsChaAttrMaxValInit() { return m_bChaAttrMaxValInit; }
	void ChaAttrMaxValInit(bool bSet) { m_bChaAttrMaxValInit = bSet; }

	void CheckSeeWithTeamChange(bool CanSeen[][2], CPlayer** pCPlayerList, char chMemberCnt);
	void RefreshTeamEyeshot(bool CanSeenOld[][2], bool CanSeenNew[][2], CPlayer** pCPlayerList, char chMemberCnt, char chRefType);
	void RefreshTeamEyeshot(CPlayer** pCPlayerList, char chMemberCnt, char chRefType);

	BOOL AddVolunteer(CCharacter* pCha);
	BOOL DelVolunteer(CCharacter* pCha);
	SVolunteer* GetVolInfo(int nIndex);
	int GetVolNum();
	SVolunteer* FindVolunteer(const char* szName);
	void ResetHangState(); //Add by sunny.sun 20090519

	DWORD m_dwFPS{0};
	DWORD m_dwRunCnt{0};
	DWORD m_dwChaCnt{0};
	DWORD m_dwPlayerCnt{0};
	DWORD m_dwActiveMgrUnit;
	std::atomic<long> m_dwRunStep{0}; // 性能监控

	BOOL m_bExecLuaCmd{false};
	std::string m_strMapNameList;

	dbc::PreAllocHeap<CPassengerMgr> m_CabinHeap{1, 1000};
	dbc::PreAllocHeap<mission::CTradeData> m_TradeDataHeap{1, 2000};
	dbc::PreAllocHeap<mission::CStallData> m_StallDataHeap{1, ROLE_MAXSIZE_STALLDATA};
	dbc::PreAllocHeap<CSkillTempData> m_SkillTDataHeap{1, defMAX_SKILL_NO};
	dbc::PreAllocHeap<CStateCell> m_MapStateCellHeap{1, 2000};
	dbc::PreAllocHeap<CChaListNode> m_ChaListHeap{1, 2000};
	dbc::PreAllocHeap<CStateCellNode> m_StateCellNodeHeap{1, 1000};

	// 用于信息统计
	dbc::Long m_lCabinHeapNum{0};
	dbc::Long m_lTradeDataHeapNum{0};
	dbc::Long m_lSkillTDataHeapNum{0};
	dbc::Long m_lMapMgrUnitHeapNum{0};
	dbc::Long m_lEntityListHeapNum{0};
	dbc::Long m_lMgrNodeHeapNum{0};
	//

	Identity m_Ident;

	struct // GameServer结束
	{
		unsigned long m_ulLeftSec{0};
		CTimer m_CTimerReset;
	};

	CEntityAlloc* m_pCEntSpace{nullptr};
	CPlayerAlloc* m_pCPlySpace{nullptr};

protected:
	void MgrUnitRun(DWORD dwCurTime);
	void GameItemRun(DWORD dwCurTime);
	void MapMgrRun(DWORD dwCurTime);

protected:
	std::array<std::unique_ptr<CMapRes>, MAX_MAP> m_MapList; // 维护一个指向所有子地图的列表
	short m_mapnum{0};

	CChaRecordSet* m_CChaRecordSet;
	CSkillRecordSet* m_CSkillRecordSet;
	CSkillStateRecordSet* m_CSkillStateRecordSet;
	CItemRecordSet* m_CItemRecordSet;
	CLevelRecordSet* m_CLevelRecordSet;
	CJobEquipRecordSet* m_CJobEquipRecordSet;
	CAreaSet* m_CAreaRecordSet;
	CSailLvRecordSet* m_CSailLvRecord;
	CLifeLvRecordSet* m_CLifeLvRecord;
	CHairRecordSet* m_CHairRecord;
	CNoLockRecordSet* m_CNoLockRecordSet;   //Add by sunny.sun 20090710
	CMapCopyRecordSet* m_CMapCopyRecordSet; //Add by sunny.sun 20090811

	std::map<DWORD, CPlayer*> _PlayerIdx; // 从DB ID映射到Player指针

	std::vector<SVolunteer> m_vecVolunteerList; // 志愿者列表

	struct
	{
		CPlayer* pCPlayerL{nullptr};   // 记录与对应socket连接的玩家
		CPlayer* pCCurPlayer{nullptr}; // 用于搜索玩家链表
	} m_GatePlayer[MAX_GATE];

public:
	CPicSet* m_PicSet{nullptr};
	CTime m_ResetTimer; //更新列表时间
private:
	DWORD _dwFPS;
	DWORD _dwLastTick{0};
	DWORD _dwRunCnt;
	DWORD _dwTempRunCnt{0};

	// 纪录不同等级不同技能的相关数值
	struct {
		std::array<std::array<CSkillTempData*, defMAX_SKILL_LV + 1>, defMAX_SKILL_NO + 1> m_pCSkillTData = {};
		short m_sSkillSetNo;
		char m_chSkillSetLv;
	};

	long m_lSStateTraOnTime[AREA_STATE_MAXID + 1][SKILL_STATE_LEVEL + 1]; // 从地表转移到角色身上的状态的持续时间

	CTimer m_CTimerItem;

	bool m_bChaAttrMaxValInit;
};

inline CMapRes* CGameApp::GetMap(int no) {
	try {
		return m_MapList[no].get();
	}
	T_E
}

inline void CGameApp::AddPlayerIdx(DWORD dwDBID, CPlayer* pPlayer) {
	try {
		_PlayerIdx[dwDBID] = pPlayer;
		//LG("player_idx", "添加DB ID = %d对应的Player\n", dwDBID);
	}
	T_E
}

inline void CGameApp::DelPlayerIdx(DWORD dwDBID) {
	try {
		//LG("player_idx", "清除DB ID = %d对应的Player\n", dwDBID);

		if (auto it = _PlayerIdx.find(dwDBID); it != _PlayerIdx.end()) {
			_PlayerIdx.erase(it);
		} else {
			LG("player_idx", "when delete PlayerIdx it appear error, DB ID = %d no find index\n", dwDBID);
		}
		//LG("player_idx", "Idx Size = %d\n\n", _PlayerIdx.size());
	}
	T_E
}

inline CPlayer* CGameApp::GetPlayerByDBID(DWORD dwDBID) {
	try {
		return _PlayerIdx[dwDBID];
	}
	T_E
}

inline CPlayer* CGameApp::GetPlayerByMainChaName(const char* sMainChaName) {
	try {
		if (!sMainChaName) {
			return nullptr;
		};

		for (auto [ID, pPlayer] : _PlayerIdx) {
			if (pPlayer && pPlayer->GetMainCha()) {
				if (strcmp(pPlayer->GetMainCha()->GetName(), sMainChaName) == 0) {
					return pPlayer;
				}
			}
		}
		return nullptr;
	}
	T_E
};

// 取对应等级技能的相关数据，如果该数据还没有初始化，则执行初始化动作。
inline CSkillTempData* CGameApp::GetSkillTData(short sSkillNo, char chSkillLv) {
	if (!m_pCSkillTData[sSkillNo][chSkillLv]) {
		m_pCSkillTData[sSkillNo][chSkillLv] = m_SkillTDataHeap.Get();
		if (!m_pCSkillTData[sSkillNo][chSkillLv])
			return nullptr;
		CSkillRecord* pCSkillRec;
		pCSkillRec = GetSkillRecordInfo(sSkillNo);
		if (!pCSkillRec)
			return nullptr;

		m_sSkillSetNo = sSkillNo;
		m_chSkillSetLv = chSkillLv;
		// SP消耗
		if (strcmp(pCSkillRec->szUseSP, "0")) {
			if (g_CParser.DoString(pCSkillRec->szUseSP, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->sUseSP = (Short)g_CParser.GetReturnNumber(0);
		} else
			m_pCSkillTData[sSkillNo][chSkillLv]->sUseSP = 0;
		// “耐久度“消耗
		if (strcmp(pCSkillRec->szUseEndure, "0")) {
			if (g_CParser.DoString(pCSkillRec->szUseEndure, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->sUseEndure = (Short)g_CParser.GetReturnNumber(0);
		} else
			m_pCSkillTData[sSkillNo][chSkillLv]->sUseEndure = 0;
		// “能量“消耗
		if (strcmp(pCSkillRec->szUseEnergy, "0")) {
			if (g_CParser.DoString(pCSkillRec->szUseEnergy, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->sUseEnergy = (Short)g_CParser.GetReturnNumber(0);
		} else
			m_pCSkillTData[sSkillNo][chSkillLv]->sUseEnergy = 0;
		// 技能区域
		m_pCSkillTData[sSkillNo][chSkillLv]->sRange[0] = enumRANGE_TYPE_NONE;
		if (strcmp(pCSkillRec->szSetRange, "0"))
			g_CParser.DoString(pCSkillRec->szSetRange, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END);
		// 技能状态
		m_pCSkillTData[sSkillNo][chSkillLv]->sStateParam[0] = SSTATE_NONE;
		if (strcmp(pCSkillRec->szRangeState, "0"))
			g_CParser.DoString(pCSkillRec->szRangeState, enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END);
		// 再施用间隔
		if (strcmp(pCSkillRec->szFireSpeed, "0")) {
			if (g_CParser.DoString(pCSkillRec->szFireSpeed, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END))
				m_pCSkillTData[sSkillNo][chSkillLv]->lResumeTime = g_CParser.GetReturnNumber(0);
		} else
			m_pCSkillTData[sSkillNo][chSkillLv]->lResumeTime = 0;
	}

	return m_pCSkillTData[sSkillNo][chSkillLv];
}

inline void CGameApp::SetSkillTDataRange(short* psRange) {
	if (m_sSkillSetNo < 0 || m_sSkillSetNo > defMAX_SKILL_NO)
		return;
	if (m_chSkillSetLv < 0 || m_chSkillSetLv > defMAX_SKILL_LV)
		return;

	memcpy(m_pCSkillTData[m_sSkillSetNo][m_chSkillSetLv]->sRange, psRange, sizeof(short) * defSKILL_RANGE_EXTEP_NUM);
}

inline void CGameApp::SetSkillTDataState(short* psState) {
	if (m_sSkillSetNo < 0 || m_sSkillSetNo > defMAX_SKILL_NO)
		return;
	if (m_chSkillSetLv < 0 || m_chSkillSetLv > defMAX_SKILL_LV)
		return;

	memcpy(m_pCSkillTData[m_sSkillSetNo][m_chSkillSetLv]->sStateParam, psState, sizeof(short) * defSKILL_STATE_PARAM_NUM);
}

// 初始化地表状态转移到角色身上所持续的时间
inline void CGameApp::InitSStateTraOnTime() {
	memset(m_lSStateTraOnTime, 0, sizeof(m_lSStateTraOnTime));
	CSkillStateRecord* pSStateR;
	for (int i = 1; i <= AREA_STATE_MAXID; i++) {
		pSStateR = GetCSkillStateRecordInfo(i);
		if (!pSStateR)
			continue;
		if (!strcmp(pSStateR->szOnTransfer, "0"))
			continue;
		for (int j = 1; j <= SKILL_STATE_LEVEL; j++) {
			if (g_CParser.DoString(pSStateR->szOnTransfer, enumSCRIPT_RETURN_NUMBER, 1, enumSCRIPT_PARAM_NUMBER, 1, j, DOSTRING_PARAM_END))
				m_lSStateTraOnTime[i][j] = g_CParser.GetReturnNumber(0);
		}
	}
}

inline long CGameApp::GetSStateTraOnTime(unsigned char uchStateID, unsigned char uchStateLv) {
	return m_lSStateTraOnTime[uchStateID][uchStateLv];
}

// 根据名称搜索本进程的所有角色
inline CCharacter* CGameApp::FindPlayerChaByName(const char* cszChaName) {
	BEGINGETGATE();
	CPlayer* pCPlayer;
	CCharacter* pCha = nullptr;
	GateServer* pGateServer;
	while (pGateServer = GETNEXTGATE()) {
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer*)GETNEXTPLAYER(pGateServer)) {
			if (++nCount > GETPLAYERCOUNT(pGateServer)) {
				//LG("玩家链表错误", "玩家数目:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				LG("player chain list error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByName");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();
			if (!pCha)
				continue;
			if (!strcmp(pCha->GetName(), cszChaName)) // 找到角色
				return pCha;
		}
	}

	return nullptr;
}

// 根据WorldID搜索本进程的所有角色
inline CCharacter* CGameApp::FindPlayerChaByID(unsigned long ulChaID) {
	BEGINGETGATE();
	CPlayer* pCPlayer;
	CCharacter* pCha = nullptr;
	GateServer* pGateServer;
	while (pGateServer = GETNEXTGATE()) {
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer*)GETNEXTPLAYER(pGateServer)) {
			if (++nCount > GETPLAYERCOUNT(pGateServer)) {
				//LG("玩家链表错误", "玩家数目:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				LG("player chain error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				break;
			}
			pCha = pCPlayer->GetCtrlCha();
			if (!pCha)
				continue;
			if (pCha->GetID() == ulChaID) // 找到角色
				return pCha;
		}
	}

	return nullptr;
}

// 根据数据ID搜索本进程的所有玩家
inline CPlayer* CGameApp::FindPlayerByDBChaID(unsigned long ulDBChaID) {
	BEGINGETGATE();
	CPlayer* pCPlayer;
	GateServer* pGateServer;
	while (pGateServer = GETNEXTGATE()) {
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer*)GETNEXTPLAYER(pGateServer)) {
			if (++nCount > GETPLAYERCOUNT(pGateServer)) {
				//LG("玩家链表错误", "玩家数目:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				LG("player chain error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				break;
			}
			if (pCPlayer->GetDBChaId() == ulDBChaID) // 找到角色
				return pCPlayer;
		}
	}

	return nullptr;
}

// 根据WorldID搜索本进程的所有主角色
inline CCharacter* CGameApp::FindMainPlayerChaByID(unsigned long ulChaID) {
	BEGINGETGATE();
	CPlayer* pCPlayer;
	CCharacter* pCha = nullptr;
	GateServer* pGateServer;
	while (pGateServer = GETNEXTGATE()) {
		if (!BEGINGETPLAYER(pGateServer))
			continue;
		int nCount = 0;
		while (pCPlayer = (CPlayer*)GETNEXTPLAYER(pGateServer)) {
			if (++nCount > GETPLAYERCOUNT(pGateServer)) {
				//LG("玩家链表错误", "玩家数目:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				LG("player chain error", "player number:%u, %s\n", GETPLAYERCOUNT(pGateServer), "FindPlayerChaByID");
				break;
			}
			pCha = pCPlayer->GetMainCha();
			if (!pCha)
				continue;
			if (pCha->GetID() == ulChaID) // 找到角色
				return pCha;
		}
	}

	return nullptr;
}

// 根据WorldID搜索本进程的所有角色
inline CCharacter* CGameApp::FindChaByID(unsigned long ulChaID) {
	CCharacter* pCCha;
	m_pCEntSpace->BeginGetCha();
	while (pCCha = m_pCEntSpace->GetNextCha()) {
		if (pCCha->GetID() == ulChaID)
			return pCCha;
	}

	return nullptr;
}

inline CCharacter* CGameApp::FindChaByName(const char* cszChaName) {
	CCharacter* pCCha;
	m_pCEntSpace->BeginGetCha();
	while (pCCha = m_pCEntSpace->GetNextCha()) {
		if (!strcmp(pCCha->GetName(), cszChaName))
			return pCCha;
	}

	return nullptr;
}

enum EChaTimerAction {
	enumCHA_TIMEER_ENTERMAP,
};

struct SSwitchMapInfo // 地图切换信息
{
	SubMap* pSrcMap;
	char szSrcMapName[256];
	Point SSrcPos;
	char szTarMapName[256];
	Point STarPos;
};

extern bool g_bLogEntity;

extern CGameApp* g_pGameApp;
extern CItemRecordAttr* g_pCItemAttr;
extern CCharacter* g_pCSystemCha;	// 系统角色
extern SubMap* g_pScriptMap;		 // 脚本用初始化地图信息全局变量
extern long g_lDeftMMaskLight;		 // 大地图默认照亮范围
extern std::string g_strChaState[2]; // 0，存放主角色的技能状态。1，存放船的技能状态
extern uLong g_ulCurID;
extern Long g_lCurHandle;

#define defINVALID_CHA_ID 0
#define defINVALID_CHA_HANDLE -1

#endif // GAMEAPP_H
