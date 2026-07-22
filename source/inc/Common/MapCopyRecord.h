//======================================================================================================================
// FileName: HairRecord.h
// Creater: Jerry li
// Date: 2005.08.29
// Comment: CHairRecordSet class
//======================================================================================================================

#ifndef MAPCOPYRECORD_H
#define MAPCOPYRECORD_H

#include "util.h"
#include "TableData.h"

class CMapCopyRecord : public CRawDataInfo {
public:
	CMapCopyRecord();
	int nMapID;
	char szMapName[32]; //副本地图名
	short m_nCopyNum;   //副本地图个数
	char m_cStartType;  //开始类型
};

class CMapCopyRecordSet : public CRawDataSet {
public:
	static CMapCopyRecordSet* I() { return _Instance; }

	CMapCopyRecordSet(int nIDStart, int nIDCnt, int nCol = 128)
		: CRawDataSet(nIDStart, nIDCnt, nCol) {
		_Instance = this;
		_Init();
	}

protected:
	static CMapCopyRecordSet* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new CMapCopyRecord[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](CMapCopyRecord*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(CMapCopyRecord);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return nullptr;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}
	// virtual void  _AfterLoad();

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override;
	//virtual void _ProcessRawDataInfo(CRawDataInfo *pInfo);
};

inline CMapCopyRecord* GetMapCopyRecordInfo(int nTypeID) {
	return (CMapCopyRecord*)CMapCopyRecordSet::I()->GetRawDataInfo(nTypeID);
}

#endif //MAPCOPYRECORD_H
