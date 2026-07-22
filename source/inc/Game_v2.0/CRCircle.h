#pragma once

#include "CRCursorObj.h"

class CShadeInfo;
class CShadeEff;

class CCircle3DCursor : public CursorObj {
public:
	CCircle3DCursor();
	~CCircle3DCursor();

	virtual void Init(CGameScene* p) override;
	virtual void Clear() override;

	void MouseDown(int nButton) override;
	void MouseUp(int nButton) override;
	void MoveTo(D3DXVECTOR3& stPos) override;

	void FrameMove(DWORD dwTime) override;
	void Render() override;

	void SetRadius(unsigned int r) override;
	void SetIsShow(bool bShow) override;

protected:
	CShadeEff* _pShade;
	CGameScene* _pScene;

	int _nRadius;
	BOOL _IsHide;
};
