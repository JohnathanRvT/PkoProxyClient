#include "stdafx.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#include "lualib.h"
#include "lauxlib.h"
#else
#include <lua.hpp>
#endif

#include "lua_platform.h" //Add by lark.li 200804111
#include "UISystemForm.h"
//#pragma comment (lib, "lua50.lib")
//#pragma comment (lib, "lualib.lib")

constexpr size_t DEFAULT_SCRIPT_NUM{1024};

DWORD CScript::_dwCount = DEFAULT_SCRIPT_NUM;
DWORD CScript::_dwFreeCount = DEFAULT_SCRIPT_NUM;
DWORD CScript::_dwLastFree = 0;

CScript** CScript::_AllObj = nullptr;

//---------------------------------------------------------------------------
// CScript
//---------------------------------------------------------------------------
bool CScript::Init() {
	_dwCount = DEFAULT_SCRIPT_NUM;
	_dwFreeCount = DEFAULT_SCRIPT_NUM;
	_dwLastFree = 0;

	_AllObj = new CScript*[_dwCount];
	memset(CScript::_AllObj, 0, _dwCount * sizeof(CScript*));
	return true;
}

bool CScript::Clear() {
	delete[] _AllObj;

	_AllObj = nullptr;
	_dwCount = 0;
	_dwFreeCount = 0;
	_dwLastFree = 0;
	return true;
}

CScript::CScript() {
	if (_dwFreeCount <= 0) {
		_dwLastFree = _dwCount + 1;
		_dwCount += DEFAULT_SCRIPT_NUM;
		_dwFreeCount = DEFAULT_SCRIPT_NUM;

		CScript** tmp = _AllObj;

		_AllObj = new CScript*[_dwCount];
		memset(_AllObj, 0, _dwCount * sizeof(CScript*));
		memcpy(_AllObj, tmp, (_dwCount - DEFAULT_SCRIPT_NUM) * sizeof(CScript*));
		delete[] tmp;
	}

	if (!_AllObj[_dwLastFree]) {
		_AllObj[_dwLastFree] = this;
		_dwScriptID = _dwLastFree;

		--_dwFreeCount;
		++_dwLastFree;
		if (_dwLastFree >= _dwCount)
			_dwLastFree = 0;
		return;
	}

	// 寻找当前队列是否有空位,如果没有空位,增加到末尾
	for (DWORD i = _dwLastFree + 1; i < _dwCount; ++i) {
		if (!_AllObj[i]) {
			_AllObj[i] = this;
			_dwScriptID = i;

			--_dwFreeCount;
			return;
		}
	}

	for (int i = _dwLastFree - 1; i >= 0; --i) {
		if (!_AllObj[i]) {
			_AllObj[i] = this;
			_dwScriptID = i;

			--_dwFreeCount;
			return;
		}
	}

	LG("error", "msgCScript::CScript Error, dwCount: %d, dwFreeCount: %d, dwLastFree: %d", _dwCount, _dwFreeCount, _dwLastFree);
}

CScript::~CScript() {
	if (_dwScriptID > _dwCount)
		LG("error", "msgCScript::~CScript Error, dwCount: %d, dwFreeCount: %d, dwLastFree: %d", _dwCount, _dwFreeCount, _dwLastFree);

	_AllObj[_dwScriptID] = nullptr;

	_dwLastFree = _dwScriptID;
	++_dwFreeCount;
}

//---------------------------------------------------------------------------
// CScriptMgr
//---------------------------------------------------------------------------
#ifdef CALUA_COMPAT
lua_State* _pLuaState = NULL; //CHANGED: Using L instead
#endif
static FILE* _pStdErr = nullptr;

CScriptMgr::CScriptMgr() {
}

CScriptMgr::~CScriptMgr() {
	Clear();
}

