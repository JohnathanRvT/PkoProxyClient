#ifdef _LUA_GAME
#ifndef LUA_PLATFORM_IMPLEMENTED
#define LUA_PLATFORM_IMPLEMENTED
#ifdef CALUA_COMPAT
extern "C" {
#include "lua.h"
#include "lualib.h"
#include <lauxlib.h>
#include <lualib.h>
}
#else
#include <lua.hpp>
#endif

// lua函数返回值定义
#define LUA_FALSE 0 // 返回错误
#define LUA_TRUE 1  // 返回正确

// 出错调试输出
#define PARAM_ERROR                                                 \
	{                                                               \
		LG("lua", RES_STRING(CL_LANGUAGE_MATCH_183), __FUNCTION__); \
	}
#define SCENE_NULL_ERROR                                            \
	{                                                               \
		LG("lua", RES_STRING(CL_LANGUAGE_MATCH_184), __FUNCTION__); \
	}

extern void InitLuaPlatform();
extern void CreateScriptDebugWindow(HINSTANCE hInst, HWND hParent);
extern void ToggleScriptDebugWindow();

extern lua_State* L;

inline void lua_dostringI(const char* pszCmd) {
#ifdef LUA50_COMPAT
	lua_dostring(L, pszCmd);
#else
	luaL_dostring(L, pszCmd);
#endif
	//luaL_loadbuffer(L, pszCmd, nCmdSize, "line");
	//lua_pcall(L, 0, 0, 0);
}

inline void lua_dostringI(string str) {
#ifdef LUA50_COMPAT
	lua_dostring(L, str.c_str()); //  (int)str.size());
#else
	luaL_dostring(L, str.c_str()); //  (int)str.size());
#endif
}

inline void lua_platform_framemove() {
	//#ifdef CALUA_COMPAT
	// lua_dostring(L, "appRunTimer()");
	//#else
	// luaL_dostring(L, "appRunTimer()");
	//#endif
}

inline void lua_platform_keydown(DWORD dwKey) {
	//char szRun[128]; _snprintf_s(szRun, _TRUNCATE, "if key_event[%d] then key_event[%d]() end", dwKey, dwKey);
	//#ifdef CALUA_COMPAT
	//lua_dostring(L, szRun);
	//#else
	//luaL_dostring(L, szRun);
	//#endif
}

inline void lua_platform_mousedown(DWORD dwButton) {
	//char szRun[128]; _snprintf_s(szRun, _TRUNCATE, "if mouse_event[%d] then mouse_event[%d]() end", dwButton, dwButton);
	//#ifdef CALUA_COMPAT
	//lua_dostring(L, szRun);
	//#else
	//luaL_dostring(L, szRun);
	//#endif
}

//CHANGED: Remove CaLua dependency by defining lua_ functions of regular CaLua functions.

#define LUA_FUNC_REG(name) lua_register(L, #name, lua_##name) //NOTE: L is implicit.

#ifndef CALUA_COMPAT
#define LUA_FUNC_NIL_HANDLING(value) \
	if (temp == value)               \
		lua_pushnil(L);              \
	else

#define LUA_FUNC_ARITY0(name, retluatype, rettype, code) \
	inline int lua_##name(lua_State* L) {                \
		rettype temp = (rettype)name();                  \
		code                                             \
			lua_push##retluatype(L, temp);               \
		return 1;                                        \
	}

#define LUA_FUNC_ARITY1(name, retluatype, luatype1, rettype, type1, code) \
	inline int lua_##name(lua_State* L) {                                 \
		if (!(lua_gettop(L) == 1 && lua_is##luatype1(L, 1)))              \
			PARAM_ERROR;                                                  \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1));      \
		code                                                              \
			lua_push##retluatype(L, temp);                                \
		return 1;                                                         \
	}

#define LUA_FUNC_ARITY2(name, retluatype, luatype1, luatype2, rettype, type1, type2, code)          \
	inline int lua_##name(lua_State* L) {                                                           \
		if (!(lua_gettop(L) == 2 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2)))              \
			PARAM_ERROR;                                                                            \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2)); \
		code                                                                                        \
			lua_push##retluatype(L, temp);                                                          \
		return 1;                                                                                   \
	}

#define LUA_FUNC_ARITY3(name, retluatype, luatype1, luatype2, luatype3, rettype, type1, type2, type3, code)                        \
	inline int lua_##name(lua_State* L) {                                                                                          \
		if (!(lua_gettop(L) == 3 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3)))                   \
			PARAM_ERROR;                                                                                                           \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3)); \
		code                                                                                                                       \
			lua_push##retluatype(L, temp);                                                                                         \
		return 1;                                                                                                                  \
	}

