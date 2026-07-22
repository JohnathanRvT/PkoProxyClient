#pragma once
#include "STStateObj.h"

namespace GUI {
struct stSelectBox;
}

class CReadingState : public CActionState {
public:
	CReadingState(CActor* p);

	virtual const char* GetExplain() override {
		return "CReadingState";
	}

	virtual void FrameMove() override;
	virtual void BeforeNewState() override {
		PopState();
	}

protected:
	virtual bool _Start() override;
	virtual void _End() override;
	virtual bool _IsAllowCancel() override {
		return false;
	}
};