#include <winsock2.h>
#include "NetWork.h"
#include "resource.h"
#include "MyTab.h"
#include "INIStream.h"
#include <direct.h>

#define TAB_SYSTEM 0
#define TAB_CLASS 1
#define TAB_STORE 2
#define TAB_AFFICHE 3

typedef std::vector<pItemInfo> ItemList;

HINSTANCE g_hInstance;
HWND g_hMainWnd;
HWND g_hStoreWnd;
HWND g_hAfficheWnd;
HWND g_hSystemWnd;
HWND g_hClassWnd;
NetWork g_NetWork;
WSADATA g_wsaData;
char g_ServerIP[64];
int g_ServerPort;
char g_key[33] = {0};
TmpNetWork tmpServer[12]; //  临时服务列表

void InitCom() {
	//CoInitialize(0);
}

void UninitCom() {
	//CoUninitialize();
}

void SetTmpServerUserID(long userID) {
	for (int i = 0; i < 12; i++) {
		tmpServer[i].m_section = userID;
	}
}
//  临时管理对象
void PostTmpServer(long msgID, long subID, long extend, byte* _buff, long size) {
	for (int i = 0; i < 12; i++) {
		if (!tmpServer[i].IsConnect()) {
			break;
		}
		tmpServer[i].PostInfoSend(msgID, subID, extend, tmpServer[i].m_section, _buff, size);
	}
}

void ShowDialog(int idx) {
	ShowWindow(g_hStoreWnd, SW_HIDE);
	ShowWindow(g_hAfficheWnd, SW_HIDE);
	ShowWindow(g_hSystemWnd, SW_HIDE);
	ShowWindow(g_hClassWnd, SW_HIDE);

	RECT rc;
	HWND hFrameWnd;

	hFrameWnd = GetDlgItem(g_hMainWnd, IDC_FRAME);
	GetWindowRect(hFrameWnd, &rc);
	ScreenToClient(g_hMainWnd, ((LPPOINT)&rc));
	ScreenToClient(g_hMainWnd, ((LPPOINT)&rc) + 1);

	switch (idx) {
	case TAB_SYSTEM: {
		SetWindowPos(g_hSystemWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
	} break;
	case TAB_CLASS: {
		SetWindowPos(g_hClassWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
	} break;
	case TAB_STORE: {
		SetWindowPos(g_hStoreWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
	} break;
	case TAB_AFFICHE: {
		SetWindowPos(g_hAfficheWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_SHOWWINDOW);
	} break;
	}
}

INT_PTR CALLBACK SystemWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG: {
		SetWindowText(GetDlgItem(hWnd, IDC_EDT_VIPID), "20");
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_BTN_RATE: {
			char buff[32] = {0};
			GetWindowText(GetDlgItem(hWnd, IDC_EDT_RATE), buff, 32);
			g_NetWork.PostInfoSend(INFO_MAN_RATE, 0, atol(buff), g_NetWork.m_section, NULL, 0);
			PostTmpServer(INFO_MAN_RATE, 0, atol(buff), NULL, 0);
		} break;
		case IDC_BTN_VIPMODIFY: {
			char vipID[32] = {0};
			char vipPrice[32] = {0};
			GetWindowText(GetDlgItem(hWnd, IDC_EDT_VIPID), vipID, 32);
			GetWindowText(GetDlgItem(hWnd, IDC_EDT_VIPPRICE), vipPrice, 32);
			g_NetWork.PostInfoSend(INFO_MAN_VIP, atol(vipID), atol(vipPrice), g_NetWork.m_section, NULL, 0);
			PostTmpServer(INFO_MAN_VIP, atol(vipID), atol(vipPrice), NULL, 0);
		} break;
		}
	} break;
	case WM_CLOSE: {
	} break;
	}
	return 0;
}

