#include "stdafx.h"
#include "Character.h"
#include "Player.h"
#include "GameDB.h"
#include "ChaAttrType.h"
#include "SubMap.h"
#include "Config.h"
#include "Guild.h"
#include "CommFunc.h"
#include "lua_gamectrl.h"
#include "atltime.h"
#include "StringPoolL.h"

char szDBLog[256] = "DBData";

//-------------------
// 检验是否有同名角色
//-------------------
bool CTableCha::VerifyName(const char* pszName) {
	try {
		std::string buf[1];
		const char* param = "cha_name";
		char filter[80]; // sprintf(filter, "cha_name='%s'", pszName);
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_name='%s'", pszName);
		return _get_row(buf, 1, param, filter);
	}
	T_E
}

#define defKITBAG_DATA_STRING_LEN 8192
#define defSKILLBAG_DATA_STRING_LEN 1500
#define defSHORTCUT_DATA_STRING_LEN 1500
#define defSSTATE_DATE_STRING_LIN 1024

const int g_cnCol = 64;
std::string g_buf[g_cnCol];

//下列这些全局变量应该都设定初始值{0} -Waiting Add 2009-03-23
//char g_sql[1024 * 1024]={0};
char g_sql[1024 * 10] = {0}; // 程序以判断最大8K，设定9K已经足够
char g_kitbag[defKITBAG_DATA_STRING_LEN] = {0};
char g_kitbagTmp[defKITBAG_DATA_STRING_LEN] = {0};
char g_equip[defKITBAG_DATA_STRING_LEN] = {0};
char g_look[defLOOK_DATA_STRING_LEN] = {0};
char g_skillbag[defSKILLBAG_DATA_STRING_LEN] = {0};
char g_shortcut[defSHORTCUT_DATA_STRING_LEN] = {0};
char g_skillstate[defSSTATE_DATE_STRING_LIN] = {0};

// Add by lark.li 20080723 begin
char g_extendAttr[ROLE_MAXSIZE_DBMISCOUNT] = {0};
// End

// 任务数据信息存储
char g_szMisInfo[ROLE_MAXSIZE_DBMISSION] = {0};
char g_szRecord[ROLE_MAXSIZE_DBRECORD] = {0};
char g_szTrigger[ROLE_MAXSIZE_DBTRIGGER] = {0};
char g_szMisCount[ROLE_MAXSIZE_DBMISCOUNT] = {0};

bool CTableMaster::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   cha_id1, cha_id2, finish, relation\
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "master");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

unsigned long CTableMaster::GetMasterDBID(CPlayer* pPlayer) {
	if (!pPlayer) {
		return 0;
	}
	CCharacter* pCha = pPlayer->GetMainCha();

	if (!pCha) {
		return 0;
	}

	int nIndex = 0;
	const char* param = "cha_id2";
	char filter[80]; //sprintf(filter, "cha_id1=%d", pPlayer->GetDBChaId());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id1=%d", pPlayer->GetDBChaId());
	int r = _get_row(g_buf, g_cnCol, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		return (unsigned long)(Str2Int(g_buf[nIndex++]));
	} else {
		return (unsigned long)0;
	}
}

bool CTableMaster::IsMasterRelation(int masterID, int prenticeID) {
	const char* param = "count(*)";
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "(cha_id1=%d) and (cha_id2=%d)", prenticeID, masterID);
	int r = _get_row(g_buf, g_cnCol, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		if (Str2Int(g_buf[0]) > 0)
			return true;
	}

	return false;
}

bool CTableCha::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   cha_id, cha_name, motto, icon, version, pk_ctrl, mem_addr, act_id, guild_id, guild_stat, guild_permission, job, degree, exp, \
				   hp, sp, ap, tp, gd, str, dex, agi, con, sta, luk, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, map, main_map, map_x, map_y, radius, \
				   angle, look, skillbag, shortcut, mission, misrecord, mistrigger, miscount, birth, login_cha, live_tp, bank, \
				   delflag, operdate, skill_state, kitbag, kitbag_tmp, kb_locked, credit, store_item \
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "character");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

//-----------------------
// 角色进入游戏时读取数据
//-----------------------
bool CTableCha::ReadAllData(CPlayer* pPlayer, DWORD cha_id) {
	try {
		if (!pPlayer) {
			LG("enter_map", "Loading database error，Player is empty.\n");
			return false;
		}
		CCharacter* pCha = pPlayer->GetMainCha();
		if (!pCha || (pPlayer->GetDBChaId() != cha_id)) {
			LG("enter_map", "Loading database error,the Main character is inexistence or not matching.\n");
			return false;
		}

		int nIndex = 0;
#ifdef SHUI_JING
		char param[] = "act_id, guild_stat, guild_id, hp, sp, exp, radius, angle, cha_name, motto, icon, version, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, main_map, map_x, map_y, birth, look, skillbag, shortcut, mission, misrecord, mistrigger, miscount, login_cha, kitbag, kitbag_tmp, map_mask, skill_state, bank, kb_locked, credit, store_item, extend, flatcoins";
#else
		const char* param = "act_id, guild_stat, guild_id, hp, sp, exp, radius, angle, cha_name, motto, icon, version, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, main_map, map_x, map_y, birth, look, skillbag, shortcut, mission, misrecord, mistrigger, miscount, login_cha, kitbag, kitbag_tmp, map_mask, skill_state, bank, kb_locked, credit, store_item, extend";
#endif
		char filter[80];
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id=%d", cha_id);
		const int r = _get_row(g_buf, g_cnCol, param, filter);
		const int r1 = get_affected_rows();

		LG("enter_map", "Loading database succeed，_get_row.\n");
		if (DBOK(r) && r1 > 0) {
			pPlayer->SetDBActId(Str2Int(g_buf[nIndex++]));
			pCha->SetGuildState(Str2Int(g_buf[nIndex++]));
			pCha->SetGuildID(Str2Int(g_buf[nIndex++]));
			pCha->SetGuildType(game_db.GetGuildTypeByID(pCha, pCha->GetGuildID()));
			//pCha->setAttr(ATTR_GUILD_STATE, Str2Int(g_buf[nIndex++]), 1);
			//pCha->setAttr(ATTR_GUILD, Str2Int(g_buf[nIndex++]), 1);
			//pCha->setAttr(ATTR_GUILD_TYPE, game_db.GetGuildTypeByID(pCha, pCha->getAttr(ATTR_GUILD)), 1);

			pCha->setAttr(ATTR_HP, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_SP, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_CEXP, _atoi64(g_buf[nIndex++].c_str()), 1);

			pCha->SetRadius(Str2Int(g_buf[nIndex++]));
			pCha->SetAngle(Str2Int(g_buf[nIndex++]));
			pCha->SetName((char*)g_buf[nIndex++].c_str());
			Char szLogName[defLOG_NAME_LEN] = "";
			_snprintf_s(szLogName, sizeof(szLogName), _TRUNCATE, "Cha-%s+%u", pCha->GetName(), pCha->GetID());
			pCha->m_CLog.SetLogName(szLogName);

			pCha->SetMotto(g_buf[nIndex++].c_str());
			pCha->SetIcon(Str2Int(g_buf[nIndex++]));

			long lVer = Str2Int(g_buf[nIndex++]);
			if (pCha->getAttr(ATTR_HP) < 0) { // 新角色
				lVer = defCHA_TABLE_NEW_VER;
			}
			pCha->SetPKCtrl(Str2Int(g_buf[nIndex++]));

			strncpy_s(pCha->m_CChaAttr.m_szName, sizeof(pCha->m_CChaAttr.m_szName), pCha->GetName(), _TRUNCATE);

			pCha->setAttr(ATTR_LV, Str2Int(g_buf[nIndex++]), 1);

			// 数据库
			//pCha->setAttr(ATTR_JOB, g_GetJobID(g_buf[nIndex++].c_str()), 1);
			pCha->setAttr(ATTR_JOB, Str2Int(g_buf[nIndex++]), 1);

			pCha->setAttr(ATTR_GD, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_AP, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_TP, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_BSTR, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_BDEX, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_BAGI, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_BCON, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_BSTA, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_BLUK, Str2Int(g_buf[nIndex++]), 1);

			pCha->setAttr(ATTR_SAILLV, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_CSAILEXP, _atoi64(g_buf[nIndex++].c_str()), 1);
			pCha->setAttr(ATTR_CLEFT_SAILEXP, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_LIFELV, Str2Int(g_buf[nIndex++]), 1);
			pCha->setAttr(ATTR_CLIFEEXP, _atoi64(g_buf[nIndex++].c_str()), 1);
			pCha->setAttr(ATTR_LIFETP, Str2Int(g_buf[nIndex++]), 1);

			pCha->SetBirthMap(g_buf[nIndex++].c_str());
			const long lPosX = Str2Int(g_buf[nIndex++]);
			const long lPosY = Str2Int(g_buf[nIndex++]);
			pCha->SetPos(lPosX, lPosY);
			pCha->SetBirthCity(g_buf[nIndex++].c_str());
			//LG("enter_map", "设置角色基本数据成功.\n");

			try {
				const int nLookDataID = nIndex;
				if (!pCha->String2LookDate(g_buf[nIndex++])) {
					LG("enter_map", "Appearance data check sum error.\n");
					LG("Check sum error", "the character (dbid %u，name %s，resid %u)'s change appearance data check sum error.\n", cha_id, pCha->GetLogName(), pCha->GetKitbagRecDBID());
					return false;
				}
				pCha->SetCat(pCha->m_SChaPart.sTypeID);
				//LG("enter_map", "设置角色外观成功.\n");
			} catch (...) {
				LG("enter_map", "Strin2LookData error!!\n");
				LG("enter_map", "Appearance String %s\n", g_buf[nIndex - 1]);
				throw;
			}

			const int nSkillBagDataID = nIndex;
			String2SkillBagData(&pCha->m_CSkillBag, g_buf[nIndex++]);
			//LG("enter_map", "设置角色技能栏成功.\n");

			const int nSortcutDataID = nIndex;
			String2ShortcutData(&pCha->m_CShortcut, g_buf[nIndex++]);
			//LG("enter_map", "设置角色快捷栏成功.\n");

			// 读取任务信息
			pPlayer->MisClear();
			memset(g_szMisInfo, 0, ROLE_MAXSIZE_DBMISSION);
			strncpy_s(g_szMisInfo, sizeof(g_szMisInfo), g_buf[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInit(g_szMisInfo)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00009));
			}
			//LG("enter_map", "设置角色任务1成功.\n");

			memset(g_szRecord, 0, ROLE_MAXSIZE_DBRECORD);
			strncpy_s(g_szRecord, sizeof(g_szRecord), g_buf[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInitRecord(g_szRecord)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00010));
			}
			//LG("enter_map", "设置角色任务2成功.\n");

			memset(g_szTrigger, 0, ROLE_MAXSIZE_DBTRIGGER);
			strncpy_s(g_szTrigger, sizeof(g_szTrigger), g_buf[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInitTrigger(g_szTrigger)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00011));
			}
			//LG("enter_map", "设置角色任务3成功.\n");

			memset(g_szMisCount, 0, ROLE_MAXSIZE_DBMISCOUNT);
			strncpy_s(g_szMisCount, sizeof(g_szMisCount), g_buf[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInitMissionCount(g_szMisCount)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00012));
			}
			//LG("enter_map", "设置角色任务4成功.\n");

			std::string strList[2];
			Util_ResolveTextLine(g_buf[nIndex++].c_str(), strList, 2, ',');
			pPlayer->SetLoginCha(Str2Int(strList[0]), Str2Int(strList[1]));

			//if (lVer != defCHA_TABLE_NEW_VER) // 需要版本转换
			//	SaveTableVer(cha_id);

			pCha->SetKitbagRecDBID(Str2Int(g_buf[nIndex++]));

			pCha->SetKitbagTmpRecDBID(Str2Int(g_buf[nIndex++])); //获取临时背包的资源ID

			pPlayer->SetMapMaskDBID(Str2Int(g_buf[nIndex++]));
			g_strChaState[0] = g_buf[nIndex++];
			pPlayer->Strin2BankDBIDData(g_buf[nIndex++]);

			//背包密码锁
			const int iLocked = Str2Int(g_buf[nIndex++]);
			pCha->GetKitbag()->SetPwdLockState(iLocked);

			//声望
			const int nCredit = Str2Int(g_buf[nIndex++]);
			pCha->SetCredit(nCredit);

			pCha->SetStoreItemID(Str2Int(g_buf[nIndex++]));

			Strin2ChaExtendAttr(pCha, g_buf[nIndex++]);
#ifdef SHUI_JING
			pCha->SetFlatMoney(atoi(g_buf[nIndex++].c_str()));
#endif
			LG("enter_map", "Set the whole data succeed.\n");
		} else {
			LG("enter_map", "Loading database error,_get_row() return value:%d.%u\n", r, r1);
			return false;
		}

		return true;
	}
	T_E
}

bool CTableCha::ReadAllDataEx(CPlayer* pPlayer, DWORD cha_id) {
	try {
		if (!pPlayer) {
			LG("enter_map", "Loading database error，Player is empty.\n");
			return false;
		}
		CCharacter* pCha = pPlayer->GetMainCha();
		if (!pCha || (pPlayer->GetDBChaId() != cha_id)) {
			LG("enter_map", "Loading database error,the Main character is inexistence or not matching.\n");
			return false;
		}

		int nIndex = 0;
#ifdef SHUI_JING
		char param[] = "act_id, guild_stat, guild_id, hp, sp, exp, radius, angle, cha_name, motto, icon, version, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, main_map, map_x, map_y, birth, look, skillbag, shortcut, mission, misrecord, mistrigger, miscount, login_cha, kitbag, kitbag_tmp, map_mask, skill_state, bank, kb_locked, credit, store_item, extend, flatcoins";
#else
		const char* param = "act_id, guild_stat, guild_id, hp, sp, exp, radius, angle, cha_name, motto, icon, version, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, main_map, map_x, map_y, birth, look, skillbag, shortcut, mission, misrecord, mistrigger, miscount, login_cha, kitbag, kitbag_tmp, map_mask, skill_state, bank, kb_locked, credit, store_item, extend";
#endif
		char filter[80];
		std::string stringBufs[64];

		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id=%d", cha_id);
		const int r = _get_row(stringBufs, 64, param, filter);
		const int r1 = get_affected_rows();

		LG("enter_map", "Loading database succeed，_get_row.\n");
		if (DBOK(r) && r1 > 0) {
			pPlayer->SetDBActId(Str2Int(stringBufs[nIndex++]));
			pCha->SetGuildState(Str2Int(stringBufs[nIndex++]));
			pCha->SetGuildID(Str2Int(stringBufs[nIndex++]));
			pCha->SetGuildType(game_db.GetGuildTypeByID(pCha, pCha->GetGuildID()));
			//pCha->setAttr(ATTR_GUILD_STATE, Str2Int(g_buf[nIndex++]), 1);
			//pCha->setAttr(ATTR_GUILD, Str2Int(g_buf[nIndex++]), 1);
			//pCha->setAttr(ATTR_GUILD_TYPE, game_db.GetGuildTypeByID(pCha, pCha->getAttr(ATTR_GUILD)), 1);

			pCha->setAttr(ATTR_HP, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_SP, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_CEXP, _atoi64(stringBufs[nIndex++].c_str()), 1);

			pCha->SetRadius(Str2Int(stringBufs[nIndex++]));
			pCha->SetAngle(Str2Int(stringBufs[nIndex++]));
			pCha->SetName((char*)stringBufs[nIndex++].c_str());
			Char szLogName[defLOG_NAME_LEN] = "";
			_snprintf_s(szLogName, sizeof(szLogName), _TRUNCATE, "Cha-%s+%u", pCha->GetName(), pCha->GetID());
			pCha->m_CLog.SetLogName(szLogName);

			pCha->SetMotto(stringBufs[nIndex++].c_str());
			pCha->SetIcon(Str2Int(stringBufs[nIndex++]));

			long lVer = Str2Int(stringBufs[nIndex++]);
			if (pCha->getAttr(ATTR_HP) < 0) { // 新角色
				lVer = defCHA_TABLE_NEW_VER;
			}
			pCha->SetPKCtrl(Str2Int(stringBufs[nIndex++]));

			strncpy_s(pCha->m_CChaAttr.m_szName, sizeof(pCha->m_CChaAttr.m_szName), pCha->GetName(), _TRUNCATE);

			pCha->setAttr(ATTR_LV, Str2Int(stringBufs[nIndex++]), 1);

			pCha->setAttr(ATTR_JOB, Str2Int(stringBufs[nIndex++]), 1);

			pCha->setAttr(ATTR_GD, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_AP, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_TP, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_BSTR, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_BDEX, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_BAGI, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_BCON, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_BSTA, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_BLUK, Str2Int(stringBufs[nIndex++]), 1);

			pCha->setAttr(ATTR_SAILLV, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_CSAILEXP, _atoi64(stringBufs[nIndex++].c_str()), 1);
			pCha->setAttr(ATTR_CLEFT_SAILEXP, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_LIFELV, Str2Int(stringBufs[nIndex++]), 1);
			pCha->setAttr(ATTR_CLIFEEXP, _atoi64(stringBufs[nIndex++].c_str()), 1);
			pCha->setAttr(ATTR_LIFETP, Str2Int(stringBufs[nIndex++]), 1);

			pCha->SetBirthMap(stringBufs[nIndex++].c_str());
			long lPosX = Str2Int(stringBufs[nIndex++]);
			long lPosY = Str2Int(stringBufs[nIndex++]);
			pCha->SetPos(lPosX, lPosY);
			pCha->SetBirthCity(stringBufs[nIndex++].c_str());
			//LG("enter_map", "设置角色基本数据成功.\n");

			try {
				int nLookDataID = nIndex;
				if (!pCha->String2LookDate(stringBufs[nIndex++])) {
					LG("enter_map", "Appearance data check sum error.\n");
					LG("Check sum error", "the character (dbid %u，name %s，resid %u)'s change appearance data check sum error.\n", cha_id, pCha->GetLogName(), pCha->GetKitbagRecDBID());
					return false;
				}
				pCha->SetCat(pCha->m_SChaPart.sTypeID);
				//LG("enter_map", "设置角色外观成功.\n");
			} catch (...) {
				LG("enter_map", "Strin2LookData error!!\n");
				LG("enter_map", "Appearance String %s\n", stringBufs[nIndex - 1]);
				throw;
			}

			int nSkillBagDataID = nIndex;
			String2SkillBagData(&pCha->m_CSkillBag, stringBufs[nIndex++]);
			//LG("enter_map", "设置角色技能栏成功.\n");

			int nSortcutDataID = nIndex;
			String2ShortcutData(&pCha->m_CShortcut, stringBufs[nIndex++]);
			//LG("enter_map", "设置角色快捷栏成功.\n");

			// 读取任务信息
			pPlayer->MisClear();

			char szMisInfo[ROLE_MAXSIZE_DBMISSION];
			memset(szMisInfo, 0, ROLE_MAXSIZE_DBMISSION);
			strncpy_s(szMisInfo, sizeof(szMisInfo), stringBufs[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInit(szMisInfo)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00009));
			}
			//LG("enter_map", "设置角色任务1成功.\n");

			char szRecord[ROLE_MAXSIZE_DBRECORD];
			memset(szRecord, 0, ROLE_MAXSIZE_DBRECORD);
			strncpy_s(szRecord, sizeof(szRecord), stringBufs[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInitRecord(szRecord)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00010));
			}
			//LG("enter_map", "设置角色任务2成功.\n");

			char szTrigger[ROLE_MAXSIZE_DBTRIGGER];
			memset(szTrigger, 0, ROLE_MAXSIZE_DBTRIGGER);
			strncpy_s(szTrigger, sizeof(szTrigger), stringBufs[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInitTrigger(szTrigger)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00011));
			}
			//LG("enter_map", "设置角色任务3成功.\n");
			char szMisCount[ROLE_MAXSIZE_DBMISCOUNT];
			memset(szMisCount, 0, ROLE_MAXSIZE_DBMISCOUNT);
			strncpy_s(szMisCount, sizeof(szMisCount), stringBufs[nIndex++].c_str(), _TRUNCATE);
			if (!pPlayer->MisInitMissionCount(szMisCount)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00012));
			}
			//LG("enter_map", "设置角色任务4成功.\n");

			std::string strList[2];
			Util_ResolveTextLine(stringBufs[nIndex++].c_str(), strList, 2, ',');
			pPlayer->SetLoginCha(Str2Int(strList[0]), Str2Int(strList[1]));

			//if (lVer != defCHA_TABLE_NEW_VER) // 需要版本转换
			//	SaveTableVer(cha_id);

			pCha->SetKitbagRecDBID(Str2Int(stringBufs[nIndex++]));

			pCha->SetKitbagTmpRecDBID(Str2Int(stringBufs[nIndex++])); //获取临时背包的资源ID

			pPlayer->SetMapMaskDBID(Str2Int(stringBufs[nIndex++]));
			g_strChaState[0] = stringBufs[nIndex++];
			pPlayer->Strin2BankDBIDData(stringBufs[nIndex++]);

			//背包密码锁
			const int iLocked = Str2Int(stringBufs[nIndex++]);
			pCha->GetKitbag()->SetPwdLockState(iLocked);

			//声望
			const int nCredit = Str2Int(stringBufs[nIndex++]);
			pCha->SetCredit(nCredit);

			pCha->SetStoreItemID(Str2Int(stringBufs[nIndex++]));

			Strin2ChaExtendAttr(pCha, stringBufs[nIndex++]);
#ifdef SHUI_JING
			pCha->SetFlatMoney(atoi(stringBufs[nIndex++].c_str()));
#endif
			LG("enter_map", "Set the whole data succeed.\n");
		} else {
			LG("enter_map", "Loading database error,_get_row() return value:%d.%u\n", r, r1);
			return false;
		}

		return true;
	}
	T_E
}

void CTableCha::OnlySavePosWhenBeSaved(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	CCharacter* pCCtrlCha = pPlayer->GetCtrlCha();
	if (!pCha || !pCCtrlCha) {
		return;
	}

	// 如果当前地图不保存则直接退出
	if (pCCtrlCha->GetSubMap()) {
		if (!pCCtrlCha->GetSubMap()->CanSavePos()) {
			return;
		}
	} else {
		return;
	}

	const DWORD cha_id = pPlayer->GetDBChaId();

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   map='%s', main_map='%s', map_x=%d, map_y=%d, angle=%d \
				   where cha_id=%d",
				_get_table(),
				pCCtrlCha->GetBirthMap(), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetAngle(),
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");

		return;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save position SQL sentence error!\n", pCha->GetLogName());
		return;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return;
	}
}

void CTableCha::OnlySavePosWhenBeSavedEx(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	CCharacter* pCCtrlCha = pPlayer->GetCtrlCha();
	if (!pCha || !pCCtrlCha) {
		return;
	}

	char sql[1024 * 1024];

	// 如果当前地图不保存则直接退出
	if (pCCtrlCha->GetSubMap()) {
		if (!pCCtrlCha->GetSubMap()->CanSavePos()) {
			return;
		}
	} else {
		return;
	}

	const DWORD cha_id = pPlayer->GetDBChaId();

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
				   map='%s', main_map='%s', map_x=%d, map_y=%d, angle=%d \
				   where cha_id=%d",
				_get_table(),
				pCCtrlCha->GetBirthMap(), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetAngle(),
				cha_id);

	if (strlen(sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");

		return;
	}

	const short sExec = exec_sql_direct(sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save position SQL sentence error!\n", pCha->GetLogName());
		return;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return;
	}
}

//-----------------
// 定时角色信息存盘
//-----------------
bool CTableCha::SaveAllData(CPlayer* pPlayer, char chSaveType, cChar* szTarMapName, Long lTarX, Long lTarY) {
	try {
		MONITOR

		if (!pPlayer || !pPlayer->IsValid()) {
			return false;
		}
		CCharacter* pCha = pPlayer->GetMainCha();
		if (!pCha) {
			return false;
		}
		const DWORD cha_id = pPlayer->GetDBChaId();

		CCharacter* pCCtrlCha = pPlayer->GetCtrlCha();
		if (pPlayer->GetLoginChaType() == enumLOGIN_CHA_BOAT) // 以船的形态登陆
		{
			CCharacter* pCLogCha = pPlayer->GetBoat(pPlayer->GetLoginChaID());
			if (pCLogCha != pCCtrlCha) { // 不应该出现的情况
				pCCtrlCha->SetToMainCha();
				pCCtrlCha = pCha;
				if (pCLogCha) {
					LG("logging character control error", "logging character %s,control character %s,Main character %s.\n", pCLogCha->GetLogName(), pCCtrlCha->GetLogName(), pCha->GetLogName());
				} else {
					LG("logging character control error", "logging character %s,control character %s,Main character %s.\n", "", pCCtrlCha->GetLogName(), pCha->GetLogName());
				}
				return false;
			}
		} else {
			if (pCha != pCCtrlCha) { // 不应该出现的情况
				pCCtrlCha = pCha;
				LG("logging character control error", "logging character %s,control character %s,Main character %s.\n", pCCtrlCha->GetLogName(), pCCtrlCha->GetLogName(), pCha->GetLogName());
				return false;
			}
		}

		if (pCha) {
			LG("enter_map", "%s start configure save data.\n", pCha->GetLogName());
			pCha->m_CLog.Log("........... now you start save character\n");
			pCha->m_CLog.Log("grade %d,map %s,coordinate [%d,%d],birth city %s.\n", (int)pCha->m_CChaAttr.GetAttr(ATTR_LV), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetBirthCity());
		}

		const DWORD hp = (long)pCha->getAttr(ATTR_HP);
		const DWORD sp = (long)pCha->getAttr(ATTR_SP);
		const LONG64 exp = pCha->getAttr(ATTR_CEXP);

		//szTarMapName, Long lTarX, Long lTarY
		const char* map = pCCtrlCha->GetBirthMap();
		const char* main_map = pCha->GetBirthMap();
		DWORD map_x = pCha->GetShape().centre.x;
		DWORD map_y = pCha->GetShape().centre.y;

		// 如果是切换地图先保存地址再说
		if (chSaveType == enumSAVE_TYPE_SWITCH) {
			map = szTarMapName;
			main_map = szTarMapName;
			map_x = lTarX;
			map_y = lTarY;
		}

		const DWORD radius = pCha->GetShape().radius;
		const short angle = pCha->GetAngle();
		const short degree = (short)pCha->getAttr(ATTR_LV);
		const short job = (short)pCha->getAttr(ATTR_JOB);

		const DWORD gd = (long)pCha->getAttr(ATTR_GD);
		const DWORD ap = (long)pCha->getAttr(ATTR_AP);
		const DWORD tp = (long)pCha->getAttr(ATTR_TP);
		const DWORD str = (long)pCha->getAttr(ATTR_BSTR);
		const DWORD dex = (long)pCha->getAttr(ATTR_BDEX);
		const DWORD agi = (long)pCha->getAttr(ATTR_BAGI);
		const DWORD con = (long)pCha->getAttr(ATTR_BCON);
		const DWORD sta = (long)pCha->getAttr(ATTR_BSTA);
		const DWORD luk = (long)pCha->getAttr(ATTR_BLUK);

		const DWORD sail_lv = (long)pCha->getAttr(ATTR_SAILLV);
		const DWORD sail_exp = (long)pCha->getAttr(ATTR_CSAILEXP);
		const DWORD sail_left_exp = (long)pCha->getAttr(ATTR_CLEFT_SAILEXP);
		const DWORD live_lv = (long)pCha->getAttr(ATTR_LIFELV);
		const DWORD live_exp = (long)pCha->getAttr(ATTR_CLIFEEXP);
		const DWORD live_tp = (long)pCha->getAttr(ATTR_LIFETP);

		const DWORD nLocked = pCha->GetKitbag()->GetPwdLockState();

		const DWORD dwCredit = (long)pCha->GetCredit();
		const DWORD dwStoreItemID = pCha->GetStoreItemID();

		const bool pk_ctrl = pCha->IsInPK();

		{
			MONITOR_STEP(LookData2String)
			g_look[defLOOK_DATA_STRING_LEN - 1] = 0;
			if (!LookData2String(&pCha->m_SChaPart, g_look, defLOOK_DATA_STRING_LEN, false)) {
				LG("enter_map", "character %s\tsave data (surface) error!\n", pCha->GetLogName());
				return false;
			}
			//LG("enter_map", "配置角色外观成功.\n");
		}

		{
			MONITOR_STEP(SkillBagData2String)
			g_skillbag[defSKILLBAG_DATA_STRING_LEN - 1] = 0;
			if (!SkillBagData2String(&pCha->m_CSkillBag, g_skillbag, defSKILLBAG_DATA_STRING_LEN)) {
				LG("enter_map", "character %s\tsave data(skill) error!\n", pCha->GetLogName());
				return false;
			}
			//LG("enter_map", "配置角色技能栏成功.\n");
		}

		{
			MONITOR_STEP(ShortcutData2String)
			g_shortcut[defSHORTCUT_DATA_STRING_LEN - 1] = 0;
			if (!ShortcutData2String(&pCha->m_CShortcut, g_shortcut, defSHORTCUT_DATA_STRING_LEN)) {
				LG("enter_map", "character %s\tsave data(shortcut)error!\n", pCha->GetLogName());
				return false;
			}
			//LG("enter_map", "配置角色快捷栏成功.\n");
		}

		{
			MONITOR_STEP(MisGetData)
			// 保存角色任务记录信息
			//memset( g_szMisInfo, 0, ROLE_MAXSIZE_DBMISSION );
			if (!pPlayer->MisGetData(g_szMisInfo, ROLE_MAXSIZE_DBMISSION - 1)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00015), pCha->GetID());
				LG(szDBLog, "save character[ID: %d\tNAME: %s]data info,Get mission data error!ID = %d\n", cha_id, pCha->GetName(), pCha->GetID());
			}
			//LG("enter_map", "配置角色任务1成功.\n");

			//memset( g_szRecord, 0, ROLE_MAXSIZE_DBRECORD );
			if (!pPlayer->MisGetRecord(g_szRecord, ROLE_MAXSIZE_DBRECORD - 1)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00015), pCha->GetID());
				LG(szDBLog, "save character[ID: %d\tNAME: %s]data info,Get mission history data error !ID = %d\n", cha_id, pCha->GetName(), pCha->GetID());
			}
			//LG("enter_map", "配置角色任务2成功.\n");

			//memset( g_szTrigger, 0, ROLE_MAXSIZE_DBTRIGGER );
			if (!pPlayer->MisGetTrigger(g_szTrigger, ROLE_MAXSIZE_DBTRIGGER - 1)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00016), pCha->GetID());
				LG(szDBLog, "save character[ID: %d\tNAME: %s]data info，Get mission trigger data error!ID = %d\n", cha_id, pCha->GetName(), pCha->GetID());
			}
			//LG("enter_map", "配置角色任务3成功.\n");

			//memset( g_szMisCount, 0, ROLE_MAXSIZE_DBMISCOUNT );
			if (!pPlayer->MisGetMissionCount(g_szMisCount, ROLE_MAXSIZE_DBMISCOUNT - 1)) {
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00017), pCha->GetID());
				LG(szDBLog, "save character[ID: %d\tNAME: %s]data info，Get randomicity mission take count of data error!ID = %d\n", cha_id, pCha->GetName(), pCha->GetID());
			}
			//LG("enter_map", "配置角色任务4成功.\n");
		}

		const char* szBirthName = pCha->GetBirthCity();

		char szLoginCha[50];
		_snprintf_s(szLoginCha, sizeof(szLoginCha), _TRUNCATE, "%u,%u", pPlayer->GetLoginChaType(), pPlayer->GetLoginChaID());

		{
			MONITOR_STEP(SStateData2String)

			if (chSaveType == enumSAVE_TYPE_OFFLINE) { // 下线
				g_skillstate[0] = '\0';
			} else if (!SStateData2String(pCha, g_skillstate, defSSTATE_DATE_STRING_LIN)) {
				LG("enter_map", "character %s\tsave data(shortcut)error!\n", pCha->GetLogName());
				return false;
			}

			//memset(g_extendAttr, 0 , ROLE_MAXSIZE_DBMISCOUNT);
			if (!ChaExtendAttr2String(pCha, g_extendAttr, ROLE_MAXSIZE_DBMISCOUNT)) {
				LG("enter_map", "character %s\tsave data (extend attr) error!\n", pCha->GetLogName());
				return false;
			}
		}

		char str_exp[32];
		_i64toa_s(exp, str_exp, sizeof(str_exp), 10);

		bool bWithPos = false;
		if (chSaveType == enumSAVE_TYPE_SWITCH) {
			bWithPos = true;
		} else if (pCCtrlCha->GetSubMap()) {
			bWithPos = pCCtrlCha->GetSubMap()->CanSavePos();
		}

		// Add by lark.li 20090311 begin
		//memset(g_sql, 0 , sizeof(g_sql));
		// End
#ifdef SHUI_JING
		const DWORD flatmoney = pCha->GetFlatMoney();
#endif
		if (bWithPos) {
#ifndef SHUI_JING
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
					   hp=%d, sp=%d, exp=%s, map='%s', main_map='%s', map_x=%d, map_y=%d, radius=%d, angle=%d, pk_ctrl=%d, degree=%d, job=%d, gd=%d, ap=%d, tp=%d, str=%d, dex=%d, agi=%d, con=%d, sta=%d, luk=%d, look='%s', skillbag='%s', \
					   shortcut='%s', mission='%s', misrecord='%s', mistrigger='%s', miscount='%s', birth='%s', login_cha='%s', \
					   sail_lv=%d, sail_exp=%d, sail_left_exp=%d, live_lv=%d, live_exp=%d, live_tp=%d, kb_locked=%d, credit=%d, store_item=%d, skill_state='%s', extend ='%s' \
					   where cha_id=%d",
						_get_table(),
						hp, sp, str_exp, map, main_map, map_x, map_y, radius, angle, pk_ctrl, degree,
						job, gd, ap, tp, str, dex, agi, con, sta, luk, g_look, g_skillbag, g_shortcut,
						g_szMisInfo, g_szRecord, g_szTrigger, g_szMisCount, szBirthName, szLoginCha,
						sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, nLocked,
						dwCredit, dwStoreItemID, g_skillstate, g_extendAttr, cha_id);
		}
#else
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
			   hp=%d, sp=%d, exp=%s, map='%s', main_map='%s', map_x=%d, map_y=%d, radius=%d, angle=%d, pk_ctrl=%d, degree=%d, job=%d, gd=%d, ap=%d, tp=%d, str=%d, dex=%d, agi=%d, con=%d, sta=%d, luk=%d, look='%s', skillbag='%s', \
			   shortcut='%s', mission='%s', misrecord='%s', mistrigger='%s', miscount='%s', birth='%s', login_cha='%s', \
			   sail_lv=%d, sail_exp=%d, sail_left_exp=%d, live_lv=%d, live_exp=%d, live_tp=%d, kb_locked=%d, credit=%d, store_item=%d, skill_state='%s', extend ='%s', flatcoins = %d \
			   where cha_id=%d",
						_get_table(),
						hp, sp, str_exp, map, main_map, map_x, map_y, radius, angle, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, g_look, g_skillbag, g_shortcut, g_szMisInfo, g_szRecord, g_szTrigger, g_szMisCount, szBirthName, szLoginCha, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, nLocked, dwCredit, dwStoreItemID, g_skillstate, g_extendAttr, flatmoney,
						cha_id);
		}
#endif
		else {
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
					   hp=%d, sp=%d, exp=%s, radius=%d, pk_ctrl=%d, degree=%d, job=%d, gd=%d, ap=%d, tp=%d, str=%d, dex=%d, agi=%d, con=%d, sta=%d, luk=%d, look='%s', skillbag='%s', \
					   shortcut='%s', mission='%s', misrecord='%s', mistrigger='%s', miscount='%s', birth='%s', login_cha='%s', \
					   sail_lv=%d, sail_exp=%d, sail_left_exp=%d, live_lv=%d, live_exp=%d, live_tp=%d, kb_locked=%d, credit=%d, store_item=%d, skill_state='%s', extend ='%s' \
					   where cha_id=%d",
						_get_table(),
						hp, sp, str_exp, radius, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, g_look, g_skillbag, g_shortcut, g_szMisInfo, g_szRecord, g_szTrigger, g_szMisCount, szBirthName, szLoginCha, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, nLocked, dwCredit, dwStoreItemID, g_skillstate, g_extendAttr,
						cha_id);
		}

		//LG("enter_map", "组织SQL语句成功.\n");

		if (strlen(g_sql) >= SQL_MAXLEN) {
			FILE* pf = nullptr;
			if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
				LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
				return false;
			}

			if (pf) {
				fprintf(pf, "%s\n\n", g_sql);
				fclose(pf);
			}
			LG("enter_map", "SQL sentence length slop over\n");
			return false;
		}
		const short sExec = exec_sql_direct(g_sql);
		if (!DBOK(sExec)) {
			LG("enter_map", "character %s\tcarry out SQL sentence error!\n", pCha->GetLogName());
			return false;
		}
		if (DBNODATA(sExec)) {
			LG("enter_map", "Database couldn't find the character %u!\n", cha_id);
			return false;
		}
		//LG("enter_map", "执行保存SQL语句成功.\n");

		//LG(szSaveCha, szLogMsg);

		// 先不上公服20090413 lark.li
		//Add by sunny.sun 20090310
		//if( chSaveType == enumSAVE_TYPE_OFFLINE )
		//	game_db.UpdateOutMapTime( pPlayer->GetDBChaId() );

		pCha->m_CLog.Log("...............you finish save the character \n");
		//pCha->SystemNotice("你的角色成功保存到数据库，等级 %d，地图 %s，坐标 [%d,%d]，出生地 %s.\n", pCha->m_CChaAttr.GetAttr(ATTR_LV), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetBirthCity());
		LG("enter_map", "save the main character whole data succeed!\n", pCha->GetLogName());

		return true;
	}
	T_E
}

