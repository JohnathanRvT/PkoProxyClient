//------------------------------------------------------------------------
//	2005.3.29	Arcol	create this file
//	2005.4.5	Arcol	add session node and handle the flash event in the QQ form
//------------------------------------------------------------------------

#include "StdAfx.h"
#include "uiformmgr.h"
#include "uilabel.h"
#include "uicloneform.h"
#include "UITeam.h"
#include "NetChat.h"
#include "UIChat.h"
#include "UIEdit.h"
#include "GameApp.h"
#include "StringLib.h"
#include "Character.h"
#include "UIGrid.h"
#include "uitreeview.h"
#include "UITextParse.h"
#include "talksessionformmgr.h"
#include "UIMenu.h"
#include "UICozeForm.h"
#include "ChatRecord.h"

#define MINIMIZE_FORM_WIDTH 185
#define MINIMIZE_FORM_HEIGHT 20
#define INFORM_FORM_SHOW_SX 100
#define NORMAL_FORM_OFFSET_WIDTH 33
#define NORMAL_FORM_OFFSET_HEIGHT 20
#define NORMAL_FORM_SHOW_SX 105
#define NORMAL_FORM_SHOW_SY 90

static CCloneForm dupe1;
static CCloneForm dupe2;
static CCloneForm dupe3;
static CCloneForm dupe4;

bool _Error(const char* strInfo, const char* strFormName, const char* strCompentName) {
	LG("gui", strInfo, strFormName, strCompentName);
	return false;
}

//------------------------------------------------------------------------
//	CSessionMember类实现
//------------------------------------------------------------------------

CSessionMember::CSessionMember(stNetSessCreate* pMember)
	: _strName(pMember->szChaName), _strMotto(pMember->szMotto),
	  _nID(pMember->lChaID), _nIcon_id(pMember->sIconID) {}

CMenu* CTalkSessionForm::m_playerMouseRight = nullptr;

//------------------------------------------------------------------------
//	CTalkSessionForm类实现
//------------------------------------------------------------------------

CTalkSessionForm::CTalkSessionForm(DWORD sessionID, eTalkSessionStyle style) : m_dwSessioinID(sessionID), m_strWaitForSend(""), m_bActive(true), m_pForm(nullptr), m_pEdit(nullptr),
																			   m_strTalkMsg(""), /*m_pList(NULL),*/ m_pInformTextBtn(nullptr), m_pInformTitle(nullptr), m_nShowChannel(NULL),
																			   m_nShowSx(NULL), m_nShowSy(NULL), m_strWaitForAdded(""), m_bFlash(false), m_nFlashTimes(0), m_pTimer1(nullptr), m_pTimer2(nullptr) {
	//建立Form
	m_bActive = (m_dwSessioinID != 0) ? true : false;
	CTextButton* btn;
	static CForm* frmRemind = CFormMgr::s_Mgr.Find("frmRemind");
	if (!frmRemind) {
		if (!frmRemind)
			Error(RES_STRING(CL_LANGUAGE_MATCH_410));
		return;
	}
	dupe1.SetSample(frmRemind);
	m_pInformForm = dupe1.Clone();
	m_pInformTextBtn = dynamic_cast<CTextButton*>(m_pInformForm->Find("btnCheck"));
	if (!m_pInformTextBtn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pInformForm->GetName(), "btnCheck");
		return;
	}
	m_pInformTextBtn->evtMouseClick = _OnMouseInformButton;
	m_pInformTextBtn->SetPointer(this);
	m_pInformTitle = dynamic_cast<CLabelEx*>(m_pInformForm->Find("labTitle"));
	if (!m_pInformTitle) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pInformForm->GetName(), "labTitle");
		return;
	}
	btn = dynamic_cast<CTextButton*>(m_pInformForm->Find("btnYes"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pInformForm->GetName(), "btnYes");
		return;
	}
	btn->evtMouseClick = _OnMouseInformButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pInformForm->Find("btnNo"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pInformForm->GetName(), "btnNo");
		return;
	}
	btn->evtMouseClick = _OnMouseQuitButton;
	btn->SetPointer(this);

	static CForm* frmChat = CFormMgr::s_Mgr.Find("frmChat");
	if (!frmChat) {
		if (!frmChat)
			Error(RES_STRING(CL_LANGUAGE_MATCH_412));
		return;
	}
	dupe2.SetSample(frmChat);
	m_pNormalForm = dupe2.Clone();
	m_pNormalForm->SetPointer(this);
	((CGuiData*)m_pNormalForm)->SetPointer(this);
	POINT pt = CTalkSessionFormMgr::GetNormalFormPosition(sessionID);
	m_pNormalForm->SetPos(pt.x, pt.y);
	m_pNormalForm->SetHotKeyHandler(_OnHotKey);
	m_bQuitFlag = false;
	m_pNormalForm->evtEscClose = GuiFormEscCloseEvent;
	btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnMin"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "btnMin");
		return;
	}
	btn->evtMouseClick = _OnMouseMinimizeButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnClose"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "btnClose");
		return;
	}
	btn->evtMouseClick = _OnMouseCloseButton;
	btn->SetPointer(this);
	//btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnCloseD"));
	//if( !btn )
	//{
	//	_Error("chat.clu界面<%s>上找不到控件<%s>", m_pNormalForm->GetName(), "btnCloseD" );
	//	return;
	//}
	//btn->evtMouseClick=_OnMouseQuitButton;
	//btn->pTag=this;
	btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnChat"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "btnChat");
		return;
	}
	btn->evtMouseClick = _OnMouseSaveButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnSend"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "btnSend");
		return;
	}
	btn->evtMouseClick = _OnMouseSendButton;
	btn->SetPointer(this);
	m_pNormalEdit = dynamic_cast<CEdit*>(m_pNormalForm->Find("edtChat"));
	if (!m_pNormalEdit) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "edtChat");
		return;
	}
	m_pNormalEdit->SetEnterButton(btn);
	m_pNormalEdit->SetIsParseText(true);
	m_pNormalEdit->SetIsWrap(true);
	m_pNormalEdit->Init();
	//m_pNormalList = dynamic_cast<CList*>(m_pNormalForm->Find("memCtrl"));
	//if( !m_pNormalList )
	//{
	//	_Error("chat.clu界面<%s>上找不到控件<%s>", m_pNormalForm->GetName(), "memCtrl" );
	//	return;
	//}
	//m_pNormalList->SetRowHeight(20);

	m_pNormalMemo = dynamic_cast<CRichMemo*>(m_pNormalForm->Find("memChat"));
	if (!m_pNormalMemo) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "memChat");
		return;
	}
	m_pNormalMemo->SetAutoScroll(true);
	m_pNormalMemo->Init();
	RECT rc;
	rc.left = 1;
	rc.top = 1;
	rc.right = 15;
	rc.bottom = 10;
	m_pNormalMemo->SetClipRect(rc);

	btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnBrow"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "btnBrow");
		return;
	}
	btn->evtMouseClick = _OnMouseFaceListButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnClean"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "btnClean");
		return;
	}
	btn->evtMouseClick = _OnMouseCleanText;
	btn->SetPointer(this);
	m_pFaceGrid = dynamic_cast<CGrid*>(m_pNormalForm->Find("grdFace"));
	if (!m_pFaceGrid) {
		Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "grdFace");
		return;
	}
	m_pFaceGrid->evtSelectChange = _OnMouseSelectFace;
	m_pFaceGrid->evtLost = _OnFaceLostFocus;
	m_pFaceGrid->SetPointer(this);
	m_pNormalNameList = dynamic_cast<CList*>(m_pNormalForm->Find("lstChat"));
	if (!m_pNormalNameList) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "lstChat");
		return;
	}
	m_pNormalNameList->SetRowHeight(16);
	m_pNormalNameList->evtListMouseDB = _OnMouseRightPlayerMenu;
	m_playerMouseRight = CMenu::FindMenu("ListMouseRight");
	if (!m_playerMouseRight) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "ListMouseRight");
		return;
	}
	m_playerMouseRight->evtListMouseDown = _OnPlayerSelect;
	btn = dynamic_cast<CTextButton*>(m_pNormalForm->Find("btnQuit"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "btnQuit");
		return;
	}
	btn->evtMouseClick = _OnMouseQuitButton;
	btn->SetPointer(this);

	static CForm* frmChatMd = CFormMgr::s_Mgr.Find("frmChatMd");
	if (!frmChatMd) {
		if (!frmChatMd)
			Error(RES_STRING(CL_LANGUAGE_MATCH_413));
		return;
	}
	dupe3.SetSample(frmChatMd);
	m_pSmallForm = dupe3.Clone();
	btn = dynamic_cast<CTextButton*>(m_pSmallForm->Find("btnMin"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pSmallForm->GetName(), "btnMin");
		return;
	}
	btn->evtMouseClick = _OnMouseMinimizeButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pSmallForm->Find("btnReturn"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pSmallForm->GetName(), "btnReturn");
		return;
	}
	btn->evtMouseClick = _OnMouseNormalButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pSmallForm->Find("btnClose"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pSmallForm->GetName(), "btnClose");
		return;
	}
	btn->evtMouseClick = _OnMouseQuitButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pSmallForm->Find("btnSend"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pSmallForm->GetName(), "btnSend");
		return;
	}
	btn->evtMouseClick = _OnMouseSendButton;
	btn->SetPointer(this);
	m_pSmallEdit = dynamic_cast<CEdit*>(m_pSmallForm->Find("edtTradeGold"));
	if (!m_pSmallEdit) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "edtTradeGold");
		return;
	}
	m_pSmallEdit->SetEnterButton(btn);
	//m_pSmallList = dynamic_cast<CList*>(m_pSmallForm->Find("memCtrl"));
	//if( !m_pSmallList )
	//{
	//	_Error("chat.clu界面<%s>上找不到控件<%s>", m_pSmallForm->GetName(), "memCtrl" );
	//	return;
	//}

	static CForm* frmChatMin = CFormMgr::s_Mgr.Find("frmChatMin");
	if (!frmChatMin) {
		if (!frmChatMin)
			Error(RES_STRING(CL_LANGUAGE_MATCH_414));
		return;
	}
	dupe4.SetSample(frmChatMin);
	m_pMinimizeForm = dupe4.Clone();
	m_pMinimizeForm->SetPos(0, 0);
	btn = dynamic_cast<CTextButton*>(m_pMinimizeForm->Find("btnMax"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pMinimizeForm->GetName(), "btnMax");
		return;
	}
	btn->evtMouseClick = _OnMouseNormalButton;
	btn->SetPointer(this);
	btn = dynamic_cast<CTextButton*>(m_pMinimizeForm->Find("btnClose"));
	if (!btn) {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pMinimizeForm->GetName(), "btnClose");
		return;
	}
	btn->evtMouseClick = _OnMouseCloseButton;
	btn->SetPointer(this);

	ChangeStyle(style);
	//if (m_pList) m_pList->GetItems()->Clear();
	if (m_pMemo)
		m_pMemo->Clear();
	if (m_pNameList)
		m_pNameList->GetItems()->Clear();
	if (m_pEdit)
		m_pEdit->SetCaption("");
}

