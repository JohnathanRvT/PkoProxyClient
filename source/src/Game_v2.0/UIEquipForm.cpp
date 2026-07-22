#include "StdAfx.h"
#include "uiequipform.h"
#include "uiskilllist.h"
#include "packetcmd.h"
#include "Character.h"
#include "gameapp.h"
#include "uiitemcommand.h"
#include "uitradeform.h"
#include "uiboatform.h"
#include "chastate.h"
#include "ProCirculate.h"
#include "stpose.h"
#include "StringLib.h"
#include "UIDoublePwdForm.h"
#include "UIStoreForm.h"

using namespace GUI;

static char szBuf[32] = {0};

CGuiPic CEquipMgr::_imgCharges[enumEQUIP_NUM];
CEquipMgr::SSplitItem CEquipMgr::SSplit;

extern CDoublePwdMgr g_stUIDoublePwd;

//---------------------------------------------------------------------------
// class CMainMgr
//---------------------------------------------------------------------------
bool CEquipMgr::Init() {
	CForm* frmMain800 = _FindForm("frmMain800");

	///////////技能系列
	frmSkill = _FindForm("frmSkill");
	if (!frmSkill) {
		return false;
	}
	frmSkill->evtShow = _evtSkillFormShow;

	lstFightSkill = dynamic_cast<CSkillList*>(frmSkill->Find("lstSkill"));
	if (!lstFightSkill) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmSkill->GetName(), "lstSkill");
	}
	lstFightSkill->evtUpgrade = _evtSkillUpgrade;

	lstLifeSkill = dynamic_cast<CSkillList*>(frmSkill->Find("lstSkillW"));
	if (!lstLifeSkill) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmSkill->GetName(), "lstSkillW");
	}
	lstLifeSkill->evtUpgrade = _evtSkillUpgrade;

	lstSailSkill = dynamic_cast<CSkillList*>(frmSkill->Find("lstSkillS"));
	if (!lstSailSkill) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmSkill->GetName(), "lstSkillS");
	}
	lstSailSkill->evtUpgrade = _evtSkillUpgrade;
	lstSailSkill->SetIsShowUpgrade(false);

	labPoint = dynamic_cast<CLabel*>(frmSkill->Find("labPoint"));
	if (!labPoint) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmSkill->GetName(), "labPoint");
	}

	labPointLife = dynamic_cast<CLabel*>(frmSkill->Find("labPoint1"));
	if (!labPointLife) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmSkill->GetName(), "labPoint1");
	}

	//////////道具系列
	frmItem = _FindForm("frmItem"); // 道具表单
	if (!frmItem) {
		return false;
	}
	frmItem->evtShow = _evtItemFormShow;
	frmItem->evtClose = _evtItemFormClose;
	frmItem->evtEntrustMouseEvent = _evtItemFormMouseEvent;

	imgItem4 = dynamic_cast<CImage*>(frmItem->Find("imgItem4"));
	if (!imgItem4) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmItem->GetName(), "imgItem4");
	}

	// 道具以及装备栏
	grdItem = dynamic_cast<CGoodsGrid*>(frmItem->Find("grdItem"));
	if (!grdItem) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmItem->GetName(), "grdItem");
	}
	grdItem->evtThrowItem = evtThrowItemEvent;
	grdItem->evtSwapItem = evtSwapItemEvent;
	grdItem->evtBeforeAccept = _evtDragToGoodsEvent;
	grdItem->evtRMouseEvent = _evtRMouseGridEvent;

	lblGold = dynamic_cast<CLabel*>(frmItem->Find("labItemgoldnumber"));
	if (!lblGold) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmItem->GetName(), "labItemgoldnumber");
	}

	// 二次密码锁
	imgLock = dynamic_cast<CImage*>(frmItem->Find("imgLock"));
	if (!imgLock) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmItem->GetName(), "imgLock");
	}
	imgUnLock = dynamic_cast<CImage*>(frmItem->Find("imgUnLock"));
	if (!imgUnLock) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmItem->GetName(), "imgUnLock");
	}
	SetIsLock(false); // 默认不锁定

	///////////////快捷键系列
	CForm* frmFast = _FindForm("frmFast");
	if (!frmFast) {
		return false;
	}

	// 上下换页按钮
	auto* btnFastUp = dynamic_cast<CTextButton*>(frmFast->Find("btnFastUp"));
	if (!btnFastUp) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmMain800->GetName(), "btnFastUp");
	}
	btnFastUp->evtMouseClick = _evtButtonClickEvent;

	auto* btnFastDown = dynamic_cast<CTextButton*>(frmFast->Find("btnFastDown"));
	if (!btnFastDown) {
		return Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmMain800->GetName(), "btnFastDown");
	}
	btnFastDown->evtMouseClick = _evtButtonClickEvent;

	_pFastCommands = new CFastCommand* [SHORT_CUT_NUM] {};
	std::string name;
	for (size_t j = 0; j < MAX_FAST_COL; j++) {
		name = "fscMainF" + std::to_string(j);
		_pFastCommands[j] = dynamic_cast<CFastCommand*>(frmFast->Find(name.c_str()));
		if (_pFastCommands[j]) {
			_pFastCommands[j]->nTag = j;
			_pFastCommands[j]->evtChange = _evtFastChange;
		}
	}
	int nIndex = 0;
	for (size_t i = 1; i < MAX_FAST_ROW; i++) {
		for (size_t j = 0; j < MAX_FAST_COL; j++) {
			if (_pFastCommands[j]) {
				nIndex = i * MAX_FAST_COL + j;
				_pFastCommands[nIndex] = new CFastCommand(*_pFastCommands[j]);
				_pFastCommands[nIndex]->AddForm();
				_pFastCommands[nIndex]->nTag = nIndex;
				_pFastCommands[j]->evtChange = _evtFastChange;
			}
		}
	}
	_pActiveFastLabel = dynamic_cast<CLabel*>(frmMain800->Find("labFast"));
	_ActiveFast(0);

	if (auto* cmdLost = dynamic_cast<COneCommand*>(frmItem->Find("cmdLost")); cmdLost) {
		cmdLost->evtBeforeAccept = _evtEquipEvent;
	}

	cnmEquip[enumEQUIP_HEAD] = dynamic_cast<COneCommand*>(frmItem->Find("cmdArmet"));
	cnmEquip[enumEQUIP_FACE] = dynamic_cast<COneCommand*>(frmItem->Find("cmdFace"));
	cnmEquip[enumEQUIP_BODY] = dynamic_cast<COneCommand*>(frmItem->Find("cmdBody"));
	cnmEquip[enumEQUIP_GLOVE] = dynamic_cast<COneCommand*>(frmItem->Find("cmdGlove"));
	cnmEquip[enumEQUIP_SHOES] = dynamic_cast<COneCommand*>(frmItem->Find("cmdShoes"));
	cnmEquip[enumEQUIP_LHAND] = dynamic_cast<COneCommand*>(frmItem->Find("cmdLeftHand"));
	cnmEquip[enumEQUIP_RHAND] = dynamic_cast<COneCommand*>(frmItem->Find("cmdRightHand"));
	cnmEquip[enumEQUIP_NECK] = dynamic_cast<COneCommand*>(frmItem->Find("cmdNecklace"));
	cnmEquip[enumEQUIP_HAND1] = dynamic_cast<COneCommand*>(frmItem->Find("cmdCirclet1"));
	cnmEquip[enumEQUIP_HAND2] = dynamic_cast<COneCommand*>(frmItem->Find("cmdCirclet2"));
	cnmEquip[enumEQUIP_Jewelry1] = dynamic_cast<COneCommand*>(frmItem->Find("cmdJewelry1"));
	cnmEquip[enumEQUIP_Jewelry2] = dynamic_cast<COneCommand*>(frmItem->Find("cmdJewelry2"));
	cnmEquip[enumEQUIP_Jewelry3] = dynamic_cast<COneCommand*>(frmItem->Find("cmdJewelry3"));
	cnmEquip[enumEQUIP_Jewelry4] = dynamic_cast<COneCommand*>(frmItem->Find("cmdJewelry4"));

	for (size_t i = 0; i < cnmEquip.size(); i++) {
		if (cnmEquip[i]) {
			cnmEquip[i]->evtBeforeAccept = _evtEquipEvent;
			//TODO: Consider enabling throwing equips from equip slots
			//(need to make sure inventory window doesn't disappear on map change - one of the reasons for accidental equip loss)
			//cnmEquip[i]->evtThrowItem = _evtThrowEquipEvent;	// 屏蔽直接将装备丢弃

			cnmEquip[i]->SetActivePic(&_imgCharges[i]);
		}
	}

	const int nTotalSkill = CSkillRecordSet::I()->GetLastID() + 1;
	for (int i = 0; i < nTotalSkill; i++) {
		CSkillRecord* pInfo = GetSkillRecordInfo(i);
		if (pInfo) {
			if (pInfo->nStateID) {
				_cancels.push_back(pInfo);
			}
		}
	}

	const int nTotalState = CSkillStateRecordSet::I()->GetLastID() + 1;
	for (int i = 0; i < nTotalState; i++) {
		CSkillStateRecord* pState = GetCSkillStateRecordInfo(i);
		if (pState) {
			if (pState->sChargeLink >= 0 && pState->sChargeLink < enumEQUIP_NUM) {
				_charges.push_back(pState);
			}
		}
	}
	return true;
}

