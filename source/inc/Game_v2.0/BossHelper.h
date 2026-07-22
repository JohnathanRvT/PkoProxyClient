#pragma once
#include "util.h"
#include "TableData.h"

#define BOSS_MAXSIZE_NAME 128 // npc名称长度

class BossData : public CRawDataInfo {
public:
	char szName[BOSS_MAXSIZE_NAME];	// 地图中显示Boss名称
	char szArea[BOSS_MAXSIZE_NAME];	   // 位置或者等级
	DWORD dwxPos0, dwyPos0;			  // Boss位置信息
	char szMapName[BOSS_MAXSIZE_NAME]; // Boss所在地图名称
};

class BossHelper : public CRawDataSet {
public:
	static BossHelper* I() { return _Instance; }

	BossHelper(int nIDStart, int nIDCnt, int nCol = 128)
		: CRawDataSet(nIDStart, nIDCnt, nCol) {
		_Instance = this;
		_Init();
	}

protected:
	static BossHelper* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new BossData[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](BossData*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(BossData);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return nullptr;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override;
};

inline BossData* GetBossDataInfo(int nTypeID) {
	return (BossData*)BossHelper::I()->GetRawDataInfo(nTypeID);
}
