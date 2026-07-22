#include "StdAfx.h"
#include "UIChat.h"
#include "UITeam.h"
#include "uiformmgr.h"
#include "uitreeview.h"
#include "uiprogressbar.h"
#include "uilabel.h"
#include "uiboatform.h"
#include "smallmap.h"
#include "ui3dcompent.h"
#include "GameApp.h"

#include "Character.h"
#include "PacketCmd.h"
#include "UICheckBox.h"
#include "UIEdit.h"
#include "NetChat.h"
#include "UIMenu.h"
#include "talksessionformmgr.h"
#include "uiboxform.h"
#include "worldscene.h"
#include "GuildMemberData.h"
#include "GuildMembersMgr.h"
#include "UIStartForm.h"
#include "GuildData.h"
#include "autoattack.h"
#include "UICozeForm.h"

#include "UIAllButtonForm.h"
#include "UIChaInfo.h"

using namespace GUI;

int SMALL_ICON_SIZE = 16;
int BIG_ICON_SIZE = 32;

CTeamMgr* CChat::_pTeamMgr = nullptr;
stPersonInfo CChat::_SelfInfo;
CForm* CChat::_frmQQ = nullptr;

//	Add by alfred	begin
CForm* CChat::frmjieguo2 = nullptr;
CForm* CChat::frmchainfo2 = nullptr;
//	End.

CForm* CChat::_frmAddFriend = nullptr;

// Add by lark.li 20080804 begin
CForm* CChat::_frmAddFrndGroup = nullptr;
CForm* CChat::_frmEditFrndGroup = nullptr;
// End

CForm* CChat::_frmChangeSytle = nullptr;
CForm* CChat::_frmEditMotto = nullptr;
//CForm* CChat::_frmDetails=NULL;
CList* CChat::_lstFrndPlayer = nullptr;
CList* CChat::_lstGroupPlayer = nullptr;
CList* CChat::_lstRoadPlayer = nullptr;
CList* CChat::_lstStyle = nullptr;
CMember* CChat::_curSelectMember = nullptr;
CEdit* CChat::_pEditFrndName = nullptr;

// Add by lark.li 20080804 begin
CEdit* CChat::_pAddFrndGroupName = nullptr;
CEdit* CChat::_pEditFrndGroupName = nullptr;

CTreeGridNode* CChat::_curSelectNode = nullptr;
// End

CEdit* CChat::_pEditMotto = nullptr;
DWORD CChat::_dwSelfID = 0;
DWORD CChat::_dwSelfIcon = 0;
std::string CChat::_strSelfMottoName = "";

CTreeGridNode* CChat::_pGuildNode = nullptr;
CTreeGridNode* CChat::_pSessionNode = nullptr;
CTreeGridNode* CChat::_pMasterNode = nullptr;
CTreeGridNode* CChat::_pPrenticeNode = nullptr;

// Add by lark.li 20080804 begin
CMenu* CChat::_frndGroupMouseRight = nullptr;
CMenu* CChat::_hitNothingMouseRight = nullptr;
// End

CMenu* CChat::_frndMouseRight = nullptr;
CMenu* CChat::_groupMouseRight = nullptr;
CMenu* CChat::_roadMouseRight = nullptr;
CMenu* CChat::_sessMouseRight = nullptr;
CMenu* CChat::_MasterMouseRight = nullptr;
CMenu* CChat::_StudentMouseRight = nullptr;
CMenu* CChat::_styMenu = nullptr;

CLabelEx* CChat::_labNameOfMottoFrm = nullptr;
CImage* CChat::_imgFaceOfMottoFrm = nullptr;
CCheckBox* CChat::_chkForbidOfMottoFrm = nullptr;
std::vector<char*> CChat::_strFilterTxt;

bool CChat::_bForbid = false;

CCharacter2D* CChat::_pCharacter[MAX_MEMBER] = {nullptr};
CLabelEx* CChat::labMenberName[MAX_MEMBER] = {nullptr};
CForm* CChat::frmTeamMenber[MAX_MEMBER] = {nullptr};
CProgressBar* CChat::proTeamMenberHP[MAX_MEMBER] = {nullptr};
CProgressBar* CChat::proTeamMenberSP[MAX_MEMBER] = {nullptr};
CLabelEx* CChat::labLv[MAX_MEMBER] = {nullptr};
CImage* CChat::imgWork[MAX_MEMBER] = {nullptr};
CImage* CChat::imgLeader[MAX_MEMBER] = {nullptr};

//---------------------------------------------------------------------------
// class CChat
//---------------------------------------------------------------------------

CChat::CChat()
	: m_pQQTreeView(nullptr), _pGroupNode(nullptr), _pFrndNode(nullptr), _pDropTreeNode(nullptr) {
	frmChatManage = nullptr;
}

