#include "stdafx.h"
#include "SceneNode.h"
#include "Scene.h"

#include "EffectObj.h"

CSceneNode::CSceneNode() {}

void CSceneNode::SetShadeShow(int iIdx, bool bShow) {
	if (_iShadeNum <= 0 || iIdx >= _iShadeNum)
		return;

	CShadeEff* pShade = _pScene->GetShadeObj(_iShadeID[iIdx]);
	if (pShade && pShade->IsValid()) {
		pShade->SetHide(!bShow);
	}
}

void CSceneNode::SetEffectShow(int iIdx, bool bShow) {
	if (_iEffNum <= 0 || iIdx >= _iEffNum)
		return;

	CEffectObj* pEff = _pScene->GetEffect(_iEffID[iIdx]);
	if (pEff && pEff->IsValid()) {
		pEff->SetHide(!bShow);
	}
}
