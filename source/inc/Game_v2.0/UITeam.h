#pragma once
#pragma once

class CCharacter;
struct stNetTeamChaPart;

namespace GUI {

//
enum eTeamStyle {
	enumTeamFrnd,	 // 好友
	enumTeamGroup,	// 队伍
	enumTeamGuild,	// 行会
	enumTeamRoad,	 // 路人
	enumTeamMaster,   // 师傅
	enumTeamPrentice, // 徒弟
};

enum eShowStyle {
	enumShowQQName,  // 显示名字
	enumShowQQMotto, // 显示座右铭
};

class CMemberData {
public:
	CMemberData();
	~CMemberData();

	DWORD GetIndex() const { return _dwIndex; }
	void SetIndex(DWORD v) { _dwIndex = v; }

	long GetHP() const { return _nHp; }
	long GetSP() const { return _nSp; }
	long GetLV() const { return _nLv; }
	long GetMaxHP() const { return _nMaxHp; }
	long GetMaxSP() const { return _nMaxSp; }
	BYTE GetWork() const { return _byWork; }

	void SetWork(BYTE v) { _byWork = v; }
	void SetHP(long v) { _nHp = v; }
	void SetSP(long v) { _nSp = v; }
	void SetLV(long v) { _nLv = v; }
	void SetMaxHP(long v) { _nMaxHp = v; }
	void SetMaxSP(long v) { _nMaxSp = v; }

	void SetOnline(bool isOnline) { _bOnline = isOnline; }
	bool IsOnline() const { return _bOnline; }

	stNetTeamChaPart* GetFace() { return _pFace.get(); }
	void SetFace(const stNetTeamChaPart& pFace);

private:
	DWORD _dwIndex{0};
	bool _bOnline{false};
	BYTE _byWork{0};
	long _nMaxHp{0};
	long _nHp{0}; // 血
	long _nSp{0}; // 魔法
	long _nLv{0};
	long _nMaxSp{0};

	std::unique_ptr<stNetTeamChaPart> _pFace{nullptr};
};

class CTeam;
class CMember {
public:
	CMember(CTeam* pTeam, unsigned long id, const char* strName, const char* strMotto, DWORD icon_id, const char* szGroupName = "");

	virtual ~CMember();

	void Refresh();

	void SetName(const char* str);
	void SetMotto(const char* str);
	void SetJobName(const char* str);

	void SetID(DWORD id) { _nID = id; }
	void SetIconID(DWORD iconId);
	void SetPointer(void* p) { _pPointer = p; }
	void SetOnline(bool isOnline);
	void ModifyAttr(std::string motto, std::string job, DWORD dwLv, DWORD nIcon_ID, std::string guildName);

	DWORD GetID() const { return _nID; }
	const char* GetName() const { return _strName.c_str(); }
	const char* GetMotto() const { return _strMotto.c_str(); }
	const char* GetShowName();
	const char* GetGuildName() const { return _strGuildName.c_str(); }

	const char* GetGroupName() const { return _strGroupName.c_str(); }
	void SetGroupName(const char* szGroupName) { _strGroupName = szGroupName; }

	DWORD GetIconID() const { return _nIcon_id; }
	DWORD GetLevel() const { return _dwLv; }
	const char* GetJobName() const { return _strJob.c_str(); }
	void* GetPointer() { return _pPointer; }
	CTeam* GetTeam() { return _pTeam; }
	CMemberData* GetData() { return _pData.get(); }

	bool IsOnline() const { return _bIsOnline; }
	void SetIsFlash(bool isFlash) { _bFlash = isFlash; }
	bool GetIsFlash() const { return _bFlash; }

	static void SetShowStyle(eShowStyle style);
	static eShowStyle _nShowStyle;

private:
	std::string _strName; // 名字
	std::string _strMotto;
	std::string _strJob;
	std::string _strShowName;
	std::string _strGuildName;

	std::string _strGroupName; // 分组名字

	DWORD _dwLv;
	DWORD _nID; // ID
	DWORD _nIcon_id;
	bool _bIsOnline{false};
	void* _pPointer{nullptr};
	bool _bFlash{false};

	std::unique_ptr<CMemberData> _pData;
	CTeam* _pTeam;
};

class CTeam {
public:
	CTeam(eTeamStyle nStyle, const char* szName);
	~CTeam();

	void Clear();

	CMember* Add(unsigned long nID, const char* szName, const char* szMotto, DWORD icon_id, const char* szGroupName = "", bool onLine = false);

	bool Del(unsigned long nID);

	CMember* Find(unsigned long nID);
	CMember* GetMember(unsigned long nIndex);

	const char* GetName() const { return _strName.c_str(); }
	int GetStyle() const { return _eStyle; }
	DWORD GetCount() const { return _nCount; }

	void SetPointer(void* p) { _pPointer = p; }
	void* GetPointer() { return _pPointer; }

	int GetGroupNum() const { return (int)_groups.size(); }
	const char* GetGroupName(int index) const;

	void AddGroupName(const char* szGroupName);
	void DelGroupName(const char* szGroupName);
	void ChangeGroupName(const char* szOldGroupName, const char* szNewGroupName);
	void MoveGroup(unsigned long nID, const char* szOldGroupName, const char* szNewGroupName);

	void ClearGroupName();

private:
	eTeamStyle _eStyle;
	std::string _strName;


	using members = std::list<CMember*>;
	members _member;

	std::vector<std::pair<std::string, int>> _groups;

	DWORD _nCount{0};

	void* _pPointer{nullptr};
}; // namespace GUI

class CTeamMgr {
public:
	CTeamMgr();
	~CTeamMgr();

	CTeam* Add(eTeamStyle eTeam, const char* szName = "");
	bool Del(eTeamStyle eTeam, const char* szName = "");
	CTeam* Find(eTeamStyle eTeam, const char* szName = "");
	void ChangeStyle(eShowStyle style);

	void SceneSwitch();
	void ResetAll();

	static DWORD GetTeamLeaderID() { return _dwTeamLeaderID; }
	static void SetTeamLeaderID(DWORD v) { _dwTeamLeaderID = v; }

private:
	std::unique_ptr<CTeam> _pFrndTeam;	 // 好友
	std::unique_ptr<CTeam> _pGroupTeam;	// 队伍
	std::unique_ptr<CTeam> _pRoadTeam;	 // 路人
	std::unique_ptr<CTeam> _pMasterTeam;   // 师傅
	std::unique_ptr<CTeam> _pPrenticeTeam; // 徒弟

	static DWORD _dwTeamLeaderID;
};

// 内联函数

} // namespace GUI
