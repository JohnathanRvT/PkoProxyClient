#include "StdAfx.h"
#include "uigoodsgrid.h"
#include "UICommand.h"
#include "UIFormMgr.h"

using namespace GUI;

//---------------------------------------------------------------------------
// class CGoodsGrid
//---------------------------------------------------------------------------

int CGoodsGrid::_nTmpX = 0;
int CGoodsGrid::_nTmpY = 0;
int CGoodsGrid::_nTmpRow = 0;
int CGoodsGrid::_nTmpCol = 0;

CGoodsGrid::CGoodsGrid(CForm& frmOwn)
	: CCommandCompent(frmOwn),
	  _pImage{std::make_unique<CGuiPic>(this)},
	  _pScroll{std::make_unique<CScroll>(frmOwn)},
	  _pUnit{std::make_unique<CGuiPic>(this)} {
	_pDrag = std::make_unique<CDrag>();
	_SetSelf();
}

CGoodsGrid::CGoodsGrid(const CGoodsGrid& rhs)
	: CCommandCompent(rhs), _pItems(nullptr), _nRow(0), _nCol(0) {
	_pScroll = std::make_unique<CScroll>(*rhs._pScroll);
	_pImage = std::make_unique<CGuiPic>(*rhs._pImage);
	_pUnit = std::make_unique<CGuiPic>(*rhs._pUnit);

	_pDrag = std::make_unique<CDrag>();

	_Copy(rhs);
}

CGoodsGrid& CGoodsGrid::operator=(const CGoodsGrid& rhs) {
	CCommandCompent::operator=(rhs);

	_ClearItem();

	*_pScroll = *rhs._pScroll;
	*_pImage = *rhs._pImage;
	*_pUnit = *rhs._pUnit;

	_Copy(rhs);
	return *this;
}

void CGoodsGrid::_Copy(const CGoodsGrid& rhs) {
	_nLeftMargin = rhs._nLeftMargin;
	_nTopMargin = rhs._nTopMargin;
	_nRightMargin = rhs._nRightMargin;
	_nBottomMargin = rhs._nBottomMargin;

	_nUnitHeight = rhs._nUnitHeight;
	_nUnitWidth = rhs._nUnitWidth;

	_nSpaceX = rhs._nSpaceX;
	_nSpaceY = rhs._nSpaceY;

	_nMaxNum = rhs._nMaxNum;

	_nTotalW = rhs._nTotalW;
	_nTotalH = rhs._nTotalW;

	evtThrowItem = rhs.evtThrowItem;
	evtSwapItem = rhs.evtSwapItem;
	evtBeforeAccept = rhs.evtBeforeAccept;
	evtUseCommand = rhs.evtUseCommand;
	evtRMouseEvent = rhs.evtRMouseEvent;

	_eShowStyle = rhs._eShowStyle;
	_IsShowHint = rhs._IsShowHint;

	SetContent(rhs._nRow, rhs._nCol);

	//for( int i=0; i<_nMaxNum; i++ )
	//{
	//	if( rhs._pItems[i] )
	//		_pItems[i] = rhs._pItems[i]->Clone();
	//}

	_nFirst = rhs._nFirst;
	_nLast = rhs._nLast;

	_SetSelf();
}

void CGoodsGrid::_SetSelf() {
	_pScroll->SetParent(this);
	_pScroll->SetIsShow(false);
	_pScroll->evtChange = _OnScrollChange;

	_pImage->SetParent(this);

	if (_pDrag) {
		_pDrag->SetIsMove(false);
		_pDrag->evtMouseDragBegin = nullptr;
		_pDrag->evtMouseDragMove = nullptr;
		_pDrag->evtMouseDragEnd = _DragEnd;
	}

	_pDragItem = nullptr;
	_nDragIndex = -1;
}

CGoodsGrid::~CGoodsGrid() {
	_ClearItem();
}