INT_PTR CALLBACK ClassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG: {
		g_NetWork.m_clsList.GetHandle(hWnd, IDC_CLASS_LIST);
		g_NetWork.m_clsList.SetExtended();
		g_NetWork.m_clsList.InsertColumn(1, "Class ID", 80);
		g_NetWork.m_clsList.InsertColumn(2, "Class Name", 140);
		g_NetWork.m_clsList.InsertColumn(3, "Parent ID", 180);
	}
		// Add by lark.li 20080319 begin
	case WM_NOTIFY: {
		if (wParam == IDC_CLASS_LIST) {
			switch (LPNMHDR(lParam)->code) {
			case NM_CLICK:
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;

				// 取得鼠标点击的项目，放到对话框上
				if (lpnmitem->iItem < 0) {
					::SetDlgItemText(hWnd, IDC_CLSID, "");
					::SetDlgItemText(hWnd, IDC_CLSNAME, "");
					::SetDlgItemText(hWnd, IDC_CLSPARENT, "");
				} else {

					HWND hwndList = GetDlgItem(hWnd, IDC_CLASS_LIST);
					char pszText[255];
					int cchTextMax = 255;

					ListView_GetItemText(hwndList, lpnmitem->iItem, 0, pszText, cchTextMax);
					::SetDlgItemText(hWnd, IDC_CLSID, pszText);

					ListView_GetItemText(hwndList, lpnmitem->iItem, 1, pszText, cchTextMax);
					::SetDlgItemText(hWnd, IDC_CLSNAME, pszText);

					ListView_GetItemText(hwndList, lpnmitem->iItem, 2, pszText, cchTextMax);
					::SetDlgItemText(hWnd, IDC_CLSPARENT, pszText);
				}

				break;
			}
		}
		break;
	}
		// End
	case WM_COMMAND: {
		char buff[32] = {0};
		switch (LOWORD(wParam)) {
		case IDC_ADD_CLASS: {
			ClassInfo ci = {0};
			GetWindowText(GetDlgItem(hWnd, IDC_CLSNAME), ci.clsName, 17);
			GetWindowText(GetDlgItem(hWnd, IDC_CLSPARENT), buff, 32);
			ci.parentID = atoi(buff);
			if (!strlen(ci.clsName)) {
				MessageBox(0, "Error class name.", "Notify", MB_OK);
				break;
			}
			g_NetWork.PostInfoSend(INFO_MAN_CLASS, INFO_MAN_ADD, 0, g_NetWork.m_section, (byte*)&ci, sizeof(ClassInfo));
			PostTmpServer(INFO_MAN_CLASS, INFO_MAN_ADD, 0, (byte*)&ci, sizeof(ClassInfo));
		} break;
		case IDC_DEL_CLASS: {
			GetWindowText(GetDlgItem(hWnd, IDC_CLSID), buff, 32);
			if (!strlen(buff)) {
				MessageBox(0, "No class id.", "Notify", MB_OK);
				break;
			}
			g_NetWork.PostInfoSend(INFO_MAN_CLASS, INFO_MAN_DEL, atol(buff), g_NetWork.m_section, 0, 0);
			PostTmpServer(INFO_MAN_CLASS, INFO_MAN_DEL, atol(buff), 0, 0);
		} break;
		}
	} break;
	case WM_CLOSE: {
	} break;
	}
	return 0;
}

