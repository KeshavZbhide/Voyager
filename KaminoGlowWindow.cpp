// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoGlowWindow.h"
#include "KaminoDirectResource.h"
#include "KaminoGlobal.h"
#include "UIMain.h"
#include "windowsx.h"


KaminoGlowWindow::KaminoGlowWindow(){
	main = NULL;
	
	win_right = NULL;
	win_left = NULL;
	win_top = NULL;
	win_bottom = NULL;

	win_right_bitmap = NULL;
	win_left_bitmap = NULL;
	win_top_bitmap = NULL;
	win_bottom_bitmap = NULL;
	mouse_win = NULL;

	wnd_cls.cbClsExtra = 0;
	wnd_cls.cbSize = sizeof(WNDCLASSEX);
	wnd_cls.cbWndExtra = 0;
	wnd_cls.hbrBackground = CreateSolidBrush(RGB(150, 150, 150));
	wnd_cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd_cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd_cls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wnd_cls.hInstance = GetModuleHandle(NULL);
	wnd_cls.lpfnWndProc = GlowWindowProc;
	wnd_cls.lpszClassName = L"KaminoGlowWindow";
	wnd_cls.lpszMenuName = NULL;
	wnd_cls.style = (CS_HREDRAW | CS_VREDRAW); 
	RegisterClassEx(&wnd_cls);
}

void KaminoGlowWindow::Init(HWND win){
	main = win;
	win_right = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, wnd_cls.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, wnd_cls.hInstance, this); 
	win_left = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, wnd_cls.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, wnd_cls.hInstance, this); 
	win_top = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, wnd_cls.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, wnd_cls.hInstance, this); 
	win_bottom = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, wnd_cls.lpszClassName, L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, wnd_cls.hInstance, this); 	
	
	SetWindowText(win_right, L"");
	ShowWindow(win_right, SW_SHOW);
	UpdateWindow(win_right);

	SetWindowText(win_left, L"");	
	ShowWindow(win_left, SW_SHOW);
	UpdateWindow(win_left);

	SetWindowText(win_top, L"");
	ShowWindow(win_top, SW_SHOW);	
	UpdateWindow(win_top) ;
	
	SetWindowText(win_bottom, L"");
	ShowWindow(win_bottom, SW_SHOW);
	UpdateWindow(win_bottom);
	BuildAllBitmaps();
}

void KaminoGlowWindow::SetMouseEventForwarding(HWND win){
	if(win == NULL){
		UI_LOG(win == NULL,0,"Recived Win That is NULL")
	}
	mouse_win = win;
}

KaminoGlowWindow::~KaminoGlowWindow(){
	if(win_right){
		DestroyWindow(win_right);
		if(win_right_bitmap)
			DeleteObject(win_right_bitmap);
	}
	if(win_left){
		DestroyWindow(win_left);
		if(win_left_bitmap)
			DeleteObject(win_left_bitmap);
	}
	if(win_top){
		DestroyWindow(win_top);
		if(win_top_bitmap)
			DeleteObject(win_top_bitmap);
	}
	if(win_bottom){
		DestroyWindow(win_bottom);
		if(win_bottom_bitmap)
			DeleteObject(win_bottom_bitmap);
	}
}

void KaminoGlowWindow::Update(int x, int y, int cx, int cy){
	SetWindowPos(win_right, NULL, x+cx, y-2, 5, cy+4, SWP_NOACTIVATE);	
	SetWindowPos(win_left, NULL, x - 5, y-2, 5, cy+4, SWP_NOACTIVATE);
	SetWindowPos(win_top, NULL, x, y-5, cx, 5, SWP_NOACTIVATE); 
	SetWindowPos(win_bottom, NULL, x, y+cy, cx, 5, SWP_NOACTIVATE);
}

