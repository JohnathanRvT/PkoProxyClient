//=============================================================================
// FileName: EyeshotCell.h
// Creater: ZhangXuedong
// Date: 2005.04.29
// Comment: Map Eyeshot Cell
//=============================================================================

#ifndef EYESHOTCELL_H
#define EYESHOTCELL_H

#include "Character.h"
#include "Entity.h"
#include "Item.h"
#include "StateCell.h"

// 视野单元类，纪录单元内的所有角色和道具，以及状态单元体
class CEyeshotCell {
public:
	CEyeshotCell();
	~CEyeshotCell();

	void AddEntity(CCharacter* pCCha);
	void AddEntity(CItem* pCItem);
	void DelEntity(Entity* pCEnt);
	long GetEntityNum() { return m_lChaCount + m_lItemCount; }
	long GetChaNum() { return m_lChaCount; }
	long GetItemNum() { return m_lItemCount; }

	void EnterEyeshot(Entity* pCEnt);
	void OutEyeshot(Entity* pCEnt);
	void RefreshEyeshot(Entity* pCEnt, bool bToEyeshot, bool bToNoHide, bool bToNoShow);

	void BeginGetCha() { m_pCChaSearch = m_pCChaL; } // 开始取单元内的角色.
	CCharacter* GetNextCha();						 // 取单元内的下一个角色

public:
	short m_sPosX; // 位置
	short m_sPosY;
	long m_lActNum{0};			   // 激活计数
	CCharacter* m_pCChaL{nullptr}; // 角色链
	CItem* m_pCItemL{nullptr};	 // 道具链

	CEyeshotCell* m_pCNext{nullptr}; // 指向“激活的管理单元链表“的指针
	CEyeshotCell* m_pCLast{nullptr};

	CStateCell*** m_pCStateCell{nullptr}; // 视野单元包含的状态单元，一旦视野被激活，则包含的状态单元也会被激活，相反某一状态单元的激活，也会激活视野
	short m_sStateCellNum{0};

private:
	long m_lChaCount{0};
	long m_lItemCount{0};
	CCharacter* m_pCChaSearch;
};

inline void CEyeshotCell::AddEntity(CCharacter* pCCha) {
	if (!pCCha) {
		return;
	}
	if (pCCha->m_pCEyeshotCellLast || pCCha->m_pCEyeshotCellNext) {
		LG("eyeshot cell operator error", " when add character entity to eyeshot cell %s ,find it is not break away foregone manage cell", pCCha->GetLogName());
		return;
	}

	pCCha->m_pCEyeshotCellLast = nullptr;
	pCCha->m_pCEyeshotCellNext = m_pCChaL;
	if (pCCha->m_pCEyeshotCellNext) {
		pCCha->m_pCEyeshotCellNext->m_pCEyeshotCellLast = pCCha;
	}
	m_pCChaL = pCCha;

	m_lChaCount++;
}

inline void CEyeshotCell::AddEntity(CItem* pCItem) {
	if (!pCItem) {
		return;
	}
	if (pCItem->m_pCEyeshotCellLast || pCItem->m_pCEyeshotCellNext) {
		LG("eyeshot cell operator error", "when add item entity to  %s ，find it is not break away foregone manage cell", pCItem->GetLogName());
		return;
	}

	pCItem->m_pCEyeshotCellLast = nullptr;
	pCItem->m_pCEyeshotCellNext = m_pCItemL;
	if (pCItem->m_pCEyeshotCellNext) {
		pCItem->m_pCEyeshotCellNext->m_pCEyeshotCellLast = pCItem;
	}
	m_pCItemL = pCItem;

	m_lItemCount++;
}

inline void CEyeshotCell::DelEntity(Entity* pCEnt) {
	if (!pCEnt) {
		return;
	}
	if (pCEnt->IsCharacter()) {
		if (m_pCChaSearch == pCEnt) {
			m_pCChaSearch = pCEnt->m_pCEyeshotCellNext ? pCEnt->m_pCEyeshotCellNext->IsCharacter() : nullptr;
		}
	}

	if (pCEnt->m_pCEyeshotCellLast) {
		pCEnt->m_pCEyeshotCellLast->m_pCEyeshotCellNext = pCEnt->m_pCEyeshotCellNext;
	}
	if (pCEnt->m_pCEyeshotCellNext) {
		pCEnt->m_pCEyeshotCellNext->m_pCEyeshotCellLast = pCEnt->m_pCEyeshotCellLast;
	}

	if (m_pCChaL == pCEnt) {
		if (pCEnt->m_pCEyeshotCellNext) {
			m_pCChaL = pCEnt->m_pCEyeshotCellNext->IsCharacter();
			m_pCChaL->m_pCEyeshotCellLast = nullptr;
		} else {
			m_pCChaL = nullptr;
		}
	} else if (m_pCItemL == pCEnt) {
		if (pCEnt->m_pCEyeshotCellNext) {
			m_pCItemL = pCEnt->m_pCEyeshotCellNext->IsItem();
			m_pCItemL->m_pCEyeshotCellLast = nullptr;
		} else {
			m_pCItemL = nullptr;
		}
	}

	pCEnt->m_pCEyeshotCellLast = nullptr;
	pCEnt->m_pCEyeshotCellNext = nullptr;

	if (pCEnt->IsCharacter()) {
		m_lChaCount--;
	} else {
		m_lItemCount--;
	}
}

//=============================================================================
// 激活的视野单元链表
class CActEyeshotCell {
public:
	CActEyeshotCell() {}

	void Add(CEyeshotCell* pObj);
	void Del(CEyeshotCell* pObj);

	void BeginGet();		 // 开始取激活单元.
	CEyeshotCell* GetNext(); // 取下一个激活单元.
	CEyeshotCell* GetCurrent();

	long GetActiveNum() { return m_lCount; }

protected:
private:
	CEyeshotCell* m_pHead{nullptr};

	CEyeshotCell* m_pCur{nullptr};

	long m_lCount{0};
};

inline void CActEyeshotCell::Add(CEyeshotCell* pObj) {
	pObj->m_pCLast = nullptr;
	if (pObj->m_pCNext = m_pHead) {
		m_pHead->m_pCLast = pObj;
	}
	m_pHead = pObj;

	m_lCount++;
}

inline void CActEyeshotCell::Del(CEyeshotCell* pObj) {
	if (!pObj) {
		return;
	}
	if (m_pCur == pObj) {
		m_pCur = pObj->m_pCNext;
	}

	if (pObj->m_pCLast) {
		pObj->m_pCLast->m_pCNext = pObj->m_pCNext;
	}
	if (pObj->m_pCNext) {
		pObj->m_pCNext->m_pCLast = pObj->m_pCLast;
	}
	if (m_pHead == pObj) {
		if (m_pHead = pObj->m_pCNext) {
			m_pHead->m_pCLast = nullptr;
		}
	}
	pObj->m_pCNext = nullptr;
	pObj->m_pCLast = nullptr;

	m_lCount--;
}

inline void CActEyeshotCell::BeginGet() {
	m_pCur = m_pHead;
}

inline CEyeshotCell* CActEyeshotCell::GetNext() {
	CEyeshotCell* pRet = m_pCur;

	if (m_pCur) {
		m_pCur = m_pCur->m_pCNext;
	}

	return pRet;
}

inline CEyeshotCell* CActEyeshotCell::GetCurrent() {
	return m_pCur;
}

#endif // EYESHOTCELL_H