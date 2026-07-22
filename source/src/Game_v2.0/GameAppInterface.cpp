#include "Stdafx.h"

#include "GameApp.h"

#include "Character.h"

#include "SmallMap.h"
#include "LoginScene.h"
#include "WorldScene.h"

#include "PacketCmd.h"
#include "UIFormMgr.h"
#include "uilabel.h"
#include "UITemplete.h"
#include "uicozeform.h"
#include "uiboxform.h"

#include "createchascene.h"
#include "SelectChaScene.h"
#include "uistartform.h"

#include "gameloading.h"

#include "StringLib.h"
#include "SceneItem.h"

extern CGameMovie g_GameMovie;

#ifndef USE_DSOUND
#include "AudioThread.h"
extern CAudioThread g_AudioThread;
#endif

extern DWORD g_dwCurMusicID;

static bool IsExistFile(const char* file) {
	HANDLE hFile = CreateFile(file, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == hFile) {
		return false;
	} else {
		CloseHandle(hFile);
		return true;
	}
}

static void __timer_frame(DWORD time) {
	g_pGameApp->FrameMove(time);
}
static void __timer_render(DWORD time) {
	g_pGameApp->Render();
}

void CALLBACK __timer_period_proc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
	auto hwnd = reinterpret_cast<HWND>(dwUser);
	::SendMessage(hwnd, WM_USER_TIMER, 0, 0);
}

void __timer_period_render() {
	if (g_pGameApp->IsRun()) {
		DWORD Tick = timeGetTime();
		if ((Tick - g_pGameApp->GetCurTick()) > 18) {
			g_pGameApp->SetTickCount(Tick);
			g_pGameApp->FrameMove(Tick);
			g_pGameApp->Render();
		}
	}
}

int CGameApp::Run() {
#define R_FAIL -1;
#define R_OK 0;

	_dwLoadingTick = g_pGameApp->GetCurTick();

#if (defined USE_TIMERPERIOD)

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#else
	_dwCurTick = 0;

#if (defined USE_INDIVIDUAL_TIMER)
	MPITimer* timer = 0;
	MPGUIDCreateObject((LW_VOID**)&timer, LW_GUID_TIMER);
	timer->SetTimer(0, __timer_frame, 1.0f / 30);
	timer->SetTimer(1, __timer_render, 1.0f / 30);
#endif

	//ResetGameCamera(0);

	//CCameraCtrl *pCam = GetMainCam();
	//MPTerrain *pTerr = GetCurScene()->GetTerrain();
	//CCharacter *pCha = _pCurScene->GetMainCha();
	//if(pCha)
	//{
	//	D3DXVECTOR3 vecCha = pCha->GetPos();

	//	_pCamTrack->Reset(vecCha.x,vecCha.y,vecCha.z);
	//	pCam->SetFollowObj(vecCha);
	//	pCam->FrameMove(0);
	//	g_Render.SetWorldViewFOV(Angle2Radian(pCam->m_ffov));
	//	g_Render.LookAt(pCam->m_EyePos, pCam->m_RefPos);
	//	g_Render.SetCurrentView(MPRender::VIEW_WORLD);
	//}

	_isRun = true;

	if (!_pSteady->Init())
		return -1;

	MSG msg{};
	while (_isRun) {
		if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		if (_pCurScene) {
			//if(g_GameMovie.IsPlaying())	// 视频播放特殊处理
			//{
			//	if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			//	{
			//		g_GameMovie.Cleanup();
			//	}
			//	continue;
			//}
			//else
			//{
			//	g_GameMovie.Cleanup();
			//}

#if (defined USE_INDIVIDUAL_TIMER)
			timer->OnTimer();
#else

			g_NetIF->PeekPacket(1); // 修正客户端CPU占用率100%
			if (_pSteady->Run()) {
				LG("frame", "time:%u\n", GetTickCount() - _dwCurTick);

				g_Render.GetInterfaceMgr()->tp_loadres->SetPoolEvent(TRUE);
				_dwCurTick = _pSteady->GetTick();
				//_FrameMoveOnce( _dwCurTick );
				FrameMove(_dwCurTick);
				Render();

				if (_nSwitchScene > 0) {
					_ShowLoading();
				}

				_pSteady->End();
				g_Render.GetInterfaceMgr()->tp_loadres->SetPoolEvent(FALSE);
			}
#endif
		}
	}

#if (defined USE_INDIVIDUAL_TIMER)
	timer->Release();
#endif
#endif
	return (int)msg.wParam;
}

void CGameApp::AutoTestUpdate() {
	if (!_pCurScene)
		return;
	if (!_isRun)
		exit(-1);

	MSG msg{};
	int i = 0;
	for (;;) {
		if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		g_Render.GetInterfaceMgr()->tp_loadres->SetPoolEvent(TRUE);
		_dwCurTick = ::GetTickCount();
		FrameMove(_dwCurTick);
		Render();
		g_Render.GetInterfaceMgr()->tp_loadres->SetPoolEvent(FALSE);

		if (i > 1)
			return;
		i++;
	}
}

void CGameApp::GotoScene(CGameScene* scene, bool isDelCurScene, bool IsShowLoading) {
	if (!scene) {
		MsgBox(RES_STRING(CL_LANGUAGE_MATCH_71));
		return;
	}

	if (_pCurScene && !_pCurScene->_Clear()) {
		_SceneError(RES_STRING(CL_LANGUAGE_MATCH_72), _pCurScene);
		SetIsRun(false);
		return;
	}

	if (isDelCurScene && _pCurScene && _pCurScene != scene) {
		delete _pCurScene;
	}
	_pCurScene = scene;

	CFormMgr::s_Mgr.SetEnabled(true);
	CFormMgr::s_Mgr.ResetAllForm();
	CFormMgr::s_Mgr.SwitchTemplete(_pCurScene->GetInitParam()->nUITemplete);

	if (!_pCurScene->_Init()) {
		_SceneError(RES_STRING(CL_LANGUAGE_MATCH_73), _pCurScene);
		SetIsRun(false);
		return;
	}

	if (IsShowLoading) {
		_dwLoadingTick = g_pGameApp->GetCurTick();

		static bool isFirstWorld = false;
		if (!isFirstWorld && dynamic_cast<CWorldScene*>(scene)) {
			// 第一次切换到游戏场景是较慢,等待时间要加长
			isFirstWorld = true;
			Loading(100);
		} else {
			Loading();
		}
	}
}

void CGameApp::ResetGameCamera(int type) {
	CCameraCtrl* pCam = GetMainCam();
	pCam->SetModel(type);
	//if(type==0)
	//{
	//	pCam->m_stDist = g_Config.m_fminxy;
	//	pCam->m_enDist = g_Config.m_fmaxxy;
	//	pCam->m_stHei = g_Config.m_fminHei;
	//	pCam->m_enHei = g_Config.m_fmaxHei;
	//	pCam->m_stFov = g_Config.m_fminfov;
	//	pCam->m_enFov = g_Config.m_fmaxfov;

	//	pCam->m_fxy = g_Config.m_fmaxxy;
	//	pCam->m_fz  = g_Config.m_fmaxHei;
	//	pCam->m_ffov = g_Config.m_fmaxfov;
	//}else
	//{
	//	pCam->m_stDist = g_Config.m_fminxy2;
	//	pCam->m_enDist = g_Config.m_fmaxxy2;
	//	pCam->m_stHei = g_Config.m_fminHei2;
	//	pCam->m_enHei = g_Config.m_fmaxHei2;
	//	pCam->m_stFov = g_Config.m_fminfov2;
	//	pCam->m_enFov = g_Config.m_fmaxfov2;

	//	pCam->m_fxy = g_Config.m_fmaxxy2;
	//	pCam->m_fz  = g_Config.m_fmaxHei2;
	//	pCam->m_ffov = g_Config.m_fmaxfov2;

	//}
	//pCam->m_fstep1 = 1.0f;

	//pCam->m_fResetRotatVel = g_Config.m_fCameraVel;
	//pCam->m_fResetRotatAccl = g_Config.m_fCameraAccl;

	//////pCam->SetFollowObj(_pCurScene->GetMainCha()->GetPos());
	//GetMainCam()->InitAngle(0.5235987f + 0.0872664f);
	////GetMainCam()->ResetCamera(0.5235987f + 0.0872664f);
	//g_Render.SetWorldViewFOV(Angle2Radian( GetMainCam()->m_ffov));
}

