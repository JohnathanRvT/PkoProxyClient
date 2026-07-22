#include "StdAfx.h"
#include "UISkillList.h"
#include "uiformmgr.h"

using namespace GUI;

CSkillList::CSkillList(CForm& frmOwn)
	: CCompent(frmOwn), _pScroll(new CScroll(frmOwn)),
	  _pButton(new CGuiPic(this, CTextButton::csEnd)),
	  _pImage(new CGuiPic(this)), _pSelect(new CGuiPic()) {
	// _IsFocus = true;

	_pDrag = std::make_unique<CDrag>();

	_SetSelf();
}

CSkillList::CSkillList(const CSkillList& rhs)
	: CCompent(rhs), _pScroll(new CScroll(*rhs._pScroll)),
	  _pSelect(new CGuiPic()), _pImage(new CGuiPic(this)),
	  _pButton(new CGuiPic(*rhs._pButton)) {

	_pDrag = std::make_unique<CDrag>();
	_Copy(rhs);
	_SetSelf();
}

CSkillList& CSkillList::operator=(const CSkillList& rhs) {
	_Copy(rhs);
	_SetSelf();
	return *this;
}

CSkillList::~CSkillList() {
	Clear();
}

void CSkillList::_SetSelf() {
	_pImage->SetParent(this);

	_pButton->SetParent(this);
	_pScroll->SetParent(this);
	_pScroll->evtChange = _ScrollChange;

	if (_pDrag) {
		_pDrag->SetIsMove(false);
		_pDrag->evtMouseDragBegin = nullptr;
		_pDrag->evtMouseDragMove = nullptr;
		_pDrag->evtMouseDragEnd = _DragEnd;
	}
}

void CSkillList::_Copy(const CSkillList& rhs) {
}

void CSkillList::Init() {
	// 初始化控件
	if (_nRowHeight <= 0) {
		_nRowHeight = 34;
	}
	if (_nUnitHeight <= 0 || _nUnitHeight > _nRowHeight) {
		_nUnitHeight = _nRowHeight;
	}

	_pScroll->SetSize(_pScroll->GetWidth(), GetHeight() - _nTopMargin - _nBottomMargin);
	_pScroll->SetPos(GetWidth() - _nRightMargin - _pScroll->GetWidth(), _nTopMargin);
	_pScroll->Init();

	_ResetPageNum();

	_nFontYOff = (_nRowHeight - CGuiFont::s_Font.GetHeight("\n")) / 2;

	_pSelect->SetScale(GetWidth() - _nLeftMargin - _nRightMargin - _nUnitWidth - _pScroll->GetWidth(), _nUnitHeight);

	// 测试
	if (GetCommand(0)) {
		return;
	}

	CSkillRecord* info;
	const int count = CSkillRecordSet::I()->GetLastID() + 1;
	for (int i = 0; i < count; i++) {
		info = GetSkillRecordInfo(i);
		if (info && info->IsShow()) {
			auto command = std::make_unique<CSkillCommand>(info);
			AddCommand(std::move(command));
		}
	}
}

void CSkillList::Render() {
	_pImage->Render(GetX(), GetY());

	if (_nSelectIndex >= _nShowFirst && _nSelectIndex < _nShowLast) {
		_pSelect->Render(_nSX1 + _nUnitWidth, _nSY1 + _nRowHeight * (_nSelectIndex - _nShowFirst));
	}

	static int y = 0, x = 0;
	x = _nSX1;
	y = _nSY1;

	for (int i = _nShowFirst; i < _nShowLast; i++) {
		auto& skill = _skills[i];
		skill->Render(x, y);
		CGuiFont::s_Font.Render(skill->GetSkillName(), x + _nFontStart, y + _nFontYOff, _dwFontColor);

		y += _nRowHeight;
	}

	// 显示升级按钮
	y = _nSY1 + _nButtonOffY;
	for (int i = _nShowFirst; i < _nShowLast; i++) {
		switch (_skills[i]->GetSkillRecord()->GetUpgrade()) {
		case enumSKILL_UPGRADE_NONE: {
		} break;
		case enumSKILL_UPGRADE_CAN: {
			if (_IsShowUpgrade && !_skills[i]->GetIsSpecial(CSkillCommand::enumNotUpgrade)) {
				if (_nMouseUpgradeID == i) {
					_pButton->FrameRender(1, _nButtonX1, y);
				} else {
					_pButton->FrameRender(0, _nButtonX1, y);
				}
			}
		} break;
		case enumSKILL_UPGRADE_MAX: {
			_pButton->FrameRender(3, _nButtonX1, y);
		} break;
		}

		y += _nRowHeight;
	}

	if (_pScroll->GetIsShow()) {
		_pScroll->Render();
	}
}

