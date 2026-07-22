#include "StdAfx.h"
#include "uiformmgr.h"
#include "UIImeInput.h"
#include "uimenu.h"
#include "Mouse.h"

using namespace GUI;

const int START_HINT = 5;

CFormMgr CFormMgr::s_Mgr;

eMouseAction CFormMgr::_eMouseAction = eMouseAction::Gui;

bool CFormMgr::_IsDebugMode = false;				// 是否在调试模式 -added by Arcol
bool CFormMgr::_IsDrawFrameInDebugMode = true;		// 仅用于调试模式 -added by Arcol
bool CFormMgr::_IsDrawBackgroundInDebugMode = true; // 仅用于调试模式 -added by Arcol

CFormMgr::CFormMgr() { // 西门文档修改
	_forms = &_defaulttemplete;
}

CFormMgr::~CFormMgr() {
	Clear();
	for (auto& it : _showforms) {
		SAFE_DELETE(it);
	}
}

bool CFormMgr::SwitchTemplete(int n) {
	if (n >= (int)GetFormTempleteMax() || n < -1)
		return false;

	if (_nTempleteNo == n)
		return true;

	_pHintGui = nullptr;

	_nTempleteNo = n;
	if (n == -1) {
		_forms = &_defaulttemplete;
		_InitFormID();
		return true;
	}

	if (_forms != _showforms[n]) {
		_forms = _showforms[n];
		_InitFormID();
		return true;
	}

	return false;
}

bool CFormMgr::SetFormTempleteMax(int n) {
	if (n >= 0) {
		for (auto& it : _showforms) {
			SAFE_DELETE(it); // UI当机处理
		}
		_showforms.resize(n);
		for (int i = 0; i < n; i++) {
			_showforms[i] = new vfrm;
		}
		_forms = &_defaulttemplete;

		if (CDrag::GetDrag()) {
			CDrag::GetDrag()->Reset();
		}
		return true;
	}
	return false;
}

void CFormMgr::_InitFormID() {
	CCompent::SetActive(nullptr);

	for (auto& it : _allForms) {
		it->_TempleteClear();
	}

	int id = 0;
	_show.clear();
	for (auto& it : *_forms) {
		it->_TempleteInit();

		if (it->GetIsShow()) {
			_show.push_back(it);
		}
	}
	_modal.clear();

	_SetNewActiveForm();
}

void CFormMgr::ResetAllForm() {
	for (auto& it : _allForms) {
		it->_bShow = false;
	}

	_show.clear();

	CCompent::SetActive(nullptr);
}

bool CFormMgr::_DelMemory(CForm* form) {
	_DelForm(_allForms, form);
	_DelForm(_modal, form);
	_DelForm(_show, form);
	_DelForm(_defaulttemplete, form);
	for (auto& it : _showforms) {
		_DelForm(*it, form);
	}
	return true;
}

void CFormMgr::_DelForm(vfrm& list, CForm* frm) {
	list.remove(frm);
}

bool CFormMgr::_AddMemory(CForm* form) {
	// "Check if there is a duplicate interface name"
	for (auto& it : _allForms) {
		if (_stricmp(form->GetName(), it->GetName()) == 0) {
			LG("gui", RES_STRING(CL_LANGUAGE_MATCH_574), form->GetName());
			return false;
		}
	}

	// "Join the list"
	if (_allForms.cend() == std::find(_allForms.cbegin(), _allForms.cend(), form)) {
		_allForms.push_back(form);
		if (!form->GetIsModal()) {
			_defaulttemplete.push_back(form);
		}
		return true;
	}
	return false;
}

bool CFormMgr::AddForm(CForm* form, int templete) {
	if (templete >= (int)GetFormTempleteMax() || templete < 0) {
		return false;
	}

	if (form->GetIsModal()) {
		return false;
	}

	vfrm* f = _showforms[templete];
	if (f->cend() == find(f->cbegin(), f->cend(), form)) {
		f->push_back(form);
		return true;
	}
	return false;
}

