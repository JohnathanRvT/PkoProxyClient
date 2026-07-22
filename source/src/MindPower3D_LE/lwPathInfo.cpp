//
#include "lwPathInfo.h"
#include "lwStdInc.h"

LW_BEGIN

LW_STD_IMPLEMENTATION(lwPathInfo)

lwPathInfo::lwPathInfo() {
	memset(_path_buf, 0, sizeof(char) * PATH_TYPE_NUM * LW_MAX_PATH);
}

char* lwPathInfo::SetPath(DWORD type, const char* path) {
	_tcsncpy_s(_path_buf[type], path, _TRUNCATE);
	return _path_buf[type];
}

char* lwPathInfo::GetPath(DWORD type) {
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
