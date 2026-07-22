//=============================================================================
// FileName: AttachManage.cpp
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CConjureMgr class
//=============================================================================

#include "stdafx.h"
#include "AttachManage.h"
#include "SubMap.h"
#include "GameAppNet.h"

_DBC_USING;

//=============================================================================
CConjureMgr::CConjureMgr() {}

CConjureMgr::~CConjureMgr() {
	m_pCLstHead = nullptr;
	m_pCLstTail = nullptr;
}

void CConjureMgr::Add(CAttachable* pCAttach) {
	try {
		if (m_pCLstTail) {
			pCAttach->m_pCConjureLast = m_pCLstTail;
			pCAttach->m_pCConjureNext = nullptr;
			m_pCLstTail->m_pCConjureNext = pCAttach;
			m_pCLstTail = pCAttach;
		} else { // ¿ÕÁ´
			pCAttach->m_pCConjureLast = nullptr;
			pCAttach->m_pCConjureNext = nullptr;
			m_pCLstHead = pCAttach;
			m_pCLstTail = pCAttach;
		}
	}
	T_E
}

void CConjureMgr::Delete(CAttachable* pCAttach) {
	try {
		if (pCAttach) {
			if (m_pCur == pCAttach) {
				m_pCur = pCAttach->m_pCConjureNext;
			}
			if (pCAttach->m_pCConjureLast) {
				pCAttach->m_pCConjureLast->m_pCConjureNext = pCAttach->m_pCConjureNext;
			}
			if (pCAttach->m_pCConjureNext) {
				pCAttach->m_pCConjureNext->m_pCConjureLast = pCAttach->m_pCConjureLast;
			}
			if (pCAttach == m_pCLstHead) {
				if (m_pCLstHead = pCAttach->m_pCConjureNext) {
					m_pCLstHead->m_pCConjureLast = nullptr;
				}
			}
			if (pCAttach == m_pCLstTail) {
				if (m_pCLstTail = pCAttach->m_pCConjureLast) {
					m_pCLstTail->m_pCConjureNext = nullptr;
				}
			}
			pCAttach->m_pCConjureLast = nullptr;
			pCAttach->m_pCConjureNext = nullptr;
		}
	}
	T_E
}

bool CConjureMgr::SetLeader(CAttachable* pCAttach) {
	try {
		bool bRet = false;

		if (!m_pCLstHead) {
			bRet = false;
		} else if (pCAttach != m_pCLstHead) {
			if (pCAttach == m_pCLstTail) {
				m_pCLstTail = pCAttach->m_pCConjureLast;
				m_pCLstTail->m_pCConjureNext = nullptr;
				pCAttach->m_pCConjureLast = nullptr;
				pCAttach->m_pCConjureNext = m_pCLstHead;
				m_pCLstHead = pCAttach;
				bRet = true;
			} else if (pCAttach->m_pCConjureLast && pCAttach->m_pCConjureNext) {
				pCAttach->m_pCConjureLast->m_pCConjureNext = pCAttach->m_pCConjureNext;
				pCAttach->m_pCConjureNext->m_pCConjureLast = pCAttach->m_pCConjureLast;
				pCAttach->m_pCConjureLast = nullptr;
				pCAttach->m_pCConjureNext = m_pCLstHead;
				m_pCLstHead = pCAttach;
				bRet = true;
			} else {
				bRet = false;
			}
		} else {
			bRet = true;
		}

		return bRet;
	}
	T_E
}

CAttachable* CConjureMgr::GetLeader() {
	try {
		//CAttachable *pCAttach;

		return m_pCLstHead;
		//if (!IsBadReadPtr(m_pCLstHead, 4))
		//	pCAttach = m_pCLstHead;
		//else
		//{
		//	LG("error", "msgCConjureMgr::GetLeader() m_pCLstHead = %p\n", m_pCLstHead);
		//}

		//return pCAttach;
	}
	T_E
}

void CConjureMgr::FreeAll() {
	try {
		CAttachable* pCTemp;
		SubMap* pSubMap;
		while (pCTemp = m_pCLstHead) {
			pCTemp->m_pCConjureLast = nullptr;
			pCTemp->m_pCConjureNext = nullptr;
			pSubMap = pCTemp->GetSubMap();
			if (pSubMap) {
				pSubMap->GoOut(pCTemp);
			}
			m_pCLstHead = m_pCLstHead->m_pCConjureNext;
			pCTemp->Free();
		}
		m_pCLstHead = nullptr;
		m_pCLstTail = nullptr;
	}
	T_E
}

CAttachable* CConjureMgr::GetTail() {
	CAttachable* pCAttach;

	pCAttach = m_pCLstTail;

	return pCAttach;
}

void CConjureMgr::BeginGet() {
	m_pCur = m_pCLstHead;
}

CAttachable* CConjureMgr::GetNext() {
	CAttachable* pRet = m_pCur;

	if (m_pCur) {
		m_pCur = m_pCur->m_pCConjureNext;
	}

	return pRet;
}

