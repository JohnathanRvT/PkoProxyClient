
#include "NetWork.h"

NetWork::NetWork()
	: m_loginState(LGN_INIT) {
}

NetWork::~NetWork() {
}

void NetWork::OnConnect(bool result) {
	if (result) {
		m_loginState = LGN_WAITING;
		PostInfoSend(INFO_LOGIN, 0, 0, m_section, (byte*)g_key, 33);
	}
}

void NetWork::OnNetMessage(pNetMessage msg) {
	switch (msg->msgHead.msgID) {
	case INFO_REQUEST_STORE: {
		Clear(TYPE_CLASS);
		Clear(TYPE_STORE);

		m_clsList.DeleteAllItem();
		m_storeList.DeleteAllItem();

		char buff[32] = {0};
		int i = 0, j = 0;
		pClassInfo pCli = 0;
		pStoreStruct pSS = 0;
		tm* ttm;

		byte* ptr = (byte*)msg->msgBody;
		int clsSize = HIWORD(msg->msgHead.msgExtend);
		int stoSize = LOWORD(msg->msgHead.msgExtend);

		for (i = 0; i < clsSize; i++) {
			pCli = new ClassInfo;
			memcpy(pCli, ptr, sizeof(ClassInfo));
			ptr += sizeof(ClassInfo);
			m_clist.push_back(pCli);

			m_clsList.InsertItem(i);
			m_clsList.SetItemText(i, 0, itoa(pCli->clsID, buff, 10));
			m_clsList.SetItemText(i, 1, pCli->clsName);
			m_clsList.SetItemText(i, 2, itoa(pCli->parentID, buff, 10));
			pCli = 0;
		}

		for (i = 0; i < stoSize; i++) {
			pSS = new StoreStruct;
			memcpy(&pSS->store_head, ptr, sizeof(StoreInfo));
			ptr += sizeof(StoreInfo);
			pSS->item_body = new ItemInfo[pSS->store_head.itemNum];
			memcpy(pSS->item_body, ptr, (sizeof(ItemInfo) * pSS->store_head.itemNum));
			ptr += (sizeof(ItemInfo) * pSS->store_head.itemNum);
			m_slist.push_back(pSS);

			m_storeList.InsertItem(i);
			m_storeList.SetItemText(i, 0, itoa(pSS->store_head.comID, buff, 10));
			m_storeList.SetItemText(i, 1, pSS->store_head.comName);
			m_storeList.SetItemText(i, 2, itoa(pSS->store_head.comClass, buff, 10));
			ttm = localtime(&pSS->store_head.comTime);
			sprintf(buff, "%04i-%02i-%02i %02i:%02i:%02i", ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday, ttm->tm_hour, ttm->tm_min, ttm->tm_sec);
			m_storeList.SetItemText(i, 3, buff);
			m_storeList.SetItemText(i, 4, ltoa(pSS->store_head.comPrice, buff, 10));
			m_storeList.SetItemText(i, 5, pSS->store_head.comRemark);
			m_storeList.SetItemText(i, 6, ((pSS->store_head.isHot == 1) ? "Y" : "N"));
			m_storeList.SetItemText(i, 7, itoa(pSS->store_head.comNumber, buff, 10));
			int hour = 0;
			if (pSS->store_head.comExpire > 0) {
				hour = (pSS->store_head.comExpire - time(0)) / 3600;
			} else {
				hour = -1;
			}
			m_storeList.SetItemText(i, 8, itoa(hour, buff, 10));
			if (pSS->store_head.beginTime > time(0)) {
				hour = (pSS->store_head.beginTime - time(0)) / 3600;
			} else {
				hour = -1;
			}
			m_storeList.SetItemText(i, 9, itoa(hour, buff, 10));

			// Add by lark.li 20080319 begin
			ttm = localtime(&pSS->store_head.beginTime);
			sprintf(buff, "%04i-%02i-%02i %02i:%02i:%02i", ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday, ttm->tm_hour, ttm->tm_min, ttm->tm_sec);
			m_storeList.SetItemText(i, 9, buff);
			// End
			pSS = 0;
		}
		//PostInfoSend(INFO_MAN_ALL_ITEM, 0, 0, m_section, 0, 0);
	} break;
	case INFO_REQUEST_AFFICHE: {
		Clear(TYPE_AFFICHE);
		m_affList.DeleteAllItem();

		pAfficheInfo pAi = 0;
		char buff[32] = {0};
		tm* ttm;
		byte* ptr = (byte*)msg->msgBody;

		for (int i = 0; i < msg->msgHead.msgExtend; i++) {
			pAi = new AfficheInfo;
			memcpy(pAi, ptr, sizeof(AfficheInfo));
			ptr += sizeof(AfficheInfo);
			m_alist.push_back(pAi);

			m_affList.InsertItem(i);
			m_affList.SetItemText(i, 0, ltoa(pAi->affiID, buff, 10));
			m_affList.SetItemText(i, 1, pAi->affiTitle);
			ttm = localtime(&pAi->affiTime);
			sprintf(buff, "%04i-%02i-%02i %02i:%02i:%02i", ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday, ttm->tm_hour, ttm->tm_min, ttm->tm_sec);
			m_affList.SetItemText(i, 2, buff);
			pAi = 0;
		}
	} break;
	case INFO_MAN_ALL_ITEM: {
		Clear(TYPE_DEL_STORE);

		m_delstoreList.DeleteAllItem();

		char buff[32] = {0};
		int i = 0, j = 0;
		pStoreStruct pSS = 0;
		tm* ttm;

		byte* ptr = (byte*)msg->msgBody;
		int stoSize = msg->msgHead.msgExtend;

		for (i = 0; i < stoSize; i++) {
			pSS = new StoreStruct;
			memcpy(&pSS->store_head, ptr, sizeof(StoreInfo));
			ptr += sizeof(StoreInfo);
			pSS->item_body = new ItemInfo[pSS->store_head.itemNum];
			memcpy(pSS->item_body, ptr, (sizeof(ItemInfo) * pSS->store_head.itemNum));
			ptr += (sizeof(ItemInfo) * pSS->store_head.itemNum);
			m_dslist.push_back(pSS);

			m_delstoreList.InsertItem(i);
			m_delstoreList.SetItemText(i, 0, itoa(pSS->store_head.comID, buff, 10));
			m_delstoreList.SetItemText(i, 1, pSS->store_head.comName);
			m_delstoreList.SetItemText(i, 2, itoa(pSS->store_head.comClass, buff, 10));
			ttm = localtime(&pSS->store_head.comTime);
			sprintf(buff, "%04i-%02i-%02i %02i:%02i:%02i", ttm->tm_year + 1900, ttm->tm_mon + 1, ttm->tm_mday, ttm->tm_hour, ttm->tm_min, ttm->tm_sec);
			m_delstoreList.SetItemText(i, 3, buff);
			m_delstoreList.SetItemText(i, 4, ltoa(pSS->store_head.comPrice, buff, 10));
			m_delstoreList.SetItemText(i, 5, pSS->store_head.comRemark);
			m_delstoreList.SetItemText(i, 6, ((pSS->store_head.isHot == 1) ? "Y" : "N"));
			m_delstoreList.SetItemText(i, 7, itoa(pSS->store_head.comNumber, buff, 10));
			int hour = 0;
			if (pSS->store_head.comExpire > 0) {
				hour = (pSS->store_head.comExpire - time(0)) / 3600;
			} else {
				hour = -1;
			}
			m_delstoreList.SetItemText(i, 8, itoa(hour, buff, 10));
			if (pSS->store_head.beginTime > time(0)) {
				hour = (pSS->store_head.beginTime - time(0)) / 3600;
			} else {
				hour = -1;
			}
			m_delstoreList.SetItemText(i, 9, itoa(hour, buff, 10));
			pSS = 0;
		}
	} break;
	case INFO_EXCEPTION_SERVICE: {
	} break;
	case INFO_MAN_ITEM:
	case INFO_MAN_CLASS:
	case INFO_MAN_AFFICHE:
	case INFO_MAN_RATE:
	case INFO_MAN_VIP: {
		switch (msg->msgHead.subID) {
		case INFO_MAN_SUCCESS: {
			MessageBox(0, "Success!", "Notify", MB_OK);
		} break;
		case INFO_MAN_FAILED:
		default: {
			MessageBox(0, "Failed!", "Notify", MB_OK);
		} break;
		}
	} break;
	case INFO_LOGIN: {
		switch (msg->msgHead.subID) {
		case INFO_SUCCESS: {
			m_loginState = LGN_SUCCESS;
			PostInfoSend(INFO_REQUEST_STORE, 0, 0, m_section, 0, 0);
			PostInfoSend(INFO_REQUEST_AFFICHE, 0, 0, m_section, 0, 0);
		} break;
		case INFO_FAILED: {
			m_loginState = LGN_FAILT;
		} break;
		}
	} break;
	}
	FreeNetMessage(msg);
}