void CGameApp::ResetCamera() {
	CCameraCtrl* pCam = g_pGameApp->GetMainCam();
	GetMainCam()->ResetCamera(0.5235987f + 0.0872664f);
	//g_Render.SetWorldViewFOV(Angle2Radian( GetMainCam()->m_cameractrl.m_ffov));
}

void CGameApp::CreateCharImg() {
	static int id = 1;
	static float fTime = *ResMgr.GetDailTime();

	CCharacter* pMainCha = _pCurScene->GetMainCha();

	fTime += *ResMgr.GetDailTime();
	bool be = false;
	CCharacter* pCha = nullptr;
	CChaRecord* pInfo = GetChaRecordInfo(id);

	D3DXVECTOR3 veye(pMainCha->GetPos());
	D3DXVECTOR3 vlookat = veye;

	veye.y += 10;
	veye.z += 3.5f;

	vlookat.z += 1.5f;

	if (fTime > 1.0f) {
		pCha = _pCurScene->AddCharacter(id);
		if (pCha) {
			pCha->PlayPose(1, PLAY_PAUSE);
			pCha->setYaw(180);

			pCha->setPos(pMainCha->GetCurX(), pMainCha->GetCurY());

			//if(pInfo->chTerritory == 1)
			//	pCha->SetHieght(1.6f);
			pCha->FrameMove(0);

			veye = pCha->GetPos();
			vlookat = veye;

			veye.y += 12;
			veye.z += 3.5f;

			vlookat.z += 1.5f;
		} else {
			id++;
			if (id > 670) {
				btest = false;
				pMainCha->SetHide(FALSE);
			}
			fTime = 0;
			return;
		}

		be = true;
		fTime = 0;
	}
	pMainCha->SetHide(TRUE);
	g_Render.LookAt(veye, vlookat);
	g_Render.SetCurrentView(MPRender::VIEW_WORLD);

	_pCurScene->_Render();

	if (be) {
		char szPath[255];
		char fileName[255];
		_snprintf_s(szPath, _TRUNCATE, "screenshot/cha");
		Util_MakeDir(szPath);

		_snprintf_s(fileName, _TRUNCATE, "%s/%s.bmp", szPath, pInfo->szDataName);

		g_Render.CaptureScreen(fileName);

		if (pCha)
			pCha->SetValid(FALSE);
		id++;
	}
	if (id > 670) {
		btest = false;
		pMainCha->SetHide(FALSE);
	}
}

////////////////////////
bool CGameApp::_CreateSmMap(MPTerrain* pTerr) {
	static bool isnext = true;

	static float fTime = *ResMgr.GetDailTime();

	if (!_pCurScene->GetMainCha())
		return false;

	fTime += *ResMgr.GetDailTime();
	if (fTime > 0.5f) {
		if (isnext) {
			pTerr->SetShowSize(SHOWRSIZE + 5, SHOWRSIZE + 5);
			_pCurScene->GetMainCha()->setPos((int)xp * 100, (int)yp * 100);
			SetCameraPos(_pCurScene->GetMainCha()->GetPos());
			isnext = false;
			fTime = 0;
			return true;
		}
		bool bSea = true;
		fTime = 0;

		if (!isnext) {
			MPTile* TileList;
			int se;
			int e = (int)xp - SHOWRSIZE / 2;
			se = e;
			int w = (int)yp - SHOWRSIZE / 2;
			for (; w < int(yp + SHOWRSIZE / 2); w++) {
				for (; e < int(xp + SHOWRSIZE / 2); e++) {
					TileList = pTerr->GetTile(e, w);
					if (!TileList->IsDefault()) {
						bSea = false;
						goto skip;
					}
				}
				e = se;
			}
		skip:

			if (int(xp / SHOWRSIZE) >= (destxp / SHOWRSIZE)) {
				xp = xp1;
				yp += SHOWRSIZE;
				if (int(yp / SHOWRSIZE) >= destyp / SHOWRSIZE) {
					MessageBox(nullptr, RES_STRING(CL_LANGUAGE_MATCH_74), "INFO", 0);
					xp = xp1;
					yp = yp1;

					_pCurScene->GetMainCha()->setPos((int)xp * 100, (int)yp * 100);
					SetCameraPos(_pCurScene->GetMainCha()->GetPos());

					EnableSprintSmMap(false);
				}
				isnext = true;
				return true;
			} else {
				xp += SHOWRSIZE;
				isnext = true;
			}
			fTime = 0;
		}
		if (bSea)
			return true; //如果全部是海，不生成。

		D3DXMATRIX matView, matProj;
		D3DXVECTOR3 vEyePt = _pCurScene->GetMainCha()->GetPos();
		vEyePt.z = SHOWRSIZE;
		D3DXVECTOR3 vLookatPt = vEyePt;
		vLookatPt.z = 0;
		D3DXVECTOR3 vUpVec = D3DXVECTOR3(0, -1, 0);
		D3DXMatrixOrthoLH(&matProj, SHOWRSIZE,
						  SHOWRSIZE, 0.0f, 1000.0f);
		g_Render.SetTransformProj(&matProj);
		char fileName[64];

		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
		g_Render.SetTransformView(&matView);

		pTerr->SetShowCenter(vEyePt.x, vEyePt.y);
		pTerr->SetShowSize(SHOWRSIZE + 5, SHOWRSIZE + 5);
		_pCurScene->RenderSMallMap();

		char szPath[255];
		_snprintf_s(szPath, _TRUNCATE, "texture/minimap/%s", _pCurScene->GetTerrainName());
		Util_MakeDir(szPath);
		_snprintf_s(fileName, _TRUNCATE, "%s/sm_%d_%d.bmp", szPath,
					int(vEyePt.x / SHOWRSIZE), int(vEyePt.y / SHOWRSIZE));

		g_Render.CaptureScreen(fileName);

		LPDIRECT3DTEXTUREX pTex = nullptr;
		D3DXCreateTextureFromFileEx(g_Render.GetDevice(),
									fileName,													 //文件名
									256,														 //文件宽，这里设为自动
									256,														 //文件高，这里设为自动
									1,															 //需要多少级mipmap，这里设为1
									0,															 //此纹理的用途
									D3DFMT_UNKNOWN,												 //自动检测文件格式
									D3DPOOL_MANAGED,											 //由DXGraphics管理
									D3DX_FILTER_TRIANGLE | D3DX_FILTER_MIRROR | D3DX_FILTER_BOX, //纹理过滤方法
									D3DX_FILTER_NONE,											 //mipmap纹理过滤方法
									0x00000000,													 //透明色颜色
									nullptr,													 //读出的图像格式存储在何变量中
									nullptr,													 //读出的调色板存储在何变量中
									&pTex);														 //要创建的纹理
		D3DXSaveTextureToFile(fileName, D3DXIFF_BMP, pTex, nullptr);
		SAFE_RELEASE(pTex);
	}
	return true;
}