CTalkSessionForm::~CTalkSessionForm() {
	if (m_pTimer1) {
		m_pTimer1->Release();
		m_pTimer1 = nullptr;
	}
	if (m_pTimer2) {
		m_pTimer2->Release();
		m_pTimer2 = nullptr;
	}
	//m_pInformForm->KillTimer(1);
	//m_pInformForm->KillTimer(2);
	m_pNormalMemo->Clear();
	//m_pNormalList->GetItems()->Clear();
	//m_pSmallList->GetItems()->Clear();
	//m_pSmallMemo->Clear();
	g_stUIChat.GetSessionNode()->DelItem((CItemObj*)m_pData);
	if (m_pForm)
		m_pForm->Close();
	dupe1.Release(m_pInformForm);
	dupe2.Release(m_pNormalForm);
	dupe3.Release(m_pSmallForm);
	dupe4.Release(m_pMinimizeForm);
	ClearMessages();
	ReleaseInformShowChannel();
}

int CTalkSessionForm::GetTotalMembers() const {
	return (int)m_Members.size();
}

CForm* CTalkSessionForm::GetForm() {
	return m_pForm;
}

bool CTalkSessionForm::hasMember(DWORD chaID) const {
	for (auto const& member : m_Members) {
		if (member->GetID() == chaID) {
			return true;
		}
	}
	return false;
}

bool CTalkSessionForm::hasMember(std::string name) const {
	for (auto const& member : m_Members) {
		if (member->GetName() == name) {
			return true;
		}
	}
	return false;
}

CSessionMember* CTalkSessionForm::GetMemberByIndex(int nIndex) const {
	if (nIndex >= (int)m_Members.size()) {
		return nullptr;
	};
	return m_Members[nIndex].get();
}

CSessionMember* CTalkSessionForm::GetMemberByID(DWORD chrID) const {
	for (auto const& member : m_Members) {
		if (member->GetID() == chrID) {
			return member.get();
		}
	}
	return nullptr;
}

void CTalkSessionForm::AddMembers(stNetSessCreate* pArrMembers[], int memberNum) {
	for (int i = 0; i < memberNum; i++) {
		if (g_stUIChat._dwSelfID == pArrMembers[i]->lChaID) {
			continue;
		}
		m_Members.push_back(std::make_unique<CSessionMember>(pArrMembers[i]));
	}
	m_playerMouseRight->SetIsShow(false);
	RefreshForm();
}

bool CTalkSessionForm::DelMember(DWORD chaID) {
	if (!hasMember(chaID)) {
		return false;
	}

	auto it = std::find_if(m_Members.begin(), m_Members.end(),
						   [&chaID](std::unique_ptr<CSessionMember> const& p) {
							   return p->GetID() == chaID;
						   });

	if (it != m_Members.end()) {
		m_Members.erase(it);
		m_playerMouseRight->SetIsShow(false);
		RefreshForm();
		return true;
	}
	return false;
}

bool CTalkSessionForm::ChangeStyle(eTalkSessionStyle style) {
	if (m_pForm)
		m_pForm->Close();
	if (m_pTimer1) {
		m_pTimer1->Release();
		m_pTimer1 = nullptr;
	}
	if (m_pTimer2) {
		m_pTimer2->Release();
		m_pTimer2 = nullptr;
	}
	//m_pInformForm->KillTimer(1);
	//m_pInformForm->KillTimer(2);
	//if (m_nShowChannel)
	//{
	//	CTalkSessionFormMgr::ReleaseShowChannel(m_nShowChannel);
	//	m_nShowChannel=0;
	//}
	ReleaseInformShowChannel();
	switch (style) {
	case eTalkSessionStyle::Hidden: {
		m_pForm = nullptr;
		m_pMemo = nullptr;
		m_pEdit = nullptr;
		m_pNameList = nullptr;
		//m_pList=NULL;
	} break;
	case eTalkSessionStyle::Inform: {
		//m_nShowChannel=CTalkSessionFormMgr::GetShowChannel();
		GetInformShowChannel();
		m_pForm = m_pInformForm;
		m_pMemo = nullptr;
		m_pEdit = nullptr;
		m_pNameList = nullptr;
		//m_pList=NULL;
		m_pTimer1 = CGuiTime::Create(100, _OnTimerInformShowing);
		m_pTimer1->SetUserID(1);
		m_pTimer1->SetUserPoint(this);
		//m_pForm->SetTimer(1,100,this);
		//m_pForm->evtOnTimer=_OnTimerInformShowing;
		m_nShowSx = INFORM_FORM_SHOW_SX + (m_nShowChannel - 1) * m_pInformForm->GetWidth();
		m_nShowSy = -m_pInformForm->GetHeight();
		m_pForm->SetPos(m_nShowSx, m_nShowSy);
		m_Style = style;
		m_pForm->Show();
		return true;
	} break;
	case eTalkSessionStyle::Normal: {
		m_pForm = m_pNormalForm;
		m_pMemo = m_pNormalMemo;
		if (m_pEdit) {
			m_pNormalEdit->SetCaption(m_pEdit->GetCaption());
		}
		m_pEdit = m_pNormalEdit;
		m_pNameList = m_pNormalNameList;
		//m_pList=m_pNormalList;
	} break;
	case eTalkSessionStyle::Small: {
		/*
			m_pForm=m_pSmallForm;
			//m_pMemo=m_pSmallMemo;
			if (m_pEdit)
			{
				m_pSmallEdit->SetCaption(m_pEdit->GetCaption());
			}
			m_pEdit=m_pSmallEdit;
			m_pNameList=NULL;
			//m_pList=m_pSmallList;
			*/
	} break;
	case eTalkSessionStyle::Minimized: {
		if (m_pMinimizeForm->GetTop() == 0 && m_pMinimizeForm->GetLeft() == 0) {
			POINT p = CTalkSessionFormMgr::GetMiniFormPosition(GetSessionID());
			m_pMinimizeForm->SetPos(p.x, p.y);
		}
		m_pForm = m_pMinimizeForm;
		m_pMemo = nullptr;
		m_pEdit = nullptr;
		m_pNameList = nullptr;
		//m_pList=NULL;
	} break;
	default:
		return false;
	}
	m_Style = style;
	if (m_Style == eTalkSessionStyle::Hidden)
		return true;
	RefreshForm();
	m_pForm->Show();
	return true;
}

