


#if !defined __INFOTYPE_H__
#define __INFOTYPE_H__


#include "iocp.h"
#include "netthread.h"
#include "storethread.h"
#include "managethread.h"
#include "logthread.h"
#include "mailthread.h"
#include "billing.h"
#include "log.h"
#include "msgqueue.h"
#include <InfoNet.h>

// Add by lark.li 20080819 begin
#include "webservicethread.h"
#include <string>
using namespace std;
// End

#define TEST_SERVER         0

#define STORE_THREAD_NUMBER         1

#define CONFIG_FILE_NAME         "TradeServer.cfg"

typedef struct _InfoMessage_
{
    pNetMessage         msg;
    IOCPSocket*         client;
}InfoMessage, *pInfoMessage;


#define INFO_INIT           0
#define INFO_RUNNING        1
#define INFO_STOPING        2

typedef _AnyQueue<pInfoMessage> MsgList;
typedef _AnyQueue<const char*>  LogMsgList;

// Add by lark.li 20080819 begin
typedef struct _WebServiceParam_
{
	string MemberPK;
	string TranID;
	int Credit;
	string RecTime;
	string Memo;
}WebServiceParam, *pWebServiceParam;

typedef _AnyQueue<WebServiceParam*>  WebServiceList;
// End

typedef struct _ServerInfo_
{
    Socket2         serv_sock;
    IOCP            serv_iocp;
    AcceptThread    serv_apt;
    IOCPWorkThread* serv_work;
    int             serv_numWork;
    short           serv_port;
    char            serv_name[260];
    int             serv_state;
    char            serv_dir[260];
    int             serv_rate;

    char            billing_serv[2][32];
    unsigned short  billing_port[2];

    long            excpt_count;
    time_t          start_time;

    BillingInterface    billing;

    char        db_serv[260];
    char        db_usr[64];
    char        db_pwd[64];
    char        db_name[128];

	// Add by lark.li 20080819 begin
	char		webHttp[256];			// 服务地址
	char		webUserName[64];		// 用户名
	char		webPassWord[64];		// 密码
	int			type;					// 游戏区
	int			typesub;				// 游戏分组
	// End

}ServerInfo, *pServerInfo;


typedef struct _LogicThreadInfo_
{
    StoreThread     lgc_store[STORE_THREAD_NUMBER];
    ManageThread    man_thr;
    LogThread       log_thr;
    MailThread      mail_thr;

	// Add by lark.li 20080819 begin
	WebServiceThread web_thr;
	// End

}LogicThreadInfo, *pLogicThreadInfo;


typedef struct _ListInfo_
{
    SockList        sock_list;
    MsgList         store_list;
    MsgList         man_list;
    MsgList         mail_list;
    LogMsgList      log_list;

	// Add by lark.li 20080819 begin
	WebServiceList	web_list;
	// End

}ListInfo, *pListInfo;


extern ServerInfo          g_servInfo;
extern LogicThreadInfo     g_logicInfo;
extern ListInfo            g_listInfo;



#define LOG_FOLDER          "LOG"

extern void WaitForExit();

#endif
