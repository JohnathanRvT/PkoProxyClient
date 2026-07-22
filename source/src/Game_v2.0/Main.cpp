//------------------------------------------------------------------------
//	2005.3.25	Arcol	锟斤拷锟斤拷锟街凤拷锟斤拷为锟叫癸拷锟斤拷陆锟斤拷锟斤拷
//------------------------------------------------------------------------

// MPTest.cpp : Defines the entry point for the application.
//
#include "stdafx.h"

//#define WIN32_LEAN_AND_MEAN

#include "Main.h"
#include "SceneObjFile.h"
#include "UIImeinput.h"
#include "PacketCmd.h"
#include "UIsystemform.h"
#include "Lua_Platform.h"
#include "packfile.h"
#include "loginscene.h"

#include "ErrorHandler.h"

#include "UdpClient.h"
using namespace client_udp;

std::string g_serverset;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;					 // current instance
char szTitle[MAX_LOADSTRING];		 // The title bar text
char szWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance, HBRUSH brush);
HWND InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HBRUSH g_bk_brush;

void StdoutRedirect();
int InstallFont(const char* pszPath); // 锟斤拷锟藉安装
void CenterWindow(HWND hWnd);		  //锟斤拷锟斤拷位锟斤拷锟斤拷锟斤拷

BOOL CheckDxVersion(DWORD& ver) // 锟芥本锟剿讹拷
{
	BOOL ret = 1;
	MPISystemInfo* sys_info = nullptr;

	MPGUIDCreateObject((LW_VOID**)&sys_info, LW_GUID_SYSTEMINFO);
	if (SUCCEEDED(sys_info->CheckDirectXVersion())) // 锟介看锟芥本
	{
		ver = sys_info->GetDirectXVersion();

		if (ver < DX_VERSION_8_1) //锟叫断版本锟角凤拷锟斤拷确
		{
			ret = 0;
		}
	}
	sys_info->Release();

	return ret;
}

bool g_IsAutoUpdate = false; // 锟斤拷锟揭拷远锟斤拷锟斤拷锟绞憋拷锟揭拷却锟斤拷锟斤拷锟斤拷顺锟绞憋拷锟斤拷锟斤拷远锟斤拷锟斤拷锟?
long g_nCurrentLogNo = 0;

////////////////////////////////////////
//
//  By Jampe
//  锟斤拷锟斤拷锟教斤拷锟杰癸拷锟斤拷
//  2006/5/22
//
const unsigned char key[] = {0xda, 0x15, 0x1c, 0x10, 0x4f, 0x8c, 0x7a, 0x4a};

//////////////////////////////////////////////////////////////////////

DWORD WINAPI LoadRes(LPVOID lpvThreadParam) // 锟斤拷锟斤拷源
{
	g_bLoadRes = true;

	return 0;
}

HINSTANCE g_hInstance;