bool CTableCha::SaveAllDataEx(CPlayer* pPlayer, char chSaveType) {
	try {
		MONITOR

		if (!pPlayer || !pPlayer->IsValid()) {
			return false;
		}
		CCharacter* pCha = pPlayer->GetMainCha();
		if (!pCha) {
			return false;
		}
		const DWORD cha_id = pPlayer->GetDBChaId();

		CCharacter* pCCtrlCha = pPlayer->GetCtrlCha();
		if (pPlayer->GetLoginChaType() == enumLOGIN_CHA_BOAT) { // 以船的形态登陆
			CCharacter* pCLogCha = pPlayer->GetBoat(pPlayer->GetLoginChaID());
			if (pCLogCha != pCCtrlCha) { // 不应该出现的情况
				pCCtrlCha->SetToMainCha();
				pCCtrlCha = pCha;
				if (pCLogCha) {
					LG("logging character control error", "logging character %s,control character %s,Main character %s.\n", pCLogCha->GetLogName(), pCCtrlCha->GetLogName(), pCha->GetLogName());
				} else {
					LG("logging character control error", "logging character %s,control character %s,Main character %s.\n", "", pCCtrlCha->GetLogName(), pCha->GetLogName());
				}
				return false;
			}
		} else {
			if (pCha != pCCtrlCha) { // 不应该出现的情况
				pCCtrlCha = pCha;
				LG("logging character control error", "logging character %s,control character %s,Main character %s.\n", pCCtrlCha->GetLogName(), pCCtrlCha->GetLogName(), pCha->GetLogName());
				return false;
			}
		}

		if (pCha) {
			LG("enter_map", "%s start configure save data.\n", pCha->GetLogName());
			pCha->m_CLog.Log("........... now you start save character\n");
			pCha->m_CLog.Log("grade %d,map %s,coordinate [%d,%d],birth city %s.\n", (int)pCha->m_CChaAttr.GetAttr(ATTR_LV), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetBirthCity());
		}

		const DWORD hp = (long)pCha->getAttr(ATTR_HP);
		const DWORD sp = (long)pCha->getAttr(ATTR_SP);
		const LONG64 exp = pCha->getAttr(ATTR_CEXP);

		const char* map = pCCtrlCha->GetBirthMap();
		const char* main_map = pCha->GetBirthMap();
		const DWORD map_x = pCha->GetShape().centre.x;
		const DWORD map_y = pCha->GetShape().centre.y;
		const DWORD radius = pCha->GetShape().radius;
		const short angle = pCha->GetAngle();
		const short degree = (short)pCha->getAttr(ATTR_LV);
		const short job = (short)pCha->getAttr(ATTR_JOB);

		const DWORD gd = (long)pCha->getAttr(ATTR_GD);
		const DWORD ap = (long)pCha->getAttr(ATTR_AP);
		const DWORD tp = (long)pCha->getAttr(ATTR_TP);
		const DWORD str = (long)pCha->getAttr(ATTR_BSTR);
		const DWORD dex = (long)pCha->getAttr(ATTR_BDEX);
		const DWORD agi = (long)pCha->getAttr(ATTR_BAGI);
		const DWORD con = (long)pCha->getAttr(ATTR_BCON);
		const DWORD sta = (long)pCha->getAttr(ATTR_BSTA);
		const DWORD luk = (long)pCha->getAttr(ATTR_BLUK);

		const DWORD sail_lv = (long)pCha->getAttr(ATTR_SAILLV);
		const DWORD sail_exp = (long)pCha->getAttr(ATTR_CSAILEXP);
		const DWORD sail_left_exp = (long)pCha->getAttr(ATTR_CLEFT_SAILEXP);
		const DWORD live_lv = (long)pCha->getAttr(ATTR_LIFELV);
		const DWORD live_exp = (long)pCha->getAttr(ATTR_CLIFEEXP);
		const DWORD live_tp = (long)pCha->getAttr(ATTR_LIFETP);

		const DWORD nLocked = pCha->GetKitbag()->GetPwdLockState();

		const DWORD dwCredit = (long)pCha->GetCredit();
		const DWORD dwStoreItemID = pCha->GetStoreItemID();

		const bool pk_ctrl = pCha->IsInPK();

		//LG("enter_map", "配置角色基本数据成功.\n");

		//char look[2048];
		StringPoolL stringPool;
		stringPool.ReserveBuf(StringPoolL::AllocateSpace1);
		stringPool.ReserveNum(StringPoolL::AllocateSpace2);
		//memset(look, 0, sizeof(look));
		stringPool[0] = "";

		//if(!LookData2String(&pCha->m_SChaPart, look, defLOOK_DATA_STRING_LEN, false))
		if (!LookData2StringEx(&pCha->m_SChaPart, stringPool, 0, false)) {
			LG("enter_map", "character %s\tsave data (surface) error!\n",
			   pCha->GetLogName());
			return false;
		}
		//LG("enter_map", "配置角色外观成功.\n");

		//char skillbag[1500];
		//memset(skillbag, 0, sizeof(skillbag));
		stringPool[1] = "";
		//if (!SkillBagData2String(&pCha->m_CSkillBag, skillbag, defSKILLBAG_DATA_STRING_LEN))
		if (!SkillBagData2StringEx(&pCha->m_CSkillBag, stringPool, 1)) {
			LG("enter_map", "character %s\tsave data(skill) error!\n",
			   pCha->GetLogName());
			return false;
		}
		//LG("enter_map", "配置角色技能栏成功.\n");

		//char shortcut[1500];
		//memset(shortcut, 0, sizeof(shortcut));
		stringPool[2] = "";
		if (!ShortcutData2StringEx(&pCha->m_CShortcut, stringPool, 2)) {
			LG("enter_map", "character %s\tsave data(shortcut)error!\n",
			   pCha->GetLogName());
			return false;
		}
		//LG("enter_map", "配置角色快捷栏成功.\n");

		// 保存角色任务记录信息
		//char szMisInfo[ROLE_MAXSIZE_DBMISSION];
		//memset( szMisInfo, 0, ROLE_MAXSIZE_DBMISSION );
		stringPool[3] = "";
		if (!pPlayer->MisGetDataEx(stringPool, 3)) {
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00015), pCha->GetID());
			LG(szDBLog, "save character[ID: %d\tNAME: %s]data info,Get mission data error!ID = %d\n",
			   cha_id, pCha->GetName(), pCha->GetID());
		}
		//LG("enter_map", "配置角色任务1成功.\n");

		//char szRecord[ROLE_MAXSIZE_DBRECORD];
		//memset( szRecord, 0, ROLE_MAXSIZE_DBRECORD );
		stringPool[4] = "";
		if (!pPlayer->MisGetRecordEx(stringPool, 4)) {
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00015), pCha->GetID());
			LG(szDBLog, "save character[ID: %d\tNAME: %s]data info,Get mission history data error !ID = %d\n",
			   cha_id, pCha->GetName(), pCha->GetID());
		}
		//LG("enter_map", "配置角色任务2成功.\n");

		//char szTrigger[ROLE_MAXSIZE_DBTRIGGER];
		//memset( szTrigger, 0, ROLE_MAXSIZE_DBTRIGGER );
		stringPool[5] = "";
		if (!pPlayer->MisGetTriggerEx(stringPool, 5)) {
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00016), pCha->GetID());
			LG(szDBLog, "save character[ID: %d\tNAME: %s]data info，Get mission trigger data error!ID = %d\n",
			   cha_id, pCha->GetName(), pCha->GetID());
		}
		//LG("enter_map", "配置角色任务3成功.\n");

		//char szMisCount[ROLE_MAXSIZE_DBMISCOUNT];
		//memset( szMisCount, 0, ROLE_MAXSIZE_DBMISCOUNT );
		stringPool[6] = "";
		if (!pPlayer->MisGetMissionCountEx(stringPool, 6)) {
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00017), pCha->GetID());
			LG(szDBLog, "save character[ID: %d\tNAME: %s]data info，Get randomicity mission take count of data error!ID = %d\n",
			   cha_id, pCha->GetName(), pCha->GetID());
		}
		//LG("enter_map", "配置角色任务4成功.\n");

		const char* szBirthName = pCha->GetBirthCity();

		char szLoginCha[50];
		_snprintf_s(szLoginCha, sizeof(szLoginCha), _TRUNCATE, "%u,%u",
					pPlayer->GetLoginChaType(), pPlayer->GetLoginChaID());

		//char skillstate[defSSTATE_DATE_STRING_LIN];
		//memset(skillstate, 0, defSSTATE_DATE_STRING_LIN);
		stringPool[7] = "";
		if (chSaveType != enumSAVE_TYPE_OFFLINE) // 下线
		{
			if (!SStateData2StringEx(pCha, stringPool, 7)) {
				LG("enter_map", "character %s\tsave data(shortcut)error!\n", pCha->GetLogName());
				return false;
			}
		}
		// Add by lark.li 20080723 begin
		//char extendAttr[ROLE_MAXSIZE_DBMISCOUNT];
		//memset(extendAttr, 0 , ROLE_MAXSIZE_DBMISCOUNT);
		stringPool[8] = "";
		if (!ChaExtendAttr2StringEx(pCha, stringPool, 8)) {
			LG("enter_map", "character %s\tsave data (extend attr) error!\n", pCha->GetLogName());
			return false;
		}

		// End

		char str_exp[32];
		//_i64toa( exp, str_exp, 10 ); // C4996
		_i64toa_s(exp, str_exp, sizeof(str_exp), 10); // C4996

		bool bWithPos = false;
		if (pCCtrlCha->GetSubMap())
			bWithPos = pCCtrlCha->GetSubMap()->CanSavePos();

		char sql[SQL_MAXLEN];

		if (bWithPos) {
			_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
											   hp=%d, sp=%d, exp=%s, map='%s', main_map='%s', map_x=%d, map_y=%d, radius=%d, angle=%d, pk_ctrl=%d, degree=%d, job=%d, gd=%d, ap=%d, tp=%d, str=%d, dex=%d, agi=%d, con=%d, sta=%d, luk=%d, look='%s', skillbag='%s', \
											   shortcut='%s', mission='%s', misrecord='%s', mistrigger='%s', miscount='%s', birth='%s', login_cha='%s', \
											   sail_lv=%d, sail_exp=%d, sail_left_exp=%d, live_lv=%d, live_exp=%d, live_tp=%d, kb_locked=%d, credit=%d, store_item=%d, skill_state='%s', extend ='%s' \
											   where cha_id=%d",
						_get_table(),
						hp, sp, str_exp, map, main_map, map_x, map_y, radius, angle, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, (const char*)stringPool[0], (const char*)stringPool[1], (const char*)stringPool[2],
						(const char*)stringPool[3], (const char*)stringPool[4], (const char*)stringPool[5], (const char*)stringPool[6], szBirthName, szLoginCha, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, nLocked, dwCredit, dwStoreItemID,
						(const char*)stringPool[7], (const char*)stringPool[8],
						cha_id);
		} else {
			_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
					   hp=%d, sp=%d, exp=%s, radius=%d, pk_ctrl=%d, degree=%d, job=%d, gd=%d, ap=%d, tp=%d, str=%d, dex=%d, agi=%d, con=%d, sta=%d, luk=%d, look='%s', skillbag='%s', \
					   shortcut='%s', mission='%s', misrecord='%s', mistrigger='%s', miscount='%s', birth='%s', login_cha='%s', \
					   sail_lv=%d, sail_exp=%d, sail_left_exp=%d, live_lv=%d, live_exp=%d, live_tp=%d, kb_locked=%d, credit=%d, store_item=%d, skill_state='%s', extend ='%s' \
					   where cha_id=%d",
						_get_table(),
						hp, sp, str_exp, radius, pk_ctrl, degree, job, gd, ap, tp, str, dex, agi, con, sta, luk, (const char*)stringPool[0], (const char*)stringPool[1], (const char*)stringPool[2],
						(const char*)stringPool[3], (const char*)stringPool[4], (const char*)stringPool[5], (const char*)stringPool[6], szBirthName, szLoginCha, sail_lv, sail_exp, sail_left_exp, live_lv, live_exp, live_tp, nLocked, dwCredit, dwStoreItemID,
						(const char*)stringPool[7], (const char*)stringPool[8],
						cha_id);
		}

		const short sExec = exec_sql_direct(sql);
		if (!DBOK(sExec)) {
			LG("enter_map", "character %s\tcarry out SQL sentence error!\n", pCha->GetLogName());
			return false;
		}
		if (DBNODATA(sExec)) {
			LG("enter_map", "Database couldn't find the character %u!\n", cha_id);
			return false;
		}
		//LG("enter_map", "执行保存SQL语句成功.\n");

		// 先不上公服20090413 lark.li
		//Add by sunny.sun 20090310
		//if( chSaveType == enumSAVE_TYPE_OFFLINE )
		//	game_db.UpdateOutMapTime( pPlayer->GetDBChaId() );

		pCha->m_CLog.Log("...............you finish save the character \n");
		//pCha->SystemNotice("你的角色成功保存到数据库，等级 %d，地图 %s，坐标 [%d,%d]，出生地 %s.\n", pCha->m_CChaAttr.GetAttr(ATTR_LV), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetBirthCity());
		LG("enter_map", "save the main character whole data succeed!\n", pCha->GetLogName());

		return true;
	}
	T_E
}

bool CTableCha::SavePos(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	CCharacter* pCCtrlCha = pPlayer->GetCtrlCha();
	if (!pCha || !pCCtrlCha) {
		return false;
	}

	const DWORD cha_id = pPlayer->GetDBChaId();
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   map='%s', main_map='%s', map_x=%d, map_y=%d, angle=%d \
				   where cha_id=%d",
				_get_table(),
				pCCtrlCha->GetBirthMap(), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetAngle(),
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");

		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save position SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SavePosEx(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	CCharacter* pCCtrlCha = pPlayer->GetCtrlCha();
	if (!pCha || !pCCtrlCha) {
		return false;
	}
	const DWORD cha_id = pPlayer->GetDBChaId();

	StringPoolL stringPool;
	stringPool.ReserveBuf(StringPoolL::AllocateSpace1);
	stringPool.ReserveNum(StringPoolL::AllocateSpace2);
	//memset(look, 0, sizeof(look));
	stringPool[0] = "";
	//sprintf(g_sql, "update %s set \
	//			map='%s', main_map='%s', map_x=%d, map_y=%d, angle=%d \
	//			where cha_id=%d", \
	//			_get_table(), \
	//			pCCtrlCha->GetBirthMap(), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetAngle(), \
	//			cha_id);
	stringPool[0].Printf("update %s set \
				   map='%s', main_map='%s', map_x=%d, map_y=%d, angle=%d \
				   where cha_id=%d",
						 _get_table(),
						 pCCtrlCha->GetBirthMap(), pCha->GetBirthMap(), pCha->GetPos().x, pCha->GetPos().y, pCha->GetAngle(),
						 cha_id);

	if (stringPool[0].Len() >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", (const char*)stringPool[0]);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");

		return false;
	}

	const short sExec = exec_sql_direct((const char*)stringPool[0]);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save position SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveMoney(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	if (!pCha) {
		return false;
	}
	const DWORD cha_id = pPlayer->GetDBChaId();

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   gd=%d \
				   where cha_id=%d",
				_get_table(),
				(int)pCha->getAttr(ATTR_GD),
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save money SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveKBagDBIDEx(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	if (!pCha) {
		return false;
	}
	const DWORD cha_id = pPlayer->GetDBChaId();

	char sql[1024 * 1024];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
				   kitbag=%d \
				   where cha_id=%d",
				_get_table(),
				pCha->GetKitbagRecDBID(),
				cha_id);

	if (strlen(sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}

	const short sExec = exec_sql_direct(sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character%s\tcarry out save kitbag indexical SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveKBagDBID(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	if (!pCha) {
		return false;
	}
	const DWORD cha_id = pPlayer->GetDBChaId();

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   kitbag=%d \
				   where cha_id=%d",
				_get_table(),
				pCha->GetKitbagRecDBID(),
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character%s\tcarry out save kitbag indexical SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveKBagTmpDBID(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	if (!pCha) {
		return false;
	}
	const DWORD cha_id = pPlayer->GetDBChaId();

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   kitbag_tmp=%d \
				   where cha_id=%d",
				_get_table(),
				pCha->GetKitbagTmpRecDBID(),
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save temp kitbag indexical SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveKBagTmpDBIDEx(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	if (!pCha) {
		return false;
	}
	const DWORD cha_id = pPlayer->GetDBChaId();

	char sql[1024 * 1024];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
				   kitbag_tmp=%d \
				   where cha_id=%d",
				_get_table(),
				pCha->GetKitbagTmpRecDBID(),
				cha_id);

	if (strlen(sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}

	const short sExec = exec_sql_direct(sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save temp kitbag indexical SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveKBState(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	CCharacter* pCha = pPlayer->GetMainCha();
	if (!pCha) {
		return false;
	}

	const DWORD cha_id = pPlayer->GetDBChaId();
	const int iLocked = pCha->GetKitbag()->GetPwdLockState();
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   kb_locked=%d \
				   where cha_id=%d",
				_get_table(),
				iLocked,
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %s\tcarry out save kitbag lock state SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

BOOL CTableCha::SaveStoreItemID(DWORD cha_id, long lStoreItemID) {
	if (cha_id == 0) {
		return false;
	}

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   store_item=%d \
				   where cha_id=%d",
				_get_table(),
				lStoreItemID,
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return FALSE;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}

	return true;
}

BOOL CTableCha::AddMoney(DWORD cha_id, long money) {
	if (cha_id == 0) {
		return false;
	}

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   gd=gd+%d \
				   where cha_id=%d",
				_get_table(),
				money,
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return FALSE;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}

	return true;
}

BOOL CTableCha::AddCreditByDBID(DWORD cha_id, long lCredit) {
	if (cha_id == 0) {
		return false;
	}

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   credit=credit+%d \
				   where cha_id=%d",
				_get_table(),
				lCredit,
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return FALSE;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}

	return true;
}

BOOL CTableCha::IsChaOnline(DWORD cha_id, BOOL& bOnline) {
	if (cha_id == 0) {
		return false;
	}

	BOOL ret = false;

	long lMemAddr = 0;

	const char* sql_syntax = "select mem_addr from %s where cha_id=%d";
	char sql[SQL_MAXLEN];
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, sql_syntax, _get_table(), cha_id);

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);

			throw 1;
		}

		sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
		if (sqlret != SQL_SUCCESS) {
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);

			if (sqlret != SQL_SUCCESS_WITH_INFO)
				throw 2;
		}

		sqlret = SQLNumResultCols(hstmt, &col_num);
		col_num = min(col_num, MAX_COL);
		col_num = min(col_num, _max_col);

		// Bind Column
		for (int i = 0; i < col_num; ++i) {
			SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
		}

		// Fetch each Row	int i; // 取出的行数
		for (int f_row = 0; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++f_row) {
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			}

			lMemAddr = atol((char const*)_buf[0]);
		}
		SQLFreeStmt(hstmt, SQL_UNBIND);
		ret = true;
	} catch (int& e) {
		LG("Store_msg", "IsChaOnline ODBC interface transfer error ,position ID：%d\n", e);
	} catch (...) {
		LG("Store_msg", "Unknown Exception raised when IsChaOnline\n");
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	if (lMemAddr > 0) {
		bOnline = true;
	} else {
		bOnline = false;
	}

	return ret;
}

bool CTableCha::SaveMMaskDBID(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}
	const DWORD cha_id = pPlayer->GetDBChaId();

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   map_mask=%d \
				   where cha_id=%d",
				_get_table(),
				pPlayer->GetMapMaskDBID(),
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %d\tcarry out save big map indexical SQL senternce error!\n", pPlayer->GetDBChaId());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveBankDBID(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}

	const short csIDBufLen = 200;
	char szIDBuf[csIDBufLen];
	if (!pPlayer->BankDBIDData2String(szIDBuf, csIDBufLen)) {
		return false;
	}

	const DWORD cha_id = pPlayer->GetDBChaId();
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   bank=%s \
				   where cha_id=%d",
				_get_table(),
				szIDBuf,
				cha_id);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");

		return false;
	}

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %d\tcarry out save bank indexcial SQL sentence error!\n", pPlayer->GetDBChaId());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveBankDBIDEx(CPlayer* pPlayer) {
	if (!pPlayer || !pPlayer->IsValid()) {
		return false;
	}

	const short csIDBufLen = 200;
	char szIDBuf[csIDBufLen];
	if (!pPlayer->BankDBIDData2String(szIDBuf, csIDBufLen))
		return false;

	char sql[1024 * 1024];

	const DWORD cha_id = pPlayer->GetDBChaId();
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
				   bank=%s \
				   where cha_id=%d",
				_get_table(),
				szIDBuf,
				cha_id);

	if (strlen(sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");

		return false;
	}

	const short sExec = exec_sql_direct(sql);
	if (!DBOK(sExec)) {
		LG("enter_map", "character %d\tcarry out save bank indexcial SQL sentence error!\n", pPlayer->GetDBChaId());
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("enter_map", "Database couldn't find the character%u!\n", cha_id);
		return false;
	}

	return true;
}

bool CTableCha::SaveTableVer(DWORD cha_id) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   version=%d \
				   where cha_id=%d",
				_get_table(),
				defCHA_TABLE_NEW_VER,
				cha_id);

	const short sExec = exec_sql_direct(g_sql);

	return DBOK(sExec) && !DBNODATA(sExec);
}

BOOL CTableCha::SaveMissionData(CPlayer* pPlayer, DWORD cha_id) {
	try {
		if (!pPlayer) {
			return FALSE;
		}
		CCharacter* pCha = pPlayer->GetMainCha();
		if (!pCha) {
			return FALSE;
		}

		// 保存角色任务记录信息
		memset(g_szMisInfo, 0, ROLE_MAXSIZE_DBMISSION);
		if (!pPlayer->MisGetData(g_szMisInfo, ROLE_MAXSIZE_DBMISSION - 1)) {
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00018), pCha->GetID());
			LG(szDBLog, "SaveMissionData:save character[ID: %d\tNAME: %s]data info,Get mission data error!ID = %d\n", cha_id, pCha->GetName(), pCha->GetID());
		}

		memset(g_szRecord, 0, ROLE_MAXSIZE_DBRECORD);
		if (!pPlayer->MisGetRecord(g_szRecord, ROLE_MAXSIZE_DBRECORD - 1)) {
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00018), pCha->GetID());
			LG(szDBLog, "SaveMissionData:save character[ID: %d\tNAME: %s]data info,Get mission history data error !ID = %d\n", cha_id, pCha->GetName(), pCha->GetID());
		}

		memset(g_szTrigger, 0, ROLE_MAXSIZE_DBTRIGGER);
		if (!pPlayer->MisGetTrigger(g_szTrigger, ROLE_MAXSIZE_DBTRIGGER - 1)) {
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00019), pCha->GetID());
			LG(szDBLog, "SaveMissionData:save character[ID: %d\tNAME: %s]data info，Get mission trigger data error!ID = %d\n", cha_id, pCha->GetName(), pCha->GetID());
		}

		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set mission='%s', misrecord='%s', mistrigger='%s' \
					where cha_id=%d",
					_get_table(), g_szMisInfo, g_szRecord, g_szTrigger, cha_id);

		if (strlen(g_sql) >= SQL_MAXLEN) {
			FILE* pf = nullptr;
			if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
				LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
				return FALSE;
			}
			if (pf) {
				fprintf(pf, "%s\n\n", g_sql);
				fclose(pf);
			}
			LG("enter_map", "SQL sentence length slop over\n");
			return FALSE;
		}
		const short sExec = exec_sql_direct(g_sql);
		return DBOK(sExec) && !DBNODATA(sExec);
	}
	T_E
}

//Add by sunny.sun 20090310
BOOL CTableCha::UpdateEnterMapTime(DWORD cha_id) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set entermaptime = getdate() where cha_id = %d", _get_table(), cha_id);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG2("distime", "update player[id] = %d entermaptime error! \n", cha_id);
		return false;
	}
	return true;
}

BOOL CTableCha::UpdateOutMapTime(DWORD cha_id) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set outmaptime = getdate() where cha_id = %d", _get_table(), cha_id);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG2("error", "update player[id] = %d outmaptime error! \n", cha_id);
		return false;
	}
	return true;
}

BOOL CTableCha::ConvertCTime(std::string stime, CTime& ctime) {
	if (!stime.empty()) {
		std::string timenum[4];
		const int n1 = Util_ResolveTextLine(stime.c_str(), timenum, 3, ' ');
		const std::string& date = timenum[0];
		const std::string& time = timenum[1];

		std::string datenum[3];
		const int n2 = Util_ResolveTextLine(date.c_str(), datenum, 3, '-');
		const int year = atoi(datenum[0].c_str());
		const int month = atoi(datenum[1].c_str());
		const int day = atoi(datenum[2].c_str());

		std::string timnum[3];
		const int n3 = Util_ResolveTextLine(time.c_str(), timnum, 3, ':');
		const int hour = atoi(timnum[0].c_str());
		const int minute = atoi(timnum[1].c_str());
		const int secend = atoi(timnum[2].c_str());
		ctime = CTime(year, month, day, hour, minute, secend);
		return true;
	}
	return false;
}

BOOL CTableCha::CalculateDisTime(CCharacter* pCCha, int& distime) {
	if (!pCCha->GetPlayer()) {
		return false;
	}

	const int MAX_GETTIME = 24 * 60;
	const int MAX_DAYTIME = 8 * 60;

	int Span[3] = {0, 0, 0}; //今天，昨天，前天
	int DisTemp = 0;		 //EXTEND10存储离线时间
	LLong distemp = 0;		 //threedistime临时存储离线时间

	std::string ltime, ntime;

	CTime LeaveTime(0), NowTime(0); //下线时间，上线时间
	CTime Curtime(0);

	std::string buf[3];
	const char* param = "entermaptime,outmaptime,threedistime";
	const char* entparam = "entermaptime";
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d ", pCCha->m_ID);
	const int r = _get_row(buf, 3, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		bool Flag = false; //是否最大离线时间
		if (buf[1].empty()) {
			LeaveTime = CTime::GetCurrentTime();
		} else {
			ltime = buf[1];
			ConvertCTime(ltime, LeaveTime);
		}
		CTimeSpan downSpan = 0; //时间差
		Curtime = CTime::GetCurrentTime();
		downSpan = Curtime - LeaveTime;
		ntime = buf[0];
		ConvertCTime(ntime, NowTime);
		distemp = (int)_atoi64(buf[2].c_str());

		DisTemp = (int)pCCha->getAttr(ATTR_EXTEND10);

		//得到最初离线时间状态 今天，昨天，前天
		Span[0] = int(distemp & ((1 << 10) - 1));
		Span[1] = int((distemp >> 9) & ((1 << 10) - 1));
		Span[2] = int((distemp >> 18) & ((1 << 10) - 1));

		if (downSpan.GetHours() < 1 && (downSpan.GetDays() == 0)) {
			if (UpdateEnterMapTime(pCCha->m_ID)) {
				LG2("distime", "%s Linear time is not more than 1 hours!\n", pCCha->m_name);
			}
			distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
			return false;
		} else {
			bool index = false;
			//如果已经是最大，则返回，不再计算
			if ((Span[0] + Span[1] + Span[2] + DisTemp) >= MAX_GETTIME) {
				Flag = true;
				//存储上线时间
				if (!UpdateEnterMapTime(pCCha->m_ID)) {
					return false;
				}
			} else //未满
			{
				if ((LeaveTime.GetDay() != NowTime.GetDay()) || downSpan.GetDays() > 0) {
					DisTemp = DisTemp + Span[2] + Span[1] + Span[0];
					Span[2] = Span[1] = Span[0] = 0;
				}
				//存储上线时间
				if (!UpdateEnterMapTime(pCCha->m_ID)) {
					distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
					return false;
				}
				_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d", pCCha->m_ID);
				const int r = _get_row(buf, 1, entparam, filter);
				const int r1 = get_affected_rows();
				if (DBOK(r) && r1 > 0) {
					ntime = buf[0];
					ConvertCTime(ntime, NowTime);
					CTimeSpan SpanOneDay(1, 0, 0, 0);
					CTimeSpan SpanTodDay(0, NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond()); //当天零点

					CTime T[3];					 //当天零点，前一天零点，前两天零点
					T[0] = NowTime - SpanTodDay; //当天零点
					T[1] = T[0] - SpanOneDay;	//前一天零点
					T[2] = T[1] - SpanOneDay;	//前两天零点

					downSpan = NowTime - LeaveTime; //上下线时间差
					if (downSpan.GetDays() > 2) {   //已经差超过3天
						Flag = true;
					} else if (downSpan.GetDays() > 1) { // 已经差超过2天
						//超过2天
						DisTemp = DisTemp + Span[2] + Span[1];
						Span[2] = Span[0];	 //上个今天已变成前天
						Span[1] = Span[0] = 0; //清空放这次的时间
						// 计算前天的时间
						downSpan = T[1] - LeaveTime;
						if (Span[2] + downSpan.GetHours() * 60 + downSpan.GetMinutes() > MAX_DAYTIME) {
							Span[2] = MAX_DAYTIME;
						} else {
							Span[2] = Span[2] + downSpan.GetHours() * 60 + downSpan.GetMinutes();
						}

						// 昨天的时间
						Span[1] = MAX_DAYTIME; // 昨天全天下线
						// 今天的时间
						downSpan = NowTime - T[0];
						if (downSpan.GetHours() > MAX_DAYTIME) {
							Span[0] = MAX_DAYTIME;
						} else {
							Span[0] = Span[0] + downSpan.GetHours() * 60 + downSpan.GetMinutes();
						}
					} else if ((NowTime.GetDay() == LeaveTime.GetDay()) && (downSpan.GetDays() == 0)) {
						downSpan = NowTime - LeaveTime;
						if (Span[0] + downSpan.GetHours() * 60 + downSpan.GetMinutes() > MAX_DAYTIME) {
							Span[0] = MAX_DAYTIME;
						} else {
							Span[0] = Span[0] + downSpan.GetHours() * 60 + downSpan.GetMinutes();
						}
					} else {
						DisTemp = DisTemp + Span[2];
						Span[2] = Span[1];
						Span[1] = Span[0];
						Span[0] = 0;

						downSpan = T[0] - LeaveTime;
						if (Span[1] + downSpan.GetHours() * 60 + downSpan.GetMinutes() > MAX_DAYTIME) {
							Span[1] = MAX_DAYTIME;
						} else {
							Span[1] = Span[1] + downSpan.GetHours() * 60 + downSpan.GetMinutes();
						}

						downSpan = NowTime - T[0];
						if (Span[0] + downSpan.GetHours() * 60 + downSpan.GetMinutes() > MAX_DAYTIME) {
							Span[0] = MAX_DAYTIME;
						} else {
							Span[0] = Span[0] + downSpan.GetHours() * 60 + downSpan.GetMinutes();
						}
					}

					if ((DisTemp + Span[0] + Span[1] + Span[2]) >= MAX_GETTIME) {
						Flag = true;
					}
				} else {
					distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
					LG2("distime", " can't find entermaptime where cha_id = %d\n", pCCha->m_ID);
					return false;
				}
			}
			if (Flag == true) {
				Span[0] = MAX_DAYTIME;
				Span[1] = MAX_DAYTIME;
				Span[2] = MAX_DAYTIME;
				DisTemp = 0;
			}
			distemp = Span[0] + (Span[1] << 9) + (Span[2] << 18);

			_snprintf_s(filter, sizeof(filter), _TRUNCATE, "update %s set threedistime = %lld where cha_id = %d", _get_table(), distemp, pCCha->m_ID);
			const short sExec = exec_sql_direct(filter);
			if (!DBOK(sExec)) {
				distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
				LG2("distime", "update %s set threedistime = %lld where cha_id = %d \n", _get_table(), distemp, pCCha->m_ID);
				return false;
			}
			pCCha->setAttr(ATTR_EXTEND10, DisTemp);
			distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
			return true;
		}
	}
	return false;
}

BOOL CTableCha::GetDisTime(CCharacter* pCCha, int& distime) {
	if (!pCCha) {
		return false;
	}
	const char* param = "threedistime";
	char filter[80];
	std::string buf[1];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d ", pCCha->m_ID);
	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		LLong distemp = (int)_atoi64(buf[0].c_str());
		int Span[3] = {0, 0, 0};
		Span[0] = int(distemp & ((1 << 10) - 1));
		Span[1] = int((distemp >> 9) & ((1 << 10) - 1));
		Span[2] = int((distemp >> 18) & ((1 << 10) - 1));

		int DisTemp = 0;
		DisTemp = (int)pCCha->getAttr(ATTR_EXTEND10);
		distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
		return true;
	}
	return false;
}

BOOL CTableCha::UpdateDisTime(CCharacter* pCCha, int iSelTime) {
	if (!pCCha) {
		return false;
	}
	const char* param = "threedistime";
	char filter[80];
	std::string buf[1];
	LLong distime = 0;
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d ", pCCha->m_ID);
	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		LLong distemp = (int)_atoi64(buf[0].c_str());
		int Span[3] = {0, 0, 0};
		Span[0] = int(distemp & ((1 << 10) - 1));
		Span[1] = int((distemp >> 9) & ((1 << 10) - 1));
		Span[2] = int((distemp >> 18) & ((1 << 10) - 1));

		int DisTemp = 0;
		DisTemp = (int)pCCha->getAttr(ATTR_EXTEND10);
		distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
		if (distime < iSelTime) {
			LG("distime", "distime is less than select time!\n");
			return false;
		} else {
			if (iSelTime * 60 > Span[0]) {
				if (iSelTime * 60 > (DisTemp + Span[0] + Span[1] + Span[2])) {
					pCCha->SystemNotice("You have not enough time to get!");
					return false;
				}
				DisTemp = DisTemp + Span[0] + Span[1] + Span[2] - iSelTime * 60;
				Span[0] = Span[1] = Span[2] = 0;
				pCCha->setAttr(ATTR_EXTEND10, DisTemp);
				_snprintf_s(filter, sizeof(filter), _TRUNCATE, "update %s set threedistime = 0 where cha_id = %d", _get_table(), pCCha->m_ID);
				short sExec = exec_sql_direct(filter);
				if (!DBOK(sExec)) {
					distime = (int)(DisTemp + Span[0] + Span[1] + Span[2]) / 60;
					LG2("distime", "updatedistime %s set threedistime = 0 where cha_id = %d \n", _get_table(), pCCha->m_ID);
					return false;
				}
			} else {
				Span[0] -= (iSelTime * 60);
				distemp = Span[0] + (Span[1] << 9) + (Span[2] << 18);
				_snprintf_s(filter, sizeof(filter), _TRUNCATE, "update %s set threedistime = %lld where cha_id = %d", _get_table(), distemp, pCCha->m_ID);
				short sExec = exec_sql_direct(filter);
				if (!DBOK(sExec)) {
					LG2("distime", "updatedistime %s set threedistime = %lld  where cha_id = %d \n", _get_table(), distemp, pCCha->m_ID);
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

BOOL CTableCha::SetDisTime(CCharacter* pCCha, int iSelTime) {
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "update %s set threedistime = 0 where cha_id = %d", _get_table(), pCCha->m_ID);
	const short sExec = exec_sql_direct(filter);
	if (DBOK(sExec)) {
		pCCha->setAttr(ATTR_EXTEND10, iSelTime * 60);
		return true;
	}
	LG("distime", " SetDisTime error!\n");
	return false;
}

//取水晶平台里的帐户金额
BOOL CTableCha::GetdwFlatMoney(const char* cha_name, DWORD& FlatMoney) {
	const char* param = "flatcoins";
	char filter[80];
	std::string buf[1];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_name = '%s' ", cha_name);
	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		FlatMoney = atoi(buf[0].c_str());
		return true;
	}
	return false;
}

BOOL CTableCha::SetdwFlatMoney(DWORD cha_tid, DWORD FlatMoney) {
	const char* param = "flatcoins";
	char filter[80];
	std::string buf[1];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d", cha_tid);
	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	if (!DBOK(r) || r1 <= 0) {
		LG("Crystal_error", "AcceptCrystalAdd TO DB 《flatcoins》 ChaID: [%d] Add flatcoins: (%ld) Failed!, Can't find flatcoins \n", cha_tid, FlatMoney);
		return false;
	}
	const DWORD OldFlatMoney = atoi(buf[0].c_str());
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set flatcoins = flatcoins + %d where cha_id = %d ",
				_get_table(), FlatMoney, cha_tid);

	const short sExec = exec_sql_direct(g_sql);
	if (DBOK(sExec)) {
		LG("Crystal_msg", "AcceptCrystalAdd TO DB《flatcoins》 ChaID: [%d], oldFlatMoney : (%lu) Add flatcoins: (%ld) Successed! \n", cha_tid, OldFlatMoney, FlatMoney);
		return true;
	}
	LG("Crystal_msg", "AcceptCrystalAdd TO DB 《flatcoins》 ChaID: [%d], oldFlatMoney : (%lu) Add flatcoins: (%ld) Failed! \n", cha_tid, OldFlatMoney, FlatMoney);
	return false;
}
//End

bool CTableLotterySetting::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select section from %s (nolock) where 1=2", _get_table());

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "LotterySetting");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

