#include "Util.h"
#include "GameAppNet.h"
#include "Player.h"
#include "atltime.h"

#define defCHA_TABLE_VER 110
#define defCHA_TABLE_NEW_VER 111
#define defRESET_CRYSTAL_TIME 60 * 60 * 24 * 1000

enum ESaveType {
	enumSAVE_TYPE_OFFLINE, // 下线
	enumSAVE_TYPE_SWITCH,  // 地图切换
	enumSAVE_TYPE_TIMER,   // 定时保存
	enumSAVE_TYPE_TRADE,   // 交易
};

class CPlayer;
class CTableCha : public cfl_rs {
public:
	CTableCha(cfl_db* pDB)
		: cfl_rs(pDB, "character", 100) {
		try {
		}
		T_E
	}

	bool Init();
	bool ReadAllData(CPlayer* pPlayer, DWORD cha_id);   // 角色进入游戏读盘
	bool ReadAllDataEx(CPlayer* pPlayer, DWORD cha_id); // 角色进入游戏读盘

	void OnlySavePosWhenBeSaved(CPlayer* pPlayer);   // 需要保存位置信息时
	void OnlySavePosWhenBeSavedEx(CPlayer* pPlayer); // 需要保存位置信息时

	bool SaveAllData(CPlayer* pPlayer, char chSaveType, cChar* szTarMapName = "", Long lTarX = 0, Long lTarY = 0); // 角色存盘

	bool SaveAllDataEx(CPlayer* pPlayer, char chSaveType); // 角色存盘（不使用全局变量版本）

	bool SavePos(CPlayer* pPlayer);   // 保存角色位置
	bool SavePosEx(CPlayer* pPlayer); // 保存角色位置

	bool SaveMoney(CPlayer* pPlayer);
	bool SaveKBagDBID(CPlayer* pPlayer);
	bool SaveKBagDBIDEx(CPlayer* pPlayer);

	bool SaveKBagTmpDBID(CPlayer* pPlayer);   // 保存临时背包ID
	bool SaveKBagTmpDBIDEx(CPlayer* pPlayer); // 保存临时背包ID

	bool SaveKBState(CPlayer* pPlayer); // 保存密码锁定状态
	bool SaveMMaskDBID(CPlayer* pPlayer);
	bool SaveBankDBID(CPlayer* pPlayer);
	bool SaveBankDBIDEx(CPlayer* pPlayer);

	bool SaveTableVer(DWORD cha_id);					  // 保存表的版本
	BOOL SaveMissionData(CPlayer* pPlayer, DWORD cha_id); // 角色任务信息存盘
	bool VerifyName(const char* pszName);				  // 角色名是否存在

	BOOL AddCreditByDBID(DWORD cha_id, long lCredit);
	BOOL IsChaOnline(DWORD cha_id, BOOL& bOnline);
	BOOL SaveStoreItemID(DWORD cha_id, long lStoreItemID);
	BOOL AddMoney(DWORD cha_id, long money);
	BOOL UpdateEnterMapTime(DWORD cha_id);					//Add by sunny.sun 20090310 记录角色进地图时间
	BOOL UpdateOutMapTime(DWORD cha_id);					// 记录角色出地图时间
	BOOL CalculateDisTime(CCharacter* pCCha, int& distime); //Add by sunny.sun 20090311
	BOOL ConvertCTime(std::string stime, CTime& ctime);
	BOOL GetDisTime(CCharacter* pCCha, int& distime);
	BOOL UpdateDisTime(CCharacter* pCCha, int iSelTime);
	BOOL SetDisTime(CCharacter* pCCha, int iSelTime);
	BOOL GetdwFlatMoney(const char* cha_name, DWORD& FlatMoney); //Add by sunny.sun 20090414取平台帐户
	BOOL SetdwFlatMoney(DWORD cha_tid, DWORD FlatMoney);
};

class CTableMaster : public cfl_rs {
public:
	CTableMaster(cfl_db* pDB)
		: cfl_rs(pDB, "master", 6) {
		try {
		}
		T_E
	}

	bool Init();
	unsigned long GetMasterDBID(CPlayer* pPlayer);
	bool IsMasterRelation(int masterID, int prenticeID);
};

// 此值和数据库对应，不可改动
enum ResDBTypeID {
	enumRESDB_TYPE_LOOK,	  // 外观
	enumRESDB_TYPE_KITBAG,	// 道具栏
	enumRESDB_TYPE_BANK,	  // 银行
	enumRESDB_TYPE_KITBAGTMP, //临时背包
};

// Add by lark.li 20080521 begin
enum IssueState {
	enumCURRENT = 0, // 当前期
	enumPASTDUE = 1, // 过期
	enumDISUSE = 2,  // 废弃
};