void CEquipMgr::End() {
	//RefreshServerShortCut(); Human已经无效
}

void CEquipMgr::_evtSkillUpgrade(CSkillList* pSender, const CSkillCommand* pSkill) {
	if (!pSkill) {
		return;
	}
	const int nSkillID = pSkill->GetSkillID();
	g_NetIF->GetProCir()->SkillUpgrade(nSkillID, 1);
}

void CEquipMgr::LoadingCall() {
	CCharacter* pMain = CGameScene::GetMainCha();
	if (pMain) {
		const int nJob = (int)pMain->getGameAttr()->get(ATTR_JOB);

		for (size_t i = 0; i < 4; i++) {
			CSkillList* pSkillList = GetSkillList(i);
			if (!pSkillList) {
				continue;
			}

			const int nCount = pSkillList->GetCount();
			CSkillCommand* pObj{nullptr};
			for (size_t j = 0; j < nCount; j++) {
				pObj = pSkillList->GetCommand(j);
				if (pObj) {
					pObj->GetSkillRecord()->Refresh(nJob);
				}
			}
		}
	}
}

void CEquipMgr::SynSkillBag(DWORD dwCharID, stNetSkillBag* pSSkillBag) {
	CCharacter* pCha = g_stUIBoat.GetHuman();
	if (!pCha || pCha->getAttachID() != dwCharID) {
		LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_547));
		return;
	}

	const int nJob = (int)pCha->getGameAttr()->get(ATTR_JOB);
	const int nCount = pSSkillBag->SBag.GetCount();
	SSkillGridEx* pSBag = pSSkillBag->SBag.GetValue();
	switch (pSSkillBag->chType) {
	case enumSYN_SKILLBAG_INIT: { // 技能栏初始化（发送全部的技能信息）
		lstSailSkill->Clear();
		lstFightSkill->Clear();
		lstLifeSkill->Clear();
		_skills.clear();

		for (size_t i = 0; i < nCount; i++) {
			CSkillRecord* pInfo = GetSkillRecordInfo(pSBag[i].sID);
			if (!pInfo) {
				LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_548), pSBag[i].sID);
				continue;
			}
			pInfo->GetSkillGrid() = pSBag[i];
			pInfo->Refresh(nJob);
			if (!pInfo->IsShow()) {
				continue;
			}

			auto command = std::make_unique<CSkillCommand>(pInfo);
			GetSkillList(pInfo->chFightType)->AddCommand(std::move(command));

			_skills.push_back(pInfo);
		}
	} break;
	case enumSYN_SKILLBAG_ADD: { // 增加技能（只发送新增加的技能的信息）

		for (size_t i = 0; i < nCount; i++) {
			CSkillRecord* pInfo = GetSkillRecordInfo(pSBag[i].sID);
			if (!pInfo) {
				LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_549), pSBag[i].sID);
				continue;
			}
			pInfo->GetSkillGrid() = pSBag[i];
			pInfo->Refresh(nJob);
			if (!pInfo->IsShow()) {
				continue;
			}

			auto command = std::make_unique<CSkillCommand>(pInfo);
			GetSkillList(pInfo->chFightType)->AddCommand(std::move(command));
			_skills.push_back(pInfo);
		}
	} break;
	case enumSYN_SKILLBAG_MODI: { // 修改技能

		for (size_t i = 0; i < nCount; i++) {
			CSkillRecord* pInfo = GetSkillRecordInfo(pSBag[i].sID);
			if (!pInfo) {
				LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_550), pSBag[i].sID);
				continue;
			}
			pInfo->GetSkillGrid() = pSBag[i];
			pInfo->Refresh(nJob);
			if (!pInfo->IsShow()) {
				continue;
			}

			if (pSBag[i].chLv == 0 && !GetSkillList(pInfo->chFightType)->DelSkill(pSBag[i].sID)) {
				LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_551), pSBag[i].sID);
				continue;
			}
		}
	} break;
	default: {
		LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_552), pSSkillBag->chType);
		return;
	}
	}

	lstSailSkill->Refresh();
	lstFightSkill->Refresh();
	lstLifeSkill->Refresh();
}