bool CChat::Init() {
	//ifstream filterTxt("scripts\\table\\filter.txt",ios::in);
	//if (filterTxt.is_open())
	//{
	//	char buf[500]={0};
	//	filterTxt.getline(buf,500);
	//	while (!filterTxt.fail())
	//	{
	//		char *pText=new char[strlen(buf)+2];
	//		strncpy_s(pText,sizeof(pText),buf, _TRUNCATE);
	//		_strFilterTxt.push_back(pText);
	//		filterTxt.getline(buf,500);
	//	}
	//	filterTxt.close();
	//}
	CFormMgr::s_Mgr.AddHotKeyEvent(CTalkSessionFormMgr::OnHotKeyShow);

	_pTeamMgr = new CTeamMgr;

	CFormMgr& mgr = CFormMgr::s_Mgr;

	// 初始化组队控件
	char szBuf[80] = {0};
	for (int i = 0; i < MAX_MEMBER; i++) {
		_snprintf_s(szBuf, _TRUNCATE, "frmTeamMenber%d", i + 1);
		frmTeamMenber[i] = mgr.Find(szBuf);
		if (!frmTeamMenber[i])
			return false;
		frmTeamMenber[i]->SetIsShow(false);
		frmTeamMenber[i]->Refresh();

		_snprintf_s(szBuf, _TRUNCATE, "proTeamMenber%dHP", i + 1);
		proTeamMenberHP[i] = dynamic_cast<CProgressBar*>(frmTeamMenber[i]->Find(szBuf));
		if (!proTeamMenberHP[i]) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), szBuf);
		}
		proTeamMenberHP[i]->SetRange(0.0f, 1.0f);

		_snprintf_s(szBuf, _TRUNCATE, "proTeamMenber%dSP", i + 1);
		proTeamMenberSP[i] = dynamic_cast<CProgressBar*>(frmTeamMenber[i]->Find(szBuf));
		if (!proTeamMenberSP[i]) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), szBuf);
		}
		proTeamMenberSP[i]->SetRange(0.0f, 1.0f);

		_snprintf_s(szBuf, _TRUNCATE, "labMenber%dName", i + 1);
		labMenberName[i] = dynamic_cast<CLabelEx*>(frmTeamMenber[i]->Find(szBuf));
		if (!labMenberName[i]) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), szBuf);
		}

		labLv[i] = dynamic_cast<CLabelEx*>(frmTeamMenber[i]->Find("labFLv"));
		if (!labLv[i]) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), "labFLv");
		}

		imgWork[i] = dynamic_cast<CImage*>(frmTeamMenber[i]->Find("imgobj"));
		if (!imgWork[i]) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), "imgobj");
		}

		imgLeader[i] = dynamic_cast<CImage*>(frmTeamMenber[i]->Find("imgLeader"));
		if (!imgLeader[i]) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), "imgLeader");
		}

		auto p3DDown = dynamic_cast<C3DCompent*>(frmTeamMenber[i]->Find("d3dDown"));
		if (!p3DDown) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), "d3dDown");
		}
		p3DDown->nTag = i;
		p3DDown->evtMouseDown = _MemberMouseDownEvent;
		p3DDown->SetMouseAction(eMouseAction::Skill);

		auto p3D = dynamic_cast<C3DCompent*>(frmTeamMenber[i]->Find("d3dHead"));
		if (!p3D) {
			return _Error(RES_STRING(CL_LANGUAGE_MATCH_45), frmTeamMenber[i]->GetName(), "d3dHead");
		}
		p3D->SetRenderEvent(_RenderEvent);
		p3D->nTag = i;

		RECT rt;
		rt.left = p3D->GetX();
		rt.right = p3D->GetX2();
		rt.top = p3D->GetY();
		rt.bottom = p3D->GetY2();

		_pCharacter[i] = new CCharacter2D;
		_pCharacter[i]->Create(rt);
	}

	// 初始化QQ界面
	_frmQQ = mgr.Find("frmQQ");
	if (!_frmQQ) {
		::Error(RES_STRING(CL_LANGUAGE_MATCH_412));
		return false;
	}

	_frmQQ->SetIsShow(true);
	_frmQQ->evtShow = _evtQQMainShow;
	CGuiTime::Create(300, _OnTimerFlash);

	//	初始化	Add by alfred.shi 20080903	begin
	frmjieguo2 = mgr.Find("frmjieguo2");
	if (!frmjieguo2) {
		return false;
	}

	frmjieguo2->SetIsShow(true);

	frmchainfo2 = mgr.Find("frmchainfo2");
	if (!frmchainfo2) {
		return false;
	}

	frmchainfo2->SetIsShow(true);
	//	End.

	auto pbtnAdd = dynamic_cast<CTextButton*>(_frmQQ->Find("btnAdd"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "btnAdd");
	}
	pbtnAdd->evtMouseClick = _evtQQMainAddFrnd;

	pbtnAdd = dynamic_cast<CTextButton*>(_frmQQ->Find("btnEdit"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "btnEdit");
	}
	pbtnAdd->evtMouseClick = _evtQQMainEditMotto;

	pbtnAdd = dynamic_cast<CTextButton*>(_frmQQ->Find("btnscr"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "btnscr");
	}
	pbtnAdd->evtMouseClick = _evtQQMainEditMotto;

	//	Add by alfred.shi 20080903 begin
	auto btnAddFriend = dynamic_cast<CTextButton*>(frmjieguo2->Find("btnAddFriend"));
	if (!btnAddFriend) {
		return false;
	}
	btnAddFriend->evtMouseClick = _evtAddFriend;

	auto btnViewFriend = dynamic_cast<CTextButton*>(frmjieguo2->Find("btnViewFriend"));
	if (!btnViewFriend) {
		return false;
	}
	btnViewFriend->evtMouseClick = _evtViewFriend;

	auto btnInfo = dynamic_cast<CTextButton*>(frmchainfo2->Find("btnInfo"));
	if (!btnInfo) {
		return false;
	}
	btnInfo->evtMouseClick = _evtViewDetailInfo;

	CForm* frmDetail = _FindForm("frmDetail");
	if (!frmDetail) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "frmDetail");
	}
	auto btnshow = dynamic_cast<CTextButton*>(frmDetail->Find("btnshow"));
	//if(btnshow)	btnshow->SetFlashCycle();
	btnshow->evtMouseClick = _evtGostMainEditMotto;
	//	End

	m_pQQTreeView = dynamic_cast<CTreeView*>(_frmQQ->Find("trvEditor"));
	if (!m_pQQTreeView) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "trvEditor");
	}

	auto pMineName = dynamic_cast<CLabelEx*>(_frmQQ->Find("labMineName"));
	if (!pMineName) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "labMineName");
	}

	// Add by lark.li 20080804 begin
	_frndGroupMouseRight = CMenu::FindMenu("frndGroupRight");
	if (!_frndGroupMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "frndGroupRight");
	}
	_frndGroupMouseRight->evtListMouseDown = _OnMouseFrndGroupMenu;

	_hitNothingMouseRight = CMenu::FindMenu("hitNothingRight");
	if (!_frndGroupMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "frndGroupRight");
	}
	_hitNothingMouseRight->evtListMouseDown = _OnMouseAddFrndGroupMenu;
	// End

	_frndMouseRight = CMenu::FindMenu("frndMouseRight");
	if (!_frndMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "frndMouseRight");
	}
	_frndMouseRight->evtListMouseDown = _OnMouseFrndMenu;

	_groupMouseRight = CMenu::FindMenu("groupMouseRight");
	if (!_groupMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "groupMouseRight");
	}
	_groupMouseRight->evtListMouseDown = _OnMouseGroupMenu;

	_roadMouseRight = CMenu::FindMenu("roadMouseRight");
	if (!_roadMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "roadMouseRight");
	}
	_roadMouseRight->evtListMouseDown = _OnMouseRoadMenu;

	_sessMouseRight = CMenu::FindMenu("ChatMouseRight");
	if (!_sessMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "ChatMouseRight");
	}
	_sessMouseRight->evtListMouseDown = _OnMouseSessMenu;

	_MasterMouseRight = CMenu::FindMenu("MasterRight");
	if (!_MasterMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "MasterRight");
	}
	_MasterMouseRight->evtListMouseDown = _OnMouseMasterMenu;

	_StudentMouseRight = CMenu::FindMenu("StudentRight");
	if (!_StudentMouseRight) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "StudentRight");
	}
	_StudentMouseRight->evtListMouseDown = _OnMouseStudentMenu;

	m_pQQTreeView->evtMouseDown = _MainMouseClick;
	m_pQQTreeView->evtMouseDB = _MainMouseClick;
	m_pQQTreeView->evtMouseDragEnd = _OnDragEnd;

	// Add by lark.li 20080805 begin
	m_pQQTreeView->evtMouseDragMove = _OnDragMouseMove;
	// End

	m_pQQTreeView->SetSelectColor(0);
	int nWidth = m_pQQTreeView->GetWidth() - m_pQQTreeView->GetScroll()->GetWidth() - 28;
	int nHeight = SMALL_ICON_SIZE + 3;

	_pSessionNode = dynamic_cast<GUI::CTreeGridNode*>(m_pQQTreeView->GetRootNode()->FindNode(RES_STRING(CL_LANGUAGE_MATCH_465)));
	_pSessionNode->GetItem()->SetColor(COLOR_BLACK);
	_pSessionNode->GetUpImage()->UnLoadImage();
	_pSessionNode->GetDownImage()->UnLoadImage();
	_pSessionNode->SetUnitSize(nWidth, nHeight);
	_pSessionNode->SetColMaxNum(1);

	_pFrndNode = dynamic_cast<GUI::CTreeGridNode*>(m_pQQTreeView->GetRootNode()->FindNode(RES_STRING(CL_LANGUAGE_MATCH_466)));
	_pFrndNode->GetItem()->SetColor(COLOR_BLACK);
	_pFrndNode->GetUpImage()->UnLoadImage();
	_pFrndNode->GetDownImage()->UnLoadImage();
	_pFrndNode->SetUnitSize(nWidth, nHeight);
	_pFrndNode->SetColMaxNum(1);

	// Add by lark.li 20080802 begin
	_pFrndNodeAll = new CTreeGridNode(m_pQQTreeView, nullptr);
	// End

	_pGroupNode = dynamic_cast<GUI::CTreeGridNode*>(m_pQQTreeView->GetRootNode()->FindNode(RES_STRING(CL_LANGUAGE_MATCH_467)));
	_pGroupNode->GetItem()->SetColor(COLOR_BLACK);
	_pGroupNode->GetUpImage()->UnLoadImage();
	_pGroupNode->GetDownImage()->UnLoadImage();
	_pGroupNode->SetUnitSize(nWidth, nHeight);
	_pGroupNode->SetColMaxNum(1);

	_pGuildNode = dynamic_cast<GUI::CTreeGridNode*>(m_pQQTreeView->GetRootNode()->FindNode(RES_STRING(CL_LANGUAGE_MATCH_468)));
	_pGuildNode->GetItem()->SetColor(COLOR_BLACK);
	_pGuildNode->GetUpImage()->UnLoadImage();
	_pGuildNode->GetDownImage()->UnLoadImage();
	_pGuildNode->SetUnitSize(nWidth, nHeight);
	_pGuildNode->SetColMaxNum(1);

	_pMasterNode = dynamic_cast<GUI::CTreeGridNode*>(m_pQQTreeView->GetRootNode()->FindNode(RES_STRING(CL_LANGUAGE_MATCH_850)));
	_pMasterNode->GetItem()->SetColor(COLOR_BLACK);
	_pMasterNode->GetUpImage()->UnLoadImage();
	_pMasterNode->GetDownImage()->UnLoadImage();
	_pMasterNode->SetUnitSize(nWidth, nHeight);
	_pMasterNode->SetColMaxNum(1);

	_pPrenticeNode = dynamic_cast<GUI::CTreeGridNode*>(m_pQQTreeView->GetRootNode()->FindNode(RES_STRING(CL_LANGUAGE_MATCH_851)));
	_pPrenticeNode->GetItem()->SetColor(COLOR_BLACK);
	_pPrenticeNode->GetUpImage()->UnLoadImage();
	_pPrenticeNode->GetDownImage()->UnLoadImage();
	_pPrenticeNode->SetUnitSize(nWidth, nHeight);
	_pPrenticeNode->SetColMaxNum(1);

	// Modify by lark.li 20080802 begin
	//_pTeamMgr->Find( enumTeamFrnd )->SetPointer( _pFrndNode );
	_pTeamMgr->Find(enumTeamFrnd)->SetPointer(_pFrndNodeAll);
	// End

	_pTeamMgr->Find(enumTeamGroup)->SetPointer(_pGroupNode);
	_pTeamMgr->Find(enumTeamMaster)->SetPointer(_pMasterNode);
	_pTeamMgr->Find(enumTeamPrentice)->SetPointer(_pPrenticeNode);

	_frmAddFriend = mgr.Find("frmAddFriend");
	if (!_frmAddFriend) {
		::Error(RES_STRING(CL_LANGUAGE_MATCH_470));
		return false;
	}
	_frmAddFriend->SetIsShow(true);
	pbtnAdd = dynamic_cast<CTextButton*>(_frmAddFriend->Find("btnYes"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmAddFriend->GetName(), "btnYes");
	}
	pbtnAdd->evtMouseClick = _evtAddFrnd;
	_pEditFrndName = dynamic_cast<CEdit*>(_frmAddFriend->Find("edtTradeGold"));
	if (!_pEditFrndName) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmAddFriend->GetName(), "edtTradeGold");
	}

	// Add by lark.li 20080804 begin
	_frmAddFrndGroup = mgr.Find("frmAddFrndGroup");
	if (!_frmAddFrndGroup) {
		::Error(RES_STRING(CL_LANGUAGE_MATCH_470));
		return false;
	}
	_frmAddFrndGroup->SetIsShow(true);
	pbtnAdd = dynamic_cast<CTextButton*>(_frmAddFrndGroup->Find("btnYes"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmAddFrndGroup->GetName(), "btnYes");
	}
	pbtnAdd->evtMouseClick = _evtAddFrndGroup;
	_pAddFrndGroupName = dynamic_cast<CEdit*>(_frmAddFrndGroup->Find("edtGroupName"));
	if (!_pAddFrndGroupName) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmAddFrndGroup->GetName(), "edtGroupName");
	}

	_frmEditFrndGroup = mgr.Find("frmEditFrndGroup");
	if (!_frmEditFrndGroup) {
		::Error(RES_STRING(CL_LANGUAGE_MATCH_470));
		return false;
	}
	_frmEditFrndGroup->SetIsShow(true);
	pbtnAdd = dynamic_cast<CTextButton*>(_frmEditFrndGroup->Find("btnYes"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditFrndGroup->GetName(), "btnYes");
	}
	pbtnAdd->evtMouseClick = _evtEditFrndGroup;
	_pEditFrndGroupName = dynamic_cast<CEdit*>(_frmEditFrndGroup->Find("edtGroupName"));
	if (!_pEditFrndGroupName) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditFrndGroup->GetName(), "edtGroupName");
	}
	// End

	_styMenu = CMenu::FindMenu("styMenu");
	if (!_styMenu) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmQQ->GetName(), "styMenu");
	}
	_styMenu->evtListMouseDown = _evtChangeStyle;

	_frmEditMotto = mgr.Find("frmEdit");
	if (!_frmEditMotto) {
		::Error(RES_STRING(CL_LANGUAGE_MATCH_471));
		return false;
	}
	_frmEditMotto->SetIsShow(true);
	pbtnAdd = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnYes"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "btnYes");
	}
	pbtnAdd->evtMouseClick = _evtChangeMotto;
	pbtnAdd = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnFlower"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "btnFlower");
	}
	pbtnAdd->evtMouseClick = _evtChangeMotto;
	pbtnAdd = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnEgg"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "btnEgg");
	}
	pbtnAdd->evtMouseClick = _evtChangeMotto;

	pbtnAdd = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnxiangce01"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "btnxiangce01");
	}
	pbtnAdd->evtMouseClick = _evtChangeMotto;

	pbtnAdd = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnxiangce02"));
	if (!pbtnAdd) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "btnxiangce02");
	}
	pbtnAdd->evtMouseClick = _evtChangeMotto;

	_pEditMotto = dynamic_cast<CEdit*>(_frmEditMotto->Find("edtTradeGold"));
	if (!_pEditMotto) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "edtTradeGold");
	}
	_labNameOfMottoFrm = dynamic_cast<CLabelEx*>(_frmEditMotto->Find("labName"));
	if (!_labNameOfMottoFrm) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "labName");
	}
	_imgFaceOfMottoFrm = dynamic_cast<CImage*>(_frmEditMotto->Find("imgMhead"));
	if (!_imgFaceOfMottoFrm) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "imgMhead");
	}
	_chkForbidOfMottoFrm = dynamic_cast<CCheckBox*>(_frmEditMotto->Find("chkChat"));
	if (!_chkForbidOfMottoFrm) {
		return _Error(RES_STRING(CL_LANGUAGE_MATCH_411), _frmEditMotto->GetName(), "chkChat");
	}

	//_frmDetails = mgr.Find("frmdetails");
	//if( !_frmDetails )
	//{
	//	::Error(RES_STRING(CL_LANGUAGE_MATCH_472));
	//	return false;
	//}
	//_frmDetails->SetIsShow(true);
	//pbtnAdd = dynamic_cast<CTextButton*>(_frmDetails->Find("btnRefurbish"));
	//if( !pbtnAdd ) return _Error(RES_STRING(CL_LANGUAGE_MATCH_473), _frmDetails->GetName(), "btnRefurbish" );
	//pbtnAdd->evtMouseClick=_evtRefreshInfo;

	return true;
}

// Add by lark.li 20080804 begin
/*
 * 根据好友分组取得好友分组接点，如果不是好友则返回缺省值
*/
CTreeGridNode* CChat::GetFrndGroupNode(const char* szGroupName, CTreeGridNode* pNode) {
	if (strcmp(RES_STRING(CL_LANGUAGE_MATCH_466), szGroupName) == 0) {
		return _pFrndNode;
	}

	if (_pFrndNodeAll != pNode) {
		return pNode;
	}

	for (const auto& [name, pNode] : vFrndNode) {
		if (name == szGroupName) {
			return pNode;
		}
	}

	return nullptr;
}

bool CChat::IsFrndGroupNode(CTreeGridNode* pNode) {
	if (pNode == _pFrndNode)
		return true;

	for (const auto& [name, p] : vFrndNode) {
		if (p == pNode) {
			return true;
		}
	}

	return false;
}

void CChat::AddFrndGroup(const char* szGroupName) {
	if (strcmp(RES_STRING(CL_LANGUAGE_MATCH_466), szGroupName) == 0)
		return;

	auto* item = new CNoteGraph(0);
	item->GetImage()->LoadAllImage("texture/ui/QQ2.tga", 87, 16, 105, 222);
	item->GetImage()->SetScale(87, 16);
	item->SetString(szGroupName);
	item->SetTextX(5);
	item->SetTextY(2);

	auto* pTreeNode = new CTreeGridNode(m_pQQTreeView, item);
	pTreeNode->SetIsExpand(false);
	pTreeNode->SetColMaxNum(1);
	pTreeNode->SetUnitSize(32, 32);

	CTreeNodeObj* parent = m_pQQTreeView->GetRootNode();

	if (parent)
		parent->AddNode(pTreeNode);

	vFrndNode.emplace_back(szGroupName, pTreeNode);

	int nWidth = m_pQQTreeView->GetWidth() - m_pQQTreeView->GetScroll()->GetWidth() - 28;
	int nHeight = SMALL_ICON_SIZE + 3;
	pTreeNode->GetItem()->SetColor(COLOR_BLACK);
	pTreeNode->GetUpImage()->UnLoadImage();
	pTreeNode->GetDownImage()->UnLoadImage();
	pTreeNode->SetUnitSize(nWidth, nHeight);
	pTreeNode->SetColMaxNum(1);

	m_pQQTreeView->Refresh();
}

