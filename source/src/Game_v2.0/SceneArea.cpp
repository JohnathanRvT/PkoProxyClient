#include "Stdafx.h"
#include "SceneArea.h"
#include "GameConfig.h"

CSceneArea::CSceneArea() {
	m_bInitSuccess = false;
	m_lUnitNum = 0;
	m_fRdWr = nullptr;
}

CSceneArea::~CSceneArea() {
	if (m_fRdWr)
		fclose(m_fRdWr);
}

long CSceneArea::Init(char* ptcsAreaFile, bool bSilence) {
	long lRet = 1;
	char tcsPrint[256];

	if (ptcsAreaFile == nullptr)
		return -1;

	if (m_bInitSuccess)
		Free();

	if (g_Config.m_bEditor)
		_tfopen_s(&m_fRdWr, ptcsAreaFile, "r+b");
	else
		_tfopen_s(&m_fRdWr, ptcsAreaFile, "rb");
	if (m_fRdWr == nullptr) {
		if (!bSilence) {
			_snprintf_s(tcsPrint, _TRUNCATE, "%s %s", ptcsAreaFile, RES_STRING(CL_LANGUAGE_MATCH_339));
			MessageBox(nullptr, tcsPrint, RES_STRING(CL_LANGUAGE_MATCH_25), 0);
		}
		lRet = -1;
		goto end;
	}
	fread(&m_SFileHead, sizeof(SFileHead), 1, m_fRdWr);
	if (m_SFileHead.lVersion != SCENE_AREA_FILE_VER100) {
		if (!bSilence) {
			_snprintf_s(tcsPrint, _TRUNCATE, "%s %s", ptcsAreaFile, RES_STRING(CL_LANGUAGE_MATCH_340));
			MessageBox(nullptr, tcsPrint, RES_STRING(CL_LANGUAGE_MATCH_25), 0);
		}
		lRet = -2;
		goto end;
	}

	m_lUnitNum = m_SFileHead.iSceneWidth * m_SFileHead.iSceneHeight;
	m_bInitSuccess = true;

end:
	if (lRet < 0)
		if (m_fRdWr)
			fclose(m_fRdWr);
	return lRet;
}

void CSceneArea::Free() {
	if (m_bInitSuccess) {
		if (m_fRdWr) {
			fclose(m_fRdWr);
			m_fRdWr = nullptr;
		}
	}
	m_lUnitNum = 0;
	m_bInitSuccess = false;
}

long CSceneArea::CreateFile(char* ptcsAreaFile, int iSceneWidth, int iSceneHeight) {
	if (m_bInitSuccess)
		Free();

	FILE* fFile = nullptr;
	SFileHead SHead;
	SAreaUnit* pSAreaUnit = nullptr;

	if (g_Config.m_bEditor)
		_tfopen_s(&fFile, ptcsAreaFile, "wb");
	else
		_tfopen_s(&fFile, ptcsAreaFile, "rb");
	if (fFile == nullptr)
		return -1;

	strncpy_s(SHead.tcsTitle, "HF Scene Area File!", _TRUNCATE);
	SHead.lVersion = SCENE_AREA_FILE_VER100;
	SHead.lFileSize = sizeof(SFileHead) + sizeof(SAreaUnit) * iSceneWidth * iSceneHeight;

	pSAreaUnit = new (SAreaUnit[iSceneWidth * iSceneHeight]);
	if (pSAreaUnit == nullptr) {
		fclose(fFile);
		return -2;
	}

	fwrite(&SHead, sizeof(SFileHead), 1, fFile);
	fwrite(pSAreaUnit, sizeof(SAreaUnit), iSceneWidth * iSceneHeight, fFile);

	if (fFile)
		fclose(fFile);
	if (pSAreaUnit)
		delete[] pSAreaUnit;
	return 1;
}

long CSceneArea::CreateFileFromMap(char* ptcsMapFile, char* ptcsAreaFile) {
	long lRet = 1;

	//CreateFile(ptcsAreaFile);
	//Init(ptcsAreaFile);

	return lRet;
}

// lUnitNO ΄ΣΑγΏΚΌ
long CSceneArea::ReadAreaInfo(long lUnitNO, SAreaUnit* pUnitData, long* lpUnitNum) {
	if (!m_bInitSuccess)
		return -1;
	if (lUnitNO >= m_lUnitNum)
		return -1;
	if (lUnitNO + *lpUnitNum > m_lUnitNum)
		*lpUnitNum = m_lUnitNum - lUnitNO;

	fseek(m_fRdWr, sizeof(SFileHead) + sizeof(SAreaUnit) * lUnitNO, SEEK_SET);
	fread(pUnitData, sizeof(SAreaUnit), *lpUnitNum, m_fRdWr);

	return 1;
}

long CSceneArea::WriteAreaInfo(long lUnitNO, SAreaUnit* pUnitData, long* lpUnitNum) {
	if (!m_bInitSuccess)
		return -1;
	if (lUnitNO >= m_lUnitNum)
		return -1;
	if (lUnitNO + *lpUnitNum > m_lUnitNum)
		*lpUnitNum = m_lUnitNum - lUnitNO;

	fseek(m_fRdWr, sizeof(SFileHead) + sizeof(SAreaUnit) * lUnitNO, SEEK_SET);
	fwrite(pUnitData, sizeof(SAreaUnit), *lpUnitNum, m_fRdWr);

	return 1;
}
