#include "stdafx.h"
#include "Character.h"
#include "Player.h"
#include "GameDB.h"
#include "GameApp.h"
#include "SubMap.h"
#include "rng.h"

#ifdef XTRAP_S
#include "Xtrap_S_Interface.h"
extern unsigned char g_XTrapMap[CSFLIE_NUM][XTRAP_CS4_BUFSIZE_MAP];

void CCharacter::Session() {
	WPACKET tPacket = GETWPACKET();
	int nRet;
	char sTemp[128] = "";
	memset(sTemp, 0, sizeof(sTemp));
	tPacket.WriteCmd(XTRAP_CMD_STEP_ONE);

	nRet = XTrap_CS_Step1(SessionBuf, sTemp);
	//tPacket.WriteString( sTemp );
	WRITE_SEQ(tPacket, sTemp, 128);
	this->ReflectINFof(this, tPacket);
	if (nRet != XTRAP_CC_RETURN_OK) {
		unsigned int DetectCode = 0;
		memcpy(&DetectCode, ((unsigned char*)sTemp + 8), 4);
		LG("XTRAP", "DetectCode value = %08x \n", DetectCode);
		switch (nRet) {
		case XTRAP_CC_RETURN_ERROR:
			LG("XTRAP", "Character[%s],ID[%ld] XTrap_CS_Step1 has error, reason = %d\n", this->GetName(), this->GetID(), XTRAP_CC_RETURN_ERROR);
			break;
		case XTRAP_CC_RETURN_UNKNOWN:
			LG("XTRAP", "Character[%s],ID[%ld] XTrap_CS_Step1 has error, reason = %d\n", this->GetName(), this->GetID(), XTRAP_CC_RETURN_UNKNOWN);
			break;
		case XTRAP_CC_RETURN_INVALID_PARAMETER:
			LG("XTRAP", "Character[%s],ID[%ld] XTrap_CS_Step1 has error, reason = %d\n", this->GetName(), this->GetID(), XTRAP_CC_RETURN_INVALID_PARAMETER);
			break;
		case XTRAP_CC_RETURN_CRC:
			LG("XTRAP", "Character[%s],ID[%ld] XTrap_CS_Step1 has error, reason = %d\n", this->GetName(), this->GetID(), XTRAP_CC_RETURN_CRC);
			break;
		case XTRAP_CC_RETURN_TIMEOUT:
			LG("XTRAP", "Character[%s],ID[%ld] XTrap_CS_Step1 has error, reason = %d\n", this->GetName(), this->GetID(), XTRAP_CC_RETURN_TIMEOUT);
			break;
		case XTRAP_CC_RETURN_DETECTHACK:
			LG("XTRAP", "Character[%s],ID[%ld] XTrap_CS_Step1 has error, reason = %d\n", this->GetName(), this->GetID(), XTRAP_CC_RETURN_DETECTHACK);
			break;
		default:
			LG("XTRAP", "Character[%s],ID[%ld] XTrap_CS_Step1 has error, reason = %d\n", this->GetName(), this->GetID(), nRet);
			break;
		}
		KICKPLAYER(this->GetPlayer(), 0);
		g_pGameApp->GoOutGame(this->GetPlayer(), true);
		return;
	}
}
#endif
//----------------------------------------------
//       所有Character自身的定时循环处理
//----------------------------------------------

