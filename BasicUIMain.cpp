// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include "KaminoGlobal.h"
#include "KaminoHistoryModule.h"
#include "KaminoGlowWindow.h"
#include "UIMain.h"
#include "KTab.h"
#include "resource.h"
#include "base\files\file_path.h"
#include "base\path_service.h"
#include "base\bind.h"
#include "content\public\browser\browser_thread.h"
#include "windowsx.h"

#include <tchar.h>

BasicUIMain *g_main_ui = NULL;
BOOL CALLBACK MiniMizeAllOpenWindows(HWND hWnd, LPARAM lParam);

UIMain* GetUIMain(){
	if(g_main_ui == NULL)
		g_main_ui = new BasicUIMain();
	return g_main_ui;
}

void DestroyUIMain(){
	if(g_main_ui != NULL)
		delete g_main_ui;
}

BasicUIMain::BasicUIMain(){
#ifdef UI_DEBUG_LOG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif
	wnd_cls.cbClsExtra = 0;
	wnd_cls.cbSize = sizeof(WNDCLASSEX);
	wnd_cls.cbWndExtra = sizeof(LONG_PTR);
	wnd_cls.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(130,130,130));
	wnd_cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd_cls.hInstance = GetModuleHandle(NULL);
	wnd_cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);//LoadIcon(wnd_cls.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wnd_cls.hIconSm = wnd_cls.hIcon;
	wnd_cls.lpfnWndProc = BasicWndProc;
	wnd_cls.lpszClassName = L"KaminoBrowser";
	wnd_cls.lpszMenuName = NULL;
	wnd_cls.style = CS_SAVEBITS;
	if(!RegisterClassEx(&wnd_cls)){
		return;
	}
	//tab_manager = NULL;
	tab_manager = new KTabManager(this);
}

BasicUIMain::~BasicUIMain(){
#ifdef UI_DEBUG_LOG	
	FreeConsole();
#endif
}

void BasicUIMain::main(){
	CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wnd_cls.lpszClassName, L"KaminoBrowser", WS_POPUP, 0,0,0,0, NULL, NULL, wnd_cls.hInstance, this);
	MoveWindow(main_window, KaminoGlobal::g_ideal_rect.left, 
		KaminoGlobal::g_ideal_rect.top, KaminoGlobal::g_ideal_rect.right, KaminoGlobal::g_ideal_rect.bottom, TRUE);
	if(main_window == NULL)
		PostQuitMessage(0);
	content::BrowserThread::PostTask(content::BrowserThread::DB, FROM_HERE, base::Bind(KaminoHistoryModule::PreInitialize));
	ShowWindow(main_window, SW_SHOW);
	UpdateWindow(main_window);
}