bool CTableLotterySetting::GetCurrentIssue(int& issue) {
	issue = -1;
	std::string buf[1];
	const char param[] = "issue";
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "state  =%d", enumCURRENT);

	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		issue = Str2Int(buf[0]);
		return true;
	}

	return false;
}

bool CTableLotterySetting::AddIssue(int issue) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert into %s (section, issue, state, createdate, updatetime) values (%d, %d, %d, getdate(), getdate())",
				_get_table(), 1, issue, enumCURRENT);

	const short sExec = exec_sql_direct(g_sql);
	if (DBOK(sExec)) {
		return true;
	}

	return false;
}

bool CTableLotterySetting::DisuseIssue(int issue, int state) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d, updatetime = getdate() where issue = %d", _get_table(), state, issue);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("lottery", "lottery couldn't find the issue %d!\n", issue);
		return false;
	}

	return true;
}

bool CTableLotterySetting::SetWinItemNo(int issue, const char* itemno) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set itemno = '%s', state = 1, updatetime = getdate() where issue = %d", _get_table(), itemno, issue);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("lottery", "lottery couldn't find the issue %d!\n", issue);
		return false;
	}

	return true;
}

bool CTableLotterySetting::GetWinItemNo(int issue, std::string& itemno) {
	std::string buf[1];
	const char* param = "itemno";
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "state  =%d and issue = %d", enumCURRENT, issue);

	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		if (buf[0].empty())
			return false;
		itemno = buf[0];
		return true;
	}

	return false;
}

bool CTableTicket::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select itemno from %s (nolock) where 1=2", _get_table());

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "Ticket");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

bool CTableTicket::AddTicket(int cha_id, int issue, char itemno[6][2]) {
	int index = -1;
	char no[10][6];

	for (int i = 0; i < 6; i++) {
		if (itemno[i][0] == 'X') {
			index = i;
			break;
		}
	}

	if (index > 0) {
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 6; j++) {
				if (j == index) {
					no[i][j] = '0' + i;
				} else {
					no[i][j] = itemno[j][0];
				}
			}
		}

		for (int i = 0; i < 10; i++) {
			AddTicket(cha_id, issue, no[i][0], no[i][1], no[i][2], no[i][3], no[i][4], no[i][5], 0);
		}
	}

	AddTicket(cha_id, issue, itemno[0][0], itemno[1][0], itemno[2][0], itemno[3][0], itemno[4][0], itemno[5][0]);

	return false;
}

bool CTableTicket::AddTicket(int cha_id, int issue, char itemno1, char itemno2, char itemno3, char itemno4, char itemno5, char itemno6, int real) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert into %s (cha_id, issue, itemno, real, buydate) values (%d, '%d', '%c%c%c%c%c%c', %d, getdate())",
				_get_table(), cha_id, issue, itemno1, itemno2, itemno3, itemno4, itemno5, itemno6, real);

	const short sExec = exec_sql_direct(g_sql);
	if (DBOK(sExec)) {
		return true;
	}

	return false;
}

bool CTableTicket::IsExist(int issue, char* itemno) {
	std::string buf[1];
	const char* param = "count(*)";
	char filter[50];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, " issue = %d and itemno  ='%s'", issue, itemno);

	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		if (Str2Int(buf[0]) > 0)
			return true;
	}

	return false;
}

bool CTableTicket::CalWinTicket(int issue, int max, std::string& itemno) {
	
	int RANGE_MIN = 1;
	int RANGE_MAX = 3;

	// 概率，3期选一个号码
	int probability = rng.uniform(RANGE_MIN, RANGE_MIN + RANGE_MAX - 1);

	// 如果选中
	if (((issue + probability) % 3) == 0) {
		char sql[256];

		_snprintf_s(sql, sizeof(sql), _TRUNCATE, "SELECT top 10 * FROM\
						(\
						SELECT     itemno, COUNT(*) AS num\
						FROM         Ticket WITH(NOLOCK) \
						WHERE     (issue = %d) AND real = 0\
						GROUP BY itemno\
						) \
						AS A\
						WHERE num <= %d\
						ORDER BY num",
					issue, max);

		std::vector<std::vector<std::string>> data;

		getalldata(sql, data);

		RANGE_MAX = (int)data.size();

		if (RANGE_MAX > 0) {
			int index = rng.uniform(RANGE_MIN, RANGE_MIN + RANGE_MAX - 1);

			itemno = data[index][0];
			return true;
		}
	}

	char buffer[7];
	RANGE_MAX = 999999;

	while (true) {
		int num = rng.uniform(RANGE_MIN, RANGE_MIN + RANGE_MAX - 1);
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%06d", num);

		LG("Ticket", "while(1) %s\n", buffer);

		if (!IsExist(issue, buffer))
			break;
	}

	itemno = buffer;

	return true;
}

bool CTableWinTicket::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select issue from %s (nolock) where 1=2", _get_table());
	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "WinTicket");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

bool CTableWinTicket::GetTicket(int issue) {
	return true;
}

bool CTableWinTicket::AddTicket(int issue, char* itemno, int grade) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert into %s (issue, itemno, grade, createdate) values (%d, '%s', %d, getdate())",
				_get_table(), issue, itemno, grade);

	const short sExec = exec_sql_direct(g_sql);
	if (DBOK(sExec)) {
		return true;
	}

	return false;
}

bool CTableWinTicket::Exchange(int issue, char* itemno) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set num = num + 1 where issue = %d and itemno = '%s'", _get_table(), issue, itemno);
	if (strlen(g_sql) >= SQL_MAXLEN) {
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}

		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}
	const short sExec = exec_sql_direct(g_sql);

	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("lottery", "lottery couldn't find the issue %d!\n", issue);
		return false;
	}

	return true;
}

bool CTableAmphitheaterSetting::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select season from %s (nolock) where 1=2", _get_table());

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "AmphitheaterSetting");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

// 获得当前赛季号和轮次号
bool CTableAmphitheaterSetting::GetCurrentSeason(int& season, int& round) {
	season = -1;
	round = -1;
	std::string buf[2];
	const char* param = "season, [round]";
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "state  =%d", AmphitheaterSetting::enumCURRENT);

	const int r = _get_row(buf, 2, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		season = Str2Int(buf[0]);
		round = Str2Int(buf[1]);
		return true;
	}

	return false;
}

// 追加一个赛季
bool CTableAmphitheaterSetting::AddSeason(int season) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert into %s (section, season, [round], state, createdate, updatetime,winner) values (%d, %d, %d, %d, getdate(), getdate(),%s)",
				_get_table(), 1, season, 1, AmphitheaterSetting::enumCURRENT, NULL);

	const short sExec = exec_sql_direct(g_sql);
	if (DBOK(sExec)) {
		return true;
	}

	return false;
}

// 更改赛季状态
bool CTableAmphitheaterSetting::DisuseSeason(int season, int state, const char* winner) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d, updatetime = getdate(),winner = %s where season = %d", _get_table(), state, winner, season);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("AmphitheaterSetting", "amphitheater couldn't find the season %d!\n", season);
		return false;
	}

	return true;
}

// 更改轮次
bool CTableAmphitheaterSetting::UpdateRound(int season, int round) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set [round] = %d, updatetime = getdate() where season = %d", _get_table(), round, season);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("AmphitheaterSetting", "amphitheater couldn't find the season %d!\n", season);
		return false;
	}

	return true;
}

bool CTableAmphitheaterTeam::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select id from %s (nolock) where 1=2", _get_table());

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "AmphitheaterTeam");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

// 取得参赛队伍数量
bool CTableAmphitheaterTeam::GetTeamCount(int& count) {
	count = -1;
	std::string buf[1];
	const char* param = "count(*)";
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "state > %d", AmphitheaterTeam::enumNotUse);

	const int r = _get_row(buf, 2, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		count = Str2Int(buf[0]);
		return true;
	}
	return false;
}

// 取得队伍编号
bool CTableAmphitheaterTeam::GetNoUseTeamID(int& teamID) {
	// 取得空白队伍ID
	teamID = 0;
	std::string buf[1];
	const char* param = "top(1) id";
	char filter[80];
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "state = %d", AmphitheaterTeam::enumNotUse);

	const int r = _get_row(buf, 2, param, filter);
	const int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		teamID = Str2Int(buf[0]);
		return true;
	}

	return false;
}

// 队伍注册 返回注册后的teamID
bool CTableAmphitheaterTeam::TeamSignUP(int& teamID, int captain, int member1, int member2) {
	// 取得空白队伍ID
	if (teamID < 0) {
		if (!GetNoUseTeamID(teamID))
			return false;
	}

	char member[50];
	_snprintf_s(member, sizeof(member), _TRUNCATE, "%d,%d", member1, member2);

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set captain = %d, member = '%s', state = %d, updatetime = getdate() where id = %d", _get_table(), captain, member, AmphitheaterTeam::enumUse, teamID);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("AmphitheaterTeam", "amphitheater couldn't find the id %d!\n", teamID);
		return false;
	}

	return true;
}

// 队伍取消
bool CTableAmphitheaterTeam::TeamCancel(int teamID) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set captain = null, member = null, matchno = 0, state = %d, updatetime = getdate() where id = %d", _get_table(), AmphitheaterTeam::enumNotUse, teamID);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		LG("AmphitheaterTeam", "amphitheater couldn't find the id %d!\n", teamID);
		return false;
	}

	return true;
}

// 队伍更新
bool CTableAmphitheaterTeam::TeamUpdate(int teamID, int matchNo, int state, int winnum, int losenum, int relivenum) {

	return false;
}

// 判断有效的队伍
bool CTableAmphitheaterTeam::IsValidAmphitheaterTeam(int teamID, int captainID, int member1, int member2) {
	std::string buf[1];
	const char* param = "count(*)";
	char filter[80];
	int HasValid = 0;
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id = %d and captain = %d and (member = '%d,%d' or member = '%d,%d')", teamID, captainID, member1, member2, member2, member1);

	int r = _get_row(buf, 1, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		HasValid = Str2Int(buf[0]);
		if (HasValid > 0) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}
//Add by sunny.sun20080714
//判断是否已经注册
bool CTableAmphitheaterTeam::IsLogin(int pActorID) {
	std::string buf[1];
	const char* param = "count(*)";
	char filter[80];
	int PActorIDNum = 0;

	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "captain = %d or member like '%d,%' or member like '%,%d'", pActorID, pActorID, pActorID);

	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();
	short sExec = exec_sql_direct(g_sql);
	if (DBOK(r) && r1 > 0) {
		PActorIDNum = Str2Int(buf[0]);
		if (PActorIDNum > 0) {
			return false;
		} else {
			return true;
		}
	}
	return false;
}

//判断是否该地图队伍个数
bool CTableAmphitheaterTeam::IsMapFull(int MapID, int& PActorIDNum) {
	std::string buf[1];
	const char* param = "count(*)";
	char filter[80];

	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "map = %d ", MapID);

	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();

	if (DBOK(r) && r1 > 0) {
		PActorIDNum = Str2Int(buf[0]);
		if (PActorIDNum > 2) {
			return false;
		} else {
			return true;
		}
	}
	return false;
}
//更新地图mapflag
bool CTableAmphitheaterTeam::UpdateMapNum(int Teamid, int Mapid, int MapFlag) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set mapflag = %d where id = %d and map = %d", _get_table(), MapFlag, Teamid, Mapid);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}

	return true;
}

//获得mapflag值
bool CTableAmphitheaterTeam::GetMapFlag(int Teamid, int& Mapflag) {
	std::string buf[1];
	const char* param = "mapflag";
	char filter[80];

	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id = %d ", Teamid);

	const int r = _get_row(buf, 1, param, filter);
	const int r1 = get_affected_rows();

	if (DBOK(r) && r1 > 0) {
		Mapflag = Str2Int(buf[0]);
		if (Mapflag >= 2) {
			return false;
		} else {
			return true;
		}
	}
	return false;
}

//设置票数最多的队伍状态为复活,state = 1 和state= 3的改为淘汰
bool CTableAmphitheaterTeam::SetMaxBallotTeamRelive() {
	std::string buf[1];
	const char* param = "count(*)";
	char filter[80];
	int state = 0;
	int OddOrEven = 0;

	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "state = %d", AmphitheaterTeam::enumPromotion);
	int r = _get_row(buf, 1, param, filter);
	if (DBOK(r)) {
		state = Str2Int(buf[0]);
		if (state % 2 == 0) {
			OddOrEven = 2;
		} else {
			OddOrEven = 1;
		}
	}
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d ,relivenum = 0 where id in (select top %d id from %s where state = %d  order by relivenum desc)", _get_table(), AmphitheaterTeam::enumPromotion, OddOrEven, _get_table(), AmphitheaterTeam::enumRelive);
	short sExec1 = exec_sql_direct(g_sql);

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d ,relivenum = 0 where state = %d or state = %d", _get_table(), AmphitheaterTeam::enumOut, AmphitheaterTeam::enumRelive, AmphitheaterTeam::enumUse);
	short sExec2 = exec_sql_direct(g_sql);

	if (!DBOK(sExec1) || !DBOK(sExec2)) {
		return false;
	}
	return true;
}

//设置比赛结果的状态
bool CTableAmphitheaterTeam::SetMatchResult(int Teamid1, int Teamid2, int Id1state, int Id2state) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d where id = %d", _get_table(), Id1state, Teamid1);
	const short sExec1 = exec_sql_direct(g_sql);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d where id = %d", _get_table(), Id2state, Teamid2);
	const short sExec2 = exec_sql_direct(g_sql);

	if (!DBOK(sExec1) || !DBOK(sExec2)) {
		return false;
	}
	if (DBNODATA(sExec1) || DBNODATA(sExec2)) {
		return false;
	}
	return true;
}
//根据mapid 取该地图的两个队长的id
bool CTableAmphitheaterTeam::GetCaptainByMapId(int Mapid, std::string& Captainid1, std::string& Captainid2) {
	std::string NoCaptain = "";
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select captain from %s where map = %d", _get_table(), Mapid);

	std::vector<std::vector<std::string>> data;
	getalldata(g_sql, data);

	int MapCaptainNum = (int)data.size();
	if (MapCaptainNum > 2 || MapCaptainNum == 0) {
		return false;
	}
	if (MapCaptainNum < 2) {
		Captainid1 = data[0][0];
		Captainid2 = NoCaptain;
		return true;
	} else {
		Captainid1 = data[0][0];
		Captainid2 = data[1][0];
		return true;
	}
	return false;
}
//更新map字段
bool CTableAmphitheaterTeam::UpdateMap(int Mapid) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set map = null where map = %d  ", _get_table(), Mapid);

	const short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}
	return true;
}
//取得排行榜数据
bool CTableAmphitheaterTeam::GetPromotionAndReliveTeam(std::vector<std::vector<std::string>>& dataPromotion, std::vector<std::vector<std::string>>& dataRelive) {
	// select A.id, A.captain, A.relivenum, B.cha_name from AmphitheaterTeam A,  character B where B.cha_id = A.captain
	//sprintf(g_sql,"select A.id, A.captain, B.cha_name, A.winnum from AmphitheaterTeam A,  character B where B.cha_id = A.captain and a.state = %d  order by A.winnum DESC", AmphitheaterTeam::enumPromotion );
	//sprintf(g_sql,"select B.cha_name,A.id,A.winnum from AmphitheaterTeam A, character B where B.cha_id = A.captain and a.state = %d  order by A.winnum DESC", AmphitheaterTeam::enumPromotion );
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select B.cha_name,A.id,A.winnum from AmphitheaterTeam A, character B where B.cha_id = A.captain and a.state = %d  order by A.winnum DESC", AmphitheaterTeam::enumPromotion);

	if (!getalldata(g_sql, dataPromotion))
		return false;

	//sprintf(g_sql,"select A.id, A.captain, B.cha_name, A.relivenum from AmphitheaterTeam A,  character B where B.cha_id = A.captain and A.state = %d  order by A.relivenum DESC", AmphitheaterTeam::enumRelive );
	//sprintf(g_sql,"select B.cha_name, A.relivenum ,A.id from AmphitheaterTeam A,  character B where B.cha_id = A.captain and A.state = %d  order by A.relivenum DESC", AmphitheaterTeam::enumRelive );
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select B.cha_name, A.relivenum ,A.id from AmphitheaterTeam A,  character B where B.cha_id = A.captain and A.state = %d  order by A.relivenum DESC", AmphitheaterTeam::enumRelive);

	if (!getalldata(g_sql, dataRelive))
		return false;

	return true;
}

//更新票数
bool CTableAmphitheaterTeam::UpdatReliveNum(int ReID) {
	std::string buf[1];
	const char* param = "relivenum";
	char filter[80];
	int relivenumber = 0;
	//sprintf(filter, "id = %d ",ReID);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id = %d ", ReID);

	int r = _get_row(buf, 1, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		relivenumber = Str2Int(buf[0]) + 1;
		//sprintf( g_sql, "update %s set relivenum = %d where id = %d ", _get_table(),relivenumber ,ReID );
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set relivenum = %d where id = %d ", _get_table(), relivenumber, ReID);
		short sExec = exec_sql_direct(g_sql);
		if (!DBOK(sExec))
			return false;
		return true;
	}
	return false;
}

//更新一轮没参加比赛的队为复活
bool CTableAmphitheaterTeam::UpdateAbsentTeamRelive() {
	//sprintf( g_sql, "update %s set state = %d where state = %d ", _get_table(),AmphitheaterTeam::enumRelive,AmphitheaterTeam::enumUse );
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d where state = %d ", _get_table(), AmphitheaterTeam::enumRelive, AmphitheaterTeam::enumUse);
	short sExec = exec_sql_direct(g_sql);

	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}
	return true;
}
//更新队伍进入地图后的map字段
bool CTableAmphitheaterTeam::UpdateMapAfterEnter(int CaptainID, int MapID) {
	//sprintf( g_sql, "update %s set map = %d where captain = %d ", _get_table(),MapID,CaptainID);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set map = %d where captain = %d ", _get_table(), MapID, CaptainID);
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}
	return true;
}

//Add by sunny.sun20080806
//更新winnum的值，胜利加1
bool CTableAmphitheaterTeam::UpdateWinnum(int teamid) {
	//sprintf( g_sql, "update %s set winnum = winnum+1 where id = %d ", _get_table(),teamid);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set winnum = winnum+1 where id = %d ", _get_table(), teamid);
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}
	return true;
}
//获取winnum最大且唯一的队伍的id
bool CTableAmphitheaterTeam::GetUniqueMaxWinnum(int& teamid) {
	//select id from AmphitheaterTeam where winnum in (select  max(winnum) from AmphitheaterTeam)
	std::string buf[1];
	const char* param = "id";
	char filter[80];

	//sprintf(filter, "winnum in (select  max(winnum) from AmphitheaterTeam)");
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "winnum in (select  max(winnum) from AmphitheaterTeam)");
	int r = _get_row(buf, 1, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		if (r1 == 1) {
			teamid = Str2Int(buf[0]);
			return true;
		} else
			return false;
	}
	return false;
}

bool CTableAmphitheaterTeam::SetMatchnoState(int teamid) {
	//sprintf( g_sql, "update %s set matchno = 1 where id = %d ", _get_table(),teamid);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set matchno = 1 where id = %d ", _get_table(), teamid);
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}
	return true;
}

bool CTableAmphitheaterTeam::UpdateState() {
	//sprintf( g_sql, "update %s set state = %d where state= %d ", _get_table(),AmphitheaterTeam::enumUse,AmphitheaterTeam::enumPromotion);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set state = %d where state= %d ", _get_table(), AmphitheaterTeam::enumUse, AmphitheaterTeam::enumPromotion);
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}
	return true;
}

bool CTableAmphitheaterTeam::CloseReliveByState(int& statenum) {
	std::string buf[1];
	const char* param = "count(*)";
	char filter[80];

	//sprintf(filter, "state = %d ",AmphitheaterTeam::enumUse);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "state = %d ", AmphitheaterTeam::enumUse);
	int r = _get_row(buf, 1, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		statenum = Str2Int(buf[0]);
		return true;
	}
	return false;
}

bool CTableAmphitheaterTeam::CleanMapFlag(int teamid1, int teamid2) {
	//sprintf( g_sql, "update %s set mapflag = null where id= %d or id = %d", _get_table(),teamid1,teamid2);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set mapflag = null where id= %d or id = %d", _get_table(), teamid1, teamid2);
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		return false;
	}
	if (DBNODATA(sExec)) {
		return false;
	}
	return true;
}

bool CTableAmphitheaterTeam::GetStateByTeamid(int teamid, int& state) {
	std::string buf[1];
	const char* param = "state";
	char filter[80];

	//sprintf(filter, "id = %d ",teamid);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id = %d ", teamid);
	int r = _get_row(buf, 1, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		state = Str2Int(buf[0]);
		return true;
	}
	return false;
}

//personinfo 初始化
bool CTablePersoninfo::Init() {
	//sprintf(g_sql, "select cha_id from %s (nolock) where 1=2", _get_table());
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select cha_id from %s (nolock) where 1=2", _get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		//MessageBox(0, "数据库(AmphitheaterSetting)初始化错误，请检查", "错误", MB_OK);
		char buffer[255];
		//sprintf(buffer, RES_STRING(GM_GAMEDB_CPP_00001), "personinfo");
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "personinfo");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

//获取birthday
bool CTablePersoninfo::GetPersonBirthday(int chaid, int& birthday) {
	std::string buf[1];
	const char* param = "birthday";
	char filter[80];

	//sprintf(filter, "cha_id = %d ",chaid);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d ", chaid);

	int r = _get_row(buf, 1, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		if (strcmp(buf[0].c_str(), "NULL") == 0)
			return false;
		else
			birthday = Str2Int(buf[0]);
		return true;
	}
	return false;
}

// End

bool CTableResource::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   id, cha_id, type_id, content \
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		//MessageBox(0, "数据库(resource)初始化错误，请检查", "错误", MB_OK);
		char buffer[255];
		//sprintf(buffer, RES_STRING(GM_GAMEDB_CPP_00001), "resource");
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "resource");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

bool CTableResource::Create(long& lDBID, long lChaId, long lTypeId) {
	//sprintf( g_sql, "insert %s (cha_id, type_id) \
	//				values(%d, %d) ",	_get_table(),
	//				lChaId, lTypeId);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert %s (cha_id, type_id) \
					values(%d, %d) ",
				_get_table(),
				lChaId, lTypeId);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}

		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}
	short sExec = exec_sql_direct(g_sql);
	if (DBOK(sExec)) {
		std::string buf[1];
		const char* param = "@@identity";
		char filter[80];
		//sprintf(filter, "1=1");
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "1=1");
		bool ret = _get_row(buf, 1, param, filter);
		lDBID = atol(buf[0].c_str());
		return true;
	}

	return false;
}

bool CTableResource::ReadKitbagData(CCharacter* pCha) {
	//LG("Thread", "DB ReadKitbagData %d\n", ::GetCurrentThreadId());

	if (!pCha) {
		//LG("enter_map", "读资源数据库错误，角色不存在.\n");
		LG("enter_map", "Load resource database error,character is inexistence\n");
		return false;
	}
	if (pCha->GetKitbagRecDBID() == 0) {
		long lDBID;
		if (!Create(lDBID, pCha->GetPlayer()->GetDBChaId(), enumRESDB_TYPE_KITBAG))
			return false;
		pCha->SetKitbagRecDBID(lDBID);
	}

	int nIndex = 0;
	char param[] = "cha_id, type_id, content";
	char filter[80]; //sprintf(filter, "id=%d", pCha->GetKitbagRecDBID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id=%d", pCha->GetKitbagRecDBID());
	int r = _get_row3(g_buf, g_cnCol, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		DWORD dwChaId = Str2Int(g_buf[nIndex++]);
		char chType = Str2Int(g_buf[nIndex++]);
		if (dwChaId != pCha->GetPlayer()->GetDBChaId() || chType != enumRESDB_TYPE_KITBAG) {
			//LG("enter_map", "读资源数据库错误，角色不匹配.\n");
			LG("enter_map", "Load resource database error，character is not matching.\n");
			return false;
		}
		if (!pCha->String2KitbagData(g_buf[nIndex++])) {
			//LG("enter_map", "背包数据校验和错误.\n");
			LG("enter_map", "kitbag data check sum error.\n");
			//LG("校验和错误", "角色（%s）的背包数据（resource_id %u）校验和错误.\n", pCha->GetLogName(), pCha->GetKitbagRecDBID());
			LG("check sum error", "character(%s) kitbag data(resource_id %u) check sum error.\n", pCha->GetLogName(), pCha->GetKitbagRecDBID());
			return false;
		}
	} else {
		//LG("enter_map", "读资源数据库错误，_get_row()返回值：%d.\n", r);
		LG("enter_map", "Load resource database error，_get_row()return value:%d.\n", r);
		return false;
	}
	//LG("enter_map", "读背包数据成功.\n");
	LG("enter_map", "Load kitbag data succeed.\n");
	return true;
}

bool CTableResource::ReadKitbagDataEx(CCharacter* pCha) {
	//LG("Thread", "DB ReadKitbagData %d\n", ::GetCurrentThreadId());

	if (!pCha) {
		//LG("enter_map", "读资源数据库错误，角色不存在.\n");
		LG("enter_map", "Load resource database error,character is inexistence\n");
		return false;
	}
	if (pCha->GetKitbagRecDBID() == 0) {
		long lDBID;
		if (!Create(lDBID, pCha->GetPlayer()->GetDBChaId(), enumRESDB_TYPE_KITBAG))
			return false;
		pCha->SetKitbagRecDBID(lDBID);
	}

	int nIndex = 0;
	char param[] = "cha_id, type_id, content";
	std::string stringBufs[64];

	char filter[80]; //sprintf(filter, "id=%d", pCha->GetKitbagRecDBID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id=%d", pCha->GetKitbagRecDBID());
	int r = _get_row3(stringBufs, 64, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		DWORD dwChaId = Str2Int(stringBufs[nIndex++]);
		char chType = Str2Int(stringBufs[nIndex++]);
		if (dwChaId != pCha->GetPlayer()->GetDBChaId() || chType != enumRESDB_TYPE_KITBAG) {
			//LG("enter_map", "读资源数据库错误，角色不匹配.\n");
			LG("enter_map", "Load resource database error，character is not matching.\n");
			return false;
		}
		if (!pCha->String2KitbagData(stringBufs[nIndex++])) {
			//LG("enter_map", "背包数据校验和错误.\n");
			LG("enter_map", "kitbag data check sum error.\n");
			//LG("校验和错误", "角色（%s）的背包数据（resource_id %u）校验和错误.\n", pCha->GetLogName(), pCha->GetKitbagRecDBID());
			LG("check sum error", "character(%s) kitbag data(resource_id %u) check sum error.\n", pCha->GetLogName(), pCha->GetKitbagRecDBID());
			return false;
		}
	} else {
		//LG("enter_map", "读资源数据库错误，_get_row()返回值：%d.\n", r);
		LG("enter_map", "Load resource database error，_get_row()return value:%d.\n", r);
		return false;
	}
	//LG("enter_map", "读背包数据成功.\n");
	LG("enter_map", "Load kitbag data succeed.\n");
	return true;
}

bool CTableResource::SaveKitbagData(CCharacter* pCha) {
	//LG("Thread", "DB SaveKitbagData %d\n", ::GetCurrentThreadId());

	if (!pCha || !pCha->IsValid())
		return false;

	//LG("enter_map", "开始保存背包数据!\n");
	//g_kitbag[0] = 0;
	memset(g_kitbag, 0, sizeof(g_kitbag));

	if (!KitbagData2String(pCha->GetKitbag(), g_kitbag, defKITBAG_DATA_STRING_LEN)) {
		//LG("enter_map", "角色%s\t保存数据（背包）时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\tsave data(kitbag) error!\n", pCha->GetLogName());
		return false;
	}
	//LG("enter_map", "转换背包数据成功\n");

	//sprintf(g_sql, "update %s set \
	//			   content='%s' \
	//			   where id=%d", \
	//			   _get_table(), \
	//			   g_kitbag, \
	//			   pCha->GetKitbagRecDBID());
	memset(g_sql, 0, sizeof(g_sql));
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   content='%s' \
				   where id=%d",
				_get_table(),
				g_kitbag,
				pCha->GetKitbagRecDBID());

	//LG("enter_map", "组织SQL语句成功\n");
	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}

		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}
	short sExec = exec_sql_direct(g_sql);
	//LG("enter_map", "执行SQL语句成功\n");
	if (!DBOK(sExec)) {
		//LG("enter_map", "角色%s\t执行SQL语句时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\t carry out SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		//LG("enter_map", "数据库没有查询到该背包资源%u!\n", pCha->GetKitbagRecDBID());
		LG("enter_map", "Database couldn't find the kitbag resource %u!\n", pCha->GetKitbagRecDBID());
		return false;
	}
	//LG("enter_map", "完成背包保存.\n");
	LG("enter_map", "finish kitbag save.\n");

	return true;
}

bool CTableResource::SaveKitbagDataEx(CCharacter* pCha) {
	//LG("Thread", "DB SaveKitbagData %d\n", ::GetCurrentThreadId());

	if (!pCha || !pCha->IsValid())
		return false;

	//LG("enter_map", "开始保存背包数据!\n");
	//g_kitbag[0] = 0;
	//char kitbag[defKITBAG_DATA_STRING_LEN];
	//memset(kitbag, 0, sizeof(kitbag));
	StringPoolL stringPool;
	stringPool.ReserveBuf(StringPoolL::AllocateSpace1);
	stringPool.ReserveNum(StringPoolL::AllocateSpace2);
	//memset(look, 0, sizeof(look));
	stringPool[0] = "";

	if (!KitbagData2StringEx(pCha->GetKitbag(), stringPool, 0)) {
		//LG("enter_map", "角色%s\t保存数据（背包）时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\tsave data(kitbag) error!\n", pCha->GetLogName());
		return false;
	}

	//::OutputDebugStr((const char*)stringPool[0]);

	//LG("enter_map", "转换背包数据成功\n");

	//sprintf(g_sql, "update %s set \
	//			   content='%s' \
	//			   where id=%d", \
	//			   _get_table(), \
	//			   g_kitbag, \
	//			   pCha->GetKitbagRecDBID());

	char sql[SQL_MAXLEN];
	//memset(sql, 0, sizeof(sql));

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
				   content='%s' \
				   where id=%d",
				_get_table(),
				(const char*)stringPool[0],
				pCha->GetKitbagRecDBID());

	short sExec = exec_sql_direct(sql);
	//LG("enter_map", "执行SQL语句成功\n");
	if (!DBOK(sExec)) {
		//LG("enter_map", "角色%s\t执行SQL语句时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\t carry out SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		//LG("enter_map", "数据库没有查询到该背包资源%u!\n", pCha->GetKitbagRecDBID());
		LG("enter_map", "Database couldn't find the kitbag resource %u!\n", pCha->GetKitbagRecDBID());
		return false;
	}
	//LG("enter_map", "完成背包保存.\n");
	LG("enter_map", "finish kitbag save.\n");

	return true;
}

bool CTableResource::ReadKitbagTmpData(CCharacter* pCha) {
	if (!pCha) {
		//LG("enter_map", "读资源数据库错误，角色不存在.\n");
		LG("enter_map", "Load resource database error,character is inexistence.\n");
		return false;
	}
	if (pCha->GetKitbagTmpRecDBID() == 0) {
		long lDBID;
		if (!Create(lDBID, pCha->GetPlayer()->GetDBChaId(), enumRESDB_TYPE_KITBAGTMP))
			return false;
		pCha->SetKitbagTmpRecDBID(lDBID);
	}

	int nIndex = 0;
	char param[] = "cha_id, type_id, content";
	char filter[80]; //sprintf(filter, "id=%d", pCha->GetKitbagTmpRecDBID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id=%d", pCha->GetKitbagTmpRecDBID());
	int r = _get_row3(g_buf, g_cnCol, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		DWORD dwChaId = Str2Int(g_buf[nIndex++]);
		char chType = Str2Int(g_buf[nIndex++]);
		if (dwChaId != pCha->GetPlayer()->GetDBChaId() || chType != enumRESDB_TYPE_KITBAGTMP) {
			//LG("enter_map", "读资源数据库错误，角色不匹配.\n");
			LG("enter_map", "Load resource database error,character is not matching.\n");
			return false;
		}
		if (!pCha->String2KitbagTmpData(g_buf[nIndex++])) {
			//LG("enter_map", "临时背包数据校验和错误.\n");
			LG("enter_map", "Temp kitbag data check sum error.\n");
			//LG("校验和错误", "角色（%s）的临时背包数据（resource_id %u）校验和错误.\n", pCha->GetLogName(), pCha->GetKitbagTmpRecDBID());
			LG("check sum error", "character(%s) temp kitbag data(resource_id %u)check sum error.\n", pCha->GetLogName(), pCha->GetKitbagTmpRecDBID());
			return false;
		}
	} else {
		//LG("enter_map", "读资源数据库错误，_get_row()返回值：%d.\n", r);
		LG("enter_map", "Load resource database error,_get_row() return value:%d.\n", r);
		return false;
	}
	//LG("enter_map", "读临时背包数据成功.\n");
	LG("enter_map", "Load temp kitbag data succeed.\n");

	return true;
}

bool CTableResource::ReadKitbagTmpDataEx(CCharacter* pCha) {
	if (!pCha) {
		//LG("enter_map", "读资源数据库错误，角色不存在.\n");
		LG("enter_map", "Load resource database error,character is inexistence.\n");
		return false;
	}
	if (pCha->GetKitbagTmpRecDBID() == 0) {
		long lDBID;
		if (!Create(lDBID, pCha->GetPlayer()->GetDBChaId(), enumRESDB_TYPE_KITBAGTMP))
			return false;
		pCha->SetKitbagTmpRecDBID(lDBID);
	}

	int nIndex = 0;
	char param[] = "cha_id, type_id, content";
	std::string stringBufs[64];

	char filter[80]; //sprintf(filter, "id=%d", pCha->GetKitbagTmpRecDBID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id=%d", pCha->GetKitbagTmpRecDBID());
	int r = _get_row3(stringBufs, 64, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		DWORD dwChaId = Str2Int(stringBufs[nIndex++]);
		char chType = Str2Int(stringBufs[nIndex++]);
		if (dwChaId != pCha->GetPlayer()->GetDBChaId() || chType != enumRESDB_TYPE_KITBAGTMP) {
			//LG("enter_map", "读资源数据库错误，角色不匹配.\n");
			LG("enter_map", "Load resource database error,character is not matching.\n");
			return false;
		}
		if (!pCha->String2KitbagTmpData(stringBufs[nIndex++])) {
			//LG("enter_map", "临时背包数据校验和错误.\n");
			LG("enter_map", "Temp kitbag data check sum error.\n");
			//LG("校验和错误", "角色（%s）的临时背包数据（resource_id %u）校验和错误.\n", pCha->GetLogName(), pCha->GetKitbagTmpRecDBID());
			LG("check sum error", "character(%s) temp kitbag data(resource_id %u)check sum error.\n", pCha->GetLogName(), pCha->GetKitbagTmpRecDBID());
			return false;
		}
	} else {
		//LG("enter_map", "读资源数据库错误，_get_row()返回值：%d.\n", r);
		LG("enter_map", "Load resource database error,_get_row() return value:%d.\n", r);
		return false;
	}
	//LG("enter_map", "读临时背包数据成功.\n");
	LG("enter_map", "Load temp kitbag data succeed.\n");

	return true;
}

