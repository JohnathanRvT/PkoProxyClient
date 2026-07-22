//################################
// MindPower 3D Engine
// class MPGameApp Implement
// Created By   : Ryan Wang
// Last Modifed : 2004/02/06
//################################

#include "MindPowerAPI.h"
#include "Stdafx.h"
#include "MPGameApp.h"
#include "d3dutil.h"
#include "dxutil.h"
#include "MPCamera.h"
#include "MPTextureSet.h"
#include "MPTerrainSet.h"
#include "MPConsole.h"

#include "lwGuidObj.h"
#include "lwInterface.h"
#include "lwIFunc.h"
#include "MPCharacter.h"

#include "MPResourceSet.h"

#include "Vim_helper.h"
//Del by lark.li 20080611
//#include "FontSystem.h"

LW_USING

MPTexSet* MPTexSet::_Instance = nullptr;
MPTerrainSet* MPTerrainSet::_Instance = nullptr;
MPResourceSet* MPResourceSet::_Instance = nullptr;

MPGameApp::MPGameApp() {
	//_pMainCam = new MPCamera;
	_pConsole = new MPConsole;

	mASCKeysState.fill(KEY_FREE);
	mKeyState.fill(KEY_FREE);
}

MPGameApp::~MPGameApp() {}

bool MPGameApp::Init(HINSTANCE hInst, const char* pszClassName, int nScrWidth, int nScrHeight, int nColorBit, bool bFullScreen) {
	_hInst = hInst;

	const DWORD dwWindowStyle = bFullScreen ? WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN
											: WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	RECT rc;
	SetRect(&rc, 0, 0, nScrWidth, nScrHeight);
	AdjustWindowRect(&rc, dwWindowStyle, FALSE);

	const int nWindowWidth = rc.right - rc.left;
	const int nWindowHeight = rc.bottom - rc.top; // nScrHeight + nFrameSize + nCaptionSize;
	_hWnd = CreateWindow(pszClassName, "MindPower3D Application", dwWindowStyle,
						 CW_USEDEFAULT, CW_USEDEFAULT, nWindowWidth, nWindowHeight, nullptr, nullptr, hInst, nullptr);

	if (!_hWnd) {
		return false;
	}

	GetClientRect(_hWnd, &rc);
	_nWindowWidth = rc.right - rc.left;
	_nWindowHeight = rc.bottom - rc.top;
	_bFullScreen = bFullScreen;

	CLogMgr::Instance()->SetMessageWnd(_hWnd);

	const int dev_width = bFullScreen ? nScrWidth : _nWindowWidth;
	const int dev_height = bFullScreen ? nScrHeight : _nWindowHeight;
	if (!g_Render.Init(_hWnd, dev_width, dev_height, nColorBit, bFullScreen)) {
		return false;
	}

	// singleton MPTexSet Instantiation
	auto* pTextureSet = new MPTexSet(0, 2048);
	_nLogoTexID = GetTextureID("texture/logo.BMP");
	pTextureSet->EnableRequest(FALSE);

	_InitInput();

	// Del by lark.li 20080611
	// Added by clp
	//FontModule::FontSystem::getSingleton().init();

	if (_Init() == 0) {
		return false;
	}

	ShowWindow(_hWnd, SW_SHOW);
	UpdateWindow(_hWnd);

	return true;
}

bool MPGameApp::LoadResource() {
	if (!g_Render.InitResource()) {
		return false;
	}
	return true;
}

bool MPGameApp::LoadRes2() {
	if (!g_Render.InitRes2()) {
		return false;
	}

	return true;
}

bool MPGameApp::LoadRes3() {
	if (!g_Render.InitRes3()) {
		return false;
	}
	return true;
}

static DWORD _time;
void MPGameApp::FrameMove(DWORD dwTimeParam) {
	MPTimer tUseTime;
	tUseTime.Begin();

	_ReadKeyboardInput();

	g_Render.UpdateCullInfo(); //added by billy

	if (_pConsole->IsVisible()) {
		g_Render.EnablePrint(INFO_CMD, TRUE);
		_pConsole->FrameMove();
		UpdateConsoleText(false);
	} else {
		g_Render.EnablePrint(INFO_CMD, FALSE);
	}

	lwISceneMgr* sm = g_Render.GetInterfaceMgr()->sys_graphics->GetSceneMgr();
	sm->Update();

	_FrameMove(_time = dwTimeParam);

	_dwFrameMoveUseTime = tUseTime.End();
}

