// EventEntity.cpp Created by knight-gongjian 2004.11.23.
// EventEntity.cpp Created by knight-gongjian 2004.11.23.
//---------------------------------------------------------

#include "stdafx.h"
#include "EventEntity.h"
#include "SubMap.h"
#include "Character.h"
#include "Packet.h"
#include "GameServerApp.h"
//---------------------------------------------------------

namespace mission {
CEventEntity::CEventEntity()
	: CCharacter() {
	SetType();
	Clear();
	m_sInfoID = -1;
}

CEventEntity::~CEventEntity() {
}

BOOL CEventEntity::Create(SubMap& Submap, const char szName[], USHORT sID, USHORT sInfoID, DWORD dwxPos, DWORD dwyPos, USHORT sDir) {
	CChaRecord* pRec = GetChaRecordInfo(sID);
	if (!pRec) {
		LG("entity_error", "CEventEntity::Create !establish failed!can't find character data info!ID[%d]", sID);
		return FALSE;
	}

	SetEyeshotAbility(false);

	szName ? strncpy_s(m_name, sizeof(m_name), szName, _TRUNCATE)
		   : strncpy_s(m_name, sizeof(m_name), pRec->szName, _TRUNCATE);

	m_ID = g_pGameApp->m_Ident.GetID();
	Char szLogName[defLOG_NAME_LEN] = "";
	_snprintf_s(szLogName, sizeof(szLogName), _TRUNCATE, "Cha-%s+%u", GetName(), GetID());

	m_CLog.SetLogName(szLogName);

	m_pCChaRecord = pRec;
	m_cat = (short)m_pCChaRecord->lID;
	SetAngle(sDir);

	m_CChaAttr.Init(sID);
	setAttr(ATTR_CHATYPE, enumCHACTRL_NPC_EVENT);

	Square SShape = {{dwxPos, dwyPos}, m_pCChaRecord->sRadii};
	if (!Submap.Enter(&SShape, this)) {
		LG("entity_error", "CEventEntity::Create entity enter map failed!");
		return FALSE;
	}
	m_sInfoID = sInfoID;

	return TRUE;
}

HRESULT CEventEntity::MsgProc(CCharacter& character, dbc::RPacket& packet) {
	return 0;
}

//---------------------------------------------------------
// CResourceEntity implemented
CResourceEntity::CResourceEntity()
	: CEventEntity() {
	SetType();
	Clear();
}

CResourceEntity::~CResourceEntity() {
}

void CResourceEntity::Clear() {
	CEventEntity::Clear();
	m_sID = 0;   // 资源信息ID
	m_sNum = 0;  // 资源数量信息
	m_sTime = 0; // 资源采集时间
}

BOOL CResourceEntity::SetData(USHORT sItemID, USHORT sNum, USHORT sTime) {
	Clear();
	m_sID = sItemID;
	m_sNum = sNum;
	m_sTime = sTime;
	this->SetResumeTime(sTime * 1000);
	return TRUE;
}

HRESULT CResourceEntity::MsgProc(CCharacter& character, dbc::RPacket& packet) {
	if (this->GetExistState() == enumEXISTS_WITHERING) {
		return 0;
	}

	if (character.IsMisNeedItem(m_sID)) {
		char szItem[32];
		strncpy_s(szItem, sizeof(szItem), RES_STRING(GM_EVENTENTITY_CPP_00001), _TRUNCATE);

		CItemRecord* pItem = GetItemRecordInfo(m_sID);
		if (!pItem) {
			character.SystemNotice(RES_STRING(GM_EVENTENTITY_CPP_00002), m_sID);
			return FALSE;
		}
		strncpy_s(szItem, sizeof(szItem), pItem->szName, _TRUNCATE);
		if (character.GiveItem(m_sID, m_sNum, enumITEM_INST_TASK, enumSYN_KITBAG_FROM_NPC)) {
			char szData[128];
			CFormatParameter param(2);
			param.setLong(0, m_sNum);
			param.setString(1, szItem);
			RES_FORMAT_STRING(GM_EVENTENTITY_CPP_00003, param, szData);
			character.SystemNotice(szData);
		} else {
			char szData[128];
			CFormatParameter param(2);
			param.setLong(0, m_sNum);
			param.setString(1, szItem);
			RES_FORMAT_STRING(GM_EVENTENTITY_CPP_00004, param, szData);
			character.SystemNotice(szData);
			return 0;
		}

		// 设置资源消失，等待重生
		this->SetExistState(enumEXISTS_WITHERING);
		this->Die();
	} else {
		character.SystemNotice(RES_STRING(GM_EVENTENTITY_CPP_00005));
	}

	return 0;
}

void CResourceEntity::GetState(CCharacter& character, BYTE& byState) {
	character.IsMisNeedItem(m_sID) ? byState = ENTITY_DISABLE
								   : byState = ENTITY_ENABLE;
}

//---------------------------------------------------------
// CTransitEntity implemented
CTransitEntity::CTransitEntity()
	: CEventEntity() {
	SetType();
	Clear();
}

CTransitEntity::~CTransitEntity() {
}

void CTransitEntity::Clear() {
	CEventEntity::Clear();
	memset(m_szMapName, 0, sizeof(char) * MAX_MAPNAME_LENGTH);
	m_sxPos = 0;
	m_syPos = 0;
}

BOOL CTransitEntity::SetData(const char szMap[], USHORT sxPos, USHORT syPos) {
	Clear();
	//strncpy( m_szMapName, szMap, MAX_MAPNAME_LENGTH - 1 );
	strncpy_s(m_szMapName, sizeof(m_szMapName), szMap, _TRUNCATE);
	m_sxPos = sxPos;
	m_syPos = syPos;
	return TRUE;
}

HRESULT CTransitEntity::MsgProc(CCharacter& character, dbc::RPacket& packet) {
	return 0;
}

void CTransitEntity::GetState(CCharacter& character, BYTE& byState) {
	byState = ENTITY_ENABLE;
}

//---------------------------------------------------------
// CBerthEntity implemented
CBerthEntity::CBerthEntity()
	: CEventEntity() {
	SetType();
	Clear();
}

CBerthEntity::~CBerthEntity() {
}

void CBerthEntity::Clear() {
	CEventEntity::Clear();
	m_sxPos = 0;
	m_syPos = 0;
	m_sDir = 0;
	m_sBerthID = 0;
}

BOOL CBerthEntity::SetData(USHORT sBerthID, USHORT sxPos, USHORT syPos, USHORT sDir) {
	Clear();
	m_sxPos = sxPos;
	m_syPos = syPos;
	m_sDir = sDir;
	m_sBerthID = sBerthID;
	return TRUE;
}

HRESULT CBerthEntity::MsgProc(CCharacter& character, dbc::RPacket& packet) {
	character.BoatBerth(m_sBerthID, m_sxPos, m_syPos, m_sDir);
	return 0;
}

void CBerthEntity::GetState(CCharacter& character, BYTE& byState) {
	byState = ENTITY_ENABLE;
}

//---------------------------------------------------------

//---------------------------------------------------------

} // namespace mission