#include "stdafx.h"
#include "uifastcommand.h"
#include "UICommand.h"
#include "UIFormMgr.h"

//---------------------------------------------------------------------------
// class CFastCommand
//---------------------------------------------------------------------------
CFastCommand::fasts CFastCommand::_fast;

CFastCommand::CFastCommand(CForm& frmOwn)
	: CCommandCompent(frmOwn) {
	_fast.push_back(this);

	_SetSelf();
}

CFastCommand::~CFastCommand() {
	auto it = std::find(_fast.begin(), _fast.end(), this);
	if (it != _fast.cend()) {
		_fast.erase(it);
	}
}

CFastCommand::CFastCommand(const CFastCommand& rhs)
	: CCommandCompent(rhs), _pCommand(rhs._pCommand), evtChange(rhs.evtChange) {
	_fast.push_back(this);

	_SetSelf();
}

CFastCommand& CFastCommand::operator=(const CFastCommand& rhs) {
	CCommandCompent::operator=(rhs);

	_pCommand = rhs._pCommand;
	evtChange = rhs.evtChange;
	return *this;
}

void CFastCommand::Render() {
	if (_pCommand) {
		_pCommand->Render(GetX(), GetY());
	}
}

bool CFastCommand::MouseRun(int x, int y, MouseClickState key) {
	if (!IsNormal()) {
		return false;
	}

	if (!_pCommand) {
		InRect(x, y);
	} else if (IsNoDrag(x, y, key)) {
		if (_pCommand && ((key & MouseClickState::LUp) != MouseClickState())) {
			_pCommand->Exec();
		}

		if ((key & MouseClickState::RDown) != MouseClickState()) {
			_pCommand = nullptr;
		}
	}

	return _IsMouseIn;
}

void CFastCommand::DelCommand(CCommandObj* p) {
	for (auto pFast : _fast) {
		if (pFast->_pCommand && pFast->_pCommand == p) {
			pFast->_pCommand = nullptr;
		}
	}
}

CFastCommand* CFastCommand::FindFastCommand(const CCommandObj* p) {
	auto it = std::find_if(_fast.cbegin(), _fast.cend(), [&p](CFastCommand* pFast) {
		if (pFast->_pCommand && pFast->_pCommand == p) {
			return true;
		}
		return false;
	});
	return it != _fast.cend() ? *it : nullptr;
}

eAccept CFastCommand::SetCommand(CCommandObj* p, int x, int y) {
	if (!p || (p && p->IsDragFast())) {
		bool isAccept = false;
		if (evtChange) {
			evtChange(this, p, isAccept);
		}
		if (isAccept) {
			if (p) {
				p->SetIsFast(true);
			}

			if (_pCommand && _pCommand != p) {
				_pCommand->SetIsFast(false);
			}

			_pCommand = p;
		}
		return enumFast;
	}
	return enumRefuse;
}

void CFastCommand::AddCommand(CCommandObj* p) {
	if (_pCommand && _pCommand != p) {
		_pCommand->SetIsFast(false);
	}
	_pCommand = p;
	if (p) {
		p->SetIsFast(true);
	}
}

void CFastCommand::RenderHint(int x, int y) {
	if (_pCommand) {
		_pCommand->RenderHint(x, y);
	} else {
		_RenderHint(_strHint.c_str(), x, y);
	}
}

void CFastCommand::DragRender() {
	if (_pCommand) {
		_pCommand->Render(GetX() + CDrag::GetDrag()->GetDragX(), GetY() + CDrag::GetDrag()->GetDragY());
	}
}

void CFastCommand::_DragEnd(int x, int y, MouseClickState key) {
	CForm* form = CFormMgr::s_Mgr.GetHitForm(x, y);
	if (form) {
		CCompent* p = form->GetHitCommand(x, y);
		if (!p) {
			return;
		}

		auto fast = dynamic_cast<CFastCommand*>(p);
		CCommandObj* oldp{nullptr};
		if (fast) {
			oldp = fast->GetCommand();
		}

		switch (p->SetCommand(_pCommand, x, y)) {
		case enumFast:
			SetCommand(oldp, x, y);
			break;
		case enumAccept:
			break;
		case enumRefuse:
			break;
		}
	} else {
		SetCommand(nullptr, x, y);
	}
}

void CFastCommand::_SetSelf() {
	if (!_pDrag) {
		_pDrag = std::make_unique<CDrag>();
	}

	_pDrag->SetIsMove(false);
	_pDrag->evtMouseDragEnd = _DragEnd;
}

CCompent* CFastCommand::GetHintCompent(int x, int y) {
	if (GetIsShow() && InRect(x, y)) {
		if (_pCommand) {
			SetHintItem(_pCommand);
			return this;
		} else if (!_strHint.empty()) {
			return this;
		}
	}
	return nullptr;
}

void CFastCommand::SetKeybind(unsigned short keybind) {
	_pKeybind = keybind;
}

