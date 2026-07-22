// GroupServerApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GroupServerApp.h"

// 初始化顺序
//#pragma init_seg( lib )
pi_LeakReporter pi_leakReporter("groupememleak.log");

//===========================================================

#if 0
class TBLTest : public  cfl_rs
{
public:
	TBLTest(cfl_db* db):cfl_rs(db,"Test", 4){ }

	void Select(int id1 , int id2)
	{
		int l_retrow	=0;
		char* param = "*";
		char filter[200];
		_snprintf_s(filter,sizeof(filter),_TRUNCATE, "id>=%d and id <= %d", id1, id2);
		_get_row(m_buf, 7, param, filter,&l_retrow);
	}

	void Update(int id)
	{
		char sql[255];

		_snprintf_s(sql,sizeof(sql),_TRUNCATE, "update %s set memo='XXX%d' where id =%d", _get_table(), id, id);

		exec_sql_direct(sql);
	}
private:
	string m_buf[4];
};

#define MAX_THREADS 60

#define RANGE_MIN 0;
#define RANGE_MAX 100;

DWORD WINAPI SelectThreadProc( LPVOID lpParam ) 
{ 
	cfl_db testDB;
	string err;
	std::string l_str;
	testDB.enable_errinfo();

	srand( (unsigned)time( NULL ) );

	bool b = testDB.connect("192.168.16.114", "GameDB", "lark", "zxcvbnm", err);

	TBLTest t(&testDB);
	
	for(int i=0; i<1000000; i++)
	{
         int r1 = ((double) rand() /  (double) RAND_MAX) * RANGE_MAX + RANGE_MIN;
		 int r2 = ((double) rand() /  (double) RAND_MAX) * RANGE_MAX + RANGE_MIN;
		 
		 t.Select(min(r1, r2), max(r1, r2));

		 //std::cout << "SelectThreadProc" << std::endl;
	}

    return 0; 
} 

DWORD WINAPI UpdateThreadProc( LPVOID lpParam ) 
{ 
	cfl_db testDB;
	string err;
	std::string l_str;
	testDB.enable_errinfo();

	srand( (unsigned)time( NULL ) );

	bool b = testDB.connect("192.168.16.114", "GameDB", "lark", "zxcvbnm", err);

	TBLTest t(&testDB);
	
	for(int i=0; i<1000000; i++)
	{
         int r = ((double) rand() /  (double) RAND_MAX) * RANGE_MAX + RANGE_MIN;
		 t.Update(r);

		 //std::cout << "UpdateThreadProc" << std::endl;
	}

    return 0; 
}
#endif
//===========================================================

int main(int argc, char* argv[]) {
	//SEHTranslator translator;
	CResourceBundleManage::Instance("GroupServer.loc"); //Add by alfred.shi 20080130

	//==================================================================
#if 0
	srand( (unsigned)time( NULL ) );

	DWORD dwThreadId[MAX_THREADS];
	HANDLE hThread[MAX_THREADS]; 

	for(int i=0; i<20; i++ )
	{
		hThread[i] = CreateThread(NULL, 0, SelectThreadProc, NULL, 0 , &dwThreadId[i]);

		if (hThread[i] == NULL) 
		{
			ExitProcess(i);
		}
	}

	for(int i=20; i<MAX_THREADS; i++ )
	{
		hThread[i] = CreateThread(NULL, 0, UpdateThreadProc, NULL, 0 , &dwThreadId[i]);

		if (hThread[i] == NULL) 
		{
			ExitProcess(i);
		}
	}

	// Wait until all threads have terminated.

	WaitForMultipleObjects(MAX_THREADS, hThread, TRUE, INFINITE);

	// Close all thread handles upon completion.

	for(int i=0; i<MAX_THREADS; i++)
	{
		CloseHandle(hThread[i]);
	}

	return 0;
#endif
	//==================================================================
	try {
		//WinExec("logvwr.exe", SW_SHOW);
		TcpCommApp::WSAStartup();
		ThreadPool* l_proc = ThreadPool::CreatePool(8, 8, 2048);
		ThreadPool* l_comm = ThreadPool::CreatePool(4, 4, 512, THREAD_PRIORITY_ABOVE_NORMAL);

		try {
			g_gpsvr = new GroupServerApp(l_proc, l_comm);
		} catch (...) {
			l_comm->DestroyPool();
			l_proc->DestroyPool();
			TcpCommApp::WSACleanup();
			Sleep(10 * 1000);
			return -1;
		}
		while (!g_exit) {

			std::string input;
			std::cout << RES_STRING(GP_GROUPSMAIN_CPP_00001);
			std::cin >> input;
			if (input == "exit" || g_exit) {
				std::cout << RES_STRING(GP_GROUPSMAINCPP_00002) << std::endl;
				break;
			} else if (input == "logbak") {
				LogStream::Backup();
			} else {
				std::cout << RES_STRING(GP_GROUPSMAIN_CPP_00003) << std::endl;
			}
		}
		//if(!g_exit)
		{
			g_exit = 1;
			while (g_ref) {
				Sleep(1);
			}
			delete g_gpsvr;

			l_comm->DestroyPool();
			l_proc->DestroyPool();
			TcpCommApp::WSACleanup();
			g_exit = 2;
			Sleep(2000);
		}
		while (g_exit != 2) {
			Sleep(1);
		}
	}
	T_FINAL

	return 0;
}
