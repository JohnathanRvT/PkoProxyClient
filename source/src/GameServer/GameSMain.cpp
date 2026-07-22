// GameServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "GameAppNet.h"
#include "GameApp.h"
#include "SystemDialog.h"
#include "Config.h"
#include "GameDB.h"
#include "PrivilegeCheck.h"

#include "ErrorHandler.h"
#ifdef XTRAP_S
#include "Xtrap_S_Interface.h"
unsigned char g_XTrapMap[CSFLIE_NUM][XTRAP_CS4_BUFSIZE_MAP];
#endif
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // 设置入口地址

extern BOOL GameServer_Begin(const char* configName);
extern void GameServer_End();
BOOL g_bGameEnd = FALSE;
CGameApp* g_pGameApp = nullptr;
//std::string g_strLogName = "GameServerLog";
HANDLE hGameT;

void DisableCloseButton();
void AppExit();

void InitCom() {
	//CoInitialize(0);
}

void UninitCom() {
	//CoUninitialize();
}

//#pragma init_seg(user)
//#pragma init_seg( lib )
pi_LeakReporter pi_leakReporter("gamememleak.log");

#ifdef XTRAP_S
bool ServerStart() {
	DWORD dwRet = 0;
	FILE* fi;

	fi = fopen("Map.cs3", "rb");
	if (fi == NULL)
		return false;
	memset(g_XTrapMap, 0, sizeof(g_XTrapMap));
	fread(g_XTrapMap[0], XTRAP_CS4_BUFSIZE_MAP, 1, fi);
	fclose(fi);

	XTrap_S_LoadDll();
	XTrap_S_Start(600, 1, g_XTrapMap, NULL);

	return true;
}
#endif
int main(int argc, char* argv[]) {
	//SEHTranslator translator;

	ErrorHandler::Initialize();
	ErrorHandler::DisableErrorDialogs();

	CResourceBundleManage::Instance("GameServer.loc"); //Add by lark.li 20080304

	try {
		InitMessage();

		if (argc >= 2) {
			strncpy(szConfigFileN, argv[1], defCONFIG_FILE_NAME_LEN - 1);
			szConfigFileN[defCONFIG_FILE_NAME_LEN - 1] = '\0';
		}

		if (!g_Config.Load(szConfigFileN)) {
			LG("init", "config init...Fail!\n");
			return FALSE;
		}

		if (!PrivilegeCheck::Instance()->Init("GMCmd.txt")) {
			LG("init", "Load GMCmd.txt...Fail!\n");
			return FALSE;
		}

		char fileName[512];
		int len = (int)strlen(g_Config.m_szLogDir);
		if (len > 0 && g_Config.m_szLogDir[len - 1] == '\\')
			//sprintf(fileName, "%s%s", g_Config.m_szLogDir, "memorymonitor.log");
			_snprintf_s(fileName, sizeof(fileName), _TRUNCATE, "%s%s", g_Config.m_szLogDir, "memorymonitor.log");

		else
			//sprintf(fileName, "%s\\%s", g_Config.m_szLogDir, "memorymonitor.log");
			_snprintf_s(fileName, sizeof(fileName), _TRUNCATE, "%s\\%s", g_Config.m_szLogDir, "memorymonitor.log");

			//pi_Memory m(fileName);
			//m.startMonitor(1);
			// End

#ifdef XTRAP_S
		if (!ServerStart())
			return -1;
#endif

#ifdef __CATCH
		LG("init", "Define __CATCH\n");
#endif

		atexit(AppExit);

		char szPID[32];
		//sprintf(szPID, "%d", GetCurrentProcessId());
		_snprintf_s(szPID, sizeof(szPID), _TRUNCATE, "%d", GetCurrentProcessId());

		std::string strConsoleT;
		strConsoleT += "[PID:";
		strConsoleT += szPID;
		strConsoleT += "]";

		//strConsoleT += "   结束服务请关闭相应的Windows窗口，直接关闭本控制台将不会触发存盘操作！";
		strConsoleT += RES_STRING(GM_GAMESMAIN_CPP_00001);
		SetConsoleTitle(strConsoleT.c_str());
		DisableCloseButton();
		//WinExec("logvwr.exe", SW_SHOW);
		if (!GameServer_Begin(szConfigFileN)) {
			return 0;
		}

		MSG msg;
		while (!g_bGameEnd) {
			if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT) {
					if (!g_bGameEnd)
						g_pGameApp->SaveAllPlayer();
					g_bGameEnd = TRUE;
					break;
				}
			} else {
				SystemReport(GetTickCount());
				Sleep(50);
			}
		}

		// Add by lark.li 20080730 begin
		//m.stopMonitor();
		//m.wait();
		// End
#ifdef XTRAP_S
		XTrap_S_FreeDll();
#endif
		GameServer_End();
		//LG("init", "游戏地图服务器成功退出\n");
		LG("init", "game map server succeed to exit\n");
	}
	T_FINAL

	return 0;
}

//_DBC_USING
ThreadPool* l_proc = nullptr;
ThreadPool* l_comm = nullptr;

// Add by lark.li 20090112 begin
//ThreadPool* m_mmsproc, * m_mmscomm;	//同MMS的任务池
//ToMMS* g_ToMMS;	//同MMS的连接对象（主动重连机制）
// End

