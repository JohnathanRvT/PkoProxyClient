

#if !defined __NETWORK_H__
#define __NETWORK_H__

#include <InfoNet.h>
#include "MyListView.h"
#include <vector>

#define TYPE_STORE 0
#define TYPE_AFFICHE 1
#define TYPE_CLASS 2
#define TYPE_DEL_STORE 3

#define LGN_INIT 0
#define LGN_WAITING 1
#define LGN_SUCCESS 2
#define LGN_FAILT 3

typedef std::vector<pStoreStruct> StoreList;
typedef std::vector<pClassInfo> ClassList;
typedef std::vector<pAfficheInfo> AfficheList;

class NetWork : public InfoNetBase {
public:
	NetWork();
	virtual ~NetWork();
	virtual void OnConnect(bool result);
	virtual void OnNetMessage(pNetMessage msg);
	virtual void OnResend(pNetMessage msg);
	virtual void OnDisconnect();

	virtual void Clear(int type);

public:
	MyListView m_affList;
	MyListView m_storeList;
	MyListView m_delstoreList;
	MyListView m_clsList;
	StoreList m_slist;
	StoreList m_dslist;
	ClassList m_clist;
	AfficheList m_alist;
	long m_section;
	long m_loginState;
};

class TmpNetWork : public InfoNetBase {
public:
	TmpNetWork();
	virtual ~TmpNetWork();

	virtual void OnConnect(bool result);
	virtual void OnNetMessage(pNetMessage msg);
	virtual void OnResend(pNetMessage msg);
	virtual void OnDisconnect();

public:
	long m_section;
};

#endif
