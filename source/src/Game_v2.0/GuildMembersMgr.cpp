//------------------------------------------------------------------------
//	2005.4.25	Arcol	create this file
//------------------------------------------------------------------------

#include "stdafx.h"
#include "Character.h"
#include "guildmemberdata.h"
#include "guildmembersmgr.h"

std::vector<CGuildMemberData*> CGuildMembersMgr::m_pGuildMembers;

CGuildMembersMgr::CGuildMembersMgr() {
}

CGuildMembersMgr::~CGuildMembersMgr() {
}

void CGuildMembersMgr::AddGuildMember(CGuildMemberData* pGuildMember) {
	m_pGuildMembers.push_back(pGuildMember);
}

bool CGuildMembersMgr::DelGuildMember(CGuildMemberData* pGuildMember) {
	if (auto it = std::find(m_pGuildMembers.begin(), m_pGuildMembers.end(), pGuildMember);
		it != m_pGuildMembers.end()) {
		SAFE_DELETE(*it);
		m_pGuildMembers.erase(it);
		return true;
	}
	return false;
}

bool CGuildMembersMgr::DelGuildMemberByID(DWORD dwID) {
	if (auto it = std::find_if(m_pGuildMembers.begin(), m_pGuildMembers.end(),
							   [dwID](CGuildMemberData* p) { return p->GetID() == dwID; });
		it != m_pGuildMembers.end()) {
		SAFE_DELETE(*it);
		m_pGuildMembers.erase(it);
		return true;
	}
	return false;
}

bool CGuildMembersMgr::DelGuildMemberByName(std::string strName) {
	if (auto it = std::find_if(m_pGuildMembers.begin(), m_pGuildMembers.end(),
							   [&strName](CGuildMemberData* p) { return p->GetName() == strName; });
		it != m_pGuildMembers.end()) {
		SAFE_DELETE(*it);
		m_pGuildMembers.erase(it);
		return true;
	}
	return false;
}

CGuildMemberData* CGuildMembersMgr::FindGuildMemberByID(DWORD dwID) {
	auto it = std::find_if(m_pGuildMembers.begin(), m_pGuildMembers.end(),
						   [dwID](CGuildMemberData* p) { return p->GetID() == dwID; });
	return it != m_pGuildMembers.end() ? (*it) : nullptr;
}

CGuildMemberData* CGuildMembersMgr::FindGuildMemberByName(std::string strName) {
	auto it = std::find_if(m_pGuildMembers.begin(), m_pGuildMembers.end(),
						   [&strName](CGuildMemberData* p) { return p->GetName() == strName; });
	return it != m_pGuildMembers.end() ? (*it) : nullptr;
}

CGuildMemberData* CGuildMembersMgr::FindGuildMemberByIndex(DWORD dwIndex) {
	if (dwIndex >= GetTotalGuildMembers())
		return nullptr;
	return m_pGuildMembers[dwIndex];
}

DWORD CGuildMembersMgr::GetTotalGuildMembers() {
	return static_cast<DWORD>(m_pGuildMembers.size());
}

void CGuildMembersMgr::ResetAll() {
	std::vector<CGuildMemberData*>::iterator Iter;
	while (!m_pGuildMembers.empty()) {
		Iter = m_pGuildMembers.begin();
		CGuildMemberData* pNode = *Iter;
		//delete pNode;
		SAFE_DELETE(pNode); // UI当机处理
		m_pGuildMembers.erase(Iter);
	}
}

CGuildMemberData* CGuildMembersMgr::GetSelfData() {
	if (CCharacter* pCharacter = CGameScene::GetMainCha(); pCharacter) {
		return CGuildMembersMgr::FindGuildMemberByID(pCharacter->getHumanID());
	}
	return nullptr;
}
