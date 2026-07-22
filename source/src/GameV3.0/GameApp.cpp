#include "GameApp.h"
#include "MPRender.h"


int GameApp::Run() {
	MSG msg{};
	while (true) {
		if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		FrameMove(1);
		Render();
		//g_Render.GetInterfaceMgr()->tp_loadres->SetPoolEvent(FALSE);
	}
		
	return (int)msg.wParam;
}






void GameApp::_PreMouseRun(MouseClickState dwMouseKey) {
}

void GameApp::_FrameMove(DWORD dwTimeParam, bool camMove) {
}

void GameApp::_Render() {
}

bool GameApp::_Init() {
	return false;
}

void GameApp::_End() {
}

void GameApp::MouseButtonDown(int nButton) {
}

void GameApp::MouseButtonUp(int nButton) {
}

void GameApp::MouseButtonDB(int nButton) {
}

void GameApp::MouseMove(int nOffsetX, int nOffsetY) {
}

void GameApp::MouseScroll(int nOffset) {
}

void GameApp::HandleKeyDown(DWORD dwKey) {
}

void GameApp::HandleKeyUp() {
}

void GameApp::MouseContinue(int nButton) {
}
