#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include "GameApp.h";

std::unique_ptr<GameApp> g_pGameApp{nullptr};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//switch (message)
	//{
	//case WM_ACTIVATE:
	//	// Add by lark.li 20080903 begin
	//	if(GameLoading::Init()->Active())
	//		return 0;
	//	// End
	//
	//	break;
	//case WM_NCACTIVATE:
	//case WM_ACTIVATEAPP:
	//	// Add by lark.li 20080903 begin
	//	if(GameLoading::Init()->Active())
	//		return 0;
	//	// End

	//	if( g_pGameApp && g_pGameApp->IsInit() )
	//		g_pGameApp->SetInputActive( LOWORD(wParam)!=WA_INACTIVE );
	//	break;
	//}

	//if (g_pGameApp) {
	//	g_pGameApp->HandleWindowMsg(message, (DWORD)wParam, (DWORD)lParam);
	//if (GetRegionMgr()->OnMessage(message, wParam, lParam))
	//return 0;
	//}

	switch (message) {
		//case WM_USER_TIMER:
		//	//__timer_period_render();
		//	return 0;
		//	break;

	case WM_ACTIVATE:
		// Add by lark.li 20080903 begin
		//if(GameLoading::Init()->Active())
		//	return 0;
		// End

		break;
	case WM_NCACTIVATE:
	case WM_ACTIVATEAPP:
		// Add by lark.li 20080903 begin
		//if(GameLoading::Init()->Active())
		//	return 0;
		// End

		//if (g_pGameApp && g_pGameApp->IsInit())
		g_pGameApp->SetInputActive(LOWORD(wParam) != WA_INACTIVE);
		break;
	case WM_CREATE: {
		//CenterWindow(hWnd);

		//g_InputEdit = CreateWindow(TEXT("EDIT"),
		//						   TEXT(RES_STRING(CL_LANGUAGE_MATCH_192)),
		//						   WS_CHILD | WS_VISIBLE,
		//						   0, 0,
		//						   0, 0,
		//						   hWnd,
		//						   (HMENU)IDC_EDIT1,
		//						   ((LPCREATESTRUCT)lParam)->hInstance,
		//						   nullptr);
		//
		//g_wpOrigEditProc = (WNDPROC)SetWindowLongPtr(g_InputEdit, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
		//
		//extern CInputBox g_InputBox;
		//g_InputBox.SetEditWindow(g_InputEdit);
		//
		//CImeInput::s_Ime.OnCreate(hWnd, hInst);
	} break;
	case WM_KEYDOWN:
	case WM_CHAR:
		break;
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP: {
		break;
	}
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		int wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		//switch (wmId) {
		//case IDM_EXIT:
		//	DestroyWindow(hWnd);
		//	break;
		//default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		//}
	} break;
	case WM_PAINT:
		break;
	case WM_CLOSE:
		//{
		//	if( g_Config.m_bEditor )
		//		break;

		//	if( !g_NetIF->IsConnected() )
		//		break;

		//	if( !dynamic_cast<CWorldScene*>( CGameApp::GetCurScene() ) )
		//		break;

		//	CForm* f = CFormMgr::s_Mgr.Find( "frmAskExit" );
		//	if( f )
		//	{
		//		long lState = GetWindowLong(hWnd, GWL_STYLE);
		//		if( lState & WS_MINIMIZE )
		//		{
		//			ShowWindow(hWnd, SW_SHOWNORMAL);
		//		}

		//		f->SetIsShow(true);
		//		g_stUISystem.GetSystemForm()->SetIsShow(false);
		//		return 0;
		//	}
		//}
		break;
	case WM_QUIT:
		break;
	case WM_DESTROY: {
		//CS_Logout();
		//CS_Disconnect(DS_DISCONN);
		//
		//SetWindowLongPtr(g_InputEdit, GWLP_WNDPROC, (LONG_PTR)g_wpOrigEditProc);
		//
		PostQuitMessage(0);
		if (g_pGameApp)
			//	g_pGameApp->SetIsRun(false);
			break;
	}
	//case WM_MUSICEND: // 锟斤拷锟街诧拷锟斤拷系统
	//{
	//	if (g_pGameApp)
	//	g_pGameApp->PlayMusic(0);
	//	break;
	//}
	case WM_SYSKEYUP:
		if (wParam == VK_MENU || wParam == VK_F10) {
			return 0;
		}
		//case WM_IME_STARTCOMPOSITION:
		//case WM_IME_ENDCOMPOSITION:
		//case WM_IME_NOTIFY:
		//case WM_IME_COMPOSITION:
		//case WM_IME_SETCONTEXT:
		//case WM_INPUTLANGCHANGEREQUEST:
		//case WM_INPUTLANGCHANGE:
		//    {
		//        return 0;
		//    }
		//    break;

	case WM_ERASEBKGND:
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int DoRegisterClass(HINSTANCE hInstance) {

	HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
	// Register the window class
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)brush;
	wcex.lpszClassName = "KOP";

	if (!RegisterClassEx(&wcex)) {
		DWORD error = GetLastError();
		printf("Error registering window class: %d\n", error);
		return 0;
	}
	return 1;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR lpCmdLine,
					   int nCmdShow) {
	if (!DoRegisterClass(hInstance)) {
		return 0;
	}

	g_pGameApp = std::make_unique<GameApp>();
	if (!g_pGameApp->Init(hInstance, "KOP", 600, 800, 32)) {
		g_pGameApp->End();
		return 0;
	}

	g_pGameApp->Run();

	g_pGameApp->End();
}