CSkillRecord* CEquipMgr::FindSkill(int nID) {
	for (auto& it : _skills) {
		if (it->nID == nID) {
			return it;
		}
	}
	return nullptr;
}

void CEquipMgr::_evtFastChange(CGuiData* pSender, CCommandObj* pItem, bool& isAccept) {
	auto* pGrid = dynamic_cast<CGoodsGrid*>(CDrag::GetParent());
	if (pGrid && pGrid != g_stUIEquip.GetGoodsGrid()) {
		return;
	}

	auto* pFast = dynamic_cast<CFastCommand*>(pSender);
	if (!pFast) {
		return;
	}

	isAccept = true;

	char chType = 0;
	short sGridID = 0;
	int nIndex = pFast->nTag;
	if (!g_stUIEquip._GetCommandShortCutType(pItem, chType, sGridID)) {
		isAccept = false;
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_553), pItem->GetName());
		return;
	}

	stNetShortCutChange param;
	memset(&param, 0, sizeof(param));
	param.chIndex = nIndex;
	if (pItem) {
		param.chType = chType;
		param.shyGrid = sGridID;
	}
	CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_SHORTCUT, (void*)&param);

	// 服务器接收到后无返回,客户端自己改变
	stNetShortCut& SCut = g_stUIEquip._stShortCut;
	SCut.chType[nIndex] = chType;
	SCut.byGridID[nIndex] = sGridID;
}

void CEquipMgr::_evtItemFormShow(CGuiData* pSender) {
	if (g_stUIStore.GetStoreForm()->GetIsShow()) {
		g_stUIEquip.frmItem->SetIsShow(false);
	}
}

void CEquipMgr::UpdataEquipData(const stNetChangeChaPart& SPart, CCharacter* pCha) {
	// ID不等于零时,更新数值
	for (size_t i = 0; i < cnmEquip.size(); i++) {
		if (cnmEquip[i] == nullptr || SPart.SLink[i].sID == 0) {
			continue;
		}
		// End

		auto pItem = dynamic_cast<CItemCommand*>(cnmEquip[i]->GetCommand());
		if (pItem && pItem->GetItemInfo()->lID == SPart.SLink[i].sID) {
			SItemGrid& Item = pItem->GetData();
			Item.bValid = SPart.SLink[i].bValid;
			Item.sEndure[0] = SPart.SLink[i].sEndure[0];
			Item.sEnergy[0] = SPart.SLink[i].sEnergy[0];
		} else {
			LG("error", RES_STRING(CL_LANGUAGE_MATCH_554), (pItem ? pItem->GetItemInfo()->lID : 0), SPart.SLink[i].sID);
		}
	}
}

void CEquipMgr::UpdataEquip(const stNetChangeChaPart& SPart, CCharacter* pCha) {
	// 默认装备不占装备栏
	stEquip = SPart;
	for (size_t i = 0; i < enumEQUIP_PART_NUM; i++) {
		if (stEquip.SLink[i].sID == pCha->GetDefaultChaInfo()->sSkinInfo[i]) {
			stEquip.SLink[i].sID = 0;
		}
	}
	//for( int i=enumEQUIP_PART_NUM; i<enumEQUIP_NUM; i++ )
	//{
	//    if( stEquip.SLink[i].sID==enumEQUIP_BOTH_HAND )
	//        stEquip.SLink[i].sID = 0;
	//}

	// 更新UI装备栏
	for (size_t i = 0; i < cnmEquip.size(); i++) {
		_UpdataEquip(stEquip.SLink[i], i);
	}
}