//static
LRESULT CALLBACK BasicUIMain::BasicWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static LONG prev_rect_left = 0;
	static LONG prev_rect_right = 0;
	static LONG prev_rect_bottom = 0;
	static LONG prev_rect_top = 0;
	static KaminoGlowWindow *glow_window = NULL;
	BasicUIMain* myself =  NULL;
	if(msg == WM_CREATE){
		CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
		myself = (BasicUIMain*)pcs->lpCreateParams;
		myself->main_window = hWnd;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(myself));
		glow_window = new KaminoGlowWindow();
		glow_window->Init(hWnd);
		return 0;
	}
	myself = reinterpret_cast<BasicUIMain*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
	if(myself == NULL){
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	switch(msg){
		case WM_GETMINMAXINFO:{
				MINMAXINFO *ptr = (MINMAXINFO *)lParam;
				ptr->ptMaxSize.x = KaminoGlobal::g_screen_width - 1;
				ptr->ptMaxSize.y = KaminoGlobal::g_screen_height - 1;
				ptr->ptMaxPosition.x = 0;
				ptr->ptMaxPosition.y = 0;
				ptr->ptMaxTrackSize.x = GetSystemMetrics(SM_CXMAXTRACK);
				ptr->ptMaxTrackSize.y = GetSystemMetrics(SM_CYMAXTRACK);
				ptr->ptMinTrackSize.x = GetSystemMetrics(SM_CXMINTRACK);
				ptr->ptMinTrackSize.y = GetSystemMetrics(SM_CYMINTRACK);
			}
			return 0;
		case WM_NCCALCSIZE:
			if(wParam == TRUE)
				return 0;
			return DefWindowProc(hWnd, msg, wParam, lParam);
		case WM_NCHITTEST:{
			if(KaminoGlobal::g_is_window_fullscreen)
				return HTCLIENT;
			RECT rc;
			GetWindowRect(hWnd, &rc);
			bool is_on_left = false;
			bool is_on_right = false;
			bool is_on_bottom = false;
			bool is_on_top = false;
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			is_on_left = ((x >= (rc.left-3)) && (x <= (rc.left + 3)));
			if(!is_on_left)
				is_on_right = ((x >= (rc.right - 3)) && (x <= (rc.right + 3)));
			is_on_bottom = ((y >= (rc.bottom - 3)) && (y <= (rc.bottom + 3)));
			if(!is_on_bottom)
				is_on_top = ((y >= (rc.top - 3)) && (y <= rc.top + 3));	
			if(is_on_left)
				if(is_on_bottom)
					return HTBOTTOMLEFT;
				else
					if (is_on_top)
						return HTTOPLEFT;
					else
						return HTLEFT;
			else
				if(is_on_right)
					if(is_on_bottom)
						return HTBOTTOMRIGHT;
					else
						if(is_on_top)
							return HTTOPRIGHT;
						else
							return HTRIGHT;
			if(is_on_bottom)
				return HTBOTTOM;
			else if(is_on_top)
				return HTTOP;

			if((y >= (rc.top + 3)) && (y <= (rc.top + 35)))
				return HTCAPTION;
		}
		case WM_PAINT:{
			if(!(myself->tab_manager->is_initialized)){
				//First set Up the win32 tray Icon
				HICON icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON3), IMAGE_ICON, GetSystemMetrics(SM_CXICON),
										GetSystemMetrics(SM_CXICON), 0);
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon); //Set Up the win32 tray Icon 
				if(myself->tab_manager->Initialize(hWnd)){
					if(myself->tab_manager->tab_window){
						glow_window->SetMouseEventForwarding(myself->tab_manager->tab_window);
					}
					if(KaminoGlobal::g_start_url != NULL)
						myself->tab_manager->AddTab(KaminoGlobal::g_start_url);
					else
						myself->tab_manager->AddTab("https:\\www.google.com");					
				}
				else
					DestroyWindow(hWnd);
				return 0;
			}
			RECT rc;
			GetClientRect(hWnd, &rc);
			myself->tab_manager->RecivedHwndMessage(msg, wParam, lParam);
			ValidateRect(hWnd, &rc); 
			return 0;
		}
		case WM_WINDOWPOSCHANGED:
			if(glow_window != NULL){
				glow_window->Update(((WINDOWPOS *)lParam)->x,
					((WINDOWPOS *)lParam)->y, ((WINDOWPOS *)lParam)->cx, ((WINDOWPOS *)lParam)->cy);
				if(myself != NULL && myself->tab_manager != NULL)
					myself->tab_manager->Focus();
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		case WM_SIZE:{
			if(wParam == SIZE_MAXIMIZED){
				KaminoGlobal::g_is_window_fullscreen = true;
				SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW);
			}
			else{
				KaminoGlobal::g_is_window_fullscreen = false;
				SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW);
			}
			RECT rc;
			GetClientRect(hWnd, &rc);
			if(((rc.top == prev_rect_top) && (rc.left == prev_rect_left) && 
				(rc.bottom == prev_rect_bottom) && (rc.right == prev_rect_right)) || (wParam == SIZE_MINIMIZED)){
				if(wParam != SIZE_MINIMIZED)
					myself->tab_manager->Render();
				return 0;
			}
			prev_rect_bottom = rc.bottom;
			prev_rect_top = rc.top;
			prev_rect_right = rc.right;
			prev_rect_left = rc.left;
			if(myself->tab_manager->is_initialized){
				myself->tab_manager->RecivedHwndMessage(msg, wParam, lParam);
			}
			return 0;
		}
		case WM_SETFOCUS:{
			if(KaminoGlobal::g_is_window_wide && !KaminoGlobal::g_is_window_fullscreen){
				EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()), MiniMizeAllOpenWindows, (LPARAM)myself);
			}
			if(KaminoGlobal::g_is_window_wide && (!KaminoGlobal::g_is_window_fullscreen)){
				RECT rc;
				GetWindowRect(hWnd, &rc);
				if(rc.right != KaminoGlobal::g_ideal_rect.right){
					MoveWindow(hWnd, KaminoGlobal::g_ideal_rect.left, KaminoGlobal::g_ideal_rect.top,
						KaminoGlobal::g_ideal_rect.right, KaminoGlobal::g_ideal_rect.bottom, FALSE);
				}
			}
			if(myself->tab_manager)
				if(myself->tab_manager->is_initialized){
					myself->tab_manager->Focus();
				}
			return 0;
		}
		case WM_KILLFOCUS:
			PostMessage(myself->tab_manager->tab_window, msg, wParam, lParam);
			return 0;
		case WM_LBUTTONUP:{
			return 0;
		}
		case WM_MOUSEMOVE:{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			return 0;
		}
		case WM_CLOSE:{
			if(myself->tab_manager){
				delete myself->tab_manager;
				myself->tab_manager = NULL;
			}
			DestroyWindow(hWnd);
			return 0;
		}
		case WM_DESTROY:{
			KaminoHistoryModule::Flush();
#ifdef UI_DEBUG_LOG
			MessageBox(NULL, L"WatchLog", L"+LOG", MB_OK);
#endif
			PostQuitMessage(0);
			return 0;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

BOOL CALLBACK MiniMizeAllOpenWindows(HWND hWnd, LPARAM lParam){
	static HWND desktop_window = NULL; 
	static HWND shell_window = NULL;
	static wchar_t *win_name = NULL;
	if(win_name == NULL)
		win_name = (wchar_t *)malloc(255 * sizeof(255));
	BasicUIMain *myself = (BasicUIMain *)lParam;
	if(desktop_window == NULL) desktop_window = GetShellWindow();
	if(shell_window == NULL) shell_window = FindWindow(L"Shell_traywnd", NULL);
	if((hWnd == desktop_window) || (hWnd == shell_window) || (hWnd == myself->main_window))
		return TRUE;
	if(GetWindowTextLength(hWnd) <= 0){
		return TRUE;
	}
	if(IsWindowVisible(hWnd)){
		RECT rc;
		GetWindowRect(hWnd, &rc);	
		int px = rc.left + ((rc.right - rc.left) / 2);
		int py = rc.top + ((rc.bottom - rc.top) / 2);
		if((rc.left <= KaminoGlobal::g_ideal_rect.left) || (rc.right >= KaminoGlobal::g_ideal_rect.right) 
			|| (rc.bottom >= KaminoGlobal::g_ideal_rect.bottom) || (rc.top <= KaminoGlobal::g_ideal_rect.top))
		if((px >= KaminoGlobal::g_ideal_rect.top) && (px <= KaminoGlobal::g_ideal_rect.right) 
			&& (py >= KaminoGlobal::g_ideal_rect.top) && (py <= KaminoGlobal::g_ideal_rect.bottom)){
				LONG style = GetWindowLong(hWnd, GWL_STYLE);
				if((style & WS_CAPTION) && (style & WS_MINIMIZEBOX) && (style & WS_MAXIMIZEBOX)){
					ShowWindow(hWnd, SW_SHOWMINIMIZED);
				}
				return TRUE;
		}
	}
	return TRUE;
}