bool CTableResource::ReadKitbagTmpData(long lRecDBID, std::string& strData) {
	if (lRecDBID == 0) {
		return false;
	}

	BOOL ret = false;

	const char* sql_syntax = "select content from %s where id=%d";
	char sql[SQL_MAXLEN];
	//sprintf(sql, sql_syntax, _get_table(), lRecDBID);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, sql_syntax, _get_table(), lRecDBID);

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);

			throw 1;
		}

		sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
		if (sqlret != SQL_SUCCESS) {
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);

			if (sqlret != SQL_SUCCESS_WITH_INFO)
				throw 2;
		}

		sqlret = SQLNumResultCols(hstmt, &col_num);
		col_num = min(col_num, MAX_COL);
		col_num = min(col_num, _max_col);

		// Bind Column
		for (int i = 0; i < col_num; ++i) {
			SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
		}

		// Fetch each Row	int i; // 取出的行数
		for (int f_row = 0; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++f_row) {
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			}

			strData = (char const*)_buf[0];
		}
		SQLFreeStmt(hstmt, SQL_UNBIND);
		ret = true;
	} catch (int& e) {
		//LG("Store_msg", "ReadKitbagTmpData ODBC 接口调用错误，位置码：%d\n",e);
		LG("Store_msg", "ReadKitbagTmpData ODBC interface transfer error,position ID:%d\n", e);
	} catch (...) {
		LG("Store_msg", "Unknown Exception raised when ReadKitbagTmpData\n");
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}

bool CTableResource::SaveKitbagTmpData(long lRecDBID, const std::string& strData) {
	if (lRecDBID == 0) {
		return false;
	}

	memset(g_sql, 0, sizeof(g_sql));
	//sprintf(g_sql, "update %s set \
	//			   content='%s' \
	//			   where id=%d", \
	//			   _get_table(), \
	//			   strData.c_str(), \
	//			   lRecDBID);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   content='%s' \
				   where id=%d",
				_get_table(),
				strData.c_str(),
				lRecDBID);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}

		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("Store_msg", "SQL语句长度越界!\n");
		LG("Store_msg", "SQL sentence length slop over\n");
		return false;
	}
	short sExec = exec_sql_direct(g_sql);

	if (!DBOK(sExec)) {
		//LG("Store_msg", "执行SQL语句时出错!\n");
		LG("Store_msg", "carry out SQL sentence error!\n");
		return false;
	}
	if (DBNODATA(sExec)) {
		//LG("Store_msg", "数据库没有查询到该临时背包资源%u!\n", lRecDBID);
		LG("Store_msg", "Database couldn't find the temp kitbag resource %u!\n", lRecDBID);
		return false;
	}
	//LG("Store_msg", "完成临时背包保存.\n");
	LG("Store_msg", "finish the temp kitbag save.\n");

	return true;
}

bool CTableResource::SaveKitbagTmpData(CCharacter* pCha) {
	if (!pCha || !pCha->IsValid())
		return false;

	//g_kitbagTmp[0] = 0;
	memset(g_kitbagTmp, 0, sizeof(g_kitbagTmp));
	if (!KitbagData2String(pCha->m_pCKitbagTmp, g_kitbagTmp, defKITBAG_DATA_STRING_LEN)) {
		//LG("enter_map", "角色%s\t保存数据（临时背包）时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\tsave data(temp kitbag)error!\n", pCha->GetLogName());
		return false;
	}

	//sprintf(g_sql, "update %s set \
	//			   content='%s' \
	//			   where id=%d", \
	//			   _get_table(), \
	//			   g_kitbagTmp, \
	//			   pCha->GetKitbagTmpRecDBID());
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   content='%s' \
				   where id=%d",
				_get_table(),
				g_kitbagTmp,
				pCha->GetKitbagTmpRecDBID());

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}

		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}
	short sExec = exec_sql_direct(g_sql);

	if (!DBOK(sExec)) {
		//LG("enter_map", "角色%s\t执行SQL语句时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\tcarry out SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		//LG("enter_map", "数据库没有查询到该临时背包资源%u!\n", pCha->GetKitbagTmpRecDBID());
		LG("enter_map", "Database couldn't find the temp kitbag resource%u!\n", pCha->GetKitbagTmpRecDBID());
		return false;
	}
	//LG("enter_map", "完成临时背包保存.\n");
	LG("enter_map", "finish save the temp kitbag.\n");

	return true;
}

bool CTableResource::SaveKitbagTmpDataEx(CCharacter* pCha) {
	if (!pCha || !pCha->IsValid())
		return false;

	//g_kitbagTmp[0] = 0;
	//char kitbagTmp[defKITBAG_DATA_STRING_LEN];
	//memset(kitbagTmp, 0, sizeof(kitbagTmp));
	StringPoolL stringPool;
	stringPool.ReserveBuf(StringPoolL::AllocateSpace1);
	stringPool.ReserveNum(StringPoolL::AllocateSpace2);
	//memset(look, 0, sizeof(look));
	stringPool[0] = "";
	if (!KitbagData2StringEx(pCha->m_pCKitbagTmp, stringPool, 0)) {
		//LG("enter_map", "角色%s\t保存数据（临时背包）时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\tsave data(temp kitbag)error!\n", pCha->GetLogName());
		return false;
	}

	//sprintf(g_sql, "update %s set \
	//			   content='%s' \
	//			   where id=%d", \
	//			   _get_table(), \
	//			   g_kitbagTmp, \
	//			   pCha->GetKitbagTmpRecDBID());
	char sql[SQL_MAXLEN];

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
				   content='%s' \
				   where id=%d",
				_get_table(),
				(const char*)stringPool[0],
				pCha->GetKitbagTmpRecDBID());

	short sExec = exec_sql_direct(sql);

	if (!DBOK(sExec)) {
		//LG("enter_map", "角色%s\t执行SQL语句时出错!\n", pCha->GetLogName());
		LG("enter_map", "character %s\tcarry out SQL sentence error!\n", pCha->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		//LG("enter_map", "数据库没有查询到该临时背包资源%u!\n", pCha->GetKitbagTmpRecDBID());
		LG("enter_map", "Database couldn't find the temp kitbag resource%u!\n", pCha->GetKitbagTmpRecDBID());
		return false;
	}
	//LG("enter_map", "完成临时背包保存.\n");
	LG("enter_map", "finish save the temp kitbag.\n");

	return true;
}

bool CTableResource::ReadBankData(CPlayer* pCPly, char chBankNO) {
	if (!pCPly) {
		LG("enter_map", "Load resource database error,character is inexistence.\n");
		return false;
	}
	if (pCPly->GetCurBankNum() == 0) {
		long lDBID;
		if (!Create(lDBID, pCPly->GetDBChaId(), enumRESDB_TYPE_BANK))
			return false;
		pCPly->AddBankDBID(lDBID);
	}

	char chStart, chEnd;
	if (chBankNO < 0) {
		chStart = 0;
		chEnd = pCPly->GetCurBankNum() - 1;
	} else {
		if (chBankNO >= pCPly->GetCurBankNum())
			return false;
		chStart = chEnd = chBankNO;
	}

	int nIndex = 0;
	char param[] = "cha_id, type_id, content";
	char filter[80];
	int r;
	int r1;
	for (char i = chStart; i <= chEnd; i++) {
		nIndex = 0;
		//sprintf(filter, "id=%d", pCPly->GetBankDBID(i));
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id=%d", pCPly->GetBankDBID(i));

		r = _get_row3(g_buf, g_cnCol, param, filter);
		r1 = get_affected_rows();
		if (DBOK(r) && r1 > 0) {
			DWORD dwChaId = Str2Int(g_buf[nIndex++]);
			char chType = Str2Int(g_buf[nIndex++]);
			if (dwChaId != pCPly->GetDBChaId() || chType != enumRESDB_TYPE_BANK) {
				//LG("enter_map", "读资源数据库错误，角色不匹配.\n");
				LG("enter_map", "Load resource database error,character is not matching.\n");
				return false;
			}
			if (!pCPly->String2BankData(i, g_buf[nIndex++])) {
				//LG("enter_map", "背包数据校验和错误.\n");
				LG("enter_map", "kitbag data check sum error.\n");
				//LG("校验和错误", "玩家（%u）的银行数据（resource_id %u）校验和错误.\n", pCPly->GetDBChaId(), pCPly->GetBankDBID(i));
				LG("check sum error", "player (%u) bank data(resource_id %u)check sum error.\n", pCPly->GetDBChaId(), pCPly->GetBankDBID(i));
				return false;
			}
		} else {
			//LG("enter_map", "读资源数据库错误，_get_row()返回值：%d.\n", r);
			LG("enter_map", "Load resource database error，_get_row() return value:%d.\n", r);
			return false;
		}
	}
	//LG("enter_map", "读银行数据成功.\n");
	LG("enter_map", "Load bank data succeed.\n");
	return true;
}

bool CTableResource::ReadBankDataEx(CPlayer* pCPly, char chBankNO) {
	if (!pCPly) {
		//LG("enter_map", "读资源数据库错误，角色不存在.\n");
		LG("enter_map", "Load resource database error,character is inexistence.\n");
		return false;
	}
	if (pCPly->GetCurBankNum() == 0) {
		long lDBID;
		if (!Create(lDBID, pCPly->GetDBChaId(), enumRESDB_TYPE_BANK))
			return false;
		pCPly->AddBankDBID(lDBID);
	}

	char chStart, chEnd;
	if (chBankNO < 0) {
		chStart = 0;
		chEnd = pCPly->GetCurBankNum() - 1;
	} else {
		if (chBankNO >= pCPly->GetCurBankNum())
			return false;
		chStart = chEnd = chBankNO;
	}

	int nIndex = 0;
	char param[] = "cha_id, type_id, content";
	char filter[80];
	std::string stringBufs[64];

	int r;
	int r1;
	for (char i = chStart; i <= chEnd; i++) {
		nIndex = 0;
		//sprintf(filter, "id=%d", pCPly->GetBankDBID(i));
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id=%d", pCPly->GetBankDBID(i));

		r = _get_row3(stringBufs, 64, param, filter);
		r1 = get_affected_rows();
		if (DBOK(r) && r1 > 0) {
			DWORD dwChaId = Str2Int(stringBufs[nIndex++]);
			char chType = Str2Int(stringBufs[nIndex++]);
			if (dwChaId != pCPly->GetDBChaId() || chType != enumRESDB_TYPE_BANK) {
				//LG("enter_map", "读资源数据库错误，角色不匹配.\n");
				LG("enter_map", "Load resource database error,character is not matching.\n");
				return false;
			}
			if (!pCPly->String2BankData(i, stringBufs[nIndex++])) {
				//LG("enter_map", "背包数据校验和错误.\n");
				LG("enter_map", "kitbag data check sum error.\n");
				//LG("校验和错误", "玩家（%u）的银行数据（resource_id %u）校验和错误.\n", pCPly->GetDBChaId(), pCPly->GetBankDBID(i));
				LG("check sum error", "player (%u) bank data(resource_id %u)check sum error.\n", pCPly->GetDBChaId(), pCPly->GetBankDBID(i));
				return false;
			}
		} else {
			//LG("enter_map", "读资源数据库错误，_get_row()返回值：%d.\n", r);
			LG("enter_map", "Load resource database error，_get_row() return value:%d.\n", r);
			return false;
		}
	}
	//LG("enter_map", "读银行数据成功.\n");
	LG("enter_map", "Load bank data succeed.\n");
	return true;
}

bool CTableResource::SaveBankData(CPlayer* pCPly, char chBankNO) {
	if (!pCPly || !pCPly->IsValid())
		return false;
	if (pCPly->GetCurBankNum() == 0)
		return true;

	char chStart, chEnd;
	if (chBankNO < 0) {
		chStart = 0;
		chEnd = pCPly->GetCurBankNum() - 1;
	} else {
		if (chBankNO >= pCPly->GetCurBankNum())
			return false;
		chStart = chEnd = chBankNO;
	}

	//LG("enter_map", "开始保存银行数据!\n");

	for (char i = chStart; i <= chEnd; i++) {
		if (!pCPly->BankWillSave(i))
			continue;
		pCPly->SetBankSaveFlag(i, false);
		//g_kitbag[0] = 0;
		memset(g_kitbag, 0, sizeof(g_kitbag));
		if (!KitbagData2String(pCPly->GetBank(i), g_kitbag, defKITBAG_DATA_STRING_LEN)) {
			//LG("enter_map", "角色%u\t保存数据（银行）时出错!\n", pCPly->GetBankDBID(i));
			LG("enter_map", "character%u\tsave data(bank) error!\n", pCPly->GetBankDBID(i));
			return false;
		}
		//LG("enter_map", "转换银行数据成功\n");

		//sprintf(g_sql, "update %s set \
		//			content='%s' \
		//			where id=%d", \
		//			_get_table(), \
		//			g_kitbag, \
		//			pCPly->GetBankDBID(i));
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
					   content='%s' \
					   where id=%d",
					_get_table(),
					g_kitbag,
					pCPly->GetBankDBID(i));

		//LG("enter_map", "组织SQL语句成功\n");
		if (strlen(g_sql) >= SQL_MAXLEN) {
			//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
			//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
			FILE* pf = nullptr;
			if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
				LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
				return false;
			}

			if (pf) {
				fprintf(pf, "%s\n\n", g_sql);
				fclose(pf);
			}
			//LG("enter_map", "SQL语句长度越界!\n");
			LG("enter_map", "SQL sentence length slop over\n");
			return false;
		}
		short sExec = exec_sql_direct(g_sql);
		//LG("enter_map", "执行SQL语句成功\n");
		if (!DBOK(sExec)) {
			//LG("enter_map", "玩家%u\t执行SQL语句时出错!\n", pCPly->GetDBChaId());
			LG("enter_map", "player %u\tcarry out SQL sentence error!\n", pCPly->GetDBChaId());
			return false;
		}
		if (DBNODATA(sExec)) {
			//LG("enter_map", "数据库没有查询到该银行资源%u!\n", pCPly->GetBankDBID(i));
			LG("enter_map", "Database couldn't find the bank resource%u!\n", pCPly->GetBankDBID(i));
			return false;
		}
	}
	//LG("enter_map", "完成全部银行[%d->%d]保存.\n", chStart, chEnd);
	LG("enter_map", "finish the whole bank[%d->%d]save\n", chStart, chEnd);
	return true;
}

bool CTableResource::SaveBankDataEx(CPlayer* pCPly, char chBankNO) {
	if (!pCPly || !pCPly->IsValid())
		return false;
	if (pCPly->GetCurBankNum() == 0)
		return true;

	char chStart, chEnd;
	if (chBankNO < 0) {
		chStart = 0;
		chEnd = pCPly->GetCurBankNum() - 1;
	} else {
		if (chBankNO >= pCPly->GetCurBankNum())
			return false;
		chStart = chEnd = chBankNO;
	}

	//LG("enter_map", "开始保存银行数据!\n");
	//char kitbag[defKITBAG_DATA_STRING_LEN];
	//char sql[1024 * 1024];
	StringPoolL stringPool;
	stringPool.ReserveBuf(StringPoolL::AllocateSpace1);
	stringPool.ReserveNum(StringPoolL::AllocateSpace2);
	//memset(look, 0, sizeof(look));
	stringPool[0] = "";

	for (char i = chStart; i <= chEnd; i++) {
		if (!pCPly->BankWillSave(i))
			continue;
		pCPly->SetBankSaveFlag(i, false);
		//g_kitbag[0] = 0;

		//memset(kitbag, 0, sizeof(g_kitbag));
		if (!KitbagData2StringEx(pCPly->GetBank(i), stringPool, 0)) {
			//LG("enter_map", "角色%u\t保存数据（银行）时出错!\n", pCPly->GetBankDBID(i));
			LG("enter_map", "character%u\tsave data(bank) error!\n", pCPly->GetBankDBID(i));
			return false;
		}
		//LG("enter_map", "转换银行数据成功\n");

		char sql[SQL_MAXLEN];

		_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set \
													content='%s' \
													where id=%d",
					_get_table(),
					(const char*)stringPool[0],
					pCPly->GetBankDBID(i));

		short sExec = exec_sql_direct(sql);
		//LG("enter_map", "执行SQL语句成功\n");
		if (!DBOK(sExec)) {
			//LG("enter_map", "玩家%u\t执行SQL语句时出错!\n", pCPly->GetDBChaId());
			LG("enter_map", "player %u\tcarry out SQL sentence error!\n", pCPly->GetDBChaId());
			return false;
		}
		if (DBNODATA(sExec)) {
			//LG("enter_map", "数据库没有查询到该银行资源%u!\n", pCPly->GetBankDBID(i));
			LG("enter_map", "Database couldn't find the bank resource%u!\n", pCPly->GetBankDBID(i));
			return false;
		}
	}
	//LG("enter_map", "完成全部银行[%d->%d]保存.\n", chStart, chEnd);
	LG("enter_map", "finish the whole bank[%d->%d]save\n", chStart, chEnd);
	return true;
}

bool CTableMapMask::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   id, cha_id, content1, content2, content3, content4, content5 \
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		//MessageBox(0, "数据库(map_mask)初始化错误，请检查", "错误", MB_OK);
		char buffer[255];
		//sprintf(buffer, RES_STRING(GM_GAMEDB_CPP_00001), "map_mask");
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "map_mask");
		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

bool CTableMapMask::GetColNameByMapName(const char* szMapName, char* szColName, long lColNameLen) {
	if (!szMapName || !szColName)
		return false;
	const char* szColBase = "content";
	if ((long)strlen(szColBase) + 1 >= lColNameLen)
		return false;

	char chIndex = 0;
	if (!strcmp(szMapName, "garner"))
		chIndex = 1;
	else if (!strcmp(szMapName, "magicsea"))
		chIndex = 2;
	else if (!strcmp(szMapName, "darkblue"))
		chIndex = 3;
	else if (!strcmp(szMapName, "winterland")) // Add by lark.li 20080812
		chIndex = 4;
	//else if (!strcmp(szMapName, "eastgoaf"))
	//	chIndex = 4;
	//else if (!strcmp(szMapName, "lonetower"))
	//	chIndex = 5;
	else
		return false;

	//sprintf(szColName, "%s%d", szColBase, chIndex);
	_snprintf_s(szColName, sizeof(szColName), _TRUNCATE, "%s%d", szColBase, chIndex);
	return true;
}

bool CTableMapMask::Create(long& lDBID, long lChaId) {
	//sprintf( g_sql, "insert %s (cha_id) \
	//				values(%d) ",	_get_table(),
	//				lChaId);
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert %s (cha_id) \
					values(%d) ",
				_get_table(),
				lChaId);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}

		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}
	short sExec = exec_sql_direct(g_sql);
	if (DBOK(sExec)) {
		std::string buf[1];
		const char* param = "@@identity";
		char filter[80];
		//sprintf(filter, "1=1");
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "1=1");

		bool ret = _get_row(buf, 1, param, filter);
		lDBID = atol(buf[0].c_str());
		return true;
	}

	return false;
}

bool CTableMapMask::ReadData(CPlayer* pCPly) {
	if (!pCPly || !pCPly->IsValid()) {
		//LG("enter_map", "读地图掩码数据库错误，角色不存在.\n");
		LG("enter_map", "Load map hideID database error,character is inexistence.\n");
		return false;
	}
	if (pCPly->GetMapMaskDBID() == 0) {
		long lDBID;
		if (!Create(lDBID, pCPly->GetDBChaId()))
			return false;
		pCPly->SetMapMaskDBID(lDBID);
	}

	char szMaskColName[30];
	if (!GetColNameByMapName(pCPly->GetMaskMapName(), szMaskColName, 30)) {
		//LG("enter_map", "选择地图掩码数据错误.\n");
		LG("enter_map", "choice map hideID data error.\n");
		return false;
	}

	int nIndex = 0;
	char param[80];
	//sprintf(param, "cha_id, %s", szMaskColName);
	_snprintf_s(param, sizeof(param), _TRUNCATE, "cha_id, %s", szMaskColName);

	char filter[80]; //sprintf(filter, "id=%d", pCPly->GetMapMaskDBID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "id=%d", pCPly->GetMapMaskDBID());
	int r = _get_row3(g_buf, g_cnCol, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		DWORD dwChaId = Str2Int(g_buf[nIndex++]);
		if (dwChaId != pCPly->GetDBChaId()) {
			//LG("enter_map", "读地图掩码数据库错误，角色不匹配.\n");
			LG("enter_map", "Load map hideID database error,character is not matching.\n");
			return false;
		}
		//if (strcmp(g_buf[nIndex-1].c_str(), "0"))
		//	LG("大地图长度", "地图 %s，长度 %u.\n", pCPly->GetMaskMapName(), strlen(g_buf[nIndex-1].c_str()));
	} else {
		//LG("enter_map", "读地图掩码数据库错误，_get_row()返回值：%d.%u\n", r);
		LG("enter_map", "Load map hideID database error,_get_row() return value:%d.%u\n", r);
		return false;
	}
	//LG("enter_map", "读大地图数据成功.\n");
	LG("enter_map", "Load big map data succeed.\n");
	return true;
}

bool CTableMapMask::SaveData(CPlayer* pCPly, BOOL bDirect) {
	if (!pCPly || !pCPly->IsValid())
		return false;

	char szMaskColName[30];
	if (!GetColNameByMapName(pCPly->GetMaskMapName(), szMaskColName, 30)) {
		//LG("enter_map", "选择地图掩码数据错误.\n");
		LG("enter_map", "choice map hideID data error.\n");
		return false;
	}

	//LG("enter_map", "开始保存大地图数据!\n");
	LG("enter_map", "start save big map data!\n");
	//sprintf(g_sql, "update %s set \
	//			   %s='%s' \
	//			   where id=%d", \
	//			   _get_table(), \
	//			   szMaskColName, \
	//			   szMaskColName, \
	//			   pCPly->GetMapMaskDBID());
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set \
				   %s='%s' \
				   where id=%d",
				_get_table(),
				szMaskColName,
				szMaskColName,
				pCPly->GetMapMaskDBID());

	//LG("enter_map", "组织大地图数据成功!\n");
	LG("enter_map", "organize big map data succeed!\n");

	if (!bDirect) {
		AddSaveQuest(g_sql);
	} else {
		return _ExecSQL(g_sql);
	}
	return true;
}

// 执行单条保存大地图的SQL语句
BOOL CTableMapMask::_ExecSQL(const char* pszSQL) {
	LETimer t;
	t.Begin();
	if (strlen(pszSQL) >= SQL_MAXLEN) {
		//FILE *pf = fopen("log\\SQL语句长度越界.txt", "a+");`
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return FALSE;
		}

		if (pf) {
			fprintf(pf, "%s\n\n", pszSQL);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return FALSE;
	}

	short sExec = exec_sql_direct(pszSQL);
	// LG("enter_map", "执行大地图SQL成功!\n");
	if (!DBOK(sExec)) {
		//LG("enter_map", "玩家执行SQL语句时出错sql = [%s]\n", pszSQL);
		LG("enter_map", "player carry out SQL sentence error sql = [%s]\n", pszSQL);
		return FALSE;
	}
	if (DBNODATA(sExec)) {
		//LG("enter_map", "数据库没有查询到该地图掩码sql = [%s]\n", pszSQL);
		LG("enter_map", "Database couldn't find the map hideID sql = [%s]\n", pszSQL);
		return FALSE;
	}
	//LG("enter_map", "完成大地图保存操作.\n");
	//LG("保存数据耗时", "保存大地图耗时[%d], 队列[%d]\n", t.End(), _SaveMapMaskList.size() - 1);
	LG("save_data_waste_time", "save big map waste time,%d,queue[%d]\n", t.End(), _SaveMapMaskList.size() - 1);
	return TRUE;
}

// 所有的大地图存盘请求一次性执行
void CTableMapMask::SaveAll() {
	for (const auto& strSQL : _SaveMapMaskList) {
		_ExecSQL(strSQL.c_str());
	}
	//LG("enter_map", "一次性执行所有大地图存盘SQL, 总计[%d]条!\n", _SaveMapMaskList.size());
	LG("enter_map", "one-off carry out every big map save SQL,totalize[%d] piece!\n", _SaveMapMaskList.size());
	_SaveMapMaskList.clear();
}

// 定时执行的大地图存盘请求
void CTableMapMask::HandleSaveList() {
	//	yyy	add!
	//RECORDCALL( __FUNCTION__ );
	DWORD dwTick = GetTickCount();
	static DWORD g_dwLastSaveTick = 0;

	if ((dwTick - g_dwLastSaveTick) > 2000) // 每两秒钟检查一次大地图存盘请求
	{
		g_dwLastSaveTick = dwTick;

		int nSize = (int)(_SaveMapMaskList.size());
		if (nSize == 0)
			return;

		std::string& strSQL = _SaveMapMaskList.front();

		_ExecSQL(strSQL.c_str());

		_SaveMapMaskList.pop_front();

		// LG("保存数据耗时", "大地图保存队列[%d]\n", nSize - 1);
	}
}

bool CTableAct::Init() {
	//sprintf(g_sql, "select \
	//			act_id, act_name, gm, cha_ids, last_ip, disc_reason, last_leave \
	//			from %s \
	//			(nolock) where 1=2", \
	//			_get_table());
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   act_id, act_name, gm, cha_ids, last_ip, disc_reason, last_leave \
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		//MessageBox(0, "数据库(account)初始化错误，请检查", "错误", MB_OK);
		char buffer[255];
		//sprintf(buffer, RES_STRING(GM_GAMEDB_CPP_00001), "account");
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "account");

		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

bool CTableAct::ReadAllData(CPlayer* pPlayer, DWORD act_id) {
	try {
		if (!pPlayer)
			return false;

		std::string buf[2];
		const char* param = "gm, act_name";
		char filter[80]; // sprintf(filter, "act_id=%d", act_id);
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "act_id=%d", act_id);
		int r = _get_row(buf, 2, param, filter);
		int r1 = get_affected_rows();
		if (DBOK(r) && r1 > 0) {
			pPlayer->SetGMLev(Str2Int(buf[0]));
			pPlayer->SetActName(buf[1].c_str());
			return true;
		}

		return false;
	}
	T_E
}

bool CTableBoat::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   boat_id, boat_berth, boat_name, boat_boatid, boat_header, boat_body, boat_engine, boat_cannon, boat_equipment, \
				   boat_bag, boat_diecount, boat_isdead, boat_ownerid, boat_createtime, boat_isdeleted, cur_endure, mx_endure, cur_supply, mx_supply, skill_state, \
				   map, map_x, map_y, angle, degree, exp \
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		//MessageBox(0, "数据库(boat)初始化错误，请检查", "错误", MB_OK);
		char buffer[255];
		//sprintf(buffer, RES_STRING(GM_GAMEDB_CPP_00001), "boat");
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "boat");

		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

BOOL CTableBoat::Create(DWORD& dwBoatID, const BOAT_DATA& Data) {
	try {
		std::string strKitbag = "";
		KitbagStringConv(Data.sCapacity, strKitbag);
		char szTime[128] = "";
		SYSTEMTIME SysTime;
		GetLocalTime(&SysTime);
		//sprintf( szTime, "%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond );
		_snprintf_s(szTime, sizeof(szTime), _TRUNCATE, "%d-%d-%d %d:%d:%d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);

		//sprintf( g_sql, "insert %s (boat_name, boat_berth, boat_boatid, boat_header, boat_body, \
	//				boat_engine, boat_cannon, boat_equipment, boat_bag, boat_diecount, \
	//				boat_isdead, boat_ownerid, boat_createtime, boat_isdeleted) \
	//				values( '%s', %d, %d, %d, %d, %d, %d, %d, '%s', %d, %d, %d, '%s', %d ) ",	_get_table(),
		//				Data.szName, Data.sBerth, Data.sBoat, Data.sHeader, Data.sBody,
		//				Data.sEngine, Data.sCannon, Data.sEquipment, strKitbag.c_str(), 0, 0, Data.dwOwnerID, szTime, 0 );
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert %s (boat_name, boat_berth, boat_boatid, boat_header, boat_body, \
					boat_engine, boat_cannon, boat_equipment, boat_bag, boat_diecount, \
					boat_isdead, boat_ownerid, boat_createtime, boat_isdeleted) \
					values( '%s', %d, %d, %d, %d, %d, %d, %d, '%s', %d, %d, %d, '%s', %d ) ",
					_get_table(),
					Data.szName, Data.sBerth, Data.sBoat, Data.sHeader, Data.sBody,
					Data.sEngine, Data.sCannon, Data.sEquipment, strKitbag.c_str(), 0, 0, Data.dwOwnerID, szTime, 0);

		if (strlen(g_sql) >= SQL_MAXLEN) {
			//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
			//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
			FILE* pf = nullptr;
			if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
				LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
				return FALSE;
			}
			if (pf) {
				fprintf(pf, "%s\n\n", g_sql);
				fclose(pf);
			}
			//LG("enter_map", "SQL语句长度越界!\n");
			LG("enter_map", "SQL sentence length slop over\n");
			return FALSE;
		}
		short sExec = exec_sql_direct(g_sql);
		if (DBOK(sExec)) {
			std::string buf[1];
			const char* param = "@@identity";
			char filter[80];
			//sprintf(filter, "1=1");
			_snprintf_s(filter, sizeof(filter), _TRUNCATE, "1=1");

			bool ret = _get_row(buf, 1, param, filter);
			dwBoatID = atol(buf[0].c_str());
			return TRUE;
		}

		return FALSE;
	}
	T_E
}

BOOL CTableBoat::GetBoat(CCharacter& Boat) {
	try {
		DWORD dwBoatID = (DWORD)Boat.getAttr(ATTR_BOAT_DBID);
		BOAT_DATA Data;
		memset(&Data, 0, sizeof(BOAT_DATA));
		int nIndex = 0;
		char param[] = "boat_name, boat_boatid, boat_berth, boat_header, boat_body, \
					boat_engine, boat_cannon, boat_equipment, boat_diecount, boat_isdead, \
					boat_ownerid, boat_isdeleted, \
					cur_endure, mx_endure, cur_supply, mx_supply, skill_state, \
					map, map_x, map_y, angle, degree, exp";
		char filter[80];
		//sprintf(filter, "boat_id=%d", dwBoatID );
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "boat_id=%d", dwBoatID);

		int r = _get_row3(g_buf, g_cnCol, param, filter);
		int r1 = get_affected_rows();
		if (DBOK(r) && r1 > 0) {
			// 船只基本属性
			//strncpy( Data.szName, g_buf[nIndex++].c_str(), BOAT_MAXSIZE_BOATNAME - 1 );
			strncpy_s(Data.szName, sizeof(Data.szName), g_buf[nIndex++].c_str(), _TRUNCATE);
			Data.sBoat = (USHORT)Str2Int(g_buf[nIndex++]);
			Data.sBerth = (USHORT)Str2Int(g_buf[nIndex++]);
			Data.sHeader = (USHORT)Str2Int(g_buf[nIndex++]);
			Data.sBody = (USHORT)Str2Int(g_buf[nIndex++]);
			Data.sEngine = (USHORT)Str2Int(g_buf[nIndex++]);
			Data.sCannon = (USHORT)Str2Int(g_buf[nIndex++]);
			Data.sEquipment = (USHORT)Str2Int(g_buf[nIndex++]);
			USHORT sDieCount = (USHORT)Str2Int(g_buf[nIndex++]);
			BYTE byIsDead = (BYTE)Str2Int(g_buf[nIndex++]);
			DWORD dwOwnerID = (DWORD)Str2Int(g_buf[nIndex++]);
			BYTE byIsDeleted = (BYTE)Str2Int(g_buf[nIndex++]);
			//if( dwOwnerID != Boat.GetPlayer()->GetDBChaId() )
			//{
			//	LG( "boat_error", "船只《%s》ID[0x%X]拥有者ID[0x%X]与当前角色《%s》ID[0x%X]不符.",
			//		Data.szName, dwBoatID, dwOwnerID,
			//		Boat.GetName(), Boat.GetPlayer()->GetDBChaId() );
			//	Boat.SystemNotice( "获取您的船只《%s》数据物主属性失败，请通知维护人员解决!谢谢!" );
			//	return FALSE;
			//}

			if (byIsDeleted == 1) {
				/*LG( "boat_error", "船只《%s》ID[0x%X]拥有者ID[0x%X]已经删除，与当前角色《%s》船长证明数据不符合.", 
				Data.szName, dwBoatID, dwOwnerID, 
				Boat.GetName() );*/
				LG("boat_error", "boat(%s)ID[0x%X]owner ID[0x%X]had delete,is not fall short of the currently character (%s) captain prove data.",
				   Data.szName, dwBoatID, dwOwnerID,
				   Boat.GetName());
				//Boat.SystemNotice( "获取您的船只《%s》数据当前状态失败，请通知维护人员解决!谢谢!", Boat.GetName());
				Boat.SystemNotice(RES_STRING(GM_GAMEDB_CPP_00020), Boat.GetName());
				return FALSE;
			}

			Boat.SetName(Data.szName);
			Boat.setAttr(ATTR_BOAT_BERTH, Data.sBerth, 1);
			Boat.setAttr(ATTR_BOAT_SHIP, Data.sBoat, 1);
			Boat.setAttr(ATTR_BOAT_HEADER, Data.sHeader, 1);
			Boat.setAttr(ATTR_BOAT_BODY, Data.sBody, 1);
			Boat.setAttr(ATTR_BOAT_ENGINE, Data.sEngine, 1);
			Boat.setAttr(ATTR_BOAT_CANNON, Data.sCannon, 1);
			Boat.setAttr(ATTR_BOAT_PART, Data.sEquipment, 1);
			Boat.setAttr(ATTR_BOAT_DIECOUNT, sDieCount, 1);
			Boat.setAttr(ATTR_BOAT_ISDEAD, byIsDead, 1);

			// 属性
			Boat.setAttr(ATTR_HP, Str2Int(g_buf[nIndex++]), 1);
			Boat.setAttr(ATTR_BMXHP, Str2Int(g_buf[nIndex++]), 1);
			Boat.setAttr(ATTR_SP, Str2Int(g_buf[nIndex++]), 1);
			Boat.setAttr(ATTR_BMXSP, Str2Int(g_buf[nIndex++]), 1);
			g_strChaState[1] = g_buf[nIndex++];
			// 位置
			Boat.SetBirthMap(g_buf[nIndex++].c_str());
			long lPosX = Str2Int(g_buf[nIndex++]);
			long lPosY = Str2Int(g_buf[nIndex++]);
			Boat.SetPos(lPosX, lPosY);
			Boat.SetAngle(Str2Int(g_buf[nIndex++]));
			// 等级
			Boat.setAttr(ATTR_LV, Str2Int(g_buf[nIndex++]), 1);
			Boat.setAttr(ATTR_CEXP, Str2Int(g_buf[nIndex++]), 1);
		} else
			return FALSE;

		if (!ReadCabin(Boat))
			return FALSE;

		//  如果船舱里有陆地道具,则为非法
		SItemGrid* pGridCont = nullptr;
		CItemRecord* pItem = nullptr;
		Short sPos = 0;
		int i = 0;
		Short sUsedNum = Boat.GetKitbag()->GetUseGridNum();
		while (i < sUsedNum) {
			pGridCont = Boat.GetKitbag()->GetGridContByNum(i);
			if (pGridCont) {
				pItem = GetItemRecordInfo(pGridCont->sID);
				if (pItem) {
					if (enumITEM_PICKTO_KITBAG == pItem->chPickTo) {
						sPos = Boat.GetKitbag()->GetPosIDByNum(i);
						LG("boat_excp", "Character %s Remove %s.\n", Boat.GetName(), pItem->szName);
						Boat.GetKitbag()->Pop(pGridCont, sPos);
						sUsedNum = Boat.GetKitbag()->GetUseGridNum();
						i = 0;
						continue;
						//return FALSE;
					}
				}
			}
			i++;
		}

		return TRUE;
	}
	T_E
}

BOOL CTableBoat::SaveBoatTempData(DWORD dwBoatID, DWORD dwOwnerID, BYTE byIsDeleted) {
	//sprintf( g_sql, "update %s set boat_ownerid=%d, boat_isdeleted=%d\
	//				where boat_id=%d", _get_table(), dwOwnerID, byIsDeleted, dwBoatID );
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set boat_ownerid=%d, boat_isdeleted=%d\
					where boat_id=%d",
				_get_table(), dwOwnerID, byIsDeleted, dwBoatID);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return FALSE;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return FALSE;
	}
	short sExec = exec_sql_direct(g_sql);
	return DBOK(sExec) && !DBNODATA(sExec);
}

