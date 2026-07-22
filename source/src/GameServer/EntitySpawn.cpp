//=============================================================================
// FileName: EntitySpawn.cpp
// Creater: ZhangXuedong
// Date: 2004.09.10
// Comment: CChaSpawn class
//=============================================================================
#include "stdafx.h"
#include "EntitySpawn.h"
#include "excp.h"
#include "Character.h"
#include "GameCommon.h"
#include "GameAppNet.h"
#include "SubMap.h"
#include "CompCommand.h"
#include "GameApp.h"
#include "NPC.h"
#include "rng.h"

extern BOOL LoadTable(CRawDataSet* pTable, const char*);

_DBC_USING

//=============================================================================
CChaSpawn::CChaSpawn() {}

CChaSpawn::~CChaSpawn() {
	SAFE_DELETE_ARRAY(m_pSMonInfo);
	SAFE_DELETE(m_pCMonRefRecordSet);
}

bool CChaSpawn::Init(char* szSpawnTable, long lRegionNum) {
	m_pCMap = nullptr;

	if (lRegionNum <= 0) {
		THROW_EXCP(excpArr, RES_STRING(GM_ENTITYSPAWN_CPP_00001));
	}
	strncpy_s(m_szSpawnTable, sizeof(m_szSpawnTable), szSpawnTable, _TRUNCATE);
	m_lRecordNum = lRegionNum;

	m_pCMonRefRecordSet = new CMonRefRecordSet(0, lRegionNum);
	if (!m_pCMonRefRecordSet) {
		THROW_EXCP(excpMem, RES_STRING(GM_ENTITYSPAWN_CPP_00002));
	}
	if (!LoadTable(m_pCMonRefRecordSet, m_szSpawnTable)) {
		return false;
	}

	m_lRegionNum = lRegionNum;
	m_pSMonInfo = new SMonInfo[m_lRegionNum];
	if (!m_pSMonInfo) {
		THROW_EXCP(excpMem, RES_STRING(GM_ENTITYSPAWN_CPP_00003));
	}

	memset(m_pSMonInfo, 0, sizeof(SMonInfo) * m_lRegionNum);
	m_lCount = 0;

	return true;
}

long CChaSpawn::Load(SubMap* pCMap) {
	
	try {
		m_pCMap = pCMap;

		long lRet = 1;
		CMonRefRecord* pMonRefRecord;

		m_lCount = 0;

		char szSpawnError[512] = "Cha born error";

		char szMap[512];
		_snprintf_s(szMap, sizeof(szMap), _TRUNCATE, "spawn_mum_%s", pCMap->GetName());
		long lNum;
		const Rect& area = pCMap->GetRange();
		//LG(szSpawnError, "角色出生错误的可能原因：1，初始出生位置非法。2，在角色表中找不到对应的记录。3，在出生位置120米范围内没有找到适合该角色的区域类型\n\n\n");
		for (int i = 0; i < m_lRegionNum; i++) {
			pMonRefRecord = GetMonRefRecordInfo(i + 1);
			if (!pMonRefRecord) {
				continue;
			}
			lNum = 0;
			for (int j = 0; j < defMAX_REGION_MONSTER_TYPE; j++) {
				for (int k = 0; k < pMonRefRecord->lMonster[j][1]; k++) {
					short sAngle = pMonRefRecord->sAngle;
					if (sAngle == -1) {
						sAngle = rng.uniform(0, 360 - 1);
					}
					Point l_pos = {
						rng.uniform(
							min(pMonRefRecord->SRegion[0].x, pMonRefRecord->SRegion[1].x),
							max(pMonRefRecord->SRegion[0].x, pMonRefRecord->SRegion[1].x) - 1),
						rng.uniform(
							min(pMonRefRecord->SRegion[0].y, pMonRefRecord->SRegion[1].y),
							max(pMonRefRecord->SRegion[0].y, pMonRefRecord->SRegion[1].y) - 1)};

					CCharacter* pCCha;
					if (pCCha = pCMap->ChaSpawn(pMonRefRecord->lMonster[j][0], enumCHACTRL_NONE, sAngle, &l_pos)) {
						Rect rRectTerritory = {pMonRefRecord->SRegion[0], pMonRefRecord->SRegion[1]};
						pCCha->SetRectTerritory(rRectTerritory);
						pCCha->SetResumeTime(pMonRefRecord->lMonster[j][3] * 1000);
						m_lCount++;
						lNum++;
						if (m_lCount >= g_Config.m_nMaxCha) {
							LG(szMap, RES_STRING(GM_GAMEAPP_CPP_00016));
							return 1;
						}
					} else
						LG(szSpawnError, "character born error,born information:map %s[%d, %d],character hatch list number %d,character list number %d,born position[%d, %d]\n",
						   pCMap->GetName(), area.width(), area.height(), i + 1, pMonRefRecord->lMonster[j][0], l_pos.x, l_pos.y);
				}
			}
			LG(szMap, "entry %d character number:\t%d\n", i + 1, lNum);
		}
		return lRet;
	}
	T_E
}