#define LUA_FUNC_ARITY4(name, retluatype, luatype1, luatype2, luatype3, luatype4, rettype, type1, type2, type3, type4, code)                                      \
	inline int lua_##name(lua_State* L) {                                                                                                                         \
		if (!(lua_gettop(L) == 4 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4)))                        \
			PARAM_ERROR;                                                                                                                                          \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4)); \
		code                                                                                                                                                      \
			lua_push##retluatype(L, temp);                                                                                                                        \
		return 1;                                                                                                                                                 \
	}

#define LUA_FUNC_ARITY5(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, rettype, type1, type2, type3, type4, type5, code)                                                    \
	inline int lua_##name(lua_State* L) {                                                                                                                                                        \
		if (!(lua_gettop(L) == 5 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5)))                             \
			PARAM_ERROR;                                                                                                                                                                         \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5)); \
		code                                                                                                                                                                                     \
			lua_push##retluatype(L, temp);                                                                                                                                                       \
		return 1;                                                                                                                                                                                \
	}

#define LUA_FUNC_ARITY6(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, luatype6, rettype, type1, type2, type3, type4, type5, type6, code)                                                                  \
	inline int lua_##name(lua_State* L) {                                                                                                                                                                                       \
		if (!(lua_gettop(L) == 6 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5) && lua_is##luatype6(L, 6)))                                  \
			PARAM_ERROR;                                                                                                                                                                                                        \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5), (type6)lua_to##luatype6(L, 6)); \
		code                                                                                                                                                                                                                    \
			lua_push##retluatype(L, temp);                                                                                                                                                                                      \
		return 1;                                                                                                                                                                                                               \
	}

#define LUA_FUNC_ARITY7(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, luatype6, luatype7, rettype, type1, type2, type3, type4, type5, type6, type7, code)                                                                                \
	inline int lua_##name(lua_State* L) {                                                                                                                                                                                                                      \
		if (!(lua_gettop(L) == 7 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5) && lua_is##luatype6(L, 6) && lua_is##luatype7(L, 7)))                                       \
			PARAM_ERROR;                                                                                                                                                                                                                                       \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5), (type6)lua_to##luatype6(L, 6), (type7)lua_to##luatype7(L, 7)); \
		code                                                                                                                                                                                                                                                   \
			lua_push##retluatype(L, temp);                                                                                                                                                                                                                     \
		return 1;                                                                                                                                                                                                                                              \
	}

#define LUA_FUNC_ARITY8(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, luatype6, luatype7, luatype8, rettype, type1, type2, type3, type4, type5, type6, type7, type8, code)                                                                                              \
	inline int lua_##name(lua_State* L) {                                                                                                                                                                                                                                                     \
		if (!(lua_gettop(L) == 8 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5) && lua_is##luatype6(L, 6) && lua_is##luatype7(L, 7) && lua_is##luatype8(L, 8)))                                            \
			PARAM_ERROR;                                                                                                                                                                                                                                                                      \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5), (type6)lua_to##luatype6(L, 6), (type7)lua_to##luatype7(L, 7), (type8)lua_to##luatype8(L, 8)); \
		code                                                                                                                                                                                                                                                                                  \
			lua_push##retluatype(L, temp);                                                                                                                                                                                                                                                    \
		return 1;                                                                                                                                                                                                                                                                             \
	}

#define LUA_FUNC_ARITY9(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, luatype6, luatype7, luatype8, luatype9, rettype, type1, type2, type3, type4, type5, type6, type7, type8, type9, code)                                                                                                            \
	inline int lua_##name(lua_State* L) {                                                                                                                                                                                                                                                                                    \
		if (!(lua_gettop(L) == 9 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5) && lua_is##luatype6(L, 6) && lua_is##luatype7(L, 7) && lua_is##luatype8(L, 8) && lua_is##luatype9(L, 9)))                                                 \
			PARAM_ERROR;                                                                                                                                                                                                                                                                                                     \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5), (type6)lua_to##luatype6(L, 6), (type7)lua_to##luatype7(L, 7), (type8)lua_to##luatype8(L, 8), (type9)lua_to##luatype9(L, 9)); \
		code                                                                                                                                                                                                                                                                                                                 \
			lua_push##retluatype(L, temp);                                                                                                                                                                                                                                                                                   \
		return 1;                                                                                                                                                                                                                                                                                                            \
	}

