//
#pragma once

#include "lwHeader.h"
#include "lwITypes.h"
#include "lwInterfaceExt.h"

LW_BEGIN

class lwPathInfo : public lwIPathInfo {
public:
	std::string _path_buf[PATH_TYPE_NUM];

	LW_STD_DECLARATION()

public:
	lwPathInfo();

	std::string SetPath(DWORD type, std::string path) override;
	std::string GetPath(DWORD type) override;
};

class lwOptionMgr : public lwIOptionMgr {
	LW_STD_DECLARATION()
private:
	BYTE _ignore_model_tex_flag;
	BYTE _byte_flag_seq[MAX_OPTION_BYTE_FLAG];

public:
	lwOptionMgr();
	~lwOptionMgr();
	void SetIgnoreModelTexFlag(BYTE flag) override { _ignore_model_tex_flag = flag; }
	BYTE GetIgnoreModelTexFlag() const override { return _ignore_model_tex_flag; }
	void SetByteFlag(DWORD type, BYTE value) override { _byte_flag_seq[type] = value; }
	BYTE GetByteFlag(DWORD type) const override { return _byte_flag_seq[type]; }
};

LW_END