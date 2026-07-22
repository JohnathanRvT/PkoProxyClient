#pragma once
#include "STStateObj.h"

class CSeatState : public CActionState {
public:
	CSeatState(CActor* p);

	virtual const char* GetExplain() override { return "CSeatState"; }

	virtual void FrameMove() override;
	virtual void BeforeNewState() override { PopState(); }

	void SetAngle(int nAngle) { _nAngle = nAngle; }
	void SetPos(int nX, int nY) {
		_nPosX = nX;
		_nPosY = nY;
	}
	void SetPose(int nPos) { _nPose = nPos; }
	void SetHeight(int nHeight) { _nHeight = nHeight; }

	int GetPosX() { return _nPosX; }
	int GetPosY() { return _nPosY; }
	int GetHeight() { return _nHeight; }
	int GetAngle() { return _nAngle; }
	int GetPose() { return _nPose; }

protected:
	virtual bool _Start() override;
	virtual void _End() override;
	virtual bool _IsAllowCancel() override { return true; }

protected:
	int _nPose;
	int _nAngle;
	int _nPosX, _nPosY;
	int _nHeight;
	bool _IsSeat;

private:
	int _nOldChaHeight;
	int _nOldX, _nOldY;
	CCharacter* _pCha;
};