bool CGameApp::_PrintScreen() {
	if (IsEnableSpAvi()) {
		_CreateAviScreen();
	} else {
		static int g_nScreenCap = 0;
		char fileName[64];
		Util_MakeDir("screenshot\\");

		char pszName[64];

		int nidx = 0;
		while (true) {
			_snprintf_s(pszName, _TRUNCATE, "screenshot\\%d\\", nidx);
			if (_access(pszName, 0) == -1) {
				Util_MakeDir(pszName);
				break;
			}
		}
		_snprintf_s(fileName, _TRUNCATE, "%scap%05d.bmp", pszName, g_nScreenCap);
		g_Render.CaptureScreen(fileName);

		char szTip[64];
		_snprintf_s(szTip, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_75), fileName);
		Tip(szTip);
		g_nScreenCap++;
		EnableSprintScreen(false);
	}
	return true;
}

bool CGameApp::_CreateAviScreen() {
	/*static int g_nAviCnt = 0;
	char fileName[64];
    Util_MakeDir("screenshot/");

	_snprintf_s(fileName, _TRUNCATE,"screenshot/cap%05d.bmp",g_nAviCnt);
	g_Render.CaptureScreen(fileName);
	g_nAviCnt++;*/
	return true;
}

CGameScene* CGameApp::CreateScene(stSceneInitParam* param) {
	if (!param)
		return nullptr;

	if (param->nTypeID >= enumSceneEnd)
		return nullptr;

	CGameScene* scene = nullptr;
	switch (param->nTypeID) {
	case enumLoginScene:
		scene = new CLoginScene(*param);
		break;
	case enumSelectChaScene:
		scene = new CSelectChaScene(*param);
		break;
	case enumCreateChaScene:
		scene = new CCreateChaScene(*param);
		break;
	case enumWorldScene: {
		scene = new CWorldScene(*param);
		break;
	}
	}

	if (scene && scene->_CreateMemory())
		return scene;

	return nullptr;
}

bool CGameApp::CreateCurrentScene(char* szMapName) {
	stSceneInitParam stInit;
	stInit.nTypeID = enumLoginScene;
	stInit.strName = RES_STRING(CL_LANGUAGE_MATCH_76);
	stInit.strMapFile = szMapName;
	stInit.nMaxEff = 300;
	stInit.nMaxCha = 300;
	stInit.nMaxObj = 400;
	stInit.nMaxItem = 400;
	CGameScene* s = g_pGameApp->CreateScene(&stInit);
	if (!s) {
		return true;
	}
	_pCurScene = s;
	return true;
}

void CGameApp::_RenderTipText() {
	if (!_TipText.empty()) {
		//屏幕上的文字提示(渐消)
		int nTextY = 220;
		SIZE sz;
		for (std::list<STipText*>::iterator it = _TipText.begin(); it != _TipText.end(); it++) {
			STipText* pText = (*it);
			if (GetCurTick() - pText->dwBeginTime > 3000) {
				if (pText->btAlpha > 10)
					pText->btAlpha -= 10;
				if (pText->btAlpha < 10)
					pText->btAlpha = 0;
			}
			DWORD dwAlpha = pText->btAlpha << 24;
			DWORD dwColor = dwAlpha | 0x00f01000;

			g_CFont.GetTextSize(pText->szText, &sz);
			g_CFont.DrawText(pText->szText, (GetWindowWidth() - sz.cx) / 2, nTextY, dwColor);
			nTextY += 24;
		}

		if (!_TipText.empty()) {
			STipText* pText = _TipText.front();
			if (pText) {
				if (pText->btAlpha <= 0)
					_TipText.pop_front();
			}
		}
	}
}

void CGameApp::PlayMusic(int nMusicNo) {
	if (!CGameApp::IsMusicSystemValid())
		return;

	//SetBkMusicNotify(_hWnd, WM_MUSICEND);
	CMusicInfo* pInfo = nullptr;
	//
	//if(nMusicNo==0) // 顺序播放
	//{
	//    static g_dwPlayMusicNo = 0;
	//    g_dwPlayMusicNo++;
	//    CMusicInfo *pInfo = GetMusicInfo(g_dwPlayMusicNo);
	//    if( !pInfo || strlen(pInfo->szDataName)==0 )
	//    {
	//        g_dwPlayMusicNo = 1; // 回到第一首
	//    }
	//    nMusicNo = g_dwPlayMusicNo;
	//}
	if (nMusicNo >= 1) // 指定编号播放
	{
		pInfo = GetMusicInfo(nMusicNo);
		if (pInfo && strlen(pInfo->szDataName) == 0) {
			pInfo = nullptr;
		}
	}

	if (!pInfo) {
		_szBkgMusic[0] = '\0';
		_eSwitchMusic = BackgroundMusic::OldMusic;
		return;
	}

	switch (_eSwitchMusic) {
	case BackgroundMusic::NoMusic: {
		strncpy_s(_szBkgMusic, sizeof(_szBkgMusic), pInfo->szDataName, _TRUNCATE);

		_eSwitchMusic = BackgroundMusic::NewMusic;
		DWORD OldMusicID = g_dwCurMusicID;
		g_dwCurMusicID = AudioSDL::get_instance()->get_resID(_szBkgMusic, AUDIO_MUSIC);
		AudioSDL::get_instance()->volume(g_dwCurMusicID, _nCurMusicSize);
#ifndef USE_DSOUND
		g_AudioThread.play(g_dwCurMusicID, true);
#else
		if (g_dwCurMusicID && (OldMusicID != g_dwCurMusicID)) {
			AudioSDL::get_instance()->stop(OldMusicID);
			AudioSDL::get_instance()->play(g_dwCurMusicID, true);
		}
#endif
	} break;
	case BackgroundMusic::OldMusic: {
		strncpy_s(_szBkgMusic, sizeof(_szBkgMusic), pInfo->szDataName, _TRUNCATE);
	} break;
	case BackgroundMusic::NewMusic:
	case BackgroundMusic::MusicPlay:
		if (strncmp(_szBkgMusic, pInfo->szDataName, sizeof(_szBkgMusic)) != 0) {
			strncpy_s(_szBkgMusic, sizeof(_szBkgMusic), pInfo->szDataName, _TRUNCATE);
			_eSwitchMusic = BackgroundMusic::OldMusic;
		}
		break;
	}
}

void CGameApp::PlaySound(int nSoundNo) {
	if (!CGameApp::IsMusicSystemValid())
		return;

	if (nSoundNo == -1)
		return;

	CMusicInfo* pInfo = GetMusicInfo(nSoundNo);
	if (!pInfo || pInfo->nType != 1)
		return;

#ifdef USE_DSOUND
	PlaySample(pInfo->szDataName);
#else
	ulong musid = AudioSDL::get_instance()->get_resID(pInfo->szDataName, AUDIO_SAMPLE);
	AudioSDL::get_instance()->volume(musid, (int)CGameScene::_fSoundSize);
	g_AudioThread.play(musid, false);
#endif
}

void CGameApp::SetMusicSize(float fVol) {
	if (!CGameApp::IsMusicSystemValid())
		return;

	if (fVol >= 0.0 && fVol <= 1.0) {
		_nMusicSize = (int)(fVol * 128.0f);

		//::bkg_snd_vol( _nMusicSize );
		AudioSDL::get_instance()->volume(g_dwCurMusicID, _nMusicSize);
	}
}

static bool WaitModalForm(CGuiData* pSender, char& key) {
	if (key == VK_RETURN) {
		auto* frmWaiting = dynamic_cast<CForm*>(pSender);
		if (frmWaiting) {
			frmWaiting->Close();
			return true;
		}
	}
	return false;
}

