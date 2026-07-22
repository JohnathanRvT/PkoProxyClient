#include "stdafx.h"
#include "MouseDown.h"
#include "SceneItem.h"
#include "WorldScene.h"
#include "GameApp.h"
#include "Character.h"
#include "mpeditor.h"

#include "stmove.h"
#include "stattack.h"
#include "netif.h"
#include "uicommand.h"
#include "stnpctalk.h"
#include "event.h"
#include "stseat.h"
#include "stpose.h"
#include "autoattack.h"
#include "isskilluse.h"
#include "uiminimapform.h"

extern MPEditor g_Editor;

static D3DXVECTOR3 vPos;
static int nScrX, nScrY;

const DWORD DELAY_TIME = 0;
int MOVE_LENGTH = -1;
int ATTACT_DIS = 200;

CMouseDown::CMouseDown() {
	_pAutoAttack = new CAutoAttack(this);

	Reset();
}

CMouseDown::~CMouseDown() {
	delete _pAutoAttack;
}

void CMouseDown::Reset() {
	_pAutoAttack->Reset();

	_eLastType = eLastNone;
	_pAttackSkill = nullptr;
	_pAttackCha = nullptr;
	_pPickItem = nullptr;
	_pEvent = nullptr;
	_pEventNode = nullptr;
	_pNpcCha = nullptr;
	_pAreaSkill = nullptr;
	_pFollow = nullptr;

	_dwAttackTime = 0;
	_dwPickTime = 0;
	_IsEnabled = true;
}

bool CMouseDown::_AttackCha(CCharacter* pMain, CSkillRecord* pSkill, CCharacter* pTarget) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pSkill) {
		return false;
	}

	if (pSkill->IsAutoAttack() && g_pGameApp->IsShiftPress()) {
		if (_pAutoAttack->AttackStart(pMain, pSkill, pTarget)) {
			return true;
		}
	}

	bool IsForce = false;
	if (g_pGameApp->IsCtrlPress() && pMain != pTarget) {
		if (!pSkill->IsAttackTime(CGameApp::GetCurTick())) {
			return false;
		}

		IsForce = true;
	}

	if (!IsForce && !g_SkillUse.IsUse(pSkill, pMain, pTarget)) {
		return false;
	}

	return ActAttackCha(pMain, pSkill, pTarget, true, IsForce);
}

bool CMouseDown::_AttackArea(CCharacter* pMain, CSkillRecord* pSkill, int nScrX, int nScrY) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pSkill) {
		return false;
	}

	if (pSkill->IsAutoAttack() && g_pGameApp->IsShiftPress()) {
		if (_pAutoAttack->AttackStart(pMain, pSkill, nScrX, nScrY)) {
			return true;
		}
	}

	bool IsForce = false;
	if (g_pGameApp->IsCtrlPress()) {
		if (!pSkill->IsAttackTime(CGameApp::GetCurTick())) {
			return false;
		}

		IsForce = true;
	}

	if (!IsForce && !g_SkillUse.IsUse(pSkill, pMain, nullptr)) {
		return false;
	}

	return ActAttackArea(pMain, pSkill, nScrX, nScrY, IsForce);
}

bool CMouseDown::PickItem(CCharacter* pMain) {
	if (!_IsEnabled) {
		return false;
	}

	int nCount = _pScene->PickItem();
	if (nCount > 0) {
		_pAutoAttack->Cancel();
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_195), nCount); //这个信息挺累赘的 by Waiting 2009-06-23
		return true;
	}
	return false;
}