void CFormMgr::FrameMove(int x, int y, MouseClickState dwMouseKey, DWORD dwTime) {
	if (!_bEnabled) {
		_eMouseAction = eMouseAction::None;
		return;
	}

	CGuiTime::FrameMove(dwTime);

	static DWORD dwNextTime = 0;
	if (dwTime >= dwNextTime) {
		dwNextTime = dwTime + 100;
		for (auto& it : _modal) {
			it->FrameMove(dwTime);
		}
		for (auto rit = _show.rbegin(); rit != _show.rend(); ++rit) {
			(*rit)->FrameMove(dwTime);
		}
	}

	if (dwMouseKey != MouseClickState()) {
		_MouseRun(x, y, dwMouseKey);
		_nMouseHover = 0;
	} else if (_eMouseAction != eMouseAction::None && _modal.empty()) {
		_nMouseHover++;
		if (_nMouseHover == START_HINT) {
			_pHintGui = nullptr;

			CGuiData::SetHintItem(nullptr);
			CForm* frm = GetHitForm(x, y);
			if (frm) {
				_pHintGui = frm->GetHintGui(x, y);
			}
			auto* p = dynamic_cast<CCompent*>(_pHintGui);
			if (p) {
				if (CGuiData::GetHintItem()) {
					CGuiData::GetHintItem()->ReadyForHint(x, y, p);
				}
			}
		}
	} else {
		_nMouseHover = 0;
	}

	if (CCursor::stEnd != CGuiData::GetCursor()) {
		CCursor::I()->SetFrame(CGuiData::GetCursor());
	} else {
		CCursor::I()->Restore();
	}
}

void CFormMgr::RenderHint(int x, int y) {
	if (_nMouseHover < START_HINT)
		return;

	if (_pHintGui) {
		GetRender().ScreenConvert(x, y);

		if (CGuiData::GetHintItem()) {
			CGuiData::GetHintItem()->RenderHint(x, y);
			return;
		}

		_pHintGui->RenderHint(x, y);
	}
}

bool CFormMgr::_MouseRun(int x, int y, MouseClickState mouse) {
	_eMouseAction = eMouseAction::None;
	GetRender().ScreenConvert(x, y);

	CGuiData::SetCursor(CCursor::stEnd);

	CGuiData::SetMousePos(x, y);

	if (!_OnMouseRun.empty()) {
		for (auto& it : _OnMouseRun)
			it(x, y, mouse);
	}

	CCompent::_pLastMouseCompent = nullptr;
	if (CDrag::GetDrag()) {
		CDrag::GetDrag()->MouseRun(x, y, mouse);
		_eMouseAction = eMouseAction::Gui;
		return true;
	}

	if (!_modal.empty()) {
		static CForm* p = nullptr;

		// 有模态函数执行
		p = _modal.back();
		p->MouseRun(x, y, mouse);
		if (CForm::mrNone != p->GetModalResult()) {
			p->Hide();
		}
		_eMouseAction = eMouseAction::Gui;
		return true;
	}

	for (auto rit = _show.rbegin(); rit != _show.rend(); ++rit) {
		if ((*rit)->MenuMouseRun(x, y, mouse)) {
			_eMouseAction = eMouseAction::Gui;
			return true;
		}
	}

	if ((mouse & MouseClickState::Down) != MouseClickState()) {
		CMenu::CloseAll();
	}

	for (auto rit = _show.rbegin(); rit != _show.rend(); ++rit) {
		if ((*rit)->MouseRun(x, y, mouse)) {
			_eMouseAction = eMouseAction::Gui;
			if (CCompent::GetLastMouseCompent()) {
				_eMouseAction = CCompent::GetLastMouseCompent()->GetMouseAction();
			}
			return _eMouseAction == eMouseAction::Gui;
		}
	}

	return false;
}

