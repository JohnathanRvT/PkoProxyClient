
#include "MyComboBox.h"


MyComboBox::MyComboBox()
{
}


MyComboBox::MyComboBox(HWND parent, int ctlid)
{
    GetComboBoxHandle(parent, ctlid);
}


MyComboBox::~MyComboBox()
{
}


void MyComboBox::GetComboBoxHandle(HWND parent, int ctlid)
{
    m_hwnd = GetDlgItem(parent, ctlid);
}


const char* MyComboBox::GetText()
{
    static char buffer[260];
    GetWindowText(m_hwnd, buffer, 260);
    return buffer;
}


void MyComboBox::AddItem(const char* item)
{
    SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)item);
}


void MyComboBox::SetCurSel(int index)
{
    SendMessage(m_hwnd, CB_SETCURSEL, index, 0);
}


void MyComboBox::SetText(const char* txt)
{
    SetWindowText(m_hwnd, txt);
}