int CTalkSessionForm::GetInformShowChannel() {
	if (!m_nShowChannel) {
		m_nShowChannel = CTalkSessionFormMgr::GetShowChannel();
	}
	return m_nShowChannel;
}

void CTalkSessionForm::ReleaseInformShowChannel() {
	if (m_nShowChannel) {
		CTalkSessionFormMgr::ReleaseShowChannel(m_nShowChannel);
		m_nShowChannel = 0;
	}
}

void CTalkSessionForm::RefreshForm() {
	if (m_Style == eTalkSessionStyle::Hidden) {
		return;
	}

	const int total = GetTotalMembers();
	char buf[100];
	std::string str;
	if (total == 1) {
		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_1083), GetMemberByIndex(0));
		str = buf;
	} else {
		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_417), total);
		str = buf;
	}

	auto pText = dynamic_cast<CLabelEx*>(m_pMinimizeForm->Find("labName"));
	if (pText) {
		pText->SetCaption(str.c_str());
	} else {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pMinimizeForm->GetName(), "labName");
	}

	if (total == 1) {
		if (strlen(GetMemberByIndex(0)->GetMotto()) > 0) {
			str = std::string("(") + GetMemberByIndex(0)->GetMotto() + std::string(")");
			str = StringLimit(str, 31);
			str = GetMemberByIndex(0)->GetName() + str;
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_1083), str.c_str());
		} else {
			_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_1083), GetMemberByIndex(0)->GetName());
		}
		str = buf;
	} else {
		_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_418), total);
		str = buf;
		for (int i = 0; i < total; i++) {
			str += GetMemberByIndex(i)->GetName();
			str += " ";
		}
		str = StringLimit(str, 36);
	}
	pText = dynamic_cast<CLabelEx*>(m_pNormalForm->Find("labName1"));
	if (pText) {
		pText->SetCaption(str.c_str());
	} else {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "labName1");
	}

	_snprintf_s(buf, _countof(buf), _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_419), total);
	str = buf;
	pText = dynamic_cast<CLabelEx*>(m_pNormalForm->Find("labParty"));
	if (pText) {
		pText->SetCaption(str.c_str());
	} else {
		_Error(RES_STRING(CL_LANGUAGE_MATCH_411), m_pNormalForm->GetName(), "labParty");
	}

	// Fill active players in chat list
	m_pNormalNameList->GetItems()->Clear();
	for (int i = 0; i < GetTotalMembers(); i++) {
		CItemRow* pItemRow = m_pNormalNameList->NewItem();
		auto* pItem = new CTextGraph(1);
		pItem->SetString(StringLimit(GetMemberByIndex(i)->GetName(), 11).c_str());
		pItem->SetHint(GetMemberByIndex(i)->GetName());
		pItem->SetColor(0xef5c3cd3);
		pItem->SetPointer(GetMemberByIndex(i));
		CChatIconInfo* pIconInfo = GetChatIconInfo(GetMemberByIndex(i)->GetIconID());
		if (pIconInfo) {
			CGuiPic* pPic = pItem->GetImage();
			std::string strPath = "texture/ui/HEAD/";
			pPic->LoadImage((strPath + pIconInfo->szSmall).c_str(), 16, 16, 0, pIconInfo->nSmallX, pIconInfo->nSmallY);
		}
		pItemRow->SetIndex(0, pItem);
		//m_pNormalNameList->Add(GetMemberByIndex(i)->GetName());
	}

	// Fill chat with messages
	if (m_pMemo) {
		m_pMemo->SetAutoScroll(FALSE);
		m_pMemo->Clear();

		for (auto const& msg : m_TalkMessages) {
			std::string strName = msg.name;
			if (msg.motto.empty()) {
				strName += ":";
			} else {
				strName += "(" + msg.motto + ")" + ":";
			}

			m_pMemo->AddText(strName.c_str(), msg.text.c_str(),
							 MEMO_COLOR_TYPE::TEXT_COLOR_BLACK, TEXT_FONT_9);
		}
		m_pMemo->SetAutoScroll(TRUE);
		m_pMemo->AutoScroll();
	}
	/*
	if (m_pMemo&&!m_TalkMsg.empty())
	{
		//m_pList->GetItems()->Clear();
		m_pMemo->Clear();
		char *buf=new char[m_strTalkMsg.length()+10];
		strncpy_s(buf,sizeof(buf),m_strTalkMsg.c_str(), _TRUNCATE);
		char *token=strtok(buf, "\n");
		while( token != NULL )
		{
			string str=token;
			DWORD color=0xFF000000;
			m_pMemo->AddText("abcd",str.c_str(),TEXT_COLOR_BLACK,TEXT_FONT_9);
			//while (!str.empty())
			//{
			//	string text;
			//	text=CutFaceText(str,41);
			//	CItemRow* pItemRow=m_pList->NewItem();
			//	CItemEx* pItem=new CItemEx(text.c_str(),color);
			//	pItem->SetAlignment(eAlignCenter);
			//	pItem->SetHeight(18);
			//	if (text.find ("#") !=-1)	
			//	{
			//		pItem->SetIsParseText(true);
			//	}
			//	pItemRow->SetBegin(pItem);
			//}
			//m_pList->Add(token);
			token = strtok(NULL, "\n");
		}
		//m_pList->GetScroll()->OnKeyDown(VK_END);
		//m_pMemo->GetScroll()->OnKeyDown(VK_END);
		delete buf;
	}
*/
	m_pForm->Refresh();
}

void CTalkSessionForm::StopFlash() {
	SetIsFlash(false);
	//CMember* pMember=NULL;
	//for (int i=0;i<GetTotalMembers();i++)
	//{
	//	string str=GetMemberByIndex(i)->GetName();
	//	CTeam* pTeam=g_stUIChat.GetTeamMgr()->Find(enumTeamFrnd);
	//	if (pTeam)
	//	{
	//		for (DWORD i=0;i<pTeam->GetCount();i++)
	//		{
	//			pMember=pTeam->GetMember(i);
	//			if (str==pMember->GetName())
	//			{
	//				pMember->SetIsFlash(false);
	//			}
	//		}
	//	}
	//	pTeam=g_stUIChat.GetTeamMgr()->Find(enumTeamGroup);
	//	if (pTeam)
	//	{
	//		for (DWORD i=0;i<pTeam->GetCount();i++)
	//		{
	//			pMember=pTeam->GetMember(i);
	//			if (str==pMember->GetName())
	//			{
	//				pMember->SetIsFlash(false);
	//			}
	//		}
	//	}
	//	pTeam=g_stUIChat.GetTeamMgr()->Find(enumTeamRoad);
	//	if (pTeam)
	//	{
	//		for (DWORD i=0;i<pTeam->GetCount();i++)
	//		{
	//			pMember=pTeam->GetMember(i);
	//			if (str==pMember->GetName())
	//			{
	//				pMember->SetIsFlash(false);
	//			}
	//		}
	//	}
	//}
}

void CTalkSessionForm::OnHotKeyShow() {
	StopFlash();
	ChangeStyle(eTalkSessionStyle::Normal);
}

