// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoDirectResource.h"
#include "UIMain.h"

#include <xmmintrin.h>
#include <mmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "WindowsCodecs.lib")


void ErrorDescription(HRESULT hr) 
{ 
     if(FACILITY_WINDOWS == HRESULT_FACILITY(hr)) 
         hr = HRESULT_CODE(hr); 
     char* szErrMsg; 

     if(FormatMessageA( 
       FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, 
       NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
       (LPSTR)&szErrMsg, 0, NULL) != 0)  
         LocalFree(szErrMsg); 
}


char *UTF16ToUTF8( __in LPCWSTR pszTextUTF16 ) {
    if (pszTextUTF16 == NULL) return "";
    int utf16len = wcslen(pszTextUTF16);
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, pszTextUTF16, utf16len, 
        NULL, 0, NULL, NULL );
    char *buffer = new char[(utf8len+1)];
	buffer[utf8len] = '\0';
    WideCharToMultiByte(CP_UTF8, 0, pszTextUTF16, utf16len, 
        buffer, utf8len, 0, 0 );
    return buffer;
}

namespace DirectResourceGlobal{
	ID2D1Factory *factory = NULL;
	IDWriteFactory *write_factory = NULL;
	IWICImagingFactory *wic_factory = NULL;
}

HRESULT HackInitWicFactory(){
	return CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
			reinterpret_cast<void **>(&DirectResourceGlobal::wic_factory));	
}

HRESULT KaminoDirectResourceInitialize(){
	HRESULT hr = CoInitialize(NULL);
	DirectResourceGlobal::factory = NULL;
	DirectResourceGlobal::wic_factory = NULL;
	DirectResourceGlobal::write_factory = NULL;
	return hr;
}

HRESULT KaminoDirectResourceDestory(){
	HRESULT hr = S_OK;
	if(DirectResourceGlobal::factory){
		DirectResourceGlobal::factory->Release();
	}
	DirectResourceGlobal::factory = NULL;
	if(DirectResourceGlobal::wic_factory){
		DirectResourceGlobal::wic_factory->Release();
	}
	DirectResourceGlobal::wic_factory = NULL;
	if(DirectResourceGlobal::write_factory){
		DirectResourceGlobal::write_factory->Release();
	}
	DirectResourceGlobal::write_factory = NULL;
	return hr;
}


//Has Global Pointer
KaminoDirect2DFactory::KaminoDirect2DFactory(){		
}
KaminoDirect2DFactory::~KaminoDirect2DFactory(){
}
HRESULT KaminoDirect2DFactory::Reset(){
	HRESULT hr = S_OK;
	if(DirectResourceGlobal::factory == NULL){
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &DirectResourceGlobal::factory);
		if(FAILED(hr))
			DirectResourceGlobal::factory = NULL;
	}
	return hr;
}
ID2D1Factory *KaminoDirect2DFactory::Get(){
	if(DirectResourceGlobal::factory == NULL)
		Reset();
	return DirectResourceGlobal::factory;
}

//Has Global Pointer
KaminoWriteFactory::KaminoWriteFactory(){
}
KaminoWriteFactory::~KaminoWriteFactory(){
}
HRESULT  KaminoWriteFactory::Reset(){
	HRESULT hr = S_OK;
	if(DirectResourceGlobal::write_factory == NULL){
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&DirectResourceGlobal::write_factory));
		if(FAILED(hr))
			DirectResourceGlobal::write_factory = NULL;
	}
	return hr;
}
IDWriteFactory *KaminoWriteFactory::Get(){
	if(DirectResourceGlobal::write_factory == NULL)
		Reset();
	return DirectResourceGlobal::write_factory;
}

//Has Global Pointer
KaminoWicImagingFactory::KaminoWicImagingFactory(){
}
KaminoWicImagingFactory::~KaminoWicImagingFactory(){
}
HRESULT KaminoWicImagingFactory::Reset(){
	HRESULT hr = S_OK;
	if(DirectResourceGlobal::wic_factory == NULL){
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
			reinterpret_cast<void **>(&DirectResourceGlobal::wic_factory));
		if(FAILED(hr)){
			DirectResourceGlobal::wic_factory = NULL;
			ErrorDescription(hr);			
		}
	}
	return hr;
}
IWICImagingFactory *KaminoWicImagingFactory::Get(){
	if(DirectResourceGlobal::wic_factory == NULL)
		Reset();
	return DirectResourceGlobal::wic_factory;
}