#ifdef CALUA_COMPAT
bool CScriptMgr::Init() {
	if (!CScript::Init())
		return false;

	_pStdErr = freopen("lua_err.txt", "w", stderr);

	int CLU_State = CLU_Init();
	CLU_LoadState(CLU_State);

	extern void MPInitLua_Scene();
	extern void MPInitLua_Gui();
	extern void MPInitLua_Cha();
	extern void MPInitLua_App();
	MPInitLua_Gui();
	MPInitLua_App();
	MPInitLua_Scene(); //CHANGED: Define functions shared between different resolutions
	if (g_stUISystem.m_sysProp.m_videoProp.bPixel1024) {
		CLU_LoadScript("scripts/lua/CameraConf1024.clu");
		g_pGameApp->GetMainCam()->MAX_SCALE = 1.30f;
	} else {
		CLU_LoadScript("scripts/lua/CameraConf.clu");
		g_pGameApp->GetMainCam()->MAX_SCALE = 1.0f;
	}
	MPInitLua_Cha();

	// Add by lark.li 20080415
	CLU_LoadScript("scripts/lua/res.clu");

	//CHANGED: These scripts are not neeeded (as of now).
	CLU_LoadScript("scripts/lua/scene.clu");
	CLU_LoadScript("scripts/lua/scene/face.clu");

	CLU_LoadScript("scripts/lua/CharacterConf.clu");

	// Modify by lark.li 20080411 begin
	//char type[6] = "char*";
	//int ret = CLU_RegisterFunction("GetResString", type, "char*", CLU_CDECL, CLU_CAST(Lua_GetResString));
	CLU_LoadScript("scripts/lua/mission/mission.lua");
	CLU_LoadScript("scripts/lua/mission/missioninfo.lua");
	// End

	// Add by rock.wang 2009.04.27 begin
	CLU_LoadScript("scripts/lua/scene/previouname.clu");
	// End

	return true;
}

bool CScriptMgr::LoadScript() {
	//#ifdef CALUA_COMPAT
	//_pLuaState = lua_open();
	//if( !_pLuaState )
	//{
	//	LG( "lua", "msglua_open error!" );
	//	return false;
	//}
	//   lua_baselibopen (_pLuaState);
	//   lua_iolibopen (_pLuaState);
	//   lua_strlibopen (_pLuaState);
	//   lua_tablibopen(_pLuaState);
	//   lua_mathlibopen (_pLuaState);
	//#else
	//L = lua_open();
	//if( !L )
	//{
	//	LG( "lua", "msglua_open error!" );
	//	return false;
	//}
	//	luaL_openlibs(L);
	//#endif

	extern lua_State* L;
	_pLuaState = L;

#ifdef LUA50_COMPAT
	lua_dofile(_pLuaState, "scripts/lua/table/scripts.lua");
#else
#ifdef CALUA_COMPAT
	luaL_dofile(_pLuaState, "scripts/lua/table/scripts.lua");
#else
	luaL_handled_dofile(L, "scripts/lua/table/scripts.lua");
#endif
#endif
	return true;
}

bool CScriptMgr::Clear() {
	//if( _pLuaState )
	//{
	//	lua_close(_pLuaState);
	//	_pLuaState = NULL;
	//}

	//if( _pStdErr )
	//{
	//	fclose( _pStdErr );
	//	_pStdErr = NULL;
	//}

	if (!CScript::Clear())
		return false;

	return true;
}
#else

//ADDED (by deguix): Functions to prevent file fragmentation.
bool EXTRA_IsEditor() {
	if (g_Config.m_bEditor)
		return true; //TODO: get a way to detect editor
	return false;
}

bool EXTRA_IsRes1024() {
	if (g_stUISystem.m_sysProp.m_videoProp.bPixel1024)
		return true; //TODO: get a way to detect resolution
	return false;
}

bool EXTRA_LoadScreen() {
	GetRender().SetScreen(g_Render.GetScrWidth(), g_Render.GetScrHeight(), (g_Render.IsFullScreen() != 0 ? true : false));
	return false;
}

LUA_FUNC_ARITY0(EXTRA_IsEditor,
				boolean,
				bool);

LUA_FUNC_ARITY0(EXTRA_IsRes1024,
				boolean,
				bool);

LUA_FUNC_ARITY0(EXTRA_LoadScreen,
				boolean,
				bool);