using namespace Vision3D;

void MPGameApp::Render() {
	MPTimer tRenderUse;
	tRenderUse.Begin();

	lwISceneMgr* sm = g_Render.GetInterfaceMgr()->sys_graphics->GetSceneMgr();

	// g_Render.LookAt(_pMainCam->m_EyePos, _pMainCam->m_RefPos);

	//  g_Render.SetCurrentView(MPRender::VIEW_WORLD);

	if (g_vimHelper.Is3D()) {

		LPDIRECT3DDEVICEX g_pd3dDevice = g_Render.GetDevice();

		LPDIRECT3DSURFACEX pOrigRenderTarget = nullptr;				   // ViM
		LPDIRECT3DSURFACEX pDepthStencilTarget = nullptr;			   // ViM
		if (FAILED(g_pd3dDevice->GetRenderTargetX(NULL, &pOrigRenderTarget))) // ViM
			return;													   // ViM

		if (FAILED(g_pd3dDevice->GetDepthStencilSurface(&pDepthStencilTarget))) // ViM
			return;																// ViM

		for (DWORD view = 0; view < g_dwNumViews; view++) // ViM
		{												  // ViM
			g_vimHelper.SetViewIndex(view);

			_FrameMove(_time, true); // VIM


#if (defined LW_USE_DX9)
			if (FAILED(g_pd3dDevice->SetRenderTarget(0, g_pRenderTargets[view])) || FAILED(g_pd3dDevice->SetDepthStencilSurface(pDepthStencilTarget))) // ViM
#elif (defined LW_USE_DX8)
			if (FAILED(g_pd3dDevice->SetRenderTarget(g_pRenderTargets[view], pDepthStencilTarget))) // ViM
#endif
				return; // ViM

			if (!g_Render.BeginRender(true))
				return;

			// _RenderAxis();

			sm->BeginRender();
			sm->Render();

			_Render(); // 子类应用程序调用

			sm->EndRender();

			g_Render.EndRender(false);

		} // ViM

#if (defined LW_USE_DX9)
		if (FAILED(g_pd3dDevice->SetRenderTarget(0, pOrigRenderTarget)) || FAILED(g_pd3dDevice->SetDepthStencilSurface(pDepthStencilTarget))) // ViM
#elif (defined LW_USE_DX8)
		if (FAILED(g_pd3dDevice->SetRenderTarget(pOrigRenderTarget, pDepthStencilTarget)))			// ViM
#endif
			return;						// ViM
		pOrigRenderTarget->Release();	// ViM
		pDepthStencilTarget->Release(); // ViM

		g_Render.BeginRender(false);

		if (FAILED(ViMD3D_RasterTextures(g_vimd3d, // ViM
										 0, 0, g_descWidth, g_descHeight,
										 g_dwNumViews, g_pRenderTextures))) // ViM
			return;															// ViM
	} else {
		///////////////////////////////////////////////////////////

		if (!g_Render.BeginRender(true))
			return;

		// _RenderAxis();

		sm->BeginRender();
		sm->Render();

		_Render(); // 子类应用程序调用

		sm->EndRender();

		///////////////////////////////////////////////////////////
	}

	//g_Render.EnableZBuffer(FALSE);
	//
	//   g_Render.SetCurrentView(MPRender::VIEW_WORLD);

#ifdef DEBUG
	g_Render.RenderDebugInfo();
#endif

	if (_pConsole->IsVisible()) {
		MPTexInfo* pTexInfo = GetTextureInfo(_nLogoTexID);
		if (pTexInfo) {
			MPTexRect TexRect;
			TexRect.nTexW = 128;
			TexRect.nTexH = 64;
			TexRect.fScaleX = (float)g_Render.GetScrWidth() / 128.0f / 1.76f;
			TexRect.fScaleY = 2.4f;
			TexRect.dwColor = 0x2FFFFFFF;
			TexRect.nTextureNo = _nLogoTexID;
			g_Render.RenderTextureRect(0, 0, &TexRect);
		}
	}

	// Del by lark.l i20080611
	// Draw font
	// FontModule::FontSystem::getSingleton().update(0.01f);

	g_Render.EndRender(true);

	_dwRenderUseTime = tRenderUse.End();
}

