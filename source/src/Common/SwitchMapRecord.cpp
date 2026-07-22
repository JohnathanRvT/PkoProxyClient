//=============================================================================
// FileName: SwitchMapRecord.cpp
// Creater: ZhangXuedong
// Date: 2004.11.23
// Comment:
//=============================================================================

#include "SwitchMapRecord.h"

CSwitchMapRecordSet* CSwitchMapRecordSet::_Instance = nullptr;

BOOL CSwitchMapRecordSet::_ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) {
	if (ParamList.empty())
		return FALSE;

	CSwitchMapRecord* pInfo = (CSwitchMapRecord*)pRawDataInfo;

	int m = 0, n = 0;
	std::string strList[80];
	std::string strLine;

	// 编号
	pInfo->lID = pInfo->nID;
	// 被绑定的实体ID
	pInfo->lEntityID = Str2Int(pInfo->szDataName);
	// 事件编号
	pInfo->lEventID = Str2Int(ParamList[m++]);
	// 实体的位置
	memset(&pInfo->SEntityPos, 0, sizeof(Point));
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	pInfo->SEntityPos.x = Str2Int(strList[0]);
	pInfo->SEntityPos.y = Str2Int(strList[1]);
	// 实体的方向
	pInfo->sAngle = Str2Int(ParamList[m++]);
	// 目标地图名
	//_tcsncpy(pInfo->szTarMapName, ParamList[m++].c_str(), defMAP_NAME_LEN);
	strncpy_s(pInfo->szTarMapName, ParamList[m++].c_str(), _TRUNCATE);
	//pInfo->szTarMapName[defMAP_NAME_LEN - 1] = '\0';
	// 目标地图的位置
	memset(&pInfo->STarPos, 0, sizeof(Point));
	strLine = ParamList[m++];
	n = Util_ResolveTextLine(strLine.c_str(), strList, 80, ',');
	pInfo->STarPos.x = Str2Int(strList[0]);
	pInfo->STarPos.y = Str2Int(strList[1]);

	return TRUE;
}