bool CScriptMgr::Init() {
	if (!CScript::Init())
		return false;

	//	freopen_s( &_pStdErr,"lua_err.txt", "w", stderr );
	//	if ( !_pStdErr )
	//	{
	//		return FALSE;
	//	}

	//MAJOR CHANGED: Make it use luajit instead. Remake all functions into lua_ functions.
	L = lua_open();
	if (!L) {
		LG("lua", "msglua_open error!");
		return false;
	}
	luaL_openlibs(L);

	extern void MPInitLua_Scene(lua_State*);
	extern void MPInitLua_Gui(lua_State*);
	extern void MPInitLua_Cha(lua_State*);
	extern void MPInitLua_App(lua_State*);
	MPInitLua_Gui(L);
	MPInitLua_App(L);
	MPInitLua_Scene(L);
	if (g_stUISystem.m_sysProp.m_videoProp.bPixel1024) {
		//MPInitLua_Scene1024(L); //CHANGED: Merged into MPInitLua_Scene.
		g_pGameApp->GetMainCam()->MAX_SCALE = 1.30f;
	} else
		g_pGameApp->GetMainCam()->MAX_SCALE = 1.0f;
	MPInitLua_Cha(L);

	//ADDED (by deguix): Functions to prevent file fragmentation.
	LUA_FUNC_REG(EXTRA_IsEditor);
	LUA_FUNC_REG(EXTRA_IsRes1024);
	LUA_FUNC_REG(EXTRA_LoadScreen);

	//CHANGED: Copied function registration from lua_platform.cpp
	LUA_FUNC_REG(MsgBox);
	LUA_FUNC_REG(GetTickCount);
	LUA_FUNC_REG(Rand);
	LUA_FUNC_REG(LG);
	LUA_FUNC_REG(SysInfo);
	LUA_FUNC_REG(GetResString); //Add by lark.li 20080411
	LUA_FUNC_REG(appGetCurScene);
	LUA_FUNC_REG(appSetCaption);
	LUA_FUNC_REG(appPlaySound);
	LUA_FUNC_REG(appUpdateRender);
	LUA_FUNC_REG(sceAddObj);
	LUA_FUNC_REG(sceRemoveObj);
	LUA_FUNC_REG(sceGetObj);
	LUA_FUNC_REG(sceSetMainCha);
	LUA_FUNC_REG(sceGetMainCha);
	LUA_FUNC_REG(sceGetHoverCha);
	LUA_FUNC_REG(sceEnableDefaultMouse);
	LUA_FUNC_REG(objSetPos);
	LUA_FUNC_REG(objGetPos);
	LUA_FUNC_REG(objSetFaceAngle);
	LUA_FUNC_REG(objGetFaceAngle);
	LUA_FUNC_REG(objSetAttr);
	LUA_FUNC_REG(objGetAttr);
	LUA_FUNC_REG(objIsValid);
	LUA_FUNC_REG(objGetID);
	LUA_FUNC_REG(chaMoveTo);
	LUA_FUNC_REG(chaSay);
	LUA_FUNC_REG(chaChangePart);
	LUA_FUNC_REG(chaPlayPose);
	LUA_FUNC_REG(chaStop);
	LUA_FUNC_REG(camGetCenter);
	LUA_FUNC_REG(camSetCenter);
	LUA_FUNC_REG(camFollow);
	LUA_FUNC_REG(camMoveForward);
	LUA_FUNC_REG(camMoveLeft);
	LUA_FUNC_REG(camMoveUp);
	LUA_FUNC_REG(camSetAngle);
	LUA_FUNC_REG(IsKeyDown);
	LUA_FUNC_REG(uiHideAll);

	//CHANGED: Load all scripts after all initialization only.
	//luaL_handled_dofile(L, "scripts/lua/initial.lua"); //NOTE: Attempted to initialize only here, but gui items need to be init later.
	//luaL_handled_dofile(L, "scripts/lua/CameraConf.clu");

	// Add by lark.li 20080415
	//luaL_handled_dofile(L, "scripts/lua/res.clu");

	//CHANGED: These scripts are not neeeded (as of now).
	//luaL_handled_dofile(L, "scripts/lua/scene.clu");
	//luaL_handled_dofile(L, "scripts/lua/scene/face.clu");

	//luaL_handled_dofile(L, "scripts/lua/CharacterConf.clu");

	// Modify by lark.li 20080411 begin
	//char type[6] = "char*";
	//int ret = CLU_RegisterFunction("GetResString", type, "char*", CLU_CDECL, CLU_CAST(Lua_GetResString));
	//luaL_handled_dofile(L, "scripts/lua/mission/mission.lua");
	//luaL_handled_dofile(L, "scripts/lua/mission/missioninfo.lua");
	// End

	// Add by rock.wang 2009.04.27 begin
	//luaL_handled_dofile(L, "scripts/lua/scene/previouname.clu");
	// End

	return true;
}

bool CScriptMgr::LoadScript() {
	//luaL_handled_dofile(L, "scripts/lua/init.lua"); //CHANGED: Make all scripts be loaded from this file - To be implemented
	return true;
}

bool CScriptMgr::Clear() {
	//CHANGED: uncommented this part
	if (L) {
		//lua_close(L);
		L = nullptr;
	}

	//if( _pStdErr )
	//{
	//	fclose( _pStdErr );
	//	_pStdErr = NULL;
	//}

	if (!CScript::Clear())
		return false;

	return true;
}
#endif

bool CScriptMgr::DoFile(const char* szLuaFile) {
	LG("lua", "DoFile(%s)\n", szLuaFile);
#ifdef LUA50_COMPAT
	return lua_dofile(_pLuaState, szLuaFile) != 0;
#else
#ifdef CALUA_COMPAT
	return luaL_dofile(_pLuaState, szLuaFile) != 0;
#else
	return luaL_handled_dofile(L, szLuaFile) != 0;
#endif
#endif
}