INT_PTR CALLBACK StoreWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static ItemList s_itemlist;
	switch (uMsg) {
	case WM_INITDIALOG: {
		g_NetWork.m_storeList.GetHandle(hWnd, IDC_STORE_LIST);
		g_NetWork.m_storeList.SetExtended();
		g_NetWork.m_storeList.InsertColumn(1, "Store ID", 80);
		g_NetWork.m_storeList.InsertColumn(2, "Store Name", 140);
		g_NetWork.m_storeList.InsertColumn(3, "Store Class", 80);
		g_NetWork.m_storeList.InsertColumn(4, "Store Time", 140);
		g_NetWork.m_storeList.InsertColumn(5, "Store Price", 80);
		g_NetWork.m_storeList.InsertColumn(6, "Store Remark", 160);
		g_NetWork.m_storeList.InsertColumn(7, "Hot Store", 80);
		g_NetWork.m_storeList.InsertColumn(8, "Store Number", 100);
		g_NetWork.m_storeList.InsertColumn(9, "Store Expire", 100);
		g_NetWork.m_storeList.InsertColumn(10, "Begin Time", 100);

		g_NetWork.m_delstoreList.GetHandle(hWnd, IDC_DEL_STORE_LIST);
		g_NetWork.m_delstoreList.SetExtended();
		g_NetWork.m_delstoreList.InsertColumn(1, "Store ID", 80);
		g_NetWork.m_delstoreList.InsertColumn(2, "Store Name", 140);
		g_NetWork.m_delstoreList.InsertColumn(3, "Store Class", 80);
		g_NetWork.m_delstoreList.InsertColumn(4, "Store Time", 140);
		g_NetWork.m_delstoreList.InsertColumn(5, "Store Price", 80);
		g_NetWork.m_delstoreList.InsertColumn(6, "Store Remark", 160);
		g_NetWork.m_delstoreList.InsertColumn(7, "Hot Store", 80);
		g_NetWork.m_delstoreList.InsertColumn(8, "Store Number", 100);
		g_NetWork.m_delstoreList.InsertColumn(9, "Store Expire", 100);
		g_NetWork.m_delstoreList.InsertColumn(10, "Begin Time", 100);
	}
		// Add by lark.li 20080319 begin
	case WM_NOTIFY: {
		if (wParam == IDC_STORE_LIST) {
			switch (LPNMHDR(lParam)->code) {
			case NM_CLICK:
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;

				LVITEM item;

				// TODO
				// 取得鼠标点击的项目，放到对话框上
				//SendMessage(GetDlgItem(hWnd, IDC_STORE_LIST), LVM_GETITEM, 0, (LPARAM) (LPLVITEM)&item);
				break;
			}
		}
	}
		// End
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_ADD_STORE: {
			char buff[32] = {0};
			StoreInfo ss = {0};

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_NAME), buff, 32);
			strncpy(ss.comName, buff, 20);

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_CLASS), buff, 32);
			ss.comClass = atol(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_PRICE), buff, 32);
			ss.comPrice = atol(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_NUMBER), buff, 32);
			ss.comNumber = atol(buff);
			if (0 == ss.comNumber) {
				ss.comNumber = -1;
			}

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_DAYS), buff, 32);
			ss.comExpire = atol(buff);
			if (0 == ss.comExpire) {
				ss.comExpire = -1;
			}

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_BEGIN), buff, 32);
			int len = strlen(buff);
			if (0 != len && 14 != len) //  日期格式20050101120100
			{
				MessageBox(0, "Begin Time Format Error.", "Notify", MB_OK);
				break;
			}
			if (len) {
				char year[5] = {0};
				char month[3] = {0};
				char day[3] = {0};
				char hour[3] = {0};
				char minute[3] = {0};
				char second[3] = {0};
				year[0] = buff[0];
				year[1] = buff[1];
				year[2] = buff[2];
				year[3] = buff[3];
				month[0] = buff[4];
				month[1] = buff[5];
				day[0] = buff[6];
				day[1] = buff[7];
				hour[0] = buff[8];
				hour[1] = buff[9];
				minute[0] = buff[10];
				minute[1] = buff[11];
				second[0] = buff[12];
				second[1] = buff[13];

				tm t;
				t.tm_year = atol(year) - 1900;
				if (t.tm_year < 0) {
					MessageBox(0, "Error Year.", "Notify", MB_OK);
					break;
				}
				t.tm_mon = atol(month) - 1;
				if (t.tm_mon < 0 || t.tm_mon > 11) {
					MessageBox(0, "Error Month.", "Notify", MB_OK);
					break;
				}
				t.tm_mday = atol(day);
				if (t.tm_mday < 0 || t.tm_mday > 31) {
					MessageBox(0, "Error Day.", "Notify", MB_OK);
					break;
				}
				t.tm_hour = atol(hour);
				if (t.tm_hour < 0 || t.tm_hour > 23) {
					MessageBox(0, "Error Hour.", "Notify", MB_OK);
					break;
				}
				t.tm_min = atol(minute);
				if (t.tm_min < 0 || t.tm_min > 59) {
					MessageBox(0, "Error Minute.", "Notify", MB_OK);
					break;
				}
				t.tm_sec = atol(second);
				if (t.tm_sec < 0 || t.tm_sec > 59) {
					MessageBox(0, "Error Second.", "Notify", MB_OK);
					break;
				}
				ss.beginTime = mktime(&t);
			}

			int ret = 0;

			if (ss.beginTime > time(0)) {
				ret = 1;
			}

			ss.isHot = (_byte)SendMessage(GetDlgItem(hWnd, IDC_CHK_HOT), BM_GETCHECK, 0, 0);

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_REMARK), ss.comRemark, 129);

			ss.itemNum = (byte)s_itemlist.size();

			if (!ss.itemNum) {
				MessageBox(0, "No items.", "Notify", MB_OK);
				break;
			}

			int size = sizeof(StoreInfo) + (ss.itemNum * sizeof(ItemInfo));
			byte* buf = new byte[size];
			byte* ptr = buf;
			memcpy(ptr, &ss, sizeof(StoreInfo));
			ptr += sizeof(StoreInfo);

			ItemList::iterator it;
			for (it = s_itemlist.begin(); it != s_itemlist.end(); it++) {
				memcpy(ptr, (*it), sizeof(ItemInfo));
				ptr += sizeof(ItemInfo);
			}

			g_NetWork.PostInfoSend(INFO_MAN_ITEM, INFO_MAN_ADD, 0, g_NetWork.m_section, buf, size);
			PostTmpServer(INFO_MAN_ITEM, INFO_MAN_ADD, 0, buf, size);
			SAFE_DELETE_ARRAY(buf);
		} break;
		case IDC_ADD_ITEM: {
			if (s_itemlist.size() > 5) {
				MessageBox(0, "Too much items.", "Notify", MB_OK);
				break;
			}
			char buff[32] = {0};
			pItemInfo pItem = new ItemInfo;

			GetWindowText(GetDlgItem(hWnd, IDC_ITEM_ID), buff, 32);
			pItem->itemID = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ITEM_SLOT), buff, 32);
			pItem->itemFlute = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ITEM_NUM), buff, 32);
			pItem->itemNum = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRID1), buff, 32);
			pItem->itemAttrID[0] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL1), buff, 32);
			pItem->itemAttrVal[0] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRID2), buff, 32);
			pItem->itemAttrID[1] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL2), buff, 32);
			pItem->itemAttrVal[1] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRID3), buff, 32);
			pItem->itemAttrID[2] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL3), buff, 32);
			pItem->itemAttrVal[2] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRID4), buff, 32);
			pItem->itemAttrID[3] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL4), buff, 32);
			pItem->itemAttrVal[3] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRID5), buff, 32);
			pItem->itemAttrID[4] = atoi(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL5), buff, 32);
			pItem->itemAttrVal[4] = atoi(buff);

			if (!pItem->itemID) {
				MessageBox(0, "Item id can't be zero.", "Notify", MB_OK);
			} else if (!pItem->itemNum) {
				MessageBox(0, "Item number can't be zero.", "Notify", MB_OK);
			} else {
				s_itemlist.push_back(pItem);
				GetWindowText(GetDlgItem(hWnd, IDC_ITEM_ID), buff, 32);
				SendMessage(GetDlgItem(hWnd, IDC_ITEM_LIST), LB_ADDSTRING, 0, (LPARAM)buff);
			}
		} break;
		case IDC_MDF_STORE: {
			char buff[32] = {0};
			StoreInfo ss = {0};
			long mdfID = 0;

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_NAME), buff, 32);
			mdfID = atol(buff);

			if (!mdfID) {
				MessageBox(0, "ID can't be zero.", "Notify", MB_OK);
				break;
			}

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_CLASS), buff, 32);
			ss.comClass = atol(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_PRICE), buff, 32);
			ss.comPrice = atol(buff);

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_NUMBER), buff, 32);
			ss.comNumber = atol(buff);
			if (0 == ss.comNumber) {
				ss.comNumber = -1;
			}

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_DAYS), buff, 32);
			ss.comExpire = atol(buff);
			if (0 == ss.comExpire) {
				ss.comExpire = -1;
			}

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_BEGIN), buff, 32);
			int len = strlen(buff);
			if (0 != len && 14 != len) //  日期格式20050101120100
			{
				MessageBox(0, "Begin Time Format Error.", "Notify", MB_OK);
				break;
			}
			if (len) {
				char year[5] = {0};
				char month[3] = {0};
				char day[3] = {0};
				char hour[3] = {0};
				char minute[3] = {0};
				char second[3] = {0};
				year[0] = buff[0];
				year[1] = buff[1];
				year[2] = buff[2];
				year[3] = buff[3];
				month[0] = buff[4];
				month[1] = buff[5];
				day[0] = buff[6];
				day[1] = buff[7];
				hour[0] = buff[8];
				hour[1] = buff[9];
				minute[0] = buff[10];
				minute[1] = buff[11];
				second[0] = buff[12];
				second[1] = buff[13];

				tm t;
				t.tm_year = atol(year) - 1900;
				if (t.tm_year < 0) {
					MessageBox(0, "Error Year.", "Notify", MB_OK);
					break;
				}
				t.tm_mon = atol(month) - 1;
				if (t.tm_mon < 0 || t.tm_mon > 11) {
					MessageBox(0, "Error Month.", "Notify", MB_OK);
					break;
				}
				t.tm_mday = atol(day);
				if (t.tm_mday < 0 || t.tm_mday > 31) {
					MessageBox(0, "Error Day.", "Notify", MB_OK);
					break;
				}
				t.tm_hour = atol(hour);
				if (t.tm_hour < 0 || t.tm_hour > 23) {
					MessageBox(0, "Error Hour.", "Notify", MB_OK);
					break;
				}
				t.tm_min = atol(minute);
				if (t.tm_min < 0 || t.tm_min > 59) {
					MessageBox(0, "Error Minute.", "Notify", MB_OK);
					break;
				}
				t.tm_sec = atol(second);
				if (t.tm_sec < 0 || t.tm_sec > 59) {
					MessageBox(0, "Error Second.", "Notify", MB_OK);
					break;
				}
				ss.beginTime = mktime(&t);
			}

			ss.isHot = (_byte)SendMessage(GetDlgItem(hWnd, IDC_CHK_HOT), BM_GETCHECK, 0, 0);

			GetWindowText(GetDlgItem(hWnd, IDC_STORE_REMARK), ss.comRemark, 129);

			g_NetWork.PostInfoSend(INFO_MAN_ITEM, INFO_MAN_MODIFY, mdfID, g_NetWork.m_section, (byte*)&ss, sizeof(StoreInfo));
			PostTmpServer(INFO_MAN_ITEM, INFO_MAN_MODIFY, mdfID, (byte*)&ss, sizeof(StoreInfo));
		} break;
		case IDC_DEL_ITEM: {
			int idx = 0;
			char buff[32] = {0};

			idx = (int)SendMessage(GetDlgItem(hWnd, IDC_ITEM_LIST), LB_GETCURSEL, 0, 0);
			SendMessage(GetDlgItem(hWnd, IDC_ITEM_LIST), LB_GETTEXT, idx, (LPARAM)buff);
			SendMessage(GetDlgItem(hWnd, IDC_ITEM_LIST), LB_DELETESTRING, idx, 0);

			ItemList::iterator it;
			idx = atoi(buff);
			for (it = s_itemlist.begin(); it != s_itemlist.end(); it++) {
				if ((*it)->itemID == idx) {
					SAFE_DELETE((*it));
					s_itemlist.erase(it);
					break;
				}
			}
		} break;
		case IDC_HOT_STORE: {
			char buff[32] = {0};
			GetWindowText(GetDlgItem(hWnd, IDC_STORE_DELID), buff, 32);
			if (!strlen(buff)) {
				MessageBox(0, "No store id.", "Notify", MB_OK);
				break;
			}
			g_NetWork.PostInfoSend(INFO_MAN_ITEM, 3, atol(buff), g_NetWork.m_section, 0, 0);
			PostTmpServer(INFO_MAN_ITEM, 3, atol(buff), 0, 0);
		} break;
		case IDC_DEL_STORE: {
			char buff[32] = {0};
			GetWindowText(GetDlgItem(hWnd, IDC_STORE_DELID), buff, 32);
			if (!strlen(buff)) {
				MessageBox(0, "No store id.", "Notify", MB_OK);
				break;
			}
			g_NetWork.PostInfoSend(INFO_MAN_ITEM, INFO_MAN_DEL, atol(buff), g_NetWork.m_section, 0, 0);
			PostTmpServer(INFO_MAN_ITEM, INFO_MAN_DEL, atol(buff), 0, 0);
		} break;
		case IDC_CLR_STORE: {
			SetWindowText(GetDlgItem(hWnd, IDC_ITEM_ID), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ITEM_SLOT), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ITEM_NUM), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID1), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL1), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID2), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL2), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID3), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL3), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID4), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL4), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID5), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL5), "");

			SetWindowText(GetDlgItem(hWnd, IDC_STORE_NAME), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_CLASS), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_PRICE), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_REMARK), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_NUMBER), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_DAYS), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_BEGIN), "");

			SetWindowText(GetDlgItem(hWnd, IDC_ITEM_ID), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ITEM_SLOT), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ITEM_NUM), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID1), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL1), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID2), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL2), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID3), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL3), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID4), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL4), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRID5), "");
			SetWindowText(GetDlgItem(hWnd, IDC_ATTRVAL5), "");

			SetWindowText(GetDlgItem(hWnd, IDC_STORE_NAME), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_CLASS), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_PRICE), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_REMARK), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_NUMBER), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_DAYS), "");
			SetWindowText(GetDlgItem(hWnd, IDC_STORE_BEGIN), "");

			s_itemlist.clear();
			SendMessage(GetDlgItem(hWnd, IDC_ITEM_LIST), LB_RESETCONTENT, 0, 0);
		} break;
		}
	} break;
	case WM_CLOSE: {
	} break;
	}
	return 0;
}

