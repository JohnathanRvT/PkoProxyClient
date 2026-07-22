
/* * * * * * * * * * * * * * * * * * * *

    数据类型
    Jampe
    2006/6/28

 * * * * * * * * * * * * * * * * * * * */

#if !defined __DATATYPE_H__
#define __DATATYPE_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned char byte;

#define __LN "\r\n"

#define SAFE_DELETE(p) \
	if ((p)) {         \
		delete (p);    \
		(p) = 0;       \
	}
#define SAFE_DELETE_ARRAY(p) \
	if ((p)) {               \
		delete[](p);         \
		(p) = 0;             \
	}

#define __BEGIN_TRY \
	try {
#define __COM_TRY            \
	}                        \
	catch (_com_error & e) { \
		LGPRT("Database", "Exception File: %s, Function: %s, Line: %i, ThreadID: %li, Code: %i, Description: %s"__LN, __FILE__, __FUNCTION__, __LINE__, GetCurrentThreadId(), e.WCode(), (char*)e.Description());
#define __END_TRY                                                                                                                                 \
	}                                                                                                                                             \
	catch (...) {                                                                                                                                 \
		LGPRT("System", "Exception File: %s, Function: %s, Line: %i, ThreadID: %li"__LN, __FILE__, __FUNCTION__, __LINE__, GetCurrentThreadId()); \
	}

extern char g_key[33];

#endif