BOOL CTableBoat::SaveBoatDelTag(DWORD dwBoatID, BYTE byIsDeleted) {
	//sprintf( g_sql, "update %s set boat_isdeleted=%d\
	//				where boat_id=%d", _get_table(), byIsDeleted, dwBoatID );
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set boat_isdeleted=%d\
					where boat_id=%d",
				_get_table(), byIsDeleted, dwBoatID);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return FALSE;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return FALSE;
	}
	short sExec = exec_sql_direct(g_sql);
	return DBOK(sExec) && !DBNODATA(sExec);
}

BOOL CTableBoat::SaveBoatTempData(CCharacter& Boat, BYTE byIsDeleted) {
	DWORD dwBoatID = (DWORD)Boat.getAttr(ATTR_BOAT_DBID);
	USHORT sDieCount = (USHORT)Boat.getAttr(ATTR_BOAT_DIECOUNT);
	BYTE byIsDead = (BYTE)Boat.getAttr(ATTR_BOAT_ISDEAD);
	DWORD dwOwnerID = Boat.GetPlayer()->GetDBChaId();

	//sprintf( g_sql, "update %s set boat_diecount=%d, boat_isdead=%d , boat_ownerid=%d, boat_isdeleted=%d\
	//				where boat_id=%d", _get_table(), sDieCount, byIsDead, dwOwnerID, byIsDeleted, dwBoatID );
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set boat_diecount=%d, boat_isdead=%d , boat_ownerid=%d, boat_isdeleted=%d\
					where boat_id=%d",
				_get_table(), sDieCount, byIsDead, dwOwnerID, byIsDeleted, dwBoatID);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return FALSE;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return FALSE;
	}
	short sExec = exec_sql_direct(g_sql);
	return DBOK(sExec) && !DBNODATA(sExec);
}

BOOL CTableBoat::SaveBoat(CCharacter& Boat, char chSaveType) {
	try {
		//LG("enter_map", "船 %s (%s)开始配置保存数据.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		DWORD dwBoatID = (DWORD)Boat.getAttr(ATTR_BOAT_DBID);
		USHORT sBerthID = (USHORT)Boat.getAttr(ATTR_BOAT_BERTH);
		if (chSaveType == enumSAVE_TYPE_OFFLINE) // 下线
			g_skillstate[0] = '\0';
		else // 切换地图
		{
			if (!SStateData2String(&Boat, g_skillstate, defSSTATE_DATE_STRING_LIN)) {
				//LG("enter_map", "船 %s (%s)组织状态数据不成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
				LG("enter_map", "boat %s (%s)organize state data failed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
				return false;
			}
		}
		//g_kitbag[0] = '\0';
		memset(g_kitbag, 0, sizeof(g_kitbag));
		KitbagData2String(Boat.GetKitbag(), g_kitbag, defKITBAG_DATA_STRING_LEN);
		//LG("enter_map", "配置状态数据成功.\n");

		bool bWithPos = false;
		if (Boat.GetPlyCtrlCha()->GetSubMap())
			bWithPos = Boat.GetPlyCtrlCha()->GetSubMap()->CanSavePos();
		if (bWithPos)
			//sprintf( g_sql, "update %s set boat_berth=%d, boat_ownerid=%d, cur_endure=%d, mx_endure=%d, cur_supply=%d, mx_supply=%d, skill_state='%s', \
		//				map='%s', map_x=%d, map_y=%d, angle=%d, degree=%d, exp=%d, boat_bag='%s' \
		//				where boat_id=%d",
			//	_get_table(),
			//	sBerthID,
			//	( Boat.GetPlayer() ) ? Boat.GetPlayer()->GetDBChaId() : 0,
			//	(long)Boat.getAttr(ATTR_HP),
			//	(long)Boat.getAttr(ATTR_BMXHP),
			//	(long)Boat.getAttr(ATTR_SP),
			//	(long)Boat.getAttr(ATTR_BMXSP),
			//	g_skillstate,
			//	Boat.GetBirthMap(),
			//	Boat.GetPos().x,
			//	Boat.GetPos().y,
			//	Boat.GetAngle(),
			//	(long)Boat.getAttr(ATTR_LV),
			//	(long)Boat.getAttr(ATTR_CEXP),
			//	g_kitbag,
			//	dwBoatID );
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set boat_berth=%d, boat_ownerid=%d, cur_endure=%d, mx_endure=%d, cur_supply=%d, mx_supply=%d, skill_state='%s', \
						map='%s', map_x=%d, map_y=%d, angle=%d, degree=%d, exp=%d, boat_bag='%s' \
						where boat_id=%d",
						_get_table(),
						sBerthID,
						(Boat.GetPlayer()) ? Boat.GetPlayer()->GetDBChaId() : 0,
						(long)Boat.getAttr(ATTR_HP),
						(long)Boat.getAttr(ATTR_BMXHP),
						(long)Boat.getAttr(ATTR_SP),
						(long)Boat.getAttr(ATTR_BMXSP),
						g_skillstate,
						Boat.GetBirthMap(),
						Boat.GetPos().x,
						Boat.GetPos().y,
						Boat.GetAngle(),
						(long)Boat.getAttr(ATTR_LV),
						(long)Boat.getAttr(ATTR_CEXP),
						g_kitbag,
						dwBoatID);

		else
			//sprintf( g_sql, "update %s set boat_berth=%d, boat_ownerid=%d, cur_endure=%d, mx_endure=%d, cur_supply=%d, mx_supply=%d, skill_state='%s', \
		//				degree=%d, exp=%d, boat_bag='%s' \
		//				where boat_id=%d",
			//	_get_table(),
			//	sBerthID,
			//	( Boat.GetPlayer() ) ? Boat.GetPlayer()->GetDBChaId() : 0,
			//	(long)Boat.getAttr(ATTR_HP),
			//	(long)Boat.getAttr(ATTR_BMXHP),
			//	(long)Boat.getAttr(ATTR_SP),
			//	(long)Boat.getAttr(ATTR_BMXSP),
			//	g_skillstate,
			//	(long)Boat.getAttr(ATTR_LV),
			//	(long)Boat.getAttr(ATTR_CEXP),
			//	g_kitbag,
			//	dwBoatID );
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set boat_berth=%d, boat_ownerid=%d, cur_endure=%d, mx_endure=%d, cur_supply=%d, mx_supply=%d, skill_state='%s', \
						degree=%d, exp=%d, boat_bag='%s' \
						where boat_id=%d",
						_get_table(),
						sBerthID,
						(Boat.GetPlayer()) ? Boat.GetPlayer()->GetDBChaId() : 0,
						(long)Boat.getAttr(ATTR_HP),
						(long)Boat.getAttr(ATTR_BMXHP),
						(long)Boat.getAttr(ATTR_SP),
						(long)Boat.getAttr(ATTR_BMXSP),
						g_skillstate,
						(long)Boat.getAttr(ATTR_LV),
						(long)Boat.getAttr(ATTR_CEXP),
						g_kitbag,
						dwBoatID);

		//LG("enter_map", "组织SQL语句成功.\n");

		if (strlen(g_sql) >= SQL_MAXLEN) {
			//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
			//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
			FILE* pf = nullptr;
			if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
				LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
				return FALSE;
			}
			if (pf) {
				fprintf(pf, "%s\n\n", g_sql);
				fclose(pf);
			}
			//LG("enter_map", "SQL语句长度越界!\n");
			LG("enter_map", "SQL sentence length slop over\n");
			return FALSE;
		}
		short sExec = exec_sql_direct(g_sql);
		//LG("enter_map", "执行SQL语句成功.\n");

		if (!DBOK(sExec)) {
			//LG("enter_map", "船 %s (%s)保存基本数据不成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
			LG("enter_map", "boat %s (%s)save basic data failed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
			return false;
		}
		if (DBNODATA(sExec)) {
			//LG("enter_map", "数据库没有查询到该玩家%u!\n", dwBoatID);
			LG("enter_map", "Database couldn't find the character%u!\n", dwBoatID);
			return false;
		}
		//LG("enter_map", "船保存基本数据成功.\n");

		//if (!SaveCabin(Boat, chSaveType))
		//	return false;

		//LG("enter_map", "船 %s (%s)全部数据保存成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		LG("enter_map", "boat %s (%s) the whole data save succeed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());

		return true;
	}
	T_E
}

BOOL CTableBoat::SaveBoatEx(CCharacter& Boat, char chSaveType) {
	try {
		//LG("enter_map", "船 %s (%s)开始配置保存数据.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		DWORD dwBoatID = (DWORD)Boat.getAttr(ATTR_BOAT_DBID);
		USHORT sBerthID = (USHORT)Boat.getAttr(ATTR_BOAT_BERTH);
		//char skillstate[defSSTATE_DATE_STRING_LIN];
		//memset(skillstate, 0, sizeof(skillstate));

		StringPoolL stringPool;
		stringPool.ReserveBuf(StringPoolL::AllocateSpace1);
		stringPool.ReserveNum(StringPoolL::AllocateSpace2);
		//memset(look, 0, sizeof(look));
		stringPool[0] = "";
		if (chSaveType != enumSAVE_TYPE_OFFLINE) // 下线
		{
			if (!SStateData2StringEx(&Boat, stringPool, 0)) {
				//LG("enter_map", "船 %s (%s)组织状态数据不成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
				LG("enter_map", "boat %s (%s)organize state data failed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
				return false;
			}
		}
		//g_kitbag[0] = '\0';
		//char kitbag[defKITBAG_DATA_STRING_LEN];

		//memset(kitbag, 0, sizeof(g_kitbag));
		stringPool[1] = "";

		KitbagData2StringEx(Boat.GetKitbag(), stringPool, 1);
		//LG("enter_map", "配置状态数据成功.\n");

		char sql[SQL_MAXLEN];

		bool bWithPos = false;
		if (Boat.GetPlyCtrlCha()->GetSubMap())
			bWithPos = Boat.GetPlyCtrlCha()->GetSubMap()->CanSavePos();
		if (bWithPos)
			_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set boat_berth=%d, boat_ownerid=%d, cur_endure=%d, mx_endure=%d, cur_supply=%d, mx_supply=%d, skill_state='%s', \
						map='%s', map_x=%d, map_y=%d, angle=%d, degree=%d, exp=%d, boat_bag='%s' \
						where boat_id=%d",
						_get_table(),
						sBerthID,
						(Boat.GetPlayer()) ? Boat.GetPlayer()->GetDBChaId() : 0,
						(long)Boat.getAttr(ATTR_HP),
						(long)Boat.getAttr(ATTR_BMXHP),
						(long)Boat.getAttr(ATTR_SP),
						(long)Boat.getAttr(ATTR_BMXSP),
						(const char*)stringPool[0],
						Boat.GetBirthMap(),
						Boat.GetPos().x,
						Boat.GetPos().y,
						Boat.GetAngle(),
						(long)Boat.getAttr(ATTR_LV),
						(long)Boat.getAttr(ATTR_CEXP),
						(const char*)stringPool[1],
						dwBoatID);

		else
			_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set boat_berth=%d, boat_ownerid=%d, cur_endure=%d, mx_endure=%d, cur_supply=%d, mx_supply=%d, skill_state='%s', \
						degree=%d, exp=%d, boat_bag='%s' \
						where boat_id=%d",
						_get_table(),
						sBerthID,
						(Boat.GetPlayer()) ? Boat.GetPlayer()->GetDBChaId() : 0,
						(long)Boat.getAttr(ATTR_HP),
						(long)Boat.getAttr(ATTR_BMXHP),
						(long)Boat.getAttr(ATTR_SP),
						(long)Boat.getAttr(ATTR_BMXSP),
						(const char*)stringPool[0],
						(long)Boat.getAttr(ATTR_LV),
						(long)Boat.getAttr(ATTR_CEXP),
						(const char*)stringPool[1],
						dwBoatID);

		//LG("enter_map", "组织SQL语句成功.\n");

		short sExec = exec_sql_direct(sql);
		//LG("enter_map", "执行SQL语句成功.\n");

		if (!DBOK(sExec)) {
			//LG("enter_map", "船 %s (%s)保存基本数据不成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
			LG("enter_map", "boat %s (%s)save basic data failed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
			return false;
		}
		if (DBNODATA(sExec)) {
			//LG("enter_map", "数据库没有查询到该玩家%u!\n", dwBoatID);
			LG("enter_map", "Database couldn't find the character%u!\n", dwBoatID);
			return false;
		}
		//LG("enter_map", "船保存基本数据成功.\n");

		//if (!SaveCabin(Boat, chSaveType))
		//	return false;

		//LG("enter_map", "船 %s (%s)全部数据保存成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		LG("enter_map", "boat %s (%s) the whole data save succeed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());

		return true;
	}
	T_E
}

bool CTableBoat::SaveAllData(CPlayer* pPlayer, char chSaveType) {
	if (!pPlayer)
		return false;
	CCharacter* pCBoat;
	for (BYTE i = 0; i < pPlayer->GetNumBoat(); i++) {
		pCBoat = pPlayer->GetBoat(i);
		if (!pCBoat)
			continue;
		if (!SaveBoat(*pCBoat, chSaveType))
			return false;
	}
	//LG("enter_map", "保存所有船数据成功.\n");
	LG("enter_map", "save the whole boat data succeed\n");

	return true;
}

bool CTableBoat::SaveAllDataEx(CPlayer* pPlayer, char chSaveType) {
	if (!pPlayer)
		return false;
	CCharacter* pCBoat;
	for (BYTE i = 0; i < pPlayer->GetNumBoat(); i++) {
		pCBoat = pPlayer->GetBoat(i);
		if (!pCBoat)
			continue;
		if (!SaveBoatEx(*pCBoat, chSaveType))
			return false;
	}
	//LG("enter_map", "保存所有船数据成功.\n");
	LG("enter_map", "save the whole boat data succeed\n");

	return true;
}

bool CTableBoat::SaveCabin(CCharacter& Boat, char chSaveType) {
	DWORD dwBoatID = (DWORD)Boat.getAttr(ATTR_BOAT_DBID);
	int kb_capacity = Boat.GetKitbag()->GetCapacity();
	//g_kitbag[0] = '\0';
	memset(g_kitbag, 0, sizeof(g_kitbag));
	KitbagData2String(Boat.GetKitbag(), g_kitbag, defKITBAG_DATA_STRING_LEN);

	//sprintf( g_sql, "update %s set boat_bag='%s' where boat_id=%d", _get_table(),
	//	g_kitbag,
	//	dwBoatID );
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set boat_bag='%s' where boat_id=%d", _get_table(),
				g_kitbag,
				dwBoatID);

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}
	short sExec = exec_sql_direct(g_sql);

	if (!DBOK(sExec)) {
		//LG("enter_map", "船 %s (%s)的船仓数据保存不成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		LG("enter_map", "boat %s (%s) cabin data save failed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		return false;
	}
	if (DBNODATA(sExec)) {
		//LG("enter_map", "数据库没有查询到该玩家%u!\n", dwBoatID);
		LG("enter_map", "Database couldn't find the character%u!\n", dwBoatID);
		return false;
	}
	//LG("enter_map", "船 %s (%s)的船仓数据保存成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
	LG("enter_map", "boat %s (%s)cabin data save succeed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());

	return true;
}

bool CTableBoat::SaveAllCabin(CPlayer* pPlayer, char chSaveType) {
	CCharacter* pCBoat;
	for (BYTE i = 0; i < pPlayer->GetNumBoat(); i++) {
		pCBoat = pPlayer->GetBoat(i);
		if (!pCBoat)
			continue;
		if (!SaveCabin(*pCBoat, chSaveType))
			return false;
	}

	return true;
}

bool CTableBoat::ReadCabin(CCharacter& Boat) // 读取船舱
{
	DWORD dwBoatID = (DWORD)Boat.getAttr(ATTR_BOAT_DBID);
	int nIndex = 0;
	char param[] = "boat_bag";
	char filter[80];
	//sprintf(filter, "boat_id=%d", dwBoatID );
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "boat_id=%d", dwBoatID);

	int r = _get_row3(g_buf, g_cnCol, param, filter);
	int r1 = get_affected_rows();
	if (DBOK(r) && r1 > 0) {
		//LG("enter_map", "船 %u (%s, %s)的船仓数据 %s.\n", dwBoatID, Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName(), g_buf[nIndex].c_str());
		if (!Boat.String2KitbagData(g_buf[nIndex++])) {
			//LG("enter_map", "船仓数据校验和错误.\n");
			LG("enter_map", "cabin data check sum error.\n");
			//LG("校验和错误", "船（%s）船仓数据（boat_id %u）校验和错误.\n", Boat.GetLogName(), Boat.getAttr( ATTR_BOAT_DBID ));
			LG("check sum error", "boat (%s) cabin data (boat_id %u)check sum error.\n", Boat.GetLogName(), Boat.getAttr(ATTR_BOAT_DBID));
			return false;
		}

		//LG("enter_map", "船 %s (%s)的船仓数据设置成功.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		LG("enter_map", "boat %s (%s) cabin data set succeed.\n", Boat.GetLogName(), Boat.GetPlyMainCha()->GetLogName());
		return true;
	}

	return false;
}

BOOL CGameDB::Init() {
	try {
		m_bInitOK = FALSE;

		printf("Connectting database [%s : %s]......\n", g_Config.m_szDBName, g_Config.m_szDBIP);

		std::string err_info;
		//if(!pswd.c_str() || pswd.length() == 0)
		//{
		//	LG("gamedb", "Database  Password Error!");
		//	return FALSE;
		//}
		bool r = _connect.connect(g_Config.m_szDBIP, g_Config.m_szDBName, g_Config.m_szDBUsr, g_Config.m_szDBPass, err_info);
		if (!r) {
			char msg[256];
			_snprintf_s(msg, _countof(msg), _TRUNCATE, "Database [%s] Connect Failed!, ERROR REPORT[%s]", g_Config.m_szDBName, err_info.c_str());
			MessageBox(nullptr, msg, "error", MB_ICONERROR | MB_OK);
			return FALSE;
		}

		printf("Database Connected!\n");

		_tab_cha = new CTableCha(&_connect);
		_tab_master = new CTableMaster(&_connect);
		_tab_res = new CTableResource(&_connect);

		// Add by lark.li 20080521 begin
		_tab_setting = new CTableLotterySetting(&_connect);
		_tab_ticket = new CTableTicket(&_connect);
		_tab_winticket = new CTableWinTicket(&_connect);

		_tab_amp_setting = new CTableAmphitheaterSetting(&_connect);
		_tab_amp_team = new CTableAmphitheaterTeam(&_connect);
		// End
		_tab_per_info = new CTablePersoninfo(&_connect); //Add by sunny.sun 20080822
#ifdef SHUI_JING
		_tab_csl_trade = new CTableCrystalTrade(&_connect);
#endif
		_tab_mmask = new CTableMapMask(&_connect);
		_tab_act = new CTableAct(&_connect);
		_tab_gld = new CTableGuild(&_connect);
		_tab_boat = new CTableBoat(&_connect);
		_tab_log = new CTableLog(&_connect);
		_tab_item = new CTableItem(&_connect);

		// Modify by lark.li 20080521
		//if (!_tab_cha || !_tab_act || !_tab_gld || !_tab_boat || !_tab_master)
#ifdef SHUI_JING
		if (!_tab_cha || !_tab_act || !_tab_gld || !_tab_boat || !_tab_master || !_tab_setting || !_tab_ticket || !_tab_winticket || !_tab_amp_setting || !_tab_amp_team || !_tab_log || !_tab_item || !_tab_per_info || !_tab_csl_trade)
#else
		if (!_tab_cha || !_tab_act || !_tab_gld || !_tab_boat || !_tab_master || !_tab_setting || !_tab_ticket || !_tab_winticket || !_tab_amp_setting || !_tab_amp_team || !_tab_log || !_tab_item || !_tab_per_info)
#endif
			return FALSE;

			//if (!_tab_cha->Init() || !_tab_act->Init() || !_tab_gld->Init() || !_tab_boat->Init() || !_tab_res->Init() || !_tab_mmask->Init() || !_tab_master->Init())
#ifdef SHUI_JING
		if (!_tab_cha->Init() || !_tab_act->Init() || !_tab_gld->Init() || !_tab_boat->Init() || !_tab_res->Init() || !_tab_mmask->Init() || !_tab_master->Init() || !_tab_setting->Init() || !_tab_ticket->Init() || !_tab_winticket->Init() || !_tab_amp_setting->Init() || !_tab_amp_team->Init() || !_tab_per_info->Init() || !_tab_csl_trade->Init())
#else
		if (!_tab_cha->Init() || !_tab_act->Init() || !_tab_gld->Init() || !_tab_boat->Init() || !_tab_res->Init() || !_tab_mmask->Init() || !_tab_master->Init() || !_tab_setting->Init() || !_tab_ticket->Init() || !_tab_winticket->Init() || !_tab_amp_setting->Init() || !_tab_amp_team->Init() || !_tab_per_info->Init())
#endif
			return FALSE;

		//int issue;
		//_tab_setting->GetCurrentIssue(issue);
		//_tab_setting->AddIssue(10);
		//_tab_setting->DisuseIssue(1, enumDISUSE);
		//_tab_ticket->AddTicket(1 , "123456", 1);
		//_tab_ticket->AddTicket(1 , "12345X", 1);
		//_tab_ticket->AddTicket(1 , "12345X", 1);
		//_tab_ticket->Exchange(1 , "123456");

		if (!_tab_log) {
			//LG("init", "gamelog数据表初始化失败\n");
			LG("init", "gamelog data list init failed\n");
		}

		m_bInitOK = TRUE;

		return TRUE;
	}
	T_E
}