// A return value of false means we should exit the game.
// Returns true if we should continue running.
bool HandleCommandLineArguments(const LPSTR cmdLine) {

	// Returns true if cmdLine contains substring 'key'
	auto cmd_contains = [cmdLine](const char* key) -> bool {
		return std::strstr(cmdLine, key) == nullptr ? false : true;
	};

	if (cmd_contains("editor")) { // 锟斤拷锟斤拷锟较凤拷嗉拷锟?
		g_Config.m_bEditor = TRUE;
		g_Config.m_IsShowConsole = true;
	} else {
		g_Config.m_bEditor = FALSE;
		g_Config.m_nCreateScene = 0;
	}

	//--------------
	if (cmd_contains("table_bin")) { //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟絫able_bin 锟斤拷锟斤拷锟缴讹拷锟斤拷锟狡憋拷
		extern void MakeBinTable();
		MakeBinTable();
		return false;
	}

	//--------------
	if (cmd_contains("-objtrim")) {
		g_ObjFile.TrimDirectory("map", cmd_contains("backup"));
		return false;
	}

	//--------------
	if (cmd_contains("pack")) {
		const char* pszPos = cmdLine;
		pszPos += 4;
		while (*pszPos == ' ') {
			pszPos++;
		}
		char pszpath[128];
		char pszFile[128];
		_snprintf_s(pszpath, _TRUNCATE, R"(texture\minimap\%s)", pszPos);
		_snprintf_s(pszFile, _TRUNCATE, R"(texture\minimap\%s\%s.pk)", pszPos, pszPos);

		CMiniPack pkfile;
		if (pkfile.SaveToPack(pszpath, pszFile)) {
			LG("ok", RES_STRING(CL_LANGUAGE_MATCH_188), pszPos);
		} else {
			LG("ok", RES_STRING(CL_LANGUAGE_MATCH_189), pszPos);
		}
		return false;
	}
	//--------------
	if (!cmd_contains("startgame")) {
		LG("Error", RES_STRING(CL_LANGUAGE_MATCH_190));
		return false;
	}

	//--------------
	//#ifdef KOP_TOM
	if (cmd_contains("tom:")) {
		const std::string strParam{cmdLine};
		const auto nTomPos = strParam.find("tom:");

		// 锟斤拷锟斤拷Tom锟街凤拷锟斤拷,锟斤拷锟斤拷为:"tom:锟斤拷锟斤拷锟斤拷IP,锟剿匡拷,锟矫伙拷锟斤拷,锟斤拷锟斤拷"
		std::string strTom = strParam.substr(nTomPos + 4, strParam.length());
		auto nEnd = (int)strTom.length();
		nEnd = (int)strTom.find(" ");
		if (nEnd != -1)
			strTom = strTom.substr(0, nEnd);

		std::string strList[4];
		Util_ResolveTextLine(strTom.c_str(), strList, 4, ',');

		g_TomServer.szServerIP = strList[0];
		g_TomServer.nPort = atoi(strList[1].c_str());
		g_TomServer.szUser = strList[2];
		g_TomServer.szPassword = strList[3];
		g_TomServer.bEnable = true;
	}
	//#endif

	//--------------
	////////////////////////////////////////
	//
	//  By Jampe
	//  锟斤拷锟斤拷锟教诧拷锟斤拷锟斤拷锟斤拷
	//  2006/5/19
	//
	g_cooperate = {};
	if (cmd_contains("/login:")) {
		const std::string strParam{cmdLine};
		const auto nCopPos = strParam.find("/login:");
		//  /login:锟教家达拷锟斤拷&锟斤拷锟斤拷/锟斤拷锟斤拷锟斤拷锟斤拷&锟矫伙拷锟斤拷&锟斤拷锟斤拷
		//SetEncKey(key);
		std::string strCop = strParam.substr(nCopPos + 7, strParam.length());
		std::string tmp[4];
		Util_ResolveTextLine(strCop.c_str(), tmp, 4, '&');

		g_cooperate.code = atol(tmp[0].c_str());
		g_cooperate.serv = tmp[1];
		g_cooperate.uid = tmp[2];
		unsigned char buff[128] = {0};
		//Decrypt(buff, 128, (unsigned char*)tmp[3].c_str(), (int)tmp[3].length());
		g_cooperate.pwd = (char*)buff;
	}
	switch (g_cooperate.code) { // 锟斤拷同锟斤拷锟斤拷锟教讹拷应锟斤拷同锟斤拷锟斤拷锟斤拷
	case COP_OURGAME: {
		g_serverset = "scripts/txt/server2.tx";
	} break;
	case COP_SINA: {
		g_serverset = "scripts/txt/server3.tx";
	} break;
	case COP_CGA: {
		g_serverset = "scripts/txt/server4.tx";
	} break;
	default: {
		g_serverset = "scripts/txt/server.tx";
	} break;
	}
	//--------------

	return true;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR lpCmdLine,
					   int nCmdShow) {
	g_hInstance = hInstance;

	// Add by lark.li 20080909 begin
	ErrorHandler::Initialize();
	ErrorHandler::DisableErrorDialogs();
	// End

	//CHANGED: Game doesn't work without this
	CResourceBundleManage::Instance("Game.loc"); //Add by lark.li 20080130

	DWORD dx_ver = DX_VERSION_X_X;

	try {

		LG("init", "Define __CATCH\n");
		//SEHTranslator translator;

		if (!HandleCommandLineArguments(lpCmdLine)) {
			return false;
		}

		// Add by lark.li 20080730 begin
		pi_Memory m("memorymonitor.log");
		m.startMonitor(1);
		// End
		/*	yangyinyu	2008-10-14	close!
	//  锟斤拷示loading锟斤拷锟斤拷
	GameLoading::Init(strParam)->Create();
*/
		::_setmaxstdio(2048); //   锟斤拷锟斤拷锟斤拷锟斤拷锟侥硷拷锟斤拷

		//setlocale(LC_CTYPE, g_oLangRec.GetString(0)); // 锟侥达拷锟街凤拷锟斤拷锟斤拷源锟侥硷拷锟斤拷锟饺? Add by Philip.Wu  2006-07-19
		setlocale(LC_CTYPE, RES_STRING(CL_LANGUAGE_MATCH_0)); // 锟侥达拷锟街凤拷锟斤拷锟斤拷源锟侥硷拷锟斤拷锟饺? Add by Philip.Wu  2006-07-19

		InstallFont(".\\Font"); // 锟皆讹拷锟斤拷装锟斤拷锟斤拷 Add by Philip.Wu  2006-08-07

		if (CheckDxVersion(dx_ver) == 0) {
			MessageBox(nullptr, RES_STRING(CL_LANGUAGE_MATCH_187), "error", MB_OK);
			return 0;
		}

		auto successDelete = []() -> bool {
			const char szUpdateFileName[] = "kop_d.exe";
			SetFileAttributes(szUpdateFileName, FILE_ATTRIBUTE_NORMAL);
			return DeleteFile(szUpdateFileName) != 0 ? true : false;
		}(); //NOTE: Immediately invoked!

		// Initialize global strings
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_KOP, szWindowClass, MAX_LOADSTRING);

		HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
		MyRegisterClass(hInstance, brush); // 注锟结定锟斤拷锟斤拷锟?

		HACCEL hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_KOP); // 锟斤拷锟截匡拷菁锟斤拷锟?锟斤拷帽锟斤拷锟?

