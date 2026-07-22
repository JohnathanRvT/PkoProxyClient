//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwErrorCode.h"
#include "lwInterfaceExt.h"

LW_BEGIN

class lwFile : public lwIFile {
	LW_STD_DECLARATION();

	static DWORD __dummy;

private:
	HANDLE _handle;
	char _file[LW_MAX_PATH];

private:
	LW_RESULT _CheckDirectory(const char* path);

public:
	lwFile() : _handle(INVALID_HANDLE_VALUE) { _file[0] = '\0'; }
	~lwFile() { Close(); }

	// Create Description
	// access_flag[in]: GENERIC_READ / GENERIC_WRITE
	// share_model[in]: 0 / FILE_SHARE_DELETE / FILE_SHARE_READ / FILE_SHARE_WRITE
	// creation_flag[in]: CREATE_NEW / CREATE_ALWAYS / OPEN_EXISTING / OPEN_ALWAYS / TRUNCATE_EXISTING
	LW_RESULT CreateFile(const char* file, DWORD access_flag, DWORD share_mode, LPSECURITY_ATTRIBUTES secu_attr, DWORD creation_flag, DWORD attributes_flag = FILE_FLAG_SEQUENTIAL_SCAN);
	LW_RESULT CreateDirectory(const char* path, LPSECURITY_ATTRIBUTES attr);
	LW_RESULT LoadFileBuffer(const char* file, lwIBuffer* buf) override;
	LW_RESULT SaveFileBuffer(const char* file, lwIBuffer* buf) override;
	LW_RESULT Close() override;
	LW_RESULT Read(void* buf, DWORD in_size, DWORD* out_size) override;
	LW_RESULT Write(const void* buf, DWORD in_size, DWORD* out_size) override;

	HANDLE GetHandle() const override { return _handle; }
	const char* GetFileName() override { return _file; }
	LW_RESULT GetCreationTime(SYSTEMTIME* st) override;
	LW_RESULT CheckExisting(const char* path, DWORD check_directory) override;

	// Seek Description
	// flag[in]: FILE_BEGIN / FILE_CURRENT / FILE_END
	LW_RESULT Seek(long offset, DWORD flag) override;
	DWORD GetSize() override { return GetFileSize(_handle, nullptr); }
	LW_RESULT Flush() override { return FlushFileBuffers(_handle); }
	LW_RESULT SetEnd() override { return SetEndOfFile(_handle); }

	LW_RESULT MoveData(DWORD src_pos, DWORD dst_pos, DWORD size) override;
	LW_RESULT ReplaceData(DWORD pos, const void* buf, DWORD size) override;
	LW_RESULT InsertData(DWORD pos, const void* buf, DWORD size) override;
	LW_RESULT RemoveData(DWORD pos, DWORD size) override;
};

class lwFileDialog : public lwIFileDialog {
	LW_STD_DECLARATION();

public:
	// int flag = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY
	LW_RESULT GetOpenFileName(HWND hwnd, char* buf, int num, const char* dir, const char* title = nullptr,
							  const char* filter = "all files(*.*)\0*.*\0\0", int flag = OFN_PATHMUSTEXIST | OFN_EXPLORER);

	// flag = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY
	LW_RESULT GetSaveFileName(HWND hwnd, char* buf, int num, const char* dir, const char* title = nullptr,
							  const char* filter = "all files(*.*)\0*.*\0\0", const char* ext = nullptr, int flag = OFN_PATHMUSTEXIST | OFN_EXPLORER);
};

LW_END