// 总循环入口
void CCharacter::Run(DWORD dwCurTime) {
	
	try {
		LETimer t;
		Char chCount = 0;

		t.Begin();

		if (m_pCPlayer && !m_pCPlayer->IsValid())
			return;
		if (!GetSubMap())
			return;

		bool bIsLiveing = IsLiveing();

		//extern CGameApp *g_pGameApp;
		//g_pGameApp->m_dwRunStep = 1000 + m_ID;

		m_dwCellRunTime[chCount++] = t.End();
#ifdef XTRAP_S
		if (m_xdrapTime.IsOK(dwCurTime) && (this->GetPlayer()) && (this->GetPlayer()->GetGMLev() == 0))
			Session();
#endif

		// 怪物(包含宠物等)生命时间检查
		if (IsPlayerCha() == false && IsNpc() == nullptr) {
			if (CheckLifeTime()) // 时间到
			{
				if (m_HostCha && m_HostCha->IsPlayerCha()) {
					int nPetNum = m_HostCha->GetPlyMainCha()->GetPetNum();
					if (nPetNum > 0)
						m_HostCha->GetPlyMainCha()->SetPetNum(nPetNum - 1);
				}
				// 调用一个脚本作为事件通知
				// Delete by lark.li 20090503 begin(没有找到脚本函数)
				//g_CParser.DoString("event_cha_lifetime", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, DOSTRING_PARAM_END);
				// End
				Free(); // 释放, 视野通知
				// char szLua[255];
				// sprintf(szLua, "CreateCha(133, %d, %d, 145, 50)", this->GetPos().x, this->GetPos().y);
				// luaL_handled_dostring(g_pLuaState, szLua); //CHANGED: better lua error handling (deguix)
				return;
			}
		}

		//防外挂暂时不上
		/*if(IsPlayerCha() && !IsGMCha2() && ((!(GetAreaAttr() & enumAREA_TYPE_NOT_FIGHT)) || IsBoat()) && !GetPlyCtrlCha()->GetSubMap()->GetMapRes()->CanPK())
	{
		GetPlyMainCha()->CheatRun(dwCurTime);
	}*/

		//add by jilinlee 2007/4/20
		//是否在读书状态
		if (IsReadBook()) {
			if (bIsLiveing) {
				if (m_SReadBook.dwLastReadCallTick == 0) {
					m_SReadBook.dwLastReadCallTick = dwCurTime;
				}

				static DWORD dwReadBookTime = 0;
				if (dwReadBookTime == 0 && g_CParser.DoString("ReadBookTime", enumSCRIPT_RETURN_NUMBER, 1, DOSTRING_PARAM_END)) {
					dwReadBookTime = g_CParser.GetReturnNumber(0);
				}
				//else
				//	dwReadBookTime = 600*1000;   //取不到的话，默认为十分钟。
				if (dwCurTime - m_SReadBook.dwLastReadCallTick >= dwReadBookTime) {
					//调用脚本函数
					char chSkillLv = 0;
					static short sSkillID = 0;
					if (sSkillID == 0 && g_CParser.DoString("ReadBookSkillId", enumSCRIPT_RETURN_NUMBER, 1, DOSTRING_PARAM_END)) {
						sSkillID = g_CParser.GetReturnNumber(0);
					}
					SSkillGrid* pSkill = this->m_CSkillBag.GetSkillContByID(sSkillID); //读书技能的技能ID
					if (pSkill) {
						chSkillLv = pSkill->chLv;
						g_CParser.DoString("Reading_Book", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 1, chSkillLv, DOSTRING_PARAM_END);
					}
					m_SReadBook.dwLastReadCallTick = dwCurTime;
				}
			} else
				SetReadBookState(false);
		}

		t.Begin();
		if (bIsLiveing)
			m_CActCache.Run();
		m_dwCellRunTime[chCount++] = t.End();

		t.Begin();
		DWORD dwResumeExecTime = m_timerScripts.IsOK(dwCurTime);
		if (dwResumeExecTime > 0)
			OnScriptTimer(dwResumeExecTime, true);
		m_dwCellRunTime[chCount++] = t.End();

		t.Begin();
		if (IsPlayerOwnCha())
			GetPlayer()->Run(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();

		// 定时器检查
		t.Begin();
		if (m_timerAI.IsOK(dwCurTime))
			OnAI(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();
		t.Begin();
		if (m_timerAreaCheck.IsOK(dwCurTime))
			OnAreaCheck(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();
		t.Begin();
		if (m_timerDie.IsOK(dwCurTime))
			OnDie(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();
		t.Begin();
		if (m_timerMission.IsOK(dwCurTime))
			OnMissionTime();
		m_dwCellRunTime[chCount++] = t.End();
		t.Begin();
		if (m_timerTeam.IsOK(dwCurTime))
			OnTeamNotice(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();

		t.Begin();
		if (bIsLiveing)
			if (m_timerSkillState.IsOK(dwCurTime))
				OnSkillState(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();
		t.Begin();
		if (bIsLiveing)
			OnMove(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();
		t.Begin();
		if (bIsLiveing)
			OnFight(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();
		t.Begin();
		if (bIsLiveing)
			if (m_timerDBUpdate.IsOK(dwCurTime))
				OnDBUpdate(dwCurTime);
		m_dwCellRunTime[chCount++] = t.End();

		// 防沉迷状态
		if (m_noticeState > 2) {
			if (m_timerSafeAdult.IsOK(dwCurTime))
				OnSafeAdultUpdate(dwCurTime);
		}

		t.Begin();
		if (IsPlayerCtrlCha()) {
			if (m_timerPing.IsOK(dwCurTime))
				CheckPing();

			if (m_timerNetSendFreq.IsOK(dwCurTime) && m_ulNetSendLen > 0) {
				WPACKET WtPk = GETWPACKET();
				WRITE_CMD(WtPk, 0xffff);
				for (uLong i = 0; i < m_ulNetSendLen; i++)
					WRITE_CHAR(WtPk, rng.uniform(0, 256 - 1));
				ReflectINFof(this, WtPk);
			}
		}
		m_dwCellRunTime[chCount++] = t.End();
	}
	T_E
}

void CCharacter::RunEnd(DWORD dwCurTime) {
	try {
		if (m_byExit == CHAEXIT_BEGIN && m_timerExit.IsOK(dwCurTime)) {
			// 正式退出
			Exit();
		}
	}
	T_E
}

void CCharacter::StartExit() {
	try {
		//LG( "延时退出", "StartExit: Name = %s,exitcode = %d\n", this->GetName(), m_byExit );
		LG("time too long exit", "StartExit: Name = %s,exitcode = %d\n", this->GetName(), m_byExit);
		if (m_byExit != CHAEXIT_BEGIN) {
			DWORD dwExitTime = 20 * 1000;
			m_byExit = CHAEXIT_BEGIN;
			m_timerExit.Begin(dwExitTime);

			WPACKET wpk = GETWPACKET();
			WRITE_CMD(wpk, CMD_MC_STARTEXIT);
			WRITE_LONG(wpk, dwExitTime);
			ReflectINFof(this, wpk);
		}
	}
	T_E
}

void CCharacter::CancelExit() {
	try {
		//LG( "延时退出", "CancelExit: Name = %s,exitcode = %d\n", this->GetName(), m_byExit );
		LG("time too long exit", "CancelExit: Name = %s,exitcode = %d\n", this->GetName(), m_byExit);
		if (m_byExit == CHAEXIT_BEGIN) {
			m_byExit = CHAEXIT_NONE;
			m_timerExit.Reset();

			WPACKET wpk = GETWPACKET();
			WRITE_CMD(wpk, CMD_MC_CANCELEXIT);
			ReflectINFof(this, wpk);
		}
	}
	T_E
}

void CCharacter::Exit() {
	try {
		// 正式退出
		//LG( "延时退出", "Exit: Name = %s, exitcode = %d\n", this->GetName(), m_byExit );
		LG("time too long exit", "Exit: Name = %s, exitcode = %d\n", this->GetName(), m_byExit);
		WPACKET wpk = GETWPACKET();
		WRITE_CMD(wpk, CMD_MT_PALYEREXIT);
		ReflectINFof(this, wpk);
		g_pGameApp->GoOutGame(this->GetPlayer(), true);

		m_byExit = CHAEXIT_NONE;
		m_timerExit.Reset();
	}
	T_E
}

// 定时检查地域变化
void CCharacter::OnAreaCheck(DWORD dwCurTime) {
}

void CCharacter::SetNoticeState(int state) {
	switch (state) {
	case 1: {
		if (this->m_noticeState != 1) {
			this->m_noticeState = 1;
			//pCha->BickerNotice("您累计在线时间已满 %d 小时。", ulTime);
			this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00001), state);
		}
	} break;
	case 2: {
		if (this->m_noticeState != 2) {
			this->m_noticeState = 2;
			//pCha->BickerNotice("您累计在线时间已满 %d 小时。", ulTime);
			this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00001), state);
		}
	} break;
	case 3: {
		if (this->m_noticeState != 3) {
			this->m_noticeState = 3;
			//pCha->BickerNotice("pCha->BickerNotice("您累计在线时间已满 3 小时，请您下线休息，做适当的身体活动。您已经进入疲劳游戏时间，您的游戏收益将降为正常值的一半，为了您的健康，请尽快下线休息，做适当身体活动，合理安排学习生活。");
			this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00002));

			this->SetExpScale(50);

			// 最少为10分钟
			if (g_Config.m_lSafeAdult >= 10 * 60 * 1000 && g_Config.m_lSafeAdult <= 45 * 60 * 1000)
				m_timerSafeAdult.Begin(g_Config.m_lSafeAdult);
			else
				m_timerSafeAdult.Begin(10 * 60 * 1000);
		}
	} break;
	case 4: {
		if (this->m_noticeState != 4) {
			this->m_noticeState = 4;
			//pCha->BickerNotice("您已经进入疲劳游戏时间，您的游戏收益将降为正常值的一半，为了您的将康，请尽快下线休息，做适当身体活动，合理安排学习生活。");
			this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00003));

			this->SetExpScale(50);

			// 最少为10分钟
			if (g_Config.m_lSafeAdult >= 10 * 60 * 1000 && g_Config.m_lSafeAdult <= 45 * 60 * 1000)
				m_timerSafeAdult.Begin(g_Config.m_lSafeAdult);
			else
				m_timerSafeAdult.Begin(10 * 60 * 1000);
		}
	} break;
	case 5: {
		if (this->m_noticeState != 5) {
			this->m_noticeState = 5;
			//pCha->BickerNotice("您已进入不健康游戏时间，为了您的健康，请您立即下线休息。如不下线，您的身体将受到损害，您的收益已降为零，直到您的累计下线时间满 5 小时后，才能恢复正常。");
			//pCha->BickerNotice("您已经进入不健康时间，60秒后系统将退出...");
			this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00008));

			this->SetExpScale(0);
			m_timerSafeAdult.Begin(60 * 1000);
		}
	} break;
	}
}