bool CEquipMgr::_UpdataEquip(SItemGrid& Item, int nLink) {
	if (!cnmEquip[nLink]) {
		return false;
	}

	const int nItemID = Item.sID;
	if (nItemID > 0 && nItemID != enumEQUIP_BOTH_HAND) {
		auto pItem = dynamic_cast<CItemCommand*>(cnmEquip[nLink]->GetCommand());
		if (pItem && pItem->GetItemInfo()->lID == nItemID) {
			pItem->SetData(Item);
			return true;
		}

		CItemRecord* pInfo = GetItemRecordInfo(nItemID);
		if (!pInfo) {
			LG("UpdataEquip", RES_STRING(CL_LANGUAGE_MATCH_555), nItemID);
			return false;
		}

		pItem = new CItemCommand(pInfo);
		pItem->SetData(Item);
		cnmEquip[nLink]->AddCommand(pItem);
	} else {
		cnmEquip[nLink]->DelCommand();
	}
	return true;
}

bool CEquipMgr::_GetCommandShortCutType(CCommandObj* pItem, char& chType, short& sGridID) {
	chType = 0;
	sGridID = 0;

	if (!pItem) {
		return true;
	}

	if (g_stUIEquip.GetGoodsGrid() && pItem->GetParent() == g_stUIEquip.GetGoodsGrid()) {
		chType = defItemShortCutType;
		sGridID = pItem->GetIndex();
	} else if (g_stUIEquip.lstFightSkill && g_stUIEquip.lstFightSkill == pItem->GetParent()) {
		auto pSkill = dynamic_cast<CSkillCommand*>(pItem);
		//NOTE: Wrong order of execution?
		//pSkill may be nullptr
		if (pSkill->GetSkillRecord()->chType == 2) {
			return false;
		}
		//NOTE: Check if pSkill is valid pointer
		if (pSkill) {
			chType = defSkillFightShortCutType;
			sGridID = pSkill->GetSkillID();
		}
	} else if (g_stUIEquip.lstLifeSkill && g_stUIEquip.lstLifeSkill == pItem->GetParent()) {
		auto pSkill = dynamic_cast<CSkillCommand*>(pItem);
		if (pSkill) {
			chType = defSkillLifeShortCutType;
			sGridID = pSkill->GetSkillID();
		}
	} else if (g_stUIEquip.lstSailSkill && g_stUIEquip.lstSailSkill == pItem->GetParent()) {
		auto pSkill = dynamic_cast<CSkillCommand*>(pItem);
		if (pSkill) {
			chType = defSkillSailShortCutType;
			sGridID = pSkill->GetSkillID();
		}
	}

	return true;
}

int CEquipMgr::RefreshServerShortCut() {

	stNetShortCut tmp{};
	for (size_t i = 0; i < SHORT_CUT_NUM; i++) {
		if (_pFastCommands[i]) {
			_GetCommandShortCutType(_pFastCommands[i]->GetCommand(), tmp.chType[i], tmp.byGridID[i]);
		}
	}

	int nCount = 0;
	stNetShortCutChange param;
	for (size_t i = 0; i < SHORT_CUT_NUM; i++) {
		if ((tmp.chType[i] != _stShortCut.chType[i]) || (tmp.byGridID[i] != _stShortCut.byGridID[i])) {
			param.chIndex = i;
			param.chType = tmp.chType[i];
			param.shyGrid = tmp.byGridID[i];
			CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_SHORTCUT, (void*)&param);
			nCount++;

			LG("shortcut", "Index:%d, Type:%d, GridID:%d\n", param.chIndex, param.chType, param.shyGrid);
		}
	}
	_stShortCut = tmp;
	LG("shortcut", "Total:%d\n\n", nCount);
	return nCount;
}

void CEquipMgr::UpdateShortCut(stNetShortCut& stShortCut) {
	_stShortCut = stShortCut;

	if (_pFastCommands) {
		for (size_t i = 0; i < SHORT_CUT_NUM; i++) {
			if (_pFastCommands[i]) {
				_pFastCommands[i]->DelCommand();
			}
		}
	}

	CGoodsGrid* const pGoodsGrid = GetGoodsGrid();
	for (size_t i = 0; i < SHORT_CUT_NUM; ++i) {
		if (_pFastCommands[i]) {
			switch (stShortCut.chType[i]) {
			case defItemShortCutType: {
				if (pGoodsGrid) {
					_pFastCommands[i]->AddCommand(pGoodsGrid->GetItem(stShortCut.byGridID[i]));
				}
			} break;
			case defSkillFightShortCutType: {
				if (lstFightSkill) {
					_pFastCommands[i]->AddCommand(lstFightSkill->FindSkill(stShortCut.byGridID[i]));
				}
			} break;
			case defSkillLifeShortCutType: {
				if (lstLifeSkill) {
					_pFastCommands[i]->AddCommand(lstLifeSkill->FindSkill(stShortCut.byGridID[i]));
				}
			} break;
			case defSkillSailShortCutType: {
				if (lstSailSkill) {
					_pFastCommands[i]->AddCommand(lstSailSkill->FindSkill(stShortCut.byGridID[i]));
				}
			} break;
			default: {
				LG("UpdateShortCut", "unknown type: %d, ", stShortCut.chType[i]);
			} break;
			}
		}
	}
}

