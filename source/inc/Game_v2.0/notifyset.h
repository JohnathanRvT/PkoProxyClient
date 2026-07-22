#pragma once

#include "TableData.h"

class CNotifyInfo : public CRawDataInfo {
public:
	CNotifyInfo() : chType(0) {
		memset(szInfo, 0, sizeof(szInfo));
	}

	char chType;
	char szInfo[64];
};

class CNotifySet : public CRawDataSet {

public:
	static CNotifySet* I() { return _Instance; }

	CNotifySet(int nIDStart, int nIDCnt)
		: CRawDataSet(nIDStart, nIDCnt) {
		_Instance = this;
		_Init();
	}

protected:
	static CNotifySet* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new CNotifyInfo[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](CNotifyInfo*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(CNotifyInfo);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return NULL;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override {
		CNotifyInfo* pInfo = (CNotifyInfo*)pRawDataInfo;

		pInfo->chType = Str2Int(ParamList[0]);

		// Modify by lark.li 20080701 begin
		//strncpy_s( pInfo->szInfo, ParamList[1].c_str(),_TRUNCATE);
		strncpy_s(pInfo->szInfo, sizeof(pInfo->szInfo), ConvertResString(ParamList[1].c_str()), _TRUNCATE);
		// End

		return TRUE;
	}
};

inline CNotifyInfo* GetNotifyInfo(int nTypeID) {
	return (CNotifyInfo*)CNotifySet::I()->GetRawDataInfo(nTypeID);
}
