

#include "MyTab.h"

MyTab::MyTab() {
}

MyTab::MyTab(HWND parent, int ctlid) {
	InitCommon();
	GetHandle(parent, ctlid);
}

MyTab::~MyTab() {
}

void MyTab::InsertItem(int idx, const char* txt, int img /* = -1*/, int mask /* = TCIF_TEXT | TCIF_IMAGE*/) {
	TCITEM tie;
	tie.mask = mask;
	tie.iImage = img;
	tie.pszText = (char*)txt;
	TabCtrl_InsertItem(m_hWnd, idx, &tie);
}

int MyTab::GetCurrentSelect() {
	return TabCtrl_GetCurSel(m_hWnd);
}

int MyTab::GetCurrentFocus() {
	return TabCtrl_GetCurFocus(m_hWnd);
}

void MyTab::SetCurrentSelect(int idx) {
	TabCtrl_SetCurSel(m_hWnd, idx);
}
