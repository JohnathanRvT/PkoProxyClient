
#include "NoLockRecord.h"
//---------------------------------------------------------------------------
// class CNoLockRecord
//---------------------------------------------------------------------------
CNoLockRecord::CNoLockRecord() {
	dwItemID = 0;
}
//---------------------------------------------------------------------------
// class CNoLockRecordSet
//---------------------------------------------------------------------------
CNoLockRecordSet* CNoLockRecordSet::_Instance = nullptr;
BOOL CNoLockRecordSet::_ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) {
	if (ParamList.empty())
		return FALSE;

	CNoLockRecord* pInfo = (CNoLockRecord*)pRawDataInfo;
	// 编号
	pInfo->dwItemID = pInfo->nID;
	return TRUE;
}