void CChat::DelFrndGroup(const char* szGroupName) {
	if (strcmp(RES_STRING(CL_LANGUAGE_MATCH_466), szGroupName) == 0) {
		_pFrndNode->Clear();
		if (_pFrndNode->GetIsExpand()) {
			_pFrndNode->RefreshNode();
		}

		return;
	}

	if (auto it = std::find_if(vFrndNode.begin(), vFrndNode.end(),
							   [szGroupName](std::pair<std::string, CTreeGridNode*> p) { return p.first == szGroupName; });
		it != vFrndNode.end()) {
		auto [name, pNode] = *it;
		if (m_pQQTreeView) {
			CTreeNodeObj* parent = m_pQQTreeView->GetRootNode();
			if (parent) {
				pNode->ClearAllChild();
				if (pNode->GetIsExpand()) {
					pNode->RefreshNode();
				}
				parent->DelNode(pNode);
			}
		}
		vFrndNode.erase(it);
	}

	if (m_pQQTreeView) {
		m_pQQTreeView->Refresh();
	}
}

void CChat::ChangeFrndGroup(const char* szOldGroupName, const char* szNewGroupName) {
	if (strcmp(RES_STRING(CL_LANGUAGE_MATCH_466), szOldGroupName) == 0) {
		return;
	}

	if (auto it = std::find_if(vFrndNode.begin(), vFrndNode.end(),
							   [szOldGroupName](std::pair<std::string, CTreeGridNode*> p) { return p.first == szOldGroupName; });
		it != vFrndNode.end()) {
		if (m_pQQTreeView) {
			auto [name, pNode] = *it;
			name = szNewGroupName;
			pNode->SetCaption(szNewGroupName);
			pNode->RefreshNode();
			m_pQQTreeView->Refresh();
		}
	}
}

void CChat::End() {
	m_pQQTreeView = nullptr;
	SAFE_DELETE(_pTeamMgr);

	for (auto* p : _pCharacter) {
		SAFE_DELETE(p);
	}

	SAFE_DELETE(_pFrndNodeAll);
}

void CChat::ClearTeam() {
	for (auto& i : frmTeamMenber) {
		i->SetIsShow(false);
	}
}

void CChat::RefreshTeamData(CMember* pCurMember) {
	if (!pCurMember || !pCurMember->GetPointer())
		return;

	CMemberData* pCurMemData = pCurMember->GetData();
	DWORD i = pCurMemData->GetIndex();

	proTeamMenberHP[i]->SetRange(0.0f, (float)pCurMemData->GetMaxHP());
	proTeamMenberHP[i]->SetPosition((float)pCurMemData->GetHP());

	static char szBuf[32] = {0};
	_snprintf_s(szBuf, _TRUNCATE, "%d", pCurMemData->GetLV());
	labLv[i]->SetCaption(szBuf);

	if (pCurMemData->GetFace())
		_pCharacter[i]->UpdataFace(*pCurMemData->GetFace());
}

void CChat::RefreshTeam() {
	CTeam* pTeam = _pTeamMgr->Find(enumTeamGroup);
	if (!dynamic_cast<CWorldScene*>(CGameApp::GetCurScene())) {
		for (auto& i : frmTeamMenber) {
			i->SetIsShow(false);
		}
		return;
	}

	if (CGameScene::GetMainCha() && CGameScene::GetMainCha()->getHumanID() == CTeamMgr::GetTeamLeaderID()) {
		// 在自已的头上显示队长标志
		g_stUIStart.SetIsLeader(true);
	} else {
		g_stUIStart.SetIsLeader(false);
	}

	int Num = pTeam->GetCount();
	for (int i = MAX_MEMBER - 1; i >= 0; i--) {
		if (i < Num) {
			CMember* pCurMember = pTeam->GetMember(i);
			if (pCurMember && pCurMember->GetPointer()) {
				CMemberData* pCurMemData = pCurMember->GetData();
				pCurMemData->SetIndex(i);

				labMenberName[i]->SetCaption(pCurMember->GetName());

				RefreshTeamData(pCurMember);

				// Modify by lark.li 20080812 begin
				frmTeamMenber[i]->nTag = pCurMember->GetID();

				imgLeader[i]->SetIsShow(pCurMember->GetID() == CTeamMgr::GetTeamLeaderID());
				frmTeamMenber[i]->SetIsShow(true);
				// End
			}
		} else {
			frmTeamMenber[i]->SetIsShow(false);
		}
	}
}

int CChat::TeamSend(DWORD dwMsg, void* pData, DWORD dwParam) {
	CTreeView* pTree = GetTeamView();
	if (!pTree)
		return -1;

	switch (dwMsg) {
	case enumSTM_ADD_GROUP: {
		auto* pTeam = static_cast<CTeam*>(pData);
		auto* pNode = static_cast<CTreeGridNode*>(pTeam->GetPointer());
		if (!pNode)
			return -1;

		pNode->Clear();
		if (pNode->GetIsExpand())
			pTree->Refresh();

		if (enumTeamGroup == dwParam)
			ClearTeam();
	} break;
	case enumSTM_DEL_GROUP: {
		auto* pTeam = static_cast<CTeam*>(pData);
		auto* pNode = static_cast<CTreeGridNode*>(pTeam->GetPointer());
		if (!pNode) {
			return -1;
		}

		pNode->Clear();
		if (pNode->GetIsExpand())
			pTree->Refresh();

		if (enumTeamGroup == dwParam)
			ClearTeam();
	} break;
	case enumSTM_ADD_MEMBER: {
		auto* pMember = static_cast<CMember*>(pData);
		auto* pNode = static_cast<CTreeGridNode*>(pMember->GetTeam()->GetPointer());
		if (!pNode)
			return -1;

		// Add by lark.li 20080804 begin
		pNode = GetFrndGroupNode(pMember->GetGroupName(), pNode);
		if (!pNode)
			return -1;
		// End

		auto* pItem = new CTextGraph(2);
		pItem->SetString(pMember->GetShowName());
		pItem->SetPointer(pMember);
		pNode->AddItem(pItem);
		pMember->SetPointer(pItem);
		CChatIconInfo* pIconInfo = GetChatIconInfo(pMember->GetIconID());
		if (pIconInfo) {
			CGuiPic* pPic = pItem->GetImage();
			std::string strPath = "texture/ui/HEAD/";
			if (pMember->IsOnline()) {
				pPic->LoadImage((strPath + pIconInfo->szSmall).c_str(), SMALL_ICON_SIZE, SMALL_ICON_SIZE, 0, pIconInfo->nSmallX, pIconInfo->nSmallY);
				pPic->SetFrame(0);
				pItem->SetColor(0xfffc2f20);
			} else {
				pPic->LoadImage((strPath + pIconInfo->szSmallOff).c_str(), SMALL_ICON_SIZE, SMALL_ICON_SIZE, 0, pIconInfo->nSmallX, pIconInfo->nSmallY);
				pPic->SetFrame(0);
				pItem->SetColor(0xff000000);
			}
		}
		std::string _strHint = pMember->GetName();
		std::string strMotto = pMember->GetMotto();
		if (!strMotto.empty()) {
			_strHint += "(" + strMotto + ")";
		}
		pItem->SetHint(_strHint.c_str());

		// Modify by lark.li 20080806 begin
		//SortOnlineFrnd();
		SortOnlineFrnd(pNode);
		// End

		if (pNode->GetIsExpand())
			pTree->Refresh();
		if (enumTeamGroup == dwParam)
			RefreshTeam();
	} break;
	case enumSTM_DEL_MEMBER: {
		auto* pMember = static_cast<CMember*>(pData);
		if (pMember == _curSelectMember) {
			_curSelectMember = nullptr;
			_frmQQ->PopMenu(nullptr);
		}
		auto* pNode = static_cast<CTreeGridNode*>(pMember->GetTeam()->GetPointer());
		if (!pNode)
			return -1;

		// Add by lark.li 20080806 begin
		pNode = GetFrndGroupNode(pMember->GetGroupName(), pNode);
		if (!pNode)
			return -1;
		// End

		pNode->DelItem(static_cast<CItemObj*>(pMember->GetPointer()));
		pMember->SetPointer(nullptr);
		if (pNode->GetIsExpand())
			pTree->Refresh();

		if (dwParam == enumTeamGroup)
			RefreshTeam();
	} break;
	case enumSTM_NODE_CHANGE: {
		auto* pMember = static_cast<CMember*>(pData);
		auto* pItem = static_cast<CTextGraph*>(pMember->GetPointer());
		CChatIconInfo* pIconInfo = GetChatIconInfo(pMember->GetIconID());
		if (pIconInfo && pItem) // modify by Philip.Wu  2006-08-13  修改去除路人后的当机
		{
			CGuiPic* pPic = pItem->GetImage();
			std::string strPath = "texture/ui/HEAD/";
			if (pMember->IsOnline()) {
				pPic->LoadImage((strPath + pIconInfo->szSmall).c_str(), SMALL_ICON_SIZE, SMALL_ICON_SIZE, 0, pIconInfo->nSmallX, pIconInfo->nSmallY);
				pPic->SetFrame(0);
				pItem->SetColor(0xfffc2f20);
			} else {
				pPic->LoadImage((strPath + pIconInfo->szSmallOff).c_str(), SMALL_ICON_SIZE, SMALL_ICON_SIZE, 0, pIconInfo->nSmallX, pIconInfo->nSmallY);
				pPic->SetFrame(0);
				pItem->SetColor(0xff000000);
			}
		}
		if (pItem) {
			pItem->SetString(pMember->GetShowName());
			std::string _strHint = pMember->GetName();
			std::string strMotto = pMember->GetMotto();
			if (!strMotto.empty()) {
				_strHint += "(" + strMotto + ")";
			}
			pItem->SetHint(_strHint.c_str());
		}

		// Modify by lark.li 20080806 begin
		//SortOnlineFrnd();
		auto* pNode = static_cast<CTreeGridNode*>(pMember->GetTeam()->GetPointer());
		if (!pNode)
			return -1;

		pNode = GetFrndGroupNode(pMember->GetGroupName(), pNode);
		if (!pNode)
			return -1;

		SortOnlineFrnd(pNode);
		// End
		pTree->Refresh();

		if (dwParam == enumTeamGroup)
			RefreshTeam();
	} break;
	case enumSTM_NODE_DATA_CHANGE: {
		if (dwParam == enumTeamGroup)
			RefreshTeamData(static_cast<CMember*>(pData));
	} break;
	case enumSTM_AFTER_DEL_MEMBER: {
		if (dwParam == enumTeamGroup)
			RefreshTeam();
	} break;
		// Add by lark.li 20080804 begin
	case enumSTM_ADD_FRIEND_GROUP: {
		const auto* szGroupName = static_cast<const char*>(pData);
		AddFrndGroup(szGroupName);
	} break;
	case enumSTM_DEL_FRIEND_GROUP: {
		const auto* szGroupName = static_cast<const char*>(pData);
		DelFrndGroup(szGroupName);
	} break;
		// End
	}
	return 0;
}