unsigned short CFastCommand::GetKeybind() {
	return _pKeybind;
}

//---------------------------------------------------------------------------
// class COneCommand
//---------------------------------------------------------------------------
COneCommand::COneCommand(CForm& frmOwn)
	: CCommandCompent(frmOwn) {
	_SetSelf();
}

COneCommand::COneCommand(const COneCommand& rhs)
	: CCommandCompent(rhs), _pCommand(nullptr) {
	_SetSelf();
	if (rhs._pCommand) {
		_pCommand = rhs._pCommand->Clone();
	}
	_Copy(rhs);
}

COneCommand& COneCommand::operator=(const COneCommand& rhs) {
	CCommandCompent::operator=(rhs);

	if (_pCommand && _pCommand != rhs._pCommand) {
		delete _pCommand;
	}
	if (rhs._pCommand) {
		_pCommand = rhs._pCommand->Clone();
	}
	_Copy(rhs);
	return *this;
}

COneCommand::~COneCommand() {
	SAFE_DELETE(_pCommand); // UIµ±»ú´¦Àí
}

void COneCommand::_SetSelf() {
	if (!_pDrag) {
		_pDrag = std::make_unique<CDrag>();
	}

	_pDrag->SetIsMove(false);
	_pDrag->evtMouseDragEnd = _DragEnd;
}

void COneCommand::_Copy(const COneCommand& rhs) {
	evtThrowItem = rhs.evtThrowItem;
	evtBeforeAccept = rhs.evtBeforeAccept;
	evtUseCommand = rhs.evtUseCommand;

	_IsShowActive = rhs._IsShowActive;
	_eShowStyle = rhs._eShowStyle;
}

void COneCommand::Render() {
	if (_pCommand) {
		if (_eShowStyle == enumSmall) {
			_pCommand->Render(GetX(), GetY());
			if (_pActive && _IsShowActive) {
				_pCommand->RenderEnergy(GetX(), GetY());
				_pActive->Render(GetX(), GetY());
			}
		} else {
			_pCommand->SaleRender(GetX(), GetY(), GetWidth(), GetHeight());
		}
	}
}

bool COneCommand::MouseRun(int x, int y, MouseClickState key) {
	if (!IsNormal()) {
		return false;
	}

	if (!_pCommand) {
		InRect(x, y);
	} else if (IsNoDrag(x, y, key)) {
		if ((key & MouseClickState::LDown) != MouseClickState()) {
			if (_pCommand->MouseDown()) {
				return true;
			}
		}

		if ((key & MouseClickState::LDB) != MouseClickState()) {
			_pCommand->Exec();
		}
	}
	return _IsMouseIn;
}

void COneCommand::DragRender() {
	if (_pCommand) {
		_pCommand->Render(GetX() + CDrag::GetDrag()->GetDragX(), GetY() + CDrag::GetDrag()->GetDragY());
	}
}

void COneCommand::_DragEnd(int x, int y, MouseClickState key) {
	CForm* form = CFormMgr::s_Mgr.GetHitForm(x, y);
	if (form) {
		CCompent* p = form->GetHitCommand(x, y);
		if (!p) {
			return;
		}

		switch (p->SetCommand(_pCommand, x, y)) {
		case enumFast:
			break;
		case enumAccept:
			_pCommand = nullptr;
			break;
		case enumRefuse:
			break;
		}
	} else {
		if (_pCommand) {
			bool isThrow = false;

			if (evtThrowItem)
				evtThrowItem(this, _pCommand, isThrow);
			if (isThrow) {
				delete _pCommand;
				_pCommand = nullptr;
			}
		}
	}
}

eAccept COneCommand::SetCommand(CCommandObj* p, int x, int y) {
	bool isAccept = false;
	if (evtBeforeAccept) {
		evtBeforeAccept(this, p, isAccept);
	}

	if (isAccept) {
		AddCommand(p);
		return enumAccept;
	}
	return enumRefuse;
}

void COneCommand::DelCommand() {
	if (_pCommand) {
		delete _pCommand;
		_pCommand = nullptr;
	}
}

void COneCommand::RenderHint(int x, int y) {
	if (_pCommand) {
		_pCommand->RenderHint(x, y);
	} else {
		_RenderHint(_strHint.c_str(), x, y);
	}
}

void COneCommand::AddCommand(CCommandObj* p) {
	if (_pCommand && _pCommand != p) {
		delete _pCommand;
		_pCommand = nullptr;
	}

	p->SetParent(this);
	p->SetIndex(defCommandDefaultIndex);
	_pCommand = p;
}

CCompent* COneCommand::GetHintCompent(int x, int y) {
	if (GetIsShow() && InRect(x, y)) {
		if (_pCommand) {
			if (_eShowStyle == enumSmall) {
				SetHintItem(_pCommand);
			}
			return this;
		} else if (!_strHint.empty()) {
			return this;
		}
	}
	return nullptr;
}
