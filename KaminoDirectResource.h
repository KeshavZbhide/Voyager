// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_DIRECT_RESOURCE_H
#define KAMINO_DIRECT_RESOURCE_H
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include "third_party\libpng\png.h"



HRESULT KaminoDirectResourceInitialize();
HRESULT KaminoDirectResourceDestory();
HRESULT HackInitWicFactory();
void ErrorDescription(HRESULT hr);
class KaminoDirect2DFactory{
public:
	KaminoDirect2DFactory();
	~KaminoDirect2DFactory();
	HRESULT Reset();
	ID2D1Factory *Get();
};

class KaminoDirect2DRenderTarget{
public:
	enum KaminoDirect2DRenderTargetType{
		KaminoDirect2DRenderTargetTypeUnknown,
		KaminoDirect2DRenderTargetTypeHwnd,
		KaminoDirect2DRenderTargetTypeBitmap,
	};
	ID2D1RenderTarget *ptr_;
	KaminoDirect2DRenderTargetType type_;
	KaminoDirect2DRenderTarget();
	~KaminoDirect2DRenderTarget();
	HRESULT Reset(HWND window, RECT &rc); //Build Hwnd Rendertarget
	HRESULT Reset(ID2D1RenderTarget *target, int width, int height); //Build Compatible Rendertarget
	ID2D1RenderTarget *Get();
	void Clean();
};

class KaminoDirect2DBrush{
public:
	enum KaminoDirect2DBrushType{
		KaminoDirect2DBrushTypeUnknown,
		KaminoDirect2DBrushTypeSolidColor,
		KaminoDirect2DBrushTypeRadialGradient,
	};
	KaminoDirect2DBrushType type_;
	ID2D1Brush *ptr_;
	KaminoDirect2DBrush();
	~KaminoDirect2DBrush();
	HRESULT Reset(float r, float g, float b, float a, ID2D1RenderTarget *target); //SolidColorBrush
	HRESULT Reset(D2D1_GRADIENT_STOP *, int len, D2D1_POINT_2F *point, float radius_width, float radius_height,ID2D1RenderTarget *target); //RadialGradient Brush
	ID2D1Brush *Get();
	void Clean();
};


class KaminoDirect2DPathGeometry{
public:
	ID2D1PathGeometry *ptr_;
	KaminoDirect2DPathGeometry();
	~KaminoDirect2DPathGeometry();
	HRESULT Reset();
	ID2D1PathGeometry *Get();
	void Clean();
};

class KaminoDirect2DBitmap{
public:
	enum KaminoDirect2DBitmapType{
		KaminoDirect2DBitmapTypeUnknown,
		KaminoDirect2DBitmapTypeFromRenderTarget,
		KaminoDirect2DBitmapTypeFromFile
	};
	KaminoDirect2DBitmapType type_;
	ID2D1Bitmap *ptr_;
	KaminoDirect2DBitmap();
	~KaminoDirect2DBitmap();
	HRESULT Reset(ID2D1BitmapRenderTarget *); //From RenderTarget
	HRESULT Reset(ID2D1RenderTarget *,wchar_t *);//File Name;
	HRESULT Reset(ID2D1RenderTarget *, char *mem, int width, int height, bool is_premultiplied);
	ID2D1Bitmap *Get();
	void Clean();
private:
	HRESULT LoadBitmapFromFile(ID2D1RenderTarget *pRenderTarget, IWICImagingFactory *pIWICFactory, PCWSTR uri, ID2D1Bitmap **ppBitmap);
	HRESULT LoadBitmapFromFileUsingLibPng(char *file, ID2D1Bitmap **bit_map, ID2D1RenderTarget *target);
};


class KaminoWriteFactory{
public:
	KaminoWriteFactory();
	~KaminoWriteFactory();
	HRESULT Reset();
	IDWriteFactory *Get();	
};

class KaminoTextFormat{
public:
	IDWriteTextFormat *ptr_;
	KaminoTextFormat();
	~KaminoTextFormat();
	HRESULT Reset(wchar_t *, float size, DWRITE_FONT_WEIGHT, DWRITE_FONT_STYLE);
	IDWriteTextFormat *Get();
	void Clean();
};

class KaminoWicImagingFactory{
public:
	KaminoWicImagingFactory();
	~KaminoWicImagingFactory();
	HRESULT Reset();
	IWICImagingFactory *Get();
};

#endif