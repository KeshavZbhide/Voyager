// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_WAIT_WINDOW_H
#define KAMINO_WAIT_WINDOW_H

#include "content\public\browser\browser_thread.h"
#include <windows.h>
#include "KaminoDirectResource.h"
#include "KaminoSimpleTimer.h"

//This is the basic Window Shown When content is Loading at the side corner.
class KaminoWaitWindow{
public:
	KaminoWaitWindow();
	~KaminoWaitWindow();
	HRESULT Initialize();
	void Show();
	void Hide();	
	void Render();
private:
	bool is_shown;
	HWND wait_window;
	WNDCLASSEX wnd_cls;
	DWORD animation_thread_id;
	HANDLE animation_thread_handle;
	KaminoDirect2DRenderTarget render_target;
	KaminoDirect2DBrush brush;
	KaminoTimer timer;
	HRESULT InitDeviceDependantResource();
	void DestroyDeviceDependantResource();
	static DWORD WINAPI animation_thread_main(LPVOID);
	static LRESULT CALLBACK KaminoWaitWndProc(HWND, UINT, WPARAM, LPARAM);
};



#endif