void CEquipMgr::DelFastCommand(CCommandObj* pObj) {
	if (pObj && pObj->GetIsFast()) {
		CFastCommand* pFast = CFastCommand::FindFastCommand(pObj);
		if (pFast) {
			int nIndex = pFast->nTag;
			stNetShortCutChange param;
			memset(&param, 0, sizeof(param));
			param.chIndex = nIndex;
			CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_SHORTCUT, (void*)&param);

			// 服务器接收到后无返回,客户端自己改变
			stNetShortCut& SCut = _stShortCut;
			SCut.chType[nIndex] = 0;
			SCut.byGridID[nIndex] = 0;
		}
	}
}

void CEquipMgr::_evtButtonClickEvent(CGuiData* pSender, int x, int y, MouseClickState key) {
	std::string name = pSender->GetName();
	if (name == "btnFastUp") {
		g_stUIEquip._ActiveFast(g_stUIEquip._nFastCur - 1);
		return;
	} else if (name == "btnFastDown") {
		g_stUIEquip._ActiveFast(g_stUIEquip._nFastCur + 1);
		return;
	}
}

void CEquipMgr::_evtEquipEvent(CGuiData* pSender, CCommandObj* pItem, bool& isAccept) {
	isAccept = false;

	auto pGood = dynamic_cast<CGoodsGrid*>(CDrag::GetParent());
	if (pGood != g_stUIEquip.GetGoodsGrid()) {
		return;
	}

	auto pItemCommand = dynamic_cast<CItemCommand*>(pItem);
	if (!pItemCommand) {
		return;
	}

	CGameScene* pScene = g_pGameApp->GetCurScene();
	if (!pScene) {
		return;
	}

	CCharacter* pCha = pScene->GetMainCha();
	if (!pCha) {
		return;
	}

	auto pCom = dynamic_cast<COneCommand*>(pSender);
	if (!pCom) {
		return;
	}

	if (pCom->nTag == 999) {
		// 直接丢弃
		g_stUIEquip._nLostGridID = g_stUIEquip.grdItem->GetDragIndex();
		g_stUIBox.ShowSelectBox(_evtLostYesNoEvent, RES_STRING(CL_LANGUAGE_MATCH_556), true);
	} else {
		//Add by sunny.sun 20080902
		const CItemRecord* _pItem = pItemCommand->GetItemInfo();
		if (_pItem->sType == EItemType::Moneybag || _pItem->sType == EItemType::Ammo ||
			_pItem->sType == EItemType::Medicine || _pItem->sType == EItemType::Scroll ||
			_pItem->sType == EItemType::General || _pItem->sType == EItemType::Mission ||
			_pItem->sType == EItemType::GemCombineScroll || _pItem->sType == 48 ||
			_pItem->sType == EItemType::PetFodder || _pItem->sType == EItemType::PetSock ||
			_pItem->sType == 65) {
			g_pGameApp->MsgBox("The prop is used in the wrong way");
			return;
		}
		//End
		stNetUseItem info;
		info.sGridID = g_stUIEquip.grdItem->GetDragIndex();
		CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_ITEM_USE, &info);
	}
}

void CEquipMgr::_evtLostYesNoEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes) {
		return;
	}

	stNetDelItem info;
	info.sGridID = g_stUIEquip._nLostGridID;
	CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_ITEM_DELETE, &info);
}

void CEquipMgr::_evtThrowEquipEvent(CGuiData* pSender, CCommandObj* pItem, bool& isThrow) {
	isThrow = false;

	CGameScene* pScene = CGameApp::GetCurScene();
	if (!pScene) {
		return;
	}

	CCharacter* pMain = CGameScene::GetMainCha();
	if (!pMain) {
		return;
	}

	auto pCom = dynamic_cast<COneCommand*>(CDrag::GetParent());
	if (pCom && pCom->nTag > 0) {
		int x = (int)(pScene->GetMouseMapX() * 100.0f);
		int y = (int)(pScene->GetMouseMapY() * 100.0f);

		if (!g_stUIEquip._GetThrowPos(x, y)) {
			return;
		}

		stUnfix& unfix = g_stUIEquip._sUnfix;
		unfix.Reset();

		unfix.nGridID = -2;
		unfix.nLink = pCom->nTag;
		unfix.nX = x;
		unfix.nY = y;
		unfix.pItem = pItem;
		g_stUIEquip._StartUnfix(unfix);
	}
}

//Based on Fastpanel tricks guide (Hex modifications) by Duduf
//https://pkodev.net/index.php?/topic/1082-fastpanel-tricks/
bool CEquipMgr::ExecFastKey(int key) {
	//if( key==VK_OEM_4 && ::GetAsyncKeyState(VK_CONTROL) )
	if (key == VK_TAB) //As in Fastpanel tricks guide #4 (to enable TAB to switch panels)
	{
		_ActiveFast(_nFastCur - 1);
		return true;
	}
	//if( key==VK_F12 || ( key==VK_OEM_6 && ::GetAsyncKeyState(VK_CONTROL) ) )
	if (key == VK_TAB && ::GetAsyncKeyState(VK_SHIFT)) //Made Shift+Tab switch panels in opposite way.
	{
		_ActiveFast(_nFastCur + 1);
		return true;
	}

	if (!CFormMgr::s_Mgr.GetEnableHotKey())
		return false; // 禁用热键

	//Runs the first command from all matched slots with same keybind
	for (auto iSlot = _nFastCur * MAX_FAST_COL; iSlot < ((_nFastCur + 1) * MAX_FAST_COL) - 1; iSlot++) {
		if (_pFastCommands[iSlot] && _pFastCommands[iSlot]->GetCommand() && (_pFastCommands[iSlot]->GetKeybind() == key)) {
			_pFastCommands[iSlot]->GetCommand()->Exec();
			return true;
		}
	}
	return false;
}