// Add by lark.li 20080806 begin
void CChat::SortOnlineFrnd(CTreeGridNode* pNode) //TODO: Order players by name as well (once game is restarted, names always get ordered - that's how group server gives them)
{
	DWORD nCount = pNode->GetItemCount();
	DWORD i = 0;
	DWORD j = 0;
	CTextGraph *pItem1, *pItem2;
	CMember *pMember1, *pMember2;
	for (; i < nCount; i++) {
		pItem1 = dynamic_cast<CTextGraph*>(pNode->GetItemByIndex(i));
		pMember1 = (CMember*)(pItem1->GetPointer());
		if (pMember1->IsOnline()) {
			continue;
		}
		if (j <= i) {
			j = i + 1;
		}
		for (; j < nCount; j++) {
			pItem2 = dynamic_cast<CTextGraph*>(pNode->GetItemByIndex(j));
			pMember2 = (CMember*)(pItem2->GetPointer());
			if (pMember2->IsOnline()) {
				//1和2项对换
				pMember1->SetPointer(pItem2);
				pMember2->SetPointer(pItem1);
				pItem1->SetPointer(pMember2);
				pItem2->SetPointer(pMember1);
				CChatIconInfo* pIconInfo1 = GetChatIconInfo(pMember1->GetIconID());
				CChatIconInfo* pIconInfo2 = GetChatIconInfo(pMember2->GetIconID());
				if (!pIconInfo1 || !pIconInfo2) {
					continue;
				}
				CGuiPic* pPic2 = pItem2->GetImage();
				std::string strPath = "texture/ui/HEAD/";
				pPic2->LoadImage((strPath + pIconInfo1->szSmallOff).c_str(), SMALL_ICON_SIZE, SMALL_ICON_SIZE, 0, pIconInfo1->nSmallX, pIconInfo1->nSmallY);
				pPic2->SetFrame(0);
				pItem2->SetColor(0xff000000);
				CGuiPic* pPic1 = pItem1->GetImage();
				pPic1->LoadImage((strPath + pIconInfo2->szSmall).c_str(), SMALL_ICON_SIZE, SMALL_ICON_SIZE, 0, pIconInfo2->nSmallX, pIconInfo2->nSmallY);
				pPic1->SetFrame(0);
				pItem1->SetColor(0xfffc2f20);
				std::string _strHint1 = pMember1->GetName();
				std::string strMotto1 = pMember1->GetMotto();
				if (!strMotto1.empty()) {
					_strHint1 += "(" + strMotto1 + ")";
				}
				pItem2->SetString(pMember1->GetShowName());
				pItem2->SetHint(_strHint1.c_str());
				std::string _strHint2 = pMember2->GetName();
				std::string strMotto2 = pMember2->GetMotto();
				if (!strMotto2.empty()) {
					_strHint2 += "(" + strMotto2 + ")";
				}
				pItem1->SetString(pMember2->GetShowName());
				pItem1->SetHint(_strHint2.c_str());
				break;
			}
		}
	}
}
// End

void CChat::SortOnlineFrnd() {
	// Modify by lark.li 20080802 begin
	//DWORD nCount=_pFrndNode->GetItemCount();
	//DWORD i=0;
	//DWORD j=0;
	//CTextGraph *pItem1,*pItem2;
	//CMember *pMember1,*pMember2;
	//for (;i<nCount;i++)
	//{
	//	pItem1=dynamic_cast<CTextGraph*>(_pFrndNode->GetItemByIndex(i));
	//	pMember1=(CMember*)(pItem1->GetPointer());
	//	if (pMember1->IsOnline())
	//	{
	//		continue;
	//	}
	//	if (j<=i)
	//	{
	//		j=i+1;
	//	}
	//	for (;j<nCount;j++)
	//	{
	//		pItem2=dynamic_cast<CTextGraph*>(_pFrndNode->GetItemByIndex(j));
	//		pMember2=(CMember*)(pItem2->GetPointer());
	//		if (pMember2->IsOnline())
	//		{
	//			//1和2项对换
	//			pMember1->SetPointer(pItem2);
	//			pMember2->SetPointer(pItem1);
	//			pItem1->SetPointer(pMember2);
	//			pItem2->SetPointer(pMember1);
	//			CChatIconInfo *pIconInfo1=GetChatIconInfo(pMember1->GetIconID());
	//			CChatIconInfo *pIconInfo2=GetChatIconInfo(pMember2->GetIconID());
	//			if (!pIconInfo1 || !pIconInfo2)
	//			{
	//				continue;
	//			}
	//			CGuiPic* pPic2=pItem2->GetImage();
	//			string strPath="texture/ui/HEAD/";
	//			pPic2->LoadImage((strPath+pIconInfo1->szSmallOff).c_str(),SMALL_ICON_SIZE,SMALL_ICON_SIZE,0,pIconInfo1->nSmallX,pIconInfo1->nSmallY);
	//			pPic2->SetFrame( 0 );
	//			pItem2->SetColor(0xff000000);
	//			CGuiPic* pPic1=pItem1->GetImage();
	//			pPic1->LoadImage((strPath+pIconInfo2->szSmall).c_str(),SMALL_ICON_SIZE,SMALL_ICON_SIZE,0,pIconInfo2->nSmallX,pIconInfo2->nSmallY);
	//			pPic1->SetFrame( 0 );
	//			pItem1->SetColor(0xfffc2f20);
	//			string _strHint1=pMember1->GetName();
	//			string strMotto1=pMember1->GetMotto();
	//			if (!strMotto1.empty())
	//			{
	//				_strHint1+="("+strMotto1+")";
	//			}
	//			pItem2->SetString(pMember1->GetShowName());
	//			pItem2->SetHint(_strHint1.c_str());
	//			string _strHint2=pMember2->GetName();
	//			string strMotto2=pMember2->GetMotto();
	//			if (!strMotto2.empty())
	//			{
	//				_strHint2+="("+strMotto2+")";
	//			}
	//			pItem1->SetString(pMember2->GetShowName());
	//			pItem1->SetHint(_strHint2.c_str());
	//			break;
	//		}
	//	}
	//}
	// End
}

void CChat::_RenderEvent(C3DCompent* pSender, int x, int y) {
	_pCharacter[pSender->nTag]->Render();
}

void CChat::_MemberMouseDownEvent(CGuiData* pSender, int x, int y, MouseClickState key) {
	if ((key & MouseClickState::LDown) != MouseClickState()) {
		auto* pScene = dynamic_cast<CWorldScene*>(CGameApp::GetCurScene());
		if (!pScene)
			return;

		CCharacter* pCha = pScene->SearchByHumanName(labMenberName[pSender->nTag]->GetCaption());
		if (!pCha) {
			g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_474), labMenberName[pSender->nTag]->GetCaption());
			return;
		}

		if (g_pGameApp->IsAltPress())
			pScene->GetMouseDown().GetAutoAttack()->Follow(CGameScene::GetMainCha(), pCha);
		else
			pScene->GetMouseDown().ActCha(CGameScene::GetMainCha(), pCha);
		return;
	}

	if ((key & MouseClickState::RDown) != MouseClickState()) {
		if (_groupMouseRight) {
			CTeam* pTeam = _pTeamMgr->Find(enumTeamGroup);
			CMember* pMember = pTeam->Find(frmTeamMenber[pSender->nTag]->nTag);
			/*		if( pMember )
			{
				_curSelectMember = pMember;
				CMenuItem* pItem = _groupMouseRight->GetMenuItem(3);
				if (pItem && stricmp( pItem->GetString(), RES_STRING(CL_LANGUAGE_MATCH_475) ) == 0)
				{
					if (g_stUIStart.GetIsLeader())
					{
						pItem->SetIsEnabled(true);
					}
					else
					{
						pItem->SetIsEnabled(false);
					}
				}
				pSender->GetForm()->PopMenu(_groupMouseRight,x,y);
				return;
			}*/
			//Modify by sunny.sun 20080820
			//Begin
			if (pMember) {
				_curSelectMember = pMember;
				CMenuItem* pItem = _groupMouseRight->GetMenuItem(3);

				const char* MapName = g_pGameApp->GetCurScene()->GetTerrainName();
				if (_stricmp(MapName, "starena1") == 0 || _stricmp(MapName, "starena2") == 0 || _stricmp(MapName, "starena3") == 0) {
					if (pItem && _stricmp(pItem->GetString(), RES_STRING(CL_LANGUAGE_MATCH_475)) == 0) {
						pItem->SetIsEnabled(false);
					}
				} else {
					if (pItem && _stricmp(pItem->GetString(), RES_STRING(CL_LANGUAGE_MATCH_475)) == 0) {
						if (g_stUIStart.GetIsLeader()) {
							pItem->SetIsEnabled(true);
						} else {
							pItem->SetIsEnabled(false);
						}
					}
					pSender->GetForm()->PopMenu(_groupMouseRight, x, y);
					return;
				}
			} //End
		}
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_476), labMenberName[pSender->nTag]->GetCaption());
	}
}