void CMouseDown::MouseButtonDown() {
	CCharacter* pMain = CGameScene::GetMainCha();
	if (!pMain) {
		return;
	}
	if (!_IsEnabled) {
		return;
	}

	_pScene->GetSign()->Hide();

	_pScene->GetPickPos(g_pGameApp->GetMouseX(), g_pGameApp->GetMouseY(), vPos);
	nScrX = (int)(vPos.x * 100.0f);
	nScrY = (int)(vPos.y * 100.0f);

	auto* seat = dynamic_cast<CInsertState*>(pMain->GetActor()->GetCurState());
	if (seat && !seat->GetIsCancel()) { // If player is resting(INSERT KEY), this is activated
		seat->SetFaceTo(nScrX, nScrY);
		seat->Start();
		return;
	}

	// Auto attack
	CIsStart start(_pAutoAttack);
	if (g_pGameApp->IsCtrlPress()) {
		CSkillRecord* pSkill = pMain->GetReadySkillInfo();
		if (!pSkill)
			pSkill = pMain->GetDefaultSkillInfo();

		if (pSkill && pSkill->IsAttackArea()) {
			_AttackArea(pMain, pSkill, nScrX, nScrY);
			return;
		}
	}

	switch (_eLastType) {
	case eLastAttack:
		_AttackCha(pMain, _pAttackSkill, _pAttackCha);
		break;
	case eLastPick:
		ActPickItem(pMain, _pPickItem);
		break;
	case eLastEvent:
		ActEvent(pMain, _pEventNode, _pEvent);
		break;
	case eLastNpc:
		ActTalkNpc(pMain, _pNpcCha);
		break;
	case eLastArea:
		_AttackArea(pMain, _pAreaSkill, nScrX, nScrY);
		break;
	case eLastDummy:
		ActDummyObj(pMain, _nDummyX, _nDummyY, _nDummyH, _nDummyAngle, _nDummyAction);
		break;
	case eLastFollow:
		_pAutoAttack->Follow(pMain, _pFollow);
		break;
	default:
		// 根据摄像头大致估算向下偏移多少为地面，要等待lemon给偏移接口
		_pScene->GetPickPos(g_pGameApp->GetMouseX(), g_pGameApp->GetMouseY() + 25, vPos);
		int nAreaX = (int)(vPos.x * 100.0f);
		int nAreaY = (int)(vPos.y * 100.0f);

		if (CGameApp::GetCurTick() <= _dwAttackTime + 500) {
			CSkillRecord* pSkill = pMain->GetReadySkillInfo();
			if (!pSkill) {
				pSkill = pMain->GetDefaultSkillInfo();
			}

			if (_pAttackSkill == pSkill && _pAttackCha->IsValid()) {
				if (GetDistance(nAreaX, nAreaY, _pAttackCha->GetCurX(), _pAttackCha->GetCurY()) <= 230 &&
					_AttackCha(pMain, _pAttackSkill, _pAttackCha)) {
					return;
				}
			}
		}

		if (CGameApp::GetCurTick() <= _dwPickTime + 500) {
			if (!pMain->GetReadySkillInfo() && _pPickItem->IsValid()) {
				if (GetDistance(nAreaX, nAreaY, _pPickItem->GetCurX(), _pPickItem->GetCurY()) <= 150) {
					if (ActPickItem(pMain, _pPickItem)) {
						return;
					}
				}
			}
		}

		if (eLastMove == _eLastType) {
			if (!_pAutoAttack->AttackMoveTo(nScrX, nScrY)) {
				ActMove(pMain, nScrX, nScrY, g_pGameApp->IsAltPress() ? true : false);
			}
		}
		break;
	}
}

void CMouseDown::CheckWalkContinue() {
	static DWORD dwAddTime = 0;
	if (CGameApp::GetCurTick() >= dwAddTime) {
		dwAddTime = CGameApp::GetCurTick() + 500;
		CCharacter* pMain = CGameScene::GetMainCha();
		if (_IsEnabled && pMain && !g_Editor.IsEnable() && CGameApp::IsMouseInScene()) {
			if (g_stUIMap.IsShowBigmap()) {
				return;
			}

			CUIInterface::MainChaMove();

			auto* seat = dynamic_cast<CInsertState*>(pMain->GetActor()->GetCurState());
			if (seat) {
				return;
			}

			_pScene->GetSign()->Hide();

			_pScene->GetPickPos(g_pGameApp->GetMouseX(), g_pGameApp->GetMouseY(), vPos);
			nScrX = (int)(vPos.x * 100.0f);
			nScrY = (int)(vPos.y * 100.0f);
			if (pMain->getMoveSpeed() > 0) {
				int len = pMain->getMoveSpeed();
				len += GetDistance(pMain->GetCurX(), pMain->GetCurY(), pMain->GetServerX(), pMain->GetServerY());
				if (GetDistance(pMain->GetCurX(), pMain->GetCurY(), nScrX, nScrY) < len) {
					GetDistancePos(pMain->GetCurX(), pMain->GetCurY(), nScrX, nScrY, len, nScrX, nScrY);
				}
			}

			ActMove(CGameScene::GetMainCha(), nScrX, nScrY, false, true);
		}
	}
}