void CEquipMgr::_ActiveFast(int num) {
	if (num >= (int)MAX_FAST_ROW) {
		num = 0;
	}

	if (num < 0) {
		num = MAX_FAST_ROW - 1;
	}

	int count = (int)SHORT_CUT_NUM;
	for (size_t i = 0; i < count; i++) {
		if (_pFastCommands[i]) {
			_pFastCommands[i]->SetIsShow(false);
		}
	}

	count = (num + 1) * MAX_FAST_COL;
	for (size_t i = num * MAX_FAST_COL; i < count; i++) {
		if (_pFastCommands[i]) {
			_pFastCommands[i]->SetIsShow(true);
		}
	}
	_nFastCur = num;

	if (_pActiveFastLabel) {
		_snprintf_s(szBuf, _TRUNCATE, "%d", _nFastCur + 1);
		_pActiveFastLabel->SetCaption(szBuf);
	}
}

void CEquipMgr::evtSwapItemEvent(CGuiData* pSender, int nFirst, int nSecond, bool& isSwap) {
	isSwap = false;
	auto pGood = dynamic_cast<CGoodsGrid*>(CDrag::GetParent());

	CCharacter* pSelf = g_stUIBoat.FindCha(pGood);
	if (!pSelf) {
		return;
	}

	if (pSelf->IsBoat() && pSelf != CGameScene::GetMainCha()) {
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_557));
		return;
	}

	auto pItem = dynamic_cast<CItemCommand*>(pGood->GetItem(nSecond));
	auto pTarget = dynamic_cast<CItemCommand*>(pGood->GetItem(nFirst));
	if (pItem && !pItem->GetIsValid()) {
		return;
	}
	if (pTarget && !pTarget->GetIsValid()) {
		return;
	}

	if (g_pGameApp->IsShiftPress()) {
		if (pItem && pItem->GetTotalNum() > 1) {
			if (!pTarget || pTarget->GetItemInfo()->nID == pItem->GetItemInfo()->nID) {
				SSplit.nFirst = nFirst;
				SSplit.nSecond = nSecond;
				SSplit.pSelf = pSelf;
				CBoxMgr::ShowNumberBox(SSplitItem::_evtSplitItemEvent, pItem->GetTotalNum());
				return;
			}
		}
	}

	stNetItemPos info;
	info.sSrcGridID = nSecond;
	info.sSrcNum = 0;
	info.sTarGridID = nFirst;
	CS_BeginAction(pSelf, enumACTION_ITEM_POS, (void*)&info);
}

void CEquipMgr::SSplitItem::_evtSplitItemEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes) {
		return;
	}

	auto pBox = static_cast<stNumBox*>(pSender->GetForm()->GetPointer());
	if (!pBox) {
		return;
	}

	stNetItemPos info;
	info.sSrcGridID = SSplit.nSecond;
	info.sSrcNum = pBox->GetNumber();
	info.sTarGridID = SSplit.nFirst;
	CS_BeginAction(SSplit.pSelf, enumACTION_ITEM_POS, (void*)&info);
}

bool CEquipMgr::_GetThrowPos(int& x, int& y) {
	CCharacter* pCha = CGameScene::GetMainCha();
	if (!pCha) {
		return false;
	}

	GetDistancePos(pCha->GetCurX(), pCha->GetCurY(), x, y, 100, x, y);
	if (CGameApp::GetCurScene()->GetIsBlockWalk(pCha, x, y)) {
		x = pCha->GetCurX();
		y = pCha->GetCurY();
	}
	return true;
}

void CEquipMgr::evtThrowItemEvent(CGuiData* pSender, int id, bool& isThrow) {
	isThrow = false;

	if (!CGameApp::GetCurScene()) {
		return;
	}

	CCharacter* pMain = CGameScene::GetMainCha();
	if (!pMain) {
		return;
	}

	auto pGood = dynamic_cast<CGoodsGrid*>(CDrag::GetParent());

	CCommandObj* obj = pGood->GetItem(id);

	CCharacter* pSelf = g_stUIBoat.FindCha(pGood);
	if (!pSelf) {
		return;
	}

	// add by Philip.Wu  2006-07-05
	// 判定道具是否合法，防止道具被锁后丢弃再捡起又可用的 BUG
	if (!obj->GetIsValid()) {
		return;
	}

	int x = (int)(CGameApp::GetCurScene()->GetMouseMapX() * 100.0f);
	int y = (int)(CGameApp::GetCurScene()->GetMouseMapY() * 100.0f);

	if (!g_stUIEquip._GetThrowPos(x, y))
		return;

	stThrow& sthrow = g_stUIEquip._sThrow;
	sthrow.nX = x;
	sthrow.nY = y;
	sthrow.nGridID = id;
	sthrow.pSelf = pSelf;

	//如果是船长证明
	auto pItem = dynamic_cast<CItemCommand*>(obj);
	if (pItem && pItem->GetItemInfo()->sType == EItemType::Boat) {
		stSelectBox* pBox = g_stUIBox.ShowSelectBox(_evtThrowBoatDialogEvent,
													RES_STRING(CL_LANGUAGE_MATCH_558),
													true);
		if (pBox) {
			pBox->pointer = &sthrow;
			return;
		}
	}

	if (obj->GetIsPile() && obj->GetTotalNum() > 1) {
		stNumBox* pBox = g_stUIBox.ShowNumberBox(_evtThrowDialogEvent, obj->GetTotalNum());
		if (pBox) {
			pBox->pointer = &sthrow;
			return;
		}
	}

	g_stUIEquip._SendThrowData(sthrow);
}