void CChat::_MainMouseClick(CGuiData* pSender, int x, int y, MouseClickState key) {
	auto* pTree = dynamic_cast<CTreeView*>(pSender);
	if (!pTree)
		return;

	int sx = x;
	int sy = y;

	if (auto* pSelectNodeItem = dynamic_cast<CNoteGraph*>(pTree->GetHitItem(x, y)); pSelectNodeItem) {
		for (const auto& [name, pNode] : g_stUIChat.vFrndNode) {
			if (pSelectNodeItem == pNode->GetItem())
				if ((key & MouseClickState::RDown) != MouseClickState()) {
					_curSelectNode = pNode;

					if (x + _frndGroupMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
						sx = g_pGameApp->GetWindowWidth() - _frndGroupMouseRight->GetWidth();
					if (y + _frndGroupMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
						sy = g_pGameApp->GetWindowHeight() - _frndGroupMouseRight->GetHeight();
					_frmQQ->PopMenu(_frndGroupMouseRight, sx, sy);
					return;
				}
		}
	}

	auto* pSelectItem = dynamic_cast<CTextGraph*>(pTree->GetHitItem(x, y));

	if (!pSelectItem && ((key & MouseClickState::RDown) != MouseClickState())) {
		if (x + _hitNothingMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
			sx = g_pGameApp->GetWindowWidth() - _hitNothingMouseRight->GetWidth();
		if (y + _hitNothingMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
			sy = g_pGameApp->GetWindowHeight() - _hitNothingMouseRight->GetHeight();
		_frmQQ->PopMenu(_hitNothingMouseRight, sx, sy);

		return;
	}

	if (!pSelectItem)
		return;

	auto* pSelectNode = dynamic_cast<CTreeGridNode*>(pTree->GetSelectNode());
	if (!pSelectNode)
		return;

	if (g_stUIChat.IsFrndGroupNode(pSelectNode)) {
		CTeam* pTeam = _pTeamMgr->Find(enumTeamFrnd);
		for (DWORD i = 0; i < pTeam->GetCount(); i++) {
			CMember* pMember = pTeam->GetMember(i);
			if (pMember->GetPointer() == pSelectItem) {
				_curSelectMember = pMember;
				if ((key & MouseClickState::LDB) != MouseClickState()) {
					CTalkSessionFormMgr::ApplySession(&_curSelectMember);
				} else if ((key & MouseClickState::RDown) != MouseClickState()) {
					if (x + _frndMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
						sx = g_pGameApp->GetWindowWidth() - _frndMouseRight->GetWidth();
					if (y + _frndMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
						sy = g_pGameApp->GetWindowHeight() - _frndMouseRight->GetHeight();
					_frmQQ->PopMenu(_frndMouseRight, sx, sy);
				}
				break;
			}
		}
	} else if (pSelectNode == _pTeamMgr->Find(enumTeamGroup)->GetPointer()) {
		CTeam* pTeam = _pTeamMgr->Find(enumTeamGroup);
		for (DWORD i = 0; i < pTeam->GetCount(); i++) {
			CMember* pMember = pTeam->GetMember(i);
			if (pMember->GetPointer() == pSelectItem) {
				_curSelectMember = pMember;
				if ((key & MouseClickState::LDB) != MouseClickState()) {
					CTalkSessionFormMgr::ApplySession(&_curSelectMember);
				} else if ((key & MouseClickState::RDown) != MouseClickState()) {
					if (x + _groupMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
						sx = g_pGameApp->GetWindowWidth() - _groupMouseRight->GetWidth();
					if (y + _groupMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
						sy = g_pGameApp->GetWindowHeight() - _groupMouseRight->GetHeight();
					_frmQQ->PopMenu(_groupMouseRight, sx, sy);
				}
				break;
			}
		}
	} else if (pSelectNode == _pTeamMgr->Find(enumTeamRoad)->GetPointer()) {
		CTeam* pTeam = _pTeamMgr->Find(enumTeamRoad);
		for (DWORD i = 0; i < pTeam->GetCount(); i++) {
			CMember* pMember = pTeam->GetMember(i);
			if (pMember->GetPointer() == pSelectItem) {
				_curSelectMember = pMember;
				if ((key & MouseClickState::LDB) != MouseClickState()) {
					CTalkSessionFormMgr::ApplySession(&_curSelectMember);
				} else if ((key & MouseClickState::RDown) != MouseClickState()) {
					if (x + _roadMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
						sx = g_pGameApp->GetWindowWidth() - _roadMouseRight->GetWidth();
					if (y + _roadMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
						sy = g_pGameApp->GetWindowHeight() - _roadMouseRight->GetHeight();
					_frmQQ->PopMenu(_roadMouseRight, sx, sy);
				}
				break;
			}
		}
	} else if (pSelectNode == _pTeamMgr->Find(enumTeamMaster)->GetPointer()) // 导师
	{
		CTeam* pTeam = _pTeamMgr->Find(enumTeamMaster);
		for (DWORD i = 0; i < pTeam->GetCount(); i++) {
			CMember* pMember = pTeam->GetMember(i);
			if (pMember->GetPointer() == pSelectItem) {
				_curSelectMember = pMember;
				if ((key & MouseClickState::LDB) != MouseClickState()) {
					CTalkSessionFormMgr::ApplySession(&_curSelectMember);
				} else if ((key & MouseClickState::RDown) != MouseClickState()) {
					if (x + _roadMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
						sx = g_pGameApp->GetWindowWidth() - _roadMouseRight->GetWidth();
					if (y + _roadMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
						sy = g_pGameApp->GetWindowHeight() - _roadMouseRight->GetHeight();
					_frmQQ->PopMenu(_MasterMouseRight, sx, sy);
				}
				break;
			}
		}
	} else if (pSelectNode == _pTeamMgr->Find(enumTeamPrentice)->GetPointer()) // 学徒
	{
		CTeam* pTeam = _pTeamMgr->Find(enumTeamPrentice);
		for (DWORD i = 0; i < pTeam->GetCount(); i++) {
			CMember* pMember = pTeam->GetMember(i);
			if (pMember->GetPointer() == pSelectItem) {
				_curSelectMember = pMember;
				if ((key & MouseClickState::LDB) != MouseClickState()) {
					CTalkSessionFormMgr::ApplySession(&_curSelectMember);
				} else if ((key & MouseClickState::RDown) != MouseClickState()) {
					if (x + _roadMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
						sx = g_pGameApp->GetWindowWidth() - _roadMouseRight->GetWidth();
					if (y + _roadMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
						sy = g_pGameApp->GetWindowHeight() - _roadMouseRight->GetHeight();
					_frmQQ->PopMenu(_StudentMouseRight, sx, sy);
				}
				break;
			}
		}
	} else if (pSelectNode == _pSessionNode) {
		if ((key & MouseClickState::LDB) != MouseClickState()) {
			CTalkSessionFormMgr::OnClickOpenSession(pSelectItem);
		} else if ((key & MouseClickState::RDown) != MouseClickState()) {
			auto* sessForm = static_cast<CTalkSessionForm*>(pSelectItem->GetPointer());
			if (sessForm) {
				_sessMouseRight->SetPointer(sessForm);
				if (x + _sessMouseRight->GetWidth() > g_pGameApp->GetWindowWidth())
					sx = g_pGameApp->GetWindowWidth() - _sessMouseRight->GetWidth();
				if (y + _sessMouseRight->GetHeight() > g_pGameApp->GetWindowHeight())
					sy = g_pGameApp->GetWindowHeight() - _sessMouseRight->GetHeight();
				_frmQQ->PopMenu(_sessMouseRight, sx, sy);
			}
		}
	} else if (pSelectNode == _pGuildNode) {
		int i = 0;
		CGuildMemberData* pMemberData = nullptr;
		while (pMemberData = CGuildMembersMgr::FindGuildMemberByIndex(i)) {
			if (pMemberData->GetPointer() == pSelectItem) {
				if ((key & MouseClickState::LDB) != MouseClickState()) {
					auto* pMember = new CTalkSessionFormMgr::sApplyMember[1];
					pMember[0].name = pMemberData->GetName();
					pMember[0].numbers = 1;
					CTalkSessionFormMgr::ApplySession(pMember);
					delete[] pMember;
				} else if ((key & MouseClickState::RDown) != MouseClickState()) {
				}
			}
			i++;
		}
	} else {
		g_pGameApp->SysInfo("Unknow Hit! File: UIChat.cpp   Class: CChat   Code: Arcol");
	}
}

void CChat::_evtQQMainShow(CGuiData* pSender) {
	CCharacter* pMain = CGameScene::GetMainCha();
	if (pMain) {
		auto* pMineName = dynamic_cast<CLabelEx*>(CFormMgr::s_Mgr.Find("frmQQ")->Find("labMineName"));
		if (pMineName) {
			pMineName->SetCaption(pMain->getHumanName());
		} else {
			_Error(RES_STRING(CL_LANGUAGE_MATCH_473), _frmQQ->GetName(), "labMineName");
		}
	}
}

void CChat::_OnMouseAddFrndGroupMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_hitNothingMouseRight->SetIsShow(false);

	CMenuItem* pItem = _hitNothingMouseRight->GetSelectMenu();
	if (!pItem)
		return;
	std::string str = pItem->GetString();
	if (str == RES_STRING(FORMS_CHAT_CLU_000017)) {
		if (g_stUIChat.vFrndNode.size() >= 9) //FIXED: Limit wasn't being obeyed
		{
			g_stUIBox.ShowMsgBox(nullptr, RES_STRING(FORMS_CHAT_CLU_000043)); //FIXED: Translated (using resource)
			return;
		}
		_pAddFrndGroupName->SetCaption("");

		_frmAddFrndGroup->Refresh();
		_frmAddFrndGroup->Show();
	}

	_frmQQ->Refresh();
}

void CChat::_OnMouseFrndGroupMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_frndGroupMouseRight->SetIsShow(false);

	CMenuItem* pItem = _frndGroupMouseRight->GetSelectMenu();
	if (!pItem)
		return;
	std::string str = pItem->GetString();
	if (str == RES_STRING(FORMS_CHAT_CLU_000015)) {
		_pEditFrndGroupName->SetCaption(_curSelectNode->GetCaption());

		_frmEditFrndGroup->Refresh();
		_frmEditFrndGroup->Show();
	} else if (str == RES_STRING(FORMS_CHAT_CLU_000016)) {
		int count = _curSelectNode->GetItemCount();

		if (count > 0) {
			char buffer[255];
			_snprintf_s(buffer, _TRUNCATE, RES_STRING(FORMS_CHAT_CLU_000041), count);
			g_stUIBox.ShowMsgBox(nullptr, buffer);
		} else {
			g_stUIBox.ShowSelectBox(_evtAskDelGroupEvent, RES_STRING(FORMS_CHAT_CLU_000042), false);
		}
	} else if (str == RES_STRING(FORMS_CHAT_CLU_000017)) {
		if (g_stUIChat.vFrndNode.size() >= 9) {
			g_stUIBox.ShowMsgBox(nullptr, RES_STRING(FORMS_CHAT_CLU_000043));
			return;
		}

		_pAddFrndGroupName->SetCaption("");

		_frmAddFrndGroup->Refresh();
		_frmAddFrndGroup->Show();
	}

	_frmQQ->Refresh();
}

void CChat::_evtAskDelGroupEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType == CForm::mrYes)
		CP_Frnd_Del_Group(_curSelectNode->GetCaption());
}

// End

// Add by lark.li 20080811 begin
void CChat::ShowDetailForm(CMember* pMember) //右击好友显示的详细信息
{
	if (pMember) {
		_UpdateFrndInfo(pMember);
		CP_Frnd_Refresh_Info(pMember->GetID());
		//_frmDetails->Show();
		g_ChaQeryInfo.SetShowChaInfo(true);
	}
}
// End

void CChat::_OnMouseFrndMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_frndMouseRight->SetIsShow(false);
	if (!_curSelectMember)
		return;
	CMenuItem* pItem = _frndMouseRight->GetSelectMenu();
	if (!pItem)
		return;
	std::string str = pItem->GetString();
	if (str == RES_STRING(CL_LANGUAGE_MATCH_477)) {
		// Modify by lark.li 20080811 begin
		g_stUIChat.ShowDetailForm(g_stUIChat._curSelectMember);
		//_UpdateFrndInfo(_curSelectMember);
		CP_Frnd_Refresh_Info(_curSelectMember->GetID());
		//_frmDetails->Show();
		// End
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_478)) {
		CTalkSessionFormMgr::ApplySession(&_curSelectMember);
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_479)) {
		std::string str = RES_STRING(CL_LANGUAGE_MATCH_480);
		str += _curSelectMember->GetName();
		stSelectBox* pSelectBox = g_stUIBox.ShowSelectBox(_OnFrndDeleteConfirm, str.c_str(), true);
		pSelectBox->dwTag = _curSelectMember->GetID();
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_481)) {
		CCozeForm::GetInstance()->OnPrivateNameSet(_curSelectMember->GetName());
	}
	_frmQQ->Refresh();
}

void CChat::_OnMouseGroupMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_groupMouseRight->SetIsShow(false);
	if (!_curSelectMember)
		return;
	CMenuItem* pItem = _groupMouseRight->GetSelectMenu();
	if (!pItem)
		return;
	std::string str = pItem->GetString();
	if (str == RES_STRING(CL_LANGUAGE_MATCH_482)) {
		CS_Frnd_Invite(_curSelectMember->GetName());
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_478)) {
		CTalkSessionFormMgr::ApplySession(&_curSelectMember);
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_483)) {
		CS_Team_Leave();
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_481)) {
		CCozeForm::GetInstance()->OnPrivateNameSet(_curSelectMember->GetName());
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_475)) {
		CS_Team_Kick(_curSelectMember->GetID());
	}
	_frmQQ->Refresh();
}

void CChat::_OnMouseRoadMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_roadMouseRight->SetIsShow(false);
	if (!_curSelectMember)
		return;

	CMenuItem* pItem = _roadMouseRight->GetSelectMenu();
	if (!pItem)
		return;
	std::string str = pItem->GetString();
	if (str == RES_STRING(CL_LANGUAGE_MATCH_482)) {
		CS_Frnd_Invite(_curSelectMember->GetName());
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_484)) {
		CS_Team_Invite(_curSelectMember->GetName());
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_478)) {
		CTalkSessionFormMgr::ApplySession(&_curSelectMember);
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_481)) {
		CCozeForm::GetInstance()->OnPrivateNameSet(_curSelectMember->GetName());
	}
	_frmQQ->Refresh();
}

void CChat::_OnMouseSessMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_frmQQ->PopMenu(nullptr);
	CMenuItem* pItem = _sessMouseRight->GetSelectMenu();
	if (!pItem)
		return;
	std::string str = pItem->GetString();
	if (str == RES_STRING(CL_LANGUAGE_MATCH_485)) // 密语对方
	{
		CTalkSessionFormMgr::OnClickCloseSession(static_cast<CTalkSessionForm*>(_sessMouseRight->GetPointer()));
	}

	_frmQQ->Refresh();
}

