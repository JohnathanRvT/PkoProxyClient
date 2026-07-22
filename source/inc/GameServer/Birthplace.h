#include "rng.h"
constexpr int MAX_BIRTHPOINT = 12;

struct SBirthPoint {	// 单个出生点
	char szMapName[16]; // 地图名
	int x;				// 地图坐标
	int y;
};

struct SBirthplace { // 出生地, 包含多个出生点
	std::array<SBirthPoint, MAX_BIRTHPOINT> PointList;
	int nCount{0};

	void Add(const char* pszMapName, int x, int y) {
		if (nCount >= MAX_BIRTHPOINT) {
			return; // 有最大出生点限制
		}

		strncpy_s(PointList[nCount].szMapName, sizeof(PointList[nCount].szMapName), pszMapName, _TRUNCATE);
		PointList[nCount].x = x;
		PointList[nCount].y = y;
		nCount++;
	}
};

class CBirthMgr { // 出生地管理
public:
	void AddBirthPoint(const char* pszLocation, const char* pszMapName, int x, int y);
	SBirthPoint* GetRandBirthPoint(const char* pszLocation);
	void ClearAll() { _LocIdx.clear(); }

protected:
	std::map<std::string, std::unique_ptr<SBirthplace>> _LocIdx;
};

// 添加单个出生点
inline void CBirthMgr::AddBirthPoint(const char* pszLocation, const char* pszMapName, int x, int y) {
	auto& birthplace = [&]() -> std::unique_ptr<SBirthplace>& {
		auto it = _LocIdx.find(pszLocation);
		if (it != _LocIdx.end()) { // Found existing birthplace
			return it->second;
		} else { // Create new birthplace
			return _LocIdx[pszLocation] = std::make_unique<SBirthplace>();
		}
	}();
	birthplace->Add(pszMapName, x, y);
}

inline SBirthPoint* CBirthMgr::GetRandBirthPoint(const char* pszLocation) {
	
	auto it = _LocIdx.find(pszLocation);
	if (it != _LocIdx.end()) {
		auto pBirthplace = it->second.get();
		const int nSel = rng.uniform(0, pBirthplace->nCount - 1);
		SBirthPoint* pPoint = &(pBirthplace->PointList[nSel]);
		//LG("birth", "选中了随机出生点[%s] %d %d\n", pPoint->szMapName, pPoint->x, pPoint->y);
		return pPoint;
	}
	return nullptr;
}

extern CBirthMgr g_BirthMgr;

inline SBirthPoint* GetRandBirthPoint(const char* pszChaName, const char* pszLocation) {
	SBirthPoint* pBirth = g_BirthMgr.GetRandBirthPoint(pszLocation);
	if (!pBirth) {
		LG("birth_error", "invalid birth place[%s], Cha = [%s],will force to silver city\n", pszLocation, pszChaName);
		pBirth = g_BirthMgr.GetRandBirthPoint(RES_STRING(GM_BIRTHPLACE_H_00001));
	}
	return pBirth;
}
