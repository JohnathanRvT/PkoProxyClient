//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwInterfaceExt.h"

LW_BEGIN

class lwConsole : public lwIConsole {
	LW_STD_DECLARATION()

private:
	HANDLE _handle;

public:
	lwConsole();
	~lwConsole();

	LW_RESULT Alloc() override;
	LW_RESULT Create(DWORD desired_access = GENERIC_READ | GENERIC_WRITE, DWORD share_mode = 0, const SECURITY_ATTRIBUTES* security_attr = nullptr) override;
	LW_RESULT Destroy() override;
	LW_RESULT Write(const char* str, ...) override;
	LW_RESULT SetBufferSize(DWORD row, DWORD column) override;
	BOOL SetActive() override { return ::SetConsoleActiveScreenBuffer(_handle); }
	HANDLE GetHandle() const override { return _handle; }
};

LW_END