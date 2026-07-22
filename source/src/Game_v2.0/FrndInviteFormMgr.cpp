//------------------------------------------------------------------------
//	2005.3.16	Arcol	create this file
//	2005.4.5	Arcol	add the event handle function
//------------------------------------------------------------------------

#include "StdAfx.h"
#include "uiformmgr.h"
#include "uilabel.h"
#include "uicloneform.h"
#include "NetChat.h"
#include "frndinviteformmgr.h"

static CCloneForm dupe;
CFrndInviteFormMgr g_TeamInviteFormMgr;
std::vector<CFrndInviteFormMgr::PS_FormNode> CFrndInviteFormMgr::m_FormLink;

CFrndInviteFormMgr::CFrndInviteFormMgr() {
}

CFrndInviteFormMgr::~CFrndInviteFormMgr() {
}

bool CFrndInviteFormMgr::AddInviteForm(DWORD id, std::string inviterName) {
	if (GetInviteForm(id))
		return false;
	auto node = new S_FormNode;
	static CForm* frmAcceptFrnd = CFormMgr::s_Mgr.Find("frmAcceptFrnd");
	if (!frmAcceptFrnd) {
		//delete node;
		SAFE_DELETE(node); // UI当机处理
		return false;
	}
	node->id = id;
	node->inviterName = inviterName;
	dupe.SetSample(frmAcceptFrnd);
	node->pForm = dupe.Clone();
	//node->pForm=dynamic_cast<CForm*>(frmAcceptFrnd->Clone());
	char str[256];
	_snprintf_s(str, _TRUNCATE, "frmAcceptFrnd_Arcol_%d", id);
	node->pForm->SetName(str);
	m_FormLink.push_back(node);
	//node->pForm->SetPos(200,200);
	auto* labFrndName = dynamic_cast<CLabelEx*>(node->pForm->Find("labFrndName"));
	if (labFrndName) {
		_snprintf_s(str, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_62), inviterName.data());
		//labFrndName->SetIsCenter(true);
		labFrndName->SetCaption(str);
	}
	auto* labFrnd = dynamic_cast<CLabelEx*>(node->pForm->Find("labFrnd"));
	if (labFrnd) {
		//labFrnd->SetIsCenter(true);
		labFrnd->SetCaption(RES_STRING(CL_LANGUAGE_MATCH_63));
	}
	node->pForm->evtEntrustMouseEvent = _MainMousePlayerFrndEvent;
	node->pForm->nTag = id;
	static int PosY = 100;
	PosY += 10;
	if (PosY > 150)
		PosY = 100;
	node->pForm->SetPos(10, PosY);
	node->pForm->Refresh();
	node->pForm->Show();
	return true;
}

bool CFrndInviteFormMgr::RemoveInviteForm(DWORD id) {
	PS_FormNode node = nullptr;
	std::vector<PS_FormNode>::iterator Iter;
	for (Iter = m_FormLink.begin(); Iter != m_FormLink.end(); Iter++)
		if ((*Iter)->id == id) {
			node = *Iter;
			break;
		}
	if (!node)
		return false;
	node->pForm->Close();
	node->pForm->nTag = 0;
	dupe.Release(node->pForm);
	m_FormLink.erase(Iter);
	//delete node;
	SAFE_DELETE(node); // UI当机处理
	return true;
}

CFrndInviteFormMgr::PS_FormNode CFrndInviteFormMgr::GetInviteForm(DWORD id) {
	std::vector<PS_FormNode>::iterator Iter;
	for (Iter = m_FormLink.begin(); Iter != m_FormLink.end(); Iter++)
		if ((*Iter)->id == id)
			return *Iter;
	return nullptr;
}

void CFrndInviteFormMgr::_MainMousePlayerFrndEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	std::string name = pSender->GetName();
	if (name == "btnNo" || name == "btnClose") {
		CS_Frnd_Refuse((DWORD)pSender->GetForm()->nTag);
	} else if (name == "btnYes") {
		CS_Frnd_Confirm((DWORD)pSender->GetForm()->nTag);
	}
	g_stFrndInviteFormMgr.RemoveInviteForm(pSender->GetForm()->nTag);
}