//=============================================================================
CPassengerMgr::CPassengerMgr(size_t) : PreAllocStru(1) {}

CPassengerMgr::~CPassengerMgr() {
}

void CPassengerMgr::Initially() {
	m_pCLstHead = nullptr;
	m_pCLstTail = nullptr;
	m_lNum = 0;
}

void CPassengerMgr::Finally() {
	DeleteAll();
}

void CPassengerMgr::Add(CAttachable* pCAttach) {
	try {
		if (m_pCLstTail) {
			pCAttach->m_pCPassengerLast = m_pCLstTail;
			pCAttach->m_pCPassengerNext = nullptr;
			m_pCLstTail->m_pCPassengerNext = pCAttach;
			m_pCLstTail = pCAttach;
		} else { // ¿ÕÁ´

			pCAttach->m_pCPassengerLast = nullptr;
			pCAttach->m_pCPassengerNext = nullptr;
			m_pCLstHead = pCAttach;
			m_pCLstTail = pCAttach;
		}
		m_lNum++;
	}
	T_E
}

void CPassengerMgr::Delete(CAttachable* pCAttach) {
	try {
		if (pCAttach && m_pCLstHead) {
			if (m_pCCurPess == pCAttach) {
				m_pCCurPess = pCAttach->m_pCPassengerNext;
			}
			if (pCAttach->m_pCPassengerLast) {
				pCAttach->m_pCPassengerLast->m_pCPassengerNext = pCAttach->m_pCPassengerNext;
			}
			if (pCAttach->m_pCPassengerNext) {
				pCAttach->m_pCPassengerNext->m_pCPassengerLast = pCAttach->m_pCPassengerLast;
			}
			if (pCAttach == m_pCLstHead)
				if (m_pCLstHead = m_pCLstHead->m_pCPassengerNext) {
					m_pCLstHead->m_pCPassengerLast = nullptr;
				}
			if (pCAttach == m_pCLstTail) {
				if (m_pCLstTail = m_pCLstTail->m_pCPassengerLast) {
					m_pCLstTail->m_pCPassengerNext = nullptr;
				}
			}
			pCAttach->m_pCPassengerLast = pCAttach->m_pCPassengerNext = nullptr;

			m_lNum--;
		}
	}
	T_E
}

bool CPassengerMgr::SetLeader(CAttachable* pCAttach) {
	try {
		bool bRet = false;

		if (!m_pCLstHead) {
			bRet = false;
		} else if (pCAttach == m_pCLstHead) {
			bRet = true;
		} else if (pCAttach == m_pCLstTail) {
			m_pCLstTail = pCAttach->m_pCPassengerLast;
			m_pCLstTail->m_pCPassengerNext = nullptr;
			pCAttach->m_pCPassengerLast = nullptr;
			pCAttach->m_pCPassengerNext = m_pCLstHead;
			m_pCLstHead = pCAttach;
			bRet = true;
		} else if (pCAttach->m_pCPassengerLast && pCAttach->m_pCPassengerNext) {
			pCAttach->m_pCPassengerLast->m_pCPassengerNext = pCAttach->m_pCPassengerNext;
			pCAttach->m_pCPassengerNext->m_pCPassengerLast = pCAttach->m_pCPassengerLast;
			pCAttach->m_pCPassengerLast = nullptr;
			pCAttach->m_pCPassengerNext = m_pCLstHead;
			m_pCLstHead = pCAttach;
			bRet = true;
		} else
			bRet = false;

		return bRet;
	}
	T_E
}

CAttachable* CPassengerMgr::GetLeader() {
	try {
		CAttachable* pCAttach;

		pCAttach = m_pCLstHead;

		return pCAttach;
	}
	T_E
}

void CPassengerMgr::FreeAll() {
	try {
		CAttachable* pCTemp;
		SubMap* pSubMap;
		while (pCTemp = m_pCLstHead) {
			pCTemp->m_pCPassengerLast = nullptr;
			pCTemp->m_pCPassengerNext = nullptr;
			pSubMap = pCTemp->GetSubMap();
			if (pSubMap) {
				pSubMap->GoOut(pCTemp);
			}
			m_pCLstHead = m_pCLstHead->m_pCPassengerNext;
			pCTemp->Free();
		}
		m_pCLstHead = nullptr;
		m_pCLstTail = nullptr;
	}
	T_E
}

void CPassengerMgr::DeleteAll() {
	try {
		CAttachable* pCTemp;
		while (pCTemp = m_pCLstHead) {
			m_pCLstHead = m_pCLstHead->m_pCPassengerNext;
			pCTemp->m_pCPassengerLast = nullptr;
			pCTemp->m_pCPassengerNext = nullptr;
		}
		m_pCLstHead = nullptr;
		m_pCLstTail = nullptr;
	}
	T_E
}