bool CGameDB::ReadPlayer(CPlayer* pPlayer, DWORD cha_id) {
	m_Lock.Lock();
#ifdef UPDATE_MUL_THREAD
	pPlayer->DBLock();
#endif
	try {
		if (!_tab_cha->ReadAllData(pPlayer, cha_id)) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		long lKbDBID = pPlayer->GetMainCha()->GetKitbagRecDBID();
		long lkbTmpDBID = pPlayer->GetMainCha()->GetKitbagTmpRecDBID(); //临时背包ID
		long lMMaskDBID = pPlayer->GetMapMaskDBID();
		long lBankNum = pPlayer->GetCurBankNum();
		if (!_tab_res->ReadKitbagData(pPlayer->GetMainCha())) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		if (lKbDBID == 0) {
			if (!SavePlayerKBagDBID(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
				pPlayer->DBUnLock();
#endif
				m_Lock.Unlock();
				return false;
			}
		}

		if (!_tab_res->ReadKitbagTmpData(pPlayer->GetMainCha())) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		if (lkbTmpDBID == 0) {
			if (!SavePlayerKBagTmpDBID(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
				pPlayer->DBUnLock();
#endif
				m_Lock.Unlock();
				return false;
			}
		}

		pPlayer->GetMainCha()->LogAssets(enumLASSETS_INIT);

		if (!_tab_res->ReadBankData(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		if (lBankNum == 0) {
			if (!_tab_cha->SaveBankDBID(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
				pPlayer->DBUnLock();
#endif
				m_Lock.Unlock();
				return false;
			}
		}

		if (!_tab_act->ReadAllData(pPlayer, pPlayer->GetDBActId())) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		// 行会信息
		if (pPlayer->m_lGuildID > 0) {
			_tab_gld->GetGuildInfo(pPlayer->GetMainCha(), pPlayer->m_lGuildID);
			//long	lType = _tab_gld->GetTypeByID(pPlayer->GetMainCha()->getAttr(ATTR_GUILD));
			//if (lType >= 0)
			//	pPlayer->GetMainCha()->setAttr(ATTR_GUILD_TYPE, lType, 1);
		}
		//LG("enter_map", "读玩家全部数据成功.\n");
		LG("enter_map", "Load the character whole data succeed.\n");

		// 记录到文件
		CKitbag* pCKb;
		CCharacter* pCMainC = pPlayer->GetMainCha();
		short sItemNum = pCMainC->GetKitbag()->GetUseGridNum();
		//g_kitbag[0] = '\0';
		//memset(g_kitbag, 0, sizeof(g_kitbag));
		//sprintf(g_kitbag, "金钱：%u 道具：%d@", pCMainC->getAttr(ATTR_GD), sItemNum);
		//sprintf(g_kitbag, RES_STRING(GM_GAMEDB_CPP_00021), pCMainC->getAttr(ATTR_GD), sItemNum);
		//_snprintf_s(g_kitbag,sizeof(g_kitbag),_TRUNCATE,RES_STRING(GM_GAMEDB_CPP_00021), pCMainC->getAttr(ATTR_GD), sItemNum);
		CFormatParameter param(2);
		param.setInt64(0, pCMainC->getAttr(ATTR_GD));
		param.setDouble(1, sItemNum);
		RES_FORMAT_STRING(GM_GAMEDB_CPP_00021, param, g_kitbag);

		SItemGrid* pGridCont;
		CItemRecord* pCItem;
		pCKb = pCMainC->GetKitbag();
		for (short i = 0; i < sItemNum; i++) {
			pGridCont = pCKb->GetGridContByNum(i);
			if (!pGridCont)
				continue;
			pCItem = GetItemRecordInfo(pGridCont->sID);
			if (!pCItem)
				continue;
			//sprintf(g_kitbag + strlen(g_kitbag), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			_snprintf_s(g_kitbag + strlen(g_kitbag), sizeof(g_kitbag) - strlen(g_kitbag), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
		}
		TL(CHA_ENTER, pCMainC->GetName(), "", g_kitbag);

		short sItemTmpNum = pCMainC->m_pCKitbagTmp->GetUseGridNum();
		//g_kitbagTmp[0] = '\0';
		//sprintf(g_kitbagTmp, "临时背包：%d@", sItemTmpNum);
		//sprintf(g_kitbagTmp, RES_STRING(GM_GAMEDB_CPP_00022), sItemTmpNum);
		_snprintf_s(g_kitbagTmp, sizeof(g_kitbagTmp), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00022), sItemTmpNum);

		pCKb = pCMainC->m_pCKitbagTmp;
		for (short i = 0; i < sItemTmpNum; i++) {
			pGridCont = pCKb->GetGridContByNum(i);
			if (!pGridCont)
				continue;
			pCItem = GetItemRecordInfo(pGridCont->sID);
			if (!pCItem)
				continue;
			//sprintf(g_kitbagTmp + strlen(g_kitbagTmp), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			_snprintf_s(g_kitbagTmp + strlen(g_kitbagTmp), sizeof(g_kitbagTmp) - strlen(g_kitbagTmp), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
		}
		TL(CHA_ENTER, pCMainC->GetName(), "", g_kitbagTmp);

		char chStart = 0, chEnd = pPlayer->GetCurBankNum() - 1;
		for (char i = chStart; i <= chEnd; i++) {
			//sprintf(g_kitbag, "银行ID(%d):", i + 1 );
			//sprintf(g_kitbag, RES_STRING(GM_GAMEDB_CPP_00023), i + 1 );
			_snprintf_s(g_kitbag, sizeof(g_kitbag), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00023), i + 1);

			pCKb = pPlayer->GetBank(i);
			sItemNum = pCKb->GetUseGridNum();
			//sprintf(g_kitbag + strlen(g_kitbag), "[%d]%d@;", i + 1, sItemNum);
			_snprintf_s(g_kitbag + strlen(g_kitbag), sizeof(g_kitbag) - strlen(g_kitbag), _TRUNCATE, "[%d]%d@;", i + 1, sItemNum);

			for (short i = 0; i < sItemNum; i++) {
				pGridCont = pCKb->GetGridContByNum(i);
				if (!pGridCont)
					continue;
				pCItem = GetItemRecordInfo(pGridCont->sID);
				if (!pCItem)
					continue;
				//sprintf(g_kitbag + strlen(g_kitbag), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
				_snprintf_s(g_kitbag + strlen(g_kitbag), sizeof(g_kitbag) - strlen(g_kitbag), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			}
			TL(CHA_ENTER, pCMainC->GetName(), "", g_kitbag);
		}

		//g_equip[0] = '\0';
		//sprintf(g_equip, "装备：%d@", enumEQUIP_NUM);
		//sprintf(g_equip, RES_STRING(GM_GAMEDB_CPP_00024), enumEQUIP_NUM);
		_snprintf_s(g_equip, sizeof(g_equip), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00024), enumEQUIP_NUM);

		for (short i = 0; i < enumEQUIP_NUM; i++) {
			pGridCont = &pCMainC->m_SChaPart.SLink[i];
			if (!pGridCont)
				continue;
			pCItem = GetItemRecordInfo(pGridCont->sID);
			if (!pCItem)
				continue;
			//sprintf(g_equip + strlen(g_equip), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			_snprintf_s(g_equip + strlen(g_equip), sizeof(g_equip) - strlen(g_equip), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
		}
		TL(CHA_EQUIP, pCMainC->GetName(), "", g_equip);
	} catch (...) {
	}

#ifdef UPDATE_MUL_THREAD
	pPlayer->DBUnLock();
#endif
	m_Lock.Unlock();
	//
	return true;
}

bool CGameDB::ReadPlayerEx(CPlayer* pPlayer, DWORD cha_id) {
	m_LockEx.Lock();
#ifdef UPDATE_MUL_THREAD
	pPlayer->DBLock();
#endif
	try {
		if (!_tab_cha->ReadAllDataEx(pPlayer, cha_id)) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		long lKbDBID = pPlayer->GetMainCha()->GetKitbagRecDBID();
		long lkbTmpDBID = pPlayer->GetMainCha()->GetKitbagTmpRecDBID(); //临时背包ID
		long lMMaskDBID = pPlayer->GetMapMaskDBID();
		long lBankNum = pPlayer->GetCurBankNum();
		if (!_tab_res->ReadKitbagData(pPlayer->GetMainCha())) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		if (lKbDBID == 0) {
			if (!SavePlayerKBagDBIDEx(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
				pPlayer->DBUnLock();
#endif
				m_LockEx.Unlock();
				return false;
			}
		}

		if (!_tab_res->ReadKitbagTmpDataEx(pPlayer->GetMainCha())) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		if (lkbTmpDBID == 0) {
			if (!SavePlayerKBagTmpDBIDEx(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
				pPlayer->DBUnLock();
#endif
				m_LockEx.Unlock();
				return false;
			}
		}

		pPlayer->GetMainCha()->LogAssets(enumLASSETS_INIT);

		if (!_tab_res->ReadBankDataEx(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		if (lBankNum == 0) {
			if (!_tab_cha->SaveBankDBIDEx(pPlayer)) {
#ifdef UPDATE_MUL_THREAD
				pPlayer->DBUnLock();
#endif
				m_LockEx.Unlock();
				return false;
			}
		}

		if (!_tab_act->ReadAllData(pPlayer, pPlayer->GetDBActId())) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		// 行会信息
		if (pPlayer->m_lGuildID > 0) {
			_tab_gld->GetGuildInfo(pPlayer->GetMainCha(), pPlayer->m_lGuildID);
			//long	lType = _tab_gld->GetTypeByID(pPlayer->GetMainCha()->getAttr(ATTR_GUILD));
			//if (lType >= 0)
			//	pPlayer->GetMainCha()->setAttr(ATTR_GUILD_TYPE, lType, 1);
		}
		//LG("enter_map", "读玩家全部数据成功.\n");
		LG("enter_map", "Load the character whole data succeed.\n");

		// 记录到文件
		CKitbag* pCKb;
		CCharacter* pCMainC = pPlayer->GetMainCha();
		short sItemNum = pCMainC->GetKitbag()->GetUseGridNum();
		//g_kitbag[0] = '\0';
		//memset(g_kitbag, 0, sizeof(g_kitbag));
		//sprintf(g_kitbag, "金钱：%u 道具：%d@", pCMainC->getAttr(ATTR_GD), sItemNum);
		//sprintf(g_kitbag, RES_STRING(GM_GAMEDB_CPP_00021), pCMainC->getAttr(ATTR_GD), sItemNum);
		//_snprintf_s(g_kitbag,sizeof(g_kitbag),_TRUNCATE,RES_STRING(GM_GAMEDB_CPP_00021), pCMainC->getAttr(ATTR_GD), sItemNum);
		CFormatParameter param(2);
		param.setInt64(0, pCMainC->getAttr(ATTR_GD));
		param.setDouble(1, sItemNum);
		RES_FORMAT_STRING(GM_GAMEDB_CPP_00021, param, g_kitbag);

		SItemGrid* pGridCont;
		CItemRecord* pCItem;
		pCKb = pCMainC->GetKitbag();

		char kitbag[8192];

		for (short i = 0; i < sItemNum; i++) {
			pGridCont = pCKb->GetGridContByNum(i);
			if (!pGridCont)
				continue;
			pCItem = GetItemRecordInfo(pGridCont->sID);
			if (!pCItem)
				continue;
			//sprintf(g_kitbag + strlen(g_kitbag), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			_snprintf_s(kitbag + strlen(kitbag), sizeof(kitbag) - strlen(kitbag), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
		}
		TL(CHA_ENTER, pCMainC->GetName(), "", kitbag);

		short sItemTmpNum = pCMainC->m_pCKitbagTmp->GetUseGridNum();
		char kitbagTmp[8192];

		//g_kitbagTmp[0] = '\0';
		//sprintf(g_kitbagTmp, "临时背包：%d@", sItemTmpNum);
		//sprintf(g_kitbagTmp, RES_STRING(GM_GAMEDB_CPP_00022), sItemTmpNum);
		_snprintf_s(kitbagTmp, sizeof(kitbagTmp), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00022), sItemTmpNum);

		pCKb = pCMainC->m_pCKitbagTmp;
		for (short i = 0; i < sItemTmpNum; i++) {
			pGridCont = pCKb->GetGridContByNum(i);
			if (!pGridCont)
				continue;
			pCItem = GetItemRecordInfo(pGridCont->sID);
			if (!pCItem)
				continue;
			//sprintf(g_kitbagTmp + strlen(g_kitbagTmp), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			_snprintf_s(kitbagTmp + strlen(kitbagTmp), sizeof(kitbagTmp) - strlen(kitbagTmp), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
		}
		TL(CHA_ENTER, pCMainC->GetName(), "", kitbagTmp);

		char chStart = 0, chEnd = pPlayer->GetCurBankNum() - 1;
		for (char i = chStart; i <= chEnd; i++) {
			//sprintf(g_kitbag, "银行ID(%d):", i + 1 );
			//sprintf(g_kitbag, RES_STRING(GM_GAMEDB_CPP_00023), i + 1 );
			_snprintf_s(kitbag, sizeof(kitbag), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00023), i + 1);

			pCKb = pPlayer->GetBank(i);
			sItemNum = pCKb->GetUseGridNum();
			//sprintf(g_kitbag + strlen(g_kitbag), "[%d]%d@;", i + 1, sItemNum);
			_snprintf_s(kitbag + strlen(kitbag), sizeof(kitbag) - strlen(kitbag), _TRUNCATE, "[%d]%d@;", i + 1, sItemNum);

			for (short i = 0; i < sItemNum; i++) {
				pGridCont = pCKb->GetGridContByNum(i);
				if (!pGridCont)
					continue;
				pCItem = GetItemRecordInfo(pGridCont->sID);
				if (!pCItem)
					continue;
				//sprintf(g_kitbag + strlen(g_kitbag), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
				_snprintf_s(kitbag + strlen(kitbag), sizeof(kitbag) - strlen(kitbag), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			}
			TL(CHA_ENTER, pCMainC->GetName(), "", kitbag);
		}

		//g_equip[0] = '\0';
		//sprintf(g_equip, "装备：%d@", enumEQUIP_NUM);
		//sprintf(g_equip, RES_STRING(GM_GAMEDB_CPP_00024), enumEQUIP_NUM);
		char equip[8192];

		_snprintf_s(equip, sizeof(equip), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00024), enumEQUIP_NUM);

		for (short i = 0; i < enumEQUIP_NUM; i++) {
			pGridCont = &pCMainC->m_SChaPart.SLink[i];
			if (!pGridCont)
				continue;
			pCItem = GetItemRecordInfo(pGridCont->sID);
			if (!pCItem)
				continue;
			//sprintf(g_equip + strlen(g_equip), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			_snprintf_s(equip + strlen(equip), sizeof(equip) - strlen(equip), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
		}
		TL(CHA_EQUIP, pCMainC->GetName(), "", equip);
	} catch (...) {
	}

#ifdef UPDATE_MUL_THREAD
	pPlayer->DBUnLock();
#endif
	m_LockEx.Unlock();
	//
	return true;
}

bool CGameDB::SavePlayer(CPlayer* pPlayer, char chSaveType, cChar* szTarMapName, Long lTarX, Long lTarY) {
	//return true;

	m_Lock.Lock();
#ifdef UPDATE_MUL_THREAD
	pPlayer->DBLock();
#endif
	try {
		if (!pPlayer || !pPlayer->GetMainCha()) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		if (pPlayer->GetMainCha()->GetPlayer() != pPlayer) {
			//LG("保存角色重大错误", "保存的Player地址 %p[dbid %u]，该玩家的主角 %s，该角色的Player地址 %p）\n",
			LG("save_character_error", "save Player address %p[dbid %u]，the character main player %s，the character 's Player address %p）\n",
			   pPlayer, pPlayer->GetDBChaId(), pPlayer->GetMainCha()->GetLogName(), pPlayer->GetMainCha()->GetPlayer());
			//pPlayer->SystemNotice("玩家角色不匹配，保存资料库不成功");
			pPlayer->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00025));

#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		bool bSaveMainCha = false, bSaveBoat = false, bSaveKitBag = false, bSaveMMask = false, bSaveBank = false;
		bool bSaveKitBagTmp = false;
		bool bSaveKBState = false;
		BeginTran();
		try {
			DWORD dwStartTick = GetTickCount();

			bSaveMainCha = _tab_cha->SaveAllData(pPlayer, chSaveType, szTarMapName, lTarX, lTarY); // 保存主角色
			DWORD dwSaveMainTick = GetTickCount();
			bSaveKitBag = _tab_res->SaveKitbagData(pPlayer->GetMainCha());
			//保存临时背包
			bSaveKitBagTmp = _tab_res->SaveKitbagTmpData(pPlayer->GetMainCha());
			//保存密码锁定状态
			//bSaveKBState = _tab_cha->SaveKBState(pPlayer);
			DWORD dwSaveKbTick = GetTickCount();
			bSaveBank = _tab_res->SaveBankData(pPlayer);
			DWORD dwSaveBankTick = GetTickCount();
			// Delete by lark.li 20080926 begin
			//if ((chSaveType != enumSAVE_TYPE_TIMER) && (g_Config.m_chMapMask > 0))
			//{
			//	if (pPlayer->IsMapMaskChange())
			//	{
			//		bSaveMMask = _tab_mmask->SaveData(pPlayer);
			//		pPlayer->ResetMapMaskChange();
			//	}
			//}
			//else
			//	bSaveMMask = true;
			//DWORD	dwSaveMMaskTick = GetTickCount();
			// End

			bSaveBoat = _tab_boat->SaveAllData(pPlayer, chSaveType); // 保存船
			DWORD dwSaveBoatTick = GetTickCount();

			//LG("保存数据耗时", "总计%-8d，主角色%-8d，主角色背包%-8d，银行%-8d，大地图%-8d，船%-8d.[%d %s]\n",
			//LG("save data waste time", "totalize %-8d，main character %-8d，main character kitbag %-8d，bank %-8d，big map %-8d，boat %-8d.[%d %s]\n",
			//	dwSaveBoatTick - dwStartTick, dwSaveMainTick - dwStartTick, dwSaveKbTick - dwSaveMainTick, dwSaveBankTick - dwSaveKbTick, dwSaveMMaskTick - dwSaveBankTick, dwSaveBoatTick - dwSaveMMaskTick,
			//	pPlayer->GetDBChaId(), pPlayer->GetMainCha()->GetLogName());
			LG("save_data_waste_time", "totalize,%d,main character %-8d,main character kitbag %-8d,bank %-8d,boat %-8d([%d %s])\n",
			   dwSaveBoatTick - dwStartTick, dwSaveMainTick - dwStartTick, dwSaveKbTick - dwSaveMainTick, dwSaveBankTick - dwSaveKbTick, dwSaveBoatTick - dwSaveBankTick,
			   pPlayer->GetDBChaId(), pPlayer->GetMainCha()->GetLogName());
		} catch (...) {
			//LG("enter_map", "保存玩家全部数据的过程中发生异常.\n");
			LG("enter_map", "It's abnormity when saving the character's whole data.\n");
		}

		if (!bSaveMainCha || !bSaveBoat || !bSaveKitBag) {
			RollBack();
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}
		CommitTran();

		//LG("enter_map", "保存玩家所有数据成功.\n");
		LG("enter_map", "save character whole data succeed.\n");
		// 记录到文件
		if (chSaveType != enumSAVE_TYPE_TIMER) {
			CKitbag* pCKb;
			CCharacter* pCMainC = pPlayer->GetMainCha();
			short sItemNum = pCMainC->GetKitbag()->GetUseGridNum();
			g_kitbag[0] = '\0';
			//sprintf(g_kitbag, "金钱：%u 道具栏：%d@", pCMainC->getAttr(ATTR_GD), sItemNum);
			//sprintf(g_kitbag, RES_STRING(GM_GAMEDB_CPP_00026), pCMainC->getAttr(ATTR_GD), sItemNum);
			//_snprintf_s(g_kitbag,sizeof(g_kitbag),_TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00026), pCMainC->getAttr(ATTR_GD), sItemNum);
			CFormatParameter param(2);
			param.setLong(0, (int)pCMainC->getAttr(ATTR_GD));
			param.setDouble(1, sItemNum);
			RES_FORMAT_STRING(GM_GAMEDB_CPP_00026, param, g_kitbag);

			SItemGrid* pGridCont;
			CItemRecord* pCItem;
			pCKb = pCMainC->GetKitbag();
			for (short i = 0; i < sItemNum; i++) {
				pGridCont = pCKb->GetGridContByNum(i);
				if (!pGridCont)
					continue;
				pCItem = GetItemRecordInfo(pGridCont->sID);
				if (!pCItem)
					continue;
				//sprintf(g_kitbag + strlen(g_kitbag), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
				_snprintf_s(g_kitbag + strlen(g_kitbag), sizeof(g_kitbag) - strlen(g_kitbag), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			}
			TL(CHA_OUT, pCMainC->GetName(), "", g_kitbag);

			short sItemTmpNum = pCMainC->m_pCKitbagTmp->GetUseGridNum();
			g_kitbagTmp[0] = '\0';
			//sprintf(g_kitbagTmp, "临时背包：%d@", sItemTmpNum);
			//sprintf(g_kitbagTmp, RES_STRING(GM_GAMEDB_CPP_00022), sItemTmpNum);
			_snprintf_s(g_kitbagTmp, sizeof(g_kitbagTmp), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00022), sItemTmpNum);

			pCKb = pCMainC->m_pCKitbagTmp;
			for (short i = 0; i < sItemTmpNum; i++) {
				pGridCont = pCKb->GetGridContByNum(i);
				if (!pGridCont)
					continue;
				pCItem = GetItemRecordInfo(pGridCont->sID);
				if (!pCItem)
					continue;
				//sprintf(g_kitbagTmp + strlen(g_kitbagTmp), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
				_snprintf_s(g_kitbagTmp + strlen(g_kitbagTmp), sizeof(g_kitbagTmp) - strlen(g_kitbagTmp), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			}
			TL(CHA_OUT, pCMainC->GetName(), "", g_kitbagTmp);

			g_equip[0] = '\0';
			//sprintf(g_equip, "装备：%d@", enumEQUIP_NUM);
			//sprintf(g_equip, RES_STRING(GM_GAMEDB_CPP_00024), enumEQUIP_NUM);
			_snprintf_s(g_equip, sizeof(g_equip), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00024), enumEQUIP_NUM);

			for (short i = 0; i < enumEQUIP_NUM; i++) {
				pGridCont = &pCMainC->m_SChaPart.SLink[i];
				if (!pGridCont)
					continue;
				pCItem = GetItemRecordInfo(pGridCont->sID);
				if (!pCItem)
					continue;
				//sprintf(g_equip + strlen(g_equip), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
				_snprintf_s(g_equip + strlen(g_equip), sizeof(g_equip) - strlen(g_equip), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
			}
			TL(CHA_EQUIP, pCMainC->GetName(), "", g_equip);

			char chStart = 0, chEnd = pPlayer->GetCurBankNum() - 1;
			//sprintf(g_kitbag, "银行ID(%d):", pPlayer->GetCurBankNum());
			//sprintf(g_kitbag, RES_STRING(GM_GAMEDB_CPP_00023), pPlayer->GetCurBankNum());
			_snprintf_s(g_kitbag, sizeof(g_kitbag), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00023), pPlayer->GetCurBankNum());

			for (char i = chStart; i <= chEnd; i++) {
				pCKb = pPlayer->GetBank(i);
				sItemNum = pCKb->GetUseGridNum();
				//sprintf(g_kitbag + strlen(g_kitbag), "[%d]%d@;", i + 1, sItemNum);
				_snprintf_s(g_kitbag + strlen(g_kitbag), sizeof(g_kitbag) - strlen(g_kitbag), _TRUNCATE, "[%d]%d@;", i + 1, sItemNum);

				for (short i = 0; i < sItemNum; i++) {
					pGridCont = pCKb->GetGridContByNum(i);
					if (!pGridCont)
						continue;
					pCItem = GetItemRecordInfo(pGridCont->sID);
					if (!pCItem)
						continue;
					//sprintf(g_kitbag + strlen(g_kitbag), "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
					_snprintf_s(g_kitbag + strlen(g_kitbag), sizeof(g_kitbag) - strlen(g_kitbag), _TRUNCATE, "%s[%d],%d;", pCItem->szName, pGridCont->sID, pGridCont->sNum);
				}
			}
			TL(CHA_BANK, pCMainC->GetName(), "", g_kitbag);
		}
		//
	} catch (...) {
	}

#ifdef UPDATE_MUL_THREAD
	pPlayer->DBUnLock();
#endif
	m_Lock.Unlock();
	return true;
}

bool CGameDB::SavePlayerEx(CPlayer* pPlayer, char chSaveType) {
	m_LockEx.Lock();
#ifdef UPDATE_MUL_THREAD
	pPlayer->DBLock();
#endif

	try {
		if (!pPlayer || !pPlayer->GetMainCha()) {
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		if (pPlayer->GetMainCha()->GetPlayer() != pPlayer) {
			//LG("保存角色重大错误", "保存的Player地址 %p[dbid %u]，该玩家的主角 %s，该角色的Player地址 %p）\n",
			LG("ave_character_error", "save Player address %p[dbid %u],the character main player %s,the character 's Player address %p\n",
			   pPlayer, pPlayer->GetDBChaId(), pPlayer->GetMainCha()->GetLogName(), pPlayer->GetMainCha()->GetPlayer());
			//pPlayer->SystemNotice("玩家角色不匹配，保存资料库不成功");
			pPlayer->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00025));

#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		if (!pPlayer->IsValid()) {
			LG("ave_character_error", "save Player address %p[dbid %u],the character main player %s,the character 's Player address %p invalid\n",
			   pPlayer, pPlayer->GetDBChaId(), pPlayer->GetMainCha()->GetLogName(), pPlayer->GetMainCha()->GetPlayer());
#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}

		bool bSaveMainCha = false, bSaveBoat = false, bSaveKitBag = false, bSaveMMask = false, bSaveBank = false;
		bool bSaveKitBagTmp = false;
		bool bSaveKBState = false;
		BeginTran();
		try {
			DWORD dwStartTick = GetTickCount();

			bSaveMainCha = _tab_cha->SaveAllDataEx(pPlayer, chSaveType); // 保存主角色
			DWORD dwSaveMainTick = GetTickCount();
			bSaveKitBag = _tab_res->SaveKitbagDataEx(pPlayer->GetMainCha());
			//保存临时背包
			bSaveKitBagTmp = _tab_res->SaveKitbagTmpDataEx(pPlayer->GetMainCha());
			//保存密码锁定状态
			//bSaveKBState = _tab_cha->SaveKBState(pPlayer);
			DWORD dwSaveKbTick = GetTickCount();
			bSaveBank = _tab_res->SaveBankDataEx(pPlayer);
			DWORD dwSaveBankTick = GetTickCount();
			// Delete by lark.li 20080926 begin
			//if ((chSaveType != enumSAVE_TYPE_TIMER) && (g_Config.m_chMapMask > 0))
			//{
			//	if (pPlayer->IsMapMaskChange())
			//	{
			//		bSaveMMask = _tab_mmask->SaveData(pPlayer);
			//		pPlayer->ResetMapMaskChange();
			//	}
			//}
			//else
			//	bSaveMMask = true;
			//DWORD	dwSaveMMaskTick = GetTickCount();
			// End

			bSaveBoat = _tab_boat->SaveAllDataEx(pPlayer, chSaveType); // 保存船
			DWORD dwSaveBoatTick = GetTickCount();

			//LG("保存数据耗时", "总计%-8d，主角色%-8d，主角色背包%-8d，银行%-8d，大地图%-8d，船%-8d.[%d %s]\n",
			//LG("save data waste time", "totalize %-8d，main character %-8d，main character kitbag %-8d，bank %-8d，big map %-8d，boat %-8d.[%d %s]\n",
			//	dwSaveBoatTick - dwStartTick, dwSaveMainTick - dwStartTick, dwSaveKbTick - dwSaveMainTick, dwSaveBankTick - dwSaveKbTick, dwSaveMMaskTick - dwSaveBankTick, dwSaveBoatTick - dwSaveMMaskTick,
			//	pPlayer->GetDBChaId(), pPlayer->GetMainCha()->GetLogName());
			LG("save_data_waste_time", "totalize,%d,main character %-8d,main character kitbag %-8d,bank %-8d,boat %-8d([%d %s])\n",
			   dwSaveBoatTick - dwStartTick, dwSaveMainTick - dwStartTick, dwSaveKbTick - dwSaveMainTick, dwSaveBankTick - dwSaveKbTick, dwSaveBoatTick - dwSaveBankTick,
			   pPlayer->GetDBChaId(), pPlayer->GetMainCha()->GetLogName());
		} catch (...) {
			//LG("enter_map", "保存玩家全部数据的过程中发生异常.\n");
			LG("enter_map", "It's abnormity when saving the character's whole data.\n");
		}

		if (!bSaveMainCha || !bSaveBoat || !bSaveKitBag) {
			RollBack();

#ifdef UPDATE_MUL_THREAD
			pPlayer->DBUnLock();
#endif
			m_LockEx.Unlock();
			return false;
		}
		CommitTran();

		//LG("enter_map", "保存玩家所有数据成功.\n");
		LG("enter_map", "save character whole data succeed.\n");
	} catch (...) {
	}

#ifdef UPDATE_MUL_THREAD
	pPlayer->DBUnLock();
#endif
	m_LockEx.Unlock();
	return true;
}

bool CGameDB::SaveChaAssets(CCharacter* pCCha) {
	m_Lock.Lock();

	// 因为此操作可能包含数据库回滚操作，所以期间不能throw异常
	try {
		//LG("enter_map", "开始保存角色资产.\n");
		LG("enter_map", "Start save character assets.\n");
		if (!pCCha || !pCCha->GetPlayer()) {
			m_Lock.Unlock();
			return false;
		}

#ifdef UPDATE_MUL_THREAD
		pCCha->GetPlayer()->DBLock();
#endif
		DWORD dwStartTick = GetTickCount();
		if (!_tab_cha->SaveMoney(pCCha->GetPlayer())) {
#ifdef UPDATE_MUL_THREAD
			pCCha->GetPlayer()->DBUnLock();
#endif
			m_Lock.Unlock();
			return false;
		}

		if (!pCCha->IsBoat()) {
			if (!_tab_res->SaveKitbagData(pCCha)) {
#ifdef UPDATE_MUL_THREAD
				pCCha->GetPlayer()->DBUnLock();
#endif
				m_Lock.Unlock();
				return false;
			}
		} else {
			if (!_tab_boat->SaveCabin(*pCCha, enumSAVE_TYPE_TRADE)) {
#ifdef UPDATE_MUL_THREAD
				pCCha->GetPlayer()->DBUnLock();
#endif
				m_Lock.Unlock();
				return false;
			}
		}

		//LG("enter_map", "保存角色 %s(%s) 资产成功.\n", pCCha->GetLogName(), pCCha->GetPlyMainCha()->GetLogName());
		LG("enter_map", "Save character %s(%s)assets succeed.\n", pCCha->GetLogName(), pCCha->GetPlyMainCha()->GetLogName());
		//LG("保存数据耗时", "总计%-8d.[%d %s]\n", GetTickCount() - dwStartTick, pCCha->GetPlayer()->GetDBChaId(), pCCha->GetLogName());
		LG("save_data_waste_time", "totalled,%d,[%d %s]\n", GetTickCount() - dwStartTick, pCCha->GetPlayer()->GetDBChaId(), pCCha->GetLogName());
	} catch (...) {
		//LG("enter_map", "保存角色资产时，发生异常!\n");
		LG("enter_map", "When save character assets occured abnormity\n");

#ifdef UPDATE_MUL_THREAD
		pCCha->GetPlayer()->DBUnLock();
#endif
		m_Lock.Unlock();
		return false;
	}

#ifdef UPDATE_MUL_THREAD
	pCCha->GetPlayer()->DBUnLock();
#endif
	m_Lock.Unlock();
	return true;
}

//===============CTableGuild Begin===========================================
bool CTableGuild::Init() {
	//sprintf(g_sql, "select \
	//			guild_id, guild_name, motto, passwd, leader_id, type, stat, money, exp, member_total,\
	//			try_total, disband_date \
	//			from %s \
	//			(nolock) where 1=2", \
	//			_get_table());
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   guild_id, guild_name, motto, passwd, leader_id, type, stat, money, exp, member_total,\
				   try_total, disband_date \
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	if (strlen(g_sql) >= SQL_MAXLEN) {
		//FILE	*pf = fopen("log\\SQL语句长度越界.txt", "a+");
		//FILE	*pf = fopen("log\\SQLsentence_length_slopover.txt", "a+");
		FILE* pf = nullptr;
		if (fopen_s(&pf, "log\\SQLsentence_length_slopover.txt", "a+") != 0) {
			LG2("error", "Load Raw Data Info Bin File [log\\SQLsentence_length_slopover.txt] Failed!\n");
			return false;
		}
		if (pf) {
			fprintf(pf, "%s\n\n", g_sql);
			fclose(pf);
		}
		//LG("enter_map", "SQL语句长度越界!\n");
		LG("enter_map", "SQL sentence length slop over\n");
		return false;
	}
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		//MessageBox(0, "数据库(guild)初始化错误，请检查", "错误", MB_OK);
		char buffer[255];
		//sprintf(buffer, RES_STRING(GM_GAMEDB_CPP_00001), "guild");
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "guild");

		MessageBox(nullptr, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}

	return true;
}

long CTableGuild::Create(CCharacter* pCha, char guildtype, char* guildname, cChar* passwd) {
	try {
		long l_ret_guild_id = 0;
		std::string buf[1];
		char sql[SQL_MAXLEN];

		while (true) {
			LG("consortia system", "Create.\n");

			//获取空公会的ID
			const char* param = "isnull(min(guild_id),0)";
			char filter[80]; //sprintf(filter, "guild_id >0 and leader_id =0 and type =%d", guildtype);
			_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id >0 and leader_id =0 and type =%d", guildtype);
			bool ret = _get_row(buf, 1, param, filter);
			if (!ret) //发生SQL错误
			{
				//pCha->SystemNotice("公会创建没有成功.");
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00027));
				//LG("公会系统","创建公会发生了SQL操作错误.");
				LG("consortia system", "found consortia system occur SQL operator error.\n");
				return 0;
			}
			l_ret_guild_id = atol(buf[0].c_str());
			if (!l_ret_guild_id) //公会数目已达系统上限
			{
				//pCha->SystemNotice((std::string(guildtype ==0?"海军":"海盗")+"公会数目已达系统上限"));
				pCha->SystemNotice((std::string(guildtype == 0 ? RES_STRING(GM_GAMEDB_CPP_00028) : RES_STRING(GM_GAMEDB_CPP_00029)) + RES_STRING(GM_GAMEDB_CPP_00030)).c_str());
				return 0;
			}
			//更新公会表创建这个公会
			if (guildtype == emGldTypeNavy) //海军
			{
				//sprintf(sql,	"update %s set leader_id =%d ,passwd ='%s',stat =0,money =0,exp =0,\
			//							member_total =1,try_total =0\
			//					where leader_id =0 and guild_id =%d",
				//	_get_table(), pCha->GetID(), passwd, l_ret_guild_id);
				_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set leader_id =%d ,passwd ='%s',stat =0,money =0,exp =0,\
							member_total =1,try_total =0\
							where leader_id =0 and guild_id =%d",
							_get_table(), pCha->GetID(), passwd, l_ret_guild_id);

				SQLRETURN l_sqlret = exec_sql_direct(sql);
				if (!DBOK(l_sqlret)) //普通SQL错误
				{
					//pCha->SystemNotice("公会创建没有成功.");
					pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00027));
					//LG("公会系统","创建公会发生了SQL操作错误.");
					LG("consortia system", "found consortia system occur SQL operator error.\n");
					return 0;
				}
				if (get_affected_rows() != 1) {
					continue;
				}

				param = "guild_name";
				//sprintf(filter, "guild_id  =%d", l_ret_guild_id);
				_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id  =%d", l_ret_guild_id);

				ret = _get_row(buf, 1, param, filter);
				//TODO
				//strcpy(guildname,buf[0].c_str());
				strncpy_s(guildname, 32, buf[0].c_str(), _TRUNCATE);
			} else if (guildtype == emGldTypePirate) //海盗
			{
				//sprintf(sql,	"update %s set leader_id =%d ,passwd ='%s', guild_name ='%s', stat =0,money =0,exp =0,\
			//							member_total =1,try_total =0\
			//					where leader_id =0 and guild_id =%d",
				//	_get_table(), pCha->GetID(), passwd, guildname, l_ret_guild_id);
				_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update %s set leader_id =%d ,passwd ='%s', guild_name ='%s', stat =0,money =0,exp =0,\
							member_total =1,try_total =0\
							where leader_id =0 and guild_id =%d",
							_get_table(), pCha->GetID(), passwd, guildname, l_ret_guild_id);

				SQLRETURN l_sqlret = exec_sql_direct(sql);
				if (!DBOK(l_sqlret)) //海盗公会名字重复
				{
					//pCha->SystemNotice("海盗公会名字已被使用");
					pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00031));
					return 0;
				}
				if (get_affected_rows() != 1) {
					continue;
				}
			} else {
				//pCha->SystemNotice("公会创建没有成功.");
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00027));
				//LG("公会系统","公会类型参数错误.");
				LG("consortia system", "constortia system type parameter error\n");
				return 0; //公会类型错误
			}
			break;
		}
		//sprintf(sql,"update character set guild_id =%d,guild_stat =0,guild_permission =-1 \
	//			where cha_id =%d",l_ret_guild_id ,pCha->GetID());
		_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =%d,guild_stat =0,guild_permission =-1 \
				where cha_id =%d",
					l_ret_guild_id, pCha->GetID());

		exec_sql_direct(sql);

		WPACKET wpk = GETWPACKET();
		WRITE_CMD(wpk, CMD_MP_GUILD_CREATE);
		WRITE_LONG(wpk, l_ret_guild_id);								  //公会ID
		WRITE_STRING(wpk, guildname);									  //公会Name
		WRITE_CHAR(wpk, guildtype);										  //公会类型
		WRITE_STRING(wpk, g_GetJobName(uShort(pCha->getAttr(ATTR_JOB)))); //职业
		WRITE_SHORT(wpk, uShort(pCha->getAttr(ATTR_LV)));				  //等级
		pCha->ReflectINFof(pCha, wpk);

		return l_ret_guild_id; //公会创建成功,返回公会ID
	}
	T_E
}
/*
海军
select gld.guild_id, gld.guild_name, gld.motto, gld.leader_id, cha.cha_name leader_name, gld.type, gld.stat, gld.money, gld.exp, gld.member_total, gld.try_total, gld.disband_date, DATEDIFF(mi,gld.disband_date,GETDATE()) 解散考察累计分钟, 7*24*60 -DATEDIFF(mi,gld.disband_date,GETDATE()) 解散考察剩余分钟
from guild As gld, character As cha
where (gld.type =0 and gld.leader_id =cha.cha_id)
union
select gld.guild_id, gld.guild_name, gld.motto, gld.leader_id, '[无]' leader_name, gld.type, gld.stat, gld.money, gld.exp, gld.member_total, gld.try_total, gld.disband_date, DATEDIFF(mi,gld.disband_date,GETDATE()) 解散考察累计分钟, 7*24*60 -DATEDIFF(mi,gld.disband_date,GETDATE()) 解散考察剩余分钟
from guild As gld
where (gld.type =0 and gld.leader_id =0)
海盗
select gld.guild_id, gld.guild_name, gld.motto, gld.leader_id, cha.cha_name leader_name, gld.type, gld.stat, gld.money, gld.exp, gld.member_total, gld.try_total, gld.disband_date, DATEDIFF(mi,gld.disband_date,GETDATE()) 解散考察累计分钟, 7*24*60 -DATEDIFF(mi,gld.disband_date,GETDATE()) 解散考察剩余分钟
from guild As gld, character As cha
where (gld.type =1 and gld.leader_id =cha.cha_id)
*/
bool CTableGuild::ListAll(CCharacter* pCha, char guildtype, char disband_days) {
	const char* sql_syntax = nullptr;
	if (!pCha || disband_days < 1) {
		return false;
	} else if (guildtype == emGldTypeNavy) {
		sql_syntax =
			"	select gld.guild_id, gld.guild_name, gld.motto, gld.leader_id, cha.cha_name leader_name,gld.type,\
						gld.stat, gld.money, gld.exp, gld.member_total, gld.try_total,gld.disband_date,\
						case when gld.stat>0 then DATEDIFF(mi,gld.disband_date,GETDATE()) else 0 end disband_time1,\
						case when gld.stat>0 then %d*24*60 -DATEDIFF(mi,gld.disband_date,GETDATE()) else 0 end disband_time2\
					from guild As gld, character As cha\
					where (gld.type =0 and gld.leader_id =cha.cha_id)\
				union\
				select gld.guild_id, gld.guild_name, gld.motto, gld.leader_id, '[None]' leader_name,gld.type,\
						gld.stat, gld.money, gld.exp, gld.member_total, gld.try_total, gld.disband_date,\
						0 disband_time1,\
						0 disband_time2\
					from guild As gld\
					where (gld.type =0 and gld.leader_id =0)\
			";
	} else if (guildtype == emGldTypePirate) {
		sql_syntax =
			"	select gld.guild_id, gld.guild_name, gld.motto, gld.leader_id, cha.cha_name leader_name,gld.type,\
						gld.stat, gld.money, gld.exp, gld.member_total, gld.try_total,gld.disband_date,\
						case when gld.stat>0 then DATEDIFF(mi,gld.disband_date,GETDATE()) else 0 end disband_time1,\
						case when gld.stat>0 then %d*24*60 -DATEDIFF(mi,gld.disband_date,GETDATE()) else 0 end disband_time2\
					from guild As gld, character As cha\
					where (gld.type =1 and gld.leader_id =cha.cha_id)\
			";
	} else {
		return false;
	}

	bool ret = false;
	char sql[SQL_MAXLEN];
	//sprintf(sql, sql_syntax, disband_days);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, sql_syntax, disband_days);

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);

			throw 1;
		}

		sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
		if (sqlret != SQL_SUCCESS) {
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);

			if (sqlret != SQL_SUCCESS_WITH_INFO)
				throw 2;
		}

		sqlret = SQLNumResultCols(hstmt, &col_num);
		col_num = min(col_num, MAX_COL);
		col_num = min(col_num, _max_col);

		// Bind Column
		for (int i = 0; i < col_num; ++i) {
			SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
		}

		WPACKET wpk, wpk0 = GETWPACKET();
		WRITE_CMD(wpk0, CMD_MC_LISTGUILD);
		WRITE_CHAR(wpk0, guildtype); //公会类别，0－海军，1－海盗

		// Fetch each Row	int i; // 取出的行数
		int f_row = 1;
		for (f_row = 1; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++f_row) {
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			}
			if ((f_row % 20) == 1) {
				wpk = wpk0;
			}
			WRITE_LONG(wpk, atol((char const*)_buf[0]));  //公会ID
			WRITE_STRING(wpk, (char const*)_buf[1]);	  //公会名
			WRITE_STRING(wpk, (char const*)_buf[2]);	  //公会座右铭
			WRITE_STRING(wpk, (char const*)_buf[4]);	  //会长名
			WRITE_SHORT(wpk, atoi((const char*)_buf[9])); //成员数
			LLong l_exp = _atoi64((char const*)_buf[8]);
			WRITE_LONG(wpk, uLong(l_exp % 0x100000000)); //经验值低32位
			WRITE_LONG(wpk, uLong(l_exp / 0x100000000)); //经验值高32位

			if (!(f_row % 20)) {
				WRITE_CHAR(wpk, ((f_row - 1) % 20) + 1); //本次包括的条数
				pCha->ReflectINFof(pCha, wpk);
			}
		}
		if ((f_row % 20) == 1) {
			wpk = wpk0;
		}
		WRITE_LONG(wpk, (f_row - 1) % 20); //本次包括的条数
		pCha->ReflectINFof(pCha, wpk);

		SQLFreeStmt(hstmt, SQL_UNBIND);
		ret = true;
	} catch (int& e) {
		//LG("公会系统", "创建公会过程ODBC 接口调用错误，位置码：%d\n",e);
		LG("consortia system", "found consortia process ODBC interfance transfer error,position ID:%d\n", e);
	} catch (...) {
		//LG("公会系统", "Unknown Exception raised when list all guilds\n");
		LG("consortia system", "Unknown Exception raised when list all guilds\n");
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}
void CTableGuild::TryFor(CCharacter* pCha, uLong guildid) {
	if (pCha->HasGuild()) {
		//pCha->SystemNotice( "您已经是公会《%s》成员,不能重复申请加入公会!", pCha->GetGuildName() );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00032), pCha->GetGuildName());
		return;
	} else if (guildid == pCha->GetGuildID()) {
		//pCha->SystemNotice( "不能重复申请加入公会《%s》!", pCha->GetGuildName() );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00033), pCha->GetGuildName());
		return;
	}

	std::string buf[3];
	char filter[80];
	const char* param = "guild_id";
	//sprintf(filter,"leader_id >0 and guild_id =%d",guildid);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "leader_id >0 and guild_id =%d", guildid);
	int l_ret = _get_row(buf, 3, param, filter);
	if (!DBOK(l_ret)) {
		//pCha->SystemNotice("申请加入公会失败.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00034));
		//LG("公会系统","用户[%s]申请加入公会[ID=%d]时候SQL执行失败.\n",pCha->GetName(),guildid);
		LG("consortia system", "character[%s]apply join in consortia [ID=%d] carry out SQL failed.\n", pCha->GetName(), guildid);
		return;
	} else if (get_affected_rows() != 1) {
		//pCha->SystemNotice("申请加入的公会无会长.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00035));
		return;
	}
	param = "c.guild_id, c.guild_stat, g.guild_name";
	std::string l_tbl_name = _tbl_name;
	_tbl_name = "character c,guild g";
	//sprintf(filter, "c.guild_id =g.guild_id and c.cha_id =%d and g.guild_id <>%d",pCha->GetID(), guildid);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "c.guild_id =g.guild_id and c.cha_id =%d and g.guild_id <>%d", pCha->GetID(), guildid);

	l_ret = _get_row(buf, 3, param, filter);
	_tbl_name = l_tbl_name;
	if (!DBOK(l_ret) || get_affected_rows() != 1) {
		//pCha->SystemNotice("申请加入公会失败.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00034));
		//LG("公会系统","用户[%s]申请加入公会[ID=%d]时候SQL执行失败.\n",pCha->GetName(),guildid);
		LG("consortia system", "character[%s]apply join in consortia [ID=%d] carry out SQL failed.\n", pCha->GetName(), guildid);
		return;
	}

	// 查询新申请公会信息
	std::string bufnew[3];
	param = "type, guild_name";
	//sprintf(filter, "guild_id =%d",guildid);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", guildid);

	int l_retrow = 0;
	l_ret = _get_row(bufnew, 3, param, filter, &l_retrow);
	if (l_retrow == 1) {
	} else {
		//LG( "公会系统", "TryFor：角色%s申请公会ID[0x%X]不存在!", pCha->GetName(), guildid );
		LG("consortia system", "TryFor: character %s apply consortia ID[0x%X]is inexistence!", pCha->GetName(), guildid);
		//pCha->SystemNotice( "您申请的公会不存在!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00036));
		return;
	}

	// 临时保存查询到的公会名称
	//strncpy( pCha->GetPlayer()->m_szTempGuildName, bufnew[1].c_str(), defGUILD_NAME_LEN - 1 );
	strncpy_s(pCha->GetPlayer()->m_szTempGuildName, sizeof(pCha->GetPlayer()->m_szTempGuildName), bufnew[1].c_str(), _TRUNCATE);

	if (atoi(buf[0].c_str())) {
		if (atoi(buf[1].c_str()) == 0) {
			//pCha->SystemNotice("你当前是公会[%s]的正式成员,不能申请加入别的公会.",buf[2].c_str());
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00037), buf[2].c_str());
			return;
		} else if (atoi(buf[1].c_str()) == 1 && pCha->GetPlayer()->m_GuildState.IsFalse(emGuildReplaceOldTry)) {
			pCha->GetPlayer()->m_GuildState.SetBit(emGuildReplaceOldTry);
			pCha->GetPlayer()->m_lTempGuildID = guildid;
			WPACKET wpk = GETWPACKET();
			WRITE_CMD(wpk, CMD_MC_GUILD_TRYFORCFM);
			WRITE_STRING(wpk, buf[2].c_str());
			pCha->ReflectINFof(pCha, wpk);
			return;
		}
	} else {
		TryForConfirm(pCha, guildid);
	}
}
void CTableGuild::TryForConfirm(CCharacter* pCha, uLong guildid) {
	char sql[SQL_MAXLEN];

	if (pCha->HasGuild()) {
		//pCha->SystemNotice( "您已经是公会《%s》成员,不能重复申请加入公会!", pCha->GetGuildName() );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00038), pCha->GetGuildName());
		return;
	}

	DWORD dwOldGuildID = pCha->GetGuildID();

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "申请公会数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00039));
		return;
	}

	//sprintf(sql,	"update character set guild_id =%d ,guild_stat =1,guild_permission =0\
	//					where cha_id =%d and\
	//							%d in (select guild_id from guild where leader_id >0 and guild_id =%d and try_total <%d and member_total <%d)",
	//		 guildid, pCha->GetID(), guildid, guildid,emMaxTryMemberNum,emMaxMemberNum);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =%d ,guild_stat =1,guild_permission =0\
					where cha_id =%d and\
					%d in (select guild_id from guild where leader_id >0 and guild_id =%d and try_total <%d and member_total <%d)",
				guildid, pCha->GetID(), guildid, guildid, emMaxTryMemberNum, emMaxMemberNum);

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "申请加入公会操作失败，可能该公会申请人数已满，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00040));
		return;
	}

	//sprintf(sql,	"update guild set try_total =try_total +1 where guild_id =%d",guildid);
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set try_total =try_total +1 where guild_id =%d", guildid);

	l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "申请加入公会操作失败，可能该公会申请人数已满，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00040));
		return;
	}

	// 判断是否是客户端更换申请公会
	if (dwOldGuildID && pCha->GetPlayer()->m_GuildState.IsTrue(emGuildReplaceOldTry)) {
		//sprintf(sql,"update guild set try_total =try_total -1 where guild_id =%d and try_total > 0"
		//	,dwOldGuildID);
		_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set try_total =try_total -1 where guild_id =%d and try_total > 0", dwOldGuildID);

		SQLRETURN l_sqlret = exec_sql_direct(sql);
		if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
			this->rollback();
			//pCha->SystemNotice( "申请退出原有公会操作失败，请稍后再试!" );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00041));
			return;
		}
	}

	if (!commit_tran()) {
		this->rollback();
		//pCha->SystemNotice( "申请加入公会操作失败，可能该公会申请人数已满，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00040));
		return;
	}

	// 设置新公会申请信息
	pCha->SetGuildID(guildid);
	pCha->SetGuildState(emGldMembStatTry);

	pCha->SetGuildName(pCha->GetPlayer()->m_szTempGuildName);
	//pCha->SystemNotice( "恭喜!你申请加入公会《%s》成功,请耐心等待公会管理员的批复.", pCha->GetGuildName() );
	pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00042), pCha->GetGuildName());
}
bool CTableGuild::GetGuildInfo(CCharacter* pCha, uLong guildid) {
	std::string buf[3];
	char filter[80];

	const char* param = "type, guild_name, motto";
	//sprintf(filter, "guild_id =%d",guildid);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", guildid);
	int l_retrow = 0;
	bool l_ret = _get_row(buf, 3, param, filter, &l_retrow);
	if (l_retrow == 1) {
		pCha->SetGuildType(atoi(buf[0].c_str()));
		pCha->SetGuildName(buf[1].c_str());
		pCha->SetGuildMotto(buf[2].c_str());
		return true;
	} else {
		return false;
	}
}
char CTableGuild::GetTypeByID(uLong guildid) {
	std::string buf[3];
	char filter[80];

	const char* param = "type, guild_name, motto";
	//sprintf(filter, "guild_id =%d",guildid);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", guildid);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 3, param, filter, &l_retrow);
	if (l_retrow == 1) {
		return atoi(buf[0].c_str());
	} else {
		return -1;
	}
}
bool CTableGuild::ListTryPlayer(CCharacter* pCha, char disband_days) {
	bool ret = false;

	if (!pCha || !pCha->HasGuild()) {
		return ret;
	}

	std::string buf[10];
	char filter[80];

	const char* sql_syntax =
		"g.guild_id, g.guild_name,g.motto, g.type, g.stat, c.cha_name, g.member_total,g.exp,g.money,\
				case when g.stat>0 then %d*24*60 -DATEDIFF(mi,g.disband_date,GETDATE()) else 0 end disband_time1\
			";
	char param[500];
	//sprintf(param , sql_syntax , disband_days);
	_snprintf_s(param, sizeof(param), _TRUNCATE, sql_syntax, disband_days);

	std::string l_tbl_name = _tbl_name;
	_tbl_name = "character c,guild g";
	//sprintf(filter, "g.leader_id =c.cha_id and g.guild_id =%d",pCha->GetGuildID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "g.leader_id =c.cha_id and g.guild_id =%d", pCha->GetGuildID());

	int l_retrow = 0;

	bool l_ret = _get_row(buf, 10, param, filter, &l_retrow);
	_tbl_name = l_tbl_name;

	if (!l_ret || !l_retrow || this->get_affected_rows() != 1) {
		return ret;
	}
	WPACKET wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MC_GUILD_LISTTRYPLAYER);
	WRITE_LONG(wpk, atol(buf[0].c_str()));  //guild_id
	WRITE_STRING(wpk, buf[1].c_str());		//guild_name
	WRITE_STRING(wpk, buf[2].c_str());		//motto
	WRITE_CHAR(wpk, atoi(buf[3].c_str()));  //type
	WRITE_CHAR(wpk, atoi(buf[4].c_str()));  //stat
	WRITE_STRING(wpk, buf[5].c_str());		//cha_name
	WRITE_SHORT(wpk, atoi(buf[6].c_str())); //member_total
	WRITE_SHORT(wpk, emMaxMemberNum);		//最大成员数
	LLong l_money = _atoi64(buf[7].c_str());
	WRITE_LONG(wpk, uLong(l_money % 0x100000000)); //money低位
	WRITE_LONG(wpk, uLong(l_money / 0x100000000)); //moeny高位
	LLong l_exp = _atoi64(buf[8].c_str());
	WRITE_LONG(wpk, uLong(l_exp % 0x100000000)); //exp低位
	WRITE_LONG(wpk, uLong(l_exp / 0x100000000)); //exp高位
	WRITE_LONG(wpk, atol(buf[9].c_str()));		 //倒计时分钟

	sql_syntax =
		"select c.cha_id,c.cha_name,c.job,c.degree from character c\
			where (c.guild_stat =1 and c.guild_id =%d and c.delflag =0)";
	char sql[SQL_MAXLEN];
	//sprintf(sql, sql_syntax, pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, sql_syntax, pCha->GetGuildID());

	// 执行查询操作
	SQLRETURN sqlret;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLSMALLINT col_num = 0;
	bool found = true;

	try {
		sqlret = SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &hstmt);
		if ((sqlret != SQL_SUCCESS) && (sqlret != SQL_SUCCESS_WITH_INFO)) {
			handle_err(_hdbc, SQL_HANDLE_DBC, sqlret);

			throw 1;
		}

		sqlret = SQLExecDirect(hstmt, (SQLCHAR*)sql, SQL_NTS);
		if (sqlret != SQL_SUCCESS) {
			handle_err(hstmt, SQL_HANDLE_STMT, sqlret);

			if (sqlret != SQL_SUCCESS_WITH_INFO)
				throw 2;
		}

		sqlret = SQLNumResultCols(hstmt, &col_num);
		col_num = min(col_num, MAX_COL);
		col_num = min(col_num, _max_col);

		// Bind Column
		for (int i = 0; i < col_num; ++i) {
			SQLBindCol(hstmt, UWORD(i + 1), SQL_C_CHAR, _buf[i], MAX_DATALEN, &_buf_len[i]);
		}

		// Fetch each Row	int i; // 取出的行数
		int f_row = 0;
		for (f_row = 0; (sqlret = SQLFetch(hstmt)) == SQL_SUCCESS || sqlret == SQL_SUCCESS_WITH_INFO; ++f_row) {
			if (sqlret != SQL_SUCCESS) {
				handle_err(hstmt, SQL_HANDLE_STMT, sqlret);
			}

			WRITE_LONG(wpk, atol((char const*)_buf[0])); //ID
			WRITE_STRING(wpk, (char const*)_buf[1]);	 //名称

			//数据库变更修正
			//WRITE_STRING(wpk,(char const *)_buf[2]);		//职业
			WRITE_STRING(wpk, g_GetJobName(atoi((const char*)_buf[2])));

			WRITE_SHORT(wpk, atoi((char const*)_buf[3])); //等级
		}

		WRITE_LONG(wpk, f_row); //本次包括的条数
		pCha->ReflectINFof(pCha, wpk);

		SQLFreeStmt(hstmt, SQL_UNBIND);
		ret = true;
	} catch (int& e) {
		//LG("公会系统", "参考公会申请成员过程ODBC 接口调用错误，位置码：%d\n",e);
		LG("consortia system", "consult apply consortia process memeberODBC interface transfer error,position ID:%d\n", e);
	} catch (...) {
		//LG("公会系统", "Unknown Exception raised when list all guilds\n");
		LG("consortia system", "Unknown Exception raised when list all guilds\n");
	}

	if (hstmt != SQL_NULL_HSTMT) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = SQL_NULL_HSTMT;
	}

	return ret;
}
bool CTableGuild::Approve(CCharacter* pCha, uLong chaid) {
	if (!pCha || !pCha->HasGuild()) {
		return false;
	}

	std::string buf[3];
	char filter[80];

	const char* param = "c.cha_id";
	std::string l_tbl_name = _tbl_name;
	_tbl_name = "character c";
	//sprintf(filter, "c.cha_id =%d and c.guild_id =%d and c.guild_permission & %d =%d",pCha->GetID(),pCha->GetGuildID(),emGldPermMgr,emGldPermMgr);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "c.cha_id =%d and c.guild_id =%d and c.guild_permission & %d =%d", pCha->GetID(), pCha->GetGuildID(), emGldPermMgr, emGldPermMgr);

	int retrow = 0;
	bool l_ret = _get_row(buf, 3, param, filter, &retrow);
	_tbl_name = l_tbl_name;
	if (!l_ret) {
		//pCha->SystemNotice("批准公会申请操作失败.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00043));
		return false;
	}
	if (!retrow) {
		//pCha->SystemNotice("你没有公会的管理权限");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00044));
		return false;
	}

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "批准公会数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00045));
		return false;
	}

	char sql[SQL_MAXLEN];
	//sprintf(sql,"update guild\
	//				set try_total =try_total -1,\
	//					member_total =member_total +1\
	//					where guild_id =%d and member_total <%d and try_total > 0"
	//				,pCha->GetGuildID(),emMaxMemberNum);

	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild\
				set try_total =try_total -1,\
				member_total =member_total +1\
				where guild_id =%d and member_total <%d and try_total > 0",
				pCha->GetGuildID(), emMaxMemberNum);
	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "批准公会操作失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00046));
		return false;
	}

	//sprintf(sql,	"update character set guild_stat =0,guild_permission =0\
	//					where cha_id =%d and guild_id =%d and guild_stat =1 and delflag =0",
	//					chaid, pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_stat =0,guild_permission =0\
					where cha_id =%d and guild_id =%d and guild_stat =1 and delflag =0",
				chaid, pCha->GetGuildID());

	l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "批准公会操作失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00046));
		return false;
	}

	if (!commit_tran()) {
		this->rollback();
		//pCha->SystemNotice( "批准公会操作失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00046));
		return false;
	}

	WPACKET wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MM_GUILD_APPROVE);
	WRITE_LONG(wpk, chaid);
	WRITE_LONG(wpk, pCha->GetGuildID());
	WRITE_CHAR(wpk, pCha->GetGuildType());
	WRITE_STRING(wpk, pCha->GetValidGuildName());
	WRITE_STRING(wpk, pCha->GetValidGuildMotto());
	pCha->ReflectINFof(pCha, wpk);

	wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MP_GUILD_APPROVE);
	WRITE_LONG(wpk, chaid);
	pCha->ReflectINFof(pCha, wpk);
	return true;
}
bool CTableGuild::Reject(CCharacter* pCha, uLong chaid) {
	if (!pCha || !pCha->HasGuild()) {
		return false;
	}

	std::string buf[3];
	char filter[80];

	const char* param = "c.cha_id";
	std::string l_tbl_name = _tbl_name;
	_tbl_name = "character c";
	//sprintf(filter, "c.cha_id =%d and c.guild_id =%d and c.guild_permission & %d =%d",pCha->GetID(),pCha->GetGuildID(),emGldPermMgr,emGldPermMgr);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "c.cha_id =%d and c.guild_id =%d and c.guild_permission & %d =%d", pCha->GetID(), pCha->GetGuildID(), emGldPermMgr, emGldPermMgr);

	int retrow = 0;
	bool l_ret = _get_row(buf, 3, param, filter, &retrow);
	_tbl_name = l_tbl_name;
	if (!l_ret) {
		//pCha->SystemNotice("拒绝公会申请操作失败.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00047));
		return false;
	}
	if (!retrow) {
		//pCha->SystemNotice("你没有公会的管理权限.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00048));
		return false;
	}

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "批准公会数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00045));
		return false;
	}

	char sql[SQL_MAXLEN];
	//sprintf(sql,	"update character set guild_id =0 ,guild_stat =0,guild_permission =0\
	//					where cha_id =%d and guild_id =%d and guild_stat =1",
	//			chaid, pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =0 ,guild_stat =0,guild_permission =0\
					where cha_id =%d and guild_id =%d and guild_stat =1",
				chaid, pCha->GetGuildID());

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "解除角色加入公会申请失败!请确认该角色申请是否已被取消!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00049));
		return false;
	}

	//sprintf(sql,"update guild set try_total =try_total -1 where guild_id =%d and try_total > 0"
	//	,pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set try_total =try_total -1 where guild_id =%d and try_total > 0", pCha->GetGuildID());

	l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "解除角色加入公会申请失败!请确认该角色申请是否已被取消!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00049));
		return false;
	}

	if (!commit_tran()) {
		this->rollback();
		//pCha->SystemNotice( "解除角色加入公会申请失败!请确认该角色申请是否已被取消!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00049));
		return false;
	}

	WPACKET wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MM_GUILD_REJECT);
	WRITE_LONG(wpk, chaid);
	WRITE_STRING(wpk, pCha->GetGuildName());
	pCha->ReflectINFof(pCha, wpk);
	return true;
}
bool CTableGuild::Kick(CCharacter* pCha, uLong chaid) {
	if (!pCha || !pCha->HasGuild()) {
		return false;
	}

	std::string buf[3];
	char filter[80];

	const char* param = "c.cha_id";
	std::string l_tbl_name = _tbl_name;
	_tbl_name = "character c";
	//sprintf(filter, "c.cha_id =%d and c.guild_id =%d and c.guild_permission & %d =%d",pCha->GetID(),pCha->GetGuildID(),emGldPermMgr,emGldPermMgr);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "c.cha_id =%d and c.guild_id =%d and c.guild_permission & %d =%d", pCha->GetID(), pCha->GetGuildID(), emGldPermMgr, emGldPermMgr);

	int retrow = 0;
	bool l_ret = _get_row(buf, 3, param, filter, &retrow);
	_tbl_name = l_tbl_name;
	if (!l_ret) {
		//pCha->SystemNotice("踢人操作失败.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00050));
		return false;
	}
	if (!retrow) {
		//pCha->SystemNotice("你没有公会的管理权限.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00048));
		return false;
	}

	if (chaid == pCha->GetID()) {
		//pCha->SystemNotice( "你是会长不可以踢出自己!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00051));
		return false;
	}

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "解除公会成员身份数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00052));
		return false;
	}

	char sql[SQL_MAXLEN];
	//sprintf(sql,	"update character set guild_id =0 ,guild_stat =0,guild_permission =0\
	//					where cha_id =%d and guild_id =%d and guild_stat =0 and\
	//							cha_id not in(select leader_id from guild where guild_id =%d)",
	//			chaid, pCha->GetGuildID(), pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =0 ,guild_stat =0,guild_permission =0\
					where cha_id =%d and guild_id =%d and guild_stat =0 and\
					cha_id not in(select leader_id from guild where guild_id =%d)",
				chaid, pCha->GetGuildID(), pCha->GetGuildID());

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "解除角色公会成员身份失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00053));
		return false;
	}

	//sprintf(sql,	"update guild set member_total =member_total -1 where guild_id =%d",pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set member_total =member_total -1 where guild_id =%d", pCha->GetGuildID());

	l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "解除角色公会成员身份失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00053));
		return false;
	}

	if (!commit_tran()) {
		this->rollback();
		//pCha->SystemNotice( "解除角色公会成员身份失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00053));
		return false;
	}

	WPACKET wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MM_GUILD_KICK);
	WRITE_LONG(wpk, chaid);
	WRITE_STRING(wpk, pCha->GetGuildName());
	pCha->ReflectINFof(pCha, wpk);

	wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MP_GUILD_KICK);
	WRITE_LONG(wpk, chaid);
	pCha->ReflectINFof(pCha, wpk);

	wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MC_GUILD_KICK);
	pCha->ReflectINFof(pCha, wpk);

	return true;
}
bool CTableGuild::Leave(CCharacter* pCha) {
	if (!pCha || !pCha->HasGuild()) {
		return false;
	}

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "离开公会数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00054));
		return false;
	}

	char sql[SQL_MAXLEN];
	//sprintf(sql,	"update character set guild_id =0 ,guild_stat =0,guild_permission =0\
	//					where cha_id =%d and guild_id =%d and guild_stat =0 and\
	//							cha_id not in(select leader_id from guild where guild_id =%d)",
	//			pCha->GetID(), pCha->GetGuildID(), pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =0 ,guild_stat =0,guild_permission =0\
					where cha_id =%d and guild_id =%d and guild_stat =0 and\
					cha_id not in(select leader_id from guild where guild_id =%d)",
				pCha->GetID(), pCha->GetGuildID(), pCha->GetGuildID());

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "离开当前公会失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00055));
		return false;
	}

	//sprintf(sql,	"update guild set member_total =member_total -1 where guild_id =%d",pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set member_total =member_total -1 where guild_id =%d", pCha->GetGuildID());

	l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "离开当前公会失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00055));
		return false;
	}

	if (!commit_tran()) {
		this->rollback();
		//pCha->SystemNotice( "离开当前公会失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00055));
		return false;
	}

	pCha->SetGuildID(0);
	pCha->SetGuildType(0);
	pCha->SetGuildName("");
	pCha->SetGuildMotto("");
	pCha->SyncGuildInfo();
	//pCha->SystemNotice("已经离开公会!");
	pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00056));

	WPacket wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MP_GUILD_LEAVE);
	pCha->ReflectINFof(pCha, wpk);

	wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MC_GUILD_LEAVE);
	pCha->ReflectINFof(pCha, wpk);
	return true;
}
bool CTableGuild::Disband(CCharacter* pCha, cChar* passwd) {
	if (!pCha || !pCha->HasGuild()) {
		return false;
	}

	std::string buf[6];
	char filter[80];
	const char* param = "challlevel";
	//sprintf(filter, "guild_id =%d", pCha->GetValidGuildID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", pCha->GetValidGuildID());

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 6, param, filter, &l_retrow);
	if (l_retrow == 1) {
		if (atoi(buf[0].c_str()) > 0) {
			//pCha->SystemNotice( "你是公会挑战擂主之一不可以解散!" );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00057));
			return false;
		} else {
			l_retrow = 0;
			//sprintf(filter, "challid =%d", pCha->GetValidGuildID());
			_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challid =%d", pCha->GetValidGuildID());

			bool l_ret = _get_row(buf, 6, param, filter, &l_retrow);
			if (!l_ret) {
				//pCha->SystemNotice( "获取你的公会挑战信息失败!" );
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00058));
				return false;
			}
			if (l_retrow >= 1) {
				//pCha->SystemNotice( "你参与了公会挑战不可以解散!" );
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00059));
				return false;
			}
		}
	} else {
		//pCha->SystemNotice( "获取你的公会挑战排名信息失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00060));
		return false;
	}

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "解散公会数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00061));
		return false;
	}

	char sql[SQL_MAXLEN];
	//sprintf(sql,	"update guild set motto ='',passwd ='',leader_id =0,stat =0,money =0,exp =0,member_total =0,try_total =0\
	//					where guild_id =%d and passwd ='%s' and leader_id =%d",
	//					pCha->GetGuildID(), passwd, pCha->GetPlayer()->GetDBChaId());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set motto ='',passwd ='',leader_id =0,stat =0,money =0,exp =0,member_total =0,try_total =0\
					where guild_id =%d and passwd ='%s' and leader_id =%d",
				pCha->GetGuildID(), passwd, pCha->GetPlayer()->GetDBChaId());

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		pCha->SystemNotice("您的密码不对，不能解散公会！");
		//pCha->SystemNotice( RES_STRING(GM_GAMEDB_CPP_00062) );
		return false;
	}

	//sprintf(sql,	"update character set guild_id =0 ,guild_stat =0,guild_permission =0\
	//					where guild_id =%d",
	//			pCha->GetGuildID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =0 ,guild_stat =0,guild_permission =0\
					where guild_id =%d",
				pCha->GetGuildID());

	l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "解散公会操作失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00062));
		return false;
	}

	if (!commit_tran()) {
		this->rollback();
		//pCha->SystemNotice( "解散公会操作失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00062));
		return false;
	}

	WPacket wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MP_GUILD_DISBAND);
	pCha->ReflectINFof(pCha, wpk);

	wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MM_GUILD_DISBAND);
	WRITE_LONG(wpk, pCha->GetGuildID());
	pCha->ReflectINFof(pCha, wpk);

	return true;
}
bool CTableGuild::Motto(CCharacter* pCha, cChar* motto) {
	if (!pCha || !pCha->HasGuild()) {
		return false;
	}

	char sql[SQL_MAXLEN];
	//sprintf(sql,	"update guild set motto ='%s'\
	//					where guild_id =%d and leader_id =%d",
	//			motto,pCha->GetGuildID(), pCha->GetID());
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set motto ='%s'\
					where guild_id =%d and leader_id =%d",
				motto, pCha->GetGuildID(), pCha->GetID());

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret)) {
		//pCha->SystemNotice("修改公会座右铭操作失败.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00063));
		return false; //普通SQL错误
	}
	if (get_affected_rows() != 1) {
		//pCha->SystemNotice("只有会长才能修改公会座右铭.");
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00064));
		return false;
	}

	WPacket wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MM_GUILD_MOTTO);
	WRITE_LONG(wpk, pCha->GetGuildID());
	WRITE_STRING(wpk, motto);
	pCha->ReflectINFof(pCha, wpk);

	wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MP_GUILD_MOTTO);
	WRITE_STRING(wpk, motto);
	pCha->ReflectINFof(pCha, wpk);

	return true;
}

