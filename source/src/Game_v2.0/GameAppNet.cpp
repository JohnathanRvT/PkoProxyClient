#include "Stdafx.h"
#include "GameApp.h"
#include "GameAppMsg.h"
#include "packetcmd.h"
#include "MPEditor.h"
#include "LoginScene.h"
#include "uiboxform.h"
#include "uisystemform.h"

static void _Disconnect(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	auto pLoginScene = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	pLoginScene ? pLoginScene->ShowServerSelection()
				: g_pGameApp->LoadScriptScene(enumLoginScene);
}

static void _SwitchMapFailed(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey) {
	auto pLogin = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	if (!pLogin) {
		g_pGameApp->LoadScriptScene(enumLoginScene);
		pLogin = dynamic_cast<CLoginScene*>(CGameApp::GetCurScene());
	}

	if (pLogin) {
		pLogin->ShowServerSelection();
	}
}

void CGameApp::_HandleMsg(DWORD dwTypeID, DWORD dwParam1, DWORD dwParam2) {
	switch (dwTypeID) {
	case APP_NET_LOGINRET:
		break;

	case APP_NET_DISCONNECT: {
		Waiting(false);

		if (g_TomServer.bEnable) {
			MessageBox(g_pGameApp->GetHWND(), RES_STRING(CL_LANGUAGE_MATCH_134), "error", 0);
			g_pGameApp->SetIsRun(false);
			return;
		}

		CGameScene* scene = CGameApp::GetCurScene();
		if (!scene)
			return;

		if (dwParam1 == 1000) {
			char szBuf[128] = {0};
			_snprintf_s(szBuf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_135), WSAGetLastError());
			_Disconnect(nullptr, NULL, NULL, NULL, MouseClickState());
			g_stUIBox.ShowMsgBox(nullptr, szBuf);
			return;
		}
		// else if(dwParam1!=DS_DISCONN && dwParam1!=DS_SHUTDOWN )
		{
			if (!dynamic_cast<CLoginScene*>(scene)) {
				if (!g_ChaExitOnTime.TimeArrived()) {
					//非登陆场景断开连接
					char szBuf[256] = {0};
					if (g_NetIF) {
						// modify by Philip.Wu  2006-06-09  非收费系统不提示用户充值
						if (g_Config.m_IsBill) {
							_snprintf_s(szBuf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_136), RES_STRING(CL_LANGUAGE_MATCH_137), dwParam1);
						} else {
							_snprintf_s(szBuf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_138));
						}
					} else {
						_snprintf_s(szBuf, _TRUNCATE, RES_STRING(CL_LANGUAGE_MATCH_139), dwParam1);
					}
					_Disconnect(nullptr, NULL, NULL, NULL, MouseClickState());
					g_stUIBox.ShowMsgBox(nullptr, szBuf);
				}
			} else { //登陆场景内断开连接
				//判断密码是否错误
				auto* pkLogin = dynamic_cast<CLoginScene*>(scene);
				if (pkLogin && pkLogin->IsPasswordError()) {
					pkLogin->ShowLoginForm();
					return;
				}
			}
		}
	} break;
	case APP_SWITCH_MAP_FAILED: {
		g_pGameApp->Waiting(false);

		char szBuf[256] = {0};
		_snprintf_s(szBuf, _TRUNCATE, "%s[%d]", g_GetServerError(dwParam1), dwParam1);
		g_stUIBox.ShowMsgBox(_SwitchMapFailed, szBuf);
	} break;
	case APP_SWITCH_MAP: {
		extern MPEditor g_Editor;
		g_Editor.Init(dwParam1);
		CMapInfo* pInfo = GetMapInfo(dwParam1);
		CCameraCtrl* pCam = g_pGameApp->GetMainCam();
		auto v = D3DXVECTOR3((float)pInfo->nInitX, (float)pInfo->nInitY, 0);
		pCam->SetFollowObj(v);
		// EnableCameraFollow(FALSE);
		break;
	} break;
	}
}
