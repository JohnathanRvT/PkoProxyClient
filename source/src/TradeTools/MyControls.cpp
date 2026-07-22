
#include "MyControls.h"

MyControls::MyControls() {
}

MyControls::~MyControls() {
}

void MyControls::InitCommon() {
	InitCommonControls();
}

void MyControls::GetHandle(HWND parent, int resID) {
	m_hWnd = GetDlgItem(parent, resID);
}
