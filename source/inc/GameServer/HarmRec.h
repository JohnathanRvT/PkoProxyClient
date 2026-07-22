#pragma once

// 仇恨度系统 - 伤害值管理
class CCharacter;

#define MAX_HARM_REC 5
#define MAX_VALID_CNT 8

extern BOOL g_bLogHarmRec;

struct SHarmRec // 伤害记录
{
	CCharacter* pAtk{nullptr}; // 攻击者指针
	DWORD sHarm{0};			   // 累加伤害值
	DWORD sHate{0};			   // 仇恨度
	BYTE btValid{0};		   // 是否有效
	DWORD dwID{0};
	DWORD dwTime{0}; // 第一次攻击的时间

	BOOL IsChaValid() { // 返回一个角色是否还有效
		if (!pAtk) {
			return FALSE;
		}
		if (g_pGameApp->IsLiveingEntity(dwID, pAtk->GetHandle()) == nullptr) {
			return FALSE;
		}
		return TRUE;
	}
};

class CHateMgr {
public:
	CHateMgr() {}

	CCharacter* GetCurTarget();
	void AddHarm(CCharacter* pAtk, short sHarm, DWORD dwID);
	void AddHate(CCharacter* pAtk, short sHate, DWORD dwID);
	void UpdateHarmRec(CCharacter* pSelf);
	void ClearHarmRec();
	void ClearHarmRecByCha(CCharacter* pAtk);
	void DebugNotice(CCharacter* pSelf);
	SHarmRec* GetHarmRec(int nNo) { return &_HarmRec[nNo]; }

protected:
	SHarmRec _HarmRec[MAX_HARM_REC];
	DWORD _dwLastDecValid{0};
	DWORD _dwLastSortTick{0};
};

inline CCharacter* CHateMgr::GetCurTarget() {
	auto pIt = std::find_if(std::begin(_HarmRec), std::end(_HarmRec), [](SHarmRec& harm) {
		return (harm.btValid > 0 && harm.IsChaValid()) ? true : false;
	});
	return pIt != std::end(_HarmRec) ? pIt->pAtk : nullptr;
}

inline void CHateMgr::ClearHarmRec() {
	// 已有角色累加伤害
	for (auto& it : _HarmRec) {
		it.btValid = 0;
		it.dwID = 0;
		it.sHarm = 0;
		it.sHate = 0;
		it.dwTime = 0;
		it.pAtk = nullptr;
	}
}

inline void CHateMgr::ClearHarmRecByCha(CCharacter* pAtk) {
	if (!pAtk) {
		return;
	}

	for (auto& it : _HarmRec) {
		if (it.pAtk == pAtk) {
			it.btValid = 0;
			it.dwID = 0;
			it.sHarm = 0;
			it.sHate = 0;
			it.dwTime = 0;
			it.pAtk = nullptr;
		}
	}
}

inline void CHateMgr::AddHarm(CCharacter* pAtk, short sHarm, DWORD dwID) {
	if (g_bLogHarmRec) {
		LG("harm", "begin to add harm, attacker[%s], harm%d\n",
		   pAtk->GetName(), sHarm);
	}

	{ // 已有角色累加伤害
		auto it = std::find_if(std::begin(_HarmRec), std::end(_HarmRec), [pAtk, dwID](const SHarmRec& harm) {
			return (harm.pAtk == pAtk && harm.pAtk->GetID() == dwID) ? true : false;
		});
		if (it != std::end(_HarmRec)) {
			it->sHarm += sHarm;
			it->sHate += sHarm; // 普通伤害时, 伤害和仇恨同步增加
			if (it->btValid < MAX_VALID_CNT) {
				it->btValid++;
				if (g_bLogHarmRec) {
					LG("harm", "attacker[%s], accumulative harm=%d,valid=%d\n",
					   pAtk->GetName(), it->sHarm, it->btValid);
				}
			}
			return;
		}
	}

	{ // 添加新的伤害
		auto it = std::find_if(std::begin(_HarmRec), std::end(_HarmRec), [](const SHarmRec& harm) {
			return harm.btValid == 0 ? true : false;
		});
		if (it != std::end(_HarmRec)) {
			it->pAtk = pAtk;
			it->sHarm = sHarm;
			it->sHate = sHarm;
			it->btValid = 3;
			it->dwID = dwID;
			it->dwTime = g_pGameApp->m_dwRunCnt;
			if (g_bLogHarmRec) {
				LG("harm", "add new attacker[%s], harm = %d\n",
				   pAtk->GetName(), it->sHarm);
			}
			return;
		}
	}
}