#define LUA_FUNC_ARITY10(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, luatype6, luatype7, luatype8, luatype9, luatype10, rettype, type1, type2, type3, type4, type5, type6, type7, type8, type9, type10, code)                                                                                                                          \
	inline int lua_##name(lua_State* L) {                                                                                                                                                                                                                                                                                                                      \
		if (!(lua_gettop(L) == 10 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5) && lua_is##luatype6(L, 6) && lua_is##luatype7(L, 7) && lua_is##luatype8(L, 8) && lua_is##luatype9(L, 9) && lua_is##luatype10(L, 10)))                                                      \
			PARAM_ERROR;                                                                                                                                                                                                                                                                                                                                       \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5), (type6)lua_to##luatype6(L, 6), (type7)lua_to##luatype7(L, 7), (type8)lua_to##luatype8(L, 8), (type9)lua_to##luatype9(L, 9), (type10)lua_to##luatype10(L, 10)); \
		code                                                                                                                                                                                                                                                                                                                                                   \
			lua_push##retluatype(L, temp);                                                                                                                                                                                                                                                                                                                     \
		return 1;                                                                                                                                                                                                                                                                                                                                              \
	}

#define LUA_FUNC_ARITY11(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, luatype6, luatype7, luatype8, luatype9, luatype10, luatype11, rettype, type1, type2, type3, type4, type5, type6, type7, type8, type9, type10, type11, code)                                                                                                                                         \
	inline int lua_##name(lua_State* L) {                                                                                                                                                                                                                                                                                                                                                        \
		if (!(lua_gettop(L) == 11 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5) && lua_is##luatype6(L, 6) && lua_is##luatype7(L, 7) && lua_is##luatype8(L, 8) && lua_is##luatype9(L, 9) && lua_is##luatype10(L, 10) && lua_is##luatype11(L, 11)))                                                            \
			PARAM_ERROR;                                                                                                                                                                                                                                                                                                                                                                         \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5), (type6)lua_to##luatype6(L, 6), (type7)lua_to##luatype7(L, 7), (type8)lua_to##luatype8(L, 8), (type9)lua_to##luatype9(L, 9), (type10)lua_to##luatype10(L, 10), (type11)lua_to##luatype11(L, 11)); \
		code                                                                                                                                                                                                                                                                                                                                                                                     \
			lua_push##retluatype(L, temp);                                                                                                                                                                                                                                                                                                                                                       \
		return 1;                                                                                                                                                                                                                                                                                                                                                                                \
	}

