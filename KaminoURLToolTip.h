// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_URL_TOOL_TIP_H
#define KAMINO_URL_TOOL_TIP_H

#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <string>
#include "KTab.h"
#include "KaminoDirectResource.h"


class KTabManager;
class KTab;

class KaminoURLToolTip
{
public:
	KaminoURLToolTip(KTabManager *);
	HRESULT Initialize(HWND);
	//HRESULT RenderText(const char *str, size_t len);
	HRESULT RenderText(const char *str, size_t len, ID2D1Bitmap *);
	HRESULT RenderText(const char *str, size_t len, KTab *);
	void Hide();
	bool is_tooltip_shown;
	static LRESULT CALLBACK ToolTipWndProc(HWND, UINT, WPARAM, LPARAM);
	~KaminoURLToolTip(void);
	KTabManager *manager;
	KTab *current_tab;
//private :
	KaminoDirect2DRenderTarget direct2d_render_target;
	KaminoTextFormat text_format;
	KaminoDirect2DBrush text_brush;
	HWND tooltip_window;
	HWND app_window;
	WNDCLASSEX tooltip_wnd_cls;
	wchar_t *text;
	size_t input_len;
	RECT client_rect;
	HRESULT InitDeviceIndependantResource();
	HRESULT InitDeviceDependantResource();
	void DestroyDeviceDependantResource();
};

#endif