KaminoDirect2DRenderTarget::KaminoDirect2DRenderTarget(){
	type_ = KaminoDirect2DRenderTargetTypeUnknown;
	ptr_ = NULL;
}
KaminoDirect2DRenderTarget::~KaminoDirect2DRenderTarget(){
	if(ptr_)
		ptr_->Release();
	ptr_ = NULL;
	type_ = KaminoDirect2DRenderTargetTypeUnknown;
}
HRESULT KaminoDirect2DRenderTarget::Reset(HWND window, RECT &rc){
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
	type_ = KaminoDirect2DRenderTargetTypeHwnd;
	KaminoDirect2DFactory factory;
	if(factory.Get() == NULL)
		return E_FAIL;
	hr = factory.Get()->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(window, 
		D2D1::SizeU(static_cast<UINT32>(rc.right - rc.left), static_cast<UINT32>(rc.bottom - rc.top))), reinterpret_cast<ID2D1HwndRenderTarget **>(&ptr_));
	if(FAILED(hr))
		ptr_ = NULL;
	return hr;
}
HRESULT KaminoDirect2DRenderTarget::Reset(ID2D1RenderTarget *target, int width, int height){
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
	type_ = KaminoDirect2DRenderTargetTypeBitmap;
	hr = target->CreateCompatibleRenderTarget(D2D1::SizeF(static_cast<FLOAT>(width), static_cast<FLOAT>(height)), reinterpret_cast<ID2D1BitmapRenderTarget **>(&ptr_));
	if(FAILED(hr))
		ptr_ = NULL;
	return hr;
}
ID2D1RenderTarget *KaminoDirect2DRenderTarget::Get(){
	return ptr_;
}
void KaminoDirect2DRenderTarget::Clean(){
	if(ptr_ != NULL)
		ptr_->Release();
	ptr_ = NULL;
}

KaminoDirect2DBrush::KaminoDirect2DBrush(){
	ptr_ = NULL;
	type_ = KaminoDirect2DBrushTypeUnknown;
}
KaminoDirect2DBrush::~KaminoDirect2DBrush(){
	if(ptr_ != NULL)
		ptr_->Release();
	type_ = KaminoDirect2DBrushTypeUnknown;
}
HRESULT KaminoDirect2DBrush::Reset(float r, float g, float b, float a, ID2D1RenderTarget *target){
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
	type_ = KaminoDirect2DBrushTypeSolidColor;
	hr = target->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), reinterpret_cast<ID2D1SolidColorBrush **>(&ptr_));
	if(FAILED(hr))
		ptr_ = NULL;
	return hr;
}
HRESULT KaminoDirect2DBrush::Reset(D2D1_GRADIENT_STOP *gradientStops, int len, D2D1_POINT_2F *point, float radius_width, float radius_height, 
	ID2D1RenderTarget *target)
{
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
	type_ = KaminoDirect2DBrushTypeRadialGradient;
	ID2D1GradientStopCollection *gradient_stops;
	hr = target->CreateGradientStopCollection(gradientStops, len, D2D1_GAMMA_1_0, D2D1_EXTEND_MODE_CLAMP, &gradient_stops);
	if(SUCCEEDED(hr))
		hr = target->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties((*point), D2D1::Point2F(0.0f,0.0f), radius_width, radius_height), 
		gradient_stops, reinterpret_cast<ID2D1RadialGradientBrush **>(&ptr_));
	if(FAILED(hr))
		ptr_ = NULL;
	return hr;
}
ID2D1Brush *KaminoDirect2DBrush::Get(){
	return ptr_;
}
void KaminoDirect2DBrush::Clean(){
	if(ptr_ != NULL){
		ptr_->Release();
	}
	ptr_ = NULL;
}



KaminoDirect2DPathGeometry::KaminoDirect2DPathGeometry(){
	ptr_ = NULL;
}
KaminoDirect2DPathGeometry::~KaminoDirect2DPathGeometry(){
	if(ptr_ != NULL){
		ptr_->Release();
	}
	ptr_ = NULL;
}
HRESULT KaminoDirect2DPathGeometry::Reset(){
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
	KaminoDirect2DFactory factory;
	hr = factory.Get()->CreatePathGeometry(&ptr_);
	return hr;
}
ID2D1PathGeometry *KaminoDirect2DPathGeometry::Get(){
	return ptr_;
}
void KaminoDirect2DPathGeometry::Clean(){
	if(ptr_ != NULL){
		ptr_->Release();
	}
	ptr_ = NULL;
}