void CTalkSessionForm::SetInformText(const char* name, const char* text) {
	char buf[100];
	_snprintf_s(buf, _countof(buf), _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_1084), name);
	m_pInformTitle->SetCaption(buf);
	std::string str = StringLimit(text, 22);
	m_pInformTextBtn->SetCaption(str.c_str());
}

void CTalkSessionForm::_OnMouseInformButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	pThis->StopFlash();
	pThis->ChangeStyle(eTalkSessionStyle::Normal);
}

void CTalkSessionForm::_OnMouseSmallButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	pThis->ChangeStyle(eTalkSessionStyle::Small);
}

void CTalkSessionForm::_OnMouseMinimizeButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	pThis->ChangeStyle(eTalkSessionStyle::Minimized);
}

void CTalkSessionForm::_OnMouseNormalButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	pThis->StopFlash();
	pThis->ChangeStyle(eTalkSessionStyle::Normal);
}

void CTalkSessionForm::_OnMouseQuitButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	CS_Sess_Leave(pThis->m_dwSessioinID);
	//if (pThis->m_nShowChannel)
	//{
	//	CTalkSessionFormMgr::ReleaseShowChannel(pThis->m_nShowChannel);
	//	pThis->m_nShowChannel=0;
	//}
	pThis->ReleaseInformShowChannel();
	pThis->StopFlash();

	CTalkSessionFormMgr::CloseSessionForm(pThis->m_dwSessioinID);
}

void CTalkSessionForm::_OnMouseCloseButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	pThis->StopFlash();
	pThis->ChangeStyle(eTalkSessionStyle::Hidden);
}

void CTalkSessionForm::_OnMouseSendButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	std::string sendText = pThis->m_pEdit->GetCaption();
	if (sendText.empty()) {
		return;
	}

	ChangeParseSymbol(sendText, g_TextParse.GetFaceCount());
	//FilterText(sendText,&g_stUIChat._strFilterTxt);
	CTextFilter::Filter(CTextFilter::DIALOG_TABLE, sendText);

	if (pThis->IsActiveSession()) {
		CS_Sess_Say(pThis->m_dwSessioinID, sendText.c_str());
		pThis->m_pEdit->SetCaption("");
	} else {
		const char** pArrName = new const char*[1];
		pArrName[0] = pThis->GetMemberByIndex(0)->GetName();
		CS_Sess_Create(pArrName, 1);
		SAFE_DELETE_ARRAY(pArrName);
		pThis->m_strWaitForSend += sendText + "\n";
		pThis->m_pEdit->SetCaption("");
	}

	// Add by ning.yan  20080726  随时保存发送的聊天内容  Begin
#if AUTOMATIC_MESSAGE_SAVING
	__time64_t t;
	_time64(&t);
	tm ltime;
	_gmtime64_s(&ltime, &t);
	char ch[12];
	_snprintf_s(ch, _TRUNCATE, "  %d:%d:%d", ltime.tm_hour + 8, ltime.tm_min, ltime.tm_sec);
	std::string time = ch;
	std::string strName = CGameScene::GetMainCha()->getName();
	std::string const strMotto = CGameScene::GetMainCha()->getSecondName();
	if (strMotto.empty()) {
		strName += ":" + time + "\r\n  ";
	} else {
		strName += +"(" + strMotto + "):" + time + "\r\n  ";
	}
	std::string const saveText = strName + sendText + "\r\n\r\n";
	for (DWORD i = 0; i < (DWORD)pThis->GetTotalMembers(); i++) {
		// "For loop handling multi-person chat" ning.yan  2008-11-13  need modify
		if (CChatRecord::Save(pThis->GetMemberByIndex(i)->GetName(), pThis->GetTotalMembers(), saveText)) {
#ifdef _DEBUG
			std::string str = RES_STRING(CL_LANGUAGE_MATCH_972) + CChatRecord::GetLastSavePath() + RES_STRING(CL_LANGUAGE_MATCH_973);
			CCozeForm::GetInstance()->OnSystemMsg(str.c_str());
		} else {
			CCozeForm::GetInstance()->OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_973));
#endif
		}
	}
#endif
	// End
}

void CTalkSessionForm::_OnTimerInformShowing(CGuiTime* pSender) {
	CTalkSessionForm* pThis = (CTalkSessionForm*)pSender->GetUserPoint();
	if (!pThis) {
		return;
	}

	if (pSender->GetUserID() == 1) {
		if (pThis->m_nShowSy < 0) {
			pThis->m_nShowSy += pThis->m_pInformForm->GetHeight() / 10;
			if (pThis->m_nShowSy > 0) {
				pThis->m_nShowSy = 0;
			}

			pThis->m_pInformForm->SetPos(pThis->m_nShowSx, pThis->m_nShowSy);
			pThis->m_pInformForm->Refresh();

			if (!pThis->m_pInformForm->GetIsShow()) {
				pThis->m_pInformForm->Show();
			}
		} else {
			pThis->m_pTimer1->Release();
			pThis->m_pTimer1 = nullptr;
			pThis->m_pTimer2 = CGuiTime::Create(2000, _OnTimerInformShowing);
			pThis->m_pTimer2->SetUserID(2);
			pThis->m_pTimer2->SetUserPoint(pThis);
			//pThis->m_pInformForm->KillTimer(1);
			//pThis->m_pInformForm->SetTimer(2,2000,pThis);
		}
	} else if (pSender->GetUserID() == 2) {
		pThis->ChangeStyle(eTalkSessionStyle::Minimized);
	}
}

void CTalkSessionForm::_OnMouseFaceListButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	int sx = x;
	int sy = y;
	if (x + pThis->m_pFaceGrid->GetWidth() > g_pGameApp->GetWindowWidth()) {
		sx = g_pGameApp->GetWindowWidth() - pThis->m_pFaceGrid->GetWidth();
	}
	if (y + pThis->m_pFaceGrid->GetHeight() > g_pGameApp->GetWindowHeight()) {
		sy = g_pGameApp->GetWindowHeight() - pThis->m_pFaceGrid->GetHeight();
	}

	sx -= pThis->m_pNormalForm->GetLeft();
	sy -= pThis->m_pNormalForm->GetTop();

	pThis->m_pFaceGrid->SetPos(sx, sy);
	pThis->m_pFaceGrid->Refresh();
	pThis->m_pFaceGrid->SetIsShow(true);
}

void CTalkSessionForm::_OnMouseCleanText(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis) {
		return;
	}

	pThis->m_pMemo->Clear();
	pThis->ClearMessages();
}

void CTalkSessionForm::_OnMouseSelectFace(CGuiData* pSender) {
	pSender->SetIsShow(false);
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis)
		return;
	if (strlen(pThis->m_pNormalEdit->GetCaption()) > 39)
		return;

	CGraph* pGraph = pThis->m_pFaceGrid->GetSelect();
	if (pGraph) {
		char buf[20];
		int index = pThis->m_pFaceGrid->GetSelectIndex();
		//pThis->m_pNormalEdit->SetIsParseText(true);
		pThis->m_pNormalEdit->SetActive(pThis->m_pNormalEdit);
		std::string str = pThis->m_pNormalEdit->GetCaption();
		_snprintf_s(buf, _TRUNCATE, "#%.2d", index);
		str += buf;
		pThis->m_pNormalEdit->SetCaption(str.c_str());
	}
}

void CTalkSessionForm::_OnFaceLostFocus(CGuiData* pSender) {
	pSender->SetIsShow(false);
}

void CTalkSessionForm::_OnMouseRightPlayerMenu(CGuiData* pSender, int x, int y, MouseClickState key) {
	if ((key & MouseClickState::RDown) != MouseClickState()) {
		auto* pList = dynamic_cast<CList*>(pSender);
		if (!pList) {
			return;
		}
		auto* pSelectItem = dynamic_cast<CTextGraph*>(pList->GetSelectItem()->GetBegin());
		if (!pSelectItem) {
			return;
		}

		int sx = x;
		int sy = y;
		if (x + m_playerMouseRight->GetWidth() > g_pGameApp->GetWindowWidth()) {
			sx = g_pGameApp->GetWindowWidth() - m_playerMouseRight->GetWidth();
		}
		if (y + m_playerMouseRight->GetHeight() > g_pGameApp->GetWindowHeight()) {
			sy = g_pGameApp->GetWindowHeight() - m_playerMouseRight->GetHeight();
		}

		m_playerMouseRight->SetPointer(pSelectItem->GetPointer());
		pList->GetForm()->PopMenu(m_playerMouseRight, sx, sy);
		//m_playerMouseRight->SetPos(sx,sy);
		//m_playerMouseRight->SetIsShow(true);
		//m_playerMouseRight->Refresh();
	}
}

