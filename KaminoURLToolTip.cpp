// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoURLToolTip.h"
#include "UIMain.h"
#include "KaminoGlobal.h"

int current_len = 0;

KaminoURLToolTip::KaminoURLToolTip(KTabManager *m)
{
	manager = m;
	app_window = NULL;
	is_tooltip_shown = false;
	text = NULL;
	current_tab = NULL;
	tooltip_wnd_cls.cbClsExtra = 0;
	tooltip_wnd_cls.cbSize = sizeof(WNDCLASSEX);
	tooltip_wnd_cls.cbWndExtra = 0;
	tooltip_wnd_cls.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	tooltip_wnd_cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	tooltip_wnd_cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	tooltip_wnd_cls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	tooltip_wnd_cls.hInstance = GetModuleHandle(NULL);
	tooltip_wnd_cls.lpfnWndProc = ToolTipWndProc;
	tooltip_wnd_cls.lpszClassName = L"URLToolTip";
	tooltip_wnd_cls.lpszMenuName = NULL;
	tooltip_wnd_cls.style = (CS_HREDRAW|CS_VREDRAW);
	RegisterClassEx(&tooltip_wnd_cls);
}

KaminoURLToolTip::~KaminoURLToolTip(void)
{
	if(tooltip_window)
		DestroyWindow(tooltip_window);
	if(text)
		delete text;
}

HRESULT KaminoURLToolTip::Initialize(HWND hWnd){
	app_window = hWnd;
	RECT rc;
	GetClientRect(app_window, &rc);
	HRESULT hr = InitDeviceIndependantResource();
	if(SUCCEEDED(hr)){
		tooltip_window = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST /*| WS_EX_LAYERED*/, tooltip_wnd_cls.lpszClassName, L"", NULL, 0, 
			0, 0, 0, manager->core_parent, NULL, tooltip_wnd_cls.hInstance, this);
		SetWindowText(tooltip_window, L"");
		//SetLayeredWindowAttributes(tooltip_window, 0, 70, LWA_ALPHA);
		if(tooltip_window == NULL)
			return E_FAIL;
	}
	return hr;		
}
LRESULT CALLBACK KaminoURLToolTip::ToolTipWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){	
	KaminoURLToolTip *myself = NULL;
	if(msg == WM_CREATE){
		CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
		myself = (KaminoURLToolTip *)pcs->lpCreateParams;
		myself->tooltip_window = hWnd;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(myself));
		return 0;
	}
	myself = reinterpret_cast<KaminoURLToolTip *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
	if(myself == NULL){
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	switch(msg){
		case WM_NCCALCSIZE:
			if(wParam == TRUE)
				return 0;
			return DefWindowProc(hWnd, msg, wParam, lParam);
		case WM_MOUSEMOVE: //this should never happen but in case, the tool tip is shown, hide it.
			ShowWindow(hWnd, SW_HIDE);
			return 0;
		case WM_PAINT:
			if(SUCCEEDED(myself->InitDeviceDependantResource())){
				myself->direct2d_render_target.Get()->BeginDraw();
				UI_LOG(0, "URL TOOLTIP RENDERING")
				myself->direct2d_render_target.Get()->Clear(D2D1::ColorF(0.176f, 0.176f, 0.176f, 1.0f));
				myself->direct2d_render_target.Get()->DrawTextW(myself->text, myself->input_len, myself->text_format.Get(), 
					D2D1::RectF(((myself->client_rect.right - myself->client_rect.left)* 0.037f), (myself->client_rect.bottom - myself->client_rect.top) * (-0.037f), 
						myself->client_rect.right + 100, myself->client_rect.bottom + 100), 
					myself->text_brush.Get()); 
				if((myself->current_tab != NULL) && (myself->current_tab->favicon.Get() != NULL)){
					myself->direct2d_render_target.Get()->DrawBitmap(myself->current_tab->favicon.Get(), 
						D2D1::RectF(myself->client_rect.left+10.0f, (((myself->client_rect.bottom - myself->client_rect.top)/1.8f) - 8.0f),
						myself->client_rect.left+26.0f, ((myself->client_rect.bottom - myself->client_rect.top)/1.8f) + 8.0f), 1.0); 
				}
				if(myself->direct2d_render_target.Get()->EndDraw() == D2DERR_RECREATE_TARGET)
					myself->DestroyDeviceDependantResource();
				ValidateRect(hWnd, NULL);
			}
			return 0;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}	
}

HRESULT KaminoURLToolTip::InitDeviceIndependantResource(){
	HRESULT hr = E_FAIL;
	float font_size = KaminoGlobal::g_screen_height / 10.0f;
	if(text_format.Get() == NULL)
		hr = text_format.Reset(L"Ubuntu", font_size, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
	return hr;
}

HRESULT KaminoURLToolTip::InitDeviceDependantResource(){
	HRESULT hr = S_OK;
	if(direct2d_render_target.Get() == NULL){
		GetClientRect(tooltip_window, &client_rect);
		hr = direct2d_render_target.Reset(tooltip_window, client_rect);
		if(SUCCEEDED(hr)){
			direct2d_render_target.Get()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			direct2d_render_target.Get()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
			if(text_brush.Get() == NULL)
				hr = text_brush.Reset(1.176f, 1.176f, 1.176f, 1.0f, direct2d_render_target.Get());
		}
	}
	return hr;
}

void KaminoURLToolTip::DestroyDeviceDependantResource(){
	direct2d_render_target.Clean();
	text_brush.Clean();
}

HRESULT KaminoURLToolTip::RenderText(const char *str, size_t len, KTab *tab){
	UI_LOG(0, "URL_TOOLTIP Should Render Text")
	if(len <= 1 || str[0] == '\0')
		return S_OK;
	if(text == NULL)
		text = new wchar_t[50];
	for(int i = 0; i<50; i++)
		text[i] = 0;
	int start = 0;
	if(str[0] == 'w' && str[1] == 'w' && str[2] == 'w' && str[3] == '.')
		start += 4;
	int limit = start;
	for(input_len = 0; input_len <= (len - limit); input_len++, start++)
		text[input_len] = str[start];
	if((text[0] >= 65) && (text[0] <= 90))
		text[0] += 32;
	current_tab = tab;
	if(!is_tooltip_shown){
		is_tooltip_shown = true;	
		ShowWindow(tooltip_window, SW_SHOWNOACTIVATE);
		int width = static_cast<int>(static_cast<double>(KaminoGlobal::g_screen_width) / 1.61);
		int height = static_cast<int>(KaminoGlobal::g_screen_height / 6.85410);
		int startX = 0;
		int startY = 0;
		if(KaminoGlobal::g_is_window_fullscreen){
			startX = 20;
			startY = KaminoGlobal::g_screen_height - height - 15;
		}
		else{
			startX = KaminoGlobal::g_ideal_rect.left + 20;
			startY = KaminoGlobal::g_ideal_rect.bottom - height - 10;
		}
		MoveWindow(tooltip_window, startX, startY, width, height, TRUE);	
	}
	InvalidateRect(tooltip_window, NULL, TRUE);
	return S_OK;
}


void KaminoURLToolTip::Hide(){
	if(is_tooltip_shown){
		if(manager)
			manager->Focus();
		ShowWindow(tooltip_window, SW_HIDE);
		is_tooltip_shown = false;
		current_tab = NULL;
	}
}