void CSkillList::Refresh() {
	CCompent::Refresh();

	_pImage->Refresh();
	_pSelect->Refresh();
	_pButton->Refresh();

	_nSX1 = _nLeftMargin + GetX();
	_nSY1 = _nTopMargin + GetY();
	_nSX2 = GetX2() - _nRightMargin;
	_nSY2 = GetY2() - _nBottomMargin;

	_nButtonX1 = _nSX2 - _pButton->GetWidth() - _pScroll->GetWidth() - 5;
	_nButtonOffY = (_nRowHeight - _pButton->GetHeight()) / 2;

	int h = GetHeight() - _nTopMargin - _nBottomMargin;
	h = h + _nRowHeight - _nUnitHeight;
	_nShowCount = h / _nRowHeight;

	_ResetPageNum();

	auto fMax = (float)(_skills.size() - _nShowCount);
	if (fMax <= 0.0f)
		fMax = 0.0f;
	_pScroll->SetRange(0.0f, fMax);
	_SetFirstShowRow(_nShowFirst);

	_pScroll->Refresh();
}

bool CSkillList::MouseRun(int x, int y, MouseClickState key) {
	if (!IsNormal()) {
		return false;
	}

	if (InRect(x, y)) {
		if (((key & MouseClickState::LDown) != MouseClickState()) && !_isChild && GetActive() != this) {
			_SetActive();
		}

		if (_pScroll->MouseRun(x, y, key)) {
			return true;
		}

		_nMouseUpgradeID = -1;

		const int row = _GetHitSkill(x, y);
		if (row == -1) {
			return true;
		}

		if (_pDrag && _pDrag->BeginMouseRun(this, _IsMouseIn, x, y, key) == CDrag::eState::Drag) {
			_nDragIndex = row;

			_nDragOffX = _nUnitWidth / 2;
			_nDragOffY = y - _nSY1 - (row - _nShowFirst) * _nRowHeight;
			return true;
		}

		if ((key & MouseClickState::LDown) != MouseClickState()) {
			if (evtUpgrade && _IsShowUpgrade && _skills[row]->GetSkillRecord()->GetIsUpgrade() &&
				!_skills[row]->GetIsSpecial(CSkillCommand::enumNotUpgrade)) {
				int tmpX = x;
				int tmpY = y - _nSY1 - (row - _nShowFirst) * _nRowHeight;

				if (tmpX >= _nButtonX1 && tmpX <= _nButtonX1 + _pButton->GetWidth() && tmpY >= _nButtonOffY && tmpY <= _nButtonOffY + _pButton->GetHeight()) {
					evtUpgrade(this, _skills[row].get());
					return true;
				}
			}

			if (row != _nSelectIndex) {
				_nSelectIndex = row;
			}
		} else if ((key & MouseClickState::Move) != MouseClickState()) {
			int tmpX = x;
			int tmpY = y - _nSY1 - (row - _nShowFirst) * _nRowHeight;

			if (tmpX >= _nButtonX1 && tmpX <= _nButtonX1 + _pButton->GetWidth() && tmpY >= _nButtonOffY && tmpY <= _nButtonOffY + _pButton->GetHeight()) {
				_nMouseUpgradeID = row;
			}
		}

		if ((key & MouseClickState::LDB) != MouseClickState()) {
			if (_nSelectIndex >= 0 && _nSelectIndex < (int)_skills.size()) {
				_skills[_nSelectIndex]->Exec();
			}
		}
		return true;
	}

	return _IsMouseIn;
}

bool CSkillList::MouseScroll(int nScroll) {
	if (!IsNormal()) {
		return false;
	}

	if (_IsMouseIn) {
		return _pScroll->MouseScroll(nScroll);
	}
	return _IsMouseIn;
}

void CSkillList::SetAlpha(BYTE alpha) {
	_pImage->SetAlpha(alpha);
	_pSelect->SetAlpha(alpha);
	_pButton->SetAlpha(alpha);
	_pScroll->SetAlpha(alpha);
}

void CSkillList::SetMargin(int left, int top, int right, int bottom) {
	_nLeftMargin = left;
	_nTopMargin = top;
	_nRightMargin = right;
	_nBottomMargin = bottom;
}