bool CTableGuild::CancelTryFor(CCharacter* pCha) {
	char sql[SQL_MAXLEN];

	if (!pCha) {
		return false;
	}
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update character set guild_id =0 ,guild_stat =0,guild_permission =0\
					where cha_id =%d ",
				pCha->GetID());

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		return false;
	}
	return true;
}
bool CTableGuild::GetGuildName(long lGuildID, std::string& strGuildName) {
	char filter[80];

	const char* param = "guild_name";
	//sprintf(filter, "guild_id =%d",lGuildID);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", lGuildID);

	int l_retrow = 0;
	return _get_row(&strGuildName, 1, param, filter, &l_retrow);
}

bool CTableGuild::Leizhu(CCharacter* pCha, BYTE byLevel, DWORD dwMoney) {
	if (!pCha || !pCha->HasGuild() || byLevel < 1 || byLevel > 3) {
		return false;
	}

	if (dwMoney == 0) {
		//pCha->SystemNotice( "抢公会擂主投标金额必须大于0￥!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00065));
		return false;
	}

	std::string buf[6];
	char filter[80];
	const char* param1 = "guild_id, guild_name, challid, challmoney, leader_id, challstart";
	if (pCha->GetValidGuildID() > 0) {
		//sprintf(filter, "guild_id =%d", pCha->GetValidGuildID());
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", pCha->GetValidGuildID());

		int l_retrow = 0;
		bool l_ret = _get_row(buf, 6, param1, filter, &l_retrow);
		if (l_retrow == 1) {
			if (pCha->GetID() == atoi(buf[4].c_str())) {
				// 是公会会长查询
			} else {
				return false;
			}
		} else {
			//pCha->SystemNotice( "查询您的公会信息失败!请稍后再试!" );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00066));
			return false;
		}

		//sprintf(filter, "challid =%d", pCha->GetValidGuildID());
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challid =%d", pCha->GetValidGuildID());

		l_ret = _get_row(buf, 6, param1, filter, &l_retrow);
		if (l_retrow >= 1) {
			//pCha->SystemNotice( "您每次只能挑战一个公会，您已经是公会《%s》的挑战者^_^!", buf[1].c_str() );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00067), buf[1].c_str());
			return false;
		}
	} else {
		return false;
	}

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "挑战公会数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00068));
		return false;
	}

	char sql[SQL_MAXLEN];
	char szGuild[64];
	memset(szGuild, 0, 64);
	DWORD dwGuildID = 0;
	DWORD dwChallID = 0;
	DWORD dwChallMoney = 0;
	const char* param = "guild_id, guild_name, challid, challmoney";
	//sprintf(filter, "challlevel =%d",byLevel);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challlevel =%d", byLevel);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 4, param, filter, &l_retrow);
	if (l_retrow == 1) {
		//pCha->SystemNotice( "公会《%s》已经是排名第%d擂主了!", buf[1].c_str(), byLevel );
		//pCha->SystemNotice( RES_STRING(GM_GAMEDB_CPP_00069), buf[1].c_str(), byLevel );
		char szData[128];
		CFormatParameter param(2);
		param.setString(0, buf[1].c_str());
		param.setDouble(1, byLevel);
		RES_FORMAT_STRING(GM_GAMEDB_CPP_00069, param, szData);
		pCha->SystemNotice(szData);
		return false;
	} else {
		const char* param = "challlevel";
		//sprintf(filter, "guild_id =%d",pCha->GetValidGuildID());
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", pCha->GetValidGuildID());

		bool l_ret = _get_row(buf, 4, param, filter, &l_retrow);
		if (l_retrow == 1) {
			if (atoi(buf[0].c_str()) > 0) {
				//pCha->SystemNotice( "您的公会已经是排名第%d的擂主了!", atoi(buf[0].c_str()) );
				pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00070), atoi(buf[0].c_str()));
				return false;
			}
		}

		DWORD dwMoneyArray[3] = {5000000, 3000000, 1000000};
		if (dwMoney < dwMoneyArray[byLevel - 1] || !pCha->HasMoney(dwMoney)) {
			//pCha->SystemNotice( "想获得排名第%d的公会称号需要金额（%uG）!", byLevel, dwMoneyArray[byLevel-1] );
			//pCha->SystemNotice( RES_STRING(GM_GAMEDB_CPP_00071), byLevel, dwMoneyArray[byLevel-1] );
			char szData[128];
			CFormatParameter param(2);
			param.setDouble(0, byLevel);
			param.setLong(1, dwMoneyArray[byLevel - 1]);
			RES_FORMAT_STRING(GM_GAMEDB_CPP_00071, param, szData);
			pCha->SystemNotice(szData);
			return false;
		}

		//sprintf(sql,	"update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
		//	byLevel, pCha->GetValidGuildID() );
		_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
					byLevel, pCha->GetValidGuildID());

		SQLRETURN l_sqlret = exec_sql_direct(sql);
		if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
			this->rollback();
			//LG( "挑战公会", "挑战公会结束，擂主败：更新失败公会战得公会数据操作失败!公会不存在!公会ID = %d.公会排名：%d", pCha->GetValidGuildID(), byLevel );
			LG("challenge consortia", "challenge consortia over,leizhu failed:update lost consortia data operater failed! consortiaID = %d.consortia level:%d", pCha->GetValidGuildID(), byLevel);
			return false;
		}

		if (!commit_tran()) {
			this->rollback();
			//pCha->SystemNotice( "公会数据操作失败，请稍后再试!" );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00072));
			return false;
		}
		//if( pCha->TakeMoney( "系统", dwMoney ) )
		if (pCha->TakeMoney(RES_STRING(GM_GAMEDB_CPP_00073), dwMoney)) {
			//pCha->SystemNotice( "恭喜你的公会《%s》获得了排名第%d公会的称号!", pCha->GetGuildName(), byLevel );
			//pCha->SystemNotice( RES_STRING(GM_GAMEDB_CPP_00074), pCha->GetGuildName(), byLevel );
			char szData[128];
			CFormatParameter param(2);
			param.setString(0, pCha->GetGuildName());
			param.setDouble(1, byLevel);
			RES_FORMAT_STRING(GM_GAMEDB_CPP_00074, param, szData);
			pCha->SystemNotice(szData);
		}
		this->ListChallenge(pCha);
	}
	return true;
}

bool CTableGuild::Challenge(CCharacter* pCha, BYTE byLevel, DWORD dwMoney) {
	if (!pCha || !pCha->HasGuild() || byLevel < 1 || byLevel > 3) {
		return false;
	}

	if (dwMoney == 0) {
		//pCha->SystemNotice( "挑战公会投标金额必须大于0￥!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00075));
		return false;
	}

	std::string buf[6];
	char filter[80];
	const char* param1 = "guild_id, guild_name, challid, challmoney, leader_id, challstart";
	if (pCha->GetValidGuildID() > 0) {
		//sprintf(filter, "guild_id =%d", pCha->GetValidGuildID());
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", pCha->GetValidGuildID());

		int l_retrow = 0;
		bool l_ret = _get_row(buf, 6, param1, filter, &l_retrow);
		if (l_retrow == 1) {
			if (pCha->GetID() == atoi(buf[4].c_str())) {
				// 是公会会长查询
			} else {
				return false;
			}
		} else {
			//pCha->SystemNotice( "查询您的公会信息失败!请稍后再试!" );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00066));
			return false;
		}

		//sprintf(filter, "challid =%d", pCha->GetValidGuildID());
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challid =%d", pCha->GetValidGuildID());

		l_ret = _get_row(buf, 6, param1, filter, &l_retrow);
		if (l_retrow >= 1) {
			//pCha->SystemNotice( "您每次只能挑战一个公会，您已经是公会《%s》的挑战者^_^!", buf[1].c_str() );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00067), buf[1].c_str());
			return false;
		}
	} else {
		return false;
	}

	// 开始事务
	if (!begin_tran()) {
		//pCha->SystemNotice( "挑战公会数据操作失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00068));
		return false;
	}

	char sql[SQL_MAXLEN];
	char szGuild[64];
	memset(szGuild, 0, 64);
	DWORD dwGuildID = 0;
	DWORD dwChallID = 0;
	DWORD dwChallMoney = 0;
	const char* param = "guild_id, guild_name, challid, challmoney";
	//sprintf(filter, "challlevel =%d",byLevel);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challlevel =%d", byLevel);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 4, param, filter, &l_retrow);
	if (l_retrow == 1) {
		dwGuildID = atoi(buf[0].c_str());
		//strncpy( szGuild, buf[1].c_str(), 63 );
		strncpy_s(szGuild, sizeof(szGuild), buf[1].c_str(), _TRUNCATE);
		dwChallID = atoi(buf[2].c_str());
		dwChallMoney = atoi(buf[3].c_str());
	} else {
		DWORD dwMoneyArray[3] = {5000000, 3000000, 1000000};
		if (dwMoney < dwMoneyArray[byLevel - 1] || !pCha->HasMoney(dwMoney)) {
			//pCha->SystemNotice( "想获得排名第%d的公会称号需要金额（%uG）!", byLevel, dwMoneyArray[byLevel-1] );
			//pCha->SystemNotice( RES_STRING(GM_GAMEDB_CPP_00077), byLevel, dwMoneyArray[byLevel-1] );
			char szData[128];
			CFormatParameter param(2);
			param.setDouble(0, byLevel);
			param.setLong(1, dwMoneyArray[byLevel - 1]);
			RES_FORMAT_STRING(GM_GAMEDB_CPP_00077, param, szData);
			pCha->SystemNotice(szData);
			return false;
		}

		//sprintf(sql,	"update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
		//	byLevel, pCha->GetValidGuildID() );
		_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
					byLevel, pCha->GetValidGuildID());

		SQLRETURN l_sqlret = exec_sql_direct(sql);
		if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
			this->rollback();
			//LG( "挑战公会", "挑战公会结束，擂主败：更新失败公会战得公会数据操作失败!公会不存在!公会ID = %d.公会排名：%d", pCha->GetValidGuildID(), byLevel );
			LG("challenge consortia", "challenge consortia over,leizhu failed:update lost consortia data operater failed! consortiaID = %d.consortia level:%d", pCha->GetValidGuildID(), byLevel);
			return false;
		}

		if (!commit_tran()) {
			this->rollback();
			//pCha->SystemNotice( "公会数据操作失败，请稍后再试!" );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00072));
			return false;
		}
		//if( pCha->TakeMoney( "系统", dwMoney ) )
		if (pCha->TakeMoney(RES_STRING(GM_GAMEDB_CPP_00073), dwMoney)) {
			//pCha->SystemNotice( "恭喜你的公会《%s》获得了排名第%d公会的称号!", pCha->GetGuildName(), byLevel );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00074), pCha->GetGuildName(), byLevel);
			char szData[128];
			CFormatParameter param(2);
			param.setString(0, pCha->GetGuildName());
			param.setDouble(1, byLevel);
			RES_FORMAT_STRING(GM_GAMEDB_CPP_00074, param, szData);
		}
		this->ListChallenge(pCha);
		return true;
	}

	BYTE byLvData = 0;
	const char* param2 = "challlevel";
	//sprintf(filter, "guild_id =%d", pCha->GetValidGuildID() );
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", pCha->GetValidGuildID());

	l_ret = _get_row(buf, 4, param2, filter, &l_retrow);
	if (l_retrow == 1) {
		byLvData = (BYTE)atoi(buf[0].c_str());
	} else {
		//pCha->SystemNotice( "查询你的公会排名信息失败!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00078));
		return false;
	}

	if (dwGuildID == 0) {
		//pCha->SystemNotice( "公会排名错误!GID = %d, LV = %d", dwGuildID, byLevel );
		//pCha->SystemNotice( RES_STRING(	), dwGuildID, byLevel );
		char szData[128];
		CFormatParameter param(2);
		param.setLong(0, dwGuildID);
		param.setDouble(1, byLevel);
		RES_FORMAT_STRING(GM_GAMEDB_CPP_00079, param, szData);
		pCha->SystemNotice(szData);
		return false;
	}

	if (byLvData != 0 && byLevel > byLvData) {
		//pCha->SystemNotice( "您不能挑战排名比你们公会低的公会!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00080));
		return false;
	}

	if (pCha->GetPlayer()->GetDBChaId() == dwChallID) {
		//pCha->SystemNotice( "您已经是公会战的挑战者了!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00081));
		return false;
	} else if (pCha->GetValidGuildID() == dwGuildID) {
		//pCha->SystemNotice( "您不能挑战自己的公会!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00082));
		return false;
	} else if (dwMoney < dwChallMoney + 50000) {
		//pCha->SystemNotice( "您的挑战金额不足!金额（%u）", dwMoney );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00083), dwMoney);
		return false;
	}

	if (!pCha->HasMoney(dwMoney)) {
		//pCha->SystemNotice( "您投标失败，没有足够得金钱!金额（%u）", dwMoney );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00084), dwMoney);
		return false;
	}

	// 更新新的挑战公会
	//sprintf(sql,	"update guild set challid =%d,challmoney =%d where guild_id =%d \
	//				and challmoney < %d and challstart = 0",
	//			pCha->GetGuildID(), dwMoney, dwGuildID, dwMoney );
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set challid =%d,challmoney =%d where guild_id =%d \
					and challmoney < %d and challstart = 0",
				pCha->GetGuildID(), dwMoney, dwGuildID, dwMoney);

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//pCha->SystemNotice( "挑战公会操作失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00085));
		return false;
	}

	if (!commit_tran()) {
		this->rollback();
		//pCha->SystemNotice( "挑战公会操作失败，请稍后再试!" );
		pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00085));
		return false;
	}

	// 扣钱
	//pCha->TakeMoney( "系统", dwMoney );
	pCha->TakeMoney(RES_STRING(GM_GAMEDB_CPP_00073), dwMoney);
	// 把之前的挑战公会的钱还过去
	if (dwChallID > 0 && dwChallMoney > 0) {
		WPacket wpk = GETWPACKET();
		WRITE_CMD(wpk, CMD_MP_GUILD_CHALLMONEY);
		WRITE_LONG(wpk, dwChallID);
		WRITE_LONG(wpk, dwChallMoney);
		WRITE_STRING(wpk, szGuild);
		WRITE_STRING(wpk, pCha->GetGuildName());
		pCha->ReflectINFof(pCha, wpk);
	}

	ListChallenge(pCha);
	return true;
}

void CTableGuild::ListChallenge(CCharacter* pCha) {
	std::string buf1[6];
	std::string buf2[6];
	char filter[80];

	DWORD dwGuildID = 0;
	DWORD dwChallID = 0;
	DWORD dwChallMoney = 0;
	DWORD dwLeaderID = 0;
	BYTE byStart = 0;

	WPacket wpk = GETWPACKET();
	WRITE_CMD(wpk, CMD_MC_GUILD_LISTCHALL);

	const char* param = "guild_id, guild_name, challid, challmoney, leader_id, challstart";
	if (pCha->GetValidGuildID() > 0) {
		//sprintf(filter, "guild_id =%d", pCha->GetValidGuildID());
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", pCha->GetValidGuildID());

		int l_retrow = 0;
		bool l_ret = _get_row(buf1, 6, param, filter, &l_retrow);
		if (l_retrow == 1) {
			if (pCha->GetID() == atoi(buf1[4].c_str())) {
				// 是公会会长查询
				WRITE_CHAR(wpk, 1);
			} else {
				WRITE_CHAR(wpk, 0);
			}
		} else {
			//pCha->SystemNotice( "查询您的公会信息失败!请稍后再试!" );
			pCha->SystemNotice(RES_STRING(GM_GAMEDB_CPP_00066));
			return;
		}
	} else {
		WRITE_CHAR(wpk, 0);
	}

	for (int i = 1; i <= 3; ++i) {
		//sprintf(filter, "challlevel =%d", i);
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challlevel =%d", i);

		int l_retrow = 0;
		bool l_ret = _get_row(buf1, 6, param, filter, &l_retrow);
		if (l_retrow == 1) {
			dwGuildID = atoi(buf1[0].c_str());
			dwChallID = atoi(buf1[2].c_str());
			dwChallMoney = atoi(buf1[3].c_str());
			byStart = (BYTE)atoi(buf1[5].c_str());

			if (dwChallID != 0) {
				//sprintf(filter, "guild_id =%d", dwChallID);
				_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", dwChallID);

				bool l_ret = _get_row(buf2, 6, param, filter, &l_retrow);
				if (l_retrow == 1) {
					WRITE_CHAR(wpk, i);
					WRITE_CHAR(wpk, byStart);
					WRITE_STRING(wpk, buf1[1].c_str());
					WRITE_STRING(wpk, buf2[1].c_str());
					WRITE_LONG(wpk, dwChallMoney);
				} else {
					WRITE_CHAR(wpk, 0);
				}
			} else {
				WRITE_CHAR(wpk, i);
				WRITE_CHAR(wpk, byStart);
				WRITE_STRING(wpk, buf1[1].c_str());
				WRITE_STRING(wpk, "");
				WRITE_LONG(wpk, dwChallMoney);
			}
		} else {
			WRITE_CHAR(wpk, 0);
		}
	}
	pCha->ReflectINFof(pCha, wpk);
}

bool CTableGuild::HasGuildLevel(CCharacter* pChar, BYTE byLevel) {
	if (!pChar->HasGuild()) {
		return false;
	}

	std::string buf[1];
	char filter[80];
	BYTE byData = 0;
	const char* param = "challlevel";
	//sprintf(filter, "guild_id =%d",pChar->GetValidGuildID());
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", pChar->GetValidGuildID());

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 1, param, filter, &l_retrow);
	if (l_retrow == 1) {
		byData = (BYTE)atoi(buf[0].c_str());
		return byLevel == byData;
	}
	return false;
}