bool CMouseDown::ActMove(CCharacter* pCha, int nScrX, int nScrY, bool isAdd, bool isLine, bool IsCompart) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha) {
		return false;
	}

	if (g_Config.m_bEditor) {
		pCha->SetServerPos(pCha->GetCurX(), pCha->GetCurY());
	}

	if (!isLine) {
		if (!g_cFindPathEx.HaveTarget())
			_pScene->GetSign()->Show(nScrX, nScrY);
	}

	static CCharacter* pLastCha = nullptr;
	static int nLastScrX = 0, nLastScrY = 0;
	static bool isLastAdd = false, isLastLine = false;
	static DWORD dwLastTime = 0;

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && nLastScrX == nScrX && nLastScrY == nScrY && isLastAdd == isAdd && isLastLine == isLine) {
		return true;
	}

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	nLastScrX = nScrX;
	nLastScrY = nScrY;
	isLastAdd = isAdd;
	isLastLine = isLine;

	if (g_Config.m_IsMoveClient) {
		auto move_state = std::make_unique<CServerMoveState>(pCha->GetActor());
		move_state->SetWalkLine(isLine);
		move_state->SetMoveTo(nScrX, nScrY);
		return pCha->GetActor()->SwitchState(std::move(move_state));

		// 分段算法
		bool isOK = false;
		if (!isLine && MOVE_LENGTH > 0) {
			isOK = g_cFindPath.GetPathFindingState() /*g_cFindPath.Find( pCha->GetScene(), pCha, pCha->GetCurX(), pCha->GetCurY(), nScrX, nScrY, isLine )*/;
		}
		if (isOK) {
			CMoveList cLocalList;
			COneMoveState::CompartMoveList(cLocalList, g_cFindPath.GetResultPath(), MOVE_LENGTH);
			POINT& pt = cLocalList.GetBack();
			pt.x = nScrX;
			pt.y = nScrY;

			int nCount = 0;
			while (!cLocalList.IsEmpty()) {
				POINT pt = cLocalList.GetFront();
				cLocalList.PopFront();

				auto move_state = std::make_unique<CServerMoveState>(pCha->GetActor());
				move_state->SetWalkLine(isLine);
				move_state->SetMoveTo(pt.x, pt.y);
				if (nCount == 0)
					pCha->GetActor()->SwitchState(std::move(move_state));
				else
					pCha->GetActor()->AddState(std::move(move_state));
				nCount++;
			}
			return true;
		} else {
			auto move_state = std::make_unique<CServerMoveState>(pCha->GetActor());
			move_state->SetWalkLine(isLine);
			move_state->SetMoveTo(nScrX, nScrY);
			return pCha->GetActor()->SwitchState(std::move(move_state));
		}
	}

	if (IsCompart) {
		auto* pMoveState = dynamic_cast<COneMoveState*>(pCha->GetActor()->GetCurState());
		if (pMoveState) {
			if (isAdd) {
				return pMoveState->AddPath(nScrX, nScrY, isLine);
			} else {
				pCha->GetActor()->ClearQueueState();
				return pMoveState->ContinueMove(nScrX, nScrY, isLine, true);
			}
		} else {
			auto move_state = std::make_unique<COneMoveState>(pCha->GetActor());
			move_state->SetWalkLine(isLine);
			move_state->SetMoveTo(nScrX, nScrY);
			return pCha->GetActor()->SwitchState(std::move(move_state));
		}
	} else {
		auto move_state = std::make_unique<COneMoveState>(pCha->GetActor());
		move_state->SetWalkLine(isLine);
		move_state->SetMoveTo(nScrX, nScrY);
		return pCha->GetActor()->SwitchState(std::move(move_state));
	}
}