void NetWork::OnResend(pNetMessage msg) {
}

void NetWork::OnDisconnect() {
}

void NetWork::Clear(int type) {
	switch (type) {
	case TYPE_STORE: {
		StoreList::iterator it;
		for (it = m_slist.begin(); it != m_slist.end(); it++) {
			SAFE_DELETE_ARRAY((*it)->item_body);
			SAFE_DELETE((*it));
		}
		m_slist.clear();
	} break;
	case TYPE_AFFICHE: {
		AfficheList::iterator it;
		for (it = m_alist.begin(); it != m_alist.end(); it++) {
			SAFE_DELETE((*it));
		}
		m_alist.clear();
	} break;
	case TYPE_CLASS: {
		ClassList::iterator it;
		for (it = m_clist.begin(); it != m_clist.end(); it++) {
			SAFE_DELETE((*it));
		}
		m_clist.clear();
	} break;
	case TYPE_DEL_STORE: {
		StoreList::iterator it;
		for (it = m_dslist.begin(); it != m_dslist.end(); it++) {
			SAFE_DELETE_ARRAY((*it)->item_body);
			SAFE_DELETE((*it));
		}
		m_slist.clear();
	} break;
	}
}

TmpNetWork::TmpNetWork() {
}

TmpNetWork::~TmpNetWork() {
}

void TmpNetWork::OnConnect(bool result) {
	if (result) {
		PostInfoSend(INFO_LOGIN, 0, 0, m_section, (byte*)g_key, 33);
	}
}

void TmpNetWork::OnNetMessage(pNetMessage msg) {
	switch (msg->msgHead.msgID) {
	case INFO_REQUEST_STORE: {
	} break;
	case INFO_REQUEST_AFFICHE: {
	} break;
	case INFO_EXCEPTION_SERVICE: {
	} break;
	case INFO_MAN_ITEM:
	case INFO_MAN_CLASS:
	case INFO_MAN_AFFICHE: {
	} break;
	case INFO_MAN_RATE: {
	} break;
	}
	FreeNetMessage(msg);
}

void TmpNetWork::OnResend(pNetMessage msg) {
}

void TmpNetWork::OnDisconnect() {
}