void CCharacter::OnSafeAdultUpdate(DWORD dwCurTime) {
	try {
		if (this->m_noticeState == 3) {
			//pCha->BickerNotice("pCha->BickerNotice("您累计在线时间已满 3 小时，请您下线休息，做适当的身体活动。您已经进入疲劳游戏时间，您的游戏收益将降为正常值的一半，为了您的健康，请尽快下线休息，做适当身体活动，合理安排学习生活。");
			this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00002));
		}
		if (this->m_noticeState == 4) {
			//pCha->BickerNotice("您已进入不健康游戏时间，为了您的健康，请您立即下线休息。如不下线，您的身体将受到损害，您的收益已降为零，直到您的累计下线时间满 5 小时后，才能恢复正常。");
			this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00003));
		}
		if (this->m_noticeState == 5) {
			//pCha->BickerNotice("您已经进入不健康时间，60秒后系统将退出...");
			//this->BickerNotice(RES_STRING(GM_GAMEAPPNET_CPP_00008));

			LG("SafeAdult", "%s kill\n", this->m_name);

			m_timerSafeAdult.Reset();
			// 最少为10分钟
			if (g_Config.m_lSafeAdult >= 10 * 60 * 1000 && g_Config.m_lSafeAdult <= 120 * 60 * 1000)
				m_timerSafeAdult.Begin(g_Config.m_lSafeAdult);

			KICKPLAYER(this->GetPlayer(), 0);
			g_pGameApp->GoOutGame(this->GetPlayer(), true);
		}
	}
	T_E
}