void CGoodsGrid::SetAlpha(BYTE alpha) {
	_pScroll->SetAlpha(alpha);
	_pImage->SetAlpha(alpha);
	_pUnit->SetAlpha(alpha);
}

void CGoodsGrid::Render() {
	_pImage->Render(GetX(), GetY());

	static int i;
	static int x, y, col;
	x = _nStartX;
	y = _nStartY;
	col = 0;

	if (!_pUnit->IsNull()) {
		for (i = _nFirst; i < _nLast; i++) {
			_pUnit->Render(x, y);
			if (++col >= _nCol) {
				col = 0;
				y += _nTotalH;
				x = _nStartX;
			} else {
				x += _nTotalW;
			}
		}
		x = _nStartX;
		y = _nStartY;
		col = 0;
	}

	switch (_eShowStyle) {
	case eShowStyle::Small: {
		for (i = _nFirst; i < _nLast; i++) {
			if (_pItems[i]) {
				_pItems[i]->Render(x, y);
			}

			if (++col >= _nCol) {
				col = 0;
				y += _nTotalH;
				x = _nStartX;
			} else {
				x += _nTotalW;
			}
		}
	} break;
	case eShowStyle::Sale: {
		for (i = _nFirst; i < _nLast; i++) {
			if (_pItems[i]) {
				_pItems[i]->SaleRender(x, y, _nUnitWidth, _nUnitHeight);
			}

			if (++col >= _nCol) {
				col = 0;
				y += _nTotalH;
				x = _nStartX;
			} else {
				x += _nTotalW;
			}
		}
	} break;
	case eShowStyle::OwnDef: {
		for (i = _nFirst; i < _nLast; i++) {
			if (_pItems[i]) {
				_pItems[i]->OwnDefRender(x, y, _nUnitWidth, _nUnitHeight);
			}

			if (++col >= _nCol) {
				col = 0;
				y += _nTotalH;
				x = _nStartX;
			} else {
				x += _nTotalW;
			}
		}
	} break;
	}

	if (_pScroll->GetIsShow()) {
		_pScroll->Render();
	}
}

void CGoodsGrid::Refresh() {
	CCompent::Refresh();

	_pImage->Refresh();
	_pScroll->Refresh();

	_nStartX = GetX() + _nLeftMargin;
	_nStartY = GetY() + _nTopMargin;

	_nPageShowNum = _nCol * (int)((GetHeight() - _nBottomMargin - _nTopMargin) / _nTotalH);
	_OnScrollChange();
}

//	2008-9-17	yangyinyu	add	begin!
extern bool _wait_select_lock_item_state_; //	�û��Ѿ�ʹ�������ߣ����ڵȴ�ѡ�������ĵ��ߡ�
//	2008-9-17	yangyinyu	add	end!

bool CGoodsGrid::MouseRun(int x, int y, MouseClickState key) {
	if (!IsNormal()) {
		return false;
	}

	if (InRect(x, y)) {
		if (((key & MouseClickState::LDown) != MouseClickState()) && !_isChild && GetActive() != this)
			_SetActive();

		if (_pScroll->MouseRun(x, y, key)) {
			return true;
		}

		_GetHitItem(x, y);
		if (_nTmpIndex == -1) {
			return true;
		}

		if ((key & MouseClickState::LDown) != MouseClickState()) {
			if (_pItems[_nTmpIndex] && _pItems[_nTmpIndex]->MouseDown()) {
				return true;
			}
		}

		if ((key & MouseClickState::LDB) != MouseClickState()) {
			if (_pItems[_nTmpIndex]) {
				_pItems[_nTmpIndex]->Exec();
			}
			return true;
		}

		if ((key & MouseClickState::RDown) != MouseClickState()) {
			//	2008-9-17	yangyinyu	add	begin!
			if (_wait_select_lock_item_state_ == true) {
				_wait_select_lock_item_state_ = false;
				CCursor::I()->SetCursor(CCursor::stNormal);
			};
			//	2008-9-17	yangyinyu	add	end!

			if (evtRMouseEvent && _pItems[_nTmpIndex]) {
				evtRMouseEvent(this, _pItems[_nTmpIndex], _nTmpIndex);
			}
			return true;
		}

		if (_pDrag && _pDrag->BeginMouseRun(this, _IsMouseIn, x, y, key) == CDrag::eState::Drag) {
			_GetHitItem(_pDrag->GetStartX(), _pDrag->GetStartY());
			if (_nTmpIndex == -1) {
				_pDrag->Reset();
				return true;
			}

			if (_pItems[_nTmpIndex]) {
				_pDragItem = _pItems[_nTmpIndex];

				_nDragIndex = _nTmpIndex;
				_nDragOffX = _nTmpX;
				_nDragOffY = _nTmpY;
				_nDragRow = _nTmpRow;
				_nDragCol = _nTmpCol;
			} else {
				_pDrag->Reset();
			}
		}
	}

	return _IsMouseIn;
}