// 导师菜单
void CChat::_OnMouseMasterMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_frmQQ->PopMenu(FALSE);

	CMenuItem* pItem = _MasterMouseRight->GetSelectMenu();
	if (!pItem)
		return;

	std::string strCommand = pItem->GetString();

	if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_482)) // 添加好友
	{
		CCharacter* pMainCha = CGameScene::GetMainCha();
		if (pMainCha && pMainCha->getGameAttr() && pMainCha->getGameAttr()->get(ATTR_LV) >= 7) {
			CS_Frnd_Invite(_curSelectMember->GetName());
		} else {
			// 七级以下禁止添加好友
			g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_865));
		}
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_484)) // 邀请组队
	{
		CCharacter* pMainCha = CGameScene::GetMainCha();
		if (pMainCha && (pMainCha->IsBoat() || (pMainCha->getGameAttr() && pMainCha->getGameAttr()->get(ATTR_LV) >= 8))) {
			CS_Team_Invite(_curSelectMember->GetName());
		} else {
			// 八级以下禁止组队
			g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_866));
		}
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_481)) // 密语对方
	{
		CCozeForm::GetInstance()->OnPrivateNameSet(_curSelectMember->GetName());
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_478)) // 发送消息
	{
		CTalkSessionFormMgr::ApplySession(&_curSelectMember);
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_854)) // 解除关系（徒弟解除师傅）
	{
		CCharacter* pMain = CGameScene::GetMainCha();
		if (pMain && pMain->getGameAttr()) {
			const auto nLevel = static_cast<long>(pMain->getGameAttr()->get(ATTR_LV));

			char szBuffer[256] = {0};
			_snprintf_s(szBuffer, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_889), _curSelectMember->GetName(), nLevel * 500); // "解除师傅 %s 您将扣除\n金钱:%d\n确定吗？"

			stSelectBox* pSelectBox = g_stUIBox.ShowSelectBox(_OnMasterDeleteConfirm, szBuffer, true);
			pSelectBox->dwTag = _curSelectMember->GetID();
			pSelectBox->pointer = (void*)_curSelectMember->GetName();
		}
	}

	_frmQQ->Refresh();
}

// 学徒菜单
void CChat::_OnMouseStudentMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	_frmQQ->PopMenu(FALSE);

	CMenuItem* pItem = _StudentMouseRight->GetSelectMenu();
	if (!pItem)
		return;

	std::string strCommand = pItem->GetString();

	if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_482)) // 添加好友
	{
		CS_Frnd_Invite(_curSelectMember->GetName());
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_484)) // 邀请组队
	{
		if (_curSelectMember->GetLevel() >= 8) {
			CS_Team_Invite(_curSelectMember->GetName());
		} else {
			// 八级以下禁止组队
			g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_866));
		}
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_481)) // 密语对方
	{
		CCozeForm::GetInstance()->OnPrivateNameSet(_curSelectMember->GetName());
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_478)) // 发送消息
	{
		CTalkSessionFormMgr::ApplySession(&_curSelectMember);
	} else if (strCommand == RES_STRING(CL_LANGUAGE_MATCH_854)) // 解除关系（师傅解除徒弟）
	{
		CCharacter* pMain = CGameScene::GetMainCha();
		if (pMain && pMain->getGameAttr()) {
			const auto nLevel = static_cast<long>(pMain->getGameAttr()->get(ATTR_LV));

			char szBuffer[256] = {0};
			_snprintf_s(szBuffer, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_856), _curSelectMember->GetName(), nLevel * 5); // 解除徒弟 %s 您将扣除\n金钱:%d  声望:%d\n确定吗？

			stSelectBox* pSelectBox = g_stUIBox.ShowSelectBox(_OnStudentDeleteConfirm, szBuffer, true);
			pSelectBox->dwTag = _curSelectMember->GetID();
			pSelectBox->pointer = (void*)_curSelectMember->GetName();
		}
	}

	_frmQQ->Refresh();
}

void CChat::_evtQQMainAddFrnd(CGuiData* pSender, int x, int y, MouseClickState key) {
	_frmAddFriend->Refresh();
	_frmAddFriend->Show();
}

void CChat::_evtAddFrndGroup(CGuiData* pSender, int x, int y, MouseClickState key) {
	const std::string NewName = _pAddFrndGroupName->GetCaption();

	if (NewName.empty()) {
		_frmAddFrndGroup->Close();
		return;
	}

	if (NewName == RES_STRING(CL_LANGUAGE_MATCH_466)) {
		g_stUIBox.ShowMsgBox(nullptr, RES_STRING(CL_LANGUAGE_MATCH_970));
		return;
	}

	for (const auto& [name, pNode] : g_stUIChat.vFrndNode) {
		if (name == NewName) {
			g_stUIBox.ShowMsgBox(nullptr, RES_STRING(CL_LANGUAGE_MATCH_970));
			return;
		}
	}

	if (!IsValidName(NewName.c_str(), (unsigned short)NewName.length())) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_51));
	}

	_frmAddFrndGroup->Close();
	CP_Frnd_Add_Group(NewName.c_str());
}

void CChat::_evtEditFrndGroup(CGuiData* pSender, int x, int y, MouseClickState key) {
	const std::string NewName = _pEditFrndGroupName->GetCaption();
	const char* oldname = _curSelectNode->GetCaption();

	if (NewName.empty()) {
		_frmEditFrndGroup->Close();
		return;
	}

	if (NewName == oldname) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_971));
		return;
	}

	if (NewName == RES_STRING(CL_LANGUAGE_MATCH_466)) {
		g_stUIBox.ShowMsgBox(nullptr, RES_STRING(CL_LANGUAGE_MATCH_970));
		return;
	}

	for (const auto& [name, pNode] : g_stUIChat.vFrndNode) {
		if (NewName == name) {
			g_stUIBox.ShowMsgBox(nullptr, RES_STRING(CL_LANGUAGE_MATCH_970));
			return;
		}
	}

	if (!IsValidName(NewName.c_str(), static_cast<unsigned short>(NewName.length()))) {
		g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_51));
	}

	_frmEditFrndGroup->Close();
	CP_Frnd_Change_Group(oldname, NewName.c_str());
}

void CChat::_evtAddFrnd(CGuiData* pSender, int x, int y, MouseClickState key) {
	const std::string name = _pEditFrndName->GetCaption();
	if (!name.empty()) {
		IsValidName(name.c_str(), (unsigned short)name.length())
			? CS_Frnd_Invite(name.c_str())
			: g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_51));
	}
	_frmAddFriend->Close();
}

void CChat::_evtQQMainChangeStyle(CGuiData* pSender, int x, int y, MouseClickState key) {
	int sx = x;
	int sy = y;
	if (x + _styMenu->GetWidth() > g_pGameApp->GetWindowWidth())
		sx = g_pGameApp->GetWindowWidth() - _styMenu->GetWidth();
	if (y + _styMenu->GetHeight() > g_pGameApp->GetWindowHeight())
		sy = g_pGameApp->GetWindowHeight() - _styMenu->GetHeight();
	_frmQQ->PopMenu(_styMenu, sx, sy);
}

void CChat::_evtChangeStyle(CGuiData* pSender, int x, int y, MouseClickState key) {
	_frmQQ->PopMenu(nullptr);

	CMenuItem* pItem = _styMenu->GetSelectMenu();
	if (!pItem)
		return;
	std::string str = pItem->GetString();
	if (str == RES_STRING(CL_LANGUAGE_MATCH_486)) {
		_pTeamMgr->ChangeStyle(enumShowQQName);
	} else if (str == RES_STRING(CL_LANGUAGE_MATCH_487)) {
		_pTeamMgr->ChangeStyle(enumShowQQMotto);
	}
	_frmQQ->Refresh();
}

// Add by lark.li 20090325 begin
void CChat::_evtGostMainEditMotto(CGuiData* pSender, int x, int y, MouseClickState key) {
	if (!g_pGameApp->GetCurScene())
		return;
	if (!g_pGameApp->GetCurScene()->GetMainCha())
		return;

	g_stAllButton.ShowAllForm(true);
}
// End
void CChat::_evtQQMainEditMotto(CGuiData* pSender, int x, int y, MouseClickState key) {
	if (!g_pGameApp->GetCurScene())
		return;
	if (!g_pGameApp->GetCurScene()->GetMainCha())
		return;
	auto* pbtnAdd = dynamic_cast<CTextButton*>(_frmQQ->Find("btnscr")); // 搜索面板
	if (pSender == pbtnAdd) {
		g_ChaQeryInfo.SetShowQChaInfo(true);
	} else // 查询自己的信息
	{
		if (!_frmEditMotto->GetIsShow())
			CP_Frnd_Refresh_Info(g_stUIChat._dwSelfID);
		_pEditMotto->SetCaption(_strSelfMottoName.c_str());
		_labNameOfMottoFrm->SetCaption(g_pGameApp->GetCurScene()->GetMainCha()->getHumanName()); //设定相册上的角色名
		auto* m_pChaPhoto = dynamic_cast<CImage*>(_frmEditMotto->Find("imgMhead"));
		m_pChaPhoto->GetImage()->UnLoadImage();

		_chkForbidOfMottoFrm->SetIsChecked(_bForbid);
		_frmEditMotto->Refresh();
		_frmEditMotto->SetIsShow(!_frmEditMotto->GetIsShow()); //	Modify by alfred.shi 20080909
	}
}
//	点击面板按钮实现添加好友的功能。	暂不实现。
//	Add by alfred.shi 20080903	begin
void CChat::_evtAddFriend(CGuiData* pSender, int x, int y, MouseClickState key) {
	CCharacter* pMainCha = CGameScene::GetMainCha();
	if (pMainCha && pMainCha->getGameAttr() && pMainCha->getGameAttr()->get(ATTR_LV) >= 7) {
		if (_curSelectMember)
			CS_Frnd_Invite(_curSelectMember->GetName());
	} else {
		// 七级以下禁止添加好友
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_865));
	}
}

void CChat::_evtViewFriend(CGuiData* pSender, int x, int y, MouseClickState key) {
	CForm* f = CFormMgr::s_Mgr.Find("frmchainfo2");
	if (f) {
		f->SetIsShow(!f->GetIsShow());
	}
}

void CChat::_evtViewDetailInfo(CGuiData* pSender, int x, int y, MouseClickState key) {
	CForm* f = CFormMgr::s_Mgr.Find("frmOtherInfo");
	if (f) {
		f->SetIsShow(!f->GetIsShow());
	}
}
//	End.

