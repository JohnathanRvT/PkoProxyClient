#include "stdafx.h"
#include "UITeam.h"
#include "GuildMemberData.h"
#include "GuildMembersMgr.h"
#include "UIGuildMgr.h"
#include "NetChat.h"
#include "TalkSessionFormMgr.h"
#include "StringLib.h"
#include "UIChat.h"

CMemberData::CMemberData() {}

CMemberData::~CMemberData() {}

void CMemberData::SetFace(const stNetTeamChaPart& pFace) {
	if (!_pFace) {
		_pFace = std::make_unique<stNetTeamChaPart>();
	}
	//TODO: Copy assignment?
	memcpy(_pFace.get(), &pFace, sizeof(stNetTeamChaPart));
}

//---------------------------------------------------------------------------
// CMember
//---------------------------------------------------------------------------

eShowStyle CMember::_nShowStyle = enumShowQQName;

CMember::CMember(CTeam* pTeam, unsigned long id, const char* strName, const char* strMotto, DWORD icon_id, const char* szGroupName)
	: _nID(id), _pTeam(pTeam), _strName(strName), _strMotto(strMotto), _nIcon_id(icon_id), _strGroupName(szGroupName) {
	if (pTeam->GetStyle() == enumTeamGroup) {
		_pData = std::make_unique<CMemberData>();
	}
}

CMember::~CMember() {
	g_stUIChat.TeamSend(enumSTM_DEL_MEMBER, this, _pTeam->GetStyle());
}

void CMember::Refresh() {
	g_stUIChat.TeamSend(enumSTM_NODE_DATA_CHANGE, this, _pTeam->GetStyle());
}

void CMember::SetName(const char* str) {
	_strName = str;
	g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, this, _pTeam->GetStyle());
}

void CMember::SetMotto(const char* str) {
	_strMotto = str;
	g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, this, _pTeam->GetStyle());
}

void CMember::SetShowStyle(eShowStyle style) {
	_nShowStyle = style;
}

void CMember::ModifyAttr(std::string motto, std::string job, DWORD dwLv, DWORD nIcon_ID, std::string guildName) {
	_strMotto = motto;
	_strJob = job;
	_dwLv = dwLv;
	_nIcon_id = nIcon_ID;
	_strGuildName = guildName;
	g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, this, _pTeam->GetStyle());
}

void CMember::SetJobName(const char* str) {
	_strJob = str;
	g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, this, _pTeam->GetStyle());
}

void CMember::SetIconID(DWORD iconId) {
	_nIcon_id = iconId;
	g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, this, _pTeam->GetStyle());
}

void CMember::SetOnline(bool isOnline) {
	_bIsOnline = isOnline;
	g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, this, _pTeam->GetStyle());
}

const char* CMember::GetShowName() {
	_strShowName = GetName();
	const std::string strMotto = GetMotto();
	if (!strMotto.empty()) {
		_strShowName += "(" + strMotto + ")";
	}
	_strShowName = StringLimit(_strShowName, 14);
	return _strShowName.c_str();
}

//---------------------------------------------------------------------------
// CTeam
//---------------------------------------------------------------------------
CTeam::CTeam(eTeamStyle nStyle, const char* szName)
	: _eStyle(nStyle), _strName(szName) {
	g_stUIChat.TeamSend(enumSTM_ADD_GROUP, this, nStyle);
}

CTeam::~CTeam() {
	Clear();
}

CMember* CTeam::Find(unsigned long nID) {
	if (auto it = std::find_if(_member.begin(), _member.end(),
							   [nID](CMember* p) { return p->GetID() == nID; });
		it != _member.end()) {
		return *it;
	}
	return nullptr;
}

CMember* CTeam::Add(unsigned long nID, const char* szName, const char* szMotto, DWORD icon_id, const char* szGroupName, bool onLine) {
	if (nID > 0) {
		_nCount++;
		const std::string strMotto = szMotto ? szMotto : RES_STRING(CL_LANGUAGE_MATCH_778);

		auto* tmp = new CMember(this, nID, szName, strMotto.c_str(), icon_id, szGroupName);
		tmp->SetOnline(onLine);

		bool newFrndFG = true;
		if (auto pMember = g_stUIChat.GetTeamMgr()->Find(enumTeamFrnd)->Find(nID); pMember) {
			newFrndFG = false;
			pMember->SetMotto(strMotto.c_str());
		}
		if (auto pMember = g_stUIChat.GetTeamMgr()->Find(enumTeamMaster)->Find(nID); pMember) {
			pMember->SetMotto(strMotto.c_str());
		}
		if (auto pMember = g_stUIChat.GetTeamMgr()->Find(enumTeamPrentice)->Find(nID); pMember) {
			pMember->SetMotto(strMotto.c_str());
		}
		if (auto pMember = g_stUIChat.GetTeamMgr()->Find(enumTeamGroup)->Find(nID); pMember) {
			pMember->SetMotto(strMotto.c_str());
		}

		if (CGuildMemberData* pGuildMember = CGuildMembersMgr::FindGuildMemberByID(nID); pGuildMember) {
			if (szMotto) {
				pGuildMember->SetMottoName(szMotto);
				if (auto pItem = static_cast<CTextGraph*>(pGuildMember->GetPointer()); pItem) {
					std::string str = pGuildMember->GetName();
					if (strlen(szMotto) > 0) {
						str += "(" + std::string(szMotto) + ")";
					}
					pItem->SetHint(str.c_str());
					str = StringLimit(str, 14);
					pItem->SetString(str.c_str());
				}
			}
			CUIGuildMgr::RefreshList();
		}
		CTalkSessionFormMgr::RefreshSessionMember(nID, strMotto.c_str());
		_member.push_back(tmp);

		if (newFrndFG &&
			(strcmp(tmp->GetGroupName(), RES_STRING(CL_LANGUAGE_MATCH_466)) != 0)) {
			AddGroupName(tmp->GetGroupName());
		}

		g_stUIChat.TeamSend(enumSTM_ADD_MEMBER, tmp, this->GetStyle());
		return tmp;
	}
	return nullptr;
}

