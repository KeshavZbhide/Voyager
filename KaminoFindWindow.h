// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_FIND_WINDOW_H
#define KAMINO_FIND_WINDOW_H

#include "KTab.h"
#include "content\public\browser\web_contents.h"

class KaminoFindWindow{
public:
	KaminoFindWindow(HWND win);
	~KaminoFindWindow();
	void Show(content::WebContents *source);
	void Hide();
	bool IsVisible();
private:
	bool is_typing;
	WNDCLASSEX wnd_cls;
	HWND parent;
	HWND main_window;
	HWND edit_window;
	content::WebContents *current_content;
	bool is_visible;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK CustomeEditProc(HWND, UINT, WPARAM, LPARAM);
};



#endif