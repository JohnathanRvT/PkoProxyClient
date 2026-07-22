#include "StdAfx.h"
#include "uistateform.h"
#include "uiformmgr.h"
#include "uilabel.h"
#include "character.h"
#include "ChaAttr.h"
#include "procirculate.h"
#include "packetcmd.h"
#include "GuildData.h"
#include "uiboatform.h"

using namespace GUI;
//---------------------------------------------------------------------------
// class CStateMgr
//---------------------------------------------------------------------------
bool CStateMgr::Init() {
	CFormMgr& mgr = CFormMgr::s_Mgr;

	frmState = _FindForm("frmState"); //属性表单
	if (!frmState)
		return false;
	frmState->evtShow = _evtMainShow;

	//frmState 内的控件列表
	labStateName = dynamic_cast<CLabelEx*>(frmState->Find("labStateName"));
	if (!labStateName)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labStateName");
	labStateName->SetIsCenter(true);

	FORM_CONTROL_LOADING_CHECK(labGuildName, frmState, CLabelEx, "preperty.clu", "labStateGuid");

	labStateJob = dynamic_cast<CLabelEx*>(frmState->Find("labStateJob"));
	if (!labStateJob)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labStateJob");

	labStateLevel = dynamic_cast<CLabelEx*>(frmState->Find("labStateLevel"));
	if (!labStateLevel)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labStateLevel");

	labStatePoint = dynamic_cast<CLabelEx*>(frmState->Find("labStatePoint"));
	if (!labStatePoint)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labStatePoint");

	labSkillPoint = dynamic_cast<CLabelEx*>(frmState->Find("labSkillPoint"));
	if (!labSkillPoint)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labSkillPoint");

	labFameShow = dynamic_cast<CLabelEx*>(frmState->Find("labFameShow"));
	if (!labFameShow)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labFameShow");

	//6个
	btnStr = dynamic_cast<CTextButton*>(frmState->Find("btnStr"));
	if (!btnStr)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "btnStr");
	btnStr->evtMouseClick = MainMouseDown;

	btnAgi = dynamic_cast<CTextButton*>(frmState->Find("btnAgi"));
	if (!btnAgi)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "btnAgi");
	btnAgi->evtMouseClick = MainMouseDown;

	btnCon = dynamic_cast<CTextButton*>(frmState->Find("btnCon"));
	if (!btnCon)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "btnCon");
	btnCon->evtMouseClick = MainMouseDown;

	btnSta = dynamic_cast<CTextButton*>(frmState->Find("btnSta"));
	if (!btnSta)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "btnSta");
	btnSta->evtMouseClick = MainMouseDown;

	btnLuk = dynamic_cast<CTextButton*>(frmState->Find("btnLuk"));
	if (!btnLuk)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "btnLuk");
	btnLuk->evtMouseClick = MainMouseDown;

	btnDex = dynamic_cast<CTextButton*>(frmState->Find("btnDex"));
	if (!btnDex)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "btnDex");
	btnDex->evtMouseClick = MainMouseDown;

	//frmState下的进度条控件
	labStateEXP = dynamic_cast<CLabelEx*>(frmState->Find("labStateEXP"));
	if (!labStateEXP)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labStateEXP");
	labStateEXP->SetIsCenter(true);

	labStateHP = dynamic_cast<CLabelEx*>(frmState->Find("labStateHP"));
	if (!labStateHP)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labStateHP");
	labStateHP->SetIsCenter(true);

	labStateSP = dynamic_cast<CLabelEx*>(frmState->Find("labStateSP"));
	if (!labStateSP)
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmState->GetName(), "labStateSP");
	labStateSP->SetIsCenter(true);

	//6个基本属性
	labStrshow = (CLabelEx*)frmState->Find("labStrshow");
	labDexshow = (CLabelEx*)frmState->Find("labDexshow");
	labAgishow = (CLabelEx*)frmState->Find("labAgishow");
	labConshow = (CLabelEx*)frmState->Find("labConshow");
	labStashow = (CLabelEx*)frmState->Find("labStashow");
	labLukshow = (CLabelEx*)frmState->Find("labLukshow");
	labSailLevel = (CLabelEx*)frmState->Find("labSailLevel");
	labSailEXP = (CLabelEx*)frmState->Find("labSailEXP");

	//8个附加属性
	labMinAtackShow = (CLabelEx*)frmState->Find("labMinAtackShow");
	labMaxAtackShow = (CLabelEx*)frmState->Find("labMaxAtackShow");
	labFleeShow = (CLabelEx*)frmState->Find("labFleeShow");
	labAspeedShow = (CLabelEx*)frmState->Find("labAspeedShow");
	labHitShow = (CLabelEx*)frmState->Find("labHitShow");
	labDefenceShow = (CLabelEx*)frmState->Find("labDefenceShow");
	labCriticalShow = (CLabelEx*)frmState->Find("labCriticalShow");
	labMfShow = (CLabelEx*)frmState->Find("labMfShow");
	labPhysDefineShow = (CLabelEx*)frmState->Find("labPhysDefineShow");
	labMspeedShow = (CLabelEx*)frmState->Find("labMspeedShow");
	labColShow = (CLabelEx*)frmState->Find("labColShow");
	return true;
}

