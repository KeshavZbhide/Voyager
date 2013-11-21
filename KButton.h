// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef K_BUTTON_H
#define K_BUTTON_H
#include "v8\include\v8.h"
#include <windows.h>
#include "KTab.h"
#include "KaminoDirectResource.h"

class KTabManager;

class KBackButton{
public:
	KBackButton(KTabManager *, int, int);
	~KBackButton();
	void Render(ID2D1RenderTarget *);
	void OnClick();
	HRESULT InitializeDeviceDependantResource(ID2D1RenderTarget *);
private:
	KTabManager *manager;
	KaminoDirect2DBitmap image;
};

class KAddTabButton{
public:
	KAddTabButton(KTabManager *);
	~KAddTabButton();
	HRESULT InitializeDeviceDependantResource(ID2D1RenderTarget *);
	bool IfMouseIsHoveringOver(int x, bool &should_render_again);
	bool IfClicked(int x);
	void MouseOut();
	void Render(ID2D1RenderTarget *target);
private:
	int startX;
	bool hovering_over;
	KTabManager *manager;
};

class KLikeButton{
public:
	KLikeButton(KTabManager *, int, int);
	~KLikeButton();
	void Render(ID2D1RenderTarget *);
	void OnClick();
	HRESULT InitializeDeviceDependantResource(ID2D1RenderTarget *);
private:
	KTabManager *manager;
	KaminoDirect2DBitmap image;
};

class KMinMaxCloseButton{

public:
	enum HoveringOver{
		HoveringOverNone,
		HoveringOverMin,
		HoveringOverMax,
		HoveringOverClose,
	};
	KMinMaxCloseButton(KTabManager *);
	~KMinMaxCloseButton();
	bool IfMouseIsHoveringOver(int x, bool &should_render_again);
	bool IfClicked(int x);
	void MouseOut();
	void Render(ID2D1RenderTarget *);
	HRESULT InitializeDeviceDependantResource(ID2D1RenderTarget *);
private:
	int startX;
	KTabManager *manager;
	KaminoDirect2DBrush yello;
	KaminoDirect2DBrush green;
	KaminoDirect2DBrush red;
	HoveringOver hovering_over;
};

#endif