void CSkillList::_OnScrollChange() {
	_SetFirstShowRow((DWORD)_pScroll->GetStep().GetPosition());
}

void CSkillList::_SetFirstShowRow(DWORD row) {
	row = row < _skills.size() ? row : 0;

	_nShowFirst = row;
	_nShowLast = _nShowCount + _nShowFirst;
	if (_nShowLast > (int)_skills.size()) {
		_nShowLast = (int)_skills.size();
	}
}

bool CSkillList::OnKeyDown(int key) {
	if (!GetIsEnabled()) {
		return false;
	}

	//_pScroll->OnKeyDown( key );

	switch (key) {
	case VK_LEFT:
	case VK_UP: {
		_nSelectIndex--;
	} break;
	case VK_RIGHT:
	case VK_DOWN: {
		_nSelectIndex++;
	} break;
	case VK_HOME: {
		_nSelectIndex = 0;
	} break;
	case VK_END: {
		_nSelectIndex = (int)_skills.size() - 1;
	} break;
	case VK_PRIOR: // pageup
	{
		_nSelectIndex -= _pScroll->GetPageNum();
	} break;
	case VK_NEXT: // pagedown
	{
		_nSelectIndex += _pScroll->GetPageNum();
	} break;
	default: {
		return false;
	}
	}

	if (_nSelectIndex < 0) {
		_nSelectIndex = 0;
	}

	if (!_skills.empty() && _nSelectIndex >= (int)_skills.size()) {
		_nSelectIndex = (int)_skills.size() - 1;
	}

	bool outRange = false;
	if (_nSelectIndex < _nShowFirst) {
		_SetFirstShowRow(_nSelectIndex);
		outRange = true;
	}

	if (_nShowLast > 0 && _nSelectIndex >= _nShowLast) {
		int n = _nSelectIndex - _nShowCount + 1;
		if (n < 0) {
			n = 0;
		}
		_SetFirstShowRow(n);
		outRange = true;
	}

	if (outRange) {
		_pScroll->GetStep().SetPosition((float)_nSelectIndex);
		_pScroll->Refresh();
	}

	return true;
}

void CSkillList::_DragEnd(int x, int y, MouseClickState key) {
	CForm* form = CFormMgr::s_Mgr.GetHitForm(x, y);
	if (form) {
		CCompent* p = form->GetHitCommand(x, y);
		if (!p) {
			return;
		}

		switch (p->SetCommand(_skills[_nDragIndex].get(), x, y)) {
		case enumFast: {
		} break;
		case enumAccept: {
			// _skills[_nDragIndex] = NULL;
		} break;
		case enumRefuse: {
		} break;
		}
	}
}

void CSkillList::DragRender() {
	if (_nDragIndex >= 0 && _nDragIndex < (int)_skills.size()) {
		_skills[_nDragIndex]->Render(_pDrag->GetX() - _nDragOffX, _pDrag->GetY() - _nDragOffY);
	}
}

CSkillCommand* CSkillList::FindSkill(int nID) {
	const auto count = (int)_skills.size();
	for (int i = 0; i < count; i++) {
		if (_skills[i] && _skills[i]->GetSkillID() == nID) {
			return _skills[i].get();
		}
	}

	return nullptr;
}

bool CSkillList::DelSkill(int nID) {
	for (auto it = _skills.begin(); it < _skills.end(); it++) {
		if ((*it)->GetSkillID() == nID) {
			_skills.erase(it);
			return true;
		}
	}
	return false;
}

int CSkillList::FindCommand(CSkillCommand* p) {
	for (size_t i = 0; i < _skills.size(); i++) {
		if (_skills[i].get() == p) {
			return i;
		}
	}
	return -1;
}

bool GUI::CSkillList::AddCommand(std::unique_ptr<CSkillCommand> p) {
	auto it = std::find(_skills.cbegin(), _skills.cend(), p);
	if (it == _skills.cend()) {
		p->SetParent(this);
		p->SetIndex((int)_skills.size());
		_skills.emplace_back(std::move(p));
		return true;
	}
	return false;
}

void CSkillList::Clear() {
	_skills.clear();
}

CCompent* CSkillList::GetHintCompent(int x, int y) {
	if (GetIsShow() && InRect(x, y)) {
		if (!_strHint.empty()) {
			return this;
		}

		int row = _GetHitSkill(x, y);
		if (row != -1) {
			SetHintItem(_skills[row].get());
			return this;
		}
	}
	return nullptr;
}
