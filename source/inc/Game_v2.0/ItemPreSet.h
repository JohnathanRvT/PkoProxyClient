#pragma once

#include "TableData.h"

class CItemPreInfo : public CRawDataInfo {
public:
};

class CItemPreSet : public CRawDataSet {
public:
	static CItemPreSet* I() { return _Instance; }

	CItemPreSet(int nIDStart, int nIDCnt)
		: CRawDataSet(nIDStart, nIDCnt) {
		_Instance = this;
		_Init();
	}

protected:
	static CItemPreSet* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new CItemPreInfo[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](CItemPreInfo*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(CItemPreInfo);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return NULL;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override {
		CItemPreInfo* pInfo = (CItemPreInfo*)pRawDataInfo;
		return TRUE;
	}
};

inline CItemPreInfo* GetItemPreInfo(int nTypeID) {
	return (CItemPreInfo*)CItemPreSet::I()->GetRawDataInfo(nTypeID);
}