void CCharacter::OnDBUpdate(DWORD dwCurTime) {
	try {
		CPlayer* pCPlayer = GetPlayer();
		if (!pCPlayer)
			return;
		if (!pCPlayer->IsPlayer() || pCPlayer->GetMainCha() != this)
			return;

		//LG("Thread", "OnDBUpdate %d\n", ::GetCurrentThreadId());

		LG("enter_map", "OnDBUpdate start!\n");
#ifdef UPDATE_MUL_THREAD

		UpdateDBCall* pCall = g_gmsvr->m_UpdateProc.Get();
		if (pCall) {
			pCall->Init(pCPlayer);
			g_gmsvr->GetProcessor()->AddTask(pCall);
		} else {
			LG("enter_map", "OnDBUpdate error!\n");
		}

#else
		LETimer t;
		t.Begin();
		game_db.SavePlayer(pCPlayer, enumSAVE_TYPE_TIMER);
		DWORD tock = t.End();

		//LG("FFF", "%u\n", tock);
#endif
		LG("enter_map", "OnDBUpdate end!\n");
	}
	T_E
}

BOOL CCharacter::SaveMissionData() {
	try {
		CPlayer* pCPlayer = GetPlayer();
		if (!pCPlayer)
			return FALSE;
		if (!game_db.SaveMissionData(pCPlayer, pCPlayer->GetDBChaId())) {
			//SystemNotice( "新建角色《%s》存储角色初始任务信息失败！ID[0x%X]", this->GetName(), pCPlayer->GetDBChaId() );
			//SystemNotice( RES_STRING(GM_CHARACTERRUN_CPP_00001), this->GetName(), pCPlayer->GetDBChaId() );
			char szData[128];
			CFormatParameter param(2);
			param.setString(0, this->GetName());
			param.setLong(1, pCPlayer->GetDBChaId());
			RES_FORMAT_STRING(GM_CHARACTERRUN_CPP_00001, param, szData);
			SystemNotice(szData);
			return FALSE;
		}
		return TRUE;
	}
	T_E
}

