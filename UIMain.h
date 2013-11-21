// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef UI_MAIN_H
#define UI_MAIN_H
#include "base\callback.h"

void ui_log(int funcEntryOrExit, char *format, ...);

#define KAMINO_BROWSER_VERSION "0"
#define KAMINO_BROWSER_VERSION_INT 0

//#define UI_DEBUG_LOG
#ifdef UI_DEBUG_LOG
#include <stdio.h>
#define UI_LOG(...) ui_log(__VA_ARGS__);
#define IF_UI_LOG(A, ...) if(A){ ui_log(__VA_ARGS__);}
#define ELSE_UI_LOG(...) else{ui_log(__VA_ARGS__);}
#else
#define UI_LOG(...) 
#define IF_UI_LOG(A, ...)
#define ELSE_UI_LOG(...)
#endif



//Main Embeder Class To build UI.
class UIMain{
public:
	UIMain() { };
	virtual ~UIMain() { };
	virtual void main() { };
};

UIMain* GetUIMain();
void DestroyUIMain();

class KTabManager;
class BasicUIMain : public UIMain{
public:
	BasicUIMain();
	~BasicUIMain();
	void main() override;	
	static LRESULT CALLBACK BasicWndProc(HWND, UINT, WPARAM, LPARAM);
	KTabManager* tab_manager;
	HWND main_window;
	WNDCLASSEX wnd_cls;	
private:
	void BuildStartUpUrl();
};

#endif