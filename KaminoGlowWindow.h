// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_GLOW_WINDOW_H
#define KAMINO_GLOW_WINDOW_H
#include <window.h>

class KaminoGlowWindow{
public:
	KaminoGlowWindow();
	~KaminoGlowWindow();
	void Init(HWND win);
	void Update(int,int,int,int);
	void SetMouseEventForwarding(HWND win);
private:
	HWND main;
	HWND win_right;
	HWND win_left;
	HWND win_top;
	HWND win_bottom;
	HWND mouse_win;

	HBITMAP win_right_bitmap;
	HBITMAP win_left_bitmap;
	HBITMAP win_top_bitmap;
	HBITMAP win_bottom_bitmap;

	WNDCLASSEX wnd_cls;
	void BuildAllBitmaps();
	static LRESULT CALLBACK GlowWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif