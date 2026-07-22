//------------------------------------------------------------------------
//	2005/3/24	Arcol	加入定时器
//------------------------------------------------------------------------
#include "StdAfx.h"
#include "uicompent.h"

using namespace GUI;

CCompent* CCompent::_pActive = nullptr;
CCompent* CCompent::_pLastMouseCompent = nullptr;
//---------------------------------------------------------------------------
// class CCompent
//---------------------------------------------------------------------------
CCompent::CCompent(CForm& frmOwn)
	: CGuiData(), _frmOwn(&frmOwn) {
	_pParent = _frmOwn;
}

CCompent::CCompent(const CCompent& rhs)
	: CGuiData(rhs) {
	_Copy(rhs);
}

CCompent& CCompent::operator=(const CCompent& rhs) {
	CGuiData::operator=(rhs);
	_Copy(rhs);
	return *this;
}

void CCompent::_Copy(const CCompent& rhs) {
	_frmOwn = rhs._frmOwn;

	evtLost = rhs.evtLost;
	evtActive = rhs.evtActive;

	_align = rhs._align;
	_isChild = rhs._isChild;
	_IsFocus = rhs._IsFocus;

	_eMouseAction = rhs._eMouseAction;
}

void CCompent::_AddForm() {
	if (!_frmOwn)
		return;
	_frmOwn->_AddFrameMove(this, false);

	if (_isChild)
		return;
	_frmOwn->_AddCompent(this, false);
}

void CCompent::AddForm() {
	if (!_frmOwn)
		return;
	_frmOwn->_AddFrameMove(this, true);

	if (_isChild)
		return;
	_frmOwn->_AddCompent(this, true);
}

CCompent::~CCompent() {
}

void CCompent::SetAlign(eCompentAlign v) {
	if (!GetParent())
		return;

	_align = v;
}

void CCompent::SetIsShow(bool v) {
	_bShow = v;
	if (v && _IsFocus) {
		SetActive(this);
	}
}

void CCompent::Refresh() {
	switch (_align) {
	case caLeft: {
		SetPos(0, 0);
		SetSize(GetWidth(), GetParent()->GetHeight());
	} break;
	case caLeftUp: {
		SetPos(0, 0);
	} break;
	case caUp: {
		SetPos(0, 0);
		SetSize(GetParent()->GetWidth(), GetHeight());
	} break;
	case caRightUp: {
		SetPos(GetParent()->GetWidth() - GetWidth(), 0);
	} break;
	case caRight: {
		SetPos(GetParent()->GetWidth() - GetWidth(), 0);
		SetSize(GetWidth(), GetParent()->GetHeight());
	} break;
	case caRightBottom: {
		SetPos(GetParent()->GetWidth() - GetWidth(), GetParent()->GetHeight() - GetHeight());
	} break;
	case caBottom: {
		SetPos(0, GetParent()->GetHeight() - GetHeight());
		SetSize(GetParent()->GetWidth(), GetHeight());
	} break;
	case caLeftBottom: {
		SetPos(0, GetParent()->GetHeight() - GetHeight());
	} break;
	case caClient: {
		SetPos(0, 0);
		SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());
	} break;
	case caWidthCenter:
		SetPos((GetParent()->GetWidth() - GetWidth()) / 2, GetTop());
		break;
	case caHeightCenter:
		SetPos(GetLeft(), (GetParent()->GetHeight() - GetHeight()) / 2);
		break;
	case caCenter:
		SetPos((GetParent()->GetWidth() - GetWidth()) / 2, (GetParent()->GetHeight() - GetHeight()) / 2);
		break;
	}
	CGuiData::Refresh();
}

bool CCompent::MouseRun(int x, int y, MouseClickState key) {
	if (!IsNormal())
		return false;

	if (IsNoDrag(x, y, key)) {
		if (!_isChild && (GetActive() != this) && ((key & MouseClickState::LDown) != MouseClickState())) {
			_SetActive();
		}
		return true;
	}
	return _IsMouseIn;
}

void CCompent::OnActive() {
	GetForm()->SetActiveCompent(this);
	if (evtActive)
		evtActive(this);
}

