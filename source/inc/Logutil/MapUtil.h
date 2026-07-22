#pragma once

struct SMapPatchHeader {
	DWORD dwFlag{20040228};
	char szMapName[16]{""};
	DWORD dwUpdateCnt{0};
	short sSectionCntX;
	short sSectionCntY;
};

BOOL MU_CreateMapPatch(const char* pszOld, const char* pszNew);
BOOL MU_PatchMapFile(const char* pszMap, const char* pszPatch);