KaminoDirect2DBitmap::KaminoDirect2DBitmap(){
	ptr_ = NULL;
	type_ = KaminoDirect2DBitmapTypeUnknown;
}
KaminoDirect2DBitmap::~KaminoDirect2DBitmap(){
	if(ptr_ != NULL){
		ptr_->Release();
	}
	ptr_ = NULL;
	type_ = KaminoDirect2DBitmapTypeUnknown;
}
HRESULT KaminoDirect2DBitmap::Reset(ID2D1BitmapRenderTarget *target){
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
	hr = target->GetBitmap(&ptr_);
	if(FAILED(hr))
		ptr_ = NULL;
	return hr;
}
HRESULT KaminoDirect2DBitmap::Reset(ID2D1RenderTarget *target, wchar_t *file_name){
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
//#define FORCE_USE_LIB_PNG_FOR_D2D_BITMAP
#ifdef FORCE_USE_LIB_PNG_FOR_D2D_BITMAP		
	char *c_str_file_name = UTF16ToUTF8(file_name);
	hr = LoadBitmapFromFileUsingLibPng(c_str_file_name, &ptr_, target);
	delete c_str_file_name;
#else
	KaminoWicImagingFactory wic_factory;
	hr = LoadBitmapFromFile(target, wic_factory.Get(), file_name, &ptr_);
	if(FAILED(hr)){
		char *c_str_file_name = UTF16ToUTF8(file_name);
		hr = LoadBitmapFromFileUsingLibPng(c_str_file_name, &ptr_, target);
		delete c_str_file_name;
	}
#endif
	if(FAILED(hr))
		ptr_ = NULL;
	return hr;
}

HRESULT KaminoDirect2DBitmap::Reset(ID2D1RenderTarget *target, char *mem, int width, int height, bool is_premultiplied){
	HRESULT hr = target->CreateBitmap(D2D1::SizeU(width, height),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &ptr_);
	ptr_->CopyFromMemory(&D2D1::RectU(0, 0, width, height), mem, width*4);
	return hr;
}

ID2D1Bitmap *KaminoDirect2DBitmap::Get(){
	return ptr_;
}
void KaminoDirect2DBitmap::Clean(){
	if(ptr_ != NULL){
		ptr_->Release();
	}
	ptr_ = NULL;
	type_ = KaminoDirect2DBitmapTypeUnknown;
}

HRESULT KaminoDirect2DBitmap::LoadBitmapFromFile(ID2D1RenderTarget *pRenderTarget, IWICImagingFactory *pIWICFactory, PCWSTR uri,ID2D1Bitmap **ppBitmap){
	HRESULT hr = S_OK;
	UINT destinationWidth = 0;
	UINT destinationHeight = 0;
    IWICBitmapDecoder *pDecoder = NULL;
    IWICBitmapFrameDecode *pSource = NULL;
    IWICStream *pStream = NULL;
    IWICFormatConverter *pConverter = NULL;
    IWICBitmapScaler *pScaler = NULL;
	if(pIWICFactory == NULL)
		return E_FAIL;
    hr = pIWICFactory->CreateDecoderFromFilename(uri, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);
    if (SUCCEEDED(hr))
        hr = pDecoder->GetFrame(0, &pSource);
    if (SUCCEEDED(hr))
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    if (SUCCEEDED(hr)){
        if (destinationWidth != 0 || destinationHeight != 0){
            UINT originalWidth, originalHeight;
            hr = pSource->GetSize(&originalWidth, &originalHeight);
            if (SUCCEEDED(hr)){
                if (destinationWidth == 0){
                    FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                    destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                }
                else if (destinationHeight == 0){
                    FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                    destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                }
                hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                if (SUCCEEDED(hr)){
                    hr = pScaler->Initialize( pSource, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);
                }
                if (SUCCEEDED(hr)){
                    hr = pConverter->Initialize(
                        pScaler,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        NULL,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                        );
                }
            }
        }
        else{
            hr = pConverter->Initialize(
                pSource,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                NULL,
                0.f,
                WICBitmapPaletteTypeMedianCut
                );
        }
    }
    if (SUCCEEDED(hr)){
        hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
    }
	if(pDecoder)
		pDecoder->Release();
	if(pSource)
		pSource->Release();
    if(pStream)
		pStream->Release();
    if(pConverter)
		pConverter->Release();
    if(pScaler)
		pScaler->Release();
    return hr;	
}

