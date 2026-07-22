#include "stdafx.h"
#include "monsterhelper.h"

MonsterHelper* MonsterHelper::_Instance = nullptr;
BOOL MonsterHelper::_ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) {
	if (ParamList.empty())
		return FALSE;

	auto* pInfo = (MonsterData*)pRawDataInfo;

	int m = 0, n = 0;
	std::string strList[8];
	std::string strLine;

	// Monster显示名称
	strncpy_s(pInfo->szName, pInfo->szDataName, _TRUNCATE);
	pInfo->szName[MONSTER_MAXSIZE_NAME - 1] = '\0';

	strncpy_s(pInfo->szArea, ParamList[m++].c_str(), _TRUNCATE);
	pInfo->szMapName[MONSTER_MAXSIZE_NAME - 1] = '\0';

	// Monster位置信息
	Util_ResolveTextLine(ParamList[m++].c_str(), strList, 8, ',');
	pInfo->dwxPos0 = Str2Int(strList[0]);
	pInfo->dwyPos0 = Str2Int(strList[1]);

	// Monster所在地图显示名称
	strncpy_s(pInfo->szMapName, ParamList[m++].c_str(), _TRUNCATE);
	pInfo->szMapName[MONSTER_MAXSIZE_NAME - 1] = '\0';

	return TRUE;
}
