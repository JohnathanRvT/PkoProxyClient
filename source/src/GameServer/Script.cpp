// Script.cpp Created by knight-gongjian 2004.12.1.
//---------------------------------------------------------

//MAJOR CHANGED: Commented all dofile except for initial.lua and map ones.

#include "stdafx.h"
#include "Script.h"
#include "NpcScript.h"
#include "CharScript.h"
#include "EntityScript.h"

#include "lua_gamectrl.h"

//---------------------------------------------------------
extern const char* GetResPath(const char* pszRes);

CCharacter* g_pNoticeChar = nullptr;
lua_State* g_pLuaState = nullptr;

BOOL InitLuaScript() {
	g_pLuaState = lua_open();
	luaL_openlibs(g_pLuaState);

	if (!RegisterScript())
		return FALSE;

	if (!LoadScript())
		return FALSE;

	return TRUE;
}

BOOL CloseLuaScript() {
	if (g_pLuaState)
		lua_close(g_pLuaState);
	g_pLuaState = nullptr;
	return TRUE;
}

BOOL RegisterScript() {
	if (!RegisterCharScript() || !RegisterNpcScript())
		return FALSE;

	if (!RegisterEntityScript())
		return FALSE;

	return TRUE;
}

void ReloadLuaInit() {

	luaL_handled_dofile(g_pLuaState, GetResPath("script/initial.lua")); //CHANGED: better lua error handling (deguix)
}

void ReloadLuaSdk() {
	//CHANGED: better lua error handling (deguix)
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisSdk/NpcSdk.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisSdk/MissionSdk.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisSdk/scriptsdk.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/ScriptDefine.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcDefine.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/templatesdk.lua") );
	//
	//// 由updateall会触发ai_sdk更新
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/birth/birth_conf.lua"));
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/ai/ai.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/calculate/skilleffect.lua"));
}

void ReloadNpcScript() {
	//CHANGED: better lua error handling (deguix)
	//// 装载NPC任务数据信息
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript01.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript02.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript03.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript04.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript05.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript06.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript07.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/MissionScript08.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/SendMission.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/EudemonScript.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/CharBornScript.lua") );
	//
	//// 装载NPC对话数据信息
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript01.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript02.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript03.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript04.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript05.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript06.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript07.lua") );
	//luaL_handled_dofile( g_pLuaState, GetResPath("script/MisScript/NpcScript08.lua") );
}

void ReloadEntity(const char szFileName[]) {
	luaL_handled_dofile(g_pLuaState, szFileName); //CHANGED: better lua error handling (deguix)
}

BOOL LoadScript() {
	ReloadLuaInit();
	ReloadLuaSdk();
	ReloadNpcScript();
	return TRUE;
}