void CCharacter::OnTeamNotice(DWORD dwCurTime) {
	try {
		CPlayer* pCPlayer = GetPlayer();
		if (!pCPlayer)
			return;

		pCPlayer->NoticeTeamMemberData();
	}
	T_E
}

// 脚本定时器，用于HP回复，宠物消耗等
void CCharacter::OnScriptTimer(DWORD dwExecTime, bool bNotice) {
	try {
		Long lOldHP = (long)getAttr(ATTR_HP);
		m_CChaAttr.ResetChangeFlag();
		if (IsPlayerCha())
			GetKitbag()->SetChangeFlag(false);

		g_CParser.DoString("cha_timer", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, this, enumSCRIPT_PARAM_NUMBER, 2, defCHA_SCRIPT_TIMER / 1000, dwExecTime, DOSTRING_PARAM_END);

		// 船在海上时会消耗生命
		if (lOldHP > 0 && getAttr(ATTR_HP) <= 0) {
			if (IsBoat() && IsPlayerCha()) {
				SetItemHostObj(nullptr);
				ItemCount(this);
				SetDie(g_pCSystemCha);
				Die();
				GetPlayer()->GetMainCha()->BoatDie(*this, *this);
			}
		}

		if (bNotice) {
			SynAttr(enumATTRSYN_AUTO_RESUME);
			if (IsPlayerCha())
				SynKitbagNew(enumSYN_KITBAG_ATTR);
		}
	}
	T_E
}