void CFormMgr::_ModalClose(CForm* form) {
	auto it = find(_modal.cbegin(), _modal.cend(), form);
	if (it != _modal.end()) {
		_modal.erase(it);
		_SetNewActiveForm();
		return;
	}

	LG("gui", "msgCFormMgr::_ModalClose(%s) not find this form", form->GetName());
}

void CFormMgr::_SetNewActiveForm() {
	_ActiveCompent();

	if (CCompent::GetActive()) {
		if (CCompent::GetActive()->GetForm() != CForm::GetActive())
			CCompent::GetActive()->GetForm()->_OnActive();
	} else {
		if (!_modal.empty()) {
			_modal.back()->_OnActive();
			return;
		}

		for (auto& it : _show) {
			if (it->IsNormal()) {
				it->_OnActive();
				break;
			}
		}
	}
}

CForm* CFormMgr::GetHitForm(int x, int y) {
	for (auto it = _show.rbegin(); it != _show.rend(); ++it) {
		vfrm& childs = (*it)->_childs;
		if (!childs.empty()) {
			for (auto ch_modal = childs.begin(); ch_modal != childs.end(); ch_modal++) {
				if ((*ch_modal)->GetIsShow() && (*ch_modal)->InRect(x, y)) {
					return (*ch_modal);
				}
			}
		}

		if ((*it)->GetIsShow() && (*it)->InRect(x, y)) {
			return (*it);
		}
	}

	return nullptr;
}

bool CFormMgr::MouseScroll(int nScroll) {
	if (!_bEnabled)
		return false;

	_nMouseHover = 0;

	if (!_OnMouseScroll.empty()) {
		for (auto& it : _OnMouseScroll)
			if (it(nScroll)) {
				return true;
			}
	}

	if (CCompent::_pLastMouseCompent) {
		return CCompent::_pLastMouseCompent->MouseScroll(nScroll);
	}
	return false;
}

bool CFormMgr::OnKeyDown(int key) {
	if (!_bEnabled)
		return false;

	if (!_OnKeyDown.empty()) {
		for (auto const& it : _OnKeyDown)
			if (it(key)) {
				return true;
			}
	}

	static CCompent* p = nullptr;
	static CForm* f = nullptr;

	f = CForm::GetActive();
	if (f && f->OnKeyDown(key))
		return true;

	p = CCompent::GetActive();
	if (p && p->OnKeyDown(key))
		return true;

	return true;
}

bool CFormMgr::OnKeyChar(char key) {
	if (!_bEnabled)
		return false;

	if (!_OnKeyChar.empty()) {
		for (auto& it : _OnKeyChar)
			if (it(key)) {
				return true;
			}
	}

	static CCompent* p = nullptr;
	static CForm* f = nullptr;

	f = CForm::GetActive();
	if (f && f->OnChar(key))
		return true;

	p = CCompent::GetActive();
	if (p && p->OnChar(key))
		return true;

	return false;
}

void CFormMgr::Render() {
	if (!_bEnabled) {
		ShowDebugInfo(); //显示调试信息	-added by Arcol
		return;
	}

	vfrm::iterator it;
	for (auto& it : _show) {
		it->Render();
	}

	for (auto& it : _modal) {
		it->Render();
	}

	if (CDrag::GetParent())
		CDrag::GetParent()->DragRender();

	CImeInput::s_Ime.Render();

	ShowDebugInfo(); //显示调试信息	-added by Arcol
}

bool CFormMgr::OnHotKey(char key, int control) {
	if (!_bEnabled)
		return false;

	if (!_modal.empty()) {
		CForm* frm = _modal.back();
		if (frm->evtHotkeyHandler != nullptr) {
			if (frm->evtHotkeyHandler(frm, key))
				return true;
		}
	} else if (!_show.empty()) {
		CForm* frm = _show.back();
		if (frm->evtHotkeyHandler != nullptr) {
			if (frm->evtHotkeyHandler(frm, key))
				return true;
		}
	}

	if (!_vhotkey.empty()) {
		for (auto& it : _vhotkey)
			if (it(key, control)) {
				return true;
			}
	}

	if (!_modal.empty())
		return true;

	for (auto& it : *_forms) {
		if (it->GetHotKey() == key) {
			if (it->GetIsShow()) {
				it->Hide();
			} else {
				it->Show();
			}
			return true;
		}
	}

	return false;
}

