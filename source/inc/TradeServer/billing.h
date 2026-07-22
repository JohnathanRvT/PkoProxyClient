

#pragma once

#include <windows.h>
#include <map>

#include "BillInterface.h"

using std::map;
using std::pair;


#define MAX_CALL            10


class IOCPSocket;


typedef struct tagBilInfo
{
	long ID;
}BilInfo, *pBilInfo;



#define TRADE_EVENT             1000
#define TRADE_VIP_EVENT         1001

typedef struct tagBilTask
{
    long msgID;
    long long orderID;
    long eventID;       //  见BillInterface的query type
    int section;
    long extend;
    void* body;
    IOCPSocket* sock;
}BilTask, *pBilTask;



typedef map<long, pBilInfo>			BillList;
typedef map<long, pBilTask>         BillTask;



class BillingInterface
{
public:
	BillingInterface();
	virtual ~BillingInterface();

	virtual bool Init(HWND hWnd);
	virtual bool Connect(const char** ip, unsigned short* port, int count);
	virtual bool Disconnect();
	virtual bool Terminate();
	virtual int Trade(__int64 order, const char* account, const char* character, const char* ip, long credit, int rate, const char* server_info, const char* memo);
	virtual int RegisterVIP(__int64 order, const char* account, long month, long credit, long vip_type, const char* ip, const char* server_info, const char* memo);
	virtual int Record(__int64 order, const char* account, const char* character, const char* ip, long store_id, const char* store_name, long store_class, long credit, long number, const char* server_info, long result);
	virtual int Query(const char* account, int operators);

	virtual int BilCallBack(long ret, long param);
    virtual int BilStatus(long stat, long threadid);

    virtual bool IsInit();

    virtual void PushTask(long id, pBilTask pTask);
    virtual void PopTask(long id);
    virtual int GetVersion();

protected:

    BillTask    m_task;
	BillList	m_list;
    HMODULE     m_hand;
    int         m_state;

};


