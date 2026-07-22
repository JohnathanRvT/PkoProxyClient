#include "stdafx.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#endif
#ifndef CALUA_COMPAT
#include "lua_platform.h" //MAJOR CHANGED: Removed CaLua dependency
#endif

void CGameApp::LoadScriptScene(eSceneType eType) {
#ifdef CALUA_COMPAT
	switch (eType) {
	case enumLoginScene:
		LoadScriptScene("scripts/lua/scene/loginscene.clu");
		break;
	case enumSelectChaScene:
		LoadScriptScene("scripts/lua/scene/selectchascene.clu");
		break;
	case enumCreateChaScene:
		LoadScriptScene("scripts/lua/scene/createchascene.clu");
		break;
	case enumWorldScene:
		LoadScriptScene("scripts/lua/scene/mainscene.clu");
		break;
	}
#else
	switch (eType) {
	case enumLoginScene:
		lua_getfield(L, LUA_GLOBALSINDEX, "LoginScene_Callback");
		break;
	case enumSelectChaScene:
		lua_getfield(L, LUA_GLOBALSINDEX, "SelectChaScene_Callback");
		break;
	case enumCreateChaScene:
		lua_getfield(L, LUA_GLOBALSINDEX, "CreateChaScene_Callback");
		break;
	case enumWorldScene:
		lua_getfield(L, LUA_GLOBALSINDEX, "MainScene_Callback"); //break;
	}
	lua_handled_pcall(L, 0, 0, NULL); //call with 1 parameter, 1 result
#endif
}

void CGameApp::LoadScriptScene(const char* script_file) {
	//try
	//{
#ifdef CALUA_COMPAT
	CLU_LoadScript((char*)script_file);
#else
	luaL_handled_dofile(L, script_file);
#endif
	//}
	//catch(...)
	//{
	//	LG( "script", "msgLoad Scene Script[%s] Error", script_file );
	//}
}

//---------------------------------------------------------------------------
// App_Script
//---------------------------------------------------------------------------
int GP_SetCameraPos(float ex, float ey, float ez, float rx, float ry, float rz) {
	CCameraCtrl* pCam = g_pGameApp->GetMainCam();
	pCam->m_EyePos.x = ex;
	pCam->m_EyePos.y = ey;
	pCam->m_EyePos.z = ez;
	pCam->m_RefPos.x = rx;
	pCam->m_RefPos.y = ry;
	pCam->m_RefPos.z = rz;

	return R_OK;
}

int GP_GotoScene(int sceneid) {
	CGameScene* p = dynamic_cast<CGameScene*>(CScript::GetScriptObj(sceneid));
	if (!p)
		return R_FAIL;

	g_pGameApp->GotoScene(p);
	return R_OK;
}

//---------------------------------------------------------------------------
// ScriptRegedit
//---------------------------------------------------------------------------
//MAJOR CHANGED: Removed CaLua dependency
#ifdef CALUA_COMPAT
void MPInitLua_App() {
	CLU_RegisterFunction("GP_SetCameraPos", "int", "float,float,float,float,float,float", CLU_CDECL, CLU_CAST(GP_SetCameraPos));
	CLU_RegisterFunction("GP_GotoScene", "int", "int", CLU_CDECL, CLU_CAST(GP_GotoScene));
}
#else
LUA_FUNC_ARITY6(GP_SetCameraPos,
				number, number, number, number, number, number, number,
				int, float, float, float, float, float, float);
LUA_FUNC_ARITY1(GP_GotoScene,
				number, number,
				int, int);
void MPInitLua_App(lua_State* L) {
	LUA_FUNC_REG(GP_SetCameraPos);
	LUA_FUNC_REG(GP_GotoScene);
}
#endif