bool CMouseDown::ActAttackCha(CCharacter* pCha, CSkillRecord* pSkill, CCharacter* pTarget, bool isTrace, bool IsForce, bool IsCompart) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha || !pSkill || !pTarget) {
		return false;
	}

	// 暂时屏蔽 by lh
	if (!g_NetIF->IsConnected()) {
		return false;
	}

	// Initialized once
	static CCharacter* pLastCha{nullptr};
	static CCharacter* pLastTarget{nullptr};
	static CSkillRecord* pLastSkill{nullptr};
	static DWORD dwLastTime = 0;

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && pLastTarget == pTarget && pLastSkill == pSkill) {
		return true;
	}

	LG("mousedown", "actattackcha: self:%s, target:%s, pos[%d, %d], skill:%s\ttick:%u\n",
	   pCha->getName(), pTarget->getName(), pTarget->GetCurX(), pTarget->GetCurY(),
	   pSkill->szName, CGameApp::GetCurTick());

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	pLastTarget = pTarget;
	pLastSkill = pSkill;

	_pScene->GetSign()->Show(pTarget);
	g_pGameApp->AddTipText(pSkill->szName);

	// 计算转折点
	if( g_Config.m_IsMoveClient && IsCompart && pSkill->GetDistance() )
	{
		int preMove = CWaitMoveState::GetPreMoveDis();
		if( preMove<ATTACT_DIS ) preMove = ATTACT_DIS;
		int nTotalDis = pCha->GetDefaultChaInfo()->sRadii + pSkill->GetDistance() + pTarget->GetDefaultChaInfo()->sRadii + preMove + ATTACT_DIS;
		bool isLine = false;
		if( g_cFindPath.Find( pCha->GetScene(), pTarget, pTarget->GetServerX(), pTarget->GetServerY(), pCha->GetServerX(), pCha->GetServerY(), isLine ) )
		{
			if( g_cFindPath.GetLength() > nTotalDis + ATTACT_DIS )
			{
				CMoveList cLocalList;
				COneMoveState::CompartMoveList( cLocalList, g_cFindPath.GetResultPath(), nTotalDis );
				int x = cLocalList.GetFront().x;
				int y = cLocalList.GetFront().y;

				pCha->GetActor()->ClearQueueState();
				COneMoveState* pMoveState = dynamic_cast<COneMoveState*>( pCha->GetActor()->GetCurState() );
				if( pMoveState )
				{
					pMoveState->ContinueMove( x, y, isLine );
				}
				else
				{
					auto move = std::make_unique<COneMoveState>(pCha->GetActor());
					move->SetWalkLine( isLine );
					move->SetMoveTo( x, y );
					pCha->GetActor()->SwitchState(std::move(move));
				}

				auto attack = isTrace ? std::make_unique<CTraceAttackState>(pCha->GetActor()) : std::make_unique<CAttackState>(pCha->GetActor());
				attack->SetIsForce( IsForce );
				attack->SetSkill( pSkill );
				attack->SetTarget( pTarget );
				attack->SetCommand( CCommandObj::GetReadyCommand() );
				return pCha->GetActor()->AddState(std::move(attack));
			}
		}
	}

	auto attack = isTrace ? std::make_unique<CTraceAttackState>(pCha->GetActor())
						  : std::make_unique<CAttackState>(pCha->GetActor());
	attack->SetIsForce(IsForce);
	attack->SetSkill(pSkill);
	attack->SetTarget(pTarget);
	attack->SetCommand(CCommandObj::GetReadyCommand());
	return pCha->GetActor()->SwitchState(std::move(attack));
}