void CCompent::SetActive(CCompent* v) {
	// 判断激活状态
	if (_pActive == v)
		return;

	if (_pActive)
		_pActive->OnLost();

	_pActive = v;

	if (_pActive)
		_pActive->OnActive();
}

void CCompent::SetParent(CGuiData* p) {
	if (p != _frmOwn)
		_isChild = true;
	_pParent = p;
}

void CCompent::SetIsDrag(bool v) {
	// 子控件无拖动
	if (_isChild)
		return;

	CGuiData::SetIsDrag(v);
}

CCompent* CCompent::GetHintCompent(int x, int y) {
	if (GetIsShow() && InRect(x, y) && !_strHint.empty())
		return this;

	return nullptr;
}

//---------------------------------------------------------------------------
// class CContainer
//---------------------------------------------------------------------------
CContainer::CContainer(CForm& frmOwn)
	: CCompent(frmOwn) {
}

CContainer::CContainer(const CContainer& rhs)
	: CCompent(rhs) {
	_SetSelf(rhs);
}

CContainer& CContainer::operator=(const CContainer& rhs) {
	CCompent::operator=(rhs);

	_SetSelf(rhs);
	return *this;
}

CContainer::~CContainer() {
}

void CContainer::Init() {
	for (auto& item : _items) {
		item->Init();
	}
}

void CContainer::_SetSelf(const CContainer& rhs) {
	CGuiData* p = nullptr;
	CCompent* c = nullptr;
	for (auto item : rhs._items) {
		p = item->Clone();

		c = dynamic_cast<CCompent*>(p);
		if (c) {
			_items.push_back(c);
			c->SetParent(this);
		}
	}
}

void CContainer::Render() {
	for (auto& item : _items) {
		if (item->GetIsShow()) {
			item->Render();
		}
	}
}

void CContainer::Refresh() {
	CCompent::Refresh();

	for (auto& item : _items) {
		item->Refresh();
	}
}

bool CContainer::MouseRun(int x, int y, MouseClickState key) {
	if (!IsNormal()) {
		return false;
	}

	for (auto& it : _mouse) {
		if (it->MouseRun(x, y, key)) {
			_pLastMouseCompent = it;
			return true;
		}
	}

	return false;
}

bool CContainer::AddCompent(CCompent* p) {
	if (p == this || p == _pParent) {
		return false;
	}

	if (auto it = std::find(_items.begin(), _items.end(), p); _items.end() == it) {
		_items.push_back(p);

		// 此处仅改变_pParnet,不改变_isChild,所以不调用SetParent
		p->_pParent = this;

		if (p->IsHandleMouse() || p->GetIsFocus()) {
			_mouse.push_back(p);
		}
		return true;
	}
	return false;
}

void CContainer::ForEach(CompentFun pFun) {
	int nIndex = 0;
	for (auto& item : _items)
		pFun(item, nIndex++);
}

int CContainer::GetIndex(CCompent* p) {
	int nIndex = 0;
	for (auto& item : _items) {
		nIndex++;
		if (item == p) {
			return nIndex;
		}
	}
	return -1;
}

void CContainer::SetAlpha(BYTE alpha) {
	for (auto& item : _items)
		item->SetAlpha(alpha);
}

CCompent* CContainer::Find(const char* str) {
	for (auto& item : _items) {
		if (strcmp(item->GetName(), str) == 0) {
			return item;
		}
	}

	return nullptr;
}

CCompent* CContainer::GetHitCommand(int x, int y) {
	CCompent* pCommand = nullptr;
	for (auto& item : _items) {
		if (pCommand = item->GetHitCommand(x, y)) {
			return pCommand;
		}
	}

	return nullptr;
}

CCompent* CContainer::GetHintCompent(int x, int y) {
	if (GetIsShow()) {
		if (InRect(x, y) && !_strHint.empty())
			return this;

		CCompent* pHint = nullptr;
		for (auto& item : _items) {
			if (pHint = item->GetHintCompent(x, y)) {
				return pHint;
			}
		}
	}
	return nullptr;
}

void CContainer::FrameMove(DWORD dwTime) {
	if (!GetIsShow())
		return;

	for (auto& item : _items) {
		item->FrameMove(dwTime);
	}
}
