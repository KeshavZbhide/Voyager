// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoWaitWindow.h"
#include "KaminoGlobal.h"
#include "UIMain.h"
#include <math.h>

#define M_PI 3.14159265359

#define WAIT_WINDOW_INITIALIZE WM_USER + 1
#define WAIT_WINDOW_HIDE WM_USER + 2
#define WAIT_WINDOW_SHOW WM_USER + 3
#define WAIT_WINDOW_EXIT WM_USER + 4

DWORD WINAPI KaminoWaitWindow::animation_thread_main(LPVOID ptr){
	KaminoWaitWindow *myself = (KaminoWaitWindow *)ptr;
	MSG msg;
	if(myself == NULL)
		return 0;
	myself->wait_window = NULL;	
	myself->wnd_cls.cbClsExtra = 0;
	myself->wnd_cls.cbSize = sizeof(WNDCLASSEX);
	myself->wnd_cls.cbWndExtra = 0;
	myself->wnd_cls.hInstance  = GetModuleHandle(NULL);
	myself->wnd_cls.hbrBackground = CreateSolidBrush(RGB(45, 45, 45));
	myself->wnd_cls.hCursor = LoadCursor(myself->wnd_cls.hInstance, IDC_ARROW);
	myself->wnd_cls.hIcon = LoadIcon(myself->wnd_cls.hInstance, IDI_APPLICATION);
	myself->wnd_cls.hIconSm = myself->wnd_cls.hIcon;
	myself->wnd_cls.lpfnWndProc = KaminoWaitWndProc;
	myself->wnd_cls.lpszClassName = L"KaminoWaitWindow";
	myself->wnd_cls.lpszMenuName = NULL;
	myself->wnd_cls.style = CS_VREDRAW | CS_HREDRAW;
	RegisterClassEx(&(myself->wnd_cls));

	myself->wait_window = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED, L"KaminoWaitWindow", 
		L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, myself->wnd_cls.hInstance, (LPVOID)myself);
	if(myself->wait_window == NULL)
		return 0;
	SetWindowRgn(myself->wait_window, CreateRoundRectRgn (0, 0, 120, 100, 5, 5), FALSE);
	SetLayeredWindowAttributes(myself->wait_window, 0, (255 * 91) / 100, LWA_ALPHA);
	SetWindowText(myself->wait_window, L"");
	myself->is_shown = false;
	
	while(GetMessage(&msg, NULL, 0, 0)){
		switch(msg.message){
			case WAIT_WINDOW_INITIALIZE:
				UI_LOG(0, "WAIT_WINDOW_INITIALIZE RECIVED")
				myself->wait_window = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED, L"KaminoWaitWindow", 
					L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, myself->wnd_cls.hInstance, (LPVOID)myself);
				if(myself->wait_window == NULL)
					return 0;
				SetWindowRgn(myself->wait_window, CreateRoundRectRgn (0, 0, 120, 100, 5, 5), FALSE);
				SetLayeredWindowAttributes(myself->wait_window, 0, (255 * 91) / 100, LWA_ALPHA);
				SetWindowText(myself->wait_window, L"");
				myself->is_shown = false;
				break;
			case WAIT_WINDOW_EXIT:
				UI_LOG(0, "WAIT_WINDOW_EXIT RECIVED")
				return 0;
			case WAIT_WINDOW_HIDE:
				UI_LOG(0, "WAIT_WINDOW_HIDE RECIVED")
				if(myself->is_shown){
					ShowWindow(myself->wait_window, SW_HIDE);
					myself->is_shown = false;
				}
				break;
			case WAIT_WINDOW_SHOW:
				UI_LOG(0, "WAIT_WINDOW_SHOW RECIVED")
				if(!myself->is_shown){
					ShowWindow(myself->wait_window, SW_SHOWNOACTIVATE);
					int StartX = 0;
					int StartY = 0;
					if(KaminoGlobal::g_is_window_fullscreen){
						StartX = KaminoGlobal::g_screen_width - (KaminoGlobal::g_screen_width * 0.0047) - 120;
						StartY = KaminoGlobal::g_screen_height - (KaminoGlobal::g_screen_height * 0.0042) - 100;
					}
					else{
						StartX = KaminoGlobal::g_ideal_rect.right - ((KaminoGlobal::g_ideal_rect.right - KaminoGlobal::g_ideal_rect.left) * 0.0042) - 120;
						StartY = KaminoGlobal::g_ideal_rect.bottom - ((KaminoGlobal::g_ideal_rect.bottom - KaminoGlobal::g_ideal_rect.top) * 0.0042) - 100;
					}
					MoveWindow(myself->wait_window, StartX, StartY, 120, 100, TRUE);
					myself->is_shown = true;
					myself->timer.Start();
				}
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				break;
		}		
	}
	return 0;
}

