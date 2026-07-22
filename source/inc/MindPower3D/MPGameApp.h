//#################################
// MindPower GameApp Header File
// Render & GameApp Routines
//
// Created By Ryan Wang
// Last Modified : 2004/02/04
//#################################
#pragma once

#include "Mouse.h"

#define KEYBOARD_BUFFERSIZE 10
#define MOUSE_BUFFERSIZE 10

#define DI_KEY_NOACTION 0
#define DI_KEY_PRESSED 1
#define DI_KEY_RELEASED 2

// Added by CLP
#define KEY_FREE 0x0001
#define KEY_PUSH 0x0002
#define KEY_HOLD 0x0004
#define KEY_POP 0x0008
// Added by CLP

class MPCamera;
class MPRender;
class MPConsole;

struct LGInfo {
	bool bCloseAll;
	bool bEraseMode;
	bool bMsgBox;
	bool bEnableAll;
	char dir[260];
};

class MINDPOWER_API MPGameApp {
public:
	MPGameApp();
	~MPGameApp();
	virtual void _PreMouseRun(MouseClickState dwMouseKey) = 0;

	// camMove 标识是否进入Vim 3D Vision的渲染循环
	virtual void _FrameMove(DWORD dwTimeParam, bool camMove = false) = 0;
	virtual void _Render() = 0;
	virtual bool _Init() = 0;
	virtual void _End() = 0;

	bool Init(HINSTANCE hInst, const char* pszClassName, int nScrWidth = 800, int nScrHeight = 600, int nColorBit = 16, bool bFullScreen = false);
	void FrameMove(DWORD dwTimeParam);
	void Render();
	virtual void End();

	virtual bool _CanInput();
	virtual void MouseButtonDown(int nButton) = 0;
	virtual void MouseButtonUp(int nButton) = 0;
	virtual void MouseButtonDB(int nButton) = 0;
	virtual void MouseMove(int nOffsetX, int nOffsetY) = 0;
	virtual void MouseScroll(int nOffset) = 0;
	virtual void HandleKeyDown(DWORD dwKey) = 0;
	virtual void HandleKeyUp() = 0;
	virtual void MouseContinue(int nButton) = 0;

	MouseClickState GetMouseKey() const { return _dwMouseKey; }

	// input interface
	bool IsKeyContinue(BYTE btDIKey) const; // 检查按键是否连续按下
	bool IsKeyDown(BYTE btDIKey) const;
	bool IsMouseButtonPress(int nButtonNo) const; // 检查某个鼠标按钮是否按下
	bool IsCtrlPress() const {
		return (GetKeyState(VK_CONTROL) & 0xff00);
	}
	bool IsAltPress() const {
		return (GetKeyState(VK_MENU) & 0xff00);
	}
	bool IsShiftPress() const {
		return (GetKeyState(VK_SHIFT) & 0xff00);
	}

	int GetMouseX() const { return _nMousePosX; }
	int GetMouseY() const { return _nMousePosY; }
	void SetMouseXY(int nPosX, int nPosY) {
		_nMousePosX = nPosX;
		_nMousePosY = nPosY;
	}
	HWND GetHWND() const { return _hWnd; }
	int GetWindowWidth() const { return _nWindowWidth; }
	int GetWindowHeight() const { return _nWindowHeight; }
	bool IsFullScreen() const { return _bFullScreen; }
	void EnableCheckInputWnd(bool bEnable) { _bCheckInputWnd = bEnable; }

	// Console
	MPConsole* GetConsole() { return _pConsole; }
	void UpdateConsoleText(bool bClear);

	//MPCamera*			GetMainCam()  { return _pMainCam; }

	void SetCaption(const char* pszCaption);

	void EnableSprintScreen(bool bEnable) { _bEnSpScreen = bEnable; }
	bool IsEnableSpScreen() const { return _bEnSpScreen; }

	void EnableSprintAvi(bool bEnable) { _bEnSpAvi = bEnable; }
	bool IsEnableSpAvi() const { return _bEnSpAvi; }

	void EnableSprintSmMap(bool bEnable) { _bEnSpSmMap = bEnable; }
	bool IsEnableSpSmMap() const { return _bEnSpSmMap; }
	DWORD GetFrameMoveUseTime() const { return _dwFrameMoveUseTime; }
	DWORD GetRenderUseTime() const { return _dwRenderUseTime; }

