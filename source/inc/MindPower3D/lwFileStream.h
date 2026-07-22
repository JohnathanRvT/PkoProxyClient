//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwInterfaceExt.h"

LW_BEGIN

class lwFileStream : public lwIFileStream {
	LW_STD_DECLARATION()

private:
	lwIFile* _adapter_file;
	lwFileStreamOpenInfo _fsoi;

private:
public:
	lwFileStream();
	~lwFileStream();

	LW_RESULT Open(const char* file, const lwFileStreamOpenInfo* info) override;
	LW_RESULT Close() override;
	LW_RESULT Read(void* buf, DWORD in_size, DWORD* out_size) override;
	LW_RESULT Write(const void* buf, DWORD in_size, DWORD* out_size) override;
	LW_RESULT Seek(DWORD* pos, long offset, DWORD flag) override;
	LW_RESULT GetSize(DWORD* size) override;
	LW_RESULT Flush() override;
	LW_RESULT SetEnd() override;
};

LW_END