int CGoodsGrid::_GetHitItem(int x, int y) {
	_nTmpX = x - _nStartX;
	_nTmpCol = _nTmpX / _nTotalW;
	if (_nTmpCol >= _nCol) {
		_nTmpIndex = -1;
		return _nTmpIndex;
	}

	_nTmpY = y - _nStartY;
	_nTmpRow = _nTmpY / _nTotalH;
	_nTmpIndex = _nTmpRow * _nCol + _nTmpCol + _nFirst;
	if (_nTmpIndex >= _nLast) {
		_nTmpIndex = -1;
		return _nTmpIndex;
	}

	_nTmpX %= _nTotalW;
	_nTmpY %= _nTotalH;
	return _nTmpIndex;
}

void CGoodsGrid::_OnScrollChange() {
	_nFirst = (int)_pScroll->GetStep().GetPosition() * _nCol;
	_nLast = _nFirst + _nPageShowNum;
	if (_nLast > _nMaxNum) {
		_nLast = _nMaxNum;
	}
}

void CGoodsGrid::Init() {
	_nTotalW = _nSpaceX + _nUnitWidth;
	_nTotalH = _nSpaceY + _nUnitHeight;

	_pUnit->SetScale(_nUnitWidth, _nUnitHeight);

	_pScroll->SetPos(GetWidth() - _nRightMargin - _pScroll->GetWidth(), _nTopMargin);

	const int nHeight = GetHeight() - _nBottomMargin - _nTopMargin;
	const int nRowHeight = nHeight / _nTotalH;
	const float fMax = (float)(_nRow - nRowHeight);
	_pScroll->SetSize(_pScroll->GetWidth(), nHeight);
	_pScroll->SetRange(0.0f, fMax);
	// xuqin modified
	//_pScroll->SetRange( 0.0f, _nRow );
	_pScroll->Init();
}

void CGoodsGrid::SetMargin(int left, int top, int right, int bottom) {
	_nLeftMargin = left;
	_nTopMargin = top;
	_nRightMargin = right;
	_nBottomMargin = bottom;
}

bool CGoodsGrid::SetContent(int nRow, int nCol) {
	if (nCol <= 0 || nRow <= 0) {
		return false;
	}

	_nCurNum = 0;
	if (nCol == _nCol && nRow == _nRow) {
		return true;
	}

	int oldMax = _nMaxNum;
	CCommandObj** pOldItems = _pItems;

	_nRow = nRow;
	_nCol = nCol;

	_nMaxNum = nRow * nCol;

	_pItems = new CCommandObj* [_nMaxNum] {};

	if (pOldItems) {
		int nMin = oldMax < _nMaxNum ? oldMax : _nMaxNum;
		memcpy(_pItems, pOldItems, sizeof(CCommandObj*) * nMin);
		for (size_t i = nMin; i < oldMax; i++) {
			SAFE_DELETE(pOldItems[i]);
		}
	}
	Init();
	return true;
}

