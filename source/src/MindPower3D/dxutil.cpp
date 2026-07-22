//-----------------------------------------------------------------------------
// File: DXUtil.cpp
//
// Desc: Shortcut macros and functions for using DX objects
//
//
// Copyright (c) 1997-2000 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdarg.h>
#include "DXUtil.h"

//-----------------------------------------------------------------------------
// Name: DXUtil_GetDXSDKMediaPath()
// Desc: Returns the DirectX SDK media path
//-----------------------------------------------------------------------------
const char* DXUtil_GetDXSDKMediaPath() {
	static char strNull[2] = "";
	static char strPath[MAX_PATH];
	DWORD dwType;
	DWORD dwSize = MAX_PATH;
	HKEY hKey;

	// Open the appropriate registry key
	LONG lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
								"Software\\Microsoft\\DirectX",
								0, KEY_READ, &hKey);
	if (ERROR_SUCCESS != lResult)
		return strNull;

	lResult = RegQueryValueEx(hKey, "DX8SDK Samples Path", nullptr,
							  &dwType, (BYTE*)strPath, &dwSize);
	RegCloseKey(hKey);

	if (ERROR_SUCCESS != lResult)
		return strNull;

	strcat_s(strPath, "\\Media\\");

	return strPath;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_FindMediaFile()
// Desc: Returns a valid path to a DXSDK media file
//-----------------------------------------------------------------------------
HRESULT DXUtil_FindMediaFile(char* strPath, char* strFilename, INT PathLen) {
	HANDLE file;

	if (nullptr == strFilename || nullptr == strPath)
		return E_INVALIDARG;

	// Check if the file exists in the current directory
	strncpy_s(strPath, strlen(strFilename) + 1, strFilename, _TRUNCATE);

	file = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ, nullptr,
					  OPEN_EXISTING, 0, nullptr);
	if (INVALID_HANDLE_VALUE != file) {
		CloseHandle(file);
		return S_OK;
	}

	// Check if the file exists in the current directory
	sprintf_s(strPath, PathLen, "%s%s", DXUtil_GetDXSDKMediaPath(), strFilename);

	file = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ, nullptr,
					  OPEN_EXISTING, 0, nullptr);
	if (INVALID_HANDLE_VALUE != file) {
		CloseHandle(file);
		return S_OK;
	}

	// On failure, just return the file as the path
	strncpy_s(strPath, strlen(strFilename) + 1, strFilename, _TRUNCATE);
	return E_FAIL;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_ReadStringRegKey()
