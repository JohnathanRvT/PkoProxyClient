#pragma once
#include "STStateObj.h"

// 表情
class CPoseState : public CActionState {
public:
	CPoseState(CActor* p);

	virtual const char* GetExplain() override { return "CPoseState"; }

	virtual void BeforeNewState() override { PopState(); }
	virtual void ActionEnd(DWORD pose_id) override;
	virtual bool IsKeepPose() override { return _isKeepPose; }
	virtual void FrameMove() override {}
	virtual void Cancel() override {}

	void SetPose(int nPos) { _nPose = nPos; }
	void SetKeepPose(bool v) { _isKeepPose = v; }

protected:
	virtual bool _Start() override;
	virtual bool _IsAllowCancel() override { return false; }

protected:
	int _nPose;
	bool _isKeepPose;
};

// 坐下状态
class CInsertState : public CActionState {
public:
	CInsertState(CActor* p);
	~CInsertState();

	virtual const char* GetExplain() override { return "CInsertState"; }
	virtual void Cancel() override;

	void SetAngle(int v) {
		_nAngle = v;
		_eAngle = enumAngle;
	}
	void SetFaceTo(int nScrX, int nScrY) {
		_nScrX = nScrX;
		_nScrY = nScrY;
		_eAngle = enumScrPos;
	}

protected:
	virtual bool _Start() override;

protected:
	enum eAngle {
		enumInit,
		enumAngle,
		enumScrPos,
	};
	eAngle _eAngle;
	int _nAngle;
	int _nScrX, _nScrY;

	bool _IsPlayPose;
	bool _IsFirst;
};

// 更换装备
struct stNetUseItem;
struct stNetItemUnfix;

class CEquipState : public CActionState {
public:
	CEquipState(CActor* p);
	~CEquipState();

	virtual const char* GetExplain() override { return "CEquipState"; }

	void SetUnfixData(stNetItemUnfix& data);
	void SetUseItemData(stNetUseItem& data);

protected:
	virtual bool _Start() override;

private:
	enum eType {
		enumInit,
		enumUseItem,
		enumUnfix,
	};
	eType _eType;

	stNetUseItem* _pUseItem;
	stNetItemUnfix* _pUnfix;
};