long CChaSpawn::Reload() {
	try {
		// 清除所属地图上的所有怪物
		return 0;
	}
	T_E
}

//=============================================================================
CMapSwitchEntitySpawn::CMapSwitchEntitySpawn() {}

CMapSwitchEntitySpawn::~CMapSwitchEntitySpawn() {
	SAFE_DELETE(m_pCSwitchMapRecSet);
}

bool CMapSwitchEntitySpawn::Init(char* szSpawnTable, long lRecordNum) {
	m_pCMap = nullptr;

	if (lRecordNum <= 0) {
		THROW_EXCP(excpArr, RES_STRING(GM_ENTITYSPAWN_CPP_00005));
	}
	strncpy_s(m_szSpawnTable, sizeof(m_szSpawnTable), szSpawnTable, _TRUNCATE);
	m_lRecordNum = lRecordNum;

	m_pCSwitchMapRecSet = new CSwitchMapRecordSet(0, lRecordNum);
	if (!m_pCSwitchMapRecSet) {
		THROW_EXCP(excpMem, RES_STRING(GM_ENTITYSPAWN_CPP_00006));
	}
	if (!LoadTable(m_pCSwitchMapRecSet, m_szSpawnTable)) {
		return false;
	}

	return true;
}

long CMapSwitchEntitySpawn::Load(SubMap* pCMap) {
	m_pCMap = pCMap;

	long lRet = 1;
	CSwitchMapRecord* pCSwitchMapRecord;

	{
		SItemGrid SItemCont;
		CItem* pCItem;
		for (int i = 0; i < m_lRecordNum; i++) {
			pCSwitchMapRecord = GetSwitchMapRecordInfo(i + 1);
			if (!pCSwitchMapRecord) {
				continue;
			}
			SItemCont.sID = (short)pCSwitchMapRecord->lEntityID;
			SItemCont.sNum = 1;
			SItemCont.SetDBParam(-1, 0);
			SItemCont.chForgeLv = 0;
			SItemCont.SetInstAttrInvalid();
			CEvent CEvtCont;
			CEvtCont.SetID((short)pCSwitchMapRecord->lEventID);
			CEvtCont.SetTouchType(enumEVENTT_RANGE);
			CEvtCont.SetExecType(enumEVENTE_SMAP_ENTRY);
			CEvtCont.SetTableRec(pCSwitchMapRecord);
			pCItem = pCMap->ItemSpawn(&SItemCont, pCSwitchMapRecord->SEntityPos.x, pCSwitchMapRecord->SEntityPos.y, enumITEM_APPE_NATURAL, 0, g_pCSystemCha->GetID(), g_pCSystemCha->GetHandle(), -1, -1,
									  &CEvtCont);
			if (pCItem) {
				pCItem->SetOnTick(0);
			}
		}
	}
	return lRet;
}

long CMapSwitchEntitySpawn::Reload() {
	return 0;
}

//=============================================================================
CNpcSpawn::CNpcSpawn() {}

CNpcSpawn::~CNpcSpawn() {
	Clear();
}