bool CMouseDown::ActAttackArea(CCharacter* pCha, CSkillRecord* pSkill, int nScrX, int nScrY, bool IsForce, bool IsCompart) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha || !pSkill) {
		return false;
	}

	if (!pSkill->IsAttackArea()) {
		g_pGameApp->SysInfo(RES_STRING(CL_LANGUAGE_MATCH_196));
		return false;
	}

	// Initialized once
	static CCharacter* pLastCha{nullptr};
	static CSkillRecord* pLastSkill{nullptr};
	static int nLastScrX = 0, nLastScrY = 0;
	static DWORD dwLastTime = 0;

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && pLastSkill == pSkill && nLastScrX == nScrX && nLastScrY == nScrY) {
		return true;
	}

	LG("mousedown", "actattackarea: self:%s, skill:%s, pos[%d, %d]\ttick:%u\n",
	   pCha->getName(), pSkill->szName, nScrX, nScrY, CGameApp::GetCurTick());

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	pLastSkill = pSkill;
	nLastScrX = nScrX;
	nLastScrY = nScrY;

	// 计算转折点
	if( g_Config.m_IsMoveClient && IsCompart && pSkill->GetDistance() )
	{
		int preMove = CWaitMoveState::GetPreMoveDis();
		if( preMove<ATTACT_DIS ) preMove = ATTACT_DIS;
		int nTotalDis = pCha->GetDefaultChaInfo()->sRadii + pSkill->GetDistance() + preMove + ATTACT_DIS;
		bool isLine = false;
		if( g_cFindPath.Find( pCha->GetScene(), pCha, nScrX, nScrY, pCha->GetServerX(), pCha->GetServerY(), isLine ) )
		{
			if( g_cFindPath.GetLength() > nTotalDis + ATTACT_DIS )
			{
				CMoveList cLocalList;
				COneMoveState::CompartMoveList( cLocalList, g_cFindPath.GetResultPath(), nTotalDis );
				int x = cLocalList.GetFront().x;
				int y = cLocalList.GetFront().y;

				pCha->GetActor()->ClearQueueState();
				COneMoveState* pMoveState = dynamic_cast<COneMoveState*>( pCha->GetActor()->GetCurState() );
				if( pMoveState )
				{
					pMoveState->ContinueMove( x, y, isLine );
				}
				else
				{
					auto move = std::make_unique<COneMoveState>(pCha->GetActor());
					move->SetWalkLine( isLine );
					move->SetMoveTo( x, y );
					pCha->GetActor()->SwitchState(std::move(move));
				}

				auto attack = std::make_unique<CAttackState>(pCha->GetActor());
				attack->SetIsForce( IsForce );
				attack->SetSkill( pSkill );
				attack->SetAttackPoint( nScrX, nScrY );
				attack->SetCommand( CCommandObj::GetReadyCommand() );
				return pCha->GetActor()->AddState(std::move(attack));
			}
		}
	}

	// 显示攻击范转特效
	auto attack = std::make_unique<CAttackState>(pCha->GetActor());
	attack->SetIsForce(IsForce);
	attack->SetSkill(pSkill);
	attack->SetAttackPoint(nScrX, nScrY);
	attack->SetCommand(CCommandObj::GetReadyCommand());

	g_pGameApp->AddTipText("AttackPoint: %s", pSkill->szName);
	return pCha->GetActor()->SwitchState(std::move(attack));
}

bool CMouseDown::ActTalkNpc(CCharacter* pCha, CCharacter* pNpc) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha || !pNpc) {
		return false;
	}

	static CCharacter* pLastCha = nullptr;
	static CCharacter* pLastTarget = nullptr;
	static DWORD dwLastTime = 0;

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && pLastTarget == pNpc) {
		return true;
	}

	LG("mousedown", "acttalknpc: self:%s, npc:%s, pos[%d, %d]\ttick:%u\n", pCha->getName(), pNpc->getName(), pNpc->GetCurX(), pNpc->GetCurY(), CGameApp::GetCurTick());

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	pLastTarget = pNpc;

	if (pNpc->DistanceFrom(pCha) > 300) {
		int x = 0;
		int y = 0;
		GetDistancePos(pNpc->GetCurX(), pNpc->GetCurY(), pCha->GetCurX(), pCha->GetCurY(), 200, x, y);
		ActMove(pCha, x, y);
	}

	auto npc = std::make_unique<CNpcState>(pCha->GetActor());
	npc->SetNpc(pNpc);
	return pCha->GetActor()->AddState(std::move(npc));
}

