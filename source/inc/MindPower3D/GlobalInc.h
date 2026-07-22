#pragma once

#pragma warning(disable : 4251)
#pragma warning(disable : 4786)

//#include <d3d8.h>
//#include <d3dx8.h>
//#include <d3dutil.h>
#include "lwDirectX.h"

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>
#include <mmsystem.h>
#include <stdio.h>
#include <tchar.h>

#include "Util.h"

struct MPTexRect {
	int nTexSX{0};
	int nTexSY{0};
	int nTexW{0};
	int nTexH{0};
	float fScaleX{1.0f};
	float fScaleY{1.0f};
	DWORD dwColor{0xFFFFFFFF};
	int nTextureNo{-1};
};

inline void __cdecl LGX(const char* format, ...) {
	char buf[512];
	buf[0] = 'm';
	buf[1] = 's';
	buf[2] = 'g';
	buf[3] = 0;

	va_list args;
	va_start(args, format);
	_vsnprintf_s(&buf[3], sizeof(buf) - 4, _TRUNCATE, format, args);
	va_end(args);

	LG("default", buf);
}

#ifndef USE_LG_MSGBOX
#define USE_LG_MSGBOX
#define lwMessageBox LGX
#endif