bool CGoodsGrid::SetItem(unsigned int nIndex, CCommandObj* pItem) {
	if (nIndex >= (unsigned int)_nMaxNum) {
		return false;
	}

	if (_pItems[nIndex]) {
		delete _pItems[nIndex];
		_pItems[nIndex] = nullptr;
	} else {
		_nCurNum++;
	}

	_pItems[nIndex] = pItem;
	pItem->SetParent(this);
	pItem->SetIndex(nIndex);
	return true;
}

bool CGoodsGrid::DelItem(int nIndex) {
	if (nIndex >= _nMaxNum) {
		return false;
	}

	if (_pItems[nIndex]) {
		_nCurNum--;
		if (_pDragItem == _pItems[nIndex]) {
			if (CDrag::GetParent() == this && _pDrag) {
				_pDrag->Reset();
			}
			_pDragItem = nullptr;
		}

		delete _pItems[nIndex];
		_pItems[nIndex] = nullptr;
		return true;
	}

	return false;
}

size_t CGoodsGrid::GetFreeIndex() const {
	for (size_t i = 0; i < _nMaxNum; i++) {
		if (!_pItems[i]) {
			return i;
		}
	}
	return -1;
}

void CGoodsGrid::Clear() {
	for (size_t i = 0; i < _nMaxNum; i++) {
		SAFE_DELETE(_pItems[i]);
	}
}

void CGoodsGrid::_ClearItem() {
	if (_pItems) {
		for (size_t i = 0; i < _nMaxNum; i++) {
			SAFE_DELETE(_pItems[i]);
		}
		SAFE_DELETE_ARRAY(_pItems);
		_nMaxNum = 0;
		_nRow = 0;
		_nCol = 0;
	}
}

bool CGoodsGrid::MouseScroll(int nScroll) {
	if (!IsNormal()) {
		return false;
	}

	if (_IsMouseIn) {
		_pScroll->MouseScroll(nScroll);
	}
	return _IsMouseIn;
}

void CGoodsGrid::DragRender() {
	if (_pDragItem) {
		switch (_eShowStyle) {
		case eShowStyle::Small: {
			_pDragItem->Render(_pDrag->GetX() - _nDragOffX, _pDrag->GetY() - _nDragOffY);
		} break;
		case eShowStyle::Sale: {
			_pDragItem->SaleRender(_pDrag->GetX() - _nDragOffX, _pDrag->GetY() - _nDragOffY, _nUnitWidth, _nUnitHeight);
		} break;
		case eShowStyle::OwnDef: {
			_pDragItem->OwnDefRender(_pDrag->GetX() - _nDragOffX, _pDrag->GetY() - _nDragOffY, _nUnitWidth, _nUnitHeight);
		} break;
		}
	}
}

void CGoodsGrid::_DragEnd(int x, int y, MouseClickState key) {
	_pDragItem = nullptr;
	if (InRect(x, y)) {
		_GetHitItem(x, y);
		if (_nTmpIndex != -1) {
			if (_nTmpIndex != _nDragIndex) {
				bool isSwap = false;
				if (evtSwapItem) {
					//NOTE: isSwap may change in evtSwapItem
					evtSwapItem(this, _nTmpIndex, _nDragIndex, isSwap);
				}

				if (isSwap) {
					std::swap(_pItems[_nTmpIndex], _pItems[_nDragIndex]);
				}
			}
		}
		return;
	}

	CForm* form = CFormMgr::s_Mgr.GetHitForm(x, y);
	if (form) {
		CCompent* p = form->GetHitCommand(x, y);
		if (!p) {
			return;
		}

		switch (p->SetCommand(_pItems[_nDragIndex], x, y)) {
		case enumFast: {
		} break;
		case enumAccept: {
			_pItems[_nDragIndex] = nullptr;
		} break;
		case enumRefuse: {
		} break;
		}
	} else {
		if (_pItems[_nDragIndex]) {
			bool isThrow = false;

			if (evtThrowItem) {
				//NOTE: isThrow may change in evtThrowItem
				evtThrowItem(this, _nDragIndex, isThrow);
			}
			if (isThrow) {
				delete _pItems[_nDragIndex];
				_pItems[_nDragIndex] = nullptr;
			}
		}
	}
}