void CEquipMgr::_evtThrowDialogEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes) {
		return;
	}

	auto pBox = static_cast<stNumBox*>(pSender->GetForm()->GetPointer());
	if (!pBox) {
		return;
	}

	auto p = static_cast<stThrow*>(pBox->pointer);
	if (!p) {
		return;
	}

	g_stUIEquip._SendThrowData(*p, pBox->GetNumber());
}

void CEquipMgr::_evtThrowBoatDialogEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes) {
		return;
	}

	auto pBox = static_cast<stSelectBox*>(pSender->GetForm()->GetPointer());
	if (!pBox) {
		return;
	}

	auto p = static_cast<stThrow*>(pBox->pointer);
	if (!p) {
		return;
	}

	g_stUIEquip._SendThrowData(*p, 1);
}

void CEquipMgr::_SendThrowData(const stThrow& sthrow, int nThrowNum) {
	if (sthrow.pSelf->IsBoat() && sthrow.pSelf != CGameScene::GetMainCha()) {
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_557));
		return;
	}

	stNetItemThrow item;
	item.lNum = nThrowNum;
	item.sGridID = sthrow.nGridID;
	item.lPosX = sthrow.nX;
	item.lPosY = sthrow.nY;
	CS_BeginAction(sthrow.pSelf, enumACTION_ITEM_THROW, (void*)&item);
}

void CEquipMgr::_evtSkillFormShow(CGuiData* pSender) {
	g_stUIEquip.RefreshUpgrade();
}

void CEquipMgr::FrameMove(DWORD dwTime) {
	static CTimeWork time(100);
	if (time.IsTimeOut(dwTime)) {
		CCharacter* pCha = g_stUIBoat.GetHuman();
		if (!pCha) {
			return;
		}

		SGameAttr* pGameAttr = pCha->getGameAttr();
		if (frmSkill->GetIsShow()) {
			// 显示剩余技能点数
			_snprintf_s(szBuf, _TRUNCATE, "%lld", pGameAttr->get(ATTR_TP));
			labPoint->SetCaption(szBuf);

			_snprintf_s(szBuf, _TRUNCATE, "%lld", pGameAttr->get(ATTR_LIFETP));
			labPointLife->SetCaption(szBuf);

			RefreshUpgrade();
		}

		CCharacter* pMainCha = CGameScene::GetMainCha();
		if (pMainCha) {
			// 刷新技能激活框
			CSkillCommand::GetActiveImage()->Next();

			CChaStateMgr* pState = pMainCha->GetStateMgr();

			for (auto& it : _cancels) {
				it->SetIsActive(pState->HasSkillState(it->nStateID));
			}

			// 刷新充电状态,仅显示时才有必要刷新
			if (frmItem->GetIsShow()) {
				// 显示或隐藏道具栏前四格图案
				imgItem4->SetIsShow(grdItem->GetFirstShow() == 0);

				_snprintf_s(szBuf, _TRUNCATE, "%s", StringSplitNum((long)pGameAttr->get(ATTR_GD)));
				lblGold->SetCaption(szBuf);

				for (auto& imgCharge : _imgCharges) {
					imgCharge.Next();
				}

				// Thunder Coral Recharging in thunderstorm
				for (auto& it : _charges) {
					COneCommand* pCmd = cnmEquip[it->sChargeLink];
					if (pCmd) {
						auto pItem = dynamic_cast<CItemCommand*>(pCmd->GetCommand());
						if (pItem && pItem->GetItemInfo()->sType == EItemType::Conch) {
							pCmd->SetIsShowActive(pState->HasSkillState(it->chID));
						} else {
							pCmd->SetIsShowActive(false);
						}
					}
				}
			}
		}
	}
}

void CEquipMgr::RefreshUpgrade() {
	CForm* f = frmSkill;
	if (!f->GetIsShow()) {
		return;
	}

	CCharacter* pCha = CGameScene::GetMainCha();
	if (!pCha) {
		return;
	}

	SGameAttr* pAttr = pCha->getGameAttr();

	lstFightSkill->SetIsShowUpgrade(pAttr->get(ATTR_TP) > 0);
	lstLifeSkill->SetIsShowUpgrade(pAttr->get(ATTR_LIFETP) > 0);

	RefreshSkillJob((int)pAttr->get(ATTR_JOB));
}

void CEquipMgr::RefreshSkillJob(int nJob) {
	const int count = lstFightSkill->GetCount();
	for (int j = 0; j < 2; j++) {
		CSkillList* pList = GetSkillList(j);
		for (int i = 0; i < count; i++) {
			CSkillCommand* tmp = pList->GetCommand(i);
			if (tmp) {
				tmp->GetSkillRecord()->Refresh(nJob);
			}
		}
	}
}

void CEquipMgr::UnfixToGrid(CCommandObj* pItem, int nGridID, int nLink) {
	_sUnfix.Reset();

	_sUnfix.nLink = nLink;
	_sUnfix.nGridID = nGridID;
	_sUnfix.pItem = pItem;

	_StartUnfix(_sUnfix);
}

void CEquipMgr::_evtItemFormClose(CForm* pForm, bool& IsClose) {
	if (g_stUITrade.IsTrading())
		IsClose = false;
}

void CEquipMgr::_SendUnfixData(const stUnfix& unfix, int nUnfixNum) {
	if (!CGameScene::GetMainCha()) {
		return;
	}

	if (unfix.nLink == -1) {
		return;
	}

	stNetItemUnfix item;
	item.chLinkID = unfix.nLink;
	item.sGridID = unfix.nGridID;
	item.lPosX = unfix.nX;
	item.lPosY = unfix.nY;

	if (g_stUIBoat.GetHuman() == CGameScene::GetMainCha()) {
		CActor* pActor = g_stUIBoat.GetHuman()->GetActor();
		auto pState = std::make_unique<CEquipState>(pActor);
		pState->SetUnfixData(item);
		pActor->SwitchState(std::move(pState));
	} else {
		CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_ITEM_UNFIX, (void*)&item);
	}
}