// 彩票设定
class CTableLotterySetting : public cfl_rs {
public:
	CTableLotterySetting(cfl_db* pDB)
		: cfl_rs(pDB, "LotterySetting", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool GetCurrentIssue(int& issue);
	bool AddIssue(int issue);
	bool DisuseIssue(int issue, int state);
	bool SetWinItemNo(int issue, const char* itemno);
	bool GetWinItemNo(int issue, std::string& itemno);
};

// 彩票保存
class CTableTicket : public cfl_rs {
public:
	CTableTicket(cfl_db* pDB)
		: cfl_rs(pDB, "Ticket", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool AddTicket(int cha_id, int issue, char itemno[6][2]);
	bool IsExist(int issue, char* itemno);
	bool CalWinTicket(int issue, int max, std::string& itemno);

private:
	bool AddTicket(int cha_id, int issue, char itemno1, char itemno2, char itemno3, char itemno4, char itemno5, char itemno6, int real = 1);
};

// 中奖号码保存
class CTableWinTicket : public cfl_rs {
public:
	CTableWinTicket(cfl_db* pDB)
		: cfl_rs(pDB, "WinTicket", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool GetTicket(int issue);
	bool AddTicket(int issue, char* itemno, int grade);
	bool Exchange(int issue, char* itemno);
};

struct AmphitheaterSetting {
	enum AmphitheaterSateSetting {
		enumCURRENT = 0,
	};
};
//Add by sunny.sun 20080725
struct AmphitheaterTeam {
	enum AmphitheaterSateTeam {
		enumNotUse = 0,	//没注册
		enumUse = 1,	   // 正常
		enumPromotion = 2, // 晋级
		enumRelive = 3,	// 复活
		enumOut = 4,	   // 淘汰
	};
};

// 竞技场设定信息保存
// 保存 当前是第几赛季 第几轮次 等信息
class CTableAmphitheaterSetting : public cfl_rs {
public:
	CTableAmphitheaterSetting(cfl_db* pDB)
		: cfl_rs(pDB, "AmphitheaterSetting", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool GetCurrentSeason(int& season, int& round);
	bool AddSeason(int season);
	bool DisuseSeason(int season, int state, const char* winner);
	bool UpdateRound(int season, int round);
};

// 竞技场参赛队伍信息保存
class CTableAmphitheaterTeam : public cfl_rs {
public:
	CTableAmphitheaterTeam(cfl_db* pDB)
		: cfl_rs(pDB, "AmphitheaterTeam", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool GetTeamCount(int& count);
	bool GetNoUseTeamID(int& teamID);
	bool TeamSignUP(int& teamID, int captain, int member1, int member2);
	bool TeamCancel(int teamID);

	bool TeamUpdate(int teamID, int matchNo, int state, int winnum, int losenum, int relivenum);
	bool IsValidAmphitheaterTeam(int teamID, int captainID, int member1, int member2);
	bool IsLogin(int pActorID); //Add by sunny.sun20080714
	bool IsMapFull(int MapID, int& PActorIDNum);
	bool UpdateMapNum(int Teamid, int Mapid, int MapFlag);
	bool SetMaxBallotTeamRelive();
	bool SetMatchResult(int Teamid1, int Teamid2, int Id1state, int Id2state);
	bool GetMapFlag(int Teamid, int& Mapflag);
	bool GetCaptainByMapId(int Mapid, std::string& Captainid1, std::string& Captainid2);
	bool UpdateMap(int Mapid);

	bool GetPromotionAndReliveTeam(std::vector<std::vector<std::string>>& dataPromotion, std::vector<std::vector<std::string>>& dataRelive);
	bool UpdatReliveNum(int ReID);
	bool UpdateAbsentTeamRelive();
	bool UpdateMapAfterEnter(int CaptainID, int MapID);
	bool UpdateWinnum(int teamid); //Add by sunnysun20080818
	bool GetUniqueMaxWinnum(int& teamid);
	bool SetMatchnoState(int teamid);
	bool UpdateState();
	bool CloseReliveByState(int& statenum);
	bool CleanMapFlag(int teamid1, int teamid2);
	bool GetStateByTeamid(int teamid, int& state);
};

// End

//Add by sunny.sun 20080822
//Begin
class CTablePersoninfo : public cfl_rs {
public:
	CTablePersoninfo(cfl_db* pDB)
		: cfl_rs(pDB, "personinfo", 10) {
		try {
		}
		T_E
	}
	bool Init();
	bool GetPersonBirthday(int chaid, int& birthday);
};
struct CrystalTradeType {
	enum TradeType {
		enumbuy = 0, //买
		enumsale = 1 //卖
	};
	enum TradeState {
		enumstart = 0,
		enumhang = 1,
		enumend = 2,
		enumstop = 3
	};
};

#ifdef SHUI_JING
class CTableCrystalTrade : public cfl_rs {
public:
	CTableCrystalTrade(cfl_db* pDB)
		: cfl_rs(pDB, "CrystalTrade", 10) {
		try {
		}
		T_E
	}
	BOOL Init(void);
	BOOL GetCrystalBuyAndSaleList(std::vector<std::vector<std::string>>& dataBuyTrade, std::vector<std::vector<std::string>>& dataSaleTrade);
	BOOL GetChaBuyAndSale(CCharacter* pCCha, std::vector<std::vector<std::string>>& chaTradeData);
	BOOL IsHasBuyorSale(DWORD cha_id, CrystalTradeType::TradeType type);
	BOOL CheckCrystalExchangeMoney(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum, CrystalTradeType::TradeType type);
	BOOL HangBuyTrade(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum);
	BOOL HangSaleTrade(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum);
	BOOL CrystalTradeCancel(CCharacter* pCCha, int type, DWORD& iCslNum, DWORD& iCslPrice);
	BOOL CheckHasAccordForTrade(DWORD& chaid, std::string& act_tName, DWORD iCslPrice, DWORD iCslNum, DWORD& dcslNum, CrystalTradeType::TradeType type);
	BOOL CheckHasMatchAccord(DWORD& chaid, std::string& act_tName, DWORD iCslPrice, DWORD iCslNum, CrystalTradeType::TradeType type);
	//有交易信息和玩家信息 修改玩家和交易者的挂单
	BOOL UpdateHangTrade(char* actname, DWORD cha_id, DWORD dtradeMoney, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state);
	BOOL UpdateHangTradeForFailed(char* actname, DWORD cha_id, DWORD dtradeMoney, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state);
	BOOL GetHangNum(CrystalTradeType::TradeType type, int& hangNum); //得到挂单人数
	BOOL ResetCrystalState(std::vector<std::vector<std::string>>& dataHangTrade);
	BOOL ChangeStateByTemp(DWORD cha_id, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state);
	BOOL GetCrystalState(CCharacter* pCha, CrystalTradeType::TradeType type, CrystalTradeType::TradeState& state, DWORD& dwNum, DWORD& dwPrice, DWORD& totalnum); //获得玩家当前交易挂单状态
	BOOL ResetCrystalTotalNum(CCharacter* pCha, DWORD dwNum, CrystalTradeType::TradeType type);																	  //设置总数为当前个数
};
#endif
//End

class CTableResource : public cfl_rs {
public:
	CTableResource(cfl_db* pDB)
		: cfl_rs(pDB, "resource", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool Create(long& lDBID, long lChaId, long lTypeId);
	bool ReadKitbagData(CCharacter* pCCha);
	bool ReadKitbagDataEx(CCharacter* pCCha);

	bool SaveKitbagData(CCharacter* pCCha);
	bool SaveKitbagDataEx(CCharacter* pCCha);

	bool ReadKitbagTmpData(CCharacter* pCCha);
	bool ReadKitbagTmpDataEx(CCharacter* pCCha);

	bool SaveKitbagTmpData(CCharacter* pCCha);
	bool SaveKitbagTmpDataEx(CCharacter* pCCha);

	bool ReadKitbagTmpData(long lRecDBID, std::string& strData);
	bool SaveKitbagTmpData(long lRecDBID, const std::string& strData);
	bool ReadBankData(CPlayer* pCPly, char chBankNO = -1);
	bool ReadBankDataEx(CPlayer* pCPly, char chBankNO = -1);

	bool SaveBankData(CPlayer* pCPly, char chBankNO = -1);
	bool SaveBankDataEx(CPlayer* pCPly, char chBankNO = -1);
};

class CTableMapMask : public cfl_rs {
public:
	CTableMapMask(cfl_db* pDB)
		: cfl_rs(pDB, "map_mask", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool Create(long& lDBID, long lChaId);
	bool ReadData(CPlayer* pCPly);
	bool SaveData(CPlayer* pCPly, BOOL bDirect = FALSE);
	bool GetColNameByMapName(const char* szMapName, char* szColName, long lColNameLen);

	void HandleSaveList();
	void AddSaveQuest(const char* pszSQL);
	void SaveAll();

protected:
	BOOL _ExecSQL(const char* pszSQL);
	std::list<std::string> _SaveMapMaskList; // 保存大地图的队列
};

void inline CTableMapMask::AddSaveQuest(const char* pszSQL) {
	_SaveMapMaskList.emplace_back(pszSQL);
}

class CTableAct : public cfl_rs {
public:
	CTableAct(cfl_db* pDB)
		: cfl_rs(pDB, "account", 10) {
		try {
		}
		T_E
	}

