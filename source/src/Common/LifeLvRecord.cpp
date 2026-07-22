//=============================================================================
// FileName: LifeLvRecord.cpp
// Creater: ZhangXuedong
// Date: 2005.05.28
// Comment: CLifeLvRecord class
//=============================================================================

#include "LifeLvRecord.h"

CLifeLvRecordSet* CLifeLvRecordSet::_Instance = nullptr;

BOOL CLifeLvRecordSet::_ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) {
	if (ParamList.empty())
		return FALSE;

	CLifeLvRecord* pInfo = (CLifeLvRecord*)pRawDataInfo;

	int m = 0, n = 0;
	std::string strList[80];
	std::string strLine;

	// 编号
	pInfo->lID = pInfo->nID;
	// 等级
	pInfo->sLevel = Str2Int(pInfo->szDataName);
	// 经验值
	pInfo->ulExp = Str2Int(ParamList[m++]);

	return TRUE;
}