INT_PTR CALLBACK AfficheWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG: {
		g_NetWork.m_affList.GetHandle(hWnd, IDC_AFFICHE_LIST);
		g_NetWork.m_affList.SetExtended();
		g_NetWork.m_affList.InsertColumn(1, "Affiche ID", 80);
		g_NetWork.m_affList.InsertColumn(2, "Affiche Content", 180);
		g_NetWork.m_affList.InsertColumn(3, "Affiche Time", 140);
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_ADD_AFFICHE: {
		} break;
		case IDC_ADD_LINK: {
		} break;
		case IDC_DEL_LINK: {
		} break;
		case IDC_DEL_AFFICHE: {
		} break;
		}
	} break;
	case WM_CLOSE: {
	} break;
	}
	return 0;
}

INT_PTR CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static MyTab s_tab;

	switch (uMsg) {
	case WM_INITDIALOG: {
		g_hStoreWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_STORE_DLG), hWnd, StoreWndProc);
		g_hAfficheWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_AFFICHE_DLG), hWnd, AfficheWndProc);
		g_hSystemWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_SYSTEM_DLG), hWnd, SystemWndProc);
		g_hClassWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_CLASS_DLG), hWnd, ClassWndProc);

		s_tab.GetHandle(hWnd, IDC_INFO_TAB);
		s_tab.InsertItem(TAB_SYSTEM, "System");
		s_tab.InsertItem(TAB_CLASS, "Class");
		s_tab.InsertItem(TAB_STORE, "Store");
		s_tab.InsertItem(TAB_AFFICHE, "Affiche");

		s_tab.SetCurrentSelect(0);
	} break;
	case WM_NOTIFY: {
		switch (LOWORD(wParam)) {
		case IDC_INFO_TAB: {
			NMHDR* pnmh = (LPNMHDR)lParam;
			switch (pnmh->code) {
			case TCN_SELCHANGE: {
				ShowDialog(s_tab.GetCurrentSelect());
			} break;
			}
		} break;
		}
	} break;
	case WM_CLOSE: {
		PostQuitMessage(0);
	} break;
	}
	return 0;
}