	bool Init();
	bool ReadAllData(CPlayer* pPlayer, DWORD act_id);
};

class CTableBoat : public cfl_rs {
public:
	CTableBoat(cfl_db* pDB)
		: cfl_rs(pDB, "boat", 100) {
		try {
		}
		T_E
	}

	bool Init();
	BOOL Create(DWORD& dwBoatID, const BOAT_DATA& Data);
	BOOL GetBoat(CCharacter& Boat);
	BOOL SaveBoat(CCharacter& Boat, char chSaveType);
	BOOL SaveBoatEx(CCharacter& Boat, char chSaveType);

	BOOL SaveBoatTempData(CCharacter& Boat, BYTE byIsDeleted = 0);
	BOOL SaveBoatTempData(DWORD dwBoatID, DWORD dwOwnerID, BYTE byIsDeleted = 0);
	BOOL SaveBoatDelTag(DWORD dwBoatID, BYTE byIsDeleted = 0);

	bool SaveAllData(CPlayer* pPlayer, char chSaveType);
	bool SaveAllDataEx(CPlayer* pPlayer, char chSaveType);

	bool ReadCabin(CCharacter& Boat);				   // 读取船舱
	bool SaveCabin(CCharacter& Boat, char chSaveType); // 保存船舱
	bool SaveAllCabin(CPlayer* pPlayer, char chSaveType);
};

class CTableGuild : public cfl_rs {
public:
	CTableGuild(cfl_db* pDB)
		: cfl_rs(pDB, "guild", 100) {
		try {
		}
		T_E
	}

	bool Init();
	long Create(CCharacter* pCha, char guildtype, char* guildname, cChar* passwd);
	bool ListAll(CCharacter* pCha, char guildtype, char disband_days);
	void TryFor(CCharacter* pCha, uLong guildid);
	void TryForConfirm(CCharacter* pCha, uLong guildid);
	char GetTypeByID(uLong guildid);
	bool GetGuildInfo(CCharacter* pCha, uLong guildid);
	bool ListTryPlayer(CCharacter* pCha, char disband_days);
	bool Approve(CCharacter* pCha, uLong chaid);
	bool Reject(CCharacter* pCha, uLong chaid);
	bool Kick(CCharacter* pCha, uLong chaid);
	bool Leave(CCharacter* pCha);
	bool Disband(CCharacter* pCha, cChar* passwd);
	bool Motto(CCharacter* pCha, cChar* motto);
	bool GetGuildName(long lGuildID, std::string& strGuildName);
	bool CancelTryFor(CCharacter* pCha); //Add by sunny.sun 20090706 for 公会邀请