void CChat::_evtChangeMotto(CGuiData* pSender, int x, int y, MouseClickState key) {
	auto* pbtnAdd = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnYes"));
	auto* pbtnAdd1 = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnEgg"));
	auto* pbtnAdd2 = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnFlower"));
	auto* pbtnAdd3 = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnxiangce01"));
	auto* pbtnAdd4 = dynamic_cast<CTextButton*>(_frmEditMotto->Find("btnxiangce02"));
	if (pSender == pbtnAdd) {
		stPersonInfo m_ShowInfo;
		char birthmonth[20] = "";
		char birthday[20] = "";
		char tradeGold[128] = "";
		size_t len = 0;
		CForm* m_pFrmSInfo = _frmEditMotto;

		auto* m_pEdtTradeGold = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("edtTradeGold"));

		auto* m_pChaSex = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingbie"));

		auto* m_pChaAge = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chanianling"));

		auto* m_pChaName = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingming"));

		auto* m_pAnimalZodiac = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengxiao"));

		auto* m_pBloodType = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxuexing"));

		auto* m_pBirthMonth = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengriyue"));

		auto* m_pBirthDay = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengriri"));

		auto* m_pState = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengfen"));

		auto* m_pCity = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaquyu"));

		auto* m_pConstellation = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingzuo"));

		auto* m_pJob = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chajob"));

		auto* m_pIdiograph = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaqianming"));

		auto* m_pStopChat = dynamic_cast<CCheckBox*>(m_pFrmSInfo->Find("chkChat"));

		auto* m_pSupport = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chaxianhua"));

		auto* m_pOppose = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chajidan"));

		len = sizeof(tradeGold) > strlen(m_pEdtTradeGold->GetCaption()) ? strlen(m_pEdtTradeGold->GetCaption()) : sizeof(tradeGold);
		memcpy(tradeGold, m_pEdtTradeGold->GetCaption(), len);

		len = sizeof(m_ShowInfo.szMotto) > strlen(m_pIdiograph->GetCaption()) ? strlen(m_pIdiograph->GetCaption()) : sizeof(m_ShowInfo.szMotto);
		memcpy(m_ShowInfo.szMotto, m_pIdiograph->GetCaption(), len);

		m_ShowInfo.bShowMotto = len > 0 ? true : false;

		len = sizeof(m_ShowInfo.szSex) > strlen(m_pChaSex->GetCaption()) ? strlen(m_pChaSex->GetCaption()) : sizeof(m_ShowInfo.szSex);
		memcpy(m_ShowInfo.szSex, m_pChaSex->GetCaption(), len);
		m_ShowInfo.szSex[9] = '\0';
		// modify by ning.yan 2008-11-18 begin
		if (strcmp(m_ShowInfo.szSex, RES_STRING(CL_LANGUAGE_MATCH_966)) != 0 && strcmp(m_ShowInfo.szSex, RES_STRING(CL_LANGUAGE_MATCH_967)) != 0)
			g_pGameApp->MsgBox(RES_STRING(CL_LANGUAGE_MATCH_968));
		// end
		//MessageBox( NULL, "性别必须是:男或女!", RES_STRING(CL_LANGUAGE_MATCH_25), MB_OK );

		m_ShowInfo.sAge = atoi(m_pChaAge->GetCaption());

		len = sizeof(m_ShowInfo.szName) > strlen(m_pChaName->GetCaption()) ? strlen(m_pChaName->GetCaption()) : sizeof(m_ShowInfo.szName);
		memcpy(m_ShowInfo.szName, m_pChaName->GetCaption(), len);

		len = sizeof(m_ShowInfo.szAnimalZodiac) > strlen(m_pAnimalZodiac->GetCaption()) ? strlen(m_pAnimalZodiac->GetCaption()) : sizeof(m_ShowInfo.szAnimalZodiac);
		memcpy(m_ShowInfo.szAnimalZodiac, m_pAnimalZodiac->GetCaption(), len);

		len = sizeof(m_ShowInfo.szBloodType) > strlen(m_pBloodType->GetCaption()) ? strlen(m_pBloodType->GetCaption()) : sizeof(m_ShowInfo.szBloodType);
		memcpy(m_ShowInfo.szBloodType, m_pBloodType->GetCaption(), len);

		len = sizeof(birthmonth) > strlen(m_pBirthMonth->GetCaption()) ? strlen(m_pBirthMonth->GetCaption()) : sizeof(birthmonth);
		memcpy(birthmonth, m_pBirthMonth->GetCaption(), len);

		len = sizeof(birthday) > strlen(m_pBirthDay->GetCaption()) ? strlen(m_pBirthDay->GetCaption()) : sizeof(birthday);
		memcpy(birthday, m_pBirthDay->GetCaption(), len);

		//m_ShowInfo.iBirthday = atoi(birthmonth)<<16 |atoi(birthday);
		m_ShowInfo.iBirthday = (atoi(birthmonth) << 5) + atoi(birthday); //Modify by sunny.sun 20080825

		len = sizeof(m_ShowInfo.szState) > strlen(m_pState->GetCaption()) ? strlen(m_pState->GetCaption()) : sizeof(m_ShowInfo.szState);
		memcpy(m_ShowInfo.szState, m_pState->GetCaption(), len);

		len = sizeof(m_ShowInfo.szCity) > strlen(m_pCity->GetCaption()) ? strlen(m_pCity->GetCaption()) : sizeof(m_ShowInfo.szCity);
		memcpy(m_ShowInfo.szCity, m_pCity->GetCaption(), len);

		len = sizeof(m_ShowInfo.szConstellation) > strlen(m_pConstellation->GetCaption()) ? strlen(m_pConstellation->GetCaption()) : sizeof(m_ShowInfo.szConstellation);
		memcpy(m_ShowInfo.szConstellation, m_pConstellation->GetCaption(), len);

		len = sizeof(m_ShowInfo.szCareer) > strlen(m_pJob->GetCaption()) ? strlen(m_pJob->GetCaption()) : sizeof(m_ShowInfo.szCareer);
		memcpy(m_ShowInfo.szCareer, m_pJob->GetCaption(), len);

		m_ShowInfo.iSupport = atoi(m_pSupport->GetCaption());
		m_ShowInfo.iOppose = atoi(m_pSupport->GetCaption());
		m_ShowInfo.iSize = 1;
		m_ShowInfo.bPprevent = m_pStopChat->GetIsChecked();
		memset(m_ShowInfo.pAvatar, 0, sizeof(m_ShowInfo.pAvatar));

		CP_Change_PersonInfo(&m_ShowInfo, tradeGold);
		_frmEditMotto->Close();
	} else if (pSender == pbtnAdd1) // 鸡蛋
	{
		auto* m_pOppose = dynamic_cast<CLabel*>(_frmEditMotto->Find("chajidan"));
		int content = atoi(m_pOppose->GetCaption());
		char strcontent[128] = "";
		_snprintf_s(strcontent, _TRUNCATE, "%d", content + 1);
		m_pOppose->SetCaption(strcontent);
	} else if (pSender == pbtnAdd2) // 鲜花
	{
		auto* m_pSupport = dynamic_cast<CLabel*>(_frmEditMotto->Find("chaxianhua"));
		int content = atoi(m_pSupport->GetCaption());
		char strcontent[128] = "";
		_snprintf_s(strcontent, _TRUNCATE, "%d", content + 1);
		m_pSupport->SetCaption(strcontent);
	} else if (pSender == pbtnAdd3) {
		CFormMgr& mgr = CFormMgr::s_Mgr;
		CForm* frmPhoto = mgr.Find("frmPhoto");
		if (!frmPhoto) {
			assert(0);
			return;
		}
		frmPhoto->SetIsShow(!frmPhoto->GetIsShow());
	}
}

bool CChat::_UpdateFrndInfo(CMember* pMember) {
	if (!pMember)
		return false;
	return true;
	//stPersonInfo m_ShowInfo;
}

bool CChat::_UpdateSelfInfo() {
	char pszCha[256] = {0};
	std::string strGuildName = "";
	char ChCharaterName[256] = "";
	char ChCharaterNickName[256] = ""; //称号
	char ChCharaterJob[256] = "";
	char ChCharaterLev[256] = "";
	char ChCharaterFame[256] = "";

	CCharacter* pCha = g_stUIBoat.GetHuman();
	if (!pCha)
		return false;

	SGameAttr* pCChaAttr = pCha->getGameAttr();
	if (!pCChaAttr)
		return false;

	//玩家名称
	_snprintf_s(ChCharaterName, _TRUNCATE, "%s", pCha->getName());

	_snprintf_s(ChCharaterNickName, _TRUNCATE, "%s", (char*)pCChaAttr->get(ATTR_TITLE));

	//公会
	if (CGuildData::GetGuildID()) {
		strGuildName = CGuildData::GetGuildName().c_str();
	}
	if (strGuildName.empty()) {
		strGuildName = RES_STRING(CL_LANGUAGE_MATCH_969);
	}
	//玩家职业
	_snprintf_s(ChCharaterJob, _TRUNCATE, "%s", g_GetJobName((short)pCChaAttr->get(ATTR_JOB)));

	//玩家等级
	_snprintf_s(ChCharaterLev, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_LV));

	// 声望
	_snprintf_s(ChCharaterFame, _TRUNCATE, "%lld", pCChaAttr->get(ATTR_FAME));

	CForm* m_pFrmSInfo = _frmEditMotto;

	auto* m_pLabName = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("labName"));
	auto* m_pChaguild = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chaguild"));
	auto* m_pChaLv = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chaLv"));
	auto* m_pChaJob = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chazhiye"));
	auto* m_pReputation = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chashengwang"));
	auto* m_pChaNickName = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chachenghao"));
	auto* m_pChaPhoto = dynamic_cast<CImage*>(m_pFrmSInfo->Find("imgMhead"));
	auto* m_pEdtTradeGold = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("edtTradeGold"));

	m_pEdtTradeGold->SetCaption(pCha->getSecondName());
	m_pLabName->SetCaption(ChCharaterName);
	m_pChaguild->SetCaption(strGuildName.c_str());
	m_pChaLv->SetCaption(ChCharaterLev);
	m_pChaJob->SetCaption(ChCharaterJob);
	// 声望不知道是什么
	m_pReputation->SetCaption(ChCharaterFame);

	m_pChaNickName->SetCaption(ChCharaterNickName);

	m_pChaPhoto->GetImage()->UnLoadImage();

	// 清空数据
	auto* m_pAttention = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chaguanzhu"));
	m_pAttention->SetCaption("");

	auto* m_pChaSex = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingbie"));
	m_pChaSex->SetCaption("");

	auto* m_pChaAge = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chanianling"));
	m_pChaAge->SetCaption("");

	auto* m_pChaName = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingming"));
	m_pChaName->SetCaption("");

	auto* m_pAnimalZodiac = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengxiao"));
	m_pAnimalZodiac->SetCaption("");

	auto* m_pBloodType = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxuexing"));
	m_pBloodType->SetCaption("");

	auto* m_pBirthMonth = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengriyue"));
	m_pBirthMonth->SetCaption("");
	auto* m_pBirthDay = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengriri"));
	m_pBirthDay->SetCaption("");
	auto* m_pState = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengfen"));
	m_pState->SetCaption("");
	auto* m_pCity = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaquyu"));
	m_pCity->SetCaption("");
	auto* m_pConstellation = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingzuo"));
	m_pConstellation->SetCaption("");
	auto* m_pJob = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chajob"));
	m_pJob->SetCaption("");
	auto* m_pIdiograph = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaqianming"));
	m_pIdiograph->SetCaption("");
	auto* m_pStopChat = dynamic_cast<CCheckBox*>(m_pFrmSInfo->Find("chkChat"));

	auto* m_pSupport = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chaxianhua"));
	m_pSupport->SetCaption("");
	auto* m_pOppose = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chajidan"));
	m_pOppose->SetCaption("");
	return true;
}

void CChat::_OnTimerFlash(CGuiTime* pSender) {
	CTextButton* pBtn = CStartMgr::GetShowQQButton();
	if (!pBtn)
		return;
	if (CTalkSessionFormMgr::hasFlashSession()) {
		if (!_frmQQ->GetIsShow()) {
			static int i = 0;
			pBtn->GetImage()->SetFrame(i);
			i = (i == 0) ? 3 : 0;
		} else {
			pBtn->GetImage()->SetFrame(0);
		}
		if (!_pSessionNode->GetIsExpand()) {
			static bool bColor = false;
			_pSessionNode->GetItem()->SetColor((bColor) ? 0xff00ff00 : COLOR_BLACK);
			bColor = !bColor;
		} else {
			_pSessionNode->GetItem()->SetColor(COLOR_BLACK);
		}
	} else {
		pBtn->GetImage()->SetFrame(0);
		_pSessionNode->GetItem()->SetColor(COLOR_BLACK);
	}
	CTalkSessionFormMgr::OnFlashSession();
}

// Add by lark.li 20080805 begin
void CChat::_OnDragMouseMove(CForm* pTargetForm, CTreeView* pTree, CTreeNodeObj* pNode1, CTreeNodeObj* pNode2, CItemObj* pItem, int x, int y, MouseClickState key) {
	auto* pTreeNode1 = dynamic_cast<CTreeGridNode*>(pNode1);
	auto* pTreeNode2 = dynamic_cast<CTreeGridNode*>(pNode2);

	if (pTreeNode1 && pTreeNode2) {
		if (g_stUIChat.IsFrndGroupNode(pTreeNode2)) {
			if (pTreeNode1 != pTreeNode2) {
				if (g_stUIChat._pDropTreeNode != pTreeNode2) {
					if (g_stUIChat._pDropTreeNode) {
						if (g_stUIChat._pDropTreeNode->GetItem())
							g_stUIChat._pDropTreeNode->GetItem()->SetColor(0xff000000);
					}

					g_stUIChat._pDropTreeNode = pTreeNode2;
					if (g_stUIChat._pDropTreeNode->GetItem())
						g_stUIChat._pDropTreeNode->GetItem()->SetColor(0xffff0000);
				}
			}
		} else {
			if (g_stUIChat._pDropTreeNode) {
				if (g_stUIChat._pDropTreeNode->GetItem())
					g_stUIChat._pDropTreeNode->GetItem()->SetColor(0xff000000);
				g_stUIChat._pDropTreeNode = nullptr;
			}
		}
	}

	if (!pTreeNode2) {
		if (g_stUIChat._pDropTreeNode) {
			if (g_stUIChat._pDropTreeNode->GetItem())
				g_stUIChat._pDropTreeNode->GetItem()->SetColor(0xff000000);
			g_stUIChat._pDropTreeNode = nullptr;
		}
	}
}
// End