INT_PTR CALLBACK LoginWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static clock_t login_t = 0;
	switch (uMsg) {
	case WM_INITDIALOG: {
	} break;
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_BTN_LOGIN: {
			HWND hPWD = GetDlgItem(hWnd, IDC_EDIT_PASSWD);
			HWND hUID = GetDlgItem(hWnd, IDC_EDIT_USERID);
			HWND hStat = GetDlgItem(hWnd, IDC_LGN_STATE);
			HWND hLgn = GetDlgItem(hWnd, IDC_BTN_LOGIN);

			if ((GetWindowTextLength(hUID) <= 0) || (GetWindowTextLength(hPWD) <= 0)) {
				SetWindowText(hStat, "Userid or Password length less than 1...");
				break;
			}

			GetWindowText(hPWD, g_key, sizeof(g_key));

			char userid[32] = {0};
			GetWindowText(hUID, userid, sizeof(userid));
			g_NetWork.m_section = atol(userid);
			SetTmpServerUserID(g_NetWork.m_section);

			g_NetWork.RunInfoService(g_ServerIP, (short)g_ServerPort);

			EnableWindow(hLgn, FALSE);
			EnableWindow(hUID, FALSE);
			EnableWindow(hPWD, FALSE);

			SetWindowText(hStat, "Waiting for response...");

			login_t = clock();
			SetTimer(hWnd, 1000, 50, 0);
		} break;
		}
	} break;
	case WM_TIMER: {
		switch (LOWORD(wParam)) {
		case 1000: {
			switch (g_NetWork.m_loginState) {
			case LGN_SUCCESS: {
				KillTimer(hWnd, 1000);
				EndDialog(hWnd, 1);
			} break;

			case LGN_FAILT: {
				KillTimer(hWnd, 1000);
				EndDialog(hWnd, -1);
			} break;

			case LGN_INIT:
			case LGN_WAITING:
			default: {
				//  do nothing
			} break;
			}

			if ((clock() - login_t) > 30000) {
				EnableWindow(GetDlgItem(hWnd, IDC_BTN_LOGIN), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_EDIT_USERID), TRUE);
				EnableWindow(GetDlgItem(hWnd, IDC_EDIT_PASSWD), TRUE);

				g_NetWork.StopInfoService();
				SetWindowText(GetDlgItem(hWnd, IDC_LGN_STATE), "Time out...");
				KillTimer(hWnd, 1000);
			}
		} break;
		}
	} break;
	case WM_CLOSE: {
		EndDialog(hWnd, 0);
	} break;
	}
	return 0;
}

