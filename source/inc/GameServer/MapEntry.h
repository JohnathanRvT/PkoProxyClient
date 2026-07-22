//=============================================================================
// FileName: MapEntry.h
// Creater: ZhangXuedong
// Date: 2005.10.21
// Comment: Map Entry class
//=============================================================================

#ifndef MAPENTRY_H
#define MAPENTRY_H

#include "Entity.h"
#include "ToolClass.h"
#include "MapRes.h"
#include "EventRecord.h"

class CMapEntryCopyCell {
public:
	CMapEntryCopyCell(dbc::Short sMaxPlyNum = 0, dbc::Short sCurPlyNum = 0)
		: m_sMaxPlyNum(sMaxPlyNum), m_sCurPlyNum(sCurPlyNum), m_sPosID(-1) {}

	void SetMaxPlyNum(dbc::Short sPlyNum) { m_sMaxPlyNum = sPlyNum; }
	dbc::Short GetMaxPlyNum() const { return m_sMaxPlyNum; }
	void SetCurPlyNum(dbc::Short sPlyNum) { m_sCurPlyNum = sPlyNum; }
	dbc::Short GetCurPlyNum() const { return m_sCurPlyNum; }
	bool AddCurPlyNum(dbc::Short sAddNum) {
		dbc::Short sNum = m_sCurPlyNum + sAddNum;
		if (sNum < 0 || sNum > m_sMaxPlyNum) {
			return false;
		}
		m_sCurPlyNum = sNum;
		return true;
	}
	bool HasFreePlyCount(dbc::Short sRequestNum) const { return GetMaxPlyNum() - GetCurPlyNum() >= sRequestNum ? true : false; }

	dbc::Long GetParam(dbc::Char chParamID) {
		if (chParamID < 0 || chParamID >= defMAPCOPY_INFO_PARAM_NUM) {
			return 0;
		}
		return m_lParam[chParamID];
	}
	bool SetParam(dbc::Char chParamID, dbc::Long lParamVal) {
		if (chParamID < 0 || chParamID >= defMAPCOPY_INFO_PARAM_NUM) {
			return false;
		}
		m_lParam[chParamID] = lParamVal;
		return true;
	}

	void WriteParamPacket(WPacket& wpk);

	void SetPosID(dbc::Long lPosID) { m_sPosID = (dbc::Short)lPosID; }
	dbc::Long GetPosID() const { return m_sPosID; }

protected:
private:
	dbc::Short m_sMaxPlyNum;
	dbc::Short m_sCurPlyNum;

	dbc::Long m_lParam[defMAPCOPY_INFO_PARAM_NUM];

	dbc::Short m_sPosID;
};

// 地图动态入口单元
class CDynMapEntryCell {
public:
	CDynMapEntryCell() {
		m_CEvtObj.Init();
		m_CEvtObj.SetTouchType(enumEVENTT_RANGE);
		m_CEvtObj.SetExecType(enumEVENTE_DMAP_ENTRY);
	}

	void SetPos(void* pPos) { m_pPos = pPos; }
	void* GetPos() const { return m_pPos; }