void CGameApp::MsgBox(const char* pszFormat, ...) {
	va_list list;
	va_start(list, pszFormat);
	_vsnprintf_s(_szOutBuf, _TRUNCATE, pszFormat, list);
	va_end(list);

	g_stUIBox.ShowMsgBox(nullptr, _szOutBuf);
}

void CGameApp::ShowBigText(const char* pszFormat, ...) {
	va_list list;
	va_start(list, pszFormat);
	_vsnprintf_s(_szOutBuf, _TRUNCATE, pszFormat, list);
	va_end(list);

	g_stUIStart.ShowBigText(_szOutBuf);
}

void CGameApp::ShowMidText(const char* pszFormat, ...) {
	va_list list;
	va_start(list, pszFormat);
	_vsnprintf_s(_szOutBuf, _TRUNCATE, pszFormat, list);
	va_end(list);

	strncpy_s(_stMidFont.szText, sizeof(_stMidFont.szText), _szOutBuf, _TRUNCATE);
	_stMidFont.dwBeginTime = GetCurTick() + SHOW_TEXT_TIME;
	_stMidFont.btAlpha = 255;

	//g_stUICoze.OnSystemSay( _szOutBuf );
	CCozeForm::GetInstance()->OnSystemMsg(_szOutBuf);
}

void CGameApp::AddTipText(const char* pszFormat, ...) {
	va_list list;
	va_start(list, pszFormat);
	_vsnprintf_s(_szOutBuf, _TRUNCATE, pszFormat, list);
	va_end(list);

	auto* pText = new STipText;
	strncpy_s(pText->szText, TIP_TEXT_NUM, _szOutBuf, _TRUNCATE);
	pText->dwBeginTime = GetTickCount();
	pText->btAlpha = 255;
	_TipText.push_back(pText);
}

void CGameApp::ShowStaticNotify(const char* szStr, DWORD dwColor) {
	staticNotify->SetFixWidth(430);

	_dwNotifyTime = GetCurTick() + 120000;

	staticNotify->Clear();

	int textlength = 0;
	if (GetRender().GetScreenWidth() == 1024) {
		textlength = 99;
	} else if (GetRender().GetScreenWidth() == 800) {
		textlength = 63;
	} else {
		textlength = 140;
	}

	static bool titleFlag;
	titleFlag = false; //"Allow special titles"
	std::string text = szStr;
	while (!text.empty()) {
		int n = 1;
		std::string strSub = text.substr(0, 1);
		for (; n < textlength; n++) {
			if (n >= (int)text.length()) {
				break;
			}
			if (text[n] == ':' && titleFlag) {
				titleFlag = false;
				n++;
				break;
			}
			strSub = text.substr(0, n);
		}
		std::string str = CutFaceText(text, n);
		staticNotify->PushHint(str.c_str(), dwColor);
	}
	//_pNotify->ReadyForHint( 170, 5 );
	staticNotify->ReadyForHintGM(170, 5);
}

void CGameApp::ShowScrollingNotify(const char* szStr, int setnum, DWORD dwColor) {
	scrollingNotify->SetFixWidth(430);

	//#CTextScrollHint
	//_dwNotifyTime1 could end rendering long before intended
	_dwNotifyTime1 = GetCurTick() + 120000;

	//#CTextScrollHint
	//If the message is critical and can't be interrupted - then we need a queue
	scrollingNotify->Clear();

	std::string text = szStr;
	scrollingNotify->PushHint(text.c_str(), dwColor);
	scrollingNotify->ReadyForHint(170, 100, setnum);
}

void CGameApp::AutoTestInfo(const char* pszFormat, ...) {
	va_list list;
	va_start(list, pszFormat);
	_vsnprintf_s(_szOutBuf, _TRUNCATE, pszFormat, list);
	va_end(list);

	std::string info = _szOutBuf;
	ShowMidText(info.c_str());
	info += "\n";
	LG("autotest", info.c_str());

	FrameMove(GetTickCount());
	Render();
}

void CGameApp::SysInfo(const char* pszFormat, ...) {
	va_list list;
	va_start(list, pszFormat);
	_vsnprintf_s(_szOutBuf, _TRUNCATE, pszFormat, list);
	va_end(list);

	std::string info = _szOutBuf;
	//g_stUICoze.OnSystemSay( info.c_str() );
	CCozeForm::GetInstance()->OnSystemMsg(info.c_str());
}

void CGameApp::Waiting(bool isWaiting) {
	static CForm* frmWaiting = nullptr;
	if (!frmWaiting) {
		frmWaiting = CFormMgr::s_Mgr.Find("frmConnect", enumSwitchSceneForm);
		if (frmWaiting) {
			frmWaiting->SetIsEnabled(false);
		}
	}

	if (frmWaiting) {
		if (isWaiting) {
			CCursor::I()->SetCursor(CCursor::stWait);
			frmWaiting->ShowModal();
		} else {
			CCursor::I()->SetCursor(CCursor::stNormal);
			frmWaiting->Close();
		}
	}
}

void CGameApp::SetCameraPos(D3DXVECTOR3& pos, bool bRestoreCustom) {
	CCameraCtrl* pCam = GetMainCam();

	//GetCameraTrack()->Reset(pos.x,pos.y,pos.z);

	pCam->InitPos(pos.x, pos.y, pos.z, bRestoreCustom);

	pCam->Update();

	pCam->SetFollowObj(pCam->m_vCurPos);

	pCam->FrameMove(0);

	g_Render.SetWorldViewFOV(Angle2Radian(pCam->m_ffov));
	g_Render.LookAt(pCam->m_EyePos, pCam->m_RefPos);
	g_Render.SetCurrentView(MPRender::VIEW_WORLD);
}

// 用Loading
static CImage* flashProgress[32] = {nullptr};
static CForm* frmLoading = nullptr;
static bool bChangeFormLoginToSelCha = false;

void CGameApp::_ShowLoading() {
	_nSwitchScene--;

	const int LOAD_DELAY = 10;
	if (GetCurScene()->GetTerrain() && _nSwitchScene > LOAD_DELAY) {
		CCharacter* pMain = CGameScene::GetMainCha();
		if (pMain) {
			// LG("load", "camera pos = %.1f  %.1f  %.1f\n", _pMainCam->m_vCurPos.x, _pMainCam->m_vCurPos.y, _pMainCam->m_vCurPos.z);

			float fX = (float)pMain->GetCurX() / 100.0f;
			float fY = (float)pMain->GetCurY() / 100.0f;
			// LG("load", "main char pos = [%.2f %.2f]\n", fX, fY);
			float fHeight = GetCurScene()->GetGridHeight(fX, fY);
			// LG("load", "terrain height = %.1f\n", fHeight);
			bool isOK = false;
			if (fHeight > 0.001f) {
				isOK = true;
			} else {
				MPTerrain* pTer = GetCurScene()->GetTerrain();
				if (pTer && pTer->GetTileRegionAttr((int)fX, (int)fY) != 0) {
					isOK = true;
				}
			}
			if (isOK) {
				_nSwitchScene = LOAD_DELAY;
				pMain->setPos(pMain->GetCurX(), pMain->GetCurY());

				// LG("load", "loading disappear!\n");
				//GetMainCam()->InitModel(pMain->IsBoat() ? 3 : 0, &D3DXVECTOR3(fX,fY,fHeight));
				//CCameraCtrl *pCam = g_pGameApp->GetMainCam();
				//MPTerrain *pTerr = GetTerrain();

				//D3DXVECTOR3 vecCha = pCha->GetPos();
				//vecCha.z = GetGridHeight(vecCha.x,vecCha.y);

				//pCam->InitModel(pCha->IsBoat() ? 3 : 0,&vecCha);
				//pCam->SetBufVel( pCha->getMoveSpeed(), nChaID);
				//pCam->FrameMove(0);
				//pCam->SetViewTransform();
			}
		}
	}

	if (frmLoading) {
		for (int i = 0; i < 32; i++) {
			if (!flashProgress[i])
				continue;
			flashProgress[i]->SetIsShow(false);
			if (i == 32 - _nSwitchScene / 4) {
				flashProgress[i]->SetPos(0, (int)(GetRender().GetScreenHeight() * 0.9f));
				flashProgress[i]->Refresh();

				flashProgress[i]->SetIsShow(true);
			}
		}

		// LOADING进度条
		RefreshLoadingProgress();
		//float fPos = (g_pGameApp->GetCurTick() - _dwLoadingTick) / 10000.0f;
		//SetLoadingProgressPos(fPos <= 1.0f ? fPos : 1.0f);
	}
	if (_nSwitchScene <= 0) {
		_IsUserEnabled = true;
		SetInputActive(true);
		//snd_enable( true );

		GetCurScene()->LoadingCall();
		CUIInterface::All_LoadingCall();

		if (frmLoading) {
			frmLoading->Close();

			//  关闭loading界面
			GameLoading::Init()->Active();
			GameLoading::Init()->Close();

			//HWND hwnd = g_pGameApp->GetHWND();
			//SetForegroundWindow (hwnd);
		}
	}
}