void CTalkSessionForm::_OnPlayerSelect(CGuiData* pSender, int x, int y, MouseClickState key) {
	CMenuItem* pItem = m_playerMouseRight->GetSelectMenu();
	if (!pItem) {
		return;
	}

	std::string str = pItem->GetString();
	if (str == RES_STRING(FORMS_CHAT_CLU_000018)) {
		auto* pSessionMember = static_cast<CSessionMember*>(m_playerMouseRight->GetPointer());
		if (pSessionMember) {
			CS_Frnd_Invite(pSessionMember->GetName());
		}
	}
}

void CTalkSessionForm::_OnMouseSaveButton(CGuiData* pSender, int x, int y, MouseClickState key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (!pThis || !pThis->GetMemberByIndex(0)) {
		return;
	}

	auto const& messages = pThis->m_TalkMessages;

	size_t estimatedSize = 0;
	for_each(messages.cbegin(), messages.cend(),
			 [&estimatedSize](sTalkMsg const& msg) {
				 estimatedSize += msg.text.length() + msg.motto.length() + msg.name.length() + 10;
			 });

	std::string saveText;
	saveText.reserve(estimatedSize);
	for (const auto& TalkMessage : pThis->m_TalkMessages) {
		std::string strName = TalkMessage.name;
		if (TalkMessage.motto.length() == 0) {
			strName += ":\r\n  ";
		} else {
			strName += +"(" + TalkMessage.motto + ")" + ":\r\n  ";
		}
		saveText += strName + TalkMessage.text + "\r\n\r\n";
	}

	if (CChatRecord::Save(pThis->GetMemberByIndex(0)->GetName(), pThis->GetTotalMembers(), saveText)) {
		// Inform user successful save through system chat
		std::string str = RES_STRING(CL_LANGUAGE_MATCH_972) + CChatRecord::GetLastSavePath() + RES_STRING(CL_LANGUAGE_MATCH_973);
		CCozeForm::GetInstance()->OnSystemMsg(str.c_str());
	} else {
		// Inform user save failure through system chat
		CCozeForm::GetInstance()->OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_974));
	}
}

void CTalkSessionForm::AddMessage(DWORD chaID, std::string talkMsg) {
	std::string nameBuf = "";
	std::string secondNameBuf = "";
	CSessionMember* pMember = GetMemberByID(chaID);
	if (pMember) {
		nameBuf = pMember->GetName();
		secondNameBuf = pMember->GetMotto();
	} else {
		if (CGameScene::GetMainCha()) {
			nameBuf = CGameScene::GetMainCha()->getHumanName();
			secondNameBuf = CGameScene::GetMainCha()->getSecondName();
		} else {
			nameBuf = "自己";
			secondNameBuf = "";
		}
	}

	m_TalkMessages.emplace_back(sTalkMsg{nameBuf, secondNameBuf, talkMsg});
	//sTalkMsg& msg = m_TalkMessages.back();
	//msg.name = nameBuf;
	//msg.motto = secondNameBuf;
	//msg.text = talkMsg;

	RefreshForm();

	// Add by ning.yan  20080727  随时保存接收到的聊天内容  Begin
#ifdef AUTOMATIC_MESSAGE_SAVING
	// Save messages received from other players in chat
	if (msg.name != CGameScene::GetMainCha()->getName()) {
		__time64_t t;
		_time64(&t);
		tm ltime;
		_gmtime64_s(&ltime, &t);
		char ch[12];
		_snprintf_s(ch, _TRUNCATE, "  %d:%d:%d", ltime.tm_hour + 8, ltime.tm_min, ltime.tm_sec);
		std::string time = ch;
		std::string strName = msg.name;
		if (msg.motto.empty()) {
			strName += ":" + time + "\r\n  ";
		} else {
			strName += +"(" + msg.motto + ");" + time + "\r\n  ";
		}

		std::string saveText = strName + msg.text + "\r\n\r\n";

		for (DWORD i = 0; i < (DWORD)GetTotalMembers(); i++) {
			// TODO: ChatRecord::Save() opens/closes stream on each call
			// propose 1: keep stream open during chats lifetime instead
			// propose 2: cache messages and write less times
			if (CChatRecord::Save(GetMemberByIndex(i)->GetName(), GetTotalMembers(), saveText)) {
#ifdef _DEBUG
				// Inform user successful save through system chat
				std::string str = RES_STRING(CL_LANGUAGE_MATCH_972) + CChatRecord::GetLastSavePath() + RES_STRING(CL_LANGUAGE_MATCH_973);
				CCozeForm::GetInstance()->OnSystemMsg(str.c_str());
			} else { // Inform user save failure through system chat
				CCozeForm::GetInstance()->OnSystemMsg(RES_STRING(CL_LANGUAGE_MATCH_974));
#endif
			}
		}
	}
#endif
}

void CTalkSessionForm::SendBufferMsg() {
	if (!m_strWaitForSend.empty()) {
		char* buf = new char[m_strWaitForSend.length() + 10];
		strncpy_s(buf, sizeof(buf), m_strWaitForSend.c_str(), _TRUNCATE);
		char *token, *nexttoken; //??????,????????????? by Waiting 2009.07.06
		token = strtok_s(buf, "\n", &nexttoken);
		while (token != nullptr) {
			CS_Sess_Say(m_dwSessioinID, token);
			token = strtok_s(nullptr, "\n", &nexttoken);
		}
		m_strWaitForSend.clear();
		//delete buf;
		SAFE_DELETE_ARRAY(buf); // UI当机处理
	}
	if (!m_strWaitForAdded.empty()) {
		char* buf = new char[m_strWaitForAdded.length() + 10];
		strncpy_s(buf, sizeof(buf), m_strWaitForAdded.c_str(), _TRUNCATE);
		char *token, *nexttoken;
		token = strtok_s(buf, "\n", &nexttoken);
		while (token != nullptr) {
			CS_Sess_Add(GetSessionID(), token);
			token = strtok_s(nullptr, "\n", &nexttoken);
		}
		m_strWaitForAdded.clear();
		//delete buf;
		SAFE_DELETE_ARRAY(buf); // UI当机处理
								//CS_Sess_Add(GetSessionID(),m_strWaitForAdded.c_str());
								//m_strWaitForAdded.clear();
	}
}

void CTalkSessionForm::ClearMessages() {
	m_TalkMessages.clear();
}

bool CTalkSessionForm::_OnHotKey(CForm* pSender, char key) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pSender->GetPointer());
	if (pThis && (key == 'X' || key == 'x')) {
		pThis->_OnMouseSendButton(pSender, 0, 0, MouseClickState());
		return true;
	}
	return false;
}

void CTalkSessionForm::GuiFormEscCloseEvent(CForm* pForm) {
	CTalkSessionForm* pThis = ((CTalkSessionForm*)pForm->GetPointer());
	if (pThis) {
		_OnMouseCloseButton(pForm, 0, 0, MouseClickState());
	}
}

//------------------------------------------------------------------------
//	CTalkSessionFormMgr类实现
//------------------------------------------------------------------------

//RECT CTalkSessionFormMgr::INFORM_FORM;
//RECT CTalkSessionFormMgr::INFORM_TITLE;
//int CTalkSessionFormMgr::INFORM_TABLE_W;
//int CTalkSessionFormMgr::INFORM_TABLE_H;
//POINT CTalkSessionFormMgr::INFORM_TABLE_POSITION;
int CTalkSessionFormMgr::m_Channel[4] = {0, 0, 0, 0};
DWORD CTalkSessionFormMgr::m_SessionCounts = 0;
UCHAR CTalkSessionFormMgr::sApplyMember::numbers = 0;
std::vector<CTalkSessionForm*> CTalkSessionFormMgr::m_SessionFormsLink;

