
//////////////////////////////////////////////////
//
//      Moliyo Billing Interface
//          Definition File
//
//          Version 1.0.1
//
//                          by: Jampe
//                      modify: 2006-11-03
//
//////////////////////////////////////////////////


#if !defined MOLIYO_BILLING_INTERFACE
#define MOLIYO_BILLING_INTERFACE

//  win32 system
#if (defined WIN32 || defined WIN64)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>

    #define bilInstance   HWND

    #ifndef BILAPI
        #define BILAPI __declspec(dllexport)
    #endif
#endif

#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif


//  window message
#define MLY_bilNotify           0x1000
#define MLY_bilStatus           0x1001

//  notify message
#define RET_SUCCEED             -10000
#define RET_NO_MONEY            -10001
#define RET_INVALID_ACCOUNT     -10002
#define RET_BANNED_ACCOUNT      -10003
#define RET_EXPIRE              -10004
#define RET_NO_SERVICE          -10005
#define RET_INVALID_TRADE       -10006
#define RET_TIMEOUT             -10007

//  status message
#define STAT_ONLINE             0x0001
#define STAT_OFFLINE            0x0002

//  query type
#define QUERY_MONEY             0x0001
#define QUERY_KOP_ACCOUNT       0x0002
#define QUERY_KOP_VIP_TYPE      0x0003

BILAPI int bilVersion();
typedef int (*PBILVERSION)();

BILAPI int bilInit(bilInstance instance);
typedef int (*PBILINIT)(bilInstance instance);

BILAPI int bilTerminate();
typedef int (*PBILTERMINATE)();

BILAPI int bilConnect(const char** ip, unsigned short* port, int count);
typedef int (*PBILCONNECT)(const char** ip, unsigned short* port, int count);

BILAPI int bilDisconnect();
typedef int (*PBILDISCONNECT)();

BILAPI int bilTrade(const char* order, const char* current_time, const char* account, const char* character, const char* ip , long credit, int rate, const char* server_info, const char* memo);
typedef int (*PBILTRADE)(const char* order, const char* current_time, const char* account, const char* character, const char* ip , long credit, int rate, const char* server_info, const char* memo);

BILAPI int bilRegisterVIP(const char* order, const char* current_time, const char* account, long month, long credit, long vip_type, const char* ip, const char* server_info, const char* memo);
typedef int (*PBILREGISTERVIP)(const char* order, const char* current_time, const char* account, long month, long credit, long vip_type, const char* ip, const char* server_info, const char* memo);

BILAPI int bilQuery(const char* account, int query);
typedef int (*PBILQUERY)(const char* account, int query);

BILAPI int bilRecord(const char* order, const char* account, const char* character, const char* ip, long store_id, const char* store_name, long store_class, const char* current_time, long credit, long number, const char* server_info, long result);
typedef int (*PBILRECORD)(const char* order, const char* account, const char* character, const char* ip, long store_id, const char* store_name, long store_class, const char* current_time, long credit, long number, const char* server_info, long result);


#ifdef __cplusplus
}
#endif


#endif