KaminoWaitWindow::KaminoWaitWindow(){
	animation_thread_id = 0;
	animation_thread_handle = NULL;
	animation_thread_handle = CreateThread(NULL, 0, KaminoWaitWindow::animation_thread_main, this, NULL, &animation_thread_id);
	is_shown = false;
}

KaminoWaitWindow::~KaminoWaitWindow(){
	if(wait_window != NULL)
		DestroyWindow(wait_window);
	if(animation_thread_handle != NULL && animation_thread_id != NULL)
		PostThreadMessage(animation_thread_id, WAIT_WINDOW_EXIT, 0, 0);
}

HRESULT KaminoWaitWindow::Initialize(){
	UI_LOG(0, "CALLED KaminoWaitWindow::Initialize()")
	//PostThreadMessage(animation_thread_id, WAIT_WINDOW_INITIALIZE, 0, 0);
	return S_OK;
}

HRESULT KaminoWaitWindow::InitDeviceDependantResource(){
	HRESULT hr = S_OK;
	if(render_target.Get() == NULL){
		RECT rc;
		GetClientRect(wait_window, &rc);
		hr = render_target.Reset(wait_window, rc);
	}
	if((brush.Get() == NULL) && SUCCEEDED(hr))
		hr = brush.Reset(1.0f, 1.0f, 1.0f, 1.0f, render_target.Get());		
	return hr;
}

void KaminoWaitWindow::DestroyDeviceDependantResource(){
	render_target.Clean();
	brush.Clean();
}

LRESULT CALLBACK KaminoWaitWindow::KaminoWaitWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	KaminoWaitWindow *myself = NULL;
	if(msg == WM_CREATE){
		CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
		myself = (KaminoWaitWindow *)pcs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(myself));
		return 0;
	}
	myself = reinterpret_cast<KaminoWaitWindow *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
	if(myself == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);
	switch(msg){
		case WM_PAINT:
			myself->Render();
			return 0;
		case WM_SIZE:{
				RECT rc;
				GetClientRect(hWnd, &rc);
				if(myself->render_target.Get())
					((ID2D1HwndRenderTarget *)myself->render_target.Get())->Resize(
						D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
			}
			return 0;
		case WM_MOUSEMOVE:
			myself->Hide();
			return 0;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);			
	}
};

void KaminoWaitWindow::Show(){
	if(animation_thread_id != NULL && animation_thread_handle != NULL)
		PostThreadMessage(animation_thread_id, WAIT_WINDOW_SHOW, 0, 0);
}

void KaminoWaitWindow::Hide(){
	if((animation_thread_id != NULL) && (animation_thread_handle != NULL))
		PostThreadMessage(animation_thread_id, WAIT_WINDOW_HIDE, 0, 0);
}


void KaminoWaitWindow::Render(){
	int bubble = 0;
	if(SUCCEEDED(InitDeviceDependantResource())){
		if(is_shown){
			double time_ellapsed = timer.getTimeElapsed();
			FLOAT radius = static_cast<FLOAT>(20.0 * sin((time_ellapsed ) * (M_PI/2.0)));
			FLOAT radius2 = static_cast<FLOAT>(11.0 * cos((time_ellapsed ) * (M_PI/2.0)));
			render_target.Get()->BeginDraw();
			render_target.Get()->Clear();
			render_target.Get()->Clear(D2D1::ColorF(0.176f, 0.176f, 0.176f, 1.0f));
			bubble = static_cast<int>(time_ellapsed / 0.33);
			switch(bubble){
				case 0:{			
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(20.0f, 50.0f), radius, radius), brush.Get());
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(60.0f, 50.0f), radius2, radius2), brush.Get());
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(100.0f, 50.0f), radius2, radius2), brush.Get());
				}
				break;
				case 1:{
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(20.0f, 50.0f), radius2, radius2), brush.Get());
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(60.0f, 50.0f), radius, radius), brush.Get());
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(100.0f, 50.0f), radius2, radius2), brush.Get());
				}
				break;
				case 2:{
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(20.0f, 50.0f), radius2, radius2), brush.Get());
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(60.0f, 50.0f), radius2, radius2), brush.Get());
					render_target.Get()->FillEllipse(D2D1::Ellipse(D2D1::Point2F(100.0f, 50.0f), radius, radius), brush.Get());
				}
				break;
			}
			if(time_ellapsed >= 1.00)
				timer.Start();
			if(render_target.Get()->EndDraw() == D2DERR_RECREATE_TARGET)
				DestroyDeviceDependantResource();
		}
		else
			ValidateRect(wait_window, NULL);
	}	
}