bool CTeam::Del(unsigned long nID) {
	if (auto it = std::find_if(_member.begin(), _member.end(),
							   [nID](CMember* p) { return p->GetID() == nID; });
		it != _member.end()) {
		--_nCount;
		SAFE_DELETE(*it);
		_member.erase(it);
		g_stUIChat.TeamSend(enumSTM_AFTER_DEL_MEMBER, nullptr, GetStyle());
		return true;
	}
	return false;
}

void CTeam::Clear() {
	_nCount = 0;
	for (auto p : _member) {
		SAFE_DELETE(p);
	}
	_member.clear();

	ClearGroupName();

	g_stUIChat.TeamSend(enumSTM_DEL_GROUP, this, _eStyle);
}

CMember* CTeam::GetMember(unsigned long nIndex) {
	if (nIndex >= _nCount) {
		return nullptr;
	}

	unsigned int n = 0;
	for (auto p : _member) {
		if (n == nIndex) {
			return p;
		}
		n++;
	}
	return nullptr;
}

const char* CTeam::GetGroupName(int index) const {
	if (index < 0 || index >= _groups.size()) {
		return nullptr;
	}
	return _groups[index].first.c_str();
}

void CTeam::AddGroupName(const char* szGroupName) {
	if (strcmp(szGroupName, "") == 0) {
		//::MessageBox(NULL,"AddGroupName","NULL",0);
		return;
	}

	bool exsitFG = false;

	//NOTE: Not sure if counter is correct description but it seems to be
	for (auto& [name, counter] : _groups) {
		if (strcmp(name.c_str(), szGroupName) == 0) {
			++counter;
			exsitFG = true;
			break;
		}
	}
	if (!exsitFG) {
		_groups.emplace_back(szGroupName, 1);

		g_stUIChat.TeamSend(enumSTM_ADD_FRIEND_GROUP, (void*)szGroupName, 0);
	}
}

void CTeam::DelGroupName(const char* szGroupName) {
	if (auto it = std::find_if(_groups.begin(), _groups.end(),
							   [szGroupName](const std::pair<std::string, int>& group) {
								   return strcmp(group.first.c_str(), szGroupName) == 0;
							   });
		it != _groups.end()) {
		g_stUIChat.DelFrndGroup(szGroupName);
		//it->second --;
		//if(it->second ==0)
		{
			_groups.erase(it);
			g_stUIChat.TeamSend(enumSTM_DEL_FRIEND_GROUP, (void*)szGroupName, 0);
		}
	}
}

void CTeam::ChangeGroupName(const char* szOldGroupName, const char* szNewGroupName) {
	for (auto p : _member) {
		if (strcmp(p->GetGroupName(), szOldGroupName) == 0) {
			p->SetGroupName(szNewGroupName);
		}
	}

	if (auto it = std::find_if(_groups.begin(), _groups.end(),
							   [szOldGroupName](std::pair<std::string, int>& group) {
								   return strcmp(group.first.c_str(), szOldGroupName) == 0;
							   });
		it != _groups.end()) {
		it->first = szNewGroupName;
		g_stUIChat.ChangeFrndGroup(szOldGroupName, szNewGroupName);
	}
}

void CTeam::MoveGroup(unsigned long nID, const char* szOldGroupName, const char* szNewGroupName) {

	if (auto it = std::find_if(_member.begin(), _member.end(),
							   [nID](CMember* p) { return p->GetID() == nID; });
		it != _member.end()) {
		auto pMember = new CMember(this, (*it)->GetID(), (*it)->GetName(), (*it)->GetMotto(), (*it)->GetIconID(), (*it)->GetGroupName());
		const bool isOnline = (*it)->IsOnline();

		--_nCount;
		SAFE_DELETE(*it);
		_member.erase(it);

		g_stUIChat.TeamSend(enumSTM_AFTER_DEL_MEMBER, nullptr, GetStyle());

		if (pMember) {
			this->Add(pMember->GetID(), pMember->GetName(), pMember->GetMotto(), pMember->GetIconID(), szNewGroupName, isOnline);
		}
	}
}

