//=============================================================================
// FileName: LevelRecord.cpp
// Creater: ZhangXuedong
// Date: 2004.12.10
// Comment: CLevelRecord class
//=============================================================================

#include "LevelRecord.h"

CLevelRecordSet* CLevelRecordSet::_Instance = nullptr;

BOOL CLevelRecordSet::_ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) {
	if (ParamList.empty())
		return FALSE;

	CLevelRecord* pInfo = (CLevelRecord*)pRawDataInfo;

	int m = 0, n = 0;
	std::string strList[80];
	std::string strLine;

	// 编号
	pInfo->lID = pInfo->nID;
	// 等级
	pInfo->sLevel = Str2Int(pInfo->szDataName);
	// 经验值
	pInfo->ulExp = _atoi64(ParamList[m++].c_str());

	return TRUE;
}
