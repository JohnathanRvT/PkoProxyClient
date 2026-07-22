#include "StdAfx.h"
#include "uiGuidata.h"

using namespace GUI;

CItemObj::~CItemObj() {
	if (this == CGuiData::GetHintItem()) {
		CGuiData::SetHintItem(nullptr);
	}
}

//---------------------------------------------------------------------------
// class CGuiData
//---------------------------------------------------------------------------
CGuiData::vcs CGuiData::_AllGui;
int CGuiData::_nMouseX = 0;
int CGuiData::_nMouseY = 0;
CCursor::eState CGuiData::_eCursor = CCursor::stNormal;

float CGuiData::_fDrawX = 1.0;
float CGuiData::_fDrawY = 1.0;
float CGuiData::_fScreenX = 1.0;
float CGuiData::_fScreenY = 1.0;

CItemObj* CGuiData::_pHintItem = nullptr;

CGuiData::CGuiData() {
	_AllGui.push_back(this);
	_dwID = (DWORD)(_AllGui.size());
}

CGuiData::CGuiData(const CGuiData& rhs)
	: _pParent(rhs._pParent), _nWidth(rhs._nWidth), _nHeight(rhs._nHeight), _nLeft(rhs._nLeft), _nTop(rhs._nTop),
	  _nX1(rhs._nX1), _nX2(rhs._nX2), _nY1(rhs._nY1), _nY2(rhs._nY2), _bShow(rhs._bShow), _bEnabled(rhs._bEnabled),
	  _strName(rhs._strName), nTag(rhs.nTag), _pVoid(rhs._pVoid), _pDrag(nullptr), _IsMouseIn(false), _strHint(rhs._strHint) {
	_AllGui.push_back(this);
	_dwID = (DWORD)(_AllGui.size());

	if (!rhs._pDrag) {
		_pDrag = nullptr;
	} else {
		_pDrag = std::make_unique<CDrag>(*rhs._pDrag);
	}
}

CGuiData& CGuiData::operator=(const CGuiData& rhs) {
	// _Parent
	// _dwID
	_strName = rhs._strName;
	_nWidth = rhs._nWidth;
	_nHeight = rhs._nHeight;

	_nLeft = rhs._nLeft;
	_nTop = rhs._nTop;

	_nX1 = rhs._nX1;
	_nX2 = rhs._nX2;
	_nY1 = rhs._nY1;
	_nY2 = rhs._nY2;

	_bShow = rhs._bShow;
	_bEnabled = rhs._bEnabled;
	nTag = rhs.nTag;
	_pVoid = rhs._pVoid;

	_strHint = rhs._strHint;

	_pDrag = std::make_unique<CDrag>(*rhs._pDrag);

	_IsMouseIn = rhs._IsMouseIn;
	return *this;
}

CGuiData::~CGuiData() {
	//static char buf[256] = { 0 };
	//_snprintf_s( buf, _TRUNCATE, "%s, %d, %d\n", GetName(), _dwID, _pDrag ? 1 : 0 );
	//OutputDebugString( buf );

	_AllGui[_dwID - 1] = nullptr;
}

void CGuiData::ClearAllGui() {
	for (auto& it : _AllGui) {
		SAFE_DELETE(it); // UIµ±»ú´¦Àí
	}
	_AllGui.clear();
}

void CGuiData::Refresh() {
	_nX1 = _nLeft;
	_nY1 = _nTop;
	CGuiData* pParent = _pParent;
	while (pParent) {
		_nX1 += pParent->GetLeft();
		_nY1 += pParent->GetTop();

		pParent = pParent->GetParent();
	}
	_nX2 = _nX1 + _nWidth;
	_nY2 = _nY1 + _nHeight;
}

void CGuiData::InitMemory() {
	for (auto& it : _AllGui) {
		it->_AddForm();
	}
}

bool CGuiData::MouseRun(int x, int y, MouseClickState key) {
	if (!IsNormal()) {
		return false;
	}

	IsNoDrag(x, y, key);
	return _IsMouseIn;
}

void CGuiData::SetIsDrag(bool v) {
	if (v) {
		if (!_pDrag) {
			_pDrag = std::make_unique<CDrag>();
		}
	} else {
		if (_pDrag) {
			_pDrag.reset();
		}
	}
}

bool CGuiData::IsAllowActive() {
	if (!IsNormal()) {
		return false;
	}

	CGuiData* pParent = _pParent;
	while (pParent) {
		if (!pParent->IsNormal()) {
			return false;
		}

		pParent = pParent->GetParent();
	}
	return true;
}

void CGuiData::RenderHint(int x, int y) {
	CGuiFont::s_Font.FrameRender(_strHint.c_str(), x + 40, y);
}

bool CGuiData::SetHintItem(CItemObj* pObj) {
	if (pObj && pObj->HasHint()) {
		_pHintItem = pObj;
		return true;
	} else {
		_pHintItem = nullptr;
		return false;
	}
}

//---------------------------------------------------------------------------
// class CDrag
//---------------------------------------------------------------------------
CDrag* CDrag::_pDrag = nullptr;
CGuiData* CDrag::_pParent = nullptr;

DWORD CDrag::_dwGridWidth = 4;
DWORD CDrag::_dwGridHeight = 4;

int CDrag::_nDragX = 0;
int CDrag::_nDragY = 0;
int CDrag::_nStartX = 0;
int CDrag::_nStartY = 0;

CCursor::eState CDrag::_crNormal = CCursor::stNormal;

void CDrag::SetSnapToGrid(DWORD dwWidth, DWORD dwHeight) {
	if (dwWidth > 0) {
		_dwGridWidth = dwWidth;
	}
	if (dwHeight > 0) {
		_dwGridHeight = dwHeight;
	}
}

//---------------------------------------------------------------------------
// class CGuiTime
//---------------------------------------------------------------------------
CGuiTime::times CGuiTime::_times;

CGuiTime* CGuiTime::Find(DWORD id) {
	for (auto& it : _times)
		if (!it->_IsRelease && it->GetID() == id) {
			return it;
		}
	return nullptr;
}

CGuiTime* CGuiTime::Create(DWORD dwInterval, GuiTimerEvent evt) {
	CGuiTime* p = nullptr;
	for (auto& it : _times) {
		if (it->_IsRelease) {
			p = it;
		}
	}
	if (!p) {
		p = new CGuiTime;
		_times.push_back(p);
	}
	p->SetInterval(dwInterval);
	p->evtTime = evt;
	p->_dwLastTime = GetTickCount();
	p->_IsRelease = false;
	return p;
}

bool CGuiTime::Release() {
	_IsRelease = true;
	return true;
}

CGuiTime::CGuiTime(DWORD dwInterval, GuiTimerEvent evt)
	: _nEventID(0), _lpData(nullptr), evtTime(evt), _dwInterval(dwInterval) {
	static int n = 0;
	n++;
	_dwIndex = n;
}

CGuiTime::~CGuiTime() {
}

void CGuiTime::FrameMove(DWORD dwTime) {
	for (auto& it : _times) {
		it->OnTime(dwTime);
	}
}