void MPGameApp::UpdateConsoleText(bool bClear) {
	int y = 4; // g_Render.GetScrHeight() - _pConsole->GetHeight() - 40;
	for (const auto& str : _pConsole->GetTextList()) {
		if (!str.empty() || bClear) {
			g_Render.Print(INFO_CMD, 2, y, "%s", str.c_str());
		}
		y += 16;
	}
	g_Render.Print(INFO_CMD, 4, _pConsole->GetHeight() - 10, "%s", _pConsole->GetInputText());
}

void MPGameApp::_RenderUI() {
}

void MPGameApp::_RenderAxis() {
	struct AXIS_VERTEX {
		D3DVECTOR pos;
		DWORD diffuse;
	};

	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);

#if (defined USE_MANAGED_RES)
	g_Render.SetTransformWorld(&mat);
#else
	g_Render.GetDevice()->SetTransform(D3DTS_WORLD, &mat);
#endif

	AXIS_VERTEX pVertices[6];

	// z
	pVertices[0].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVertices[1].pos = D3DXVECTOR3(0.0f, 0.0f, 50.0f);
	pVertices[0].diffuse = 0xffffffff;
	pVertices[1].diffuse = 0xffffffff;

	// x
	pVertices[2].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVertices[3].pos = D3DXVECTOR3(100.0f, 0.0f, 0.0f);
	pVertices[2].diffuse = 0xffff0000;
	pVertices[3].diffuse = 0xffff0000;

	// y
	pVertices[4].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	pVertices[5].pos = D3DXVECTOR3(0.0f, 100.0f, 0.0f);
	pVertices[4].diffuse = 0xFF00ffff;
	pVertices[5].diffuse = 0xFF00ffff;

	g_Render.SetRenderState(D3DRS_LIGHTING, FALSE);
	g_Render.SetTexture(0, nullptr);
#if (defined LW_USE_DX9)
	g_Render.SetVertexShader(nullptr);
#endif
	g_Render.SetFVFX(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	g_Render.GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 3, &pVertices, sizeof(AXIS_VERTEX));
	// _pD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
}