void CStateMgr::End() {
}

void CStateMgr::FrameMove(DWORD dwTime) {
	if (frmState->GetIsShow()) {
		static CTimeWork time(100);
		if (time.IsTimeOut(dwTime))
			RefreshStateFrm();
	}
}

void CStateMgr::_evtMainShow(CGuiData* pSender) {
	g_stUIState.RefreshStateFrm();
}

void CStateMgr::RefreshStateFrm() {
	CForm* f = g_stUIState.frmState;
	if (!f->GetIsShow()) {
		return;
	}

	CCharacter* pCha = g_stUIBoat.GetHuman();
	if (!pCha) {
		return;
	}

	SGameAttr* pCChaAttr = pCha->getGameAttr();
	if (!pCChaAttr) {
		return;
	}

	char pszCha[256] = {0};

	// 血条
	if (labStateHP) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld/%lld", pCChaAttr->get(ATTR_HP), pCChaAttr->get(ATTR_MXHP));
		labStateHP->SetCaption(pszCha);
	}

	// 玩家经验
	LONG64 num = pCChaAttr->get(ATTR_CEXP);
	LONG64 curlev = pCChaAttr->get(ATTR_CLEXP);
	LONG64 nextlev = pCChaAttr->get(ATTR_NLEXP);

	LONG64 max = nextlev - curlev;
	num = num - curlev;
	if (num < 0) {
		num = 0;
	}

	if (labStateEXP) {
		if (max != 0) {
			_snprintf_s(pszCha, _TRUNCATE, "%4.2f%%", num * 100.0f / max);
		} else {
			_snprintf_s(pszCha, _TRUNCATE, "0.00%%");
		}

		labStateEXP->SetCaption(pszCha);
	}

	// 魔法
	num = pCChaAttr->get(ATTR_SP);
	max = pCChaAttr->get(ATTR_MXSP);
	if (labStateSP) {
		_snprintf_s(pszCha, _TRUNCATE, "%d/%d", (int)num, (int)max);
		labStateSP->SetCaption(pszCha);
	}

	//玩家名称
	if (labStateName) {
		_snprintf_s(pszCha, _TRUNCATE, "%s", pCha->getName());
		labStateName->SetCaption(pszCha);
	}

	//公会
	if (labGuildName) {
		if (CGuildData::GetGuildID()) {
			labGuildName->SetCaption(CGuildData::GetGuildName().c_str());
		} else {
			labGuildName->SetCaption("");
		}
	}

	//玩家职业
	if (labStateJob) {
		_snprintf_s(pszCha, _TRUNCATE, "%s", g_GetJobName(static_cast<short>(pCChaAttr->get(ATTR_JOB))));
		labStateJob->SetCaption(pszCha);
	}

	//玩家等级
	if (labStateLevel) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_LV)); //显示角色等级
		labStateLevel->SetCaption(pszCha);
	}

	//玩家属性点
	if (labStatePoint) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_AP)); //显示角色的属性点
		labStatePoint->SetCaption(pszCha);
	}

	if (labSkillPoint) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_TP)); //显示角色的技能点
		labSkillPoint->SetCaption(pszCha);
	}

	//属性点大于0，则显示6个基本属性按钮
	if (pCChaAttr->get(ATTR_AP) > 0) {
		btnStr->SetIsShow(true);
		btnAgi->SetIsShow(true);
		btnCon->SetIsShow(true);
		btnSta->SetIsShow(true);
		btnLuk->SetIsShow(true);
		btnDex->SetIsShow(true);
	} else {
		btnStr->SetIsShow(false);
		btnAgi->SetIsShow(false);
		btnCon->SetIsShow(false);
		btnSta->SetIsShow(false);
		btnLuk->SetIsShow(false);
		btnDex->SetIsShow(false);
	}
	//6个基本属性
	if (labStrshow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_STR));
		labStrshow->SetCaption(pszCha);
	}

	if (labDexshow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_DEX));
		labDexshow->SetCaption(pszCha);
	}

	if (labAgishow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_AGI));
		labAgishow->SetCaption(pszCha);
	}

	if (labConshow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_CON));
		labConshow->SetCaption(pszCha);
	}

	if (labStashow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_STA));
		labStashow->SetCaption(pszCha);
	}

	if (labLukshow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_LUK));
		labLukshow->SetCaption(pszCha);
	}

	if (labSailLevel) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_SAILLV));
		labSailLevel->SetCaption(pszCha);
	}

	if (labSailEXP) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_CSAILEXP));
		labSailEXP->SetCaption(pszCha);
	}

	//8个附加属性
	if (labMinAtackShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_MNATK)); // 最小攻击力
		labMinAtackShow->SetCaption(pszCha);
	}

	if (labMaxAtackShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_MXATK)); // 最大攻击力
		labMaxAtackShow->SetCaption(pszCha);
	}

	if (labFleeShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_FLEE)); // 闪避率
		labFleeShow->SetCaption(pszCha);
	}

	if (labAspeedShow) {
		LONG64 v = (pCChaAttr->get(ATTR_ASPD));
		if (v == 0) {
			_snprintf_s(pszCha, _TRUNCATE, "-1");
		} else {
			_snprintf_s(pszCha, _TRUNCATE, "%lld", 100000 / v); // 攻击间隔
		}

		labAspeedShow->SetCaption(pszCha);
	}

	if (labHitShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_HIT)); // 命中率
		labHitShow->SetCaption(pszCha);
	}

	if (labDefenceShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_DEF)); // 防御力
		labDefenceShow->SetCaption(pszCha);
	}

	if (labCriticalShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_CRT)); // 爆击率
		labCriticalShow->SetCaption(pszCha);
	}

	if (labMfShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_MF)); // 寻宝率
		labMfShow->SetCaption(pszCha);
	}

	if (labPhysDefineShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_PDEF)); // 物理抵抗
		labPhysDefineShow->SetCaption(pszCha);
	}

	if (labMspeedShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_MSPD));
		labMspeedShow->SetCaption(pszCha);
	}

	if (labColShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_COL));
		labColShow->SetCaption(pszCha);
	}

	if (labFameShow) {
		_snprintf_s(pszCha, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_FAME)); // 声望
		labFameShow->SetCaption(pszCha);
	}
}

