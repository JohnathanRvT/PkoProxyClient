//=============================================================================
// FileName: Attachable.h
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CAttachable class
//=============================================================================

#ifndef ATTACHABLE_H
#define ATTACHABLE_H

#include "Entity.h"

class CPlayer;

/**
 * @class CAttachable 
 * @author 
 * @brief 设置实体的所有者等信息
 */
class CAttachable : public Entity {
	friend class CConjureMgr;
	friend class CPassengerMgr;
	friend class Entity;

public:
	CAttachable();
	void SetPlayer(CPlayer* pCPlayer) { m_pCPlayer = pCPlayer; }
	CPlayer* GetPlayer() { return m_pCPlayer; }
	void SetShip(CPassengerMgr* pCShip) { m_pCShip = pCShip; }
	CPassengerMgr* GetShip() { return m_pCShip; }
	void SetShipMaster(CAttachable* pCShipM) { m_pCShipMaster = pCShipM; }
	CAttachable* GetShipMaster() { return m_pCShipMaster; }

protected:
	void Initially();
	void Finally();

	CAttachable* IsAttachable() override { return this; }

	CPlayer* m_pCPlayer{nullptr};
	CAttachable* m_pCShipMaster{nullptr};
	CPassengerMgr* m_pCShip{nullptr};

private:
	CAttachable* m_pCConjureLast{nullptr};
	CAttachable* m_pCConjureNext{nullptr};

	CAttachable* m_pCPassengerLast{nullptr};
	CAttachable* m_pCPassengerNext{nullptr};
};

#endif // ATTACHABLE_H