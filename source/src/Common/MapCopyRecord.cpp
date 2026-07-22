
#include "MapCopyRecord.h"
//---------------------------------------------------------------------------
// class CNoLockRecord
//---------------------------------------------------------------------------
CMapCopyRecord::CMapCopyRecord() {
	nMapID = 0;
	memset(szMapName, 0, sizeof(szMapName));
	m_nCopyNum = 0;
	m_cStartType = 2;
}
//---------------------------------------------------------------------------
// class CNoLockRecordSet
//---------------------------------------------------------------------------
CMapCopyRecordSet* CMapCopyRecordSet::_Instance = nullptr;
BOOL CMapCopyRecordSet::_ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) {
	if (ParamList.empty())
		return FALSE;

	CMapCopyRecord* pInfo = (CMapCopyRecord*)pRawDataInfo;
	int m = 0;
	if (pInfo) {
		// 编号
		pInfo->nMapID = pInfo->nID;

		strncpy_s(pInfo->szMapName, sizeof(pInfo->szMapName), pInfo->szDataName, _TRUNCATE);
		pInfo->m_nCopyNum = Str2Int(ParamList[m++]);
		pInfo->m_cStartType = Str2Int(ParamList[m++]);
	}
	return TRUE;
}