void CTeam::ClearGroupName() {
	for (auto& [name, counter] : _groups) {
		g_stUIChat.DelFrndGroup(name.c_str());
	}
	_groups.clear();
}

// End

//---------------------------------------------------------------------------
// CTeamMgr
//---------------------------------------------------------------------------
DWORD CTeamMgr::_dwTeamLeaderID = 0;

CTeamMgr::CTeamMgr()
	: _pFrndTeam(std::make_unique<CTeam>(enumTeamFrnd, RES_STRING(CL_LANGUAGE_MATCH_466))),
	  _pGroupTeam(std::make_unique<CTeam>(enumTeamGroup, RES_STRING(CL_LANGUAGE_MATCH_299))),
	  _pRoadTeam(std::make_unique<CTeam>(enumTeamRoad, RES_STRING(CL_LANGUAGE_MATCH_469))),
	  _pMasterTeam(std::make_unique<CTeam>(enumTeamMaster, RES_STRING(CL_LANGUAGE_MATCH_850))),
	  _pPrenticeTeam(std::make_unique<CTeam>(enumTeamPrentice, RES_STRING(CL_LANGUAGE_MATCH_851))) {}

CTeamMgr::~CTeamMgr() {
}

CTeam* CTeamMgr::Add(eTeamStyle eTeam, const char* szName) {
	switch (eTeam) {
	case enumTeamFrnd: {
		_pFrndTeam->Clear();
		return _pFrndTeam.get();
	}
	case enumTeamGroup: {
		_pGroupTeam->Clear();
		return _pGroupTeam.get();
	}
	case enumTeamRoad: {
		_pRoadTeam->Clear();
		return _pRoadTeam.get();
	}
	case enumTeamMaster: {
		_pMasterTeam->Clear();
		return _pMasterTeam.get();
	}
	case enumTeamPrentice: {
		_pPrenticeTeam->Clear();
		return _pPrenticeTeam.get();
	}
	}
	return nullptr;
}

bool CTeamMgr::Del(eTeamStyle eTeam, const char* szName) {
	switch (eTeam) {
	case enumTeamFrnd: {
		_pFrndTeam->Clear();
		return true;
	}
	case enumTeamGroup: {
		_dwTeamLeaderID = 0;

		_pGroupTeam->Clear();
		return true;
	}
	case enumTeamRoad: {
		_pRoadTeam->Clear();
		return true;
	}
	case enumTeamMaster: {
		_pMasterTeam->Clear();
		return true;
	}
	case enumTeamPrentice: {
		_pPrenticeTeam->Clear();
		return true;
	}
	}
	return false;
}

CTeam* CTeamMgr::Find(eTeamStyle eTeam, const char* szName) {
	switch (eTeam) {
	case enumTeamFrnd: {
		return _pFrndTeam.get();
	}
	case enumTeamGroup: {
		return _pGroupTeam.get();
	}
	case enumTeamRoad: {
		return _pRoadTeam.get();
	}
	case enumTeamMaster: {
		return _pMasterTeam.get();
	}
	case enumTeamPrentice: {
		return _pPrenticeTeam.get();
	}
	}
	return nullptr;
}

void CTeamMgr::ChangeStyle(eShowStyle style) {
	CMember::SetShowStyle(style);
	for (DWORD i = 0; i < _pFrndTeam->GetCount(); i++) {
		DWORD j = _pFrndTeam->GetCount();
		g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, _pFrndTeam->GetMember(i), enumTeamFrnd);
	}
	for (DWORD i = 0; i < _pGroupTeam->GetCount(); i++) {
		g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, _pGroupTeam->GetMember(i), enumTeamGroup);
	}
	for (DWORD i = 0; i < _pRoadTeam->GetCount(); i++) {
		g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, _pRoadTeam->GetMember(i), enumTeamRoad);
	}
	for (DWORD i = 0; i < _pMasterTeam->GetCount(); i++) {
		g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, _pMasterTeam->GetMember(i), enumTeamMaster);
	}
	for (DWORD i = 0; i < _pPrenticeTeam->GetCount(); i++) {
		g_stUIChat.TeamSend(enumSTM_NODE_CHANGE, _pPrenticeTeam->GetMember(i), enumTeamPrentice);
	}
}

void CTeamMgr::SceneSwitch() {
	_pRoadTeam->Clear();
	g_stUIChat._curSelectMember = nullptr;
}

void CTeamMgr::ResetAll() {
	_pRoadTeam->Clear();
	_pGroupTeam->Clear();
	_pFrndTeam->Clear();
	_pMasterTeam->Clear();
	_pPrenticeTeam->Clear();

	g_stUIChat._curSelectMember = nullptr;
	g_stUIChat._bForbid = false;
}