void KaminoGlowWindow::BuildAllBitmaps(){
	void *bits_holder;
	KaminoDirect2DFactory d2d_factory;	
	ID2D1DCRenderTarget *target = NULL;
	ID2D1LinearGradientBrush *brush = NULL;
	ID2D1SolidColorBrush *brush2 = NULL;
	RECT rc;
	SIZE sz;
	POINT p1;
	BITMAPINFO bitmap_info = { };
	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
	BLENDFUNCTION blend = { };
	blend.AlphaFormat =  AC_SRC_ALPHA;
	blend.BlendFlags = 0;
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	D2D1_GRADIENT_STOP stops[3];
	stops[0].color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.0f);
	stops[0].position = 0.0f;
	stops[1].color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.1f);
	stops[1].position = 0.5f;
	stops[2].color = D2D1::ColorF(1.145f, 1.145f, 1.145f, 0.7f);
	stops[2].position = 1.0f;
	p1.x = 0;
	p1.y = 0;	

	const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
	const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, format);
	ID2D1GradientStopCollection *gradient_stops;
	d2d_factory.Get()->CreateDCRenderTarget(&properties, &target);
	target->CreateGradientStopCollection(stops, 3, &gradient_stops);

	//Building win_left Bitmap ....
    bitmap_info.bmiHeader.biWidth = 5;
	bitmap_info.bmiHeader.biHeight = 0 - KaminoGlobal::g_screen_height;
	win_left_bitmap = CreateDIBSection(0, &bitmap_info, DIB_RGB_COLORS, &bits_holder, NULL, NULL);
	sz.cx = 5;
	sz.cy = KaminoGlobal::g_screen_height;
	rc.top = 0;
	rc.bottom = KaminoGlobal::g_screen_height;
	rc.left = 0;
	rc.right = 5;
	HDC bitmap_dc = CreateCompatibleDC(GetDC(NULL));
	HBITMAP old = (HBITMAP)SelectObject(bitmap_dc, win_left_bitmap);
	target->BindDC(bitmap_dc, &rc);
	target->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(0.0f, 0.0f), D2D1::Point2F(5.0f, 0.0f)), gradient_stops, &brush);
	target->BeginDraw();
	target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
	target->FillRectangle(D2D1::RectF(0.0f, 0.0f, 5.0f, static_cast<FLOAT>(KaminoGlobal::g_screen_height)), brush);
	target->EndDraw();		
	UpdateLayeredWindow(win_left, NULL, &p1, &sz, bitmap_dc, &p1, 0, &blend, ULW_ALPHA); 		
	SelectObject(bitmap_dc, old);
	
	//Build win_right bitmap ....
    bitmap_info.bmiHeader.biWidth = 5;
	bitmap_info.bmiHeader.biHeight = 0 - KaminoGlobal::g_screen_height;
	win_right_bitmap = CreateDIBSection(0, &bitmap_info, DIB_RGB_COLORS, &bits_holder, NULL, NULL);
	sz.cx = 5;
	sz.cy = KaminoGlobal::g_screen_height;
	rc.top = 0;
	rc.bottom = KaminoGlobal::g_screen_height;
	rc.left = 0;
	rc.right = 5;
	old = (HBITMAP)SelectObject(bitmap_dc, win_right_bitmap);
	target->BindDC(bitmap_dc, &rc);
	brush->SetStartPoint(D2D1::Point2F(5.0f, 0.0f));
	brush->SetEndPoint(D2D1::Point2F(0.0f, 0.0f));
	//target->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(10.0f, 0.0f), D2D1::Point2F(0.0f, 0.0f)), gradient_stops, &brush);		
	target->BeginDraw();
	target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
	target->FillRectangle(D2D1::RectF(0.0f, 0.0f, 5.0f, static_cast<FLOAT>(KaminoGlobal::g_screen_height)), brush);
	target->EndDraw();		
	UpdateLayeredWindow(win_right, NULL, &p1, &sz, bitmap_dc, &p1, 0, &blend, ULW_ALPHA); 		
	SelectObject(bitmap_dc, old);
	
	//Build win_top bitmap ....
	bitmap_info.bmiHeader.biWidth = KaminoGlobal::g_screen_width;
	bitmap_info.bmiHeader.biHeight = 0 - 5;
	win_top_bitmap = CreateDIBSection(0,&bitmap_info, DIB_RGB_COLORS, &bits_holder, NULL, NULL);
	sz.cx = KaminoGlobal::g_screen_width;
	sz.cy = 5;
	rc.top = 0;
	rc.right = KaminoGlobal::g_screen_width;
	rc.left = 0;
	rc.bottom = 5;
	old = (HBITMAP)SelectObject(bitmap_dc, win_top_bitmap);
	target->BindDC(bitmap_dc, &rc);
	brush->SetStartPoint(D2D1::Point2F(0.0f, 0.0f));
	brush->SetEndPoint(D2D1::Point2F(0.0f, 5.0f));
	//target->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(0.0f, 0.0f), D2D1::Point2F(0.0f, 10.0f)), gradient_stops, &brush);		
	target->BeginDraw();
	target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
	target->FillRectangle(D2D1::RectF(0.0f, 0.0f, static_cast<FLOAT>(KaminoGlobal::g_screen_width), 5.0f), brush);
	target->EndDraw();		
	UpdateLayeredWindow(win_top, NULL, &p1, &sz, bitmap_dc, &p1, 0, &blend, ULW_ALPHA); 		
	SelectObject(bitmap_dc, old);
	//brush->Release();

	//Build win_bottom bitmap ....
	bitmap_info.bmiHeader.biWidth = KaminoGlobal::g_screen_width;
	bitmap_info.bmiHeader.biHeight = 0 - 5;
	win_bottom_bitmap = CreateDIBSection(0,&bitmap_info, DIB_RGB_COLORS, &bits_holder, NULL, NULL);
	sz.cx = KaminoGlobal::g_screen_width;
	sz.cy = 5;
	rc.top = 0;
	rc.right = KaminoGlobal::g_screen_width;
	rc.left = 0;
	rc.bottom = 5;
	old = (HBITMAP)SelectObject(bitmap_dc, win_bottom_bitmap);
	target->BindDC(bitmap_dc, &rc);
	brush->SetStartPoint(D2D1::Point2F(0.0f, 5.0f));
	brush->SetEndPoint(D2D1::Point2F(0.0f, 0.0f));
	//target->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(0.0f, 10.0f), D2D1::Point2F(0.0f, 0.0f)), gradient_stops, &brush);		
	target->BeginDraw();
	target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
	target->FillRectangle(D2D1::RectF(0.0f, 0.0f, static_cast<FLOAT>(KaminoGlobal::g_screen_width), 5.0f), brush);
	target->EndDraw();		
	UpdateLayeredWindow(win_bottom, NULL, &p1, &sz, bitmap_dc, &p1, 0, &blend, ULW_ALPHA); 		
	SelectObject(bitmap_dc, old);
	brush->Release();	
	gradient_stops->Release();
	target->Release();
}