void CChat::_OnDragEnd(CForm* pTargetForm, CTreeView* pTree, CTreeNodeObj* pNode1, CTreeNodeObj* pNode2, CItemObj* pItem, int x, int y, MouseClickState key) {
	// Add by lark.li 20080805 begin
	auto* pTreeNode1 = dynamic_cast<CTreeGridNode*>(pNode1);
	auto* pTreeNode2 = dynamic_cast<CTreeGridNode*>(pNode2);

	if (pTreeNode1 && pTreeNode2) {
		if (g_stUIChat.IsFrndGroupNode(pTreeNode2)) {
			if (g_stUIChat._pDropTreeNode != pTreeNode1) {
				if (g_stUIChat._pDropTreeNode) {
					if (g_stUIChat._pDropTreeNode->GetItem())
						g_stUIChat._pDropTreeNode->GetItem()->SetColor(0xff000000);
				}

				auto* pText = dynamic_cast<CTextGraph*>(pItem);
				if (pText) {
					auto* pMember = static_cast<CMember*>(pText->GetPointer());
					if (!pMember)
						return;

					CP_Frnd_Move_Group(pMember->GetID(), pTreeNode1->GetCaption(), pTreeNode2->GetCaption());
				}
			}
		}
		return;
	}

	if (g_stUIChat._pDropTreeNode) {
		if (g_stUIChat._pDropTreeNode->GetItem())
			g_stUIChat._pDropTreeNode->GetItem()->SetColor(0xff000000);
		g_stUIChat._pDropTreeNode = nullptr;
	}

	// End

	CTalkSessionForm* pSessForm;
	pSessForm = CTalkSessionFormMgr::GetSessionFormByForm(pTargetForm);
	if (!pSessForm)
		return;
	std::string strCaption = pNode1->GetCaption();
	if (strCaption == RES_STRING(CL_LANGUAGE_MATCH_465))
		return;
	auto* pText = dynamic_cast<CTextGraph*>(pItem);
	if (pText) {
		auto* pMember = static_cast<CMember*>(pText->GetPointer());
		if (!pMember)
			return;
		if (pSessForm->IsActiveSession()) {
			CS_Sess_Add(pSessForm->GetSessionID(), pMember->GetName());
		} else {
			if (std::string(pMember->GetName()) == pSessForm->GetMemberByIndex(0)->GetName()) {
				g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_488));
				return;
			}
			pSessForm->AddMemberToBuffer(pMember->GetName());
			const auto** pArrName = new const char*[1];
			pArrName[0] = pSessForm->GetMemberByIndex(0)->GetName();
			CS_Sess_Create(pArrName, 1);
			SAFE_DELETE_ARRAY(pArrName);
		}
	} else {
		auto* pGridNode = dynamic_cast<CTreeGridNode*>(pNode1);
		if (!pGridNode)
			return;
		int nCount = 0;
		for (DWORD i = 0; i < pGridNode->GetItemCount(); i++) {
			pText = dynamic_cast<CTextGraph*>(pGridNode->GetItemByIndex(i));
			if (!pText)
				continue;
			auto* pMember = static_cast<CMember*>(pText->GetPointer());
			if (!pMember)
				continue;
			std::string strName = pMember->GetName();
			if (pSessForm->hasMember(strName))
				continue;
			nCount++;
			if (pSessForm->IsActiveSession()) {
				CS_Sess_Add(pSessForm->GetSessionID(), strName.c_str());
			} else {
				pSessForm->AddMemberToBuffer(strName);
			}
		}
		if (nCount > 0 && !pSessForm->IsActiveSession()) {
			const auto** pArrName = new const char*[1];
			pArrName[0] = pSessForm->GetMemberByIndex(0)->GetName();
			CS_Sess_Create(pArrName, 1);
			SAFE_DELETE_ARRAY(pArrName);
		}
	}
}

void CChat::_OnFrndDeleteConfirm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes)
		return;

	auto* pSelect = static_cast<stSelectBox*>(pSender->GetForm()->GetPointer());
	if (pSelect) {
		CS_Frnd_Delete(pSelect->dwTag);
	}
}

void CChat::_OnMasterDeleteConfirm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes)
		return;

	auto* pSelect = static_cast<stSelectBox*>(pSender->GetForm()->GetPointer());
	if (pSelect && pSelect->pointer) {
		CS_MasterDel((const char*)pSelect->pointer, pSelect->dwTag);
	}
}

void CChat::_OnStudentDeleteConfirm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	if (nMsgType != CForm::mrYes)
		return;

	auto* pSelect = static_cast<stSelectBox*>(pSender->GetForm()->GetPointer());
	if (pSelect && pSelect->pointer) {
		CS_PrenticeDel((const char*)pSelect->pointer, pSelect->dwTag);
	}
}

void CChat::MasterAsk(const char* szName, DWORD dwCharID) {
	char szBuffer[512] = {0};
	_snprintf_s(szBuffer, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_860), szName);

	stSelectBox* pSelectBox = g_stUIBox.ShowSelectBox(_OnStudentAskConfirm, szBuffer, true);
	pSelectBox->pointer = (void*)szName;
	pSelectBox->dwTag = dwCharID;
}

void CChat::_OnStudentAskConfirm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	auto* pSelect = static_cast<stSelectBox*>(pSender->GetForm()->GetPointer());
	if (pSelect && pSelect->pointer) {
		CS_MasterAsr(nMsgType == CForm::mrYes, (const char*)pSelect->pointer, pSelect->dwTag);
	}
}

void CChat::PrenticeAsk(const char* szName, DWORD dwCharID) {
	char szBuffer[512] = {0};
	_snprintf_s(szBuffer, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_861), szName);

	stSelectBox* pSelectBox = g_stUIBox.ShowSelectBox(_OnMasterAskConfirm, szBuffer, true);
	pSelectBox->pointer = (void*)szName;
	pSelectBox->dwTag = dwCharID;
}

void CChat::_OnMasterAskConfirm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	auto* pSelect = static_cast<stSelectBox*>(pSender->GetForm()->GetPointer());
	if (pSelect && pSelect->pointer) {
		CS_PrenticeAsr(nMsgType == CForm::mrYes, (const char*)pSelect->pointer, pSelect->dwTag);
	}
}

// Add by lark.li 20080707 begin
void CChat::CaptainConfirm(DWORD dwTeamID) {
	stMsgTimeBox* pBox = g_stUIBox.ShowMsgTime(_OnCaptainAskConfirm, RES_STRING(CL_UI_CHAT_CPP_00001), 5);
	pBox->teamID = (short)dwTeamID;
}

void CChat::_OnCaptainAskConfirm(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	auto* pBox = static_cast<stMsgTimeBox*>(pSender->GetForm()->GetPointer());
	if (pBox) {
		pBox->frmDialog->Close();
		CS_CaptainConfirmAsr(true, pBox->teamID);
	}
}
void CChat::ResetSelfInfo(const stPersonInfo* _info) {
	
	// _info不可能为NULL，因为外边是引用传过来的
	// 更新自己的信息
	if (!_info)
		stPersonInfo* _info = &_SelfInfo;
	else
		_SelfInfo = *_info;
	// 更新基本属性
	_UpdateSelfInfo();

	char birthmonth[20] = "";
	char birthday[20] = "";
	char tradeGold[128] = "";
	_W64 unsigned int len = 0;

	CForm* m_pFrmSInfo = _frmEditMotto;

	auto* m_pEdtTradeGold = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("edtTradeGold"));

	auto* m_pAttention = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chaguanzhu"));

	auto* m_pChaSex = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingbie"));

	auto* m_pChaAge = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chanianling"));

	auto* m_pChaName = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingming"));

	auto* m_pAnimalZodiac = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengxiao"));

	auto* m_pBloodType = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxuexing"));

	auto* m_pBirthMonth = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengriyue"));

	auto* m_pBirthDay = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengriri"));

	auto* m_pState = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chashengfen"));

	auto* m_pCity = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaquyu"));

	auto* m_pConstellation = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaxingzuo"));

	auto* m_pJob = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chajob"));

	auto* m_pIdiograph = dynamic_cast<CEdit*>(m_pFrmSInfo->Find("chaqianming"));

	auto* m_pStopChat = dynamic_cast<CCheckBox*>(m_pFrmSInfo->Find("chkChat"));

	auto* m_pSupport = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chaxianhua"));

	auto* m_pOppose = dynamic_cast<CLabel*>(m_pFrmSInfo->Find("chajidan"));

	char buff[64] = "";

	//m_pEdtTradeGold->SetCaption(_info->szMotto);

	_snprintf_s(buff, _TRUNCATE, "%d", _info->iSupport);
	m_pSupport->SetCaption(buff);
	_snprintf_s(buff, _TRUNCATE, "%d", _info->iOppose);
	m_pOppose->SetCaption(buff);

	// 关注
	srand((unsigned int)time(nullptr));
	_snprintf_s(buff, _TRUNCATE, "%d", rng.uniform(0, 1000 - 1));
	m_pAttention->SetCaption(buff);

	m_pChaSex->SetCaption(_info->szSex);
	_snprintf_s(buff, _TRUNCATE, "%d", _info->sAge);
	m_pChaAge->SetCaption(buff);
	m_pChaName->SetCaption(_info->szName);
	m_pAnimalZodiac->SetCaption(_info->szAnimalZodiac);
	m_pBloodType->SetCaption(_info->szBloodType);

	//_snprintf_s(buff, _TRUNCATE, "%d",(_info->iBirthday &&0xffff0000));
	//m_pBirthMonth->SetCaption(buff);
	//_snprintf_s(buff, _TRUNCATE, "%d",(_info->iBirthday &&0xffff));
	//Modify by sunny.sun 20080825
	_snprintf_s(buff, _TRUNCATE, "%d", (_info->iBirthday >> 5));
	m_pBirthMonth->SetCaption(buff);
	_snprintf_s(buff, _TRUNCATE, "%d", (_info->iBirthday - (_info->iBirthday >> 5 << 5)));
	m_pBirthDay->SetCaption(buff);

	m_pState->SetCaption(_info->szState);
	m_pCity->SetCaption(_info->szCity);
	m_pConstellation->SetCaption(_info->szConstellation);
	m_pJob->SetCaption(_info->szCareer);
	m_pIdiograph->SetCaption(_info->szMotto);

	//if(_info->bShowMotto)
	//	m_pStopChat->SetCaption("有");
	//else
	//	m_pStopChat->SetCaption("没");

	auto* m_pChaPhoto = dynamic_cast<CImage*>(m_pFrmSInfo->Find("imgMhead"));
	CChatIconInfo* pIconInfo = GetChatIconInfo(g_stUIChat._dwSelfIcon);
	if (pIconInfo) {
		CGuiPic* pPic = m_pChaPhoto->GetImage();
		std::string strPath = "texture/ui/HEAD/";
		if (strcmp(_info->szSex, RES_STRING(CL_LANGUAGE_MATCH_967)) == 0) // ning.yan modify
			pPic->LoadImage((strPath + "1.bmp").c_str(), 100, 100, 0, pIconInfo->nBigX, pIconInfo->nBigY);
		else
			pPic->LoadImage((strPath + "2.bmp").c_str(), 100, 100, 0, pIconInfo->nBigX, pIconInfo->nBigY);

		//pPic->LoadImage((strPath+pIconInfo->szBig).c_str(),BIG_ICON_SIZE,BIG_ICON_SIZE,0,pIconInfo->nBigX,pIconInfo->nBigY);
	}
}
// End