void CGameApp::RefreshLoadingProgress() {

	// All proportional scales is calculated from
	// the image created by sub-images in /LOADING folder
	static constexpr auto ORIGINAL_IMAGE_WIDTH{1024};
	static constexpr auto ORIGINAL_IMAGE_HEIGHT{768};

	const auto PROGRESS_CYCLE = 1800; // 一个轮循（毫秒）

	// Distance from left side of window to left side of progress bar
	const auto progress_x = [&]() -> int {
		constexpr auto proportional_scale = 140.0 / ORIGINAL_IMAGE_WIDTH;
		return static_cast<int>(GetWindowWidth() * proportional_scale);
	}();

	// The location of the progress bar is proportional to the window size.
	// Distance from top of window to top of progress bar.
	const auto progress_y = [&]() -> int {
		constexpr auto proportional_scale = 675.0 / ORIGINAL_IMAGE_HEIGHT;
		return static_cast<int>(GetWindowHeight() * proportional_scale);
	}();

	const auto progress_bar_width = [&]() -> int {
		constexpr auto proportional_scale = 730.0 / ORIGINAL_IMAGE_WIDTH;
		return static_cast<int>(GetWindowWidth() * proportional_scale);
	}();

	CForm* frmLoading = CFormMgr::s_Mgr.Find("frmLoading", enumSwitchSceneForm);
	if (frmLoading) {
		auto* imgLoading = dynamic_cast<CImage*>(frmLoading->Find("imgLoading"));
		if (imgLoading) {
			const float fPrecent = ((GetCurTick() - _dwLoadingTick) % PROGRESS_CYCLE) / (float)(PROGRESS_CYCLE);

			const auto nX = static_cast<int>(((progress_bar_width - imgLoading->GetWidth()) * fPrecent) + progress_x);
			const auto nY = progress_y;

			// Adjust size of the progress cursor
			const auto [barWidth, barHeight] = [&]() -> std::pair<int, int> {
				constexpr auto proportional_scale_width = 132.0 / ORIGINAL_IMAGE_WIDTH;
				constexpr auto proportional_scale_height = 23.0 / ORIGINAL_IMAGE_HEIGHT;
				const auto width = static_cast<int>(GetWindowWidth() * proportional_scale_width);
				const auto height = static_cast<int>(GetWindowHeight() * proportional_scale_height);
				return {width, height};
			}();
			imgLoading->SetSize(barWidth, barHeight);
			imgLoading->SetPos(nX, nY);
			imgLoading->Refresh();
		}

		auto* labLoad = dynamic_cast<CLabelEx*>(frmLoading->Find("labLoad"));
		if (labLoad) {
			const int x = (GetWindowWidth() / 2) - labLoad->GetWidth();
			const int y = [&]() -> int {
				constexpr auto proportional_scale = 679.0 / ORIGINAL_IMAGE_HEIGHT;
				return static_cast<int>(GetWindowHeight() * proportional_scale);
			}();
			labLoad->SetPos(x, y);
			labLoad->Refresh();
		}
	}
}

void CGameApp::ResetCaption() {
	/*
	extern short g_sClientVer;
	char szSpace[250] = "";
	int nSpace = 40;
	int nWidth = GetRender().GetScreenWidth();
	if(nWidth == 1024)
	{
		nSpace = 140;
	}
   
	for (int i = 0; i < nSpace; i++)
		szSpace[i] = ' ';
	szSpace[nSpace] = '\0';
	*/

	char szCaption[350];
	_snprintf_s(szCaption, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_77) /*, (float)(g_sClientVer % 1000) / 100, szSpace */); // , curr_ver.c_str());
	SetCaption(szCaption);
}

void CGameApp::Loading(int nFrame) {
	_IsUserEnabled = false;

	//snd_enable( false );
	_nSwitchScene = nFrame;
	SetInputActive(false);

	if (!frmLoading) {
		frmLoading = CFormMgr::s_Mgr.Find("frmLoading", enumSwitchSceneForm);
	}
	if (!frmLoading) {
		return;
	}
	frmLoading->SetSize(GetRender().GetScreenWidth(), GetRender().GetScreenHeight());
	frmLoading->Refresh();
	frmLoading->ShowModal();

	const auto fullscreen = g_Config.m_bFullScreen;
	const int width = (fullscreen ? GetSystemMetrics(SM_CXSCREEN) : GetWindowWidth()) / 4;
	const int height = (fullscreen ? GetSystemMetrics(SM_CYSCREEN) : GetWindowHeight()) / 3;

	const std::string sImgFile = "imgLoading";
	for (int i = 1; i <= 12; ++i) {
		auto imgLoading = dynamic_cast<CImage*>(frmLoading->Find((sImgFile + std::to_string(i)).c_str()));
		if (imgLoading) {
			imgLoading->SetPos(((i - 1) % 4) * width, ((i - 1) / 4) * height);
			imgLoading->SetSize(width, height);
			imgLoading->Refresh();
		}
	}
}

