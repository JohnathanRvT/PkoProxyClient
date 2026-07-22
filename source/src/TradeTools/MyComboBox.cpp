
#include "MyComboBox.h"

MyComboBox::MyComboBox() {
}

MyComboBox::MyComboBox(HWND parent, int ctlid) {
	GetHandle(parent, ctlid);
}

MyComboBox::~MyComboBox() {
}

const char* MyComboBox::GetText() {
	static char buffer[260];
	GetWindowText(m_hWnd, buffer, 260);
	return buffer;
}

void MyComboBox::AddItem(const char* item) {
	SendMessage(m_hWnd, CB_ADDSTRING, 0, (LPARAM)item);
}

void MyComboBox::SetCurSel(int index) {
	SendMessage(m_hWnd, CB_SETCURSEL, index, 0);
}

void MyComboBox::SetText(const char* txt) {
	SetWindowText(m_hWnd, txt);
}