eAccept CGoodsGrid::SetCommand(CCommandObj* p, int x, int y) {
	if (!p) {
		return enumRefuse;
	}

	_GetHitItem(x, y);
	if (_nTmpIndex != -1) {
		bool isAccept = false;
		if (evtBeforeAccept) {
			//NOTE: isAccept may change in evtBeforeAccept
			evtBeforeAccept(this, p, _nTmpIndex, isAccept);
		}

		if (isAccept) {
			if (_pItems[_nTmpIndex]) {
				delete _pItems[_nTmpIndex];
				_pItems[_nTmpIndex] = nullptr;
				_nCurNum--;
			}

			SetItem(_nTmpIndex, p);
			return enumAccept;
		}
	}
	return enumRefuse;
}

void CGoodsGrid::RenderHint(int x, int y) {
	if (_nTmpIndex != -1 && _pItems[_nTmpIndex]) {
		_pItems[_nTmpIndex]->RenderHint(x, y);
	} else {
		_RenderHint(_strHint.c_str(), x, y);
	}
}

CCompent* CGoodsGrid::GetHintCompent(int x, int y) {
	if (GetIsShow() && InRect(x, y)) {
		if (!_strHint.empty())
			return this;

		if (_IsShowHint) {
			_GetHitItem(x, y);
			if (_nTmpIndex != -1 && _pItems[_nTmpIndex] && SetHintItem(_pItems[_nTmpIndex])) {

				return this;
			}
		}
	}
	return nullptr;
}

void CGoodsGrid::GetFreeIndex(int* nFreeIndexes, size_t& nCount, size_t nSize) const {
	nCount = 0;
	if (nSize <= 0) {
		return;
	}

	for (size_t i = 0; i < _nMaxNum; i++) {
		if (!_pItems[i]) {
			nFreeIndexes[nCount++] = i;
			if (nCount >= nSize) {
				return;
			}
		}
	}
}

size_t CGoodsGrid::FindCommand(CCommandObj* p) const {
	for (size_t i = 0; i < _nMaxNum; i++) {
		if (_pItems[i] == p) {
			return i;
		}
	}
	return -1;
}

bool CGoodsGrid::SwapItem(unsigned int nFirst, unsigned int nSecond) {
	if (nFirst > (unsigned int)_nMaxNum ||
		nSecond > (unsigned int)_nMaxNum) {
		return false;
	}

	std::swap(_pItems[nFirst], _pItems[nSecond]);
	if (_pItems[nFirst]) {
		_pItems[nFirst]->SetIndex(nFirst);
	}
	if (_pItems[nSecond]) {
		_pItems[nSecond]->SetIndex(nSecond);
	}

	return true;
}

void CGoodsGrid::Reset() {
	_pScroll->Reset();
}

void CGoodsGrid::SetItemValid(bool v) {
	for (size_t i = 0; i < _nMaxNum; i++) {
		if (_pItems[i]) {
			_pItems[i]->SetIsValid(v);
		}
	}
}

int CGoodsGrid::GetEmptyGridCount() {
	int nCount = 0;
	for (int i = 0; i < _nMaxNum; i++) {
		if (!_pItems[i]) {
			++nCount;
		}
	}
	return nCount;
}

int CGoodsGrid::GetUsedGridCount() {
	int nCount = 0;
	for (int i = 0; i < _nMaxNum; i++) {
		if (_pItems[i]) {
			++nCount;
		}
	}
	return nCount;
}