/*
void CGameApp::InitAllTable() {
	BOOL bBinary = FALSE; // 此变量已改为全局g_bBinaryTable控制

	CSceneObjSet* pObjSet = new CSceneObjSet(0, g_Config.m_nMaxSceneObjType);
	pObjSet->LoadRawDataInfo("scripts/table/sceneobjinfo", bBinary);

	CEffectSet* pEffSet = new CEffectSet(0, g_Config.m_nMaxEffectType);
	pEffSet->LoadRawDataInfo("scripts/table/sceneffectinfo", bBinary);

	CShadeSet* pShadeSet = new CShadeSet(0, g_Config.m_nMaxEffectType);
	pShadeSet->LoadRawDataInfo("scripts/table/shadeinfo", bBinary);

	CEventSoundSet* pEventSoundSet = new CEventSoundSet(0, 30);
	pEventSoundSet->LoadRawDataInfo("scripts/table/eventsound", bBinary);

	CMusicSet* pMusicSet = new CMusicSet(0, 500);
	pMusicSet->LoadRawDataInfo("scripts/table/musicinfo", bBinary);

	CPoseSet* pPoseSet = new CPoseSet(0, 100);
	pPoseSet->LoadRawDataInfo("scripts/table/characterposeinfo", bBinary);

	CChaCreateSet* pChaCreateSet = new CChaCreateSet(0, 60);
	pChaCreateSet->LoadRawDataInfo("scripts/table/selectcha", bBinary);

	CSkillRecordSet* pSkillSet = new CSkillRecordSet(0, defMAX_SKILL_NO);
	pSkillSet->LoadRawDataInfo("scripts/table/skillinfo", bBinary);

	CMapSet* pMapSet = new CMapSet(0, 100);
	pMapSet->LoadRawDataInfo("scripts/table/mapinfo", bBinary);

	CChaRecordSet* pChaSetAttrib = new CChaRecordSet(0, 2500);
	pChaSetAttrib->LoadRawDataInfo("scripts/table/Characterinfo", bBinary);

	CItemRecordSet* pItemSet = new CItemRecordSet(0, g_Config.m_nMaxItemType);
	pItemSet->LoadRawDataInfo("scripts/table/iteminfo", bBinary);

	CEventRecordSet* pEvent = new CEventRecordSet(0, 10);
	pEvent->LoadRawDataInfo("scripts/table/objevent", bBinary);

	CAreaSet* pArea = new CAreaSet(0, 300);
	pArea->LoadRawDataInfo("scripts/table/AreaSet", bBinary);

	CServerSet* pServer = new CServerSet(0, 100);

	if (g_bBinaryTable) {
		////////////////////////////////////////
		//
		//  By Jampe
		//  合作商ServerSet处理
		//  2006/5/19
		//
		switch (g_cooperate.code) {
		case COP_OURGAME: {
			pServer->LoadRawDataInfo("scripts/table/ServerSet2", bBinary);
		} break;
		case COP_SINA: {
			pServer->LoadRawDataInfo("scripts/table/ServerSet3", bBinary);
		} break;
		case COP_CGA: {
			pServer->LoadRawDataInfo("scripts/table/ServerSet4", bBinary);
		} break;
		case 0:
		default:
			//CLanguageRecord t;
			//t.MadeBinFile("./scripts/table/serverset.bin", "./scripts/table/serverset.txt");

			pServer->LoadRawDataInfo("scripts/table/ServerSet", bBinary);
		}
		//  end
	} else {
		// add by Philip.Wu  2006-06-12  修改 makebin 不能完整生成全部的 serverset.txt （策划需求）
		pServer->LoadRawDataInfo("scripts/table/ServerSet", bBinary);

		// Delete by lark.li 20080305
		// add by Philip.Wu  2006-07-31  多语言统一后的字符串资源生成 BIN 文件
		//g_oLangRec.MadeBinFile("./scripts/table/StringSet.bin", "./scripts/table/StringSet.txt");
	}

	CNotifySet* pNotify = new CNotifySet(0, 100);
	pNotify->LoadRawDataInfo("scripts/table/notifyset", bBinary);

	// Modify by lark.li 20080822 begin
	//CSkillStateRecordSet* pState = new CSkillStateRecordSet( 0, 240 );
	CSkillStateRecordSet* pState = new CSkillStateRecordSet(0, 300);
	// End

	pState->LoadRawDataInfo("scripts/table/skilleff", bBinary);

	CChatIconSet* pIcon = new CChatIconSet(0, 100);
	pIcon->LoadRawDataInfo("scripts/table/chaticons", bBinary);

	CItemTypeSet* pItemType = new CItemTypeSet(0, 100);
	pItemType->LoadRawDataInfo("scripts/table/itemtype", bBinary);

	CItemPreSet* pItemPre = new CItemPreSet(0, 100);
	pItemPre->LoadRawDataInfo("scripts/table/itempre", bBinary);

	CForgeRecordSet* pRecordSet = new CForgeRecordSet(1, ROLE_MAXNUM_FORGE);
	pRecordSet->LoadRawDataInfo("scripts/table/forgeitem", bBinary);

	xShipSet* pShipSet = new xShipSet(0, 120);
	pShipSet->LoadRawDataInfo("scripts/table/shipinfo", bBinary);

	xShipPartSet* pShipItem = new xShipPartSet(0, 500);
	pShipItem->LoadRawDataInfo("scripts/table/shipiteminfo", bBinary);

	CHairRecordSet* pHair = new CHairRecordSet(0, 500);
	pHair->LoadRawDataInfo("scripts/table/hairs", bBinary);

	// txt
	MPTerrainSet* pTerrainSet = new MPTerrainSet(0, 100);
	pTerrainSet->LoadRawDataInfo("scripts/table/TerrainInfo", bBinary);

	CEff_ParamSet* pEffSetp = new CEff_ParamSet(0, 100);
	pEffSetp->LoadRawDataInfo("scripts/table/MagicSingleinfo", bBinary);

	CGroup_ParamSet* pGroupSet = new CGroup_ParamSet(0, 10);
	pGroupSet->LoadRawDataInfo("scripts/table/MagicGroupInfo", bBinary);

	CItemRefineSet* pItemRefineSet = new CItemRefineSet(0, g_Config.m_nMaxItemType);
	pItemRefineSet->LoadRawDataInfo("scripts/table/ItemRefineInfo", bBinary);

	CItemRefineEffectSet* pItemRefineEffectSet = new CItemRefineEffectSet(0, 5000);
	pItemRefineEffectSet->LoadRawDataInfo("scripts/table/ItemRefineEffectInfo", bBinary);

	CStoneSet* pStoneSet = new CStoneSet(0, 100);
	pStoneSet->LoadRawDataInfo("scripts/table/StoneInfo", bBinary);

	CHelpInfoSet* pHelpInfoSet = new CHelpInfoSet(0, 20);
	pHelpInfoSet->LoadRawDataInfo("scripts/table/helpinfoset", bBinary);

	//add by alfred.shi 20080710

	NPCHelper* pNpcHelpSet = new NPCHelper(0, 1000);
	pNpcHelpSet->LoadRawDataInfo("scripts/table/MonsterList", bBinary);
	SAFE_DELETE(pNpcHelpSet);

	pNpcHelpSet = new NPCHelper(0, 1000);
	pNpcHelpSet->LoadRawDataInfo("scripts/table/NPCList", bBinary);

	MPResourceSet* pResourceSet = new MPResourceSet(0, g_Config.m_nMaxResourceNum);
	pResourceSet->LoadRawDataInfo("scripts/table/ResourceInfo", bBinary);

	CElfSkillSet* pElfSkillSet = new CElfSkillSet(0, 100);
	pElfSkillSet->LoadRawDataInfo("scripts/table/ElfSkillInfo", bBinary);
	//Add by sunny.sun 20080903
	CMonsterSet* pMonsterSet = new CMonsterSet(0, 1000);
	pMonsterSet->LoadRawDataInfo("scripts/table/MonsterInfo", bBinary);
}

void CGameApp::ReleaseAllTable() {
	CMusicSet* pMusicSet = CMusicSet::I();
	SAFE_DELETE(pMusicSet);

	CPoseSet* pPoseSet = CPoseSet::I();
	SAFE_DELETE(pPoseSet);

	CChaCreateSet* pChaCreateSet = CChaCreateSet::I();
	SAFE_DELETE(pChaCreateSet);

	CSceneObjSet* pSceneObjSet = CSceneObjSet::I();
	SAFE_DELETE(pSceneObjSet);

	CEffectSet* pEffectSet = CEffectSet::I();
	SAFE_DELETE(pEffectSet);

	CShadeSet* pShadeSet = CShadeSet::I();
	SAFE_DELETE(pShadeSet);

	CSkillRecordSet* pSkillSet = CSkillRecordSet::I();
	SAFE_DELETE(pSkillSet);

	CMapSet* pMapSet = CMapSet::I();
	SAFE_DELETE(pMapSet);

	CEventSoundSet* pEventSoundSet = CEventSoundSet::I();
	SAFE_DELETE(pEventSoundSet);

	CEventRecordSet* pEvent = CEventRecordSet::I();
	SAFE_DELETE(pEvent);

	CServerSet* pServe = CServerSet::I();
	SAFE_DELETE(pServe);

	CNotifySet* pNotify = CNotifySet::I();
	SAFE_DELETE(pNotify);

	CSkillStateRecordSet* pState = CSkillStateRecordSet::I();
	SAFE_DELETE(pState);

	CChatIconSet* pIcon = CChatIconSet::I();
	SAFE_DELETE(pIcon);

	CItemTypeSet* pItemType = CItemTypeSet::I();
	SAFE_DELETE(pItemType);

	CItemPreSet* pItemPreSet = CItemPreSet::I();
	SAFE_DELETE(pItemPreSet);

	CForgeRecordSet* pRecordSet = CForgeRecordSet::I();
	SAFE_DELETE(pRecordSet);

	xShipSet* pShipSet = xShipSet::I();
	SAFE_DELETE(pShipSet);

	xShipPartSet* pShipItem = xShipPartSet::I();
	SAFE_DELETE(pShipItem);

	CHairRecordSet* pHair = CHairRecordSet::I();
	SAFE_DELETE(pHair);

	CItemRefineSet* pRefine = CItemRefineSet::I();
	SAFE_DELETE(pRefine);

	CStoneSet* pStone = CStoneSet::I();
	SAFE_DELETE(pStone);

	CHelpInfoSet* pHelpInfoSet = CHelpInfoSet::I();
	SAFE_DELETE(pHelpInfoSet);

	//add by aflred.shi 20080711
	NPCHelper* pNpcHelpSet = NPCHelper::I();
	SAFE_DELETE(pNpcHelpSet);

	CElfSkillSet* pElfSkillSet = CElfSkillSet::I();
	SAFE_DELETE(pElfSkillSet);

	//MPResourceSet *pResourceSet = MPResourceSet::GetInstancePtr();
	//SAFE_DELETE(pResourceSet);
}
*/

