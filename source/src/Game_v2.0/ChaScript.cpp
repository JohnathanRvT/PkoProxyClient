#include "stdafx.h"
#ifdef CALUA_COMPAT
#include "caLua.h" //CHANGED: Removed CaLua dependency
#endif
//#include "SelChaScene.h"
#ifndef CALUA_COMPAT
#include "lua_platform.h" //MAJOR CHANGED: Removed CaLua dependency
#endif

int GetChaPhotoTexID(int nTypeID) {
	CChaRecord* pInfo = GetChaRecordInfo(nTypeID);
	if (pInfo) {
		char szPhoto[80] = {0};
		_snprintf_s(szPhoto, _TRUNCATE, "texture/photo/%s.bmp", pInfo->szIconName);
		return GetTextureID(szPhoto);
	}
	return 0;
}

int CH_Create(int sceneid, int type) {
	auto* s = dynamic_cast<CGameScene*>(CScript::GetScriptObj(sceneid));
	if (!s)
		return R_FAIL;

	CCharacter* c = s->AddCharacter(type);
	if (c)
		return c->GetScriptID();

	return R_FAIL;
}

int CH_SetPos(int id, int x, int y) {
	auto* c = dynamic_cast<CCharacter*>(CScript::GetScriptObj(id));
	if (!c)
		return R_FAIL;

	c->setPos(x, y);
	return R_OK;
}

int CH_SetYaw(int id, int yaw) {
	auto* c = dynamic_cast<CCharacter*>(CScript::GetScriptObj(id));
	if (!c)
		return R_FAIL;

	c->setYaw(yaw);
	return R_OK;
}

int CH_PlayPos(int id, int pose, int posetype) {
	auto* c = dynamic_cast<CCharacter*>(CScript::GetScriptObj(id));
	if (!c)
		return R_FAIL;

	c->PlayPose(pose, posetype);
	return R_OK;
}
//---------------------------------------------------------------------------
// ScriptRegedit
//---------------------------------------------------------------------------
//MAJOR CHANGED: Removed CaLua dependency
#ifdef CALUA_COMPAT
void MPInitLua_Cha() {
	CLU_RegisterFunction("CH_Create", "int", "int,int", CLU_CDECL, CLU_CAST(CH_Create));
	CLU_RegisterFunction("CH_SetPos", "int", "int,int,int", CLU_CDECL, CLU_CAST(CH_SetPos));
	CLU_RegisterFunction("CH_SetYaw", "int", "int,int", CLU_CDECL, CLU_CAST(CH_SetYaw));
	CLU_RegisterFunction("CH_PlayPos", "int", "int,int,int", CLU_CDECL, CLU_CAST(CH_PlayPos));
}
#else
LUA_FUNC_ARITY2(CH_Create,
				number, number, number,
				int, int, int,
				LUA_FUNC_NIL_HANDLING(-1));
LUA_FUNC_ARITY3(CH_SetPos,
				number, number, number, number,
				int, int, int, int);
LUA_FUNC_ARITY2(CH_SetYaw,
				number, number, number,
				int, int, int);
LUA_FUNC_ARITY3(CH_PlayPos,
				number, number, number, number,
				int, int, int, int);
void MPInitLua_Cha(lua_State* L) {
	LUA_FUNC_REG(CH_Create);
	LUA_FUNC_REG(CH_SetPos);
	LUA_FUNC_REG(CH_SetYaw);
	LUA_FUNC_REG(CH_PlayPos);
}
#endif