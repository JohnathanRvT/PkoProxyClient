#include "StdAfx.h"
#include "uipicture.h"

using namespace GUI;
//---------------------------------------------------------------------------
// class CGuiPic
//---------------------------------------------------------------------------
CGuiPic::CGuiPic(CGuiData* pParent, unsigned int max)
	: _pParent(pParent), _max(max) {
	if (_max == 0) {
		_max = 1;
	}
	_pImage = std::make_unique<MPTexRect[]>(_max);
}

CGuiPic::CGuiPic(const CGuiPic& rhs)
	: _pParent(rhs._pParent), _pImage(new MPTexRect[rhs._max]),
	  _max(rhs._max), _frame(rhs._frame), _bIsScale(rhs._bIsScale) {
	for (unsigned int i = 0; i < _max; i++) {
		_pImage[i] = rhs._pImage[i];
	}
}

CGuiPic& CGuiPic::operator=(const CGuiPic& rhs) {
	_max = rhs._max;
	_frame = rhs._frame;
	_bIsScale = rhs._bIsScale;
	for (unsigned int i = 0; i < _max; i++) {
		_pImage[i] = rhs._pImage[i];
	}
	return *this;
}

void CGuiPic::SetMax(int v) {
	if (v < 1) {
		return;
	}
	if (_max == v) {
		return;
	}
	_max = v;
	_pImage = std::make_unique<MPTexRect[]>(v);
	_frame = 0;
}

void CGuiPic::SetAlpha(BYTE alpha) {
	for (size_t i = 0; i < _max; i++) {
		_pImage[i].dwColor = (_pImage[i].dwColor & 0x00FFFFFF) | (alpha << 24);
	}
}

void CGuiPic::SetScale(int w, int h) {
	if (_pImage[0].nTextureNo == -1) {
		return;
	}

	for (size_t i = 0; i < _max; i++) {
		_pImage[i].fScaleX = (float)w / (float)_pImage[i].nTexW;
		_pImage[i].fScaleY = (float)h / (float)_pImage[i].nTexH;
	}
}

void CGuiPic::SetScaleW(int nW) {
	for (size_t i = 0; i < _max; i++) {
		_pImage[i].fScaleX = (float)nW / (float)_pImage[i].nTexW;
	}
}

void CGuiPic::SetScaleH(int nH) {
	for (size_t i = 0; i < _max; i++) {
		_pImage[i].fScaleY = (float)nH / (float)_pImage[i].nTexH;
	}
}

void CGuiPic::UnLoadImage(int frame) {
	if (frame < 0) {
		for (size_t i = 0; i < _max; i++) {
			_pImage[i] = {};
		}
	} else if (frame < (int)_max) {
		_pImage[frame] = {};
	}
}
//---------------------------------------------------------------------------
// class CFramePic
//---------------------------------------------------------------------------
CFramePic::CFramePic(CGuiData* pOwn)
	: _pImage(new MPTexRect[ppEnd]), _pOwn(pOwn) {
}

CFramePic::CFramePic(const CFramePic& rhs)
	: _pImage(new MPTexRect[ppEnd]), _bIsShowFrame(rhs._bIsShowFrame),
	  _bIsTitle(rhs._bIsTitle), _nX(rhs._nX), _nY(rhs._nY) {
	// _pOwn = rhs._pOwn;
	for (size_t i = 0; i < ppEnd; i++) {
		_pImage[i] = rhs._pImage[i];
	}
}

CFramePic& CFramePic::operator=(const CFramePic& rhs) {
	// _pOwn = rhs._pOwn;
	_bIsShowFrame = rhs._bIsShowFrame;
	_bIsTitle = rhs._bIsTitle;
	_nX = rhs._nX;
	_nY = rhs._nY;
	for (size_t i = 0; i < ppEnd; i++) {
		_pImage[i] = rhs._pImage[i];
	}
	return *this;
}

void CFramePic::SetAlpha(BYTE alpha) {
	for (size_t i = 0; i < ppEnd; i++) {
		_pImage[i].dwColor = (_pImage[i].dwColor & 0x00FFFFFF) | (alpha << 24);
	}
}