	// 公会挑战
	bool Challenge(CCharacter* pCha, BYTE byLevel, DWORD dwMoney);
	bool Leizhu(CCharacter* pCha, BYTE byLevel, DWORD dwMoney);
	void ListChallenge(CCharacter* pCha);
	bool GetChallInfo(BYTE byLevel, DWORD& dwGuildID1, DWORD& dwGuildID2, DWORD& dwMoney);
	bool StartChall(BYTE byLevel);
	bool HasCall(BYTE byLevel);
	void EndChall(DWORD dwGuild1, DWORD dwGuild2, BOOL bChall);
	void ChallMoney(BYTE byLevel, BOOL bChall, DWORD dwGuildID, DWORD dwChallID, DWORD dwMoney);
	bool ChallWin(BOOL bUpdate, BYTE byLevel, DWORD dwWinGuildID, DWORD dwFailerGuildID);
	bool HasGuildLevel(CCharacter* pChar, BYTE byLevel);
};

// Log专用表
class CTableLog : public cfl_rs {
public:
	CTableLog(cfl_db* pDB)
		: cfl_rs(pDB, "gamelog", 10) {
		try {
		}
		T_E
	}
};

//	2008-7-28	yyy	add	begin!
//	增加道具的唯一ID表，用于道具锁定。
class CTableItem : public cfl_rs {
public:
	CTableItem(cfl_db* pDB)
		: cfl_rs(pDB, "property", 10) {
		try {
		}
		T_E
	}

	//	锁定道具。
	bool LockItem(SItemGrid* sig, int iChaId);
	bool UnlockItem(SItemGrid* sig, int iChaId); // 道具解锁 add by ning.yan  2008-11-12
};
//	2008-7-28	yyy	add	end!

class CGameDB {
public:
	CGameDB()
#ifdef SHUI_JING
		: _connect(), _tab_cha(NULL), _tab_master(NULL), _tab_act(NULL), _tab_gld(NULL), _tab_boat(NULL), _tab_log(NULL), _tab_item(NULL), _tab_csl_trade(NULL), m_bInitOK(FALSE)
#else
		: _connect(), _tab_cha(nullptr), _tab_master(nullptr), _tab_act(nullptr), _tab_gld(nullptr), _tab_boat(nullptr), _tab_log(nullptr), _tab_item(nullptr), m_bInitOK(FALSE)
#endif
	{
		try {
		}
		T_E
	}

	~CGameDB() {
		try {
			if (_tab_cha != nullptr) {
				delete _tab_cha;
				_tab_cha = nullptr;
			}
			if (_tab_act != nullptr) {
				delete _tab_act;
				_tab_act = nullptr;
			}
			if (_tab_gld != nullptr) {
				delete _tab_gld;
				_tab_gld = nullptr;
			}
			if (_tab_master != nullptr) {
				delete _tab_master;
				_tab_master = nullptr;
			}
			SAFE_DELETE(_tab_per_info);
#ifdef SHUI_JING
			SAFE_DELETE(_tab_csl_trade);
#endif
			SAFE_DELETE(_tab_boat);
			SAFE_DELETE(_tab_log);
			SAFE_DELETE(_tab_item);
			_connect.disconn();
		}
		T_E
	}

	BOOL Init();

	bool BeginTran() {
		return _connect.begin_tran();
	}

	bool RollBack() {
		return _connect.rollback();
	}

	bool CommitTran() {
		return _connect.commit_tran();
	}

	bool ReadPlayer(CPlayer* pPlayer, DWORD cha_id);

	bool ReadPlayerEx(CPlayer* pPlayer, DWORD cha_id);

	bool SavePlayer(CPlayer* pPlayer, char chSaveType, cChar* szTarMapName = "", Long lTarX = 0, Long lTarY = 0);

	bool SavePlayerEx(CPlayer* pPlayer, char chSaveType);

	bool SaveChaAssets(CCharacter* pCCha);

	// Add by lark.li 20080527 begin
	bool GetWinItemno(int issue, std::string& itemno) {
		bool flag = true;

		m_Lock.Lock();

		try {
			flag = this->_tab_setting->GetWinItemNo(issue, itemno);
		} catch (...) {
			flag = false;
		}

		m_Lock.Unlock();
		return flag;
	}

	bool CalWinTicket(int issue, int max, std::string& itemno) {
		m_Lock.Lock();

		try {
			if (!this->_tab_ticket->CalWinTicket(issue, max, itemno)) {
				m_Lock.Unlock();
				return false;
			}

			if (!this->_tab_setting->SetWinItemNo(issue, itemno.c_str())) {
				m_Lock.Unlock();
				return false;
			}

			//this->DisuseIssue(issue, 1);
			if (!this->AddIssue(issue + 1)) {
				m_Lock.Unlock();
				return false;
			}
		} catch (...) {
			m_Lock.Unlock();
			return false;
		}

		m_Lock.Unlock();
		return true;
	}

	bool GetLotteryIssue(int& issue) {
		bool flag = true;
		m_Lock.Lock();

		try {
			flag = this->_tab_setting->GetCurrentIssue(issue);
		} catch (...) {
			flag = false;
		}

		m_Lock.Unlock();
		return flag;
	}

	bool LotteryIsExsit(int issue, char* itemno) {
		bool flag = true;
		m_Lock.Lock();

		try {
			flag = this->_tab_ticket->IsExist(issue, itemno);
		} catch (...) {
			flag = false;
		}

		m_Lock.Unlock();
		return flag;
	}

	bool AddLotteryTicket(CCharacter* pCCha, int issue, char itemno[6][2]) {
		bool flag = true;
		m_Lock.Lock();

		try {
			flag = this->_tab_ticket->AddTicket(pCCha->m_ID, issue, itemno);
		} catch (...) {
			flag = false;
		}

		m_Lock.Unlock();
		return flag;
	}

