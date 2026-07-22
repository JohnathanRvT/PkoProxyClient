#include "StdAfx.h"
#include "event.h"
#include "character.h"
#include "packetcmd.h"
#include "gameapp.h"
#include "uiheadsay.h"

//---------------------------------------------------------------------------
// class CEvent
//---------------------------------------------------------------------------
CEvent::CEvent(CGameScene* pScene)
	: _pScene(pScene) {
	LG("event", "new\n");
	Init();
}

CEvent::~CEvent() {
	LG("event", "del\n");
}

bool CEvent::DistanceTrigger(int x, int y) {
	if (GetIsValid() && GetIsEnabled() && GetInfo()->sArouseType == enumEVENT_AROUSE_DISTANCE) {
		if (!_IsActive) {
			if (GetDistance(x, y, GetNode()->GetCurX(), GetNode()->GetCurY()) <= GetInfo()->sArouseRadius) {
				_IsActive = true;
				_dwLastTime = CGameApp::GetCurTick() + 10000;
				return true;
			}
		} else {
			// 如果已经激活的,过10秒后取消激活
			if (CGameApp::GetCurTick() > _dwLastTime) {
				_IsActive = false;
			}
		}
	}
	return false;
}

void CEvent::ExecEvent(CCharacter* pCha) {
	if (!IsNormal())
		return;

	CEventRecord* pEvent = GetInfo();
	CSceneNode* pNode = GetNode();
	if (pEvent->sEffect != -1) {
		pCha->SelfEffect(pEvent->sEffect);
	}

	if (pEvent->sMusic != -1) {
		_pScene->PlayEnvSound(pEvent->sMusic, pNode->GetCurX(), pNode->GetCurY());
	}

	switch (pEvent->sEventType) {
	case enumEVENT_TYPE_ACTION:
		stNetActivateEvent event;
		event.lTargetID = pNode->getAttachID();
		event.lHandle = pNode->lTag;
		event.sEventID = (short)pEvent->lID;
		CS_BeginAction(pCha, enumACTION_EVENT, (void*)&event);
		break;
	case enumEVENT_TYPE_ENTITY:
		CS_EntityEvent(pNode->getAttachID());
		break;
	}
}

void CEvent::Render() {
	if (_strName.empty())
		return;

	static int x = 0, y = 0;
	D3DXVECTOR3& vPos = _pNode->GetPos();
	g_Render.WorldToScreen(vPos.x, vPos.y, vPos.z, &x, &y);

	CHeadSay::RenderText((char*)_strName.c_str(), x, y);
}

//---------------------------------------------------------------------------
// class CEventMgr
//---------------------------------------------------------------------------
CEventMgr::CEventMgr(CGameScene* pScene)
	: _pScene(pScene) {
}

CEventMgr::~CEventMgr() {
	Clear();
}

CEvent* CEventMgr::CreateEvent(DWORD dwEventID) {
	int nEventID = dwEventID & 0x0fff;
	CEventRecord* pInfo = GetEventRecordInfo(nEventID);
	if (!pInfo)
		return nullptr;

	bool IsEnabled = true;
	if (pInfo->sEventType == enumEVENT_TYPE_ENTITY) {
		IsEnabled = ((dwEventID & 0xf000) >> 12) == mission::ENTITY_ENABLE;
	}

	for (auto& event : _events) {
		if (!event->GetIsValid()) {
			event->Init();
			event->SetInfo(pInfo);
			event->SetIsEnabled(IsEnabled);
			return event;
		}
	}

	auto* pEvent = new CEvent(_pScene);
	pEvent->SetInfo(pInfo);
	pEvent->SetIsEnabled(IsEnabled);
	_events.push_back(pEvent);
	return pEvent;
}

void CEventMgr::DistanceTrigger(CCharacter* pCha) {
	for (auto& event : _events) {
		if (event->IsNormal() && event->DistanceTrigger(pCha->GetCurX(), pCha->GetCurY())) {
			event->ExecEvent(pCha);
		}
	}
}

void CEventMgr::Clear() {
	for (auto& event : _events)
		delete event;

	_events.clear();
}

void CEventMgr::Render() {
	for (auto& event : _events)
		if (event->IsNormal())
			event->Render();
}

CEvent* CEventMgr::Search(long lEntityID) {
	for (auto& event : _events)
		if (event->GetIsValid() &&
			event->GetNode()->getAttachID() == lEntityID)
			return event;

	return nullptr;
}
