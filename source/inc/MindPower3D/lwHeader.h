//
#pragma once

#define LW_NAMESPACE MindPower
#define LW_BEGIN \
	namespace LW_NAMESPACE {
#define LW_END }
#define LW_USING using namespace LW_NAMESPACE;

#define LW_MAX_PATH 260
#define LW_MAX_FILE 128
#define LW_MAX_NAME 64
#define LW_MAX_STRING 256
#define LW_CHAR_16 16
#define LW_CHAR_32 32

#define LW_MALLOC(obj) malloc(obj)
#define LW_REALLOC(p, obj) realloc(p, obj)
#define LW_FREE(obj) free(obj)
#define LW_NEW(obj) new obj
#define LW_DELETE(obj) delete obj
#define LW_DELETE_A(obj) delete[] obj
#define LW_RELEASE(obj) (obj)->Release()

#ifndef LW_SAFE_FREE
#define LW_SAFE_FREE(obj) \
	{                     \
		if (obj) {        \
			LW_FREE(obj); \
			obj = 0;      \
		}                 \
	}
#endif

#ifndef LW_IF_FREE
#define LW_IF_FREE(obj) \
	if (obj) {          \
		LW_FREE(obj);   \
	}
#endif

#ifndef LW_SAFE_DELETE
#define LW_SAFE_DELETE(obj) \
	if (obj) {              \
		LW_DELETE(obj);     \
		obj = 0;            \
	}
#endif

#ifndef LW_IF_DELETE
#define LW_IF_DELETE(obj) \
	if (obj) {            \
		LW_DELETE(obj);   \
	}
#endif

#ifndef LW_SAFE_DELETE_A
#define LW_SAFE_DELETE_A(obj) \
	if (obj) {                \
		LW_DELETE_A(obj);     \
		obj = 0;              \
	}
#endif

#ifndef LW_IF_DELETE_A
#define LW_IF_DELETE_A(obj) \
	if (obj) {              \
		LW_DELETE_A(obj);   \
	}
#endif

#ifndef LW_SAFE_RELEASE
#define LW_SAFE_RELEASE(obj) \
	{                        \
		if (obj) {           \
			LW_RELEASE(obj); \
			obj = 0;         \
		}                    \
	}
#endif

#ifndef LW_IF_RELEASE
#define LW_IF_RELEASE(obj) \
	if (obj) {             \
		LW_RELEASE(obj);   \
	}
#endif

#define LW_SIMPLE_IF_GET(dst, src) \
	if (dst) {                     \
		*dst = src;                \
	}

#define LW_SUCCEEDED_RET(code) \
	if (LW_SUCCEEDED(code)) {  \
		goto __ret;            \
	}
#define LW_FAILED_RET(code) \
	if (LW_FAILED(code)) {  \
		goto __ret;         \
	}
#define LW_ZERO_RET(code) \
	if ((code) == 0) {    \
		goto __ret;       \
	}
//((size * count) > sizeof(buffer)) || 
#define LW_fread(buffer, size, count, pointer) \
	if (fread(buffer, size, count, pointer) != count) { \
		return LW_RET_FAILED; \
	}

#define LW_INVALID_INDEX 0xffffffff
#define LW_INVALID_HANDLE LW_INVALID_INDEX

using LW_VOID = void;
using LW_CHAR = char;
using LW_BOOL = int;
using LW_LONG = long;
using LW_ULONG = unsigned long;
using LW_INT64 = unsigned __int64;
using LW_RESULT = LW_LONG;
using LW_DWORD = LW_ULONG;
using LW_HANDLE = LW_ULONG;

// ===================
using lxi8 = char;
using lxi16 = short;
using lxi32 = int;
using lxi64 = __int64;
using lxu8 = unsigned char;
using lxu16 = unsigned short;
using lxu32 = unsigned int;
using lxu64 = unsigned __int64; // ???
using lxf32 = float;
using lxf64 = double;
using lxvoid = void;
using lxdword = lxu32;
using lxresult = lxi32;
using lxhandle = lxi32;
// end

#define LW_INLINE __forceinline
#define LW_DECLSPEC_NOVTABLE __declspec(novtable)
#define PURE_METHOD = 0
//#define LW_DECLSPEC_NOVTABLE

#define LW_NULL 0
#define LW_MAKEINT64(l, h) ((LW_INT64)(((LW_DWORD)(l)) | ((LW_INT64)((LW_DWORD)(h))) << 32))
#define LW_LODWORD(i) ((LW_DWORD)(i))
#define LW_HIDWORD(i) ((LW_DWORD)((((LW_INT64)(i)) >> 32) & 0xffffffff))

#define LW_ALIGN(adrs, bytes) (((UINT)adrs + ((bytes)-1)) & (~((bytes)-1)))
#define LW_ARRAY_LENGTH(buf) (sizeof(buf) / sizeof(buf[0]))

#define LW_RET_OK 0
#define LW_RET_FAILED -1