LRESULT CALLBACK KaminoGlowWindow::GlowWindowProc(HWND hWnd, UINT msg, WPARAM wParam , LPARAM lParam){
	static KaminoGlowWindow *myself = NULL;
	static WNDPROC input_wndproc = NULL;
	switch(msg){
		case WM_CREATE:{
			CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
			myself = (KaminoGlowWindow *)pcs->lpCreateParams;
			return 0;
		}
		case WM_MOUSEMOVE:{
				if(input_wndproc == NULL)
				input_wndproc = (WNDPROC)GetWindowLongPtr(myself->mouse_win, GWLP_WNDPROC);
				if((myself != NULL) && (myself->win_top == hWnd) && (myself->mouse_win != NULL)){
					//SendMessage(myself->mouse_win, msg, wParam, lParam);
					//CallWindowProc(input_wndproc, myself->mouse_win, msg, wParam, lParam);
					input_wndproc(myself->mouse_win, msg, MK_CONTROL, lParam);
				}
				return 0;
			}
		case WM_LBUTTONUP:{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);
				if(input_wndproc == NULL)
					input_wndproc = (WNDPROC)GetWindowLongPtr(myself->mouse_win, GWLP_WNDPROC);
				if((myself != NULL) && (myself->win_top == hWnd) && (myself->mouse_win != NULL)){
					input_wndproc(myself->mouse_win, msg, MK_CONTROL, lParam);
				}
				return 0;
			}
		case WM_LBUTTONDOWN:{
				int x = GET_X_LPARAM(lParam);
				int y = GET_Y_LPARAM(lParam);
				if(input_wndproc == NULL)
					input_wndproc = (WNDPROC)GetWindowLongPtr(myself->mouse_win, GWLP_WNDPROC);
				if((myself != NULL) && (myself->win_top == hWnd) && (myself->mouse_win != NULL)){
					input_wndproc(myself->mouse_win, msg, MK_CONTROL, lParam);
				}
				return 0;
			}
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		case WM_DESTROY:
			if(myself){
				if(myself->win_bottom == hWnd){
					myself->win_bottom = NULL;
					DeleteObject(myself->win_bottom_bitmap);
					myself->win_bottom_bitmap = NULL;
					return 0;
				}
				if(myself->win_top == hWnd){
					myself->win_top = NULL;
					DeleteObject(myself->win_top_bitmap);
					myself->win_top_bitmap = NULL;
					return 0;
				}
				if(myself->win_right == hWnd){
					myself->win_right = NULL;
					DeleteObject(myself->win_right_bitmap);
					myself->win_right_bitmap = NULL;
					return 0;
				}
				if(myself->win_left == hWnd){
					myself->win_left = NULL;
					DeleteObject(myself->win_left_bitmap);
					myself->win_left_bitmap = NULL;
					return 0;
				}
			}
			return 0;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}		
}
