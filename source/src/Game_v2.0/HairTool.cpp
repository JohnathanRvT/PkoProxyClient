#include "StdAfx.h"
#include "hairtool.h"
#include "HairRecord.h"

//---------------------------------------------------------------------------
// class CHairName
//---------------------------------------------------------------------------
CHairName::CHairName(CHairRecord* pInfo)
	: _szName(pInfo->szDataName) {
	AddInfo(pInfo);
}

//---------------------------------------------------------------------------
// class CHairName
//---------------------------------------------------------------------------
CHairTools::~CHairTools() {
	_Clear();
}

void CHairTools::_Clear() {
	for (auto* hair : _hairs)
		delete hair;

	_hairs.clear();
}

bool CHairTools::RefreshCha(DWORD dwChaID) {
	_Clear();
	if (dwChaID == 0 || dwChaID > 4)
		return false;
	dwChaID--;

	int nCount = CHairRecordSet::I()->GetLastID() + 1;
	CHairRecord* pInfo = nullptr;
	for (int i = 0; i < nCount; i++) {
		pInfo = GetHairRecordInfo(i);
		if (!pInfo)
			continue;

		if (!pInfo->IsChaUse[dwChaID])
			continue;

		_AddInfo(pInfo);
	}
	return true;
}

void CHairTools::_AddInfo(CHairRecord* pInfo) {
	for (auto* hair : _hairs) {
		if (strcmp(hair->GetName(), pInfo->szDataName) == 0) {
			hair->AddInfo(pInfo);
			return;
		}
	}

	_hairs.push_back(new CHairName(pInfo));
}