extern DWORD WINAPI g_GameLogicProcess(LPVOID lpParameter);
BOOL GameServer_Begin(const char* configName) {
	try {
		//MONITOR

		_setmaxstdio(2048);

		//LG("init", "游戏地图服务器[%s]启动...\n", g_Config.m_szName);
		LG("init", "game map server [%s] startup...\n", g_Config.m_szName);

		g_pGameApp = new CGameApp();
		if (!g_pGameApp->Init()) {
			//LG("init", "GameApp 初始化失败, 退出!\n");
			LG("init", "GameApp initialization failed, exit!\n");
			return FALSE;
		}

		TcpCommApp::WSAStartup();
		l_proc = ThreadPool::CreatePool(8, 8, 512);
		l_comm = ThreadPool::CreatePool(8, 8, 512); //,THREAD_PRIORITY_ABOVE_NORMAL);

		g_gmsvr = new GameServerApp(l_proc, l_comm);

		// 启动 GateServer 连接线程
		// LG("init", "启动Gate服务器连接线程...\n");
		LG("init", "startup Gate server connect thread...\n");
		l_comm->AddTask(new ConnectGateServer(g_gmsvr));

		//连接并监听TradeServer
		//LG("init", "启动信息服务器连接线程...\n");
		LG("init", "startup information server connect thread...\n");
		l_comm->AddTask(new ToTradeServer(g_gmsvr));

		// Add by lark.li 20090112 begin
		//m_mmsproc = ThreadPool::CreatePool(1, 2, 24, THREAD_PRIORITY_NORMAL);
		//m_mmscomm = ThreadPool::CreatePool(2, 4, 56, THREAD_PRIORITY_ABOVE_NORMAL);
		//g_ToMMS = new ToMMS(configName, m_mmsproc, m_mmscomm);
		//m_mmsproc->AddTask(new ConnectMMS(g_ToMMS));
		// End

		// 创建游戏线程
		//LG("init", "启动游戏线程...\n");
		LG("init", "startup game thread...\n");
		DWORD dwThreadID;
		hGameT = CreateThread(nullptr, 0, g_GameLogicProcess, nullptr, 0, &dwThreadID);
		LG("init", "Game Thread ID = %d\n", dwThreadID);
		//

		//LG("init",  "开始创建Win32 控制对话框\n");
		LG("init", "start create Win32 control dialog box\n");
		HINSTANCE hInst = GetModuleHandle(nullptr);
		CreateMainDialog(hInst, nullptr);

		//Log("重启", "GameServer重启", g_Config.m_szMapList[0], "", "", "");
		Log("restart", "GameServer restart", g_Config.m_szMapList[0], "", "", "");

		return TRUE;
	}
	T_E
}

void GameServer_End() {
	try {
		//LG("init", "开始结束游戏地图服务器\n");
		LG("init", "start to exit game map server\n");
		//GameServerLG(g_strLogName.c_str(), "GameServer End!\n");
		CloseHandle(hGameT);

		// 输出信息
		MONITOR_DUMP

		HWND hConsole = GetConsoleWindow();
		if (hConsole) {
			SendMessage(hConsole, WM_CLOSE, 0, 0);
		}

		Sleep(400);

		SAFE_DELETE(g_pGameApp);

		delete g_gmsvr;

		// Add by lark.li 20090112 begin
		//delete g_ToMMS;
		//m_mmsproc->DestroyPool();
		//m_mmscomm->DestroyPool();
		// End

		l_comm->DestroyPool();
		l_proc->DestroyPool();

		TcpCommApp::WSACleanup();
	}
	T_E
}

using LPGETCONSOLEWINDOW = HWND (*)();
void DisableCloseButton() {
	HMODULE hMod = LoadLibrary("kernel32.dll");

	LPGETCONSOLEWINDOW lpfnGetConsoleWindow =
		(LPGETCONSOLEWINDOW)GetProcAddress(hMod, "GetConsoleWindow");

	HWND hWnd = lpfnGetConsoleWindow();
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	if (hMenu != nullptr) {
		EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
	}

	FreeLibrary(hMod);
}

void AppExit() {
	//int	*p = NULL;
	// *p = 1;
}

/*
 GameServer设计

 GameServer负责游戏逻辑的处理

 包含的主要模块有 
 
[GameData]
子数据类型有 
Map        地图 
MgrUnit    地图管理单元
Player     玩家
Character  角色
Item       道具
Skill      技能
SkillState 技能状态
Mission    任务

 
GameData应该单纯的做为数据容器

[GameControl]
App       应用程序框架 
TimerMgr  定时器管理
AI        AI控制

[EventHandler] 事件处理器

GameServer的运作方式为 

由GameControl启动应用程序, 启动AI定时器, 启动碰撞检测定时器

GameControl 向 EventHandler 产生Event, 比如AI事件, 与跳转点碰撞
客户端 向 EventHandler 产生Event, 比如请求行走, 请求使用技能, 请求使用道具

EventHandler对Event的处理为应答式, 即当时返回结果, 无任何中间状态

EventHandler在对Event的处理过程中, 产生Modify GameData的操作

此时EventHandler对于服务器内部逻辑和来自客户端的请求， 表现为一个双向插头, 并产生
唯一的结果Modify GameData


Control -> Event 
 





*/