void CStateMgr::MainMouseDown(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCharacter* pCha = g_stUIBoat.GetHuman();
	if (!pCha)
		return;

	if (pCha->getGameAttr()->get(ATTR_AP) <= 0)
		return;

	CChaAttr attr;
	attr.ResetChangeFlag();

	std::string name = pSender->GetName();
	if (name == "btnStr") {
		attr.DirectSetAttr(ATTR_STR, 1);
		attr.SetChangeBitFlag(ATTR_STR);
		CProCirculateCS* proCir = (CProCirculateCS*)g_NetIF->GetProCir();
		proCir->SynBaseAttribute(&attr);
	} else if (name == "btnAgi") {
		attr.DirectSetAttr(ATTR_AGI, 1);
		attr.SetChangeBitFlag(ATTR_AGI);
		CProCirculateCS* proCir = (CProCirculateCS*)g_NetIF->GetProCir();
		proCir->SynBaseAttribute(&attr);
	} else if (name == "btnCon") {
		attr.DirectSetAttr(ATTR_CON, 1);
		attr.SetChangeBitFlag(ATTR_CON);
		CProCirculateCS* proCir = (CProCirculateCS*)g_NetIF->GetProCir();
		proCir->SynBaseAttribute(&attr);
	} else if (name == "btnSta") {
		attr.DirectSetAttr(ATTR_STA, 1);
		attr.SetChangeBitFlag(ATTR_STA);
		CProCirculateCS* proCir = (CProCirculateCS*)g_NetIF->GetProCir();
		proCir->SynBaseAttribute(&attr);
	} else if (name == "btnLuk") {
		attr.DirectSetAttr(ATTR_LUK, 1);
		attr.SetChangeBitFlag(ATTR_LUK);
		CProCirculateCS* proCir = (CProCirculateCS*)g_NetIF->GetProCir();
		proCir->SynBaseAttribute(&attr);
	} else if (name == "btnDex") {
		attr.DirectSetAttr(ATTR_DEX, 1);
		attr.SetChangeBitFlag(ATTR_DEX);
		CProCirculateCS* proCir = (CProCirculateCS*)g_NetIF->GetProCir();
		proCir->SynBaseAttribute(&attr);
	}
}