bool CMouseDown::ActPickItem(CCharacter* pCha, CSceneItem* pItem, bool IsMove) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha || !pItem) {
		return false;
	}

	if (!pItem->IsPick()) {
		return false;
	}

	// Initialized once
	static CCharacter* pLastCha{nullptr};
	static CSceneItem* pLastItem{nullptr};
	static DWORD dwLastTime{0};

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && pLastItem == pItem) {
		return true;
	}

	LG("mousedown", "actpicktime: self:%s, item:%s, pos[%d, %d]\ttick:%u\n",
	   pCha->getName(), pItem->GetItemInfo()->szName,
	   pItem->GetCurX(), pItem->GetCurY(), CGameApp::GetCurTick());

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	pLastItem = pItem;

	if (IsMove) {
		// Move to item proximity to pick up?
		ActMove(pCha, pItem->GetCurX(), pItem->GetCurY());
	}

	float h = 0.8f;
	pItem->GetItemHeight(&h);
	_pScene->GetSign()->Show(pItem->GetCurX(), pItem->GetCurY(), (int)(h * 100.0f));

	auto pState = std::make_unique<CPickState>(pCha->GetActor());
	pState->SetItem(pItem);
	return pCha->GetActor()->AddState(std::move(pState));
}

bool CMouseDown::ActEvent(CCharacter* pCha, CSceneNode* pNode, CEvent* pEvent) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha || !pNode || !pEvent) {
		return false;
	}

	if (pEvent->IsNormal() && pEvent->GetInfo()->IsValid(pCha->GetMainType()) && pEvent->GetInfo()->sArouseType == enumEVENT_AROUSE_CLICK) {

		// Initialized once
		static CCharacter* pLastCha{nullptr};
		static CSceneNode* pLastNode{nullptr};
		static CEvent* pLastEvent{nullptr};
		static DWORD dwLastTime = 0;

		if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && pLastNode == pNode && pLastEvent == pEvent) {
			return true;
		}

		LG("mousedown", "actevent: self:%s, nodeid:%u, pos[%d, %d], event:%s\ttick:%u\n",
		   pCha->getName(), pNode->getAttachID(), pNode->GetCurX(), pNode->GetCurY(),
		   pEvent->GetInfo()->szName, CGameApp::GetCurTick());

		dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
		pLastCha = pCha;
		pLastNode = pNode;
		pLastEvent = pEvent;

		pCha->GetActor()->CancelState();

		ActMove(pCha, pNode->GetCurX(), pNode->GetCurY());

		auto pState = std::make_unique< CEventState>(pCha->GetActor());
		pState->SetEvent(pEvent);
		pState->SetNode(pNode);
		return pCha->GetActor()->AddState(std::move(pState));
	}
	return false;
}