bool CScriptMgr::DoString(const char* szLuaString) {
	LG("lua", "DoString(%s)\n", szLuaString);
	FILE* fp;
	fopen_s(&fp, "luaexec.txt", "wt");
	if (fp == nullptr)
		return false;
	fwrite(szLuaString, strlen(szLuaString), 1, fp);
	fclose(fp);
#ifdef LUA50_COMPAT
	return lua_dofile(_pLuaState, "luaexec.tmp") != 0;
#else
#ifdef CALUA_COMPAT
	return luaL_dofile(_pLuaState, "luaexec.tmp") != 0;
#else
	return luaL_handled_dofile(L, "luaexec.tmp") != 0;
#endif
#endif
}

#ifdef CALUA_COMPAT
bool CScriptMgr::DoString(const char* szFunc, const char* szFormat, ...) {
	const double value = 1081000000.0;
	double dd = value / 1000.0 * 1000.0;
	if (dd != value) {
		_control87(_CW_DEFAULT, 0xfffff);
		LG("float", RES_STRING(CL_LANGUAGE_MATCH_380), szFunc, szFormat);
	}

	int narg, nres; // 参数及返回值个数

	va_list vl;
	va_start(vl, szFormat);
	lua_getglobal(_pLuaState, szFunc);
	if (!lua_isfunction(_pLuaState, -1)) // 不是函数名
	{
		lua_settop(_pLuaState, 0);
		LG("luaerror", "Func is Error, Func:%s, Fromat:%s\n", szFunc, szFormat);
		return false;
	}

	narg = 0;
	while (*szFormat) {
		switch (*szFormat++) {
		case 'f':
			lua_pushnumber(_pLuaState, va_arg(vl, double));
			break;
		case 'd':
			lua_pushnumber(_pLuaState, va_arg(vl, int));
			break;
		case 'u':
			lua_pushnumber(_pLuaState, va_arg(vl, unsigned int));
			break;
		case 's':
			lua_pushstring(_pLuaState, va_arg(vl, char*));
			break;
		case '-':
			goto endwhile;
		default:
			lua_settop(_pLuaState, 0);
			LG("luaerror", "Param Error, Func:%s, Fromat:%s\n", szFunc, szFormat);
			return false;
		}
		narg++;
		luaL_checkstack(_pLuaState, 1, "too many arguments");
	}

endwhile:

	nres = (int)strlen(szFormat);
	if (lua_handled_pcall(_pLuaState, narg, nres, 0) != 0) {
		LG("luaerror", "Func call is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
		return false;
	}

	nres = -nres;
	while (*szFormat) {
		switch (*szFormat++) {
		case 'f':
			if (!lua_isnumber(_pLuaState, nres)) {
				lua_settop(_pLuaState, 0);
				LG("luaerror", "return value(f) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, double*) = (double)lua_tonumber(_pLuaState, nres);
			break;
		case 'd':
			if (!lua_isnumber(_pLuaState, nres)) {
				lua_settop(_pLuaState, 0);
				LG("luaerror", "return value(d) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, int*) = (int)lua_tonumber(_pLuaState, nres);
			break;
		case 'u':
			if (!lua_isnumber(_pLuaState, nres)) {
				lua_settop(_pLuaState, 0);
				LG("luaerror", "return value(u) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, unsigned int*) = (unsigned int)lua_tonumber(_pLuaState, nres);
			break;
		case 's':
			if (!lua_isstring(_pLuaState, nres)) {
				lua_settop(_pLuaState, 0);
				LG("luaerror", "return value(s) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, std::string*) = lua_tostring(_pLuaState, nres);
			break;
		default:
			lua_settop(_pLuaState, 0);
			LG("luaerror", "return value(?) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
			return false;
		}
		nres++;
	}
	va_end(vl);
	lua_settop(_pLuaState, 0);
	return true;
}

std::string CScriptMgr::GetStoneHint(const char* szHintFun, int Lv) {
	const double value = 1081000000.0;
	double dd = value / 1000.0 * 1000.0;
	if (dd != value) {
		_control87(_CW_DEFAULT, 0xfffff);
		LG("float", RES_STRING(CL_LANGUAGE_MATCH_381), szHintFun, Lv);
	}

	lua_getglobal(_pLuaState, szHintFun);
	if (!lua_isfunction(_pLuaState, -1)) // 不是函数名
	{
		lua_pop(_pLuaState, 1);
		return RES_STRING(CL_LANGUAGE_MATCH_382);
	}

	int nParamNum = 0;
	lua_pushnumber(_pLuaState, Lv);
	nParamNum = 1;
	if (lua_handled_pcall(_pLuaState, nParamNum, LUA_MULTRET, 0) != 0)
		LG("lua_err", "DoString %s\n", szHintFun);
	return "lua_pcall error";
}

std::string hint;
int nRetNum = 1;
if (!lua_isstring(_pLuaState, -1)) {
	LG("error", RES_STRING(CL_LANGUAGE_MATCH_383));
} else {
	hint = lua_tostring(_pLuaState, -1);
}
lua_pop(_pLuaState, nRetNum);
return hint;
}
#else
bool CScriptMgr::DoString(const char* szFunc, const char* szFormat, ...) {
	const double value = 1081000000.0;
	double dd = value / 1000.0 * 1000.0;
	if (dd != value) {
		_control87(_CW_DEFAULT, 0xfffff);
		LG("float", RES_STRING(CL_LANGUAGE_MATCH_380), szFunc, szFormat);
	}

	int narg, nres; // 参数及返回值个数

	va_list vl;
	va_start(vl, szFormat);
	lua_getglobal(L, szFunc);
	if (!lua_isfunction(L, -1)) // 不是函数名
	{
		lua_settop(L, 0);
		LG("luaerror", "Func is Error, Func:%s, Fromat:%s\n", szFunc, szFormat);
		return false;
	}

	narg = 0;
	while (*szFormat) {
		switch (*szFormat++) {
		case 'f':
			lua_pushnumber(L, va_arg(vl, double));
			break;
		case 'd':
			lua_pushnumber(L, va_arg(vl, int));
			break;
		case 'u':
			lua_pushnumber(L, va_arg(vl, unsigned int));
			break;
		case 's':
			lua_pushstring(L, va_arg(vl, char*));
			break;
		case '-':
			goto endwhile;
		default:
			lua_settop(L, 0);
			LG("luaerror", "Param Error, Func:%s, Fromat:%s\n", szFunc, szFormat);
			return false;
		}
		narg++;
		luaL_checkstack(L, 1, "too many arguments");
	}

endwhile:

	nres = (int)strlen(szFormat);
	if (lua_handled_pcall(L, narg, nres, 0) != 0)
		return false;

	nres = -nres;
	while (*szFormat) {
		switch (*szFormat++) {
		case 'f':
			if (!lua_isnumber(L, nres)) {
				lua_settop(L, 0);
				LG("luaerror", "return value(f) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, double*) = (double)lua_tonumber(L, nres);
			break;
		case 'd':
			if (!lua_isnumber(L, nres)) {
				lua_settop(L, 0);
				LG("luaerror", "return value(d) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, int*) = (int)lua_tonumber(L, nres);
			break;
		case 'u':
			if (!lua_isnumber(L, nres)) {
				lua_settop(L, 0);
				LG("luaerror", "return value(u) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, unsigned int*) = (unsigned int)lua_tonumber(L, nres);
			break;
		case 's':
			if (!lua_isstring(L, nres)) {
				lua_settop(L, 0);
				LG("luaerror", "return value(s) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
				return false;
			}

			*va_arg(vl, std::string*) = lua_tostring(L, nres);
			break;
		default:
			lua_settop(L, 0);
			LG("luaerror", "return value(?) is error, Func:%s, Fromat:%s\n", szFunc, szFormat);
			return false;
		}
		nres++;
	}
	va_end(vl);
	lua_settop(L, 0);
	return true;
}

std::string CScriptMgr::GetStoneHint(const char* szHintFun, int Lv) {
	const double value = 1081000000.0;
	double dd = value / 1000.0 * 1000.0;
	if (dd != value) {
		_control87(_CW_DEFAULT, 0xfffff);
		LG("float", RES_STRING(CL_LANGUAGE_MATCH_381), szHintFun, Lv);
	}

	lua_getglobal(L, szHintFun);
	if (!lua_isfunction(L, -1)) // 不是函数名
	{
		lua_pop(L, 1);
		return RES_STRING(CL_LANGUAGE_MATCH_382);
	}

	int nParamNum = 0;
	lua_pushnumber(L, Lv);
	nParamNum = 1;
	if (lua_handled_pcall(L, nParamNum, LUA_MULTRET, 0) != 0)
		return "lua_pcall error";

	std::string hint;
	int nRetNum = 1;
	if (!lua_isstring(L, -1)) {
		LG("error", RES_STRING(CL_LANGUAGE_MATCH_383));
	} else {
		hint = lua_tostring(L, -1);
	}
	lua_pop(L, nRetNum);
	return hint;
}
#endif