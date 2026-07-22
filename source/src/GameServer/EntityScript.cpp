//---------------------------------------------------------
// EntityScript.cpp Created by knight-gong in 2005.5.12.

#include "stdafx.h"
#include "EntityScript.h"
#include "GameAppNet.h"
#include "Character.h"
#include "lua_gamectrl.h"

#include "stdafx.h" //add by alfred.shi 20080203

//---------------------------------------------------------
_DBC_USING
using namespace mission;

inline int lua_GetCurSubmap(lua_State* L) {
	if (!g_pScriptMap) {
		LG("entity_error", RES_STRING(GM_ENTITYSCRIPT_CPP_00001));
		printf(RES_STRING(GM_ENTITYSCRIPT_CPP_00001));
		E_LUANULL;
		return 0;
	}

	lua_pushnumber(L, LUA_TRUE);
	lua_pushlightuserdata(L, g_pScriptMap);

	return 2;
}

inline int lua_CreateEventEntity(lua_State* L) {
	if (const bool bValid = lua_gettop(L) == 8 && lua_isnumber(L, 1) && lua_islightuserdata(L, 2) &&
							lua_isstring(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5) &&
							lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8);
		!bValid) {
		E_LUAPARAM;
		return 0;
	}

	const auto byType = static_cast<BYTE>(lua_tonumber(L, 1));
	auto pMap = static_cast<SubMap*>(lua_touserdata(L, 2));
	const char* pszName = lua_tostring(L, 3);
	const auto sID = static_cast<USHORT>(lua_tonumber(L, 4));
	const auto sInfoID = static_cast<USHORT>(lua_tonumber(L, 5));
	const auto dwxPos = static_cast<DWORD>(lua_tonumber(L, 6));
	const auto dwyPos = static_cast<DWORD>(lua_tonumber(L, 7));
	const auto sDir = static_cast<USHORT>(lua_tonumber(L, 8));
	mission::CEventEntity* pEntity = g_pGameApp->CreateEntity(byType);
	if (!pMap || !pEntity) {
		E_LUANULL;
		return 0;
	}

	const BOOL bRet = pEntity->Create(*pMap, pszName, sID, sInfoID, dwxPos, dwyPos, sDir);
	lua_pushnumber(L, (bRet) ? LUA_TRUE : LUA_FALSE);
	lua_pushlightuserdata(L, pEntity);

	return 2;
}

int lua_SetEntityData(lua_State* L) {
	if (bool bValid = lua_gettop(L) >= 1; !bValid) {
		E_LUAPARAM;
		return 0;
	}

	BOOL bRet = FALSE;
	auto pEntity = static_cast<mission::CEventEntity*>(lua_touserdata(L, 1));
	switch (pEntity->GetType()) {

	case BASE_ENTITY: { // 基本实体
	} break;
	case RESOURCE_ENTITY: { // 资源实体
		bool bValid = lua_gettop(L) >= 4;
		if (!bValid) {
			E_LUAPARAM;
			return 0;
		}
		const auto sItemID = static_cast<USHORT>(lua_tonumber(L, 2));
		const auto sCount = static_cast<USHORT>(lua_tonumber(L, 3));
		const auto sTime = static_cast<USHORT>(lua_tonumber(L, 4));
		bRet = static_cast<mission::CResourceEntity*>(pEntity)->SetData(sItemID, sCount, sTime);
	} break;

	case TRANSIT_ENTITY: { // 传送实体
	} break;

	case BERTH_ENTITY: { // 停泊实体
		bool bValid = lua_gettop(L) >= 5;
		if (!bValid) {
			E_LUAPARAM;
			return 0;
		}
		const auto sBerthID = static_cast<USHORT>(lua_tonumber(L, 2));
		const auto sxPos = static_cast<USHORT>(lua_tonumber(L, 3));
		const auto syPos = static_cast<USHORT>(lua_tonumber(L, 4));
		const auto sDir = static_cast<USHORT>(lua_tonumber(L, 5));
		bRet = static_cast<mission::CBerthEntity*>(pEntity)->SetData(sBerthID, sxPos, syPos, sDir);
	} break;
	default: {
		E_LUAPARAM;
		return 0;
	} break;
	}

	lua_pushnumber(L, (bRet) ? LUA_TRUE : LUA_FALSE);

	return 1;
}

BOOL RegisterEntityScript() {
	lua_State* L = g_pLuaState;

	REGFN(GetCurSubmap);
	REGFN(CreateEventEntity);
	REGFN(SetEntityData);

	return TRUE;
}