bool CGameApp::LoadRes4() {
	int test = 0;
	char szBone[128];
	CCharacter* pChar = this->GetCurScene()->_GetFirstInvalidCha();
	for (int i = 1; i < 1200; i++) {
		CChaRecord* pInfo = GetChaRecordInfo(i);
		if (pInfo && (pInfo->chCtrlType == 1 || pInfo->chCtrlType == 2)) {
			_snprintf_s(szBone, _TRUNCATE, "%04d.lab", pInfo->sModel);
			pChar->InitBone(szBone);
			test++;
		}
	}
	return true;
}

void LoadResModelBuf(MPIResourceMgr* res_mgr) {
	MPIResBufMgr* buf_mgr = res_mgr->GetResBufMgr();
	MPIPathInfo* path_info = res_mgr->GetSysGraphics()->GetSystem()->GetPathInfo();
	std::string model_path = path_info->GetPath(PATH_TYPE_MODEL_SCENE);

	char path[LW_MAX_PATH];
	LW_HANDLE handle;

	CSceneObjInfo* obj_info;
	const DWORD model_num = 500;
	for (DWORD i = 0; i < model_num; i++) {
		if ((obj_info = GetSceneObjInfo(i)) == nullptr)
			continue;

		if (stricmp(obj_info->szDataName, "sl-bd026-05.lmo") == 0) {
			int x = 0;
		}
		handle = obj_info->nID;
		model_path.append(obj_info->szDataName);

		if (LW_FAILED(buf_mgr->RegisterModelObjInfo(handle, path))) {
			LG("init", "msgcannot find model file: %s", path);
			continue;
		}
	}
}

void CGameApp::SetFPSInterval(DWORD v) {
	if (v > 0)
		_pSteady->SetFPS(v);
}

void CGameApp::_SceneError(const char* info, CGameScene* p) {
	if (p) {
		MsgBox(RES_STRING(CL_LANGUAGE_MATCH_78), p->GetInitParam()->strName.c_str(), info);
	} else {
		MsgBox(RES_STRING(CL_LANGUAGE_MATCH_79), info);
	}
}

bool CGameApp::HasLogFile(const char* log_file, bool isOpen) {
	// 检查是否有gui
	std::string file;
	GetLGDir(file);
	file += "/";
	file += log_file;
	file += ".log";
	FILE* fp;
	fopen_s(&fp, file.c_str(), "r");
	if (fp) {
		fclose(fp);

		if (isOpen) {
			char buf[256] = {0};
			_snprintf_s(buf, _TRUNCATE, "notepad %s", file.c_str());
			::WinExec(buf, SW_SHOWNORMAL);
		}
		return true;
	}
	return false;
}

void LimitCurrentProc() {
	HANDLE hCurrentProcess = GetCurrentProcess();

	// Get the processor affinity mask for this process
	DWORD_PTR dwProcessAffinityMask = 0;
	DWORD_PTR dwSystemAffinityMask = 0;

	if (GetProcessAffinityMask(hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask) != 0 && dwProcessAffinityMask) {
		// Find the lowest processor that our process is allows to run against
		DWORD_PTR dwAffinityMask = (dwProcessAffinityMask & ((~dwProcessAffinityMask) + 1));

		// Set this as the processor that our thread must always run against
		// This must be a subset of the process affinity mask
		HANDLE hCurrentThread = GetCurrentThread();
		if (INVALID_HANDLE_VALUE != hCurrentThread) {
			SetThreadAffinityMask(hCurrentThread, dwAffinityMask);
			CloseHandle(hCurrentThread);
		}
	}

	CloseHandle(hCurrentProcess);
}