#define LUA_FUNC_ARITY12(name, retluatype, luatype1, luatype2, luatype3, luatype4, luatype5, luatype6, luatype7, luatype8, luatype9, luatype10, luatype11, luatype12, rettype, type1, type2, type3, type4, type5, type6, type7, type8, type9, type10, type11, type12, code)                                                                                                                                                        \
	inline int lua_##name(lua_State* L) {                                                                                                                                                                                                                                                                                                                                                                                          \
		if (!(lua_gettop(L) == 12 && lua_is##luatype1(L, 1) && lua_is##luatype2(L, 2) && lua_is##luatype3(L, 3) && lua_is##luatype4(L, 4) && lua_is##luatype5(L, 5) && lua_is##luatype6(L, 6) && lua_is##luatype7(L, 7) && lua_is##luatype8(L, 8) && lua_is##luatype9(L, 9) && lua_is##luatype10(L, 10) && lua_is##luatype11(L, 11) && lua_is##luatype12(L, 12)))                                                                  \
			PARAM_ERROR;                                                                                                                                                                                                                                                                                                                                                                                                           \
		rettype temp = (rettype)name((type1)lua_to##luatype1(L, 1), (type2)lua_to##luatype2(L, 2), (type3)lua_to##luatype3(L, 3), (type4)lua_to##luatype4(L, 4), (type5)lua_to##luatype5(L, 5), (type6)lua_to##luatype6(L, 6), (type7)lua_to##luatype7(L, 7), (type8)lua_to##luatype8(L, 8), (type9)lua_to##luatype9(L, 9), (type10)lua_to##luatype10(L, 10), (type11)lua_to##luatype11(L, 11), (type12)lua_to##luatype12(L, 12)); \
		code                                                                                                                                                                                                                                                                                                                                                                                                                       \
			lua_push##retluatype(L, temp);                                                                                                                                                                                                                                                                                                                                                                                         \
		return 1;                                                                                                                                                                                                                                                                                                                                                                                                                  \
	}

#include <TryUtil.h>

// 使用lua pcall的错误报告函数
inline void lua_callalert(lua_State* L) {
	T_B
		lua_getglobal(L, "_ALERT");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_call(L, 1, 0);
	} else { // no _ALERT function; print it on stderr
		printf("%s\n", lua_tostring(L, -2));
		LG("lua", "%s\n", lua_tostring(L, -2));
		lua_pop(L, 2);
	}
	TRY_END_NOTHROW
	LG("lua", "%s\n", lua_tostring(L, -2));
}

static int traceback(lua_State* L) {
	luaL_traceback(L, L, NULL, 0);
	//lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	//lua_getfield(L, -1, "traceback");
	////lua_pushthread(L);
	////lua_pushvalue(L, 1);
	////lua_pushinteger(L, 2);
	//lua_call(L, 0, 1);
	//LG("lua", "%s\n", "test");
	LG("lua", "%s\n", lua_tostring(L, -1));
	return 1;
}

inline int lua_handled_pcall(lua_State* L, int nargs, int nresults, int errfunc) {
	T_B
		//		lua_pushcfunction(L, traceback);
		int nStatus = lua_pcall(L, nargs, nresults, -2);
	//		if (!nStatus)
	//			return lua_pcall(L, 0, 0, lua_gettop(L) - 1);
	//		LG("lua", "%s\n", lua_tostring(L, -1));
	//		return nStatus;
	if (nStatus != 0) {
		switch (nStatus) {
		case LUA_ERRRUN:
			LG("lua", "%s\n", "Runtime error occured while running a script.");
			break;
		case LUA_ERRMEM:
			LG("lua", "%s\n", "Memory allocation error while running a script.");
			break;
		case LUA_ERRERR:
			LG("lua", "%s\n", "Error handler failed while running a script.");
			break;
		default: // not supposed to happen
			LG("lua", "%s\n", "An unknown error has occurred while a running script.");
		} // if an error occurs then there should be a message on the stack

		lua_callalert(L);
		lua_settop(L, 0);
	}
	return nStatus;
	TRY_END_NOTHROW
	LG("lua", "%s\n", "An unknown error has occurred while a running script.");
	return -1;
}

inline int luaL_handled_dostring(lua_State* L, const char* str) {
	T_B int nStatus = luaL_loadstring(L, str);
	switch (nStatus) {
	case 0:
		return (nStatus || lua_handled_pcall(L, 0, LUA_MULTRET, 0));
	case LUA_ERRSYNTAX:
		LG("lua", "%s%s\n", "Syntax error while loading string", str);
		break;
	case LUA_ERRMEM:
		LG("lua", "%s%s\n", "Memory allocation error while loading string", str);
		break;
	default:
		LG("lua", "%s%s\n", "An unknown error has occurred while loading string", str);
	}
	return nStatus;
	TRY_END_NOTHROW
	LG("lua", "%s%s\n", "An unknown error has occurred while loading string", str);
	return -1;
}

inline int luaL_handled_dofile(lua_State* L, const char* filename) {
	T_B int nStatus = luaL_loadfile(L, filename);
	switch (nStatus) {
	case 0:
		return (nStatus || lua_handled_pcall(L, 0, LUA_MULTRET, 0));
	case LUA_ERRSYNTAX:
		LG("lua", "%s%s\n", "Syntax error while loading file: ", filename);
		break;
	case LUA_ERRMEM:
		LG("lua", "%s%s\n", "Memory allocation error while loading file: ", filename);
		break;
	case LUA_ERRFILE:
		LG("lua", "%s%s\n", "Unable to load file: ", filename);
		break;
	default:
		LG("lua", "%s%s\n", "An unknown error has occurred while loading file: ", filename);
	}
	return nStatus;
	TRY_END_NOTHROW
	LG("lua", "%s%s\n", "An unknown error has occurred while loading file: ", filename);
	return -1;
}
#endif

#include "GameApp.h"
#include "Scene.h"
#include "worldscene.h"
#include "SceneItem.h"
#include "Character.h"
#include "Actor.h"
#include "uicozeform.h"
#include "UIFormMgr.h"
#include "cameractrl.h"

#include "lua_app.h"
#include "lua_scene.h"
#include "lua_object.h"
#include "lua_input.h"
#include "lua_ui.h"
#include "lua_network.h"
#include "lua_camera.h"
#include "lua_util.h"
#endif
#endif
