#pragma once
#include "TableData.h"

class CItemTypeInfo : public CRawDataInfo {
public:
	CItemTypeInfo() {
	}
};

class CItemTypeSet : public CRawDataSet {
public:
	static CItemTypeSet* I() { return _Instance; }

	CItemTypeSet(int nIDStart, int nIDCnt)
		: CRawDataSet(nIDStart, nIDCnt) {
		_Instance = this;
		_Init();
	}

protected:
	static CItemTypeSet* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new CItemTypeInfo[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](CItemTypeInfo*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(CItemTypeInfo);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return NULL;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override {
		// if(ParamList.size()==0) return FALSE;

		CItemTypeInfo* pInfo = (CItemTypeInfo*)pRawDataInfo;
		return TRUE;
	}
};

inline CItemTypeInfo* GetItemTypeInfo(int nTypeID) {
	return (CItemTypeInfo*)CItemTypeSet::I()->GetRawDataInfo(nTypeID);
}