bool CFormMgr::HandleWindowMsg(DWORD dwMsg, DWORD dwParam1, DWORD dwParam2) {
	if (!_bEnabled)
		return false;

	switch (dwMsg) {
	case WM_HOTKEY: {
		if (LOWORD(dwParam2) == MOD_ALT) {
			for (auto& it : *_forms) {
				if (it->GetHotKey() > 0 && it->GetHotKey() == (UINT)HIWORD(dwParam2)) {
					if ((it->GetIsShow())) {
						it->Hide();
					} else {
						it->Show();
					}
				}
			}
		}
		return true;
	}
	};

	CImeInput::s_Ime.HandleWindowMsg(dwMsg, dwParam1, dwParam2);
	return false;
}

void CFormMgr::Refresh() {
	for (auto& it : _allForms) {
		it->Refresh();
	}
}

CForm* CFormMgr::Find(const char* str, int no) {
	if (no >= (int)GetFormTempleteMax() || no < -1)
		return nullptr;

	vfrm* form = no == -1 ? &_defaulttemplete : _showforms[no];
	for (auto& it : *form) {
		if (strcmp(it->GetName(), str) == 0) {
			return it;
		}
	}

	return nullptr;
}

void CFormMgr::_ShowModal(CForm* form) {
	if (_modal.cend() == find(_modal.cbegin(), _modal.cend(), form)) {
		_modal.push_back(form);
	}
	_ActiveCompent();
}

CForm* CFormMgr::FindAll(const char* str) {
	for (auto& it : _allForms)
		if (strcmp(it->GetName(), str) == 0) {
			return it;
		}

	return nullptr;
}

void CFormMgr::ForEach(FormFun pFun) {
	for (auto& it : _allForms) {
		pFun(it);
	}
}

bool CFormMgr::AddKeyCharEvent(KeyCharEvent event) {
	auto it = find(_OnKeyChar.cbegin(), _OnKeyChar.cend(), event);
	if (it == _OnKeyChar.cend()) {
		_OnKeyChar.push_back(event);
		return true;
	}
	return false;
}

bool CFormMgr::DelKeyCharEvent(KeyCharEvent event) {
	auto it = find(_OnKeyChar.cbegin(), _OnKeyChar.cend(), event);
	if (it != _OnKeyChar.cend()) {
		_OnKeyChar.erase(it);
		return true;
	}
	return false;
}

bool CFormMgr::AddKeyDownEvent(KeyDownEvent event) {
	auto it = find(_OnKeyDown.cbegin(), _OnKeyDown.cend(), event);
	if (it == _OnKeyDown.cend()) {
		_OnKeyDown.push_back(event);
		return true;
	}
	return false;
}

bool CFormMgr::DelKeyDownEvent(KeyDownEvent event) {
	auto it = std::find(_OnKeyDown.begin(), _OnKeyDown.end(), event);
	if (_OnKeyDown.end() != it) {
		_OnKeyDown.erase(it);
		return true;
	}
	return false;
}

bool CFormMgr::AddMouseEvent(MouseEvent event) {
	auto it = find(_OnMouseRun.cbegin(), _OnMouseRun.cend(), event);
	if (it == _OnMouseRun.cend()) {
		_OnMouseRun.push_back(event);
		return true;
	}
	return false;
}

bool CFormMgr::DelMouseEvent(MouseEvent event) {
	auto it = find(_OnMouseRun.cbegin(), _OnMouseRun.cend(), event);
	if (it != _OnMouseRun.cend()) {
		_OnMouseRun.erase(it);
		return true;
	}
	return false;
}