bool CMouseDown::ActDummyObj(CCharacter* pCha, int x, int y, int h, int t_angle, int action) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha) {
		return false;
	}

	if (pCha->IsBoat()) {
		return false;
	}

	// Initialized once
	static CCharacter* pLastCha{nullptr};
	static int nLastX = 0, nLastY = 0, nLastH = 0, nLastAngle = 0, nLastAction = 0;
	static DWORD dwLastTime = 0;

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && nLastX == x && nLastY == y && nLastH == h && nLastAngle == t_angle && nLastAction == action) {
		return true;
	}

	LG("mousedown", "actdummyobj: self:%s, x:%d, y:%d, h:%d, angle:%d, action:%d\ttick:%u\n",
	   pCha->getName(), x, y, h, t_angle, action, CGameApp::GetCurTick());

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	nLastX = x, nLastY = y;
	nLastH = h, nLastAngle = t_angle;
	nLastAction = action;

	auto* seat = dynamic_cast<CSeatState*>(pCha->GetActor()->FindStateClass(typeid(CSeatState)));
	if (seat) {
		if (seat->GetAngle() == t_angle && seat->GetPosX() == x && seat->GetPosY() == y && seat->GetHeight() == h && seat->GetPose() == action) {
			return true;
		}
	}

	int dx, dy;
	GetAnglePos(x, y, pCha->GetDefaultChaInfo()->sRadii, t_angle, dx, dy);

	ActMove(pCha, dx, dy);

	seat = new CSeatState(pCha->GetActor());
	seat->SetPos(x, y);
	seat->SetAngle(t_angle);
	seat->SetPose(action);
	seat->SetHeight(h);
	return pCha->GetActor()->AddState(std::unique_ptr<CSeatState>(seat));
}

bool CMouseDown::ActCha(CCharacter* pCha, CCharacter* pTarget) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha || !pTarget) {
		return false;
	}

	// Initialized once
	static CCharacter* pLastCha{nullptr};
	static CCharacter* pLastTarget{nullptr};
	static DWORD dwLastTime = 0;

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && pLastTarget == pTarget) {
		return true;
	}

	LG("mousedown", "actcha: self:%s, target:%s\ttick:%u\n",
	   pCha->getName(), pTarget->getName(), CGameApp::GetCurTick());

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	pLastTarget = pTarget;

	if (pTarget->getEvent()) {
		return ActEvent(pCha, pTarget, pTarget->getEvent());
	}

	if (pTarget->IsNPC()) {
		return ActTalkNpc(pCha, pTarget);
	}

	CSkillRecord* pSkill = pCha->GetReadySkillInfo();
	if (!pSkill) {
		pSkill = pCha->GetDefaultSkillInfo();
	}

	return ActAttackCha(pCha, pSkill, pTarget);
}

bool CMouseDown::ActBackMove(CCharacter* pCha, int nScrX, int nScrY, bool isAdd) {
	auto move_state = std::make_unique<CBackMoveState>(pCha->GetActor());
	move_state->SetMoveTo(nScrX, nScrY);

	if (isAdd) {
		return pCha->GetActor()->AddState(std::move(move_state));
	}
	return pCha->GetActor()->SwitchState(std::move(move_state));
}

void CMouseDown::FrameMove() {
	_pAutoAttack->FrameMove();
}

bool CMouseDown::ActShop(CCharacter* pCha, CCharacter* pTarget) {
	if (!_IsEnabled) {
		return false;
	}

	if (!pCha || !pTarget) {
		return false;
	}

	if (pCha == pTarget) {
		return false;
	}

	if (pCha->IsShop() || !pTarget->IsShop()) {
		return false;
	}

	// Initialized once
	static CCharacter* pLastCha{nullptr};
	static CCharacter* pLastTarget{nullptr};
	static DWORD dwLastTime = 0;

	if (CGameApp::GetCurTick() <= dwLastTime && pLastCha == pCha && pLastTarget == pTarget) {
		return true;
	}

	dwLastTime = CGameApp::GetCurTick() + DELAY_TIME;
	pLastCha = pCha;
	pLastTarget = pTarget;

	if (pCha->DistanceFrom(pTarget) > 300) {
		int x = 0;
		int y = 0;

		GetAnglePos(pTarget->GetCurX(), pTarget->GetCurY(), 380, pTarget->getYaw(), x, y);
		ActMove(pCha, x, y);

		GetDistancePos(pTarget->GetCurX(), pTarget->GetCurY(), x, y, 250, x, y);
		ActMove(pCha, x, y, true);
	}

	auto shop = std::make_unique<CShopState>(pCha->GetActor());
	shop->SetShop(pTarget);
	return pCha->GetActor()->AddState(std::move(shop));
}
