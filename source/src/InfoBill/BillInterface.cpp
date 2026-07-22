#include "stdafx.h"
#include "BillInterface.h"
#include <stdio.h>

HWND g_HWnd;
FILE * g_logFile;
static int index = 1;

void Log(const char* msg)
{
	fprintf(g_logFile, "%s\r\n", msg);
	fflush(g_logFile);
}

BILAPI int bilVersion()
{
	return 1;
}

BILAPI int bilInit(bilInstance instance)
{
	//g_logFile = fopen("E:\\Item\\Source\\VC2003\\Unicode\\Project_Kop\\vss_server\\TradeSystem\\TradeServer\\bin\\InfoBill.log","w+");
	fopen_s(&g_logFile, "InfoBill.log", "w+");

	Log("bilInit");
	g_HWnd = instance;
	return 1;
}

BILAPI int bilTerminate()
{
	Log("bilTerminate");
	fclose(g_logFile);

	return 1;
}

BILAPI int bilConnect(const char** ip, unsigned short* port, int count)
{
	Log("bilConnect");
	for(int i=0;i<count;i++)
	{
		
	}

	return 1;
}

BILAPI int bilDisconnect()
{
	Log("bilDisconnect");
	return 1;
}

BILAPI int bilTrade(const char* order, const char* current_time, const char* account, const char* character, const char* ip , long credit, int rate, const char* server_info, const char* memo)
{
	Log("bilTrade");
	return index ++;
}

BILAPI int bilRegisterVIP(const char* order, const char* current_time, const char* account, long month, long credit, long vip_type, const char* ip, const char* server_info, const char* memo)
{
	Log("bilRegisterVIP");
	return index ++;
}

typedef struct _MyData {
    int i;
    int query;
} MYDATA, *PMYDATA;

DWORD WINAPI ThreadProc( LPVOID lpParam ) 
{
	PMYDATA pData = (PMYDATA)lpParam;

	::Sleep(100);

	int i = pData->i;

	if(QUERY_MONEY == pData->query )
        SendMessage(g_HWnd, MLY_bilNotify, i,100000 );

	if(QUERY_KOP_ACCOUNT == pData->query)
        SendMessage(g_HWnd, MLY_bilNotify, i,10 );

	if(QUERY_KOP_VIP_TYPE == pData->query)
        SendMessage(g_HWnd, MLY_bilNotify, i,20 );


	return 0; 
}

BILAPI int bilQuery(const char* account, int query)
{
	Log("bilQuery");


	index ++;

	DWORD dwThreadId;
	PMYDATA pData;
	pData = (PMYDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(MYDATA));
	
	pData->i = index;
	pData->query = query;

	CreateThread( NULL, 0, ThreadProc,  pData, 0, &dwThreadId);


	return index;
}

BILAPI int bilRecord(const char* order, const char* account, const char* character, const char* ip, long store_id, const char* store_name, long store_class, const char* current_time, long credit, long number, const char* server_info, long result)
{
	Log("bilRecord");
	return index ++;
}