#define CONFIG_FILE_NAME "TradeTools.cfg"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {
	g_hInstance = hInstance;
	try {
		char cfgPath[MAX_PATH] = {0}; //  配置文件路径
		char workPath[MAX_PATH] = {0};
		getcwd(workPath, 260); //  获取工作目录
		sprintf(cfgPath, "%s\\%s", workPath, CONFIG_FILE_NAME);

		mkdir("LOG");

		char* ptr = 0;
		INIStream ini(cfgPath);
		ini.ReadString(workPath, MAX_PATH, "InfoServer", "ServerList");
		//ini.ReadString(g_ServerIP, 64, "InfoServer", "ServerList");
		g_ServerPort = ini.ReadInt("InfoServer", "ServerPort");

		//  临时对象
		if (!strlen(workPath) || !g_ServerPort) {
			MessageBox(0, "Read config file failt.", "Notify", MB_OK);
			return 1;
		}

		int count = -1;
		ptr = strtok(workPath, ",");
		while (ptr) {
			if (count < 0) {
				strcpy(g_ServerIP, ptr);
			} else {
				tmpServer[count].RunInfoService(ptr, g_ServerPort);
			}
			++count;
			ptr = strtok(0, ",");
		}
		//  end

		WSAStartup(0x0202, &g_wsaData);

		g_hMainWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_MAIN_WINDOW), 0, MainWndProc);
		int ret = DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DLG_LOGIN), 0, LoginWndProc);

		if (ret > 0) {
			ShowDialog(0);

			if (g_hMainWnd) {
				ShowWindow(g_hMainWnd, SW_SHOW);

				MSG msg;
				while (GetMessage(&msg, NULL, 0, 0)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			} else {
				MessageBox(0, "CreateWindow Failed.", "Error", MB_OK);
			}
			g_NetWork.StopInfoService();
		} else if (ret < 0) {
			MessageBox(0, "Login Failed. \r\nIf you are administrator of TradeServer, please try again.", "Error", MB_OK);
			g_NetWork.StopInfoService();
		}

		WSACleanup();
	} catch (...) {
		MessageBox(0, "Unknown Exception", "Error", MB_OK);
	}
	return 0;
}
