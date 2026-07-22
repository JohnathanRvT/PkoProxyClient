
#pragma once

#include "ThreadBase.h"

class CAudioThread : public CThreadBase {
public:
	CAudioThread();
	virtual ~CAudioThread();

	unsigned int Run() override;

	void play(DWORD musID, bool loop = false);
	void FrameMove();

private:
	DWORD _nCurMusicID;
	bool _bLoop;
	DWORD _nLastTime;
};
