//-----------------------------------------------------------------------------
// File: DXUtil.h
//
// Desc: Helper functions and typing shortcuts for DirectX programming.
//
// Copyright (c) 1997-2000 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#ifndef DXUTIL_H
#define DXUTIL_H

//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  \
	{                   \
		if (p) {        \
			delete (p); \
			(p) = NULL; \
		}               \
	}
#define SAFE_DELETE_ARRAY(p) \
	{                        \
		if (p) {             \
			delete[](p);     \
			(p) = NULL;      \
		}                    \
	}
#define SAFE_RELEASE(p)     \
	{                       \
		if (p) {            \
			(p)->Release(); \
			(p) = NULL;     \
		}                   \
	}
#endif

//-----------------------------------------------------------------------------
// Name: DXUtil_GetDXSDKMediaPath() and DXUtil_FindMediaFile()
// Desc: Returns the DirectX SDK path, as stored in the system registry
//       during the SDK install.
//-----------------------------------------------------------------------------
const char* DXUtil_GetDXSDKMediaPath();
HRESULT DXUtil_FindMediaFile(char* strPath, char* strFilename);

//-----------------------------------------------------------------------------
// Name: DXUtil_Read*RegKey() and DXUtil_Write*RegKey()
// Desc: Helper functions to read/write a string registry key
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteStringRegKey(HKEY hKey, char* strRegName, char* strValue);
HRESULT DXUtil_WriteIntRegKey(HKEY hKey, char* strRegName, DWORD dwValue);
HRESULT DXUtil_WriteGuidRegKey(HKEY hKey, char* strRegName, GUID guidValue);
HRESULT DXUtil_WriteBoolRegKey(HKEY hKey, char* strRegName, BOOL bValue);

HRESULT DXUtil_ReadStringRegKey(HKEY hKey, char* strRegName, char* strValue, DWORD dwLength, char* strDefault);
HRESULT DXUtil_ReadIntRegKey(HKEY hKey, char* strRegName, DWORD* pdwValue, DWORD dwDefault);
HRESULT DXUtil_ReadGuidRegKey(HKEY hKey, char* strRegName, GUID* pGuidValue, GUID& guidDefault);
HRESULT DXUtil_ReadBoolRegKey(HKEY hKey, char* strRegName, BOOL* pbValue, BOOL bDefault);

//-----------------------------------------------------------------------------
// Name: DXUtil_Timer()
// Desc: Performs timer opertations. Use the following commands:
//          TIMER_RESET           - to reset the timer
//          TIMER_START           - to start the timer
//          TIMER_STOP            - to stop (or pause) the timer
//          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
//          TIMER_GETABSOLUTETIME - to get the absolute system time
//          TIMER_GETAPPTIME      - to get the current time
//          TIMER_GETELAPSEDTIME  - to get the time that elapsed between
//                                  TIMER_GETELAPSEDTIME calls
//-----------------------------------------------------------------------------
enum TIMER_COMMAND {
	TIMER_RESET,
	TIMER_START,
	TIMER_STOP,
	TIMER_ADVANCE,
	TIMER_GETABSOLUTETIME,
	TIMER_GETAPPTIME,
	TIMER_GETELAPSEDTIME
};
FLOAT __stdcall DXUtil_Timer(TIMER_COMMAND command);

//-----------------------------------------------------------------------------
// Debug printing support
//-----------------------------------------------------------------------------
VOID DXUtil_Trace(TCHAR* strMsg, ...);
HRESULT _DbgOut(TCHAR*, DWORD, HRESULT, TCHAR*);

#if defined(DEBUG) | defined(_DEBUG)
#define DXTRACE DXUtil_Trace
#else
#define DXTRACE sizeof
#endif

#if defined(DEBUG) | defined(_DEBUG)
#define DEBUG_MSG(str) _DbgOut(__FILE__, (DWORD)__LINE__, 0, str)
#else
#define DEBUG_MSG(str) (0L)
#endif

#endif // DXUTIL_H