bool CTableGuild::HasCall(BYTE byLevel) {
	std::string buf[5];
	char filter[80];

	char szGuild[64];
	memset(szGuild, 0, 64);
	DWORD dwGuildID = 0;
	DWORD dwChallID = 0;
	DWORD dwChallMoney = 0;
	const char* param = "guild_id, guild_name, challid, challmoney, challstart";
	//sprintf(filter, "challlevel =%d",byLevel);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challlevel =%d", byLevel);

	int l_retrow = 0;
	BYTE byStart = 0;
	bool l_ret = _get_row(buf, 5, param, filter, &l_retrow);
	if (l_retrow == 1) {
		dwGuildID = atoi(buf[0].c_str());
		//strncpy( szGuild, buf[1].c_str(), 63 );
		strncpy_s(szGuild, sizeof(szGuild), buf[1].c_str(), _TRUNCATE);
		dwChallID = atoi(buf[2].c_str());
		dwChallMoney = atoi(buf[3].c_str());
		byStart = (BYTE)atoi(buf[4].c_str());
		return dwChallID != 0 && byStart == 1;
	}
	return false;
}

bool CTableGuild::StartChall(BYTE byLevel) {
	//LG( "挑战公会", "排名公会第%d挑战赛开始处理...\n", byLevel );
	LG("challenge consortia", "range level %d challenge start treat with....\n", byLevel);
	std::string buf[4];
	char filter[80];

	char szGuild[64];
	memset(szGuild, 0, 64);
	DWORD dwGuildID = 0;
	DWORD dwChallID = 0;
	DWORD dwChallMoney = 0;
	const char* param = "guild_id, guild_name, challid, challmoney";
	//sprintf(filter, "challlevel =%d",byLevel);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challlevel =%d", byLevel);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 4, param, filter, &l_retrow);
	if (l_retrow == 1) {
		dwGuildID = atoi(buf[0].c_str());
		//strncpy( szGuild, buf[1].c_str(), 63 );
		strncpy_s(szGuild, sizeof(szGuild), buf[1].c_str(), _TRUNCATE);
		dwChallID = atoi(buf[2].c_str());
		dwChallMoney = atoi(buf[3].c_str());
	} else {
		return false;
	}

	if (dwGuildID == 0) {
		return false;
	}

	// 更新新的挑战公会
	char sql[SQL_MAXLEN];
	//sprintf(sql,	"update guild set challstart = 1 where guild_id =%d and challstart = 0",
	//			dwGuildID );
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set challstart = 1 where guild_id =%d and challstart = 0",
				dwGuildID);

	SQLRETURN l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		//LG( "挑战公会", "挑战公会数据操作失败!公会战已经开始或者公会不存在!" );
		LG("challenge consortia", "challenge consortia data operator failed!consortia battle already start or inexistence!");
		return false;
	}

	//LG( "挑战公会", "排名第%d公会挑战成功开始!GUILD1 = %d, GUILD2 = %d, Money = %u.\n", byLevel, dwGuildID, dwChallID, dwChallMoney );
	LG("challenge consortia", "range level %d challenge start succeed !GUILD1 = %d, GUILD2 = %d, Money = %u.\n", byLevel, dwGuildID, dwChallID, dwChallMoney);
	return true;
}

void CTableGuild::EndChall(DWORD dwGuild1, DWORD dwGuild2, BOOL bChall) {
	//LG( "挑战公会", "排名公会挑战赛开始结束处理GUILD1 = %d, GUILD2 = %d...\n", dwGuild1, dwGuild2 );
	LG("challenge consortia", "arranger level consortia game start operator finish GUILD1 = %d, GUILD2 = %d...\n", dwGuild1, dwGuild2);
	std::string buf[5];
	char filter[80];

	char szGuild[64];
	memset(szGuild, 0, 64);
	DWORD dwGuildID = 0;
	DWORD dwChallID = 0;
	DWORD dwChallMoney = 0;
	BYTE byLevel = 0;
	BYTE byStart = 0;
	const char* param = "challstart, guild_name, challid, challmoney, challlevel";
	//sprintf(filter, "guild_id =%d",dwGuild1);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", dwGuild1);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 5, param, filter, &l_retrow);
	if (l_retrow == 1) {
		byStart = (BYTE)atoi(buf[0].c_str());
		//strncpy( szGuild, buf[1].c_str(), 63 );
		strncpy_s(szGuild, sizeof(szGuild), buf[1].c_str(), _TRUNCATE);
		dwChallID = atoi(buf[2].c_str());
		dwChallMoney = atoi(buf[3].c_str());
		byLevel = (BYTE)atoi(buf[4].c_str());
		if (dwChallID == dwGuild2) {
			ChallMoney(byLevel, bChall, dwGuild1, dwGuild2, dwChallMoney);
			//LG( "挑战公会", "排名第%d公会挑战结束!GUILD1 = %d, GUILD2 = %d, Money = %u.\n", byLevel, dwGuild1, dwGuild2, dwChallMoney );
			LG("challenge consortia", "range level %d consortia challenge over!GUILD1 = %d, GUILD2 = %d, Money = %u.\n", byLevel, dwGuild1, dwGuild2, dwChallMoney);
			return;
		}
	}

	//sprintf(filter, "guild_id =%d",dwGuild2);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", dwGuild2);

	l_ret = _get_row(buf, 5, param, filter, &l_retrow);
	if (l_retrow == 1) {
		byStart = (BYTE)atoi(buf[0].c_str());
		//strncpy( szGuild, buf[1].c_str(), 63 );
		strncpy_s(szGuild, sizeof(szGuild), buf[1].c_str(), _TRUNCATE);
		dwChallID = atoi(buf[2].c_str());
		dwChallMoney = atoi(buf[3].c_str());
		byLevel = (BYTE)atoi(buf[4].c_str());
		if (dwChallID == dwGuild1) {
			ChallMoney(byLevel, !bChall, dwGuild2, dwGuild1, dwChallMoney);
			//LG( "挑战公会", "排名第%d公会挑战结束!GUILD1 = %d, GUILD2 = %d, Money = %u.\n", byLevel, dwGuild2, dwGuild1, dwChallMoney );
			LG("challenge consortia", "range level %d consortia challenge over!GUILD1 = %d, GUILD2 = %d, Money = %u.\n", byLevel, dwGuild2, dwGuild1, dwChallMoney);
			return;
		}
	}

	//LG( "挑战公会", "公会挑战结果处理失败!GUILD1 = %d, GUILD2 = %d, ChallFlag = %d.\n", dwGuild1, dwGuild2, ( bChall ) ? 1 : 0 );
	LG("challenge consortia", "consortia challenge result disposal failed!GUILD1 = %d, GUILD2 = %d, ChallFlag = %d.\n", dwGuild1, dwGuild2, (bChall) ? 1 : 0);
}

bool CTableGuild::ChallWin(BOOL bUpdate, BYTE byLevel, DWORD dwWinGuildID, DWORD dwFailerGuildID) {
	// 开始事务
	if (!begin_tran()) {
		//LG( "挑战公会", "挑战公会结束，更新公会数据开始事务失败!" );
		LG("challenge consortia", "challenge consortia finish,update consortia data start affair failed!");
		return false;
	}

	// 更新新的挑战公会
	char sql[SQL_MAXLEN];
	SQLRETURN l_sqlret;
	if (!bUpdate) {
		//sprintf(sql,	"update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = 0 where guild_id =%d",
		//	dwFailerGuildID );
		//l_sqlret =exec_sql_direct(sql);
		//if( !DBOK( l_sqlret ) || get_affected_rows() == 0 )
		//{
		//	this->rollback();
		//	LG( "挑战公会", "挑战公会结束，擂主胜：更新失败公会战得公会数据操作失败!公会不存在!公会ID = %d.公会排名：%d", dwFailerGuildID, byLevel );
		//	return false;
		//}
	} else {
		std::string buf[5];
		char filter[80];

		BYTE byLvData = 0;
		const char* param = "challlevel";
		//sprintf(filter, "guild_id =%d",dwWinGuildID);
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "guild_id =%d", dwWinGuildID);

		int l_retrow = 0;
		bool l_ret = _get_row(buf, 5, param, filter, &l_retrow);
		if (l_retrow == 1) {
			byLvData = (BYTE)atoi(buf[0].c_str());
		} else {
			//LG( "挑战公会", "挑战公会结束，擂主败：查询失败公会的排名信息失败!GUILDID = %d, WINID = %d.\n", dwFailerGuildID, dwWinGuildID );
			LG("challenge consortia", "finish challenge consortia,leizhu failed:inquire about failed consortia level info failed!GUILDID = %d, WINID = %d.\n", dwFailerGuildID, dwWinGuildID);
			return false;
		}

		if (byLvData > 0) {
			// 排名互换
			if (byLvData < byLevel) {
				BYTE byTemp = byLevel;
				byLevel = byLvData;
				byLvData = byTemp;
			}

			//sprintf(sql,	"update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
			//	byLvData, dwFailerGuildID );
			_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
						byLvData, dwFailerGuildID);

			l_sqlret = exec_sql_direct(sql);
			if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
				this->rollback();
				//LG( "挑战公会", "挑战公会结束，擂主败：更新失败公会战得公会数据操作失败!公会不存在!公会ID = %d.公会排名：%d.\n", dwFailerGuildID, byLevel );
				LG("challenge consortia", "challenge consortia over,leizhu failed:update lost consortia data operater failed! consortiaID = %d.consortia level:%d.\n", dwFailerGuildID, byLevel);
				return false;
			}
		} else {
			//sprintf(sql,	"update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = 0 where guild_id =%d",
			//	dwFailerGuildID );
			_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = 0 where guild_id =%d",
						dwFailerGuildID);

			l_sqlret = exec_sql_direct(sql);
			if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
				this->rollback();
				//LG( "挑战公会", "挑战公会结束，擂主败：更新失败公会战得公会数据操作失败!公会不存在!公会ID = %d.公会排名：%d.\n", dwFailerGuildID, byLevel );
				LG("challenge consortia", "challenge consortia over,leizhu failed:update lost consortia data operater failed! consortiaID = %d.consortia level:%d.\n", dwFailerGuildID, byLevel);
				return false;
			}
		}
	}

	//sprintf(sql,	"update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
	//			byLevel, dwWinGuildID  );
	_snprintf_s(sql, sizeof(sql), _TRUNCATE, "update guild set challid = 0, challstart = 0, challmoney = 0, challlevel = %d where guild_id =%d",
				byLevel, dwWinGuildID);

	l_sqlret = exec_sql_direct(sql);
	if (!DBOK(l_sqlret) || get_affected_rows() == 0) {
		this->rollback();
		//LG( "挑战公会", "挑战公会结束，更新赢得公会战得公会数据操作失败!公会不存在!公会ID = %d.公会排名：%d.\n", dwWinGuildID, byLevel );
		LG("challenge consortia", "challenge consortia over,update winner consortia data operator failed!inexistence consortia!consortiaID = %d.consortia level:%d.\n", dwWinGuildID, byLevel);
		return false;
	}

	if (!commit_tran()) {
		this->rollback();
		//LG( "挑战公会", "挑战公会数据提交失败，稍后再试!.\n" );
		LG("challenge consortia", "challenge consortia data referring failed,retry later on\n");
		return false;
	}
	return true;
}

void CTableGuild::ChallMoney(BYTE byLevel, BOOL bChall, DWORD dwGuildID, DWORD dwChallID, DWORD dwMoney) {
	if (bChall) {
		//LG( "挑战公会结果", "挑战失败：胜方：ID = %d, 败方：ID = %d, 金钱：%u, 排名：%d.\n", dwGuildID, dwChallID, dwMoney, byLevel  );
		LG("challenge consortia result", "challenge failed: winner:ID = %d,loser:ID = %d, money = %u,level:%d.\n", dwGuildID, dwChallID, dwMoney, byLevel);
		if (!ChallWin(FALSE, byLevel, dwGuildID, dwChallID)) {
			return;
		}

		if (dwChallID != 0) {
			dwMoney = DWORD(float(dwMoney * 80) / 100);
			WPacket wpk = GETWPACKET();
			WRITE_CMD(wpk, CMD_MP_GUILD_CHALL_PRIZEMONEY);
			WRITE_LONG(wpk, dwGuildID);
			WRITE_LONG(wpk, dwMoney);
			SENDTOGROUP(wpk);
		}
	} else {
		//LG( "挑战公会结果", "挑战成功：胜方：ID = %d, 败方：ID = %d, 金钱：%u, 排名：%d.\n", dwChallID, dwGuildID, dwMoney, byLevel  );
		LG("challenge consortia result", "challenge succeed:winner:ID = %d,loser:ID = %d, money = %u,level:%d.\n", dwChallID, dwGuildID, dwMoney, byLevel);
		if (!ChallWin(TRUE, byLevel, dwChallID, dwGuildID)) {
			return;
		}

		dwMoney = DWORD(float(dwMoney * 80) / 100);
		WPacket wpk = GETWPACKET();
		WRITE_CMD(wpk, CMD_MP_GUILD_CHALL_PRIZEMONEY);
		WRITE_LONG(wpk, dwChallID);
		WRITE_LONG(wpk, dwMoney);
		SENDTOGROUP(wpk);
	}
}

bool CTableGuild::GetChallInfo(BYTE byLevel, DWORD& dwGuildID1, DWORD& dwGuildID2, DWORD& dwMoney) {
	std::string buf[3];
	char filter[80];

	DWORD dwGuildID = 0;
	DWORD dwChallID = 0;
	DWORD dwChallMoney = 0;
	const char* param = "guild_id, challid, challmoney";
	//sprintf(filter, "challlevel =%d",byLevel);
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "challlevel =%d", byLevel);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 3, param, filter, &l_retrow);
	if (l_retrow == 1) {
		dwGuildID1 = atoi(buf[0].c_str());
		dwGuildID2 = atoi(buf[1].c_str());
		dwMoney = atoi(buf[2].c_str());

		return true;
	}
	return false;
}

//===============CTableGuild End===========================================

//	2008-7-28	yyy	add	function	begin!

//	记录道具到道具表。
bool CTableItem::LockItem(SItemGrid* sig, int iChaId) {
	//
	char param[80];
	//sprintf(param, "TOP 1 id");
	_snprintf_s(param, sizeof(param), _TRUNCATE, "TOP 1 id");

	//
	char filter[80];
	//sprintf(filter, "ORDER BY id DESC");
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "ORDER BY id DESC");

	//
	std::string buf[1];

	int r1 = 0;
	int r = _get_rowOderby(buf, 1, param, filter, &r1);
	DWORD dwDropertyID;

	if (DBOK(r) && r1 > 0) {
		dwDropertyID = atol(buf[0].c_str()) + 1;
	} else {
		dwDropertyID = 1;
	}

	//	DBID等于零表示尚未加锁。
	if (sig && !sig->dwDBID) {
		//	设置道具的数据库ID号。
		sig->dwDBID = dwDropertyID;

		//	转义道具串。
		std::string s;
		long lnCheckSum = 0;

		if (SItemGrid2String(s, lnCheckSum, sig, 0)) {
			//	写数据库表。
			//sprintf( g_sql, "insert into property (	id,	cha_id,	context,	sum,	time	) values (	%d,	%d, '%s', %d,	getdate())",
			//	dwDropertyID,
			//	iChaId,
			//	s.c_str(),
			//	lnCheckSum
			//	);
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "insert into property (	id,	cha_id,	context,	sum,	time	) values (	%d,	%d, '%s', %d,	getdate())",
						dwDropertyID,
						iChaId,
						s.c_str(),
						lnCheckSum);

			short sExec = exec_sql_direct(g_sql);
			return DBOK(sExec);
		};
	};

	//
	return false;
};

//	2008-7-28	yyy	add	function	end!

// add by ning.yan 2008-11-12 道具解锁 begin
bool CTableItem::UnlockItem(SItemGrid* sig, int iChaId) {
	if (sig) {
		//sprintf( g_sql, "update property set id = 0 where id =%d and cha_id =%d",sig->dwDBID, iChaId );
		//sprintf( g_sql, "delete from property where id =%d and cha_id =%d",sig->dwDBID, iChaId );
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "delete from property where id =%d and cha_id =%d", sig->dwDBID, iChaId);

		short sExec = exec_sql_direct(g_sql);
		sig->dwDBID = 0; // 0表示道具没有锁定
		return DBOK(sExec);
	}
	return false;
}
// end

#ifdef SHUI_JING
BOOL CTableCrystalTrade::Init() {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select \
				   cha_id, cha_name,act_name, BusinessID, Totalnum,crystal_num, crystal_price, createtime, tradetype, ishang, tempcoins\
				   from %s \
				   (nolock) where 1=2",
				_get_table());

	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		char buffer[255];
		//sprintf(buffer, RES_STRING(GM_GAMEDB_CPP_00001), "CrystalTrade");
		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, RES_STRING(GM_GAMEDB_CPP_00001), "CrystalTrade");
		MessageBox(0, buffer, RES_STRING(GM_GAMEDB_CPP_00002), MB_OK);
		return false;
	}
	return true;
}

//获取买卖挂单列表
BOOL CTableCrystalTrade::GetCrystalBuyAndSaleList(std::vector<std::vector<std::string>>& dataBuyTrade, std::vector<std::vector<std::string>>& dataSaleTrade) {
	const int NLIST = 5;
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select top %d crystal_price,crystal_num from %s where tradetype = %d and ishang = %d order by crystal_price desc,crystal_num desc", NLIST, _get_table(), CrystalTradeType::enumbuy, CrystalTradeType::enumhang);

	if (!getalldata(g_sql, dataBuyTrade))
		return false;

	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select top %d crystal_price,crystal_num from %s where tradetype = %d and ishang = %d order by crystal_price asc,crystal_num desc", NLIST, _get_table(), CrystalTradeType::enumsale, CrystalTradeType::enumhang);

	if (!getalldata(g_sql, dataSaleTrade))
		return false;
	return true;
}
//玩家挂单
BOOL CTableCrystalTrade::GetChaBuyAndSale(CCharacter* pCCha, std::vector<std::vector<std::string>>& chaTradeData) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select crystal_price, crystal_num, tradetype, tempcoins from %s where cha_id = %d and ishang = %d", _get_table(), pCCha->m_ID, CrystalTradeType::enumhang);

	if (!getalldata(g_sql, chaTradeData))
		return false;
	return true;
}

BOOL CTableCrystalTrade::IsHasBuyorSale(DWORD cha_id, CrystalTradeType::TradeType type) {
	std::string buf[1];
	char filter[80];

	int isHang = 0;
	const char* param = "ishang";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, " cha_id = %d and tradetype = %d ", cha_id, type);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 1, param, filter, &l_retrow);

	if (l_ret && l_retrow > 0) {
		isHang = atoi(buf[0].c_str());
		if (isHang == 1)
			return true;
	}
	return false;
}

BOOL CTableCrystalTrade::HangBuyTrade(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum) {
	std::string buf[1];
	char filter[255];
	DWORD dwNeedMoney = iCslPrice * iCslNum;
	DWORD dwFlatMoney = pCCha->GetFlatMoney();
	LLong lpGD = pCCha->getAttr(ATTR_GD);
	//查看是否存在该角色买的挂单
	const char* param = "BusinessID";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d and tradetype = %d ", pCCha->m_ID, CrystalTradeType::enumbuy);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 1, param, filter, &l_retrow);
	if (l_ret) {
		if (l_retrow == 0) {
			//不存在就插入一条新的买单
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "Insert into %s ( cha_id, cha_name, act_name, Totalnum, crystal_num, crystal_price, createtime, tradetype, ishang, tempcoins) \
															values (%d, '%s','%s', %d, %d, %d, getdate(), %d, %d, %d)",
						_get_table(), pCCha->m_ID, pCCha->m_name, pCCha->GetPlayer()->GetActName(), iCslNum, iCslNum, iCslPrice, CrystalTradeType::enumbuy, CrystalTradeType::enumhang, dwNeedMoney);
			short sExec = exec_sql_direct(g_sql);
			if (!DBOK(sExec)) {
				LG("Crystal_msg", "INSERT ActName: [%s],ChaName: [%s] Hang Buy Trade Failed! Num: (%lu), Price: (%lu), Total: (%lu) \n",
				   pCCha->GetPlayer()->GetActName(), pCCha->m_name, iCslNum, iCslPrice, iCslNum * iCslPrice);
				return false;
			}
		} else if (l_retrow == 1) {
			//存在就更改
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set crystal_price = %d, Totalnum = %d, crystal_num = %d , ishang = %d,  createtime = getdate(),tempcoins = %d where cha_id = %d and tradetype = %d ", _get_table(), iCslPrice,
						iCslNum, iCslNum, CrystalTradeType::enumhang, dwNeedMoney, pCCha->m_ID, CrystalTradeType::enumbuy);
			short sExec = exec_sql_direct(g_sql);
			if (!DBOK(sExec)) {
				LG("Crystal_msg", "UPDATE ActName: [%s],ChaName: [%s] Hang Buy Trade Failed! Num: (%lu), Price: (%lu), Total: (%lu) \n",
				   pCCha->GetPlayer()->GetActName(), pCCha->m_name, iCslNum, iCslPrice, iCslNum * iCslPrice);
				return false;
			}
		} else
			return false;
		//扣钱
		if (dwNeedMoney > dwFlatMoney) {
			pCCha->SetFlatMoney(0);
			pCCha->TakeMoney(pCCha->GetName(), dwNeedMoney - dwFlatMoney);
		} else {
			pCCha->SetFlatMoney(dwFlatMoney - dwNeedMoney);
		}
		LG("Crystal_msg", "ActName: [%s],ChaName: [%s] Hang Buy Trade Success! Num: (%lu), Price: (%lu), Total: (%lu) \n",
		   pCCha->GetPlayer()->GetActName(), pCCha->m_name, iCslNum, iCslPrice, iCslNum * iCslPrice);
		return true;
	}
	return false;
}

BOOL CTableCrystalTrade::HangSaleTrade(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum) {
	std::string buf[1];
	char filter[255];
	DWORD dwNeedMoney = iCslPrice * iCslNum;
	DWORD dwFlatMoney = pCCha->GetFlatMoney();
	DWORD dwrplMoney = pCCha->GetPlayer()->GetRplMoney();
	//查看是否存在该角色卖的挂单
	const char* param = "BusinessID";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d and tradetype = %d ", pCCha->m_ID, CrystalTradeType::enumsale);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 1, param, filter, &l_retrow);
	if (l_ret) {
		if (l_retrow == 0) {
			//不存在就插入一条新的卖单
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "Insert into %s ( cha_id, cha_name, act_name, Totalnum, crystal_num, crystal_price, createtime, tradetype, ishang, tempcoins) \
															values (%d, '%s', '%s', %d, %d, %d, getdate(), %d, %d, %d)",
						_get_table(), pCCha->m_ID, pCCha->m_name, pCCha->GetPlayer()->GetActName(), iCslNum, iCslNum, iCslPrice, CrystalTradeType::enumsale, CrystalTradeType::enumhang, dwNeedMoney);
			short sExec = exec_sql_direct(g_sql);
			if (DBOK(sExec)) {
				return true;
			}
		} else if (l_retrow == 1) {
			//存在就更改
			_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set crystal_price = %d, Totalnum = %d, crystal_num = %d , ishang = %d,  createtime = getdate(),tempcoins = %d where cha_id = %d and tradetype = %d", _get_table(), iCslPrice,
						iCslNum, iCslNum, CrystalTradeType::enumhang, dwNeedMoney, pCCha->m_ID, CrystalTradeType::enumsale);
			short sExec = exec_sql_direct(g_sql);
			if (DBOK(sExec)) {
				return true;
			}
		}
	}
	return false;
}

BOOL CTableCrystalTrade::CrystalTradeCancel(CCharacter* pCCha, int type, DWORD& iCslNum, DWORD& iCslPrice) {
	std::string buf[3];
	char filter[255];
	DWORD dwHangMoney = 0;
	const char* param = "tempcoins,crystal_num,crystal_price";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, " cha_id = %d and ishang = %d and tradetype = %d ", pCCha->m_ID, CrystalTradeType::enumhang, type);
	int l_retrow = 0;
	bool l_ret = _get_row(buf, 3, param, filter, &l_retrow);
	if (l_ret && l_retrow > 0) {
		dwHangMoney = atoi(buf[0].c_str());
		iCslNum = atoi(buf[1].c_str());
		iCslPrice = atoi(buf[2].c_str());

		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, " update %s set Totalnum = %d, ishang = %d where cha_id = %d and tradetype = %d ", _get_table(), 0, CrystalTradeType::enumstart, pCCha->m_ID, type);
		short sExec = exec_sql_direct(g_sql);
		if (DBOK(sExec)) {
			return true;
		}
	}
	return false;
}

BOOL CTableCrystalTrade::CheckCrystalExchangeMoney(CCharacter* pCCha, DWORD iCslPrice, DWORD iCslNum, CrystalTradeType::TradeType type) {
	std::string buf[1];
	char filter[80];

	DWORD dwHangMoney = 0;
	DWORD dwflatMoney = 0;

	DWORD dwNeedMoney = iCslPrice * iCslNum;
	const char* param = "tempcoins";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d and ishang = %d ", pCCha->m_ID, CrystalTradeType::enumhang);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 1, param, filter, &l_retrow);
	if (l_retrow < 2) {
		dwHangMoney = atoi(buf[0].c_str());
		DWORD dwflatMoney = pCCha->GetFlatMoney();

		if (type == CrystalTradeType::enumbuy) {
			//是否存在足够海盗币
			if (dwflatMoney < dwNeedMoney) {
				LLong pGD = pCCha->getAttr(ATTR_GD);
				if (dwflatMoney + pGD < dwNeedMoney) {
					return false;
				}
			}
			//有足够钱，看是否挂单超过上限(卖水晶得到的钱+平台上的钱)
			if (dwHangMoney + dwflatMoney > MAX_TRADE_MONEY) {
				return false;
			}
			return true;
		} else if (type == CrystalTradeType::enumsale) {
			DWORD pRplMoney = pCCha->GetPlayer()->GetRplMoney();
			if (pRplMoney < iCslNum) {
				return false;
			}
			if ((dwHangMoney + dwNeedMoney + dwflatMoney) > MAX_TRADE_MONEY) {
				return false;
			}
			return true;
		} else
			return false;
	}
	return false;
}
//首先价格相同，个数相同的有没， 没有则按价格相同，个数最多
BOOL CTableCrystalTrade::CheckHasAccordForTrade(DWORD& chaid, std::string& act_tName, DWORD iCslPrice, DWORD iCslNum, DWORD& dcslNum, CrystalTradeType::TradeType type) {
	std::string bufhas[2];
	char filter[255];

	const char* paramhas = "top 1 cha_id, act_name";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "crystal_price = %d and tradetype = %d and  ishang = %d and crystal_num = %d order by createtime desc", iCslPrice, type, CrystalTradeType::enumhang, iCslNum);
	int l_retrow = 0;
	bool l_ret = _get_row(bufhas, 2, paramhas, filter, &l_retrow);
	if (l_ret && (l_retrow > 0)) {
		chaid = atoi(bufhas[0].c_str());
		act_tName = bufhas[1].c_str();
		dcslNum = iCslNum;
		return true;
	} else {
		std::string buf[3];
		const char* param = "top 1 cha_id, act_name,crystal_num";
		_snprintf_s(filter, sizeof(filter), _TRUNCATE, "crystal_price = %d and tradetype = %d and  ishang = %d order by crystal_num desc", iCslPrice, type, CrystalTradeType::enumhang);

		int l_retrow = 0;
		bool l_ret = _get_row(buf, 3, param, filter, &l_retrow);
		if (l_ret && (l_retrow > 0)) {
			chaid = atoi(buf[0].c_str());
			act_tName = buf[1].c_str();
			dcslNum = atoi(buf[2].c_str());
			return true;
		}
	}
	return false;
}
//快买检查有没完全符合条件的
BOOL CTableCrystalTrade::CheckHasMatchAccord(DWORD& chaid, std::string& act_tName, DWORD iCslPrice, DWORD iCslNum, CrystalTradeType::TradeType type) {
	std::string bufhas[2];
	char filter[255];
	const char* paramhas = "top 1 cha_id, act_name";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "crystal_price = %d and tradetype = %d and ishang = %d and crystal_num >= %d order by crystal_num desc", iCslPrice, type, CrystalTradeType::enumhang, iCslNum);
	int l_retrow = 0;
	bool l_ret = _get_row(bufhas, 2, paramhas, filter, &l_retrow);
	if (l_ret && (l_retrow > 0)) {
		chaid = atoi(bufhas[0].c_str());
		act_tName = bufhas[1].c_str();
		return true;
	}
	return false;
}

BOOL CTableCrystalTrade::UpdateHangTrade(char* actname, DWORD cha_id, DWORD dtradeMoney, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state) {

	if (state == CrystalTradeType::enumhang) {
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set crystal_num = crystal_num - %d, tempcoins =tempcoins-%d*crystal_price  where act_name = '%s' and tradetype = %d and cha_id = %d", _get_table(), dtradeMoney, dtradeMoney, actname, type, cha_id);
		short sExec = exec_sql_direct(g_sql);
		if (!DBOK(sExec)) {
			LG("Crystal_msg", "UpdateHangTrade ActName: [%s], Cha_id: [%d] TradeType: [%d], subtract Crystal_Num :(%lu) Failed !\n", actname, cha_id, type, dtradeMoney);

			return false;
		}
		LG("Crystal_msg", "UpdateHangTrade ActName: [%s], Cha_id: [%d] TradeType: [%d], subtract Crystal_Num :(%lu) Successed !\n", actname, cha_id, type, dtradeMoney);
		return true;
	} else if ((state == CrystalTradeType::enumend) || (state == CrystalTradeType::enumstart)) {
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set ishang =  %d where act_name = '%s' and tradetype = %d and cha_id = %d", _get_table(), state, actname, type, cha_id);
		short sExec = exec_sql_direct(g_sql);
		if (!DBOK(sExec)) {
			LG("Crystal_msg", "UpdateHangTrade ActName: [%s], Cha_id: [%d] TradeType: [%d], Changed TradeState to state( %d ) Failed !\n ", actname, cha_id, type, state);
			return false;
		}
		LG("Crystal_msg", "UpdateHangTrade ActName: [%s], Cha_id: [%d] TradeType: [%d], Changed TradeState to state( %d ) Successed !\n ", actname, cha_id, type, state);
		return true;
	} else {
		LG("Crystal_error", "UpdateHangTrade ActName: [%s],Cha_id: [%d] Error type!\n", actname, cha_id);
		return false;
	}
}

BOOL CTableCrystalTrade::UpdateHangTradeForFailed(char* actname, DWORD cha_id, DWORD dtradeMoney, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state) {
	if (state == CrystalTradeType::enumhang) {
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set crystal_num = crystal_num + %d, tempcoins =tempcoins+%d*crystal_price  where act_name = '%s' and tradetype = %d and cha_id = %d", _get_table(), dtradeMoney, dtradeMoney, actname, type, cha_id);
		short sExec = exec_sql_direct(g_sql);
		if (!DBOK(sExec)) {
			LG("Crystal_msg", "UpdateHangTradeForFailed ActName: [%s], Cha_id: [%d] TradeType: [%d], subtract Crystal_Num :(%lu) Failed !\n", actname, cha_id, type, dtradeMoney);
			return false;
		}
		LG("Crystal_msg", "UpdateHangTradeForFailed ActName: [%s], Cha_id: [%d] TradeType: [%d], subtract Crystal_Num :(%lu) Successed !\n", actname, cha_id, type, dtradeMoney);
		return true;
	} else if ((state == CrystalTradeType::enumend) || (state == CrystalTradeType::enumstart)) {
		_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set ishang = %d where act_name = '%s' and tradetype = %d and cha_id = %d", _get_table(), CrystalTradeType::enumhang, actname, type, cha_id);
		short sExec = exec_sql_direct(g_sql);
		if (!DBOK(sExec)) {
			LG("Crystal_msg", "UpdateHangTradeForFailed ActName: [%s], Cha_id: [%d] TradeType: [%d], Changed TradeState to state( %d ) Failed !\n ", actname, cha_id, type, CrystalTradeType::enumhang);
			return false;
		}
		LG("Crystal_msg", "UpdateHangTradeForFailed ActName: [%s], Cha_id: [%d] TradeType: [%d], Changed TradeState to state( %d ) Successed !\n ", actname, cha_id, type, CrystalTradeType::enumhang);
		return true;
	} else {
		LG("Crystal_error", "UpdateHangTrade ActName: [%s], Cha_id: [%d] Error type!\n", actname, cha_id);
		return false;
	}
}
//根据类型获得挂单个数
BOOL CTableCrystalTrade::GetHangNum(CrystalTradeType::TradeType type, int& hangNum) {
	std::string buf[1];
	char filter[80];
	//select count(BusinessID) from dbo.CrystalTrade where ishang = 1 and tradetype = 0
	const char* param = "count(BusinessID)";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "ishang = %d and tradetype = %d", CrystalTradeType::enumhang, type);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 1, param, filter, &l_retrow);
	if (l_ret && l_retrow >= 0) {
		hangNum = atoi(buf[0].c_str());
		return true;
	}
	return false;
}
//修改过期（一周）账单，查询部分满足条件的先修改
BOOL CTableCrystalTrade::ResetCrystalState(std::vector<std::vector<std::string>>& dataHangTrade) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "select top 10 cha_id, cha_name, act_name, tradetype, crystal_num, crystal_price from %s with(nolock) where ishang = %d and createtime < getdate() - 7 ", _get_table(), CrystalTradeType::enumhang);
	if (!getalldata(g_sql, dataHangTrade))
		return false;
	return true;
}
//暂时修改挂单状态，防止在操作退单时，玩家操作挂单
BOOL CTableCrystalTrade::ChangeStateByTemp(DWORD cha_id, CrystalTradeType::TradeType type, CrystalTradeType::TradeState state) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set ishang = %d where cha_id = %d and tradetype = %d ", _get_table(), state, cha_id, type);
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("Crystal_msg", "ChangeStateByTemp Cha_id: [%d] TradeType: [%d], Changed TradeState to state( %d ) Failed !\n ", cha_id, type, state);
		return false;
	}
	LG("Crystal_msg", "ChangeStateByTemp Cha_id: [%d] TradeType: [%d], Changed TradeState to state( %d ) Successed !\n ", cha_id, type, state);
	return true;
}
//取被停的挂单信息
BOOL CTableCrystalTrade::GetCrystalState(CCharacter* pCha, CrystalTradeType::TradeType type, CrystalTradeType::TradeState& state, DWORD& dwNum, DWORD& dwPrice, DWORD& totalnum) {
	std::string buf[4];
	char filter[225];

	const char* param = "crystal_num, crystal_price, Totalnum, ishang";
	_snprintf_s(filter, sizeof(filter), _TRUNCATE, "cha_id = %d and tradetype = %d ", pCha->GetID(), type);

	int l_retrow = 0;
	bool l_ret = _get_row(buf, 4, param, filter, &l_retrow);
	if (l_ret && l_retrow > 0) {
		dwNum = atoi(buf[0].c_str());
		dwPrice = atoi(buf[1].c_str());
		totalnum = atoi(buf[2].c_str());
		state = (CrystalTradeType::TradeState)atoi(buf[3].c_str());
		return true;
	}
	return false;
}

BOOL CTableCrystalTrade::ResetCrystalTotalNum(CCharacter* pCha, DWORD dwNum, CrystalTradeType::TradeType type) {
	_snprintf_s(g_sql, sizeof(g_sql), _TRUNCATE, "update %s set Totalnum = %d where cha_id = %d and tradetype = %d ", _get_table(), dwNum, pCha->GetID(), type);
	short sExec = exec_sql_direct(g_sql);
	if (!DBOK(sExec)) {
		LG("Crystal_msg", "ResetCrystalTotalNum Cha_id: [%d], tradetype [%d] Totalnum to : [%d] Failed !\n ", pCha->GetID(), type, dwNum);
		return false;
	}
	LG("Crystal_msg", "ResetCrystalTotalNum Cha_id: [%d], tradetype [%d] Totalnum to : [%d] Successed !\n ", pCha->GetID(), type, dwNum);
	return true;
}

#endif

CGameDB game_db;

CGameDBPool* CGameDBPool::_instance = nullptr;

CGameDBPool* CGameDBPool::Instance(int size) {
	if (_instance == nullptr) {
		_instance = new CGameDBPool(size);
	}

	return _instance;
}

CGameDBPool::CGameDBPool(int size) : m_iIndex(0) {
	for (int i = 0; i < size; i++) {
		auto* pDB = new CGameDB();
		pDB->Init();

		m_DBPool.push_back(pDB);
	}
}

CGameDBPool::~CGameDBPool() {
	for (auto* db : m_DBPool) {
		if (db) {
			delete db;
		}
	}
	m_DBPool.clear();
}

CGameDB* CGameDBPool::GetGameDB() {
	m_Lock.Lock();
	CGameDB* pDB = nullptr;
	if (!m_DBPool.empty()) {
		pDB = m_DBPool[m_iIndex++ % m_DBPool.size()];
	}
	m_Lock.Unlock();

	return pDB;
}
