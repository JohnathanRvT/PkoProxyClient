#include "stdafx.h"
#include "Actor.h"
#include "GameApp.h"
#include "Character.h"
#include "HMAttack.h"
#include "EffectObj.h"
#include "stpose.h"
#include "HMManage.h"
#include "SceneItem.h"
#include "chastate.h"

//---------------------------------------------------------------------------
// class CActor
//---------------------------------------------------------------------------
CActor::CActor(CCharacter* pCha)
	: _pCha(pCha), _eState(enumNormal), _nWaitingTime(-1) {
}

CActor::~CActor() {
	ClearQueueState();
}

bool CActor::AddState(std::unique_ptr<CActionState> pState) {
	if (!IsEnabled()) {
		pState.reset();
		return false;
	}

	if (_nWaitingTime > 0)
		_nWaitingTime = 0;

	if (!_pCurState) {
		_pCurState = std::move(pState);
		_pCurState->Start();
	} else {
		_pCurState->BeforeNewState();

		pState->SetIsWait(true);
		_statelist.push_back(std::move(pState));
	}
	return true;
}

bool CActor::InsertState(std::unique_ptr<CActionState> pState, bool IsFront /*= false*/) {
	if (!IsEnabled()) {
		pState.reset();
		return false;
	}

	if (_nWaitingTime > 0)
		_nWaitingTime = 0;

	if (!_pCurState) {
		_pCurState = std::move(pState);
		_pCurState->Start();
	} else {
		_pCurState->BeforeNewState();

		pState->SetIsWait(true);
		_statelist.push_front(std::move(pState));
	}
	return true;
}

bool CActor::SwitchState(std::unique_ptr<CActionState> pState) {
	if (!pState->IsAllowUse()) {
		pState.reset();
		return false;
	}

	if (_nWaitingTime > 0)
		_nWaitingTime = 0;

	if (_pCurState) {
		_pCurState->BeforeNewState();

		if (_pCurState->GetIsCancel()) {
			// 如果上次已经取消，则服务器肯定能返回，暂将这次操作压入队列即可
			ClearQueueState();

			pState->SetIsWait(true);
			_statelist.push_back(std::move(pState));
			return true;
		}

		ClearQueueState(); // 切换时,清除原来的队列

		_pCurState->Cancel();

		pState->SetIsWait(true);
		_statelist.push_back(std::move(pState));
		return true;
	} else {
		ClearQueueState(); // 切换时,清除原来的队列

		_pCurState = std::move(pState);
		_pCurState->Start();
		return true;
	}
}

void CActor::CancelState() {
	ClearQueueState();

	if (_pCurState) {
		_pCurState->Cancel();
	} else {
		IdleState();
	}
}

void CActor::ClearQueueState() {
	_statelist.clear();
}

void CActor::IdleState() {
	
	if (_eState == enumNormal) {
		_pCha->PlayPose(_pCha->GetPose(POSE_WAITING), PLAY_LOOP_SMOOTH);

		_nWaitingTime = -1;

		switch (_pCha->getChaCtrlType()) {
		case enumCHACTRL_PLAYER:
			if (_pCha->getChaModalType() != enumMODAL_BOAT)
				_nWaitingTime = 30 * 32;
			break;
		case enumCHACTRL_MONS:
			if (_pCha->getChaModalType() == enumMODAL_OTHER)
				_nWaitingTime = rng.uniform(3, 3 + 15 - 1) * 32;
			break;
		}
	}
}

void CActor::ExecAllNet() {
	ClearQueueState();
	_pCurState.reset();

	for (auto* fight : _fights) {
		if (!fight->GetIsExecEnd()) {
			fight->ExecAll();
			fight->SetIsOuter(false);
		}
	}

	ExecDied();
}

void CActor::ExecDied() {
	for (auto* die : _dies) {
		die->Exec();
		SAFE_DELETE(die);
	}
	_dies.clear();

	_pCha->GetStateMgr()->ChaDied();
}

void CActor::FailedAction() {
	ClearQueueState();

	if (_pCurState) {
		_pCurState->ServerEnd(0);
		_pCurState->MoveEnd(_pCha->GetCurX(), _pCha->GetCurY(), 0);
	}
}

void CActor::FrameMove(DWORD dwTimeParam) {
	if (_pCurState) {
		if (!_pCurState->GetIsExecEnd()) {
			_pCurState->FrameMove();
		}

		if (_pCurState->GetIsExecEnd()) {
			_pCurState->End();

			if (_statelist.empty()) {
				if (!_pCurState->IsKeepPose())
					IdleState();

				_pCurState.reset();
			} else {
				_pCurState.reset();

				_pCurState = std::move(_statelist.front());
				_statelist.pop_front();

				_pCurState->SetIsWait(false);
				_pCurState->Start();
			}
		}
	} else if (_eState == enumNormal) {
		if (_pCha->GetStateMgr()->GetSkillStateNum() == 0 && _nWaitingTime > 0) {
			_nWaitingTime--;
			if (_nWaitingTime == 0) {
				switch (_pCha->getChaCtrlType()) {
				case enumCHACTRL_PLAYER:
					if (_pCha->getChaModalType() != enumMODAL_BOAT)
						_pCha->PlayPose(POSE_SHOW, PLAY_ONCE);
					break;
				case enumCHACTRL_MONS:
					_pCha->PlayPose(POSE_SHOW, PLAY_ONCE);
					break;
				}
			}
		}
	}
}