bool CFormMgr::AddHotKeyEvent(HotKeyEvent event) {
	auto it = find(_vhotkey.cbegin(), _vhotkey.cend(), event);
	if (it == _vhotkey.cend()) {
		_vhotkey.push_back(event);
		return true;
	}
	return false;
}

bool CFormMgr::DelHotKeyEvent(HotKeyEvent event) {
	auto it = find(_vhotkey.begin(), _vhotkey.end(), event);
	if (it != _vhotkey.end()) {
		_vhotkey.erase(it);
		return true;
	}
	return false;
}

bool CFormMgr::AddMouseScrollEvent(MouseScrollEvent event) {
	auto it = find(_OnMouseScroll.cbegin(), _OnMouseScroll.cend(), event);
	if (it == _OnMouseScroll.end()) {
		_OnMouseScroll.push_back(event);
		return true;
	}
	return false;
}

bool CFormMgr::DelMouseScrollEvent(MouseScrollEvent event) {
	auto it = find(_OnMouseScroll.cbegin(), _OnMouseScroll.cend(), event);
	if (it != _OnMouseScroll.end()) {
		_OnMouseScroll.erase(it);
		return true;
	}
	return false;
}

bool CFormMgr::AddFormInit(FormMgrEvent pInitFun) {
	auto it = find(_vinits.cbegin(), _vinits.cend(), pInitFun);
	if (it == _vinits.cend()) {
		_vinits.push_back(pInitFun);
		return true;
	}
	return false;
}

void CFormMgr::SetScreen() {
	for (auto& allForm : _allForms)
		allForm->OnSetScreen();

	Refresh();
}

void CFormMgr::_UpdataShowForm(CForm* frm) {
	if (_show.empty())
		return;

	if (frm == _show.back())
		return;

	auto it = find(_show.cbegin(), _show.cend(), frm);
	if (it != _show.cend()) {
		_show.erase(it);
		_show.push_back(frm);
	}
	_ActiveCompent();
}

void CFormMgr::_ActiveCompent() {
	CCompent* pCompent = nullptr;
	if (!_modal.empty()) {
		pCompent = _modal.back()->FindActiveCompent();
		CCompent::SetActive(pCompent);
		return;
	} else {
		for (auto rit = _show.rbegin(); rit != _show.rend(); ++rit) {
			if (pCompent == (*rit)->FindActiveCompent()) {
				continue;
			}
		}
	}

	if (pCompent) {
		CCompent::SetActive(pCompent);
	} else {
		if (CCompent::GetActive()) {
			if (!CCompent::GetActive()->GetForm()->IsNormal()) {
				CCompent::SetActive(nullptr);
			}
		}
	}
}

void CFormMgr::_DelShowForm(CForm* frm) {
	auto it = find(_show.cbegin(), _show.cend(), frm);
	if (it != _show.cend()) {
		_show.erase(it);
	}
}

void CFormMgr::_AddShowForm(CForm* frm) {
	auto it = find(_show.cbegin(), _show.cend(), frm);
	if (it == _show.cend()) {
		_show.push_back(frm);
	}
}

CForm* CFormMgr::FindESCForm() {
	CForm* frm = CForm::GetActive();
	if (frm->GetIsEscClose())
		return frm;

	for (auto& it : _modal) {
		if (it->GetIsEscClose()) {
			return it;
		}
	}

	for (auto rit = _show.rbegin(); rit != _show.rend(); ++rit)
		if ((*rit)->GetIsEscClose()) {
			return (*rit);
		}

	return nullptr;
}

struct sUIDebugInfo {
	CForm* form;
	std::string formName;
	std::string isShow;
	std::string isEnable;
	std::string hasPopMenu;
	std::string coordinate;
	int totalCompents;
	int layer;
	std::string renderTime;
	std::string hotkey;
};