#ifdef _LUA_GAME
		InitLuaPlatform();
#endif

		// 锟斤拷锟斤拷突锟斤拷锟绞癸拷貌锟酵拷锟絣og锟侥硷拷锟斤拷
		HANDLE hFile = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(long), "KopClinetNO");
		if (!hFile) {
			return 0;
		}

		LGInfo lg_info = {};
		lg_info.bEnableAll = g_Config.m_bEnableLG != 0;
		lg_info.bMsgBox = g_Config.m_bEnableLGMsg != 0;
		lg_info.bEraseMode = true;

		if (!g_Config.IsPower()) { // 锟斤拷锟角编辑锟斤拷模式时log锟截憋拷

			lg_info.bCloseAll = true;
		}

		auto* pClientLogNO = static_cast<long*>(MapViewOfFile(hFile, FILE_MAP_WRITE, 0, 0, 0));
		if (pClientLogNO == nullptr) {
			throw std::exception("pClientLogNo is nullptr!");
		}
		g_nCurrentLogNo = *pClientLogNO;
		(*pClientLogNO)++;
		if (g_nCurrentLogNo > 0) {
			_snprintf_s(lg_info.dir, _TRUNCATE, "%s%d", "log", g_nCurrentLogNo);
		} else {
			_snprintf_s(lg_info.dir, _TRUNCATE, "%s", "log");
		}

		MPTimer t;
		t.Begin();
		g_pGameApp = std::make_unique<CGameApp>();
		g_pGameApp->LG_Config(lg_info);

		//FIXED by Snre3n: Moved the following code here - prevents a crash.
		//锟斤拷锟斤拷锟斤拷业锟斤拷远锟斤拷锟斤拷锟侥伙拷锟斤拷锟?
		if (!g_stUISystem.m_isLoad) {
			// NOTE: Calls g_pGameApp internally!
			g_stUISystem.LoadCustomProp(); //锟斤拷取系统锟斤拷锟矫诧拷应锟斤拷
		}

		constexpr int nScreenMode[2][2] = {{800, 600}, {1024, 768}}; // 锟斤拷幕锟斤拷示锟斤拷式

		g_Config.m_bFullScreen = g_stUISystem.m_sysProp.m_videoProp.bFullScreen;
		int nWidth(0), nHeight(0), nDepth(0);
		if (g_stUISystem.m_sysProp.m_videoProp.bPixel1024) {
			nWidth = 1024;
			nHeight = 768;
		} else {
			nWidth = 800;
			nHeight = 600;
		}
		nDepth = g_stUISystem.m_sysProp.m_videoProp.bDepth32 ? 32 : 16;

		MPD3DCreateParamAdjustInfo cpai;
		cpai.multi_sample_type = (D3DMULTISAMPLE_TYPE)g_Config.m_dwFullScreenAntialias;

		g_Render.SetD3DCreateParamAdjustInfo(&cpai);

		// g_Render.EnableClearTarget(FALSE);
		g_Render.SetBackgroundColor(D3DCOLOR_XRGB(10, 10, 125));

		if (g_Config.m_bFullScreen) {
			nWidth = GetSystemMetrics(SM_CXSCREEN);
			nHeight = GetSystemMetrics(SM_CYSCREEN);
		}

		if (!g_pGameApp->Init(hInstance, szWindowClass, nWidth, nHeight, nDepth)) {
			LG("init", RES_STRING(CL_LANGUAGE_MATCH_191));
			g_pGameApp->End();
			return 0;
		}

		if (g_Config.m_bFullScreen) {
			g_pGameApp->ChangeVideoStyle(nWidth, nHeight, D3DFMT_D16, true);
		}

		//DWORD ThreadID;
		//   HANDLE hThread = ::CreateThread(NULL,0,LoadRes,
		//                             0,0,&ThreadID);

		//g_pGameApp->LoadRes2();

		if (g_Config.m_bEditor)
			StdoutRedirect();

		LG("init", "init use time = %d ms\n", t.End());

		DWORD st_dwtick = GetTickCount();
		DWORD st_tickcount = (GetTickCount() - st_dwtick) / unsigned long(g_NetIF->m_framedelay);

		g_NetIF->m_framedelay = 40; // 帧锟接筹拷

		g_pGameApp->Run();
		// Add by lark.li 20080730 begin
		m.stopMonitor();
		m.wait();
		// End
		g_pGameApp->End();

		::DeleteObject(brush);

		UnmapViewOfFile(pClientLogNO);
		CloseHandle(hFile);
		GetRegionMgr()->Exit();

		if (g_IsAutoUpdate) // 锟斤拷锟揭拷远锟斤拷锟斤拷锟绞?锟斤拷示原始锟斤拷锟斤拷 锟斤拷锟斤拷kop.exe
		{
			::WinExec("kop.exe", SW_SHOWNORMAL);
		}
		// 锟皆讹拷锟斤拷锟斤拷时 锟届常锟斤拷锟斤拷锟斤拷锟?
	} catch (std::exception& e) {
		std::string strfile;
		LG_GetDir(strfile);
		strfile += "\\exception.txt";

		FILE* fp = fopen(strfile.c_str(), "a+");

		if (fp) {
			SYSTEMTIME st;
			char tim[100] = {0};
			GetLocalTime(&st);
			_snprintf_s(tim, _TRUNCATE, "%02d-%02d %02d:%02d:%02d\r\n", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			fwrite(tim, strlen(tim), 1, fp);

			try {
				OSVERSIONINFOEX osvi;
				BOOL bOsVersionInfoEx;

				// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
				ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
				osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
				if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi))) {
					// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
					osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					if (!GetVersionEx((OSVERSIONINFO*)&osvi)) {
						bOsVersionInfoEx = FALSE;
					}
				}

				std::string mOSStringSimple = "Unknown Windwos Ver";

				if (bOsVersionInfoEx) {
					switch (osvi.dwPlatformId) {
					case VER_PLATFORM_WIN32_NT: {
						// Test for the product.
						if (osvi.dwMajorVersion <= 4) {
							mOSStringSimple = "Microsoft Windows NT ";
						} else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
							mOSStringSimple = "Microsoft Windows 2000 ";
						} else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
							mOSStringSimple = "Microsoft Windows XP ";
						} else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
							if (osvi.wProductType == VER_NT_WORKSTATION)
								mOSStringSimple = "Microsoft Windows XP x64 Edition ";
							else
								mOSStringSimple = "Microsoft Windows Server 2003 ";
						} else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0) {
							if (osvi.wProductType == VER_NT_WORKSTATION)
								mOSStringSimple = "Microsoft Windows Vista ";
							else
								mOSStringSimple = "Microsoft Windows Vista Server ";
						} else // Use the registry on early versions of Windows NT.
						{
							mOSStringSimple += "Unknown Windows NT";
						}
					} break;

					case VER_PLATFORM_WIN32_WINDOWS:
						// Test for the Windows 95 product family.
						if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
							mOSStringSimple = "Microsoft Windows 95 ";
							if (osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B') {
								mOSStringSimple += "OSR2 ";
							}
						}
						if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
							mOSStringSimple = "Microsoft Windows 98 ";
							if (osvi.szCSDVersion[1] == 'A') {
								mOSStringSimple += "SE ";
							}
						}
						if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
							mOSStringSimple = "Microsoft Windows Millennium Edition ";
						}
						break;
					}

					fprintf(fp, "%s\r\n", mOSStringSimple.c_str());
				}

				fprintf(fp, "DirectX Ver %X\r\n", dx_ver);

				D3DCAPSX caps;
				g_Render.GetDevice()->GetDeviceCaps(&caps);

				fprintf(fp, "DeviceType %X\r\n", caps.DeviceType);
				fprintf(fp, "AdapterOrdinal %X\r\n", caps.AdapterOrdinal);
				fprintf(fp, "Caps %X\r\n", caps.Caps);
				fprintf(fp, "Caps2 %X\r\n", caps.Caps2);
				fprintf(fp, "Caps3 %X\r\n", caps.Caps3);
				fprintf(fp, "PresentationIntervals %X\r\n", caps.PresentationIntervals);

				fprintf(fp, "CursorCaps %X\r\n", caps.CursorCaps);
				fprintf(fp, "DevCaps %X\r\n", caps.DevCaps);

				fprintf(fp, "PrimitiveMiscCaps %X\r\n", caps.PrimitiveMiscCaps);
				fprintf(fp, "RasterCaps %X\r\n", caps.RasterCaps);
				fprintf(fp, "ZCmpCaps %X\r\n", caps.ZCmpCaps);
				fprintf(fp, "SrcBlendCaps %X\r\n", caps.SrcBlendCaps);
				fprintf(fp, "DestBlendCaps %X\r\n", caps.DestBlendCaps);
				fprintf(fp, "AlphaCmpCaps %X\r\n", caps.AlphaCmpCaps);
				fprintf(fp, "ShadeCaps %X\r\n", caps.ShadeCaps);
				fprintf(fp, "TextureCaps %X\r\n", caps.TextureCaps);
				fprintf(fp, "TextureFilterCaps %X\r\n", caps.TextureFilterCaps);

				fprintf(fp, "CubeTextureFilterCaps %X\r\n", caps.CubeTextureFilterCaps);
				fprintf(fp, "VolumeTextureFilterCaps %X\r\n", caps.VolumeTextureFilterCaps);
				fprintf(fp, "TextureAddressCaps %X\r\n", caps.TextureAddressCaps);
				fprintf(fp, "VolumeTextureAddressCaps %X\r\n", caps.VolumeTextureAddressCaps);

				fprintf(fp, "LineCaps %X\r\n", caps.LineCaps);
				fprintf(fp, "MaxTextureWidth %X\r\n", caps.MaxTextureWidth);
				fprintf(fp, "MaxTextureHeight %X\r\n", caps.MaxTextureHeight);

				fprintf(fp, "MaxVolumeExtent %X\r\n", caps.MaxVolumeExtent);
				fprintf(fp, "MaxTextureRepeat %X\r\n", caps.MaxTextureRepeat);
				fprintf(fp, "MaxTextureAspectRatio %X\r\n", caps.MaxTextureAspectRatio);
				fprintf(fp, "MaxAnisotropy %X\r\n", caps.MaxAnisotropy);
				fprintf(fp, "MaxVertexW %f\r\n", caps.MaxVertexW);
				fprintf(fp, "GuardBandLeft %f\r\n", caps.GuardBandLeft);
				fprintf(fp, "GuardBandTop %f\r\n", caps.GuardBandTop);
				fprintf(fp, "GuardBandRight %f\r\n", caps.GuardBandRight);
				fprintf(fp, "GuardBandBottom %f\r\n", caps.GuardBandBottom);
				fprintf(fp, "ExtentsAdjust %f\r\n", caps.ExtentsAdjust);
				fprintf(fp, "StencilCaps %X\r\n", caps.StencilCaps);
				fprintf(fp, "FVFCaps %X\r\n", caps.FVFCaps);
				fprintf(fp, "TextureOpCaps %X\r\n", caps.TextureOpCaps);
				fprintf(fp, "MaxTextureBlendStages %X\r\n", caps.MaxTextureBlendStages);
				fprintf(fp, "MaxSimultaneousTextures %X\r\n", caps.MaxSimultaneousTextures);
				fprintf(fp, "VertexProcessingCaps %X\r\n", caps.VertexProcessingCaps);
				fprintf(fp, "MaxActiveLights %X\r\n", caps.MaxActiveLights);
				fprintf(fp, "MaxUserClipPlanes %X\r\n", caps.MaxUserClipPlanes);
				fprintf(fp, "MaxVertexBlendMatrices %X\r\n", caps.MaxVertexBlendMatrices);
				fprintf(fp, "MaxVertexBlendMatrixIndex %X\r\n", caps.MaxVertexBlendMatrixIndex);
				fprintf(fp, "MaxPointSize %f\r\n", caps.MaxPointSize);
				fprintf(fp, "MaxPrimitiveCount %X\r\n", caps.MaxPrimitiveCount);
				fprintf(fp, "MaxVertexIndex %X\r\n", caps.MaxVertexIndex);
				fprintf(fp, "MaxStreams %X\r\n", caps.MaxStreams);
				fprintf(fp, "MaxStreamStride %X\r\n", caps.MaxStreamStride);
				fprintf(fp, "VertexShaderVersion %X\r\n", caps.VertexShaderVersion);
				fprintf(fp, "MaxVertexShaderConst %X\r\n", caps.MaxVertexShaderConst);
				fprintf(fp, "PixelShaderVersion %X\r\n", caps.PixelShaderVersion);
