#include "stdafx.h"

#ifdef _LUA_GAME

#include "resource.h"
#include "lua_platform.h"

lua_State* L = nullptr;
HWND g_dlgScript = nullptr;
std::list<std::string> g_luaFNList;

int lua_sysAddHelpString(lua_State* L) {
	const char* pszHelp = lua_tostring(L, 1);
	g_luaFNList.emplace_back(pszHelp);
	return 0;
}

void InitLuaPlatform() {
	LG("init", RES_STRING(CL_LANGUAGE_MATCH_182));

	L = lua_open();
#ifdef LUA50_COMPAT
	lua_baselibopen(L);
	lua_iolibopen(L);
	lua_strlibopen(L);
	lua_tablibopen(L);
	lua_mathlibopen(L);
#else
	luaL_openlibs(L);
#endif

#define REGFN(fn) (lua_pushstring(L, "" #fn ""),      \
				   lua_pushcfunction(L, lua_##fn),    \
				   lua_settable(L, LUA_GLOBALSINDEX), \
				   g_luaFNList.push_back("" #fn ""))

	lua_register(L, "sysAddHelpString", lua_sysAddHelpString);

	// util对象操作函数
	g_luaFNList.emplace_back("[util]");
	REGFN(MsgBox);
	REGFN(GetTickCount);
	REGFN(Rand);
	REGFN(LG);
	REGFN(SysInfo);
	lua_register(L, "GetResString", lua_GetResString);
	REGFN(GetResString); //Add by lark.li 20080411
	g_luaFNList.emplace_back("\r");

	// app对象操作函数
	g_luaFNList.emplace_back("[app]");
	REGFN(appGetCurScene);
	REGFN(appSetCaption);
	REGFN(appPlaySound);
	REGFN(appUpdateRender);
#ifdef LUA50_COMPAT
	lua_dofile(L, "scripts/gamesdk/app.lua");
#else
#ifdef CALUA_COMPAT
	luaL_dofile(L, "scripts/gamesdk/app.lua");
#else
	luaL_handled_dofile(L, "scripts/gamesdk/app.lua");
#endif
#endif
	g_luaFNList.emplace_back("\r");

	// scene对象操作函数
	g_luaFNList.emplace_back("[scene]");
	REGFN(sceAddObj);
	REGFN(sceRemoveObj);
	REGFN(sceGetObj);
	REGFN(sceSetMainCha);
	REGFN(sceGetMainCha);
	REGFN(sceGetHoverCha);
	REGFN(sceEnableDefaultMouse);
#ifdef LUA50_COMPAT
	lua_dofile(L, "scripts/gamesdk/scene.lua");
#else
#ifdef CALUA_COMPAT
	luaL_dofile(L, "scripts/gamesdk/scene.lua");
#else
	luaL_handled_dofile(L, "scripts/gamesdk/scene.lua");
#endif
#endif
	g_luaFNList.emplace_back("\r");

	// object对象操作函数
	g_luaFNList.emplace_back("[object]");
	REGFN(objSetPos);
	REGFN(objGetPos);
	REGFN(objSetFaceAngle);
	REGFN(objGetFaceAngle);
	REGFN(objSetAttr);
	REGFN(objGetAttr);
	REGFN(objIsValid);
	REGFN(objGetID);
	REGFN(chaMoveTo);
	REGFN(chaSay);
	REGFN(chaChangePart);
	REGFN(chaPlayPose);
	REGFN(chaStop);
#ifdef LUA50_COMPAT
	lua_dofile(L, "scripts/gamesdk/object.lua");
#else
#ifdef CALUA_COMPAT
	luaL_dofile(L, "scripts/gamesdk/object.lua");
#else
	luaL_handled_dofile(L, "scripts/gamesdk/object.lua");
#endif
#endif
	g_luaFNList.emplace_back("\r");

	// 镜头对象操作函数
	g_luaFNList.emplace_back("[camera]");
	REGFN(camGetCenter);
	REGFN(camSetCenter);
	REGFN(camFollow);
	REGFN(camMoveForward);
	REGFN(camMoveLeft);
	REGFN(camMoveUp);
	REGFN(camSetAngle);
	g_luaFNList.emplace_back("\r");

	// Input对象操作函数
	g_luaFNList.emplace_back("[input]");
	REGFN(IsKeyDown);
#ifdef LUA50_COMPAT
	lua_dofile(L, "scripts/gamesdk/input.lua");
#else
#ifdef CALUA_COMPAT
	luaL_dofile(L, "scripts/gamesdk/input.lua");
#else
	luaL_handled_dofile(L, "scripts/gamesdk/input.lua");
#endif
#endif
	g_luaFNList.emplace_back("\r");

	// UI对象操作函数
	g_luaFNList.emplace_back("[UI]");
	REGFN(uiHideAll);
	//#ifdef CALUA_COMPAT
	// lua_dofile(L, "scripts/gamesdk/input.lua");
	//#else
	// luaL_handled_dofile(L, "scripts/gamesdk/input.lua");
	//#endif
	g_luaFNList.emplace_back("\r");
}

INT_PTR CALLBACK ScriptDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG: {
		HWND hEdit1 = GetDlgItem(hwndDlg, IDC_EDIT1);
		//extern CInputBox g_InputBox;
		//g_InputBox.SetEditWindow(hEdit1);

		HWND hEdit = GetDlgItem(hwndDlg, IDC_EDIT2);
		for (auto& it : g_luaFNList) {
			SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)it.c_str());
			SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM) "\r\n");
		}
		ShowWindow(hEdit, SW_HIDE);
		break;
	}
	case WM_KEYDOWN: {
		break;
	}
	case WM_COMMAND: {
		HWND hEdit1 = GetDlgItem(hwndDlg, IDC_EDIT1);
		HWND hEdit2 = GetDlgItem(hwndDlg, IDC_EDIT2);
		HWND hButHelp = GetDlgItem(hwndDlg, IDFNLIST);

		switch (wParam) {
		case IDOK: {
			FILE* fp;
			fopen_s(&fp, "tmp.txt", "wt");
			if (!fp)
				break;
			char szText[8192];
			int n = GetWindowText(hEdit1, szText, 8192);
			fwrite(szText, n, 1, fp);
			fclose(fp);
#ifdef LUA50_COMPAT
			lua_dofile(L, "tmp.txt");
#else
#ifdef CALUA_COMPAT
			luaL_dofile(L, "tmp.txt");
#else
			luaL_handled_dofile(L, "tmp.txt");
#endif
#endif
			break;
		}
		case IDFNLIST: {
			if (SendMessage(hButHelp, BM_GETCHECK, 0, 0) == BST_CHECKED) {
				ShowWindow(hEdit1, SW_HIDE);
				ShowWindow(hEdit2, SW_SHOW);
			} else {
				ShowWindow(hEdit2, SW_HIDE);
				ShowWindow(hEdit1, SW_SHOW);
			}
			break;
		}
		case IDCLEAR: {
			SetWindowText(hEdit1, "");
			break;
		}
		case IDCANCEL: {
			ShowWindow(hwndDlg, SW_HIDE);
		}
		}
		break;
	}
	}
	return FALSE;
}

void CreateScriptDebugWindow(HINSTANCE hInst, HWND hParent) {
	g_dlgScript = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DLG_SCRIPT), hParent, ScriptDialogProc);
	if (g_Config.m_bEditor) {
		ShowWindow(g_dlgScript, SW_SHOW);
	} else {
		ShowWindow(g_dlgScript, SW_HIDE);
	}
	RECT rc;
	GetWindowRect(hParent, &rc);
	SetWindowPos(g_dlgScript, nullptr, rc.right - 300, rc.bottom - 300, 0, 0, SWP_NOSIZE);
}

void ToggleScriptDebugWindow() {
	if (IsWindowVisible(g_dlgScript)) {
		ShowWindow(g_dlgScript, SW_HIDE);
	} else {
		ShowWindow(g_dlgScript, SW_SHOW);
	}
}

#endif
