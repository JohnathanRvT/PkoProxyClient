//=============================================================================
// FileName: MapEntry.cpp
// Creater: ZhangXuedong
// Date: 2005.10.21
// Comment: Map Entry class
//=============================================================================

#include "stdafx.h"
#include "MapEntry.h"
#include "Parser.h"
#include "NetCommand.h"

CDynMapEntry g_CDMapEntry;
#ifdef NEW_MAPCOPY
//char	g_szTFightMapName[MAX_MAPNAME_LENGTH] = "";	// 队伍挑战地图名
#else
char g_szTFightMapName[MAX_MAPNAME_LENGTH] = ""; // 队伍挑战地图名
#endif

//=============================================================================
void CMapEntryCopyCell::WriteParamPacket(WPacket& wpk) {
	for (const auto& param : m_lParam) {
		wpk.WriteLong(param);
	}
}

//=============================================================================
void CDynMapEntryCell::SetCopyNum(dbc::Short sCopyNum) {
	try {
		if (sCopyNum > defMAX_MAP_COPY_NUM) {
			LG("copy number error", RES_STRING(GM_GAMEAPP_CPP_00008), sCopyNum, defMAX_MAP_COPY_NUM);
			return;
		}

		if (m_LCopyInfo.GetSize() == 0) {
			if (m_LCopyInfo.Init(defMAX_MAP_COPY_NUM) == false) {
				LG("copy number error", "CopyNum = %d\n", sCopyNum);
			}
		} else {
			m_LCopyInfo.Reset();
		}

		m_sMapCopyNum = sCopyNum;
		m_LCopyInfo.SetCapacity(m_sMapCopyNum);
	}
	T_E
}

void CDynMapEntryCell::SetEventName(dbc::cChar* cszEventName) {
	try {
		m_CEvtObj.SetName(cszEventName);

		if (m_pCEnt) {
			m_pCEnt->m_CEvent.SetName(cszEventName);
			m_pCEnt->SynEventInfo();
		}
	}
	T_E
}

void CDynMapEntryCell::FreeEnti() {
	try {
		if (m_pCEnt) {
			m_pCEnt->Free();
			m_pCEnt = nullptr;
		}
	}
	T_E
}

void CDynMapEntryCell::SynCopyParam(dbc::Short sCopyID) {
	try {
		CMapEntryCopyCell* pCCopyCell = GetCopy(sCopyID);
		if (!pCCopyCell) {
			return;
		}

		WPacket wpk = GETWPACKET();
		wpk.WriteCmd(CMD_MT_MAPENTRY);
		wpk.WriteString(GetTMapName());
		wpk.WriteString(GetMapName());
		wpk.WriteChar(enumMAPENTRY_COPYPARAM);
		wpk.WriteShort(sCopyID);
		pCCopyCell->WriteParamPacket(wpk);

		BEGINGETGATE();
		while (GateServer* pGateServer = GETNEXTGATE()) {
			pGateServer->SendData(wpk);
		}
	}
	T_E
}

void CDynMapEntryCell::SynCopyRun(dbc::Short sCopyID, dbc::Char chCdtType, dbc::Long chCdtVal) {
	try {
		CMapEntryCopyCell* pCCopyCell = GetCopy(sCopyID);
		if (!pCCopyCell) {
			return;
		}

		WPacket wpk = GETWPACKET();
		wpk.WriteCmd(CMD_MT_MAPENTRY);
		wpk.WriteString(GetTMapName());
		wpk.WriteString(GetMapName());
		wpk.WriteChar(enumMAPENTRY_COPYRUN);
		wpk.WriteShort(sCopyID);
		wpk.WriteChar(chCdtType);
		wpk.WriteLong(chCdtVal);

		BEGINGETGATE();
		while (GateServer* pGateServer = GETNEXTGATE()) {
			pGateServer->SendData(wpk);
			break;
		}
	}
	T_E
}

//=============================================================================
CDynMapEntryCell* CDynMapEntry::Add(CDynMapEntryCell* pCCell) {
	try {
		CDynMapEntryCell* pCObj = GetEntry(pCCell->GetTMapName());
		if (!pCObj) {
			pCObj = m_LEntryList.Add(pCCell);
		} else {
			pCObj->FreeEnti();
		}
		if (!pCObj) {
			return nullptr;
		}
		CEvent* pCEvent = pCObj->GetEvent();
		pCEvent->SetTableRec(pCObj);

		return pCObj;
	}
	T_E
}

bool CDynMapEntry::Del(CDynMapEntryCell* pCCell) {
	try {
		if (!pCCell) {
			return false;
		}

		pCCell->FreeEnti();
		return m_LEntryList.Del(pCCell);
	}
	T_E
}

bool CDynMapEntry::Del(dbc::cChar* cszTMapName) {
	try {
		if (!cszTMapName) {
			return false;
		}

		CDynMapEntryCell* pCCell;
		m_LEntryList.BeginGet();
		while (pCCell = m_LEntryList.GetNext()) {
			if (strncmp(pCCell->GetTMapName(), cszTMapName, MAX_MAPNAME_LENGTH) == 0) {
				pCCell->FreeEnti();
				return m_LEntryList.Del(pCCell);
			}
		}

		return false;
	}
	T_E
}

CDynMapEntryCell* CDynMapEntry::GetEntry(dbc::cChar* cszTMapName) {
	try {
		CDynMapEntryCell* pCCell;
		m_LEntryList.BeginGet();
		while (pCCell = m_LEntryList.GetNext()) {
			if (strncmp(pCCell->GetTMapName(), cszTMapName, MAX_MAPNAME_LENGTH) == 0)
				return pCCell;
		}

		return nullptr;
	}
	T_E
}

void CDynMapEntry::AfterPlayerLogin(const char* cszName) {
	try {
		if (!cszName) {
			return;
		}

		CDynMapEntryCell* pCCell;
		m_LEntryList.BeginGet();
		while (pCCell = m_LEntryList.GetNext()) {
			std::string strScript = "after_player_login_";
			strScript += pCCell->GetTMapName();
			g_CParser.DoString(strScript.c_str(), enumSCRIPT_RETURN_NONE, 0,
							   enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCCell,
							   enumSCRIPT_PARAM_STRING, 1, cszName, DOSTRING_PARAM_END);
		}
	}
	T_E
}

#ifndef NEW_MAPCOPY
//=============================================================================
void g_SetTeamFightMapName(const char* cszMapName) {
	try {
		if (cszMapName) {
			strncpy_s(g_szTFightMapName, sizeof(g_szTFightMapName), cszMapName, _TRUNCATE);
			g_szTFightMapName[MAX_MAPNAME_LENGTH - 1] = '\0';
		} else {
			memset(g_szTFightMapName, 0, sizeof(g_szTFightMapName));
		}
	}
	T_E
}
#endif