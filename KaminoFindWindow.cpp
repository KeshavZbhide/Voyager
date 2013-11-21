// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoFindWindow.h"
#include "KaminoGlobal.h"
#include "UIMain.h"
#include <window.h>
#include "content\public\browser\render_view_host.h"
#include "third_party\WebKit\public\web\WebFindOptions.h"

namespace{
	WNDPROC basic_edit_proc = NULL;
}

KaminoFindWindow::KaminoFindWindow(HWND win){
	parent = win;
	is_typing = false;
	basic_edit_proc = NULL;
	main_window = NULL;
	wnd_cls.cbClsExtra = 0;
	wnd_cls.cbSize = sizeof(WNDCLASSEX);
	wnd_cls.cbWndExtra = 0;
	wnd_cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd_cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd_cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd_cls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wnd_cls.hInstance = GetModuleHandle(NULL);
	wnd_cls.lpfnWndProc = WndProc;
	wnd_cls.lpszClassName = L"FindWindow";
	wnd_cls.lpszMenuName = NULL;
	wnd_cls.style = (CS_HREDRAW | CS_VREDRAW);
	RegisterClassEx(&wnd_cls);
	is_visible = false;
	main_window = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, wnd_cls.lpszClassName, L"Search Term", WS_CAPTION | WS_SYSMENU, 0, 0, 0, 0, parent, NULL, wnd_cls.hInstance, this);	
	SetWindowPos(main_window, HWND_TOPMOST, KaminoGlobal::g_ideal_rect.right - 400, KaminoGlobal::g_ideal_rect.top + 60 , 300, 53, SWP_HIDEWINDOW); 
}

KaminoFindWindow::~KaminoFindWindow(){
	if(main_window)
		DestroyWindow(main_window);
}

void KaminoFindWindow::Show(content::WebContents *source){
	is_visible = true;
	is_typing = false;
	current_content = source;
	ShowWindow(main_window, SW_SHOW);
}

void KaminoFindWindow::Hide(){
	is_visible = false;
	if(current_content != NULL && is_typing)
		current_content->GetRenderViewHost()->StopFinding(content::STOP_FIND_ACTION_CLEAR_SELECTION);
	is_typing = false;
	ShowWindow(main_window, SW_HIDE);
}

bool KaminoFindWindow::IsVisible(){ return is_visible; }

LRESULT CALLBACK KaminoFindWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	KaminoFindWindow *myself = NULL;
	static HWND edit_window = NULL;
	static HFONT edit_window_font;
	static LOGFONT lf;
	if(msg == WM_CREATE){
		CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
		myself = (KaminoFindWindow *)pcs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(myself));
		edit_window = CreateWindowEx(NULL, L"EDIT", L"", WS_CHILD, 0, 0, 300, 100, hWnd, NULL, myself->wnd_cls.hInstance, myself);
		SetWindowLongPtrW(edit_window, GWLP_USERDATA, PtrToUlong(myself));
		GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf); 
		edit_window_font = CreateFont(lf.lfHeight * 1.5, lf.lfWidth,  lf.lfEscapement, lf.lfOrientation, lf.lfWeight, lf.lfItalic, lf.lfUnderline, 
			lf.lfStrikeOut, lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, lf.lfPitchAndFamily, L"Verdana"); 
		SendMessage(edit_window, WM_SETFONT, WPARAM(edit_window_font), TRUE);
		basic_edit_proc = (WNDPROC)SetWindowLongPtrW(edit_window, GWL_WNDPROC, (LONG)KaminoFindWindow::CustomeEditProc);
		ShowWindow(edit_window, SW_SHOW);
		UpdateWindow(edit_window);
		return 0;
	}
	myself = reinterpret_cast<KaminoFindWindow *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
	if(myself == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);
	switch(msg){
		case WM_CLOSE:
			myself->Hide();
			return 0;
		case WM_DESTROY:
			if(edit_window != NULL)
				DestroyWindow(edit_window);
			return 0;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT CALLBACK KaminoFindWindow::CustomeEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	KaminoFindWindow *myself = NULL;
	myself = reinterpret_cast<KaminoFindWindow *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));	
	if((myself != NULL) && (msg == WM_KEYUP || msg == WM_CHAR || msg == WM_KEYDOWN)){
		int ret = 0;
		if(wParam != VK_RETURN)
			ret = CallWindowProc(basic_edit_proc, hWnd, msg, wParam, lParam);
		if(msg == WM_KEYUP){
			myself->is_typing = true;
			wchar_t *str = NULL;
			int len = GetWindowTextLength(hWnd);
			if(len > 0){
				str = (wchar_t *)malloc(len * sizeof(wchar_t));
				GetWindowText(hWnd, str, len);
				myself->current_content->GetRenderViewHost()->Find(0, str, WebKit::WebFindOptions());
				free(str);
			}
		}
		return ret;
	}
	return CallWindowProc(basic_edit_proc, hWnd, msg, wParam, lParam);
}