void CEquipMgr::_evtUnfixDialogEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes) {
		return;
	}

	auto pBox = static_cast<stNumBox*>(pSender->GetForm()->GetPointer());
	if (!pBox) {
		return;
	}

	auto p = static_cast<stUnfix*>(pBox->pointer);
	if (!p) {
		return;
	}

	g_stUIEquip._SendUnfixData(*p, pBox->GetNumber());
}

void CEquipMgr::_StartUnfix(stUnfix& unfix) {
	CCommandObj* pItem = unfix.pItem;
	if (!pItem) {
		return;
	}

	if (unfix.nGridID == -2) {
		//if( !pItem->IsAllowThrow() )
		//{
		//    g_pGameApp->SysInfo( "特殊物品不能丢弃" );
		//    return;
		//}

		if (!_GetThrowPos(unfix.nX, unfix.nY)) {
			return;
		}
	}

	if (pItem->GetIsPile() && pItem->GetTotalNum() > 1) {
		stNumBox* pBox = g_stUIBox.ShowNumberBox(_evtUnfixDialogEvent, pItem->GetTotalNum());
		if (pBox) {
			pBox->pointer = &unfix;
			return;
		}
	}

	_SendUnfixData(unfix);
}

CItemCommand* CEquipMgr::GetEquipItem(unsigned int nLink) {
	if (nLink < cnmEquip.size() && cnmEquip[nLink]) {
		return dynamic_cast<CItemCommand*>(cnmEquip[nLink]->GetCommand());
	}
	return nullptr;
}

bool CEquipMgr::IsEquipCom(COneCommand* pCom) const {
	return frmItem == pCom->GetForm();
}

CGuiPic* CEquipMgr::GetChargePic(unsigned int n) {
	if (n < enumEQUIP_NUM) {
		return &_imgCharges[n];
	}
	return nullptr;
}

void CEquipMgr::_evtRMouseGridEvent(CGuiData* pSender, CCommandObj* pItem, int nGridID) {
	if (!g_stUIBoat.GetHuman()) {
		return;
	}

	auto pItemCommand = dynamic_cast<CItemCommand*>(pItem);
	if (!pItemCommand || pItemCommand->GetItemInfo()->sType != EItemType::Boat) {
		return;
	}

	stNetItemInfo info;
	info.chType = mission::VIEW_CHAR_BAG;
	info.sGridID = nGridID;
	CS_BeginAction(g_stUIBoat.GetHuman(), enumACTION_ITEM_INFO, &info);
}

void CEquipMgr::_evtRepairEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	CS_ItemRepairAnswer(nMsgType == CForm::mrYes);
}

void CEquipMgr::ShowRepairMsg(const char* pItemName, long lMoney) {
	char szBuf[255] = {0};
	_snprintf_s(szBuf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_559), pItemName, lMoney);
	g_stUIBox.ShowSelectBox(_evtRepairEvent, szBuf, true);
}

void CEquipMgr::CloseAllForm() {
	if (frmItem && frmItem->GetIsShow()) {
		frmItem->SetIsShow(false);
	}

	if (frmSkill && frmSkill->GetIsShow()) {
		frmSkill->SetIsShow(false);
	}
}

// 获得道具在当前背包中总的个数
int CEquipMgr::GetItemCount(int nID) {
	int nRet = 0;
	for (size_t i = 0; i < grdItem->GetMaxNum(); ++i) {
		auto pItem = dynamic_cast<CItemCommand*>(grdItem->GetItem(i));
		if (pItem && pItem->GetItemInfo()) {
			if (pItem->GetItemInfo()->lID == nID) {
				nRet += pItem->GetTotalNum();
			}
		}
	}
	return nRet;
}

///////////////////////////////////////////////////////////////////////////////////
//
//  二次密码相关
//

// 锁定图片鼠标点击事件
void CEquipMgr::_evtItemFormMouseEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	const std::string strName = pSender->GetName();

	if (strName == "btnLock") {
		if (g_stUIEquip.GetIsLock()) { // 解锁
			g_stUIDoublePwd.SetType(CDoublePwdMgr::PACKAGE_UNLOCK);
			g_stUIDoublePwd.ShowDoublePwdForm();
		} else { // 上锁
			CGoodsGrid* pGrid = g_stUIEquip.GetGoodsGrid();
			const int nCount = pGrid->GetMaxNum();
			for (size_t i = 0; i < nCount; ++i) {
				auto pCmd = dynamic_cast<CItemCommand*>(pGrid->GetItem(i));
				if (pCmd) {
					CItemRecord* pItemRecord = pCmd->GetItemInfo();
					if (pItemRecord && pItemRecord->lID == 2440) {
						CBoxMgr::ShowSelectBox(_CheckLockMouseEvent, RES_STRING(CL_LANGUAGE_MATCH_824), true); //确认锁定背包

						//::CS_LockKitbag();
						return;
					}
				}
			}

			g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_825)); //背包内没有密码锁
		}
	}
}

void CEquipMgr::SetIsLock(bool bLock) {
	imgLock->SetIsShow(bLock);
	imgUnLock->SetIsShow(!bLock);
}

bool CEquipMgr::GetIsLock() const {
	return imgLock->GetIsShow();
}

// 加锁 MSGBOX 确认
void CEquipMgr::_CheckLockMouseEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType == CForm::mrYes) {
		CS_LockKitbag();
	}
}