void CActor::Stop() {
	ClearQueueState();

	if (_pCurState) {
		_pCurState->Cancel();
	}
}

void CActor::SetState(eActorState v) {
	_eState = v;

	GetCha()->RefreshShadow();

	if (_eState == enumNormal) {
		IdleState();
	}
}

void CActor::PlayPose(int poseid, bool isKeep, bool isSend) {
	auto state = std::make_unique<CPoseState>(this);
	state->SetKeepPose(isKeep);
	state->SetPose(poseid);
	state->SetIsSend(isSend);
	SwitchState(std::move(state));
}

CActionState* CActor::FindStateClass(const type_info& info) {
	if (_pCurState) {
		if (typeid(*_pCurState) == info)
			return _pCurState.get();

		for (auto & it : _statelist)
			if (typeid(*it) == info)
				return it.get();
	}
	return nullptr;
}

CServerHarm* CActor::CreateHarmMgr() {
	for (auto* fight : _fights) {
		if (fight->GetIsExecEnd()) {
			fight->InitMemory();
			return fight;
		}
	}

	auto p = _fights.emplace_back(new CServerHarm(this));
	LG(_pCha->getLogName(), "CServerHarm CreateHarmMgr, Size[%d]\n", _fights.size());
	return p;
}

CServerHarm* CActor::FindHarm(int nFightID) {
	for (auto* fight : _fights) {
		if (fight->GetFightID() == nFightID) {
			return fight;
		}
	}
	return nullptr;
}

CServerHarm* CActor::FindHarm() {
	for (auto* fight : _fights) {
		if (!fight->GetIsExecEnd()) {
			return fight;
		}
	}
	return nullptr;
}

void CActor::OverAllState() {
	ClearQueueState();
	if (_pCurState) {
		_pCurState->PopState();
	}
}

CActionState* CActor::GetNextState() {
	if (_statelist.empty())
		return nullptr;

	return _statelist.front().get();
}

//---------------------------------------------------------------------------
// class CMonsterItemSynchro
//---------------------------------------------------------------------------
CMonsterItem::CMonsterItem()
	: _pCha(nullptr), _pItem(nullptr) {
}

CMonsterItem::~CMonsterItem() {
}

void CMonsterItem::Exec() {
	if (!_pItem)
		return;

	if (_pCha) {
		_pItem->PlayArcAni(_pCha->GetPos(), _pItem->getPos(), 3.5f, 5.0f);
	}
	_pItem->SetHide(FALSE);

	int nEffectID = _pItem->GetItemInfo()->sAreaEffect[0];
	if (nEffectID <= 0)
		return;

	int nDummy = _pItem->GetItemInfo()->sAreaEffect[1];

	// 以下代码有问题，没有特效显示出来
	// 地上的模型带有粒子特效
	CEffectObj* pEffect = CGameApp::GetCurScene()->GetFirstInvalidEffObj();
	if (!pEffect)
		return;

	if (!pEffect->Create(nEffectID)) {
		//LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_1), nEffectID );
		LG("protocol", RES_STRING(CL_LANGUAGE_MATCH_1), nEffectID);
		return;
	}
	pEffect->setFollowObj((CSceneNode*)_pItem, NODE_ITEM, nDummy);
	pEffect->Emission(-1, nullptr, nullptr);

	pEffect->SetValid(TRUE);
	_pItem->AddEffect(pEffect->getID());
}

//---------------------------------------------------------------------------
// class CMissionTrigger
//---------------------------------------------------------------------------
CMissionTrigger::CMissionTrigger() {
	_pData = new stNetNpcMission;
}

CMissionTrigger::~CMissionTrigger() {
	//delete _pData;
	SAFE_DELETE(_pData); // UI当机处理
}

void CMissionTrigger::SetData(stNetNpcMission& v) {
	memcpy(_pData, &v, sizeof(stNetNpcMission));
}

void CMissionTrigger::Exec() {
	char szData[64] = {0};
	strncpy_s(szData, sizeof(szData), RES_STRING(CL_LANGUAGE_MATCH_2), _TRUNCATE);

	CChaRecord* pCharRecord = GetChaRecordInfo(_pData->sID);
	if (pCharRecord) {
		strncpy_s(szData, sizeof(szData), pCharRecord->szName, _TRUNCATE);
	}
	g_pGameApp->ShowMidText(RES_STRING(CL_LANGUAGE_MATCH_3), szData, _pData->sCount, _pData->sNum);
}
