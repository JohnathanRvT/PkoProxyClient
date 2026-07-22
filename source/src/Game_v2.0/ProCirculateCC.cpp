#include "StdAfx.h"
#include "procirculate.h"
#include "NetProtocol.h"
#include "Character.h"

void CProCirculateCC::BeginAction(CCharacter* pCha, DWORD type, const void* param, CActionState* pState) {
	if (!pState) {
		return;
	}

	switch (type) {
	case enumACTION_MOVE: {
		auto pMove = static_cast<const stNetMoveInfo*>(param);
		for (size_t i = 1; i < pMove->pos_num; i++) {
			pState->PushPoint(pMove->pos_buf[i].x, pMove->pos_buf[i].y);
		}
		pState->MoveEnd(pMove->pos_buf[pMove->pos_num - 1].x, pMove->pos_buf[pMove->pos_num - 1].y, 0);
		pState->ServerEnd(0);
	} break;
	case enumACTION_SKILL_POSE:
	case enumACTION_SKILL:
		pState->ServerEnd(0);
		break;
	}
}

void CProCirculateCC::EndAction(CActionState* pState) {
	if (!pState) {
		return;
	}

	pState->GetActor()->GetCha()->StopMove();
	pState->ServerEnd(0);
}