bool MPGameApp::_InitInput() {
	if (FAILED(DirectInput8Create(_hInst, DIRECTINPUT_VERSION,
								  IID_IDirectInput8,
								  (VOID**)&_pDI,
								  nullptr))) {
		LG("init", "Create DirectInput 8 Error!\n");
		return false;
	}

	_KeyboardLayout = GetKeyboardLayout(0);

	if (FAILED(_pDI->CreateDevice(GUID_SysKeyboard, &_pDIKeyboard, nullptr))) {
		if (_pDI)
			SAFE_RELEASE(_pDI);
		LG("init", "Create Keyboard Device Error\n");
		return false;
	}

	DIPROPDWORD dipdw;

	// Create buffer to hold keyboard data
	ZeroMemory(&dipdw, sizeof(DIPROPDWORD));
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = KEYBOARD_BUFFERSIZE; // Buffer Size

	HRESULT hr;
	// Set the format of the keyboard
	if (FAILED(hr = _pDIKeyboard->SetDataFormat(&c_dfDIKeyboard)))
		return false; //DisplayErrorMsg(hr, MSGERR_APPMUSTEXIT);

	// Set the co-operative level to exclusive access
	if (FAILED(hr = _pDIKeyboard->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
		return false; //DisplayErrorMsg(hr, MSGERR_APPMUSTEXIT);

	// Set the size of the buffer
	if (FAILED(hr = _pDIKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		return false; // DisplayErrorMsg(hr, MSGERR_APPMUSTEXIT);

	// Acquire the keyboard device
	_pDIKeyboard->Acquire();

	if (FAILED(hr = _pDI->CreateDevice(GUID_SysMouse, &_pDIMouse, nullptr)))
		return false;

	if (FAILED(hr = _pDIMouse->SetDataFormat(&c_dfDIMouse2)))
		return false;

	_pDIMouse->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

	_pDIMouse->Acquire();

	_nDBClickTime = ::GetDoubleClickTime();
	_nLastClickTime = timeGetTime();

	return true;
}

void MPGameApp::SetInputActive(bool bActive) {
	mASCKeysState.fill(KEY_FREE);
	mKeyState.fill(KEY_FREE);

	if (!bActive) {
		_pDIKeyboard->Unacquire();
		_pDIMouse->Unacquire();
	} else {
		_pDIKeyboard->Acquire();
		_pDIMouse->Acquire();
	}
	_bActive = bActive;
}
bool MPGameApp::LoadTerrainSet(const char* file, bool bBinary) {
	auto* pTerrainSet = new MPTerrainSet(0, 100);
	//pTerrainSet->LoadRawDataInfo("scripts/table/TerrainInfo", FALSE);
	return pTerrainSet->LoadRawDataInfo(file, bBinary);
}
bool MPGameApp::UnloadTerrainSet() {
	MPTerrainSet* pTerrainSet = MPTerrainSet::I();
	//SAFE_DELETE(pTerrainSet);
	return true;
}
//-----------------------------------------------------------------------------
bool MPGameApp::LoadResourceSet(const char* file, int iMaxIndex, bool bBinary) {
#if RESOURCE_SCRIPT == 2 || RESOURCE_SCRIPT == 3
	MPResourceSet* pResourceSet = new MPResourceSet(0, iMaxIndex);
	return pResourceSet->LoadRawDataInfo(file, bBinary);
#endif
	return true;
}
//-----------------------------------------------------------------------------
bool MPGameApp::UnloadResourceSet() {
	//MPResourceSet *pResourceSet = MPResourceSet::GetInstancePtr();
	//SAFE_DELETE(pResourceSet);

	return true;
}
//-----------------------------------------------------------------------------

void MPGameApp::_ReadKeyboardInput() {
	// set the single key strokes to 0 each time, so it just records changes between frames
	_btButtonState.fill(0); // 0 is no action
	_dwMouseKey = MouseClickState();

	if (!_bActive)
		return;

	if (!_CanInput())
		return;

	// Don't read the keyboard if the devices are invalid
	if (!_pDIKeyboard || !_pDI) {
		LG("error", "Keyboard interface is NULL!\n");
		return;
	}

	HRESULT hr = S_OK;

	//getdata:

	// GetDeviceData version
	static DIDEVICEOBJECTDATA keyDataBuffer[KEYBOARD_BUFFERSIZE];
	DWORD dwItems = KEYBOARD_BUFFERSIZE;

	// Read the buffered data
	hr = _pDIKeyboard->GetDeviceData(
		sizeof(DIDEVICEOBJECTDATA), keyDataBuffer, &dwItems, 0);
	if (FAILED(hr)) {
		// Keyboard may have been lost, reacquire it
		_pDIKeyboard->Acquire();
		return;
	}

	// Added by CLP
	// Proces the data if there is any
	if (dwItems) {
		// Process the buffered data
		for (DWORD i = 0; i < dwItems; ++i) {
			// Map scan-code to ASCII code
			UINT codeScan = keyDataBuffer[i].dwOfs;
			UINT codeASCII = MapVirtualKeyEx(codeScan, 1, _KeyboardLayout);

			if (keyDataBuffer[i].dwData & 0x80) {
				// The key was pressed
				switch (mKeyState[codeScan]) {
				case KEY_POP:
				case KEY_FREE:
					mKeyState[codeScan] = mASCKeysState[codeASCII] = KEY_PUSH;
					HandleKeyDown(codeScan);
					break;
				}
			} else {
				// The key was released
				switch (mKeyState[codeScan]) {
				case KEY_HOLD:
				case KEY_PUSH:
					mKeyState[codeScan] = mASCKeysState[codeASCII] = KEY_POP;
					break;
				}
			}
		}
	} else {
		// There isn't any buffered data, so keys state were not changed.
		for (DWORD i = 0; i < 256; ++i) {
			switch (mKeyState[i]) {
			case KEY_PUSH:
				mKeyState[i] = mASCKeysState[i] = KEY_HOLD;
				break;
			case KEY_POP:
				mKeyState[i] = mASCKeysState[i] = KEY_FREE;
				break;
			}
		}
	}

	/*
	// GetDeviceState version
	memcpy ( _lastKeyState, _curKeyState, sizeof ( _lastKeyState ) );

	// Get the immediate state of the keyboard
	hr = _pDIKeyboard->GetDeviceState ( sizeof ( _curKeyState ), _curKeyState );
	if ( FAILED ( hr ) )
	{
		// Keyboard may have been lost, reacquire it
		_pDIKeyboard->Acquire();
		return;
	}

	for ( size_t i = 0; i < 256; ++i )
	{
		if ( _curKeyState[ i ] & 0x80 && _lastKeyState[ i ] & 0x80 )
		{
			// The key has been hold yet.
			mKeyState[ i ] = keyHold;
			continue;
		} else if ( _curKeyState[ i ] & 0x80 ) {
			// The key was just pressed.
			mKeyState[ i ] = keyDown;
			continue;
		} else if ( _lastKeyState[ i ] & 0x80 ) {
			// The key was just released.
			mKeyState[ i ] = keyUp;
			continue;
		} else {
			// The key has been released yet.
			mKeyState[ i ] = keyReleased;
		}
	}
	// ----- Added by CLP ----- //
*/

	DIMOUSESTATE2 dims2;

	// Get the input's device state, and put the state in dims
	ZeroMemory(&dims2, sizeof(dims2));
	hr = _pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &dims2);
	if (FAILED(hr)) {
		_pDIMouse->Acquire();
		return;
	}

	int nOffsetX = dims2.lX;
	int nOffsetY = dims2.lY;
	int nScroll = dims2.lZ; // 中键滚动

	_btButtonState[0] = dims2.rgbButtons[0] & 0x80;
	_btButtonState[1] = dims2.rgbButtons[1] & 0x80;
	_btButtonState[2] = dims2.rgbButtons[2] & 0x80;
	_btButtonState[3] = dims2.rgbButtons[3] & 0x80;
	_btButtonState[4] = dims2.rgbButtons[4] & 0x80;

	if (nOffsetX || nOffsetY) // 鼠标有移动
	{
		//MouseMove(nOffsetX, nOffsetY);
		_bCanDB = false;

		_dwMouseKey |= MouseClickState::Move;
	}

	if (nScroll) {
		MouseScroll(nScroll);
	}
	UINT time = timeGetTime();

	for (auto i = 0; i < _btButtonState.size(); i++) {
		if (_btButtonState[i] && !_btLastButtonState[i]) {
			//MouseButtonDown(i);
			_dwMouseKey |= MouseClickState::Down;
			switch (i) {
			case 0: {
				_dwMouseKey |= MouseClickState::LDown;
				break;
			}
			case 1: {
				_dwMouseKey |= MouseClickState::RDown;
				break;
			}
			case 2: {
				_dwMouseKey |= MouseClickState::MDown;
				break;
			}
			case 3: {
				_dwMouseKey |= MouseClickState::X1Down;
				break;
			}
			case 4: {
				_dwMouseKey |= MouseClickState::X2Down;
				break;
			}
			}

			if ((time - _nLastClickTime < _nDBClickTime)) {
				if (!nOffsetX && !nOffsetY && _bCanDB) {
					if (_bLastDBClick) {
						if (time - _nDBTime > _nDBClickTime)
							_bLastDBClick = false;
					}
					if (!_bLastDBClick) {
						//MouseButtonDB(i);
						_bLastDBClick = true;
						_nDBTime = time;

						switch (i) {
						case 0: {
							_dwMouseKey |= MouseClickState::LDB;
							break;
						}
						case 1: {
							_dwMouseKey |= MouseClickState::RDB;
							break;
						}
						case 2: {
							_dwMouseKey |= MouseClickState::MDB;
							break;
						}
						case 3: {
							_dwMouseKey |= MouseClickState::X1DB;
							break;
						}
						case 4: {
							_dwMouseKey |= MouseClickState::X2DB;
							break;
						}
						}
					}
				}
			} else {
				_bLastDBClick = false;
			}
			_nLastClickTime = time;
			_bCanDB = true;
		}

		if (!_btButtonState[i] && _btLastButtonState[i]) {
			//MouseButtonUp(i);
			switch (i) {
			case 0: {
				_dwMouseKey |= MouseClickState::LUp;
				break;
			}
			case 1: {
				_dwMouseKey |= MouseClickState::RUp;
				break;
			}
			case 2: {
				_dwMouseKey |= MouseClickState::MUp;
				break;
			}
			case 3: {
				_dwMouseKey |= MouseClickState::X1Up;
				break;
			}
			case 4: {
				_dwMouseKey |= MouseClickState::X2Up;
				break;
			}
			}
		}
		if (_btButtonState[i] && _btLastButtonState[i]) {
			MouseContinue(i);
		}

		_btLastButtonState[i] = _btButtonState[i];
	}

	_PreMouseRun(_dwMouseKey);

	if ((_dwMouseKey & MouseClickState::Move) != MouseClickState()) {
		MouseMove(nOffsetX, nOffsetY);
	}

	if ((_dwMouseKey & MouseClickState::Down) != MouseClickState()) {
		if ((_dwMouseKey & MouseClickState::LDown) != MouseClickState())
			MouseButtonDown(0);
		if ((_dwMouseKey & MouseClickState::RDown) != MouseClickState())
			MouseButtonDown(1);
		if ((_dwMouseKey & MouseClickState::MDown) != MouseClickState())
			MouseButtonDown(2);
		if ((_dwMouseKey & MouseClickState::X1Down) != MouseClickState())
			MouseButtonDown(3);
		if ((_dwMouseKey & MouseClickState::X2Down) != MouseClickState())
			MouseButtonDown(4);
	}

	if ((_dwMouseKey & MouseClickState::LDB) != MouseClickState())
		MouseButtonDB(0);
	if ((_dwMouseKey & MouseClickState::RDB) != MouseClickState())
		MouseButtonDB(1);
	if ((_dwMouseKey & MouseClickState::MDB) != MouseClickState())
		MouseButtonDB(2);
	if ((_dwMouseKey & MouseClickState::X1DB) != MouseClickState())
		MouseButtonDB(3);
	if ((_dwMouseKey & MouseClickState::X2DB) != MouseClickState())
		MouseButtonDB(4);

	if ((_dwMouseKey & MouseClickState::LUp) != MouseClickState())
		MouseButtonUp(0);
	if ((_dwMouseKey & MouseClickState::RUp) != MouseClickState())
		MouseButtonUp(1);
	if ((_dwMouseKey & MouseClickState::MUp) != MouseClickState())
		MouseButtonUp(2);
	if ((_dwMouseKey & MouseClickState::X1Up) != MouseClickState())
		MouseButtonUp(3);
	if ((_dwMouseKey & MouseClickState::X2Up) != MouseClickState())
		MouseButtonUp(4);
}

void MPGameApp::SetCaption(const char* pszCaption) {
	if (_hWnd == nullptr)
		return;
	SetWindowText(_hWnd, pszCaption);
}

bool MPGameApp::_CanInput() {
	return true;
	if (::GetFocus() != _hWnd)
		return false;

	POINT p;
	RECT rc;
	::GetCursorPos(&p);
	::GetWindowRect(_hWnd, &rc);
	if (!PtInRect(&rc, p)) {
		return false;
	}
	return true;
}

void MPGameApp::End() {
	_End();

	SAFE_DELETE(_pConsole);

	SAFE_RELEASE(_pDIKeyboard);
	SAFE_RELEASE(_pDIMouse);

	SAFE_RELEASE(_pDI);

	g_Render.End();

	LG("end", "exit game appliaction successful!\n");
}

void MPGameApp::LG_Config(const LGInfo& info) {
	if (info.bCloseAll) {
		::LG_CloseAll();
	}
	::LG_SetEraseMode(info.bEraseMode);
	::LG_SetDir(info.dir);
	::LG_EnableAll(info.bEnableAll);
	::LG_EnableMsgBox(info.bMsgBox);

	_lgInfo = info;
}