	void SetMapName(dbc::cChar* cszMapName) {
		if (!cszMapName) {
			return;
		}
		strncpy_s(m_szMapName, sizeof(m_szMapName), cszMapName, _TRUNCATE);
		m_szMapName[MAX_MAPNAME_LENGTH - 1] = '\0';
	}
	dbc::cChar* GetMapName() const { return m_szMapName; }
	void SetTMapName(dbc::cChar* cszTMapName) {
		if (!cszTMapName) {
			return;
		}
		strncpy_s(m_szTMapName, sizeof(m_szTMapName), cszTMapName, _TRUNCATE);
		m_szTMapName[MAX_MAPNAME_LENGTH - 1] = '\0';
	}
	dbc::cChar* GetTMapName() const { return m_szTMapName; }
	const Point* GetEntiPos() const { return &m_SEntiPos; }
	void SetEntiPos(const Point* cpSPos) { m_SEntiPos = *cpSPos; }
	void SetEntiPos(dbc::Long lPosX, dbc::Long lPosY) {
		m_SEntiPos.x = lPosX;
		m_SEntiPos.y = lPosY;
	}
	void SetEntiID(dbc::Long lEntiID) { m_lEntiID = lEntiID; }
	dbc::Long GetEntiID() const { return m_lEntiID; }
	void SetEventID(dbc::Long lEventID) { m_CEvtObj.SetID((dbc::uShort)lEventID); }
	dbc::Long GetEventID() { return m_CEvtObj.GetID(); }
	void SetEventName(dbc::cChar* cszEventName);
	void SetEnti(Entity* pCEnt) { m_pCEnt = pCEnt; }
	void GetPosInfo(const dbc::Char** pMapN, dbc::Long* lpPosX, dbc::Long* lpPosY, const dbc::Char** pTMapN) { *pMapN = m_szMapName, *lpPosX = m_SEntiPos.x, *lpPosY = m_SEntiPos.y, *pTMapN = m_szTMapName; }
	CEvent* GetEvent() { return &m_CEvtObj; }
	void SetCopyNum(dbc::Short sCopyNum);
	dbc::Short GetCopyNum() const { return m_sMapCopyNum; }
	void SetCopyPlyNum(dbc::Short sCopyNum) { m_sCopyPlyNum = sCopyNum; }
	dbc::Short GetCopyPlyNum() const { return m_sCopyPlyNum; }
	CMapEntryCopyCell* AddCopy(CMapEntryCopyCell* pCCpyCell) { return m_LCopyInfo.Add(pCCpyCell); }
	CMapEntryCopyCell* GetCopy(dbc::Short sCopyID) { return m_LCopyInfo.Get(sCopyID); }
	bool ReleaseCopy(CMapEntryCopyCell* pCCpyCell) { return m_LCopyInfo.Del(pCCpyCell); }
	bool ReleaseCopy(dbc::Long lCopyNO) { return m_LCopyInfo.Del(lCopyNO); }
	void FreeEnti();

	void SynCopyParam(dbc::Short sCopyID);
	void SynCopyRun(dbc::Short sCopyID, dbc::Char chCdtType, dbc::Long chCdtVal);

protected:
private:
	dbc::Char m_szMapName[MAX_MAPNAME_LENGTH]{};
	Point m_SEntiPos;
	dbc::Long m_lEntiID{0};
	dbc::Char m_szTMapName[MAX_MAPNAME_LENGTH]{};
	CEvent m_CEvtObj;
	Entity* m_pCEnt{nullptr};

	dbc::Short m_sMapCopyNum;
	dbc::Short m_sCopyPlyNum;
	CListArray<CMapEntryCopyCell> m_LCopyInfo;

	void* m_pPos{nullptr};
};

// 动态地图入口链表，纪录本进程的所有当前存在的动态入口
class CDynMapEntry {
public:
	CDynMapEntry() { m_LEntryList.Init(); }
	~CDynMapEntry() { m_LEntryList.Free(); }

	CDynMapEntryCell* Add(CDynMapEntryCell* pCCell);
	bool Del(CDynMapEntryCell* pCCell);
	bool Del(dbc::cChar* cszTMapName);
	CDynMapEntryCell* GetEntry(dbc::cChar* cszTMapN);
	void AfterPlayerLogin(const char* cszName);

protected:
private:
	CResidentList<CDynMapEntryCell> m_LEntryList;
};

extern CDynMapEntry g_CDMapEntry;

// 队伍挑战地图入口
extern void g_SetTeamFightMapName(const char* cszMapName);

extern char g_szTFightMapName[MAX_MAPNAME_LENGTH]; // 队伍挑战地图名
//

#endif // MAPENTRY_H