	bool AddIssue(int issue) {
		bool flag = true;
		m_Lock.Lock();

		try {
			flag = this->_tab_setting->AddIssue(issue);
		} catch (...) {
			flag = false;
		}

		m_Lock.Unlock();
		return flag;
	}

	bool DisuseIssue(int issue, int state) {
		bool flag = true;
		m_Lock.Lock();

		try {
			flag = this->_tab_setting->DisuseIssue(issue, state);
		} catch (...) {
			flag = false;
		}

		m_Lock.Unlock();
		return flag;
	}

	bool IsValidAmphitheaterTeam(int teamID, int captainID, int member1, int member2) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->IsValidAmphitheaterTeam(teamID, captainID, member1, member2);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 师徒关系判断
	bool IsMasterRelation(int masterID, int prenticeID) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_master->IsMasterRelation(masterID, prenticeID);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 取得当前赛季 和 轮次s
	bool GetAmphitheaterSeasonAndRound(int& season, int& round) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_setting->GetCurrentSeason(season, round);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 追加赛季
	bool AddAmphitheaterSeason(int season) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_setting->AddSeason(season);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 更新赛季状态
	bool DisuseAmphitheaterSeason(int season, int state, const char* winner) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_setting->DisuseSeason(season, state, winner);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 更新轮次
	bool UpdateAmphitheaterRound(int season, int round) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_setting->UpdateRound(season, round);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 获取队伍个数
	bool GetAmphitheaterTeamCount(int& count) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->GetTeamCount(count);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 获取队伍ID
	bool GetAmphitheaterNoUseTeamID(int& teamID) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->GetNoUseTeamID(teamID);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 注册
	bool AmphitheaterTeamSignUP(int& teamID, int captain, int member1, int member2) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->TeamSignUP(teamID, captain, member1, member2);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	// 注销
	bool AmphitheaterTeamCancel(int teamID) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->TeamCancel(teamID);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	//Add by sunny.sun20080714
	//查询该角色ID是否已经注册
	bool IsAmphitheaterLogin(int pActorID) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->IsLogin(pActorID);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	//判断是否地图队伍已满
	bool IsMapFull(int MapID, int& PActorIDNum) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->IsMapFull(MapID, PActorIDNum);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	//更新地图队伍mapflag
	bool UpdateMapNum(int Teamid, int Mapid, int MapFlag) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->UpdateMapNum(Teamid, Mapid, MapFlag);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool GetMapFlag(int Teamid, int& Mapflag) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->GetMapFlag(Teamid, Mapflag);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool SetMaxBallotTeamRelive() {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->SetMaxBallotTeamRelive();
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool SetMatchResult(int Teamid1, int Teamid2, int Id1state, int Id2state) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->SetMatchResult(Teamid1, Teamid2, Id1state, Id2state);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool GetCaptainByMapId(int Mapid, std::string& Captainid, std::string& Captainid2) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->GetCaptainByMapId(Mapid, Captainid, Captainid2);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool UpdateMap(int Mapid) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->UpdateMap(Mapid);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool UpdateMapAfterEnter(int CaptainID, int MapID) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->UpdateMapAfterEnter(CaptainID, MapID);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool GetPromotionAndReliveTeam(std::vector<std::vector<std::string>>& dataPromotion, std::vector<std::vector<std::string>>& dataRelive) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->GetPromotionAndReliveTeam(dataPromotion, dataRelive);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool UpdatReliveNum(int ReID) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->UpdatReliveNum(ReID);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool UpdateAbsentTeamRelive() {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->UpdateAbsentTeamRelive();
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool UpdateWinnum(int teamid) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->UpdateWinnum(teamid);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool GetUniqueMaxWinnum(int& teamid) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->GetUniqueMaxWinnum(teamid);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool SetMatchnoState(int teamid) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->SetMatchnoState(teamid);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	bool UpdateState() {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->UpdateState();
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool CloseReliveByState(int& statenum) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->CloseReliveByState(statenum);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool CleanMapFlag(int teamid1, int teamid2) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->CleanMapFlag(teamid1, teamid2);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	bool GetStateByTeamid(int teamid, int& state) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_amp_team->GetStateByTeamid(teamid, state);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool GetPersonBirthday(int chaid, int& birthday) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = this->_tab_per_info->GetPersonBirthday(chaid, birthday);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	// End

	bool SavePlayerPos(CPlayer* pPlayer) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SavePos(pPlayer);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool SavePlayerPosEx(CPlayer* pPlayer) {
		bool ret = false;
		m_LockEx.Lock();

		try {
			ret = _tab_cha->SavePosEx(pPlayer);
		} catch (...) {
			ret = false;
		}

		m_LockEx.Unlock();
		return ret;
	}

	void OnlySavePosWhenBeSaved(CPlayer* pPlayer) {
		m_Lock.Lock();

		try {
			_tab_cha->OnlySavePosWhenBeSaved(pPlayer);
		} catch (...) {
		}

		m_Lock.Unlock();
	}

	void OnlySavePosWhenBeSavedEx(CPlayer* pPlayer) {
		m_LockEx.Lock();

		try {
			_tab_cha->OnlySavePosWhenBeSavedEx(pPlayer);
		} catch (...) {
		}

		m_LockEx.Unlock();
	}

	bool SavePlayerKBagDBID(CPlayer* pPlayer) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SaveKBagDBID(pPlayer);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool SavePlayerKBagDBIDEx(CPlayer* pPlayer) {
		bool ret = false;
		m_LockEx.Lock();

		try {
			ret = _tab_cha->SaveKBagDBIDEx(pPlayer);
		} catch (...) {
			ret = false;
		}

		m_LockEx.Unlock();
		return ret;
	}

	bool SavePlayerKBagTmpDBID(CPlayer* pPlayer) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SaveKBagTmpDBID(pPlayer);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	bool SavePlayerKBagTmpDBIDEx(CPlayer* pPlayer) {
		bool ret = false;
		m_LockEx.Lock();

		try {
			ret = _tab_cha->SaveKBagTmpDBIDEx(pPlayer);
		} catch (...) {
			ret = false;
		}

		m_LockEx.Unlock();
		return ret;
	}

	bool SavePlayerMMaskDBID(CPlayer* pPlayer) {
		bool ret = false;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SaveMMaskDBID(pPlayer);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	bool CreatePlyBank(CPlayer* pCPly) {
		bool ret = true;
		m_Lock.Lock();

		try {
			if (pCPly->GetCurBankNum() >= MAX_BANK_NUM) {
				m_Lock.Unlock();
				return false;
			}

			long lBankDBID;
			if (!_tab_res->Create(lBankDBID, pCPly->GetDBChaId(), enumRESDB_TYPE_BANK)) {
				m_Lock.Unlock();
				return false;
			}

			pCPly->AddBankDBID(lBankDBID);
			if (!_tab_cha->SaveBankDBID(pCPly)) {
				m_Lock.Unlock();
				return false;
			}
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	bool SavePlyBank(CPlayer* pCPly, char chBankNO = -1) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_res->SaveBankData(pCPly, chBankNO);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}

	unsigned long GetPlayerMasterDBID(CPlayer* pPlayer) {
		unsigned long ret = 0;
		m_Lock.Lock();

		try {
			ret = _tab_master->GetMasterDBID(pPlayer);
		} catch (...) {
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL AddCreditByDBID(DWORD cha_id, long lCredit) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->AddCreditByDBID(cha_id, lCredit);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL SaveStoreItemID(DWORD cha_id, long lStoreItemID) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SaveStoreItemID(cha_id, lStoreItemID);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL AddMoney(DWORD cha_id, long money) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->AddMoney(cha_id, money);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL ReadKitbagTmpData(DWORD res_id, std::string& strData) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_res->ReadKitbagTmpData(res_id, strData);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL SaveKitbagTmpData(DWORD res_id, const std::string& strData) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_res->SaveKitbagTmpData(res_id, strData);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL IsChaOnline(DWORD cha_id, BOOL& bOnline) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->IsChaOnline(cha_id, bOnline);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL SaveMissionData(CPlayer* pPlayer, DWORD cha_id) // 角色任务信息存盘
	{
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SaveMissionData(pPlayer, cha_id);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	//Add by sunny.sun 20090310
	BOOL UpdateEnterMapTime(DWORD cha_id) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->UpdateEnterMapTime(cha_id);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL UpdateOutMapTime(DWORD cha_id) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->UpdateOutMapTime(cha_id);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL CalculateDisTime(CCharacter* pCCha, int& distime) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->CalculateDisTime(pCCha, distime);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL GetDisTime(CCharacter* pCCha, int& distime) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->GetDisTime(pCCha, distime);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL UpdateDisTime(CCharacter* pCCha, int iSelTime) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->UpdateDisTime(pCCha, iSelTime);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL SetDisTime(CCharacter* pCCha, int iSelTime) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SetDisTime(pCCha, iSelTime);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
#ifdef SHUI_JING
	BOOL GetdwFlatMoney(const char* cha_name, DWORD& FlatMoney) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->GetdwFlatMoney(cha_name, FlatMoney);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL SetdwFlatMoney(DWORD cha_tid, DWORD FlatMoney) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_cha->SetdwFlatMoney(cha_tid, FlatMoney);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	////取平台帐户
	//BOOL	GetChadwFlatMoney( CCharacter* pCCha,  DWORD& FlatMoney )
	//{try {
	//	return _tab_cha->GetChadwFlatMoney( pCCha, FlatMoney );
	//} T_E}

	//水晶兑换
	BOOL GetCrystalBuyAndSaleList(std::vector<std::vector<std::string>>& dataBuyTrade, std::vector<std::vector<std::string>>& dataSaleTrade) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->GetCrystalBuyAndSaleList(dataBuyTrade, dataSaleTrade);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL GetChaBuyAndSale(CCharacter* pCCha, std::vector<std::vector<std::string>>& chaTradeData) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->GetChaBuyAndSale(pCCha, chaTradeData);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL IsHasBuyorSale(DWORD cha_id, CrystalTradeType::TradeType type) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->IsHasBuyorSale(cha_id, type);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL CheckCrystalExchangeMoney(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum, CrystalTradeType::TradeType type) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->CheckCrystalExchangeMoney(pCCha, iCslPrice, iCslNum, type);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL HangBuyTrade(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->HangBuyTrade(pCCha, iCslPrice, iCslNum);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL HangSaleTrade(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->HangSaleTrade(pCCha, iCslPrice, iCslNum);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL CrystalTradeCancel(CCharacter* pCCha, int type, DWORD& iCslNum, DWORD& iCslPrice) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->CrystalTradeCancel(pCCha, type, iCslNum, iCslPrice);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL CheckHasAccordForTrade(DWORD& chaid, std::string& act_tName, DWORD iCslPrice, DWORD iCslNum, DWORD& dcslnum, CrystalTradeType::TradeType type) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->CheckHasAccordForTrade(chaid, act_tName, iCslPrice, iCslNum, dcslnum, type);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL CheckHasMatchAccord(DWORD& chaid, std::string& act_tName, DWORD iCslPrice, DWORD iCslNum, CrystalTradeType::TradeType type) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->CheckHasMatchAccord(chaid, act_tName, iCslPrice, iCslNum, type);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	BOOL UpdateHangTrade(char* actname, DWORD cha_id, DWORD dtradeMoney, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->UpdateHangTrade(actname, cha_id, dtradeMoney, type, state);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL UpdateHangTradeForFailed(char* actname, DWORD cha_id, DWORD dtradeMoney, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->UpdateHangTradeForFailed(actname, cha_id, dtradeMoney, type, state);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	BOOL GetHangNum(CrystalTradeType::TradeType type, int& hangNum) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->GetHangNum(type, hangNum);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL ResetCrystalState(std::vector<std::vector<std::string>>& dataHangTrade) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->ResetCrystalState(dataHangTrade);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL ChangeStateByTemp(DWORD cha_id, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->ChangeStateByTemp(cha_id, type, state);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL GetCrystalState(CCharacter* pCha, CrystalTradeType::TradeType type, CrystalTradeType::TradeState& state, DWORD& dwNum, DWORD& dwPrice, DWORD& totalnum) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->GetCrystalState(pCha, type, state, dwNum, dwPrice, totalnum);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

	BOOL ResetCrystalTotalNum(CCharacter* pCha, DWORD dwNum, CrystalTradeType::TradeType type) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_csl_trade->ResetCrystalTotalNum(pCha, dwNum, type);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}

#endif
	// 船只存储
	BOOL Create(DWORD& dwBoatID, const BOAT_DATA& Data) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_boat->Create(dwBoatID, Data);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	BOOL GetBoat(CCharacter& Boat) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_boat->GetBoat(Boat);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	BOOL SaveBoat(CCharacter& Boat, char chSaveType) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_boat->SaveBoat(Boat, chSaveType);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	BOOL SaveBoatDelTag(DWORD dwBoatID, BYTE byIsDeleted = 0) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_boat->SaveBoatDelTag(dwBoatID, byIsDeleted);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	BOOL SaveBoatTempData(CCharacter& Boat, BYTE byIsDeleted = 0) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_boat->SaveBoatTempData(Boat, byIsDeleted);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	BOOL SaveBoatTempData(DWORD dwBoatID, DWORD dwOwnerID, BYTE byIsDeleted = 0) {
		BOOL ret = TRUE;
		m_Lock.Lock();

		try {
			ret = _tab_boat->SaveBoatTempData(dwBoatID, dwOwnerID, byIsDeleted);
		} catch (...) {
			ret = FALSE;
		}

		m_Lock.Unlock();
		return ret;
	}
	long CreateGuild(CCharacter* pCha, char guildtype, char* guildname, cChar* passwd) {
		long ret = -1;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Create(pCha, guildtype, guildname, passwd);
		} catch (...) {
		}

		m_Lock.Unlock();
		return ret;
	}
	bool ListAllGuild(CCharacter* pCha, char guildtype, char disband_days = 1) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->ListAll(pCha, guildtype, disband_days);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();
		return ret;
	}
	void GuildTryFor(CCharacter* pCha, uLong guildid) {
		m_Lock.Lock();

		try {
			_tab_gld->TryFor(pCha, guildid);
		} catch (...) {
		}

		m_Lock.Unlock();
	}
	void GuildTryForConfirm(CCharacter* pCha, uLong guildid) {
		m_Lock.Lock();

		try {
			_tab_gld->TryForConfirm(pCha, guildid);
		} catch (...) {
		}

		m_Lock.Unlock();
	}
	char GetGuildTypeByID(CCharacter* pCha, uLong guildid) {
		char ret = -1;
		m_Lock.Lock();

		try {
			ret = _tab_gld->GetTypeByID(guildid);
		} catch (...) {
		}

		m_Lock.Unlock();

		return ret;
	}

	bool GuildListTryPlayer(CCharacter* pCha, char disband_days) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->ListTryPlayer(pCha, disband_days);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	bool GuildApprove(CCharacter* pCha, uLong chaid) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Approve(pCha, chaid);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	bool GuildReject(CCharacter* pCha, uLong chaid) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Reject(pCha, chaid);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	bool GuildKick(CCharacter* pCha, uLong chaid) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Kick(pCha, chaid);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	bool GuildLeave(CCharacter* pCha) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Leave(pCha);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	bool GuildDisband(CCharacter* pCha, cChar* passwd) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Disband(pCha, passwd);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	bool GuildMotto(CCharacter* pCha, cChar* motto) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Motto(pCha, motto);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	CTableMapMask* GetMapMaskTable() {
		return _tab_mmask;
	}
	bool GetGuildName(long lGuildID, std::string& strGuildName) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->GetGuildName(lGuildID, strGuildName);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	bool GuildCancelTryFor(CCharacter* pCha) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->CancelTryFor(pCha);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}

	bool Challenge(CCharacter* pCha, BYTE byLevel, DWORD dwMoney) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Challenge(pCha, byLevel, dwMoney);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}

	bool Leizhu(CCharacter* pCha, BYTE byLevel, DWORD dwMoney) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->Leizhu(pCha, byLevel, dwMoney);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}

	void ListChallenge(CCharacter* pCha) {
		m_Lock.Lock();

		try {
			_tab_gld->ListChallenge(pCha);
		} catch (...) {
		}

		m_Lock.Unlock();
	}

	bool StartChall(BYTE byLevel) {
		m_Lock.Lock();

		try {
			for (int i = 0; i < 100; i++) {
				if (_tab_gld->StartChall(byLevel)) {
					m_Lock.Unlock();
					return true;
				}
			}
		} catch (...) {
		}

		m_Lock.Unlock();

		return false;
	}

	bool GetChall(BYTE byLevel, DWORD& dwGuildID1, DWORD& dwGuildID2, DWORD& dwMoney) {
		m_Lock.Lock();

		try {
			for (int i = 0; i < 100; i++) {
				if (_tab_gld->GetChallInfo(byLevel, dwGuildID1, dwGuildID2, dwMoney)) {
					m_Lock.Unlock();
					return true;
				}
			}
		} catch (...) {
		}

		m_Lock.Unlock();
		return false;
	}

	void EndChall(DWORD dwGuild1, DWORD dwGuild2, BOOL bChall) {
		m_Lock.Lock();

		try {
			_tab_gld->EndChall(dwGuild1, dwGuild2, bChall);
		} catch (...) {
		}

		m_Lock.Unlock();
	}

	bool HasChall(BYTE byLevel) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->HasCall(byLevel);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}

	bool HasGuildLevel(CCharacter* pChar, BYTE byLevel) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_gld->HasGuildLevel(pChar, byLevel);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}

	// 执行sql语句到gamelog表
	void ExecLogSQL(const char* pszSQL) {
		SQLRETURN l_sqlret = _tab_log->exec_sql_direct(pszSQL);
		if (!DBOK(l_sqlret)) {
			//LG("gamelog", "添加log记录失败, sql = [%s]!\n", pszSQL);
			LG("gamelog", "add log note failed, sql = [%s]!\n", pszSQL);
		}
	}

	// 可以Log 5个字符串字段, 最后一个长度为8000字符以内
	//void Log(const char *type, const char *c1, const char *c2, const char *c3, const char *c4, const char *p, BOOL bAddToList = TRUE);
	//void Log1(int nType, const char *cha1, const char *cha2, const char *pszContent);
	//void Log2(int nType, CCharacter *pCha1, CCharacter *pCha2, const char *pszContent);

	// Add by lark.li 20080324 begin
	void ExecTradeLogSQL(const char* gameServerName, const char* action, const char* pszChaFrom, const char* pszChaTo, const char* pszTrade) {
		time_t ltime;
		time(&ltime);

		tm ttm;
		//tm* ttm  = localtime(&ltime);
		localtime_s(&ttm, &ltime);
		char time[20];
		time[19] = 0;
		//sprintf(time, "%04i/%02i/%02i %02i:%02i:%02i", ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday, ttm->tm_hour, ttm->tm_min, ttm->tm_sec);
		_snprintf_s(time, sizeof(time), _TRUNCATE, "%04i/%02i/%02i %02i:%02i:%02i", ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday, ttm.tm_hour, ttm.tm_min, ttm.tm_sec);

		char format[] = "insert into Trade_Log(ExecuteTime,GameServer,[Action],[From],[To],Memo) values ('%s','%s','%s','%s','%s','%s')";

		char sql[1024];
		memset(sql, 0, sizeof(sql));
		//sprintf(sql, format, time, gameServerName, action, pszChaFrom, pszChaTo, pszTrade);
		_snprintf_s(sql, sizeof(sql), _TRUNCATE, format, time, gameServerName, action, pszChaFrom, pszChaTo, pszTrade);

		ExecLogSQL(sql);
	}
	// End

	BOOL m_bInitOK;

	//	2008-7-28	yyy	add	function	begin!
	//	记录道具到道具表。
	bool LockItem(SItemGrid* sig, int iChaId) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_item->LockItem(sig, iChaId);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	};
	//	2008-7-28	yyy	add	function	end!

	// 道具解锁 add by ning.yan  2008-11-12 begin
	bool UnlockItem(SItemGrid* sig, int iChaId) {
		bool ret = true;
		m_Lock.Lock();

		try {
			ret = _tab_item->UnlockItem(sig, iChaId);
		} catch (...) {
			ret = false;
		}

		m_Lock.Unlock();

		return ret;
	}
	// end

protected:
	cfl_db _connect;
	CTableCha* _tab_cha;
	CTableMaster* _tab_master;
	CTableResource* _tab_res;

	// Add by lark.li 20080521 begin
	// 彩票
	CTableLotterySetting* _tab_setting;
	CTableTicket* _tab_ticket;
	CTableWinTicket* _tab_winticket;

	// 竞技场
	CTableAmphitheaterSetting* _tab_amp_setting;
	CTableAmphitheaterTeam* _tab_amp_team;
	// End

	CTablePersoninfo* _tab_per_info; //Add by sunny.sun20080822
#ifdef SHUI_JING
	CTableCrystalTrade* _tab_csl_trade;
#endif
	CTableMapMask* _tab_mmask;
	CTableAct* _tab_act;
	CTableGuild* _tab_gld;
	CTableBoat* _tab_boat;
	CTableLog* _tab_log;
	CTableItem* _tab_item;

	CThrdLock m_Lock; // 普通用的

	CThrdLock m_LockEx; // 为了分出的线程使用
};

extern CGameDB game_db;

#include <vector>

/**
 * @class CGameDBPool 
 * @author lark
 * @brief 数据库对象池，便于多线程访问数据库
 *			
 */
class CGameDBPool {
public:
	static CGameDBPool* Instance(int sizoe = 10);

	CGameDB* GetGameDB();

protected:
	CGameDBPool(int size);
	~CGameDBPool();

private:
	int m_iSize;
	int m_iIndex;
	CThrdLock m_Lock;

	std::vector<CGameDB*> m_DBPool;

	static CGameDBPool* _instance;
};