#pragma once

#include "MindPower.h"


class GameApp : public MPGameApp {
public:
	int Run();
	// Inherited via MPGameApp
	virtual void _PreMouseRun(MouseClickState dwMouseKey) override;
	virtual void _FrameMove(DWORD dwTimeParam, bool camMove = false) override;
	virtual void _Render() override;
	virtual bool _Init() override;
	virtual void _End() override;
	virtual void MouseButtonDown(int nButton) override;
	virtual void MouseButtonUp(int nButton) override;
	virtual void MouseButtonDB(int nButton) override;
	virtual void MouseMove(int nOffsetX, int nOffsetY) override;
	virtual void MouseScroll(int nOffset) override;
	virtual void HandleKeyDown(DWORD dwKey) override;
	virtual void HandleKeyUp() override;
	virtual void MouseContinue(int nButton) override;
};
