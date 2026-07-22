#pragma once

#include "MindPowerAPI.h"
#include "TableData.h"

//如果定义了该宏资源文件将按照脚本文件加载
//#define USE_RESOURCE_SCRIPT
#define RESOURCE_SCRIPT 0 // 0-不使用资源脚本，1-资源脚本定义，2-资源脚本使用测试时， 3-正式使用资源脚本文件

#pragma warning(disable : 4275)

class MINDPOWER_API MPResourceInfo : public CRawDataInfo {
public:
	enum { // ResourceType (.Abbr RT)
		RT_PAR = 0,
		RT_PATH = 1,
		RT_EFF = 2,
		RT_MESH = 3,
		RT_TEXTURE = 4,
		RT_UNKNOWN = -1,
	};

public:
	MPResourceInfo() : m_iType(RT_UNKNOWN) {}

	int GetType() const { return m_iType; }

public:
	int m_iType;
};

class MINDPOWER_API MPResourceSet : public CRawDataSet {
public:
	static MPResourceSet* I() { return _Instance; }

	MPResourceSet(int nIDStart, int nIDCnt) : CRawDataSet(nIDStart, nIDCnt) {
		_Init();
		_Instance = this;
	}

	MPResourceInfo* GetResourceInfoByID(int nID) {
		return (MPResourceInfo*)GetRawDataInfo(nID);
	}

protected:
	static MPResourceSet* _Instance; // 相当于单键, 把自己记住

	virtual CRawDataInfo* _CreateRawDataArray(int nCnt) override {
		return new MPResourceInfo[nCnt];
	}

	virtual void _DeleteRawDataArray() override {
		delete[](MPResourceInfo*) _RawDataArray;
	}

	virtual int _GetRawDataInfoSize() const override {
		return sizeof(MPResourceInfo);
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) override {
		return nullptr;
	}

	virtual void _DeleteRawData(CRawDataInfo* pInfo) override {
		SAFE_DELETE(pInfo->pData);
	}

	virtual BOOL _ReadRawDataInfo(CRawDataInfo* pRawDataInfo, std::vector<std::string>& ParamList) override {
		if (ParamList.size() == 0)
			return FALSE;

		MPResourceInfo* pInfo = (MPResourceInfo*)pRawDataInfo;

		pInfo->m_iType = Str2Int(ParamList[0]);

		return TRUE;
	}
};

#pragma warning(default : 4275)