// Desc: Helper function to read a registry key string
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadStringRegKey(HKEY hKey, char* strRegName, char* strValue,
								DWORD dwLength, char* strDefault) {
	DWORD dwType;

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, strRegName, nullptr, &dwType,
										 (BYTE*)strValue, &dwLength)) {
		strncpy_s(strValue, strlen(strDefault) + 1, strDefault, _TRUNCATE);
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_WriteStringRegKey()
// Desc: Helper function to write a registry key string
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteStringRegKey(HKEY hKey, char* strRegName,
								 char* strValue) {
	if (ERROR_SUCCESS != RegSetValueEx(hKey, strRegName, 0, REG_SZ,
									   (BYTE*)strValue,
									   (strlen(strValue) + 1) * sizeof(char)))
		return E_FAIL;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_ReadIntRegKey()
// Desc: Helper function to read a registry key int
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadIntRegKey(HKEY hKey, char* strRegName, DWORD* pdwValue,
							 DWORD dwDefault) {
	DWORD dwType;
	DWORD dwLength = sizeof(DWORD);

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, strRegName, nullptr, &dwType,
										 (BYTE*)pdwValue, &dwLength)) {
		*pdwValue = dwDefault;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_WriteIntRegKey()
// Desc: Helper function to write a registry key int
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteIntRegKey(HKEY hKey, char* strRegName, DWORD dwValue) {
	if (ERROR_SUCCESS != RegSetValueEx(hKey, strRegName, 0, REG_DWORD,
									   (BYTE*)&dwValue, sizeof(DWORD)))
		return E_FAIL;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_ReadBoolRegKey()
// Desc: Helper function to read a registry key BOOL
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadBoolRegKey(HKEY hKey, char* strRegName, BOOL* pbValue,
							  BOOL bDefault) {
	DWORD dwType;
	DWORD dwLength = sizeof(BOOL);

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, strRegName, nullptr, &dwType,
										 (BYTE*)pbValue, &dwLength)) {
		*pbValue = bDefault;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_WriteBoolRegKey()
// Desc: Helper function to write a registry key BOOL
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteBoolRegKey(HKEY hKey, char* strRegName, BOOL bValue) {
	if (ERROR_SUCCESS != RegSetValueEx(hKey, strRegName, 0, REG_DWORD,
									   (BYTE*)&bValue, sizeof(BOOL)))
		return E_FAIL;

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_ReadGuidRegKey()
// Desc: Helper function to read a registry key guid
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadGuidRegKey(HKEY hKey, char* strRegName, GUID* pGuidValue,
							  GUID& guidDefault) {
	DWORD dwType;
	DWORD dwLength = sizeof(GUID);

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, strRegName, nullptr, &dwType,
										 (LPBYTE)pGuidValue, &dwLength)) {
		*pGuidValue = guidDefault;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_WriteGuidRegKey()
// Desc: Helper function to write a registry key guid
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteGuidRegKey(HKEY hKey, char* strRegName, GUID guidValue) {
	if (ERROR_SUCCESS != RegSetValueEx(hKey, strRegName, 0, REG_BINARY,
									   (BYTE*)&guidValue, sizeof(GUID)))
		return E_FAIL;

	return S_OK;
}

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
FLOAT __stdcall DXUtil_Timer(TIMER_COMMAND command) {
	static BOOL m_bTimerInitialized = FALSE;
	static BOOL m_bUsingQPF = FALSE;
	static LONGLONG m_llQPFTicksPerSec = 0;

	// Initialize the timer
	if (FALSE == m_bTimerInitialized) {
		m_bTimerInitialized = TRUE;

		// Use QueryPerformanceFrequency() to get frequency of timer.  If QPF is
		// not supported, we will timeGetTime() which returns milliseconds.
		LARGE_INTEGER qwTicksPerSec;
		m_bUsingQPF = QueryPerformanceFrequency(&qwTicksPerSec);
		if (m_bUsingQPF)
			m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
	}

	if (m_bUsingQPF) {
		static LONGLONG m_llStopTime = 0;
		static LONGLONG m_llLastElapsedTime = 0;
		static LONGLONG m_llBaseTime = 0;
		double fTime;
		double fElapsedTime;
		LARGE_INTEGER qwTime;

		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		if (m_llStopTime != 0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
			qwTime.QuadPart = m_llStopTime;
		else
			QueryPerformanceCounter(&qwTime);

		// Return the elapsed time
		if (command == TIMER_GETELAPSEDTIME) {
			fElapsedTime = (double)(qwTime.QuadPart - m_llLastElapsedTime) / (double)m_llQPFTicksPerSec;
			m_llLastElapsedTime = qwTime.QuadPart;
			return (FLOAT)fElapsedTime;
		}

		// Return the current time
		if (command == TIMER_GETAPPTIME) {
			double fAppTime = (double)(qwTime.QuadPart - m_llBaseTime) / (double)m_llQPFTicksPerSec;
			return (FLOAT)fAppTime;
		}

		// Reset the timer
		if (command == TIMER_RESET) {
			m_llBaseTime = qwTime.QuadPart;
			m_llLastElapsedTime = qwTime.QuadPart;
			return 0.0f;
		}

		// Start the timer
		if (command == TIMER_START) {
			m_llBaseTime += qwTime.QuadPart - m_llStopTime;
			m_llStopTime = 0;
			m_llLastElapsedTime = qwTime.QuadPart;
			return 0.0f;
		}

		// Stop the timer
		if (command == TIMER_STOP) {
			m_llStopTime = qwTime.QuadPart;
			m_llLastElapsedTime = qwTime.QuadPart;
			return 0.0f;
		}

		// Advance the timer by 1/10th second
		if (command == TIMER_ADVANCE) {
			m_llStopTime += m_llQPFTicksPerSec / 10;
			return 0.0f;
		}

		if (command == TIMER_GETABSOLUTETIME) {
			fTime = qwTime.QuadPart / (double)m_llQPFTicksPerSec;
			return (FLOAT)fTime;
		}

		return -1.0f; // Invalid command specified
	} else {
		// Get the time using timeGetTime()
		static double m_fLastElapsedTime = 0.0;
		static double m_fBaseTime = 0.0;
		static double m_fStopTime = 0.0;
		double fTime;
		double fElapsedTime;

		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		if (m_fStopTime != 0.0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
			fTime = m_fStopTime;
		else
			fTime = timeGetTime() * 0.001;

		// Return the elapsed time
		if (command == TIMER_GETELAPSEDTIME) {
			fElapsedTime = (double)(fTime - m_fLastElapsedTime);
			m_fLastElapsedTime = fTime;
			return (FLOAT)fElapsedTime;
		}

		// Return the current time
		if (command == TIMER_GETAPPTIME) {
			return (FLOAT)(fTime - m_fBaseTime);
		}

		// Reset the timer
		if (command == TIMER_RESET) {
			m_fBaseTime = fTime;
			m_fLastElapsedTime = fTime;
			return 0.0f;
		}

		// Start the timer
		if (command == TIMER_START) {
			m_fBaseTime += fTime - m_fStopTime;
			m_fStopTime = 0.0f;
			m_fLastElapsedTime = fTime;
			return 0.0f;
		}

		// Stop the timer
		if (command == TIMER_STOP) {
			m_fStopTime = fTime;
			return 0.0f;
		}

		// Advance the timer by 1/10th second
		if (command == TIMER_ADVANCE) {
			m_fStopTime += 0.1f;
			return 0.0f;
		}

		if (command == TIMER_GETABSOLUTETIME) {
			return (FLOAT)fTime;
		}

		return -1.0f; // Invalid command specified
	}
}

//-----------------------------------------------------------------------------
// Name: _DbgOut()
// Desc: Outputs a message to the debug stream
//-----------------------------------------------------------------------------
HRESULT _DbgOut(char* strFile, DWORD dwLine, HRESULT hr, char* strMsg) {
	char buffer[256];
	sprintf_s(buffer, _TRUNCATE, "%s(%ld): ", strFile, dwLine);
	OutputDebugString(buffer);
	OutputDebugString(strMsg);

	if (hr) {
		sprintf_s(buffer, _TRUNCATE, "(hr=%08lx)\n", hr);
		OutputDebugString(buffer);
	}

	OutputDebugString("\n");

	return hr;
}

//-----------------------------------------------------------------------------
// Name: DXUtil_Trace()
// Desc: Outputs to the debug stream a formatted string with a variable-
//       argument list.
//-----------------------------------------------------------------------------
VOID DXUtil_Trace(char* strMsg, ...) {
#if defined(DEBUG) | defined(_DEBUG)
	char strBuffer[512];

	va_list args;
	va_start(args, strMsg);
	_vsnprintf_s(strBuffer, 512, _TRUNCATE, strMsg, args);
	va_end(args);

	OutputDebugString(strBuffer);
#endif
}
