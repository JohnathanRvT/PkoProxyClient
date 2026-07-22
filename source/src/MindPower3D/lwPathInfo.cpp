//
#include "lwPathInfo.h"
#include "lwStdInc.h"

LW_BEGIN

LW_STD_IMPLEMENTATION(lwPathInfo)

lwPathInfo::lwPathInfo() {
}

std::string lwPathInfo::SetPath(DWORD type, std::string path) {
	_path_buf[type] = path;
	return _path_buf[type];
}

std::string lwPathInfo::GetPath(DWORD type) {
	return _path_buf[type];
}

// lwOptionMgr
LW_STD_IMPLEMENTATION(lwOptionMgr)

lwOptionMgr::lwOptionMgr() {
	memset(_byte_flag_seq, 0, sizeof(_byte_flag_seq));

	_ignore_model_tex_flag = 0;
}
lwOptionMgr::~lwOptionMgr() {
}

LW_END