CTalkSessionFormMgr::CTalkSessionFormMgr() {
}

CTalkSessionFormMgr::~CTalkSessionFormMgr() {
	std::vector<CTalkSessionForm*>::iterator Iter;
	while (!m_SessionFormsLink.empty()) {
		Iter = m_SessionFormsLink.begin();
		//delete (*Iter);
		SAFE_DELETE(*Iter); // UI当机处理
		m_SessionFormsLink.erase(Iter);
	}
}

//CTalkSessionForm* CTalkSessionFormMgr::CreateSessionForm(CMember *pMember,eTalkSessionStyle style)
//{
//	vector <PS_Sessions>::iterator Iter;
//	for (Iter=m_SessionFormsLink.begin();Iter!=m_SessionFormsLink.end();Iter++)
//	{
//		if ((*Iter)->session->hasMember(pMember))
//		{
//			if ((*Iter)->session->GetTotalMembers()==1)
//			{
//				(*Iter)->session->ChangeStyle(enumTalkSessionNormal);
//				return (*Iter)->session;
//			}
//		}
//	}
//	if (m_SessionCounts>MAX_SESSION_FORM) return NULL;
//	CTalkSessionForm *pSessionForm=new CTalkSessionForm(enumTalkSessionNormal);
//	if (pSessionForm->AddMember(pMember)==false)
//	{
//		delete pSessionForm;
//		return NULL;
//	}
//	_sSessions *psSession=new _sSessions;
//	psSession->sessionID=++m_SessionIDCount;
//	psSession->session=pSessionForm;
//	m_SessionFormsLink.push_back(psSession);
//	return NULL;
//}

//bool CTalkSessionFormMgr::AddToSessionForm(CMember *pMember,CTalkSessionForm *session)
//{
//	if (!pMember||!session) return false;
//	return session->AddMember(pMember);
//}

//void CTalkSessionFormMgr::LeaveSessionForm(CTalkSessionForm *session)
//{
//	vector <PS_Sessions>::iterator Iter;
//	for (Iter=m_SessionFormsLink.begin();Iter!=m_SessionFormsLink.end();Iter++)
//	{
//		if ((*Iter)->session==session)
//		{
//			delete (*Iter)->session;
//			delete (*Iter);
//			m_SessionFormsLink.erase(Iter);
//			return;
//		}
//	}
//}

bool CTalkSessionFormMgr::AddToSession(DWORD sessionID, stNetSessCreate* pMember) {
	if (!pMember || sessionID == 0)
		return false;
	CTalkSessionForm* pSessionForm = GetSessionFormByID(sessionID);
	if (!pSessionForm)
		return false;
	pSessionForm->AddMembers(&pMember);
	//pSessionForm->SetInformText("系统消息",pMember.  );
	return true;
}

void CTalkSessionFormMgr::CloseSessionForm(DWORD sessionID) {
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->GetSessionID() == sessionID) {
			g_stUIChat.GetSessionNode()->DelItem((CItemObj*)((*Iter)->GetData()));
			g_stUIChat.GetTeamView()->Refresh();
			//delete (*Iter);
			SAFE_DELETE(*Iter); // UI当机处理
			m_SessionFormsLink.erase(Iter);
			--m_SessionCounts;
			return;
		}
	}
}

void CTalkSessionFormMgr::ClearAll() {
	std::vector<CTalkSessionForm*>::iterator Iter;
	while (!m_SessionFormsLink.empty()) {
		Iter = m_SessionFormsLink.begin();
		//delete (*Iter);
		SAFE_DELETE(*Iter); // UI当机处理
		m_SessionFormsLink.erase(Iter);
	}
	m_SessionCounts = 0;
}

void CTalkSessionFormMgr::LeaveSession(DWORD sessionID, DWORD chaID) {
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->GetSessionID() == sessionID) {
			if ((*Iter)->GetTotalMembers() == 1 && chaID == (*Iter)->GetMemberByIndex(0)->GetID()) {
				(*Iter)->SetActiveSession(false);
			} else {
				(*Iter)->DelMember(chaID);
			}
			return;
		}
	}
}

CTalkSessionForm* CTalkSessionFormMgr::GetSessionFormByIndex(int nIndex) {
	if (nIndex >= (int)m_SessionFormsLink.size())
		return nullptr;
	return m_SessionFormsLink[nIndex];
}

CTalkSessionForm* CTalkSessionFormMgr::GetSessionFormByID(DWORD sessionID) {
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->GetSessionID() == sessionID) {
			return *Iter;
		}
	}
	return nullptr;
}

CTalkSessionForm* CTalkSessionFormMgr::GetSessionFormByName(std::string name, int index) {
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->GetTotalMembers() == 1 && (*Iter)->hasMember(name)) {
			if (index == 0)
				return *Iter;
			--index;
		}
	}
	return nullptr;
}

CTalkSessionForm* CTalkSessionFormMgr::GetSessionFormByForm(CForm* form) {
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->m_pForm == form) {
			return *Iter;
		}
	}
	return nullptr;
}

void CTalkSessionFormMgr::ApplySession(sApplyMember pApplyMembers[]) {
	for (DWORD i = 0; i < m_SessionCounts; i++) {
		CTalkSessionForm* pSessionForm = GetSessionFormByIndex(i);
		if (pSessionForm) {
			if (pSessionForm->GetTotalMembers() == sApplyMember::numbers) {
				int j = 0;
				for (; j < sApplyMember::numbers; j++) {
					if (!pSessionForm->hasMember(pApplyMembers[j].name)) {
						break;
					}
				}
				if (j == sApplyMember::numbers) {
					eTalkSessionStyle const style = pSessionForm->GetStyle();
					if (style == eTalkSessionStyle::Hidden ||
						style == eTalkSessionStyle::Inform ||
						style == eTalkSessionStyle::Minimized) {
						pSessionForm->ChangeStyle(eTalkSessionStyle::Normal);
					}
					if (pSessionForm->m_pForm) {
						pSessionForm->m_pForm->Show();
					}
					return;
				}
			}
		}
	}
	const char** pArrName = new const char*[sApplyMember::numbers];
	for (int i = 0; i < sApplyMember::numbers; i++) {
		pArrName[i] = pApplyMembers[i].name.c_str();
	}
	CS_Sess_Create(pArrName, sApplyMember::numbers);
	SAFE_DELETE_ARRAY(pArrName);
}

void CTalkSessionFormMgr::ApplySession(CMember* pArrMembers[], int memberNum) {
	for (DWORD i = 0; i < m_SessionCounts; i++) {
		CTalkSessionForm* pSessionForm = GetSessionFormByIndex(i);
		if (pSessionForm) {
			if (pSessionForm->GetTotalMembers() == memberNum) {
				int j = 0;
				for (; j < memberNum; j++) {
					if (!pSessionForm->hasMember(pArrMembers[j]->GetName())) {
						break;
						//pSessionForm->SetActiveSession(true,sessionID);
						//pSessionForm->SendBufferMsg();
						//return pSessionForm;
					}
				}
				if (j == memberNum) {
					eTalkSessionStyle const style = pSessionForm->GetStyle();
					if (style == eTalkSessionStyle::Hidden ||
						style == eTalkSessionStyle::Inform ||
						style == eTalkSessionStyle::Minimized) {
						pSessionForm->ChangeStyle(eTalkSessionStyle::Normal);
					}
					pSessionForm->SetIsFlash(false);
					if (pSessionForm->m_pForm) {
						pSessionForm->m_pForm->Show();
					}
					return;
				}
			}
		}
	}

	const char** pArrName = new const char*[memberNum];
	for (int i = 0; i < memberNum; i++) {
		pArrName[i] = pArrMembers[i]->GetName();
	}
	CS_Sess_Create(pArrName, memberNum);
	SAFE_DELETE_ARRAY(pArrName);
}

