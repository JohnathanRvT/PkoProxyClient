//------------------------------------------------------------------------
//	2005.5.8	Arcol	create this file
//------------------------------------------------------------------------

#include "stdafx.h"
#include "RecruitMemberData.h"
#include "recruitmembersmgr.h"

std::vector<CRecruitMemberData*> CRecruitMembersMgr::m_pRecruitMembers;

CRecruitMembersMgr::CRecruitMembersMgr() {}

CRecruitMembersMgr::~CRecruitMembersMgr() {}

void CRecruitMembersMgr::AddRecruitMember(CRecruitMemberData* pRecruitMember) {
	m_pRecruitMembers.push_back(pRecruitMember);
}

bool CRecruitMembersMgr::DelRecruitMember(CRecruitMemberData* pRecruitMember) {
	if (auto it = std::find(m_pRecruitMembers.begin(), m_pRecruitMembers.end(), pRecruitMember);
		it != m_pRecruitMembers.end()) {
		SAFE_DELETE(*it);
		m_pRecruitMembers.erase(it);
		return true;
	}
	return false;
}

bool CRecruitMembersMgr::DelRecruitMemberByID(DWORD dwID) {
	if (auto it = std::find_if(m_pRecruitMembers.begin(), m_pRecruitMembers.end(),
							   [dwID](CRecruitMemberData* p) { return p->GetID() == dwID; });
		it != m_pRecruitMembers.end()) {
		SAFE_DELETE(*it);
		m_pRecruitMembers.erase(it);
		return true;
	}
	return false;
}

bool CRecruitMembersMgr::DelRecruitMemberByName(std::string strName) {
	if (auto it = std::find_if(m_pRecruitMembers.begin(), m_pRecruitMembers.end(),
							   [&strName](CRecruitMemberData* p) { return p->GetName() == strName; });
		it != m_pRecruitMembers.end()) {
		SAFE_DELETE(*it);
		m_pRecruitMembers.erase(it);
		return true;
	}
	return false;
}

CRecruitMemberData* CRecruitMembersMgr::FindRecruitMemberByID(DWORD dwID) {
	auto it = std::find_if(m_pRecruitMembers.begin(), m_pRecruitMembers.end(),
						   [dwID](CRecruitMemberData* p) { return p->GetID() == dwID; });
	return it != m_pRecruitMembers.end() ? *it : nullptr;
}

CRecruitMemberData* CRecruitMembersMgr::FindRecruitMemberByName(std::string strName) {
	auto it = std::find_if(m_pRecruitMembers.begin(), m_pRecruitMembers.end(),
						   [&strName](CRecruitMemberData* p) { return p->GetName() == strName; });
	return it != m_pRecruitMembers.end() ? *it : nullptr;
}

CRecruitMemberData* CRecruitMembersMgr::FindRecruitMemberByIndex(DWORD dwIndex) {
	if (dwIndex >= GetTotalRecruitMembers())
		return nullptr;
	return m_pRecruitMembers[dwIndex];
}

DWORD CRecruitMembersMgr::GetTotalRecruitMembers() {
	return static_cast<DWORD>(m_pRecruitMembers.size());
}

void CRecruitMembersMgr::ResetAll() {
	std::vector<CRecruitMemberData*>::iterator Iter;
	while (!m_pRecruitMembers.empty()) {
		Iter = m_pRecruitMembers.begin();
		CRecruitMemberData* pNode = *Iter;
		//delete pNode;
		SAFE_DELETE(pNode); // UI当机处理
		m_pRecruitMembers.erase(Iter);
	}
}
