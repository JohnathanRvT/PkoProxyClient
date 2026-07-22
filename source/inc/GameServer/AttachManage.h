//=============================================================================
// FileName: AttachManage.h
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CConjureMgr class
//=============================================================================

#ifndef ATTACHMANAGE_H
#define ATTACHMANAGE_H

#include "Attachable.h"
#include "GameAppNet.h"

class CConjureMgr {
public:
	CConjureMgr();
	~CConjureMgr();

	void Add(CAttachable* pCAttach);
	void Delete(CAttachable* pCAttach);
	bool SetLeader(CAttachable* pCAttach);
	CAttachable* GetLeader();
	void FreeAll();
	CAttachable* GetTail();

	void BeginGet();
	CAttachable* GetNext();

protected:
private:
	CAttachable* m_pCLstHead{nullptr}; // 链首对象也是主控对象
	CAttachable* m_pCLstTail{nullptr};

	CAttachable* m_pCur;
};

class CPassengerMgr : public dbc::PreAllocStru {
public:
	CPassengerMgr(size_t);
	~CPassengerMgr();

	void Add(CAttachable* pCAttach);
	void Delete(CAttachable* pCAttach);
	bool SetLeader(CAttachable* pCAttach);
	CAttachable* GetLeader();

	void BeginGet();
	CAttachable* GetNext();

	void FreeAll();
	void DeleteAll();

protected:
private:
	void Initially() override;
	void Finally() override;

	CAttachable* m_pCLstHead{nullptr}; // 链首对象也是主控对象
	CAttachable* m_pCLstTail{nullptr};

	long m_lNum{0};
	CAttachable* m_pCCurPess; // 用于遍历链表
};

inline void CPassengerMgr::BeginGet() {
	m_pCCurPess = m_pCLstHead;
}

inline CAttachable* CPassengerMgr::GetNext() {
	CAttachable* pRet = m_pCCurPess;

	if (m_pCCurPess)
		m_pCCurPess = m_pCCurPess->m_pCPassengerNext;

	return pRet;
}

#endif // ATTACHMANAGE_H
