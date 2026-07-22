#pragma once
#include "CRCursorObj.h"

class CShadeEff;
class CRCursor : public CursorObj {
public:
	CRCursor();
	~CRCursor();

	virtual void Init(CGameScene* p) override;
	virtual void Clear() override;

	virtual void MouseDown(int nButton) override;
	virtual void MouseUp(int nButton) override;
	virtual void FrameMove(DWORD dwTime) override;
	virtual void Render() override;
	virtual void MoveTo(D3DXVECTOR3& vPos) override;

private:
	D3DXVECTOR3 _vPos;
	CShadeEff* _pCursorEff;
};