#if (defined LW_USE_DX8)
				fprintf(fp, "MaxPixelShaderValue %f\r\n", caps.MaxPixelShaderValue);
#endif
			} catch (...) {
				fwrite("GetDeviceCaps error\r\n", strlen("GetDeviceCaps error\r\n") - 1, 1, fp);
			}

			fwrite(e.what(), strlen(e.what()) - 1, 1, fp);
			fclose(fp);
		}
		WinExec("system/errorreport.exe", SW_SHOW);
	}
	//catch( ... )
	//{
	//	MessageBox( NULL, RES_STRING(CL_LANGUAGE_MATCH_186), RES_STRING(CL_LANGUAGE_MATCH_185), 0 );
	//}

	return 0;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, HBRUSH brush) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_KOP);
	wcex.hCursor = nullptr;
	wcex.hbrBackground = (HBRUSH)brush; //(HBRUSH)(COLOR_BACKGROUND+1);
	wcex.lpszMenuName = (LPCTSTR)IDC_KOP;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow) {
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return nullptr;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

extern void __timer_period_render();

static WNDPROC g_wpOrigEditProc = nullptr;
// Subclass procedure
LRESULT APIENTRY EditSubclassProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam) {
	if (uMsg == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;

	switch (uMsg) {
	case WM_KEYDOWN:
		if (wParam == VK_UP || wParam == VK_DOWN) {
			WndProc(hwnd, uMsg, wParam, lParam);
			return 0;
		}
	case WM_CHAR:
	case WM_SYSKEYDOWN:
		WndProc(hwnd, uMsg, wParam, lParam);
		break;
	case WM_SYSKEYUP:
		if (wParam == VK_MENU || wParam == VK_F10) {
			return 0;
		}
		WndProc(hwnd, uMsg, wParam, lParam);
		break;
		//case WM_INPUTLANGCHANGEREQUEST:
		//case WM_INPUTLANGCHANGE:
		//case WM_IME_NOTIFY:
		//case WM_IME_STARTCOMPOSITION:
		//case WM_IME_ENDCOMPOSITION:
		//    {
		//        WndProc( hwnd, uMsg, wParam, lParam);
		//    }
		//    break;
		//case WM_IME_COMPOSITION:
		//case WM_IME_SETCONTEXT:
		//    {
		//        WndProc( hwnd, uMsg, wParam, lParam);
		//        if(g_Config.m_bFullScreen)
		//        {
		//            return 0;
		//        }
		//    }
		//    break;
	}
	return CallWindowProc(g_wpOrigEditProc, hwnd, uMsg, wParam, lParam);
}

HWND g_InputEdit = nullptr;
#include "inputbox.h"
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//switch (message)
	//{
	//case WM_ACTIVATE:
	//	// Add by lark.li 20080903 begin
	//	if(GameLoading::Init()->Active())
	//		return 0;
	//	// End
	//
	//	break;
	//case WM_NCACTIVATE:
	//case WM_ACTIVATEAPP:
	//	// Add by lark.li 20080903 begin
	//	if(GameLoading::Init()->Active())
	//		return 0;
	//	// End

	//	if( g_pGameApp && g_pGameApp->IsInit() )
	//		g_pGameApp->SetInputActive( LOWORD(wParam)!=WA_INACTIVE );
	//	break;
	//}

	if (g_pGameApp) {
		g_pGameApp->HandleWindowMsg(message, (DWORD)wParam, (DWORD)lParam);
		if (GetRegionMgr()->OnMessage(message, wParam, lParam))
			return 0;
	}

	switch (message) {
	case WM_USER_TIMER:
		__timer_period_render();
		return 0;
		break;

	case WM_ACTIVATE:
		// Add by lark.li 20080903 begin
		//if(GameLoading::Init()->Active())
		//	return 0;
		// End

		break;
	case WM_NCACTIVATE:
	case WM_ACTIVATEAPP:
		// Add by lark.li 20080903 begin
		//if(GameLoading::Init()->Active())
		//	return 0;
		// End

		if (g_pGameApp && g_pGameApp->IsInit())
			g_pGameApp->SetInputActive(LOWORD(wParam) != WA_INACTIVE);
		break;
	case WM_CREATE: {
		CenterWindow(hWnd);

		g_InputEdit = CreateWindow(TEXT("EDIT"),
								   TEXT(RES_STRING(CL_LANGUAGE_MATCH_192)),
								   WS_CHILD | WS_VISIBLE,
								   0, 0,
								   0, 0,
								   hWnd,
								   (HMENU)IDC_EDIT1,
								   ((LPCREATESTRUCT)lParam)->hInstance,
								   nullptr);

		g_wpOrigEditProc = (WNDPROC)SetWindowLongPtr(g_InputEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);

		extern CInputBox g_InputBox;
		g_InputBox.SetEditWindow(g_InputEdit);

		CImeInput::s_Ime.OnCreate(hWnd, hInst);
	} break;
	case WM_KEYDOWN:
	case WM_CHAR:
		break;
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP: {
		break;
	}
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	} break;
	case WM_PAINT:
		break;
	case WM_CLOSE:
		//{
		//	if( g_Config.m_bEditor )
		//		break;

		//	if( !g_NetIF->IsConnected() )
		//		break;

		//	if( !dynamic_cast<CWorldScene*>( CGameApp::GetCurScene() ) )
		//		break;

		//	CForm* f = CFormMgr::s_Mgr.Find( "frmAskExit" );
		//	if( f )
		//	{
		//		long lState = GetWindowLong(hWnd, GWL_STYLE);
		//		if( lState & WS_MINIMIZE )
		//		{
		//			ShowWindow(hWnd, SW_SHOWNORMAL);
		//		}

		//		f->SetIsShow(true);
		//		g_stUISystem.GetSystemForm()->SetIsShow(false);
		//		return 0;
		//	}
		//}
		break;
	case WM_QUIT:
		break;
	case WM_DESTROY: {
		CS_Logout();
		CS_Disconnect(DS_DISCONN);

		SetWindowLongPtr(g_InputEdit, GWLP_WNDPROC, (LONG_PTR)g_wpOrigEditProc);

		PostQuitMessage(0);
		if (g_pGameApp)
			g_pGameApp->SetIsRun(false);
		break;
	}
	case WM_MUSICEND: // 锟斤拷锟街诧拷锟斤拷系统
	{
		if (g_pGameApp)
			g_pGameApp->PlayMusic(0);
		break;
	}
	case WM_SYSKEYUP:
		if (wParam == VK_MENU || wParam == VK_F10) {
			return 0;
		}
		//case WM_IME_STARTCOMPOSITION:
		//case WM_IME_ENDCOMPOSITION:
		//case WM_IME_NOTIFY:
		//case WM_IME_COMPOSITION:
		//case WM_IME_SETCONTEXT:
		//case WM_INPUTLANGCHANGEREQUEST:
		//case WM_INPUTLANGCHANGE:
		//    {
		//        return 0;
		//    }
		//    break;

	case WM_ERASEBKGND:
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void MakeBinTable() {
	g_bBinaryTable = FALSE;

	/*TODO: Do we need this? 
	LGInfo lg_info;
	lg_info = *g_pGameApp->GetLGConfig();
	lg_info.bMsgBox = true;
	lg_info.bEnableAll = true;
	g_pGameApp->LG_Config(lg_info);
	*/

	// Compile tables
	Tables s(g_Config, false);

	/*
	//CHANGED(Snre3n):
	// See commit: Derived types of CRawDataSet might leak memory if not used properly
	MPResourceSet* pResourceSet = new MPResourceSet(0, g_Config.m_nMaxResourceNum);
	pResourceSet->LoadRawDataInfo("scripts/table/ResourceInfo", g_bBinaryTable);
	*/

	MessageBox(nullptr, RES_STRING(CL_LANGUAGE_MATCH_193), "Info", 0);
}

// 锟斤拷printf锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟角坝︼拷贸锟斤拷锟斤拷edit锟截硷拷锟斤拷
void DisplayError(const char* szMsg) {
	OutputDebugString(szMsg);
	//TRACE(szMsg);
	//TRACE(" ");
}

std::list<CCharacter*> g_Loading;

HANDLE hOutputReadTmp = nullptr;
HANDLE hOutputWrite = nullptr;
DWORD WINAPI ReadStdout(LPVOID lpvThreadParam) {
	//CHAR lpBuffer[256];
	//DWORD nBytesRead;

	//extern CMyIDEApp theApp;

	while (true) {
		if (!g_Loading.empty()) {
		} else {
			Sleep(1);
		}

		//  if (!ReadFile(hOutputReadTmp, lpBuffer,sizeof(lpBuffer),
		//                                   &nBytesRead,NULL) || !nBytesRead)
		//  {
		//     if (GetLastError() == ERROR_BROKEN_PIPE)
		//        break; // pipe done - normal exit path.
		//     else
		//        DisplayError("ReadFile"); // Something bad happened.
		//  }
		//
		//  if (nBytesRead >0 )
		//  {
		//      //锟斤拷取锟斤拷printf锟斤拷锟斤拷锟?
		//      lpBuffer[nBytesRead] = 0;
		////if(theApp.m_pMainWnd)
		////{
		////CMyIDEDlg *pDlg = (CMyIDEDlg*)theApp.m_pMainWnd; //锟斤拷Edit锟截硷拷写锟斤拷锟斤拷艿锟斤拷锟斤拷址锟斤拷锟斤拷锟斤拷
		////pDlg->m_edit1.ReplaceSel(lpBuffer);
		//   //}
		//if( g_pGameApp )
		//{
		// g_pGameApp->SysInfo( "lua printf:%s", lpBuffer );
		//}
		//  }
	}

	return 0;
}

#include <FCNTL.h>
void StdoutRedirect() {
	if (!CreatePipe(&hOutputReadTmp, &hOutputWrite, nullptr, 0))
		DisplayError("CreatePipe");

	int hCrt;
	FILE* hf;
	//AllocConsole();
	hCrt = _open_osfhandle((long)hOutputWrite, _O_TEXT);
	hf = _fdopen(hCrt, "w");
	*stdout = *hf;
	int i = setvbuf(stdout, nullptr, _IONBF, 0);

	// Launch the thread that gets the input and sends it to the child.
	DWORD ThreadID;
	HANDLE hThread = ::CreateThread(nullptr, 0, ReadStdout, //锟斤拷锟斤拷锟竭筹拷
									nullptr, 0, &ThreadID);
}

// 锟斤拷装锟斤拷锟斤拷
int InstallFont(const char* pszPath) {
	int nRet = 0;
	char szWindow[256] = {0};
	char szBuffer[256] = {0};

	// 锟叫讹拷锟角凤拷锟窖撅拷锟斤拷装锟斤拷锟斤拷锟斤拷锟藉，锟斤拷锟矫伙拷邪锟阶帮拷锟斤拷远锟斤拷锟阶?
	GetWindowsDirectory(szWindow, sizeof(szWindow) / sizeof(szWindow[0])); // 锟斤拷取Windows目录锟斤拷锟斤拷锟斤拷路锟斤拷锟斤拷
	_snprintf_s(szBuffer, _TRUNCATE, "%s\\fonts\\simsun.ttc", szWindow);
	if (-1 != access(szBuffer, 0)) {
		return nRet;
	} else {
		_snprintf_s(szBuffer, _TRUNCATE, "%s\\simsun.ttc", pszPath);
		nRet += AddFontResource(szBuffer);
	}

	//WIN32_FIND_DATA oFinder;

	//_snprintf_s(szBuffer, _TRUNCATE, "%s%s", pszPath, "*.*");
	//HANDLE hFind = FindFirstFile(szBuffer, &oFinder);

	//if(hFind == INVALID_HANDLE_VALUE)
	//{
	//	return 0;
	//}

	//// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥柯?
	//while(FindNextFile(hFind, &oFinder) != 0)
	//{
	//	if(oFinder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	//	{
	//		continue;
	//	}

	//	_snprintf_s(szBuffer, _TRUNCATE, "%s%s", pszPath, oFinder.cFileName);
	//	nRet += AddFontResource(szBuffer);	//AddFontResourceEx(szBuffer, FR_NOT_ENUM, 0);
	//}

	//FindClose(hFind);

	if (nRet) {
		SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0); // 锟姐播通知系统
	}

	return nRet;
}

void CenterWindow(HWND hWnd) // 锟斤拷锟斤拷锟斤拷锟斤拷幕锟斤拷示位锟斤拷锟斤拷锟斤拷
{
	RECT rc;
	GetWindowRect(hWnd, &rc); // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷幕锟叫碉拷锟斤拷锟斤拷

	int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;   // 锟斤拷锟斤拷锟斤拷呓锟?
	int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;   // 锟斤拷锟斤拷锟竭斤拷
	MoveWindow(hWnd, x, y, rc.right - rc.left, rc.bottom - rc.top, TRUE); // 指锟斤拷锟斤拷锟节碉拷位锟矫和尺达拷
}