extern const char* ConsoleCallback(const char*);
void CGameApp::AutoTest() {
	char szBuf[256] = {0};
	_snprintf_s(szBuf, _TRUNCATE, "testeffect 0 5000 1");
	ConsoleCallback(szBuf);
	_snprintf_s(szBuf, _TRUNCATE, "testskilleffect 0 5000 1");
	ConsoleCallback(szBuf);

	CGameScene* pScene = GetCurScene();
	if (!pScene)
		return;

	D3DXVECTOR3 TestPos;
	int nTestX = 0;
	int nTestY = 0;
	if (CGameScene::GetMainCha()) {
		TestPos = CGameScene::GetMainCha()->GetPos();
		TestPos.x -= 3.0f;

		nTestX = CGameScene::GetMainCha()->GetCurX() - 300;
		nTestY = CGameScene::GetMainCha()->GetCurY();
	}

	for (int i(0); i < 1; i++) {
		// 正在测试所有特效
		{
			AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_80));
			int nCount = CEffectSet::I()->GetLastID() + 1;
			CEffectObj* pEffect = nullptr;
			CMagicInfo* pInfo = nullptr;
			for (int i = 0; i < nCount; i++) {
				pInfo = GetMagicInfo(i);
				if (!pInfo)
					continue;

				pEffect = pScene->GetFirstInvalidEffObj();
				if (!pEffect)
					continue;

				if (!pEffect->Create(i)) {
					AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_81), i);
					continue;
				}

				pEffect->Emission(-1, &TestPos, nullptr);
				pEffect->SetValid(TRUE);

				AutoTestUpdate();

				pEffect->SetValid(FALSE);
			}
		}
	}
	// 自动创建所有角色,然后删除,用于检查角色,角色动作等合法性
	{
		AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_82));

		int nCount = CChaRecordSet::I()->GetLastID() + 1;
		CCharacter* pCha = nullptr;
		for (int i = 0; i < nCount; i++) {
			CChaRecord* pInfo = GetChaRecordInfo(i);
			if (!pInfo)
				continue;

			pCha = pScene->AddCharacter(i);
			if (!pCha) {
				AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_83), i);
				continue;
			}

			pCha->setPos(nTestX, nTestY);
			AutoTestUpdate();
			pCha->SetValid(FALSE);
		}
	}

	// 自动测试头发表begin
	{
		AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_84));

		CCharacter* pHairCha[4] = {nullptr};
		int nMax = 4;
		for (int i = 0; i < nMax; i++) {
			pHairCha[i] = pScene->AddCharacter(i + 1);
			pHairCha[i]->setPos(nTestX, nTestY + i * 100 - 200);
		}
		int nCount = CHairRecordSet::I()->GetLastID() + 1;
		CHairRecord* pHair = nullptr;
		for (int i = 0; i < nCount; i++) {
			pHair = GetHairRecordInfo(i);
			if (!pHair)
				continue;

			for (int j = 0; j < nMax; j++) {
				if (pHair->IsChaUse[j]) {
					if (!pHairCha[j]->ChangePart(enumEQUIP_HEAD, pHair->dwItemID))
						SysInfo(RES_STRING(CL_LANGUAGE_MATCH_85), i, j + 1, pHair->dwItemID);

					for (int k = 0; k < pHair->GetFailItemNum(); k++) {
						if (!pHairCha[j]->ChangePart(enumEQUIP_HEAD, pHair->dwFailItemID[k]))
							SysInfo(RES_STRING(CL_LANGUAGE_MATCH_86), i, j + 1, pHair->dwFailItemID[k]);
					}
					AutoTestUpdate();
				}
			}
		}
		for (int i = 0; i < nMax; i++) {
			pHairCha[i]->SetValid(FALSE);
		}
		// end
	}

	// 正在测试所有特效
	{
		AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_80));
		int nCount = CEffectSet::I()->GetLastID() + 1;
		CEffectObj* pEffect = nullptr;
		CMagicInfo* pInfo = nullptr;
		for (int i = 0; i < nCount; i++) {
			pInfo = GetMagicInfo(i);
			if (!pInfo)
				continue;

			pEffect = pScene->GetFirstInvalidEffObj();
			if (!pEffect)
				continue;

			if (!pEffect->Create(i)) {
				AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_81), i);
				continue;
			}

			pEffect->Emission(-1, &TestPos, nullptr);
			pEffect->SetValid(TRUE);

			AutoTestUpdate();

			pEffect->SetValid(FALSE);
		}
	}

	// 检查所有道具
	{
		int x = 0;
		int y = 0;
		std::string file;
		if (CGameScene::GetMainCha()) {
			x = CGameScene::GetMainCha()->GetCurX() - 300;
			y = CGameScene::GetMainCha()->GetCurY();
		}

		AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_87));
		CItemRecord* pInfo = nullptr;
		CSceneItem* pItem = nullptr;
		CMagicInfo* pEffectInfo = nullptr;
		int nEffectID = 0;
		int nCount = CItemRecordSet::I()->GetLastID() + 1;
		for (int i = 0; i < nCount; i++) {
			pInfo = GetItemRecordInfo(i);
			if (!pInfo)
				continue;

			for (int j = 0; j < 5; j++) {
				if (strcmp(pInfo->chModule[j], "0") != 0) {
					file = "model/item/";
					file += pInfo->chModule[j];
					file += ".lgo";
					if (!IsExistFile(file.c_str())) {
						file = "model/character/";
						file += pInfo->chModule[j];
						file += ".lgo";
						if (!IsExistFile(file.c_str())) {
							if (j == 0) {
								AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_88), i, pInfo->szName, pInfo->chModule[0]);
							} else {
								AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_89), i, pInfo->szName, j, pInfo->chModule[j]);
							}
						}
					}
				}
			}

			if (strcmp(pInfo->szICON, "0") != 0) {
				if (!IsExistFile(pInfo->GetIconFile()))
					AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_90), i, pInfo->szName, pInfo->szICON);
			}

			nEffectID = pInfo->sDrap;
			if (nEffectID > 0) {
				pEffectInfo = GetMagicInfo(nEffectID);
				if (!pEffectInfo) {
					AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_91), i, pItem->GetItemInfo()->szName, nEffectID);
				}
			}
			for (int j = 0; j < pInfo->sEffNum; j++) {
				nEffectID = pInfo->sEffect[j][0];
				if (nEffectID > 0) {
					pEffectInfo = GetMagicInfo(nEffectID);
					if (!pEffectInfo) {
						AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_92), i, pItem->GetItemInfo()->szName, nEffectID);
					}
				}
			}
			nEffectID = pInfo->sItemEffect[0];
			if (nEffectID > 0) {
				pEffectInfo = GetMagicInfo(nEffectID);
				if (!pEffectInfo) {
					AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_93), i, pItem->GetItemInfo()->szName, nEffectID);
				}
			}
			nEffectID = pInfo->sAreaEffect[0];
			if (nEffectID > 0) {
				pEffectInfo = GetMagicInfo(nEffectID);
				if (!pEffectInfo) {
					AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_94), i, pItem->GetItemInfo()->szName, nEffectID);
				}
			}

			pItem = pScene->AddSceneItem(i, 0);
			if (!pItem) {
				AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_95), i, pInfo->szName);
				continue;
			}

			pItem->setPos(nTestX, nTestY);
			AutoTestUpdate();

			pItem->SetValid(FALSE);
		}
	}

	// 正在测试精炼索引表...
	{
		AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_96));
		CItemRefineInfo* pRefine = nullptr;
		int nCount = CItemRefineSet::I()->GetLastID() + 1;
		int nEffectID = 0;
		CItemRefineEffectInfo* pEffectInfo = nullptr;
		for (int i = 0; i < nCount; i++) {
			pRefine = GetItemRefineInfo(i);
			if (!pRefine)
				continue;

			for (int k = 0; k < ITEM_REFINE_NUM; k++) {
				nEffectID = pRefine->Value[k];
				if (nEffectID > 0) {
					pEffectInfo = GetItemRefineEffectInfo(nEffectID);
					if (!pEffectInfo) {
						AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_97), i, pRefine->szDataName, nEffectID);
					}
				}
			}
		}
	}

	// 正在测试精炼效果表...
	{
		AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_98));
		int nCount = CItemRefineEffectSet::I()->GetLastID() + 1;
		CItemRefineEffectInfo* pInfo = nullptr;
		CMagicInfo* pEffectInfo = nullptr;
		int nEffectNum;
		int nEffectID;
		for (int i = 0; i < nCount; i++) {
			pInfo = GetItemRefineEffectInfo(i);
			if (!pInfo)
				continue;

			// 4个角色
			for (int k = 0; k < 4; k++) {
				nEffectNum = pInfo->GetEffectNum(k);
				for (int j = 0; j < nEffectNum; j++) {
					if (pInfo->sEffectID[k][j] <= 0)
						continue;

					for (int level = 0; level < 4; level++) {
						nEffectID = pInfo->sEffectID[k][j] * 10 + level;
						pEffectInfo = GetMagicInfo(nEffectID);
						if (!pEffectInfo) {
							AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_99), i, pInfo->szDataName, nEffectID);
						}
					}
				}
			}
		}
	}

	// 检测是否有道具表错误
	{
		AutoTestInfo(RES_STRING(CL_LANGUAGE_MATCH_100));

		g_pGameApp->HasLogFile("iteminfoerror");
	}

	g_pGameApp->HasLogFile("autotest");
}

void CGameApp::LG_Config(const LGInfo& info) {
	LGInfo in = info;
	if (g_Config.m_bEditor) {
		in.bMsgBox = true;
		in.bEnableAll = true;
	}

	MPGameApp::LG_Config(in);

	if (in.bCloseAll) {
		::LG_CloseAll();
	}
	::LG_SetEraseMode(in.bEraseMode);
	::LG_SetDir(in.dir);
	::LG_EnableAll(in.bEnableAll);
	::LG_EnableMsgBox(in.bMsgBox);
}
