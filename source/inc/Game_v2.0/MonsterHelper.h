#pragma once
#include "util.h"
#include "TableData.h"

#define MONSTER_MAXSIZE_NAME 128 // npc名称长度

class MonsterData : public CRawDataInfo {
public:
	char szName[MONSTER_MAXSIZE_NAME];	// 地图中显示Monster名称
	char szArea[MONSTER_MAXSIZE_NAME];	// 位置或者等级
	DWORD dwxPos0, dwyPos0;			  // Monster位置信息
	char szMapName[MONSTER_MAXSIZE_NAME]; // Monster所在地图名称
};

class MonsterHelper : public CRawDataSet {
public:
	static MonsterHelper* I() { return _Instance; }

	MonsterHelper(int nIDStart, int nIDCnt, int nCol = 128)
		: CRawDataSet(nIDStart, nIDCnt, nCol) {
		_Instance = this;
		_Init();
	}

protected:
	static MonsterHelper* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new MonsterData[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](MonsterData*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(MonsterData);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return nullptr;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override;
};

inline MonsterData* GetMonsterDataInfo(int nTypeID) {
	return (MonsterData*)MonsterHelper::I()->GetRawDataInfo(nTypeID);
}
