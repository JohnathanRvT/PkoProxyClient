
#include "MyListView.h"

#pragma comment(lib, "comctl32.lib")

MyListView::MyListView() {
	InitCommon();
}

MyListView::MyListView(HWND parent, int ctlid) {
	InitCommon();
	GetHandle(parent, ctlid);
}

MyListView::~MyListView() {
}

void MyListView::InsertColumn(int index, const char* txt, int size, int fmt /* = LVCFMT_RIGHT*/) {
	LVCOLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = m_ColIndex;
	lvc.cx = size;
	lvc.fmt = fmt;

	lvc.pszText = (char*)txt;
	ListView_InsertColumn(m_hWnd, index, &lvc);

	++m_ColIndex;
}

void MyListView::DeleteColum(int index) {
	ListView_DeleteColumn(m_hWnd, index);
	--m_ColIndex;
}

void MyListView::DeleteAllColum() {
	for (int i = m_ColIndex; i >= 0; i--) {
		DeleteColum(i);
	}
}

int MyListView::InsertItem(int index) {
	LVITEM li;

	li.mask = LVIF_TEXT;
	li.state = LVIS_ACTIVATING;
	li.stateMask = 0;
	li.pszText = new char[128];
	strncpy_s(li.pszText, 128, "", _TRUNCATE);
	li.iItem = index;
	li.iSubItem = 0;

	return ListView_InsertItem(m_hWnd, &li);
}

void MyListView::SetItemText(int index, int col, const char* txt) {
	ListView_SetItemText(m_hWnd, index, col, (char*)txt);
}

void MyListView::DeleteAllItem() {
	ListView_DeleteAllItems(m_hWnd);
}

void MyListView::SetExtended(ulong ext) {
	ListView_SetExtendedListViewStyle(m_hWnd, ext);
}