CTalkSessionForm* CTalkSessionFormMgr::CreateSessionForm(DWORD sessionID, stNetSessCreate* pArrMembers[], int memberNum) {
	CTalkSessionForm* pSessionForm = GetSessionFormByID(sessionID);
	if (pSessionForm)
		return pSessionForm;

	if (memberNum == 1) {
		for (DWORD i = 0; i < m_SessionCounts; i++) {
			if (!(pSessionForm = GetSessionFormByIndex(i))->IsActiveSession()) {
				if (pSessionForm->GetTotalMembers() == 1) {
					if (pSessionForm->hasMember(pArrMembers[0]->szChaName)) {
						pSessionForm->SetActiveSession(true, sessionID);
						pSessionForm->SendBufferMsg();
						return pSessionForm;
					}
				}
			}
		}
	}
	pSessionForm = new CTalkSessionForm(sessionID, eTalkSessionStyle::Normal);
	pSessionForm->AddMembers(pArrMembers, memberNum);
	m_SessionFormsLink.push_back(pSessionForm);
	++m_SessionCounts;

	char buf[20];
	std::string str = pArrMembers[0]->szChaName;
	_itoa_s(memberNum, buf, 10);
	str += "(" + std::string(buf) + ")";
	auto* pItem = new CTextGraph(2);
	pItem->SetHint(str.c_str());
	str = StringLimit(str, 14);
	pItem->SetString(str.c_str());
	pItem->SetColor(0xef5c3cd3);
	pItem->SetPointer(pSessionForm);
	g_stUIChat.GetSessionNode()->AddItem(pItem);
	pSessionForm->SetData(pItem);
	CChatIconInfo* pIconInfo = GetChatIconInfo(pArrMembers[0]->sIconID);
	if (pIconInfo) {
		CGuiPic* pPic = pItem->GetImage();
		std::string strPath = "texture/ui/HEAD/";
		pPic->LoadImage((strPath + pIconInfo->szSmall).c_str(), 16, 16, 0, pIconInfo->nSmallX, pIconInfo->nSmallY);
		pPic->LoadImage("texture/ui/system/slide.tga", 16, 16, 1, pIconInfo->nSmallX, pIconInfo->nSmallY);
		pPic->SetFrame(0);
	}
	g_stUIChat.GetTeamView()->Refresh();

	return pSessionForm;
}

CTalkSessionForm* CTalkSessionFormMgr::CreateHiddenSessionForm(DWORD sessionID, stNetSessCreate* pArrMembers[], int memberNum) {
	CTalkSessionForm* pSessionForm = GetSessionFormByID(sessionID);
	if (pSessionForm)
		return pSessionForm;

	if (memberNum == 1) {
		for (DWORD i = 0; i < m_SessionCounts; i++) {
			if (!(pSessionForm = GetSessionFormByIndex(i))->IsActiveSession()) {
				if (pSessionForm->GetTotalMembers() == 1) {
					if (pSessionForm->hasMember(pArrMembers[0]->szChaName)) {
						pSessionForm->SetActiveSession(true, sessionID);
						pSessionForm->SendBufferMsg();
						return pSessionForm;
					}
				}
			}
		}
	}
	pSessionForm = new CTalkSessionForm(sessionID, eTalkSessionStyle::Hidden);
	pSessionForm->AddMembers(pArrMembers, memberNum);
	m_SessionFormsLink.push_back(pSessionForm);
	++m_SessionCounts;

	char buf[20];
	std::string str = pArrMembers[0]->szChaName;
	_itoa_s(memberNum, buf, 10);
	str += "(" + std::string(buf) + ")";
	auto* pItem = new CTextGraph(2);
	pItem->SetString(str.c_str());
	pItem->SetColor(0xef5c3cd3);
	pItem->SetPointer(pSessionForm);
	g_stUIChat.GetSessionNode()->AddItem(pItem);
	pSessionForm->SetData(pItem);
	CChatIconInfo* pIconInfo = GetChatIconInfo(pArrMembers[0]->sIconID);
	if (pIconInfo) {
		CGuiPic* pPic = pItem->GetImage();
		std::string strPath = "texture/ui/HEAD/";
		pPic->LoadImage((strPath + pIconInfo->szSmall).c_str(), 16, 16, 0, pIconInfo->nSmallX, pIconInfo->nSmallY);
	}
	g_stUIChat.GetTeamView()->Refresh();

	return pSessionForm;
}

void CTalkSessionFormMgr::SessionReceiveMsg(DWORD sessionID, DWORD chaID, std::string talkMsg) {
	CTalkSessionForm* pSessionForm = GetSessionFormByID(sessionID);
	if (!pSessionForm) {
		return;
	}
	//if (!pSessionForm->hasMember(chaID)) return;
	if (pSessionForm->GetStyle() == eTalkSessionStyle::Hidden) {
		std::string strMsg = "";
		CSessionMember* pMember = pSessionForm->GetMemberByID(chaID);
		if (pMember) {
			strMsg = pMember->GetName();
		}
		pSessionForm->SetInformText(strMsg.c_str(), talkMsg.c_str());
		pSessionForm->ChangeStyle(eTalkSessionStyle::Inform);
	}
	pSessionForm->AddMessage(chaID, talkMsg);
}

//void CTalkSessionFormMgr::TableInit()
//{
//	RECT rect1,rect2;
//	SetRect(&rect1,0,0,184,19);
//	SetRect(&rect2,0,0,184,100);
//	INFORM_TABLE_W=3;
//	INFORM_TABLE_H=10;
//	POINT p={580,440};
//	SetTablePosition(p);
//}
//
//void CTalkSessionFormMgr::SetInformSize(RECT rectFrom,RECT rectTitle)
//{
//	//正规化矩形框
//	LONG tmp;
//	if (rectFrom.top>rectFrom.bottom)
//	{
//		tmp=rectFrom.bottom;
//		rectFrom.bottom=rectFrom.top;
//		rectFrom.top=tmp;
//	}
//	if (rectFrom.left>rectFrom.right)
//	{
//		tmp=rectFrom.left;
//		rectFrom.left=rectFrom.right;
//		rectFrom.right=tmp;
//	}
//	if (rectTitle.top>rectTitle.bottom)
//	{
//		tmp=rectTitle.bottom;
//		rectTitle.bottom=rectTitle.top;
//		rectTitle.top=tmp;
//	}
//	if (rectTitle.left>rectTitle.right)
//	{
//		tmp=rectTitle.left;
//		rectTitle.left=rectTitle.right;
//		rectTitle.right=tmp;
//	}
//	INFORM_FORM=rectFrom;
//	INFORM_TITLE=rectTitle;
//}
//
//void CTalkSessionFormMgr::SetTablePosition(POINT tPosition)
//{
//	INFORM_TABLE_POSITION=tPosition;
//}
//
//RECT CTalkSessionFormMgr::TableToTitle(int tx,int ty)
//{
//	RECT ret;
//	int h=INFORM_TITLE.bottom-INFORM_TITLE.top+1;
//	int w=INFORM_TITLE.right-INFORM_TITLE.left+1;
//	if (tx>INFORM_TABLE_W)
//	{
//		tx=tx % INFORM_TABLE_W;
//	}
//	if (ty>INFORM_TABLE_H)
//	{
//		ty=ty % INFORM_TABLE_H;
//	}
//	ret.top=INFORM_TABLE_POSITION.x-(tx*w+w);
//	ret.left=INFORM_TABLE_POSITION.y-(ty*h+h);
//	ret.bottom=ret.top+h-1;
//	ret.right=ret.left+w-1;
//	return ret;
//}
//
//RECT CTalkSessionFormMgr::TableToForm(int tx,int ty)
//{
//	RECT ret=TableToTitle(tx,ty);
//	ret.top-=(INFORM_FORM.bottom-INFORM_FORM.top+1)-(INFORM_TITLE.bottom-INFORM_TITLE.top+1);
//	return ret;
//}

int CTalkSessionFormMgr::GetShowChannel() {
	if (m_Channel[1] <= m_Channel[2]) {
		m_Channel[1]++;
		return 1;
	}
	if (m_Channel[2] <= m_Channel[3]) {
		m_Channel[2]++;
		return 2;
	}
	if (m_Channel[3] <= m_Channel[4]) {
		m_Channel[3]++;
		return 3;
	}
	m_Channel[1]++;
	return 1;
}

void CTalkSessionFormMgr::ReleaseShowChannel(int n) {
	if (n > 0 && n < 4) {
		m_Channel[n]--;
	}
}