bool CNpcSpawn::Init(char* szSpawnTable, long lRecordNum) {
	if (lRecordNum <= 0 || !szSpawnTable) {
		char szTemp[128];
		CFormatParameter param(2);
		param.setString(0, szSpawnTable);
		param.setLong(1, lRecordNum);
		RES_FORMAT_STRING(GM_ENTITYSPAWN_CPP_00007, param, szTemp);
		THROW_EXCP(excpArr, szTemp);
	}

	strncpy_s(m_szSpawnTable, sizeof(m_szSpawnTable), szSpawnTable, _TRUNCATE);
	m_lRecordNum = lRecordNum;

	m_pNpcRecordSet = new CNpcRecordSet(0, lRecordNum);
	if (!m_pNpcRecordSet) {
		THROW_EXCP(excpMem, RES_STRING(GM_ENTITYSPAWN_CPP_00008));
	}
	if (!LoadTable(m_pNpcRecordSet, m_szSpawnTable)) {
		return false;
	}

	return true;
}

void CNpcSpawn::Clear() {
	SAFE_DELETE(m_pNpcRecordSet);
}

mission::CNpc* CNpcSpawn::FindNpc(const char szName[]) {
	for (int i = 0; i < m_sNumNpc; i++) {
		if (strcmp(m_NpcList[i]->GetNpcName(), szName) == 0) {
			return m_NpcList[i];
		}
	}
	return nullptr;
}

long CNpcSpawn::Load(SubMap& submap) {
	//"Initialize the list of NPC pointers loaded by the map"
	m_NpcList.fill(nullptr);
	m_sNumNpc = 0;

	printf(RES_STRING(GM_ENTITYSPAWN_CPP_00009), m_szSpawnTable);

	for (int i = 0; i < m_lRecordNum; i++) {
		auto pNpcRecord = static_cast<CNpcRecord*>(m_pNpcRecordSet->GetRawDataInfo(i));
		if (!pNpcRecord) {
			continue;
		}

		CChaRecord* pCharRecord = GetChaRecordInfo(pNpcRecord->sCharID);
		if (!pCharRecord) {
			printf(RES_STRING(GM_ENTITYSPAWN_CPP_00010), pNpcRecord->sCharID);
			LG("npcinit_error", "initialization map error：not find appoint ID roll attribute information！ID = %d", pNpcRecord->sCharID);
			continue;
		}

		switch (pNpcRecord->sNpcType) {
		case mission::CNpc::TALK: {
			mission::CTalkNpc* pTalk = g_pGameApp->GetNewTNpc();
			if (!pTalk) {
				break;
			}
			if (pTalk->Load(*pNpcRecord, *pCharRecord) == FALSE) {
				pTalk->Free();
				continue;
			}
			//
			Square SShape = {{pNpcRecord->dwxPos0, pNpcRecord->dwyPos0}, pCharRecord->sRadii};
			if (!submap.Enter(&SShape, pTalk)) {
				pTalk->Free();
				continue;
			}
			if (m_sNumNpc < ROLE_MAXNUM_MAPNPC) {
				m_NpcList[m_sNumNpc++] = pTalk;
			}
		} break;
		default: {
		} break;
		}
	}
	printf(RES_STRING(GM_ENTITYSPAWN_CPP_00011), m_szSpawnTable);
	return 0;
}

long CNpcSpawn::Reload() {
	return 0;
}

//NOTE: Does not return NpcInfo, why?
CNpcRecord* CNpcSpawn::GetNpcInfo(USHORT sNpcID) {
	if (m_pNpcRecordSet) {
		(CNpcRecord*)m_pNpcRecordSet->GetRawDataInfo(sNpcID);
	}

	return nullptr;
}

//NOTE: Why is return value 'true' commented?
BOOL CNpcSpawn::SummonNpc(const char szNpc[], USHORT sAreaID, USHORT sTime) {
	for (USHORT i = 0; i < m_sNumNpc; i++) {
		if (m_NpcList[i] && m_NpcList[i]->GetIslandID() == sAreaID && strcmp(m_NpcList[i]->GetName(), szNpc) == 0) {
			m_NpcList[i]->Summoned(sTime);
			//return TRUE;
		}
	}
	return FALSE;
}