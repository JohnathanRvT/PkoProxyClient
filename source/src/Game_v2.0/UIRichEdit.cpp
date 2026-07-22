#include "StdAfx.h"
#include "uirichedit.h"
#include "gameapp.h"
#include "uieditkey.h"

using namespace GUI;
//---------------------------------------------------------------------------
// class CRichEdit
//---------------------------------------------------------------------------
CRichEdit::CRichEdit(CForm& frmOwn)
	: CCompent(frmOwn), _IsReadyOnly(false) {
	_IsFocus = true;

	_pEditKey = new CEditKey;

	_SetSelf();
}

CRichEdit::CRichEdit(const CRichEdit& rhs)
	: CCompent(rhs) {
	_pEditKey = new CEditKey;

	_Copy(rhs);
}

CRichEdit& CRichEdit::operator=(const CRichEdit& rhs) {
	_Copy(rhs);
	return *this;
}

void CRichEdit::_SetSelf() {
	_cArticle.SetEdit(this);
}

void CRichEdit::_Copy(const CRichEdit& rhs) {
	_SetSelf();
}

CRichEdit::~CRichEdit() {
}

void CRichEdit::Init() {
	_pEditKey->Init();
}

void CRichEdit::Refresh() {
}

void CRichEdit::OnActive() {
}

void CRichEdit::OnLost() {
}

bool CRichEdit::OnKeyDown(int key) {
	if (_IsReadyOnly)
		return false;

	_cArticle.OnKeyDown(key, g_pGameApp->IsShiftPress() != 0);
	switch (key) {
	case VK_LEFT:
		break;
	case VK_RIGHT:
		break;
	case VK_UP:
		break;
	case VK_DOWN:
		break;
	case VK_HOME:
		break;
	case VK_END:
		break;
	case VK_PRIOR: // pageup
		break;
	case VK_NEXT: // pagedown
		break;
	case VK_DELETE:
		break;
	default:
		return false;
	}
	return false;
}

bool CRichEdit::OnChar(char c) {
	if (_IsReadyOnly)
		return false;

	// 有三种情况：一、英文字符，二、汉字，三、控制字符
	switch (c) {
	case '\r': // 回车
		_cArticle.AddControl(c);
		break;
	case '\b': // 退格
		break;
	case '\t':
		break;
	case 3: // copy
		break;
	case 22: // paste
		break;
	case 24: // cut
		break;
	case 27: // ESC
		break;
	default:
		_cArticle.AddChar(c);
	}

	return false;
}

void CRichEdit::Render() {
	_cArticle.Render();

	_pEditKey->Render();
}