// "Add only damage without adding hate"
inline void CHateMgr::AddHate(CCharacter* pAtk, short sHate, DWORD dwID) {

	// Try to accumulate hate
	auto it = std::find_if(std::begin(_HarmRec), std::end(_HarmRec), [pAtk, dwID](const SHarmRec& harm) {
		return (harm.pAtk == pAtk && harm.pAtk->GetID() == dwID) ? true : false;
	});
	if (it != std::end(_HarmRec)) {
		it->sHate += sHate;

		if (sHate > 0) {
			if (it->btValid < MAX_VALID_CNT) {
				it->btValid++;
			}
		} else {
			if (it->btValid > 0) {
				it->btValid--;
			}
		}

		if (it->sHate < 0) {
			it->sHate = 0;
		}
	} else { // Try to add new hate
		it = std::find_if(std::begin(_HarmRec), std::end(_HarmRec), [](const SHarmRec& harm) {
			return harm.btValid == 0 ? true : false;
		});
		if (it != std::end(_HarmRec)) {
			it->pAtk = pAtk;
			it->sHate = sHate;
			it->btValid = 3;
			it->dwID = dwID;
			it->dwTime = g_pGameApp->m_dwRunCnt;
		}
	}
}

inline int CompareHarm(const void* p1, const void* p2) {
	auto pRec2 = static_cast<const SHarmRec*>(p1);
	auto pRec1 = static_cast<const SHarmRec*>(p2);

	if (pRec1->sHate < pRec2->sHate) {
		return 1;
	}
	return -1;
}

inline void CHateMgr::UpdateHarmRec(CCharacter* pSelf) {
	const DWORD dwCurTick = GetTickCount();

	// 重新记录有效的HarmRec
	int nValid = 0;
	for (int i = 0; i < MAX_HARM_REC; i++) {
		SHarmRec* pHarm = &_HarmRec[i];
		if (pHarm->btValid > 0 && pHarm->IsChaValid()) {
			memcpy(&_HarmRec[nValid], pHarm, sizeof(SHarmRec));
			nValid++;
		}
	}

	// 剩下的都清除, 保证HarmRec是紧凑的, 没有空位
	for (int j = nValid; j < MAX_HARM_REC; j++) {
		SHarmRec* pHarm = &_HarmRec[j];
		pHarm->btValid = 0;
		pHarm->sHarm = 0;
		pHarm->sHate = 0;
		pHarm->dwID = 0;
		pHarm->pAtk = nullptr;
		pHarm->dwTime = 0;
	}

	// 每 2 秒钟,排序一次
	if ((dwCurTick - _dwLastSortTick) > 2000) {
		_dwLastSortTick = dwCurTick;

		qsort(&_HarmRec, MAX_HARM_REC, sizeof(SHarmRec), CompareHarm);
		if (g_bLogHarmRec) {
			DebugNotice(pSelf);
		}
	}

	// 每 5 秒钟 btValid - 1
	if ((dwCurTick - _dwLastDecValid) > 5000) {
		_dwLastDecValid = dwCurTick;
		for (int i = 0; i < MAX_HARM_REC; i++) {
			SHarmRec* pHarm = &_HarmRec[i];
			if (pHarm->btValid > 0) {
				pHarm->btValid--;
				if (pHarm->btValid == 0) // 忘掉了
				{
					pHarm->sHarm = 0;  // 伤害不累计
					pHarm->sHate = 0;  // 仇恨不累计
					pHarm->dwTime = 0; // 时间清0
					pHarm->pAtk = nullptr;
					pHarm->dwID = 0;
				}
				if (g_bLogHarmRec) {
					//LG("harm", "攻击者[%s]的valid--, valid = %d\n", pHarm->pAtk->GetName(), pHarm->btValid);
					LG("harm", "attacker[%s]的valid--, valid = %d\n", pHarm->pAtk->GetName(), pHarm->btValid);
				}
			}
		}
	}
}

inline void CHateMgr::DebugNotice(CCharacter* pSelf) {
	std::string strNotice = pSelf->GetName();
	//strNotice+="目标列表:";
	strNotice += RES_STRING(GM_HARMREC_H_00001);
	BOOL bSend = FALSE;
	char szHate[64];
	for (int i = 0; i < MAX_HARM_REC; i++) {
		SHarmRec* pHarm = &_HarmRec[i];
		if (pHarm->btValid > 0) {
			//sprintf(szHate, ",%d(time=%d)", pHarm->sHate, pHarm->dwTime);
			_snprintf_s(szHate, sizeof(szHate), _TRUNCATE, ",%d(time=%d)", pHarm->sHate, pHarm->dwTime);
			strNotice += "[";
			strNotice += pHarm->pAtk->GetName();
			strNotice += szHate;
			strNotice += "]";
			bSend = TRUE;
		}
	}

	if (bSend) {
		LG("harm", "Notice = [%s]\n", strNotice.c_str());
		g_pGameApp->WorldNotice((char*)(strNotice.c_str()));
	}
}