void CFormMgr::ShowDebugInfo() {
	if (!_IsDebugMode) {
		return;
	}

	int i = 0;
	char temp[25];
	char buf[10240];

	int data0 = 0, data1 = 0, data2 = 0, data3 = 0, data4 = 0, data5 = 0, data6 = 0, data7 = 0, totalTick = 0;
	auto* pInfo = new sUIDebugInfo[_forms->size()];
	i = 0;
	CForm* pForm = nullptr;
	vfrm::iterator it;
	for (it = _forms->begin(); it != _forms->end(); it++, i++) {
		pForm = *it;
		pInfo[i].form = (*it);
		pInfo[i].formName = pForm->GetName();
		pInfo[i].isShow = pForm->GetIsShow() ? "Y" : "N";
		pInfo[i].isEnable = pForm->GetIsEnabled() ? "Y" : "N";
		_snprintf_s(buf, _TRUNCATE, "(%4d,%4d)-(%4d,%4d)", pForm->GetLeft(), pForm->GetTop(), pForm->GetRight(), pForm->GetBottom());
		pInfo[i].coordinate = buf;
		pInfo[i].totalCompents = (int)pForm->_allCompents.size();
		_snprintf_s(temp, _TRUNCATE, "%c", pForm->GetHotKey());
		pInfo[i].hotkey = (pForm->GetHotKey() == '\0') ? "" : temp;
		pInfo[i].hasPopMenu = (pForm->_pPopMenu) ? "Y" : "N";
		pInfo[i].layer = -1;
		pInfo[i].renderTime = "N/A";
	}

	DWORD tick;
	int j = (int)_show.size() + (int)_modal.size() - 1;
	static DWORD frameColor = 0x90FFFFFF;
	static int changeColor = 0;
	if (changeColor % 7 == 1)
		frameColor = (frameColor == 0x90FFFFFF) ? 0x90000000 : 0x90FFFFFF;
	changeColor = (changeColor + 1) % 7;
	for (it = _show.begin(); it != _show.end(); it++) {
		for (i = 0; i < (int)_forms->size(); i++) {
			if (pInfo[i].form == (*it)) {
				tick = GetTickCount();
				(*it)->Render();
				tick = GetTickCount() - tick;
				totalTick += tick;
				_snprintf_s(buf, _TRUNCATE, "%d", tick);
				pInfo[i].renderTime = buf;
				pInfo[i].layer = j;
				j--;
			}
		}
		if (_IsDrawFrameInDebugMode) {
			GetRender().LineFrame((*it)->GetLeft(), (*it)->GetTop(), (*it)->GetRight(), (*it)->GetBottom(), frameColor);
			CGuiFont::s_Font.Render((*it)->GetName(), (*it)->GetLeft(), (*it)->GetTop(), ~frameColor);
		}
	}

	for (it = _modal.begin(); it != _modal.end(); it++) {
		for (i = 0; i < (int)_forms->size(); i++) {
			if (pInfo[i].form == (*it)) {
				tick = GetTickCount();
				(*it)->Render();
				tick = GetTickCount() - tick;
				totalTick += tick;
				_snprintf_s(buf, _TRUNCATE, "%d", tick);
				pInfo[i].renderTime = buf;
				pInfo[i].layer = j;
				j--;
			}
		}
	}

	GetRender().FillFrame(0, 16, 600, 27);
	//CGuiFont::s_Font.Render("窗口ID       窗口名称      显示  禁止          坐标            父对象        控件数    层",0,16,COLOR_WHITE);
	CGuiFont::s_Font.Render(RES_STRING(CL_LANGUAGE_MATCH_575), 0, 16, COLOR_WHITE);
	for (i = 0; i < (int)_forms->size(); i++) {
		//pForm = *it;
		//pInfo[i].formName=pForm->GetName();
		//pInfo[i].isShow=pForm->GetIsShow()?"Y":"N";
		//pInfo[i].isEnable=pForm->GetIsEnabled()?"N":"Y";
		//_snprintf_s(buf, _TRUNCATE,"(%4d,%4d)-(%4d,%4d)",pForm->GetLeft(),pForm->GetTop(),pForm->GetRight(),pForm->GetBottom());
		//pInfo[i].coordinate=buf;
		//pInfo[i].parent=pForm->GetParent()?pForm->GetParent()->GetName():"没有父对象";
		//pInfo[i].totalCompents=pForm->_allCompents.size();
		//pInfo[i].hotkey=pForm->GetHotKey();
		//pInfo[i].hasPopMenu=(pForm->_pPopMenu)"Y":"N";

		//DWORD color=COLOR_WHITE;
		//_snprintf_s(buf, _TRUNCATE,"%-6d %-20s %s     %s  (%4d,%4d)-(%4d,%4d) %-15s   %3d",i,pForm->GetName(),pForm->GetIsShow()?"Y":"N",pForm->GetIsEnabled()?"N":"Y",pForm->GetLeft(),pForm->GetTop(),pForm->GetRight(),pForm->GetBottom(),
		//	pForm->GetParent()?pForm->GetParent()->GetName():"没有父对象",pForm->_allCompents.size());

		DWORD color = COLOR_WHITE;
		_itoa_s(pInfo[i].layer, temp, 10);
		//"窗口ID       窗口名称          坐标            显示  允许  菜单   层   控件数    热键     渲染时间"
		_snprintf_s(buf, _TRUNCATE, "%-6d %-20s %s   %s     %s     %s     %3s    %3d    %2s     %4s",
					i, pInfo[i].formName.c_str(), pInfo[i].coordinate.c_str(), pInfo[i].isShow.c_str(), pInfo[i].isEnable.c_str(), pInfo[i].hasPopMenu.c_str(), (pInfo[i].layer >= 0) ? temp : "N/A",
					pInfo[i].totalCompents, pInfo[i].hotkey.c_str(), pInfo[i].renderTime.c_str());

		if (pInfo[i].form->GetIsShow()) {
			color = 0xFFFFFF7F;
		}
		if (pInfo[i].layer == 0) {
			color = 0xFFA0A0FF;
		}
		if (_IsDrawBackgroundInDebugMode) {
			GetRender().FillFrame(0, 30 + i * 11, 600, 41 + i * 11);
		}
		CGuiFont::s_Font.Render(buf, 0, 30 + i * 11, color);
	}
	//delete[] pInfo;
	SAFE_DELETE_ARRAY(pInfo); // UI当机处理

	data0 = GetFormTempleteMax(); //模板总数
	for (i = 0; i < (int)GetFormTempleteMax(); i++) {
		if (!_showforms[i]->empty()) {
			data1++; //有效模板数
		}
	}
	data2 = GetFormTempletetNum();					//当前模板编号
	data3 = (int)_allForms.size();					//窗口总数
	data4 = (int)_forms->size();					//当前模板窗口总数
	data5 = (int)_show.size() + (int)_modal.size(); //当前模板显示窗口
	data6 = (int)_modal.size();						//当前模板模态窗口数
	data7 = (int)CGuiTime::_times.size();
	_snprintf_s(buf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_576), data0, data1, data2, data3, data4, data5, data6, data7, totalTick);
	GetRender().FillFrame(0, 0, CGuiFont::s_Font.GetWidth(buf), 11);
	CGuiFont::s_Font.Render(buf, 0, 0, COLOR_WHITE);
	int help_sx = 620;
	int help_sy = 160;
	if (_IsDrawBackgroundInDebugMode) {
		GetRender().FillFrame(help_sx, help_sy, 785, 511);
	}
	CGuiFont::s_Font.Render("      控制台UI命令帮助\n1.显示窗口:\n  show 窗口名称\n2.隐藏窗口:\n  hide 窗口名称\n3.显示/隐藏窗口边框\n  frame 1 //显示\n  frame 0 //隐藏\n3.调试模式底色开关\n  background 1 //显示\n  background 0 //关闭", help_sx, help_sy, COLOR_WHITE);
}

void CFormMgr::SetEnableHotKey(int flag, bool v) {
	// 西门文档修改
	if (v) {
		_nEnableHotKey |= flag;
	} else {
		_nEnableHotKey &= ~flag;
	}
}