POINT CTalkSessionFormMgr::GetMiniFormPosition(DWORD sessionID) {
	POINT pt;
	RECT rect;
	int Flag[30];
	ZeroMemory(Flag, sizeof(int) * 30);
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->GetSessionID() != sessionID) {
			CForm* pForm = (*Iter)->m_pMinimizeForm;
			SetRect(&rect, pForm->GetLeft(), pForm->GetTop(), pForm->GetRight(), pForm->GetBottom());
			pt.x = (rect.right + rect.left) / 2;
			pt.x = (pt.x - INFORM_FORM_SHOW_SX) / MINIMIZE_FORM_WIDTH;
			pt.y = (rect.top + rect.bottom) / 2;
			pt.y = pt.y / MINIMIZE_FORM_HEIGHT;
			if (pt.x <= 2 && pt.x >= 0 && pt.y >= 0 && pt.y <= 9) {
				Flag[pt.y * 3 + pt.x]++;
			}
		}
	}
	int least = -1;
	for (int i = 0; i < 30; i++) {
		if (Flag[i] < least || least == -1) {
			least = Flag[i];
			pt.x = INFORM_FORM_SHOW_SX + MINIMIZE_FORM_WIDTH * (i % 3);
			pt.y = MINIMIZE_FORM_HEIGHT * (i / 3);
		}
	}
	return pt;

	//for (int i=0;i<30;i++)
	//{
	//	if (Flag[i]==0)
	//	{
	//		pt.x=INFORM_FORM_SHOW_SX+MINIMIZE_FORM_WIDTH*(i%3);
	//		pt.y=MINIMIZE_FORM_HEIGHT*(i/3);
	//		return pt;
	//	}
	//}
	//pt.x=INFORM_FORM_SHOW_SX;
	//pt.y=0;
	//return pt;
}

POINT CTalkSessionFormMgr::GetNormalFormPosition(DWORD sessionID) {
	POINT pt;
	int Flag[36];
	POINT point;
	ZeroMemory(Flag, sizeof(int) * 36);
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->GetSessionID() != sessionID) {
			CForm* pForm = (*Iter)->m_pNormalForm;
			//point.SetPoint(pForm->GetLeft(),pForm->GetTop());
			point.x = pForm->GetLeft();
			point.y = pForm->GetTop();
			pt.x = (point.x - NORMAL_FORM_SHOW_SX) / NORMAL_FORM_OFFSET_WIDTH;
			if (pt.x < 0 || pt.x > 5)
				continue;
			pt.y = (point.y - NORMAL_FORM_SHOW_SY - NORMAL_FORM_OFFSET_HEIGHT * pt.x) / (NORMAL_FORM_OFFSET_HEIGHT + NORMAL_FORM_OFFSET_HEIGHT / 2);
			if (pt.y >= 0 && pt.y <= 5) {
				Flag[pt.y * 6 + pt.x]++;
			}
		}
	}
	int least = -1;
	for (int i = 0; i < 36; i++) {
		if (Flag[i] < least || least == -1) {
			least = Flag[i];
			pt.x = NORMAL_FORM_SHOW_SX + NORMAL_FORM_OFFSET_WIDTH * (i % 6);
			pt.y = NORMAL_FORM_SHOW_SY + (NORMAL_FORM_OFFSET_HEIGHT + NORMAL_FORM_OFFSET_HEIGHT / 2) * (i / 6) + NORMAL_FORM_OFFSET_HEIGHT * (i % 6);
		}
	}
	return pt;
}

void CTalkSessionFormMgr::OnClickOpenSession(CItemObj* pItem) {
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		if ((*Iter)->GetData() == (void*)pItem) {
			(*Iter)->StopFlash();
			(*Iter)->ChangeStyle(eTalkSessionStyle::Normal);
		}
	}
}

void CTalkSessionFormMgr::OnClickCloseSession(CTalkSessionForm* pSession) {
	if (!pSession)
		return;
	CS_Sess_Leave(pSession->m_dwSessioinID);
	//if (pSession->m_nShowChannel)
	//{
	//	ReleaseShowChannel(pSession->m_nShowChannel);
	//	pSession->m_nShowChannel=0;
	//}
	pSession->ReleaseInformShowChannel();
	pSession->StopFlash();
	CloseSessionForm(pSession->m_dwSessioinID);
}

void CTalkSessionFormMgr::OnFlashSession() {
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		CTextGraph* pItem = (CTextGraph*)((*Iter)->GetData());
		char buf[20];
		std::string str = (*Iter)->GetMemberByIndex(0)->GetName();
		_itoa_s((*Iter)->GetTotalMembers(), buf, 10);
		str += "(" + std::string(buf) + ")";
		pItem->SetHint(str.c_str());
		str = StringLimit(str, 14);
		pItem->SetString(str.c_str());
		CGuiPic* pPic = pItem->GetImage();
		if ((*Iter)->GetIsFlash()) {
			pPic->Next();
			if ((*Iter)->GetStyle() == eTalkSessionStyle::Minimized) {
				auto* pText = dynamic_cast<CLabelEx*>((*Iter)->m_pMinimizeForm->Find("labName"));
				if (pText) {
					pText->SetTextColor(((*Iter)->m_nFlashTimes & 1) ? 0xffa0a030 : COLOR_BLACK);
					if ((*Iter)->m_nFlashTimes < 13)
						(*Iter)->m_nFlashTimes++;
				}
			}
		} else {
			pPic->SetFrame(0);
			auto* pText = dynamic_cast<CLabelEx*>((*Iter)->m_pMinimizeForm->Find("labName"));
			if (pText) {
				pText->SetTextColor(COLOR_BLACK);
			}
			(*Iter)->m_nFlashTimes = 0;
		}
		g_stUIChat.GetTeamView()->Refresh();
	}
}

void CTalkSessionFormMgr::RefreshSessionMember(DWORD chaID, const char* szMotto) {
	if (szMotto == nullptr)
		return;
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		CSessionMember* pSessMember = (*Iter)->GetMemberByID(chaID);
		if (pSessMember) {
			pSessMember->SetMotto(szMotto);
			(*Iter)->RefreshForm();
		}
	}
}

bool CTalkSessionFormMgr::hasFlashSession() {
	int i = 0;
	CTalkSessionForm* pSess;
	while (pSess = GetSessionFormByIndex(i)) {
		if (pSess->GetIsFlash())
			return true;
		i++;
	}
	return false;
}

bool CTalkSessionFormMgr::OnHotKeyShow(char& key, int& control) {
	char qqKey = g_stUIChat.GetQQFrom()->GetHotKey();
	if (key != qqKey) {
		return false;
	}
	int i = 0;
	CTalkSessionForm* pSessionForm;
	for (pSessionForm = GetSessionFormByIndex(i); pSessionForm; pSessionForm = GetSessionFormByIndex(++i)) {
		eTalkSessionStyle eTalkStyle = pSessionForm->GetStyle();
		if (eTalkStyle == eTalkSessionStyle::Inform) {
			pSessionForm->OnHotKeyShow();
			return true;
		} else if (eTalkStyle == eTalkSessionStyle::Minimized) {
			pSessionForm->OnHotKeyShow();
			return true;
		}
	}
	return false;
}

DWORD CTalkSessionFormMgr::GetSessionCount() {
	return m_SessionCounts;
}

void CTalkSessionFormMgr::PrintTraceInfo() {
	std::string strInfo = "";
	char szBuf[2048] = {0};
	std::vector<CTalkSessionForm*>::iterator Iter;
	for (Iter = m_SessionFormsLink.begin(); Iter != m_SessionFormsLink.end(); Iter++) {
		CTalkSessionForm* pSessionForm = (*Iter);
		DWORD dwSessionID = pSessionForm->GetSessionID();
		int nMemberCount = pSessionForm->GetTotalMembers();
		_snprintf_s(szBuf, _TRUNCATE, "会话ID=%d 人数=%d 成员:", nMemberCount, dwSessionID);
		strInfo += szBuf;
		for (int i = 0; i < nMemberCount; i++) {
			CSessionMember* pMember = pSessionForm->GetMemberByIndex(i);
			strInfo += std::string(pMember->GetName()) + "|";
		}
		strInfo += "|\n";
	}
	MessageBox(g_pGameApp->GetHWND(), "error", strInfo.c_str(), MB_OK);
}