	void SetInputActive(bool bActive);
	bool LoadTerrainSet(const char* file, bool bBinary);
	bool UnloadTerrainSet();
	bool LoadResourceSet(const char* file, int iMaxIndex, bool bBinary);
	bool UnloadResourceSet();

	bool LoadResource();
	bool LoadRes2();
	bool LoadRes3();

	virtual void LG_Config(const LGInfo& info);
	LGInfo* GetLGConfig() { return &_lgInfo; }

protected:
	bool _InitInput();
	void _RenderAxis();
	void _ReadKeyboardInput();
	void _SetupView(MPCamera* pCamera); // 通过镜头的信息设置D3D ViewMatrix

	// UI
	void _RenderUI();

protected:
	HINSTANCE _hInst;
	HWND _hWnd;

	bool _bActive{false};

	LGInfo _lgInfo;

	// Texture Management

	// Direct Input
	LPDIRECTINPUT8 _pDI{nullptr};
	LPDIRECTINPUTDEVICE8 _pDIKeyboard{nullptr}; // Keyboard
	LPDIRECTINPUTDEVICE8 _pDIMouse{nullptr};

	BYTE _bCanDB{false};

	HKL _KeyboardLayout;
	std::array<BYTE, 5> _btButtonState{};
	decltype(_btButtonState) _btLastButtonState{};

	UINT _nDBClickTime;
	UINT _nLastClickTime;
	bool _bLastDBClick{false};

	UINT _nDBTime{0};

	bool _bDrag;
	int _nLastDragX;
	int _nLastDragY;
	int _nMousePosX{0};
	int _nMousePosY{0};

	int _nLogoTexID;

	// Console
	MPConsole* _pConsole;

	//MPCamera*				_pMainCam;

	bool _bEnSpScreen{false};
	bool _bEnSpAvi{false};
	bool _bEnSpSmMap{false};
	int _nWindowWidth;
	int _nWindowHeight;
	bool _bFullScreen{false};
	bool _bCheckInputWnd;

	// performance routines
	DWORD _dwRenderUseTime{0};
	DWORD _dwFrameMoveUseTime{0};

	MouseClickState _dwMouseKey;

	DIMOUSESTATE2 _sDims2;

public:
	// ----- Added by CLP ----- //
	inline BYTE getKeyState(BYTE dikey) const;
	inline BYTE getASCIIKeyState(BYTE codeASCII) const;
	inline bool isKeyFree(BYTE dikey) const;
	inline bool isKeyPush(BYTE dikey) const;
	inline bool isKeyHold(BYTE dikey) const;
	inline bool isKeyPop(BYTE dikey) const;
	inline bool isKeyChange(BYTE dikey) const;

	inline bool isKeyStateDown(BYTE dikey) const;
	inline bool isKeyStateUp(BYTE dikey) const;

protected:
	std::array<BYTE, 256> mASCKeysState;
	std::array<BYTE, 256> mKeyState;
};

// 检查某个按键是否被连续按下
inline bool MPGameApp::IsKeyContinue(BYTE dikey) const {
	return isKeyHold(dikey);
}

inline bool MPGameApp::IsKeyDown(BYTE dikey) const {
	return isKeyPush(dikey);
}

inline bool MPGameApp::IsMouseButtonPress(int nButtonNo) const {
	return _btButtonState[nButtonNo];
}

// Added by CLP
inline bool MPGameApp::isKeyFree(BYTE dikey) const {
	return mKeyState[dikey] & KEY_FREE;
}

inline bool MPGameApp::isKeyPush(BYTE dikey) const {
	return mKeyState[dikey] & KEY_PUSH;
}

inline bool MPGameApp::isKeyHold(BYTE dikey) const {
	return mKeyState[dikey] & KEY_HOLD;
}

inline bool MPGameApp::isKeyPop(BYTE dikey) const {
	return mKeyState[dikey] & KEY_POP;
}

inline bool MPGameApp::isKeyChange(BYTE dikey) const {
	return (isKeyPush(dikey) || isKeyPop(dikey));
}

inline bool MPGameApp::isKeyStateDown(BYTE dikey) const {
	return (isKeyPush(dikey) || isKeyHold(dikey));
}

inline bool MPGameApp::isKeyStateUp(BYTE dikey) const {
	return (isKeyFree(dikey) || isKeyPop(dikey));
}

inline BYTE MPGameApp::getKeyState(BYTE dikey) const {
	return mKeyState[dikey];
}

inline BYTE MPGameApp::getASCIIKeyState(BYTE codeASCII) const {
	return mASCKeysState[codeASCII];
}
// Added by CLP
