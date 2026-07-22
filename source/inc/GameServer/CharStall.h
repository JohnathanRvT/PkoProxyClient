// CharStall.h Created by knight-gongjian 2005.8.29.
//---------------------------------------------------------
#pragma once

#ifndef _CHARSTALL_H_
#define _CHARSTALL_H_

#include "Character.h"
//---------------------------------------------------------

namespace mission {
typedef struct _STALL_GOODS {
	DWORD dwMoney;
	BYTE byGrid;
	BYTE byIndex;
	WORD wCount;
	USHORT sItemID;

} STALL_GOODS, *PSTALL_GOODS;

class CStallSystem;
class CStallData : public dbc::PreAllocStru {
	friend CStallSystem;

public:
	CStallData(size_t lSize);
	virtual ~CStallData();

	virtual void Initially() override { Clear(); }
	virtual void Finally() override { Clear(); }

private:
	void Clear();

	BYTE m_byNum;
	char m_szName[ROLE_MAXNUM_STALL_NUM];
	STALL_GOODS m_Goods[ROLE_MAXNUM_STALL_GOODS];
};

class CStallSystem {
public:
	CStallSystem();
	~CStallSystem();

	void StartStall(CCharacter& staller, RPACKET& packet);
	void CloseStall(CCharacter& staller);
	void OpenStall(CCharacter& character, RPACKET& packet);
	void BuyGoods(CCharacter& character, RPACKET& packet);

private:
	void SyncData(CCharacter& character, CCharacter& staller);
	void DelGoods(CCharacter& staller, BYTE byGrid, WORD wCount);
};
} // namespace mission

extern mission::CStallSystem g_StallSystem;

//---------------------------------------------------------
#endif // _CHARSTALL_H_