HRESULT KaminoDirect2DBitmap::LoadBitmapFromFileUsingLibPng(char *file, ID2D1Bitmap **bit_map, ID2D1RenderTarget *target){
	HRESULT hr = E_FAIL;
	unsigned char header[8];
	unsigned char *image_mem;
	png_struct *png = NULL;
	png_info *image_info;
	int width = 0;
	int height = 0;
	FILE *fp = fopen(file, "rb");
	if(!fp)
		return E_FAIL;
	fread(header, 1, 8, fp);
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	image_info = png_create_info_struct(png);
	if (setjmp(png_jmpbuf(png)))
		 return E_FAIL;
	png_init_io(png, fp);
	png_set_sig_bytes(png, 8);
	png_read_info(png, image_info);
	width = png_get_image_width(png, image_info);
	height = png_get_image_height(png, image_info);	
 	unsigned char **row_bytes = new unsigned char *[height];
	image_mem = (unsigned char *)malloc(png_get_rowbytes(png,image_info) * height);
	for(int i = 0; i <  height; i++)
		row_bytes[i] = image_mem + (png_get_rowbytes(png,image_info)*i);
	png_read_image(png, row_bytes);
	int *temp_reg = (int *)_aligned_malloc(sizeof(float) * 4, 16);
	int loop_limit = png_get_rowbytes(png,image_info) * height;	
	for(int i = 0; i<loop_limit; i+=4){				
		__m128i pixel_int = _mm_setr_epi32(image_mem[i+2], image_mem[i+1], image_mem[i], image_mem[i+3]);  //load in bgra format
		__m128i alpha_int = _mm_setr_epi32(image_mem[i+3], image_mem[i+3], image_mem[i+3], 255);
		__m128 alpha_float = _mm_cvtepi32_ps(alpha_int);
		__m128 pixel_float = _mm_cvtepi32_ps(pixel_int);
		__m128 max_alpha_float_rcp = _mm_setr_ps(255.0f, 255.0f, 255.0f, 255.0f);
		max_alpha_float_rcp = _mm_rcp_ps(max_alpha_float_rcp);
		alpha_float = _mm_mul_ps(alpha_float, max_alpha_float_rcp);
		pixel_float = _mm_mul_ps(pixel_float, alpha_float);
		__m128i pixel_result = _mm_cvtps_epi32(pixel_float);
		_mm_store_si128((__m128i *)temp_reg, pixel_result);

		image_mem[i] = (unsigned char)temp_reg[0];
		image_mem[i+1] = (unsigned char)temp_reg[1];
		image_mem[i+2] = (unsigned char)temp_reg[2];
		image_mem[i+3] = (unsigned char)temp_reg[3];
	}
	hr = target->CreateBitmap(D2D1::SizeU(width, height),
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), bit_map);
	(*bit_map)->CopyFromMemory(&D2D1::RectU(0, 0, width, height), image_mem, png_get_rowbytes(png,image_info));
	free(image_mem);
	image_mem = NULL;
	png_infop end_info = png_create_info_struct(png); 
	png_destroy_read_struct(&png, &image_info, &end_info);
	delete row_bytes;	
	return hr;
}



KaminoTextFormat::KaminoTextFormat(){
	ptr_ = NULL;
}
KaminoTextFormat::~KaminoTextFormat(){
	if(ptr_ != NULL)
		ptr_->Release();
	ptr_ = NULL;
}
HRESULT KaminoTextFormat::Reset(wchar_t *font_name, float size, DWRITE_FONT_WEIGHT font_weight, DWRITE_FONT_STYLE font_style){
	HRESULT hr = S_OK;
	if(ptr_ != NULL)
		ptr_->Release();
	KaminoWriteFactory factory;
	hr = factory.Get()->CreateTextFormat(font_name, NULL, font_weight, font_style, DWRITE_FONT_STRETCH_NORMAL, size, L"", &ptr_);
	if(FAILED(hr))
		ptr_ = NULL;
	return hr;
}
IDWriteTextFormat *KaminoTextFormat::Get(){
	return ptr_;
}
void KaminoTextFormat::Clean(){
	if(ptr_ != NULL)
		ptr_->Release();
	ptr_ = NULL;
}





