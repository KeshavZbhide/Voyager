// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoGlobal.h"
#include "KTab.h"
#include "windowsx.h"
#include <math.h>
#include <Commctrl.h>
#include <stdlib.h>
#include "KaminoHistoryModule.h"
#include "KaminoUpdateStatus.h"
#include "content\public\browser\browser_thread.h"
#include "content\public\browser\notification_types.h"
#include "content\public\browser\notification_source.h"
#include "content\public\browser\native_web_keyboard_event.h"
#include "content\public\browser\render_view_host.h"
#include "net\url_request\url_fetcher.h"
#include "net\url_request\url_fetcher_delegate.h"
#include "third_party\skia\include\core\SkBitmap.h"
#include <tchar.h>

#include "third_party\libxml\chromium\libxml_utils.h"

#define FONT_SIZE 54.0f
#define BUILD_TEXT_GEOMETRY_SUM_LIMIT 70.0f
#define TAB_START_X_OFFSET 100
#define TAB_WIDTH 112
#define TAB_CAPTION_WIDTH 39

#define M_PI 3.14159265358979323846264338327950288

class SKBitmap;

//Initializes the bitmap_render_target and builds the text geometry, but does not touch the text_bitmap.
HRESULT KTabDefaultInitializeDeviceDependantResource(KTab *myself, ID2D1RenderTarget *target){
	HRESULT hr = S_OK;
	if((myself->should_repaint) || (myself->bitmap_render_target.Get() == NULL)){
		if(myself->bitmap_render_target.Get() == NULL){
			if(target == NULL){
				return E_FAIL;
			}
			hr = myself->bitmap_render_target.Reset(target, 130, 35); //Creates Compatible RenderTarget
			if(FAILED(hr))
				return hr;
		}
		if(myself->text_geometry.ptr_ == NULL)
			hr = myself->BuildTextGeometry();
	}
	return hr;
}

void KTabDefaultDestrorDeviceDependantResource(KTab *myself){
	myself->bitmap_render_target.Clean();
	myself->text_bitmap.Clean();
	myself->favicon.Clean();
	myself->should_repaint = true;
	GURL url = myself->web_content->GetURL();
	if(!url.has_host() && url.SchemeIsFile()){
		myself->current_url = url.ExtractFileName();
		size_t len = myself->current_url.find('.');
		myself->current_url.resize(len);
	}
	else
		myself->current_url = myself->web_content->GetURL().host();
}

//OnPaint Event in charge on TextBitmap.
void KTabDefaultOnPaintDirect2DEvent(KTab *self, ID2D1HwndRenderTarget *target){
	//Recreates The TextBitMap. mostly when url changes or during first Paint.
	if((self->text_bitmap.Get() == NULL) || (self->should_repaint)){
		self->bitmap_render_target.Get()->BeginDraw();
		self->bitmap_render_target.Get()->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
		self->bitmap_render_target.Get()->SetTransform(D2D1::Matrix3x2F::Translation(43.0f, 38.0f));
		self->bitmap_render_target.Get()->FillGeometry(self->text_geometry.Get(), self->manager->GiveMeTextBrush());
		self->bitmap_render_target.Get()->EndDraw();
		HRESULT hr  = self->text_bitmap.Reset(reinterpret_cast<ID2D1BitmapRenderTarget *>(self->bitmap_render_target.Get()));
		if(FAILED(hr) || (self->text_bitmap.Get() == NULL))
			return;
		self->should_repaint = false;
	}
	target->SetTransform(D2D1::Matrix3x2F::Translation(static_cast<FLOAT>((self->draged_left_pos == -1) ? self->left_pos : self->draged_left_pos),
		static_cast<FLOAT>(self->top_pos)));
	if(self->top_pos > 2){
		double time_ellapsed = self->manager->timer.getTimeElapsed();
		self->top_pos = static_cast<int>(40.0 - (sin(time_ellapsed*(M_PI/2.f)) * 1300));
	}
	target->DrawGeometry(self->manager->GiveMeTabGeometry(), self->manager->GiveMeTabBorderLineBrush2(), 2.0f);
	target->FillGeometry(self->manager->GiveMeTabGeometry(), self->manager->GiveMeBackgroundBrush(self));
	ID2D1Layer *layer;
	target->CreateLayer(&layer);
	target->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), self->manager->GiveMeTabGeometry()), layer);
	FLOAT opacity = (self == self->manager->current) ? 0.96f : 0.588f;
	target->DrawBitmap(self->text_bitmap.Get(), D2D1::RectF(0.0f, 0.0f, 130.0f, 35.0f), opacity);
	if(self->is_mouse_over){
		target->DrawLine(D2D1::Point2F(20.0f, 5.0f), D2D1::Point2F(30.0f, 4.0f), self->manager->GiveMeTabCloseBrush(), 0.5f);
		target->DrawLine(D2D1::Point2F(20.0f, 7.0f), D2D1::Point2F(30.0f, 7.0f), self->manager->GiveMeTabCloseBrush(), 0.5f);
	}	
	target->PopLayer();
	layer->Release();	
}

void KTabDefaultOnClick(KTab *myself, int x, int y){
	if((x >= (myself->left_pos+19)) && (x <= (myself->left_pos + 31)) && (y >= 4) && (y <= 13))
		myself->manager->CloseTab(myself, false);
	else
		myself->manager->SwitchTo(myself);
	return;
}

//Builds The Url's Font Geometry
HRESULT KTab::BuildTextGeometry(){
	HRESULT hr = S_OK;
	IDWriteFontFile *font_file;
	IDWriteFontFace *font_face;
	KaminoWriteFactory write_factory;
	KaminoDirect2DFactory factory;
	if((write_factory.Get() == NULL) || (factory.Get() == NULL)){
		return E_FAIL;
	}
	hr = write_factory.Get()->CreateFontFileReference(L"../KaminoResource/Oxygen-Bold.ttf", NULL, &font_file);
	if(font_file == NULL){
		UI_LOG(0, "Font File Non Existant")
		return E_FAIL;
	}
	if(SUCCEEDED(hr)){
		hr = write_factory.Get()->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE, 1, &font_file, 0, DWRITE_FONT_SIMULATIONS_NONE, &font_face);
		if(font_face == NULL){
			return E_FAIL;
		}
		if(SUCCEEDED(hr)){
			int start = 0;
			//Remove The Front WWW to give the usere the front letter of the host, should be moved to diffrent method (maybe url manuplation callback)
			if((current_url[0] == 'w') && (current_url[1] == 'w') && (current_url[2] == 'w'))
				start = 4;
			current_url[start] -= 32; //Captialize the first letter
			size_t len = current_url.length();//minmum Size of any url.
			if(len > 4) len = 4;
			DWRITE_FONT_METRICS font_matrics;
			for(size_t i = 0, j = start; i < len; i++, j++)
				KaminoGlobal::g_pCodePoints[i] = static_cast<UINT>(current_url[j]);
			hr = font_face->GetGlyphIndicesW(KaminoGlobal::g_pCodePoints, len, KaminoGlobal::g_pGlyphIndices);
			if(FAILED(hr))
				return hr;
			hr = font_face->GetDesignGlyphMetrics(KaminoGlobal::g_pGlyphIndices, len, KaminoGlobal::g_pGlyphMatrics, FALSE);
			if(FAILED(hr))
				return hr;
			font_face->GetMetrics(&font_matrics);
			FLOAT sum = 0.0f;
			for(size_t i = 0; i<len; i++){
				KaminoGlobal::g_pGlyphAdvances[i] = ((static_cast<FLOAT>(KaminoGlobal::g_pGlyphMatrics[i].advanceWidth) / (font_matrics.designUnitsPerEm))*FONT_SIZE);
				KaminoGlobal::g_pGlyphAdvances[i] -= 0.17f * KaminoGlobal::g_pGlyphAdvances[i];	
				if(sum >= BUILD_TEXT_GEOMETRY_SUM_LIMIT){
					len = i;
					break;
				}
				sum +=  KaminoGlobal::g_pGlyphAdvances[i];
			}
			hr = text_geometry.Reset();
			if(SUCCEEDED(hr)){
				ID2D1GeometrySink *sink = NULL;
				text_geometry.Get()->Open(&sink);
				hr = font_face->GetGlyphRunOutline(FONT_SIZE, KaminoGlobal::g_pGlyphIndices, KaminoGlobal::g_pGlyphAdvances, NULL, len, FALSE, FALSE, sink);
				if(FAILED(hr)){					
					return E_FAIL;
				}
				sink->Close();
			}
			if(font_face)
				font_face->Release();
			if(font_file)
				font_file->Release();
		}
	}
	return hr;
}

//KTab WebContentDelegate Methods;
void KTab::LoadingStateChanged(content::WebContents* source){
	if(!source->IsLoading()){
		is_tittle_set = true;
	}
}

content::WebContents* KTab::OpenURLFromTab(content::WebContents* source, const content::OpenURLParams& params){
	source->GetController().LoadURL(params.url, params.referrer, params.transition, std::string());
	return source;
}

void KTab::WebContentsCreated(content::WebContents* source_contents, int64 source_frame_id, const GURL& target_url,
								content::WebContents* new_contents){
	manager->AddTabNextTo(this, new_contents);
}

void KTab::DidNavigateMainFramePostCommit(content::WebContents* source){
	GURL url = source->GetURL();
	if(!url.has_host() && url.SchemeIsFile()){
		current_url = url.ExtractFileName();
		size_t len = current_url.find('.');
		current_url.resize(len);
	}
	else
		current_url = source->GetURL().host();
	should_repaint = true;
	text_geometry.ptr_->Release();
	text_geometry.ptr_ = NULL;
	is_tittle_set = false;
	if(manager->current == this)
		manager->Render();
}

void KTab::HandleKeyboardEvent(content::WebContents* source,
	const content::NativeWebKeyboardEvent& event){
		if(event.os_event.message == WM_SYSKEYUP){
			//Switch To Next Tab....
			if(((event.os_event.wParam == VK_MENU) && (GetAsyncKeyState(VK_RIGHT) & 0x8000)) || 
				((event.os_event.wParam == VK_RIGHT) && (GetAsyncKeyState(VK_MENU) & 0x8000))){
					if(next != NULL)
						manager->SwitchTo(next);
					else
						manager->AddTab(KaminoGlobal::g_start_url);
					return;
			}						
			//Switch To Prev Tab....
			if(((event.os_event.wParam == VK_MENU) && (GetAsyncKeyState(VK_LEFT) & 0x8000)) || 
				((event.os_event.wParam == VK_LEFT) && (GetAsyncKeyState(VK_MENU) & 0x8000))){
					if(prev != NULL)
						manager->SwitchTo(prev);
					return;
			}

			//Close Tab
			if(((event.os_event.wParam == VK_MENU) && (GetAsyncKeyState(0x58) & 0x8000)) || 
				((event.os_event.wParam == 0x58) && (GetAsyncKeyState(VK_MENU) & 0x8000))){
					manager->CloseTab(this, false);				
					return;
			}									

			//Home shortcut
			if(((event.os_event.wParam == VK_MENU) && (GetAsyncKeyState(0x31) & 0x8000)) || 
				((event.os_event.wParam == 0x31) && (GetAsyncKeyState(VK_MENU) & 0x8000))){
					manager->GoToUrl(KaminoGlobal::g_start_url);
					return;
			}									
			//GoogleShorcut
			if(((event.os_event.wParam == VK_MENU) && (GetAsyncKeyState(0x32) & 0x8000)) || 
				((event.os_event.wParam == 0x32) && (GetAsyncKeyState(VK_MENU) & 0x8000))){
					manager->GoToUrl("https://www.google.com");
					return;
			}			
			//facebookShortCut
			if(((event.os_event.wParam == VK_MENU) && (GetAsyncKeyState(0x33) & 0x8000)) || 
				((event.os_event.wParam == 0x33) && (GetAsyncKeyState(VK_MENU) & 0x8000))){
					manager->GoToUrl("https://www.facebook.com");
					return;
			}
			//YouTubeShortCut
			if(((event.os_event.wParam == VK_MENU) && (GetAsyncKeyState(0x34) & 0x8000)) || 
				((event.os_event.wParam == 0x34) && (GetAsyncKeyState(VK_MENU) & 0x8000))){
					manager->GoToUrl("https://www.youtube.com");
				return;
			}
			if(event.os_event.wParam == VK_MENU){
					manager->AddTab(KaminoGlobal::g_start_url);
				return;
			}
		}

		if(event.os_event.message == WM_KEYUP){
			//Zoom in
			if(((event.os_event.wParam == VK_CONTROL) && (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000)) || 
				((event.os_event.wParam == VK_OEM_PLUS) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))){								
				source->GetRenderViewHost()->Zoom(content::PAGE_ZOOM_IN);					
				return;
			}
			//Zoom Out
			if(((event.os_event.wParam == VK_CONTROL) && (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000)) || 
				((event.os_event.wParam == VK_OEM_MINUS) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))){				
				source->GetRenderViewHost()->Zoom(content::PAGE_ZOOM_OUT);					
				return;
			}
			//Zoom Reset
			if(((event.os_event.wParam == VK_CONTROL) && (GetAsyncKeyState(0x30) & 0x8000)) || 
				((event.os_event.wParam == 0x30) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))){				
				source->GetRenderViewHost()->Zoom(content::PAGE_ZOOM_RESET);					
				return;
			}
			
			//Search Shortcut
			if(((event.os_event.wParam == VK_CONTROL) && (GetAsyncKeyState(0x46) & 0x8000)) ||
				((event.os_event.wParam == 0x46) && (GetAsyncKeyState(VK_CONTROL) & 0x8000))){
					if(manager->find_window != NULL)
						manager->find_window->Show(source);
					else{
						manager->find_window = new KaminoFindWindow(manager->core_parent);
						manager->find_window->Show(source);
					}
					return;
			}			
		}
}

void KTab::KTabFaviconDownloadCallBack(int id, const GURL& url, bool errors, int size, 
	const std::vector<SkBitmap>& bitmaps){
	if(!errors){
		if(bitmaps.size() > 0){
			int size = 0;
			int index = 0;
			//Pick 32 X 32 bitmap or the largest;
			for(unsigned int i = 0; i < bitmaps.size(); i++){
				if(bitmaps[i].height() == 32 && bitmaps[i].width() == 32){
					index = i;
					break;
				}
				int new_size = bitmaps[i].height() * bitmaps[i].width();
				if(new_size > size){
					index = i;
					size = new_size;
				}
			}
			HRESULT hr =  E_FAIL;
			if(manager->tool_tip != NULL && manager->tool_tip->target != NULL)
				hr = favicon.Reset((ID2D1RenderTarget *)manager->tool_tip->target, (char *)bitmaps[index].getPixels(), bitmaps[index].width(),
					bitmaps[index].height(), true);
			else{
				UI_LOG(0, "Unable to Build Favicon Bitmap cause tooltip is NULL or tool_tip render target == NULL; HELP !!!!")
			}
			if(favicon.Get() == NULL){
				UI_LOG(0, "Unable to Build Favicon Bitmap favicon.get() == NULL; HELP !!!!")
			}
			if(SUCCEEDED(hr)){
				manager->Render();
			}
		}
	}
}

void KTab::Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details){  
	if(type == content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED){
		if(favicon.Get())
			favicon.Clean();
		if(web_content->GetURL().SchemeIsFile())
			web_content->DownloadFavicon(GURL("https://s3-us-west-2.amazonaws.com/www.voyjor.com/LogoPractice.ico"), 0, 
				base::Bind(&KTab::KTabFaviconDownloadCallBack, base::Unretained(this)));
		else
			web_content->DownloadFavicon(GURL("http://"+web_content->GetURL().host()+"/favicon.ico"), 0, 
				base::Bind(&KTab::KTabFaviconDownloadCallBack, base::Unretained(this)));
	}      
}


//-----------------------------------------------KTabManager--------------------------------------------------------
KTabManager::KTabManager(){
	main = NULL;
	current = NULL;
	back_button = NULL;
	add_tab_button = NULL;
	min_max_close_button = NULL;
	find_window = NULL;
	is_initialized = false;
	browser_context.reset(new content::KaminoBrowserContext());
	tool_tip = NULL;
	tab_window = NULL;
	tab_x_sum_limit = 0;
	KaminoGlobal::RegisterVoidPointerA(this);
}

KTabManager::~KTabManager(){
	is_initialized = false;
	CloseAllTabs();
	browser_context.reset(NULL);
	if(find_window)
		delete find_window;
	if(back_button != NULL)
		delete back_button;
	back_button = NULL;
	if(min_max_close_button != NULL)
		delete min_max_close_button;
	min_max_close_button = NULL;
	if(add_tab_button != NULL)
		delete add_tab_button;
	add_tab_button = NULL;
	if(tab_window != NULL)
		DestroyWindow(tab_window);
	tab_window = NULL;
	DestroyDeviceIndependantResource();
}

bool KTabManager::Initialize(HWND parent){
	wchar_t cur_dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cur_dir);
	GetClientRect(parent, &region);
	WNDCLASSEX wnd;
	core_parent = parent;
	wnd.cbClsExtra = 0;
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.cbWndExtra = sizeof(LONG_PTR);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wnd.hIconSm = wnd.hIcon;
	wnd.hInstance = GetModuleHandle(NULL);
	wnd.lpfnWndProc = TabManagerWndProc;
	wnd.lpszClassName = L"KTabManager";
	wnd.lpszMenuName = NULL;
	wnd.style = (CS_VREDRAW | CS_HREDRAW);
	RegisterClassEx(&wnd);
	tab_window = CreateWindowEx(NULL, wnd.lpszClassName, L"KTabManagerWorkSpace", WS_CHILD | WS_CLIPCHILDREN, region.left, region.top, region.right, TAB_CAPTION_WIDTH/*37*/, 
		parent, NULL, wnd.hInstance, this);
	if(SUCCEEDED(InitDeviceIndependantResource())){
		if(tab_window == NULL){
			return false;
		}
		is_initialized = true;
		ShowWindow(tab_window, SW_SHOW);//This is when it first Calls ( InitDeviceDependantResource() )
		UpdateWindow(tab_window);
		return true;
	}
	return false;
}

//Incharge(Creator) >> (direct2d_factory, write_factory >> Indirect thorgh KaminoDirectResource.h), tab_geometry, tab_border_line_geometry, tool_tip
HRESULT KTabManager::InitDeviceIndependantResource(){
	HRESULT hr = E_FAIL;
	if(SUCCEEDED(hr = KaminoDirectResourceInitialize())){
		hr = tab_geometry.Reset();
		hr = tab_border_line_geometry.Reset();
		if(SUCCEEDED(hr)){
			ID2D1GeometrySink *sink = NULL;
			D2D1_POINT_2F start = D2D1::Point2F(0.0f, 40.0f);
			D2D1_POINT_2F leftSideLowerBaring = D2D1::Point2F(5.0f, 40.0f);
			D2D1_POINT_2F leftSideHigherBaring = D2D1::Point2F(10.0f, -2.0f);
			D2D1_POINT_2F point1 = D2D1::Point2F(20.0f, 0.0f);
			D2D1_POINT_2F point2 = D2D1::Point2F(110.0f, 0.0f);
			D2D1_POINT_2F rightSideHigherBaring = D2D1::Point2F(120.0f, -2.0f);
			D2D1_POINT_2F rightSideLowerBaring = D2D1::Point2F(125.0f, 40.0f);
			D2D1_POINT_2F end = D2D1::Point2F(130.0f, 40.0f);

/*			D2D1_POINT_2F start = D2D1::Point2F(2.0f, 40.0f);//(5.0f, 33.0f);////done 0.0f, 35.0f
			D2D1_POINT_2F leftSideLowerBaring = D2D1::Point2F(6.5f, 25.25f);//(6.5f, 23.25f);//done 6.5f, 23.25f
			D2D1_POINT_2F leftSideHigherBaring = D2D1::Point2F(12.5f, -2.5f);//(12.5f, -2.5f);//(12.5f, -2.5f);//done 12.5f, -2.5f
			D2D1_POINT_2F point1 = D2D1::Point2F(20.0f, 0.0f);//(20.0f, 0.0f);//done 20.0f, 0.0f
			D2D1_POINT_2F point2 = D2D1::Point2F(110.0f, 0.0f);//(116.0f, 0.0f);//done 110.0f, 0.0f
			D2D1_POINT_2F rightSideHigherBaring = D2D1::Point2F(117.5f, -2.5f);//(122.5f, -2.5f);//done 117.5f, -2.5f
			D2D1_POINT_2F rightSideLowerBaring = D2D1::Point2F(123.5f, 25.25f);//(128.5f, 23.25f);//done 123.5f, 23.25f
			D2D1_POINT_2F end = D2D1::Point2F(128.0f, 40.0f);//(130.0f, 33.0f);//done 130.0f, 35.0f*/

			tab_geometry.Get()->Open(&sink);
			sink->BeginFigure(start, D2D1_FIGURE_BEGIN_FILLED);
			sink->AddBezier(D2D1::BezierSegment(leftSideLowerBaring, leftSideHigherBaring, point1));
			sink->AddLine(point2);
			sink->AddBezier(D2D1::BezierSegment(rightSideHigherBaring, rightSideLowerBaring, end));
			sink->AddLine(start);
			sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			hr = sink->Close();
			sink->Release();
		
			tab_border_line_geometry.Get()->Open(&sink);
			sink->BeginFigure(start, D2D1_FIGURE_BEGIN_HOLLOW);
			sink->AddBezier(D2D1::BezierSegment(leftSideLowerBaring, leftSideHigherBaring, point1));
			sink->EndFigure(D2D1_FIGURE_END_OPEN);
			hr = sink->Close();
			sink->Release();
			if(SUCCEEDED(hr)){
				if(SUCCEEDED(hr)){
					tool_tip = new KaminoLayeredToolTip(this);
					if(tool_tip == NULL)
						return E_FAIL;
					//Since The ToolTip is another window we initialize it hear. and not at InitDeviceDependantResource()
					hr = tool_tip->Initialize(core_parent);
				}
			}
		}
	}
	return hr;
}

void  KTabManager::DestroyDeviceIndependantResource(){
	if(tool_tip)
		delete tool_tip;
	tool_tip = NULL;
	KaminoDirectResourceDestory();
}

//InCharge(Creator) of, direct2d_render_target, active_tab_brush, inactive_tab_brush, tab_border_line_brush, tab_text_brush, 
//background_brush, and making sure all ktab struct's are perfectly device initialized.
HRESULT KTabManager::InitDeviceDependantResource(){
	HRESULT hr = S_OK;
	if(tab_window == NULL)
		return E_FAIL;
	if(direct2d_render_target.Get() == NULL){
		RECT rc;
		GetClientRect(tab_window, &rc);
		hr = direct2d_render_target.Reset(tab_window, rc);
		if(SUCCEEDED(hr)){
			direct2d_render_target.Get()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			direct2d_render_target.Get()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
		}
	}
	if((active_tab_brush.Get() == NULL) && SUCCEEDED(hr)){
		hr = active_tab_brush.Reset(0.909f, 0.360f, 0.000f, 1.0f, direct2d_render_target.Get());
		//hr = active_tab_brush.Reset(0.939f, 0.939f, 0.939f, 0.5f, direct2d_render_target.Get());
	}	
	if((inactive_tab_brush.Get() == NULL) && SUCCEEDED(hr)){
		hr  = inactive_tab_brush.Reset(0.176f, 0.176f, 0.176f, 1.0f, direct2d_render_target.Get());
		//hr = inactive_tab_brush.Reset(0.939f, 0.939f, 0.939f, 0.5f, direct2d_render_target.Get());
	}	
	if((tab_border_line_brush.Get() == NULL) && SUCCEEDED(hr)){
		//hr = tab_border_line_brush.Reset(0.45972f, 0.45972f, 0.45972f, 0.4463f, direct2d_render_target.Get());
		hr = tab_border_line_brush.Reset(0.3515f, 0.3515f, 0.3515f, 0.36f, direct2d_render_target.Get());
	}
	if((tab_border_line_brush3.Get() == NULL) && SUCCEEDED(hr)){
		hr = tab_border_line_brush3.Reset(0.666f, 0.666f, 0.666f, 0.45f, direct2d_render_target.Get());
	}
	if((tab_border_line_brush2.Get() == NULL) && SUCCEEDED(hr)){
		//hr = tab_border_line_brush2.Reset(0.45972f, 0.45972f, 0.45972f, 0.4463f, direct2d_render_target.Get());
		hr = tab_border_line_brush2.Reset(0.3515f, 0.3515f, 0.3515f, 0.36f, direct2d_render_target.Get());
	}
	if((tab_text_brush.Get() == NULL) && SUCCEEDED(hr)){
		hr = tab_text_brush.Reset(1.0f, 1.0f, 1.0f, 1.0f, direct2d_render_target.Get());
	}
	if((background_brush.Get() == NULL) && SUCCEEDED(hr)){
		background_brush.Reset(1.00f, 1.00f, 1.00f, 1.0f, direct2d_render_target.Get());
		//hr = background_brush.Reset(1.0f, 1.0f, 1.0f, 1.0f, direct2d_render_target.Get());
		/*D2D1_GRADIENT_STOP gradientStops[2];
		gradientStops[0].color = D2D1::ColorF(0.45972f, 0.45972f, 0.45972f, 1.0f);
		gradientStops[0].position = 0.0f;
		gradientStops[1].color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
		gradientStops[1].position = 1.0f;
		 hr = background_brush.Reset(gradientStops, 2,
			&D2D1::Point2F(static_cast<FLOAT>((region.right - region.left)/2), static_cast<FLOAT>((region.bottom - region.top)/2)), 
			(static_cast<FLOAT>(region.right - region.left)*2.0), 
			(static_cast<FLOAT>(region.bottom - region.top)/1.95f),direct2d_render_target.Get());*/
	}
	if((background_brush2.Get() == NULL) && SUCCEEDED(hr)){
		hr = background_brush2.Reset(1.00f, 1.00f, 1.00f, 0.80f, direct2d_render_target.Get());	
		/*D2D1_GRADIENT_STOP gradientStops[2];
		gradientStops[0].color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
		gradientStops[0].position = 0.0f;
		gradientStops[1].color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
		gradientStops[1].position = 1.0f;
		 hr = background_brush2.Reset(gradientStops, 2,
			&D2D1::Point2F(static_cast<FLOAT>((region.right - region.left)/2), static_cast<FLOAT>((region.bottom - region.top)/2)), 
			(static_cast<FLOAT>(region.right - region.left)*2.0), 
			(static_cast<FLOAT>(region.bottom - region.top)/1.95f),direct2d_render_target.Get());*/

	}
	if((tab_close_button_brush.Get() == NULL) && (SUCCEEDED(hr))){
		hr = tab_close_button_brush.Reset(1.0f, 1.0f, 1.0f, 0.6f, direct2d_render_target.Get());
	}
	if(KaminoGlobal::g_is_incognito_mode)
		if((incognito_image.Get() == NULL) && SUCCEEDED(hr))
			hr = incognito_image.Reset(direct2d_render_target.Get(), L"../KaminoResource/otr_icon.png");
	//Initialize the BackButton, since its a part of the tab_window we Initialize it Hear rather than DeviceIndependantResource
	if(SUCCEEDED(hr)){ 
		if(back_button == NULL){
			back_button = new KBackButton(this, 0, 60);
		}
		hr = back_button->InitializeDeviceDependantResource(direct2d_render_target.Get());
		if(FAILED(hr) && back_button != NULL){
			delete back_button;
			back_button = NULL;
		}
	}
	//Initialize AddTabButton.
	if(SUCCEEDED(hr)){
		if(add_tab_button == NULL){
			add_tab_button = new KAddTabButton(this);
		}
		hr = add_tab_button->InitializeDeviceDependantResource(direct2d_render_target.Get());
		if(FAILED(hr) && add_tab_button != NULL){
			delete add_tab_button;
			add_tab_button = NULL;
		}
	}
	//Initialize MinMax Button.
	if(SUCCEEDED(hr)){
		if(min_max_close_button == NULL)
			min_max_close_button = new KMinMaxCloseButton(this);
		hr = min_max_close_button->InitializeDeviceDependantResource(direct2d_render_target.Get());
		if(FAILED(hr) && min_max_close_button != NULL){
			delete min_max_close_button;
			min_max_close_button = NULL;
		}
	}
	if(SUCCEEDED(hr))
		for(KTab *temp = main; temp != NULL; temp = temp->next)
			if(!SUCCEEDED(temp->InitDeviceDependantResource(temp,  direct2d_render_target.Get()))){
				return E_FAIL;
			}
	return hr;
}

void KTabManager::DestroyDeiviceDependantResource(){
	direct2d_render_target.Clean();
	active_tab_brush.Clean();
	inactive_tab_brush.Clean();
	tab_border_line_brush.Clean();
	tab_border_line_brush2.Clean();
	tab_border_line_brush3.Clean();
	tab_text_brush.Clean();
	background_brush.Clean();
	background_brush2.Clean();
	tab_close_button_brush.Clean();
	incognito_image.Clean();
	if(back_button)
		delete back_button;
	back_button = NULL;
	if(min_max_close_button)
		delete min_max_close_button;
	min_max_close_button = NULL;
	if(add_tab_button)
		delete add_tab_button;
	add_tab_button = NULL;
	for(KTab *temp = main; temp != NULL; temp = temp->next)
		temp->DestoryDeviceDependantResource(temp);
}

bool KTabManager::AddTab(GURL *url){
	content::WebContents::CreateParams param(browser_context.get());
	AddTab(content::WebContents::Create(param));
	GoToUrl(url);
	return true;
}

bool KTabManager::AddTab(char *url){
	content::WebContents::CreateParams param(browser_context.get());
	AddTab(content::WebContents::Create(param));
	GoToUrl(url);
	return true;
}

void UpdateProcessExecutor(std::string version){
	UI_LOG(0, "Thinking of launching Update Process Got String %s, Mother Fucker", version.c_str())
	if((atoi(version.c_str()) > KaminoGlobal::g_current_version_int) && 
		(FindWindow(L"KaminoUpdateProcess", L"KaminoUpdateProcessWindow") == NULL)){
			UI_LOG(0,"Launching Update Process")
			STARTUPINFO info = {sizeof(info)};
			PROCESS_INFORMATION processInfo;
			CreateProcess(KaminoGlobal::g_executable_name, L" -update", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
	}
	else
		printf("\n Not Necessary For Update Process Execution");
}

//Builds the Basic KTabStruct, should move this to KTab's constructor.
bool KTabManager::AddTab(content::WebContents *new_content){
	static bool checked_for_update = false;
	if(!checked_for_update && (!KaminoGlobal::g_is_test_run) && (!KaminoGlobal::g_is_incognito_mode)){
		KaminoUpdateStatus *should_update = new KaminoUpdateStatus(new_content->GetBrowserContext()->GetRequestContext());
		should_update->Execute(base::Bind(UpdateProcessExecutor));
		checked_for_update = true;
	}
	KTab* tab = new KTab();
	tab->top_pos = 40;
	tab->manager = this;
	tab->next = NULL;
	tab->OnClick = KTabDefaultOnClick;
	tab->OnPaint = KTabDefaultOnPaintDirect2DEvent;
	tab->InitDeviceDependantResource = KTabDefaultInitializeDeviceDependantResource;
	tab->DestoryDeviceDependantResource = KTabDefaultDestrorDeviceDependantResource;
	tab->web_content.reset(new_content);
	tab->should_repaint = true;
	tab->is_mouse_over = false;
	tab->is_tittle_set = false;
	tab->draged_left_pos = -1;
	tab->is_window_active = false;
	tab->registrar.Add(tab, content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED, 
		content::Source<content::WebContents>(tab->web_content.get()));
	KTab* temp = NULL;
	if(main == NULL){
		main = tab;
	}
	else{
		for(temp = main; (temp->next != NULL); temp = temp->next);
		temp->next = tab;	
	}
	tab->prev = temp;
	if((!tab->web_content.get()) || (tab_window == NULL)){
		temp->next = NULL;
		tab->web_content.reset(NULL);
		delete tab;
		return false;
	}
	tab_x_sum_limit += TAB_WIDTH; //112 is tab width;
	tab->web_content->SetDelegate(tab);
	SetParent(tab->web_content->GetNativeView(), core_parent);
	SwitchTo(tab);
	return true;
}


bool KTabManager::AddTabNextTo(KTab *prev_tab, content::WebContents *new_content){	
	KTab* tab = new KTab();
	tab->top_pos = 40;
	tab->manager = this;
	tab->next = NULL;
	tab->OnClick = KTabDefaultOnClick;
	tab->OnPaint = KTabDefaultOnPaintDirect2DEvent;
	tab->InitDeviceDependantResource = KTabDefaultInitializeDeviceDependantResource;
	tab->DestoryDeviceDependantResource = KTabDefaultDestrorDeviceDependantResource;
	tab->web_content.reset(new_content);
	tab->should_repaint = true;
	tab->is_mouse_over = false;
	tab->is_tittle_set = false;
	tab->draged_left_pos = -1;
	tab->is_window_active = false;
	tab->registrar.Add(tab, content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED, 
		content::Source<content::WebContents>(tab->web_content.get()));
	if(prev_tab->next == NULL){
		prev_tab->next = tab;
		tab->next = NULL;
		tab->prev = prev_tab;
	}
	else{
		KTab *next_tab = prev_tab->next;
		prev_tab->next = tab;
		tab->next = next_tab;
		tab->prev = prev_tab;
		next_tab->prev = tab;
	}
	tab_x_sum_limit += TAB_WIDTH; //112 is tab width;
	tab->web_content->SetDelegate(tab);
	SetParent(tab->web_content->GetNativeView(), core_parent);
	SwitchTo(tab);
	return true;
}


void KTabManager::CloseTab(KTab *tab, bool closing_all_tabs){
	//Flush To Recently Closed.
	if((tab->web_content->GetURL() != GURL(KaminoGlobal::g_start_url)) && (!KaminoGlobal::g_is_incognito_mode)) //dont flush it if its the home page
		KaminoHistoryModule::push(tab->web_content->GetURL().spec(), tab->web_content->GetTitle());
	if(current == tab)
		if(tab->next != NULL){
			current = tab->next;
		}
		else
			if(tab->prev != NULL)
				current = tab->prev;
			else
				current = NULL;
	if(tab->next != NULL  && !closing_all_tabs)
		tab->next->is_mouse_over = true;
	if(tab->web_content.get()){
		tab->web_content->Close();
		tab->web_content.reset(NULL);
	}
	//Appropirately Remove the linked structure(KTab).
	if((tab->prev == NULL) && (tab->next == NULL)){
			main = NULL;
			PostMessage(core_parent, WM_CLOSE, NULL, NULL);
	}
	if((tab->prev != NULL) && (tab->next == NULL)){
		tab->prev->next = NULL;		
	}
	if((tab->prev == NULL) && (tab->next != NULL)){
			main = tab->next;
			tab->next->prev = NULL;
	}
	if((tab->prev != NULL) && (tab->next != NULL)){
		tab->prev->next = tab->next;
		tab->next->prev = tab->prev;
	}
	tab_x_sum_limit -= 112;
	delete tab;
	if(!closing_all_tabs) //donot render if closing all tabs
		Render();
}

//Destroys all KTab Objects.
void KTabManager::CloseAllTabs(){
	KTab* temp = main;
	while(temp != NULL){
		if(temp->next != NULL){
			temp = temp->next;
			CloseTab(temp->prev, true);
		}
		else{
			CloseTab(temp, true);
			break;
		}
	}
	main = NULL;
	current = NULL;
}

void KTabManager::SwitchTo(KTab *tab){
	if(current == tab)
		return;
	if((find_window != NULL) && find_window->IsVisible())
		find_window->Hide();
	if((current != NULL) && (current->web_content.get())){
		ShowWindow(current->web_content->GetNativeView(), SW_HIDE);
		current->web_content->WasHidden();
		current->is_window_active = false;
	}
	current = tab;
	Render();
}

//Indirect WndProc Recives Messagess From parent_window ( BasicUIMain );
void KTabManager::RecivedHwndMessage(UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_PAINT:{
			Render();
			break;
		}
		case WM_SIZE:{
			GetClientRect(core_parent, &region);
			MoveWindow(tab_window, 0, 0, region.right - region.left, TAB_CAPTION_WIDTH, TRUE);
			//Currently we only have one, size so calling move window, would be a waste.
			//XX//let the The Tab Window Handle the resizing of current's web_content window;
			if(current){
				SetWindowPos(current->web_content->GetNativeView(), NULL, 1, TAB_CAPTION_WIDTH, region.right-2, region.bottom - TAB_CAPTION_WIDTH, SWP_NOREDRAW);
				current->web_content->Focus();
			}
			break;
		}
	}
}
LRESULT CALLBACK KTabManager::TabManagerWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static bool mouse_entry = false;
	static int prev_grid = -1; //used by mouse move
	static KTab *selected_tab = NULL; //For Draging
	static int drag_offset = 0;
	KTabManager* myself = NULL;
	if(msg == WM_CREATE){
		CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
		myself = (KTabManager *)pcs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(myself));
		return 0;
	}
	myself = reinterpret_cast<KTabManager*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
	if(myself == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);
	switch(msg){
		case WM_NCHITTEST:{
			RECT rc;
			GetWindowRect(myself->core_parent, &rc);
			if(selected_tab != NULL)
				return HTCLIENT;
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			x -= rc.left;
			if((x <= (myself->tab_x_sum_limit + TAB_START_X_OFFSET + 100)) ||
				(x >= static_cast<int>(static_cast<FLOAT>(myself->region.right) * 0.934f)) || KaminoGlobal::g_is_window_fullscreen){
				return HTCLIENT;
			}
			return HTTRANSPARENT;			
		}
		case WM_SIZE:{
			D2D1_SIZE_U size = D2D1::SizeU(myself->region.right, TAB_CAPTION_WIDTH);
			((ID2D1HwndRenderTarget *)myself->direct2d_render_target.Get())->Resize(size);
			return 0;	
		}
		case WM_PAINT:{
			myself->Render();
			ValidateRect(hWnd, NULL);
			return 0;
		}
		case WM_LBUTTONDOWN:{
			selected_tab = NULL;
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			for(KTab *temp = myself->main; temp != NULL; temp = temp->next)
				if((x <= temp->left_pos + TAB_WIDTH) && (x >= temp->left_pos)){
					selected_tab = temp;
					for(KTab *temp = myself->main; temp != NULL; temp = temp->next)
						temp->draged_left_pos = -1;
				}
			drag_offset = (x - TAB_START_X_OFFSET) % TAB_WIDTH;
			return 0;
		}
		case WM_LBUTTONUP:{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			if(selected_tab != NULL && selected_tab->draged_left_pos != -1 && (wParam != MK_CONTROL)){
				for(KTab *temp = myself->main; temp != NULL; temp = temp->next)
					temp->draged_left_pos = -1;
				selected_tab = NULL;
				myself->Render();
				return 0;
			}
			//Check if BackButton is Clicked.
			if((x <= (TAB_START_X_OFFSET -  25)))
				if(myself->back_button != NULL){
					myself->back_button->OnClick();
					return 0;
				}
			if(myself->min_max_close_button->IfClicked(x))
				return 0;
			if(myself->add_tab_button->IfClicked(x))
				return 0;
			if(selected_tab != NULL){
				selected_tab->draged_left_pos = -1;
				selected_tab->OnClick(selected_tab , x, y);
				selected_tab = NULL;
			}
			return 0;
		}
		case WM_RBUTTONUP :{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			for(KTab *temp = myself->main; temp != NULL; temp = temp->next)
				if((x >= temp->left_pos) && (x <= temp->left_pos + TAB_WIDTH)){
					temp->web_content->GetController().LoadURL(GURL(KaminoGlobal::g_start_url),content::Referrer(), 
						content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR), 
						std::string());
					break;
				}
			return 0;
		}
		case WM_MOUSEMOVE:{
			if((mouse_entry == false) && (wParam != MK_CONTROL)){
				mouse_entry = true;
				TRACKMOUSEEVENT evt;
				evt.cbSize = sizeof(TRACKMOUSEEVENT);
				evt.dwFlags = (TME_LEAVE);
				evt.dwHoverTime = 400;
				evt.hwndTrack = hWnd;
				TrackMouseEvent(&evt);
				return 0;
			}
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			if(selected_tab != NULL){
				int draged_right_pos = 0;
				selected_tab->draged_left_pos = x - drag_offset;
				draged_right_pos = selected_tab->draged_left_pos + TAB_WIDTH;
				if(selected_tab->next != NULL){
					KTab *next = selected_tab->next;
					if((next->left_pos > selected_tab->draged_left_pos) && (next->left_pos < draged_right_pos)){
						next->draged_left_pos = next->left_pos - (draged_right_pos - next->left_pos);
						if(selected_tab->draged_left_pos > next->draged_left_pos){
							myself->KTabSwap(selected_tab, next);	
						}
						myself->Render();
						return 0;
					}	
				}
				if(selected_tab->prev != NULL){
					KTab *prev = selected_tab->prev;
					if(((prev->left_pos + TAB_WIDTH) > selected_tab->draged_left_pos) && ((prev->left_pos + TAB_WIDTH) < draged_right_pos)){
						prev->draged_left_pos = prev->left_pos + ((prev->left_pos + TAB_WIDTH) - selected_tab->draged_left_pos);
						if(prev->draged_left_pos > selected_tab->draged_left_pos){
							myself->KTabSwap(prev, selected_tab);
						}
						myself->Render();
						return 0;
					}				
				}
				myself->Render();
				return 0;
			}
			bool should_render_again_1 = false;
			bool should_render_again_2 = false;
			if(myself->min_max_close_button->IfMouseIsHoveringOver(x, should_render_again_1)){
				if(should_render_again_1){
					for(KTab *temp = myself->main; temp != NULL; temp = temp->next)
						temp->is_mouse_over = false;
					myself->Render();
				}
				return 0;
			}
			if(myself->add_tab_button->IfMouseIsHoveringOver(x, should_render_again_2)){
				if(should_render_again_2){
					for(KTab *temp = myself->main; temp != NULL; temp = temp->next)
						temp->is_mouse_over = false;
					myself->Render();
				}
				return 0;
			}
			if((x >= (myself->tab_x_sum_limit + TAB_START_X_OFFSET)) || (x <= TAB_START_X_OFFSET)){
				myself->tool_tip->Hide();
				prev_grid = -1;
				for(KTab *temp = myself->main; temp != NULL; temp = temp->next)
					temp->is_mouse_over = false;
				if(should_render_again_1 || should_render_again_2)
					myself->Render();
				return 0;
			}
			int temp_grid = (x - TAB_START_X_OFFSET) / TAB_WIDTH;
			if(prev_grid == temp_grid)
				return 0;
			prev_grid = temp_grid;
			KTab *tab = myself->main;
			for(KTab *temp_tab = tab; temp_tab != NULL; temp_tab = temp_tab->next)
				temp_tab->is_mouse_over = false;
			for(int i = 0; (tab != NULL) && (i < temp_grid); tab = tab->next, i++);
			if(tab == NULL){
				prev_grid = -1;
				if(should_render_again_1 || should_render_again_2)
					myself->Render();
				return 0;
			}
			tab->is_mouse_over = true;
			myself->Render();
			if((myself->tool_tip != NULL) && (wParam != MK_CONTROL))
				myself->tool_tip->RenderText(tab->current_url.c_str(), tab->current_url.length(), tab);			
			return 0;
		}
		case WM_MOUSELEAVE:{
			selected_tab = NULL;
			mouse_entry = false;
			prev_grid = -1;
			myself->tool_tip->Hide();
			myself->min_max_close_button->MouseOut();
			myself->add_tab_button->MouseOut();
			for(KTab *temp = myself->main; temp != NULL; temp = temp->next){
				temp->is_mouse_over = false;
				temp->draged_left_pos = -1;
			}
			if(myself->current)
				myself->current->web_content->Focus();
			myself->Render();
			return 0;
		}
		default:{
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}
}

//Methods called by KtabCallbacks for painting, should make enable late DeviceDependant Resource Creation if needed.
ID2D1PathGeometry *KTabManager::GiveMeTabGeometry(){
	return tab_geometry.Get();
}

ID2D1PathGeometry *KTabManager::GiveMeTabBorderLineGeometry(){
	return tab_border_line_geometry.Get();
}

ID2D1Brush *KTabManager::GiveMeTabBorderLineBrush(){
	return tab_border_line_brush.Get();
}

ID2D1Brush *KTabManager::GiveMeTabBorderLineBrush2(){
	return tab_border_line_brush2.Get();
}

ID2D1Brush *KTabManager::GiveMeBackgroundBrush(KTab *tab){
	if(tab == current)
		return active_tab_brush.Get();
	return inactive_tab_brush.Get();
}

ID2D1Brush *KTabManager::GiveMeTextBrush(){
	return tab_text_brush.Get();
}

ID2D1Brush *KTabManager::GiveMeTabCloseBrush(){
	return tab_close_button_brush.Get();
}

void KTabManager::KTabSwap(KTab *first, KTab *second){
	if(main == first)
		main = second;
	KTab *front = first->prev;
	KTab *end = second->next;
	first->next = end;
	first->prev = second;
	second->prev = front;
	second->next = first;
	if(front != NULL)
		front->next = second;
	if(end != NULL)
		end->prev = first;
}

//GoBack for current's web_content.
void KTabManager::GoBack(){
	if((current != NULL) && (current->web_content.get()))
		if(current->web_content->GetController().CanGoBack()){
				current->web_content->GetController().GoBack();
				current->web_content->Focus();
		}
}
//GoForward for current's web_content.
void KTabManager::GoForward(){
	if((current != NULL) && (current->web_content.get()))
		if(current->web_content->GetController().CanGoForward()){
			current->web_content->GetController().GoForward();
			current->web_content->Focus();
		}
}
//FOCUS
void KTabManager::Focus(){
	if(current != NULL){
		current->web_content->Focus();
		SetFocus(current->web_content->GetNativeView());
	}
}

//GoToUrl for current's web_content.
void KTabManager::GoToUrl(char *url){
	if((current != NULL) && (current->web_content.get())){
		current->web_content->GetController().LoadURL(GURL(std::string(url)), content::Referrer(), 
			content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR), std::string());
		current->web_content->Focus();
	}
}

void KTabManager::GoToUrl(GURL *url){
	if((current != NULL) && (current->web_content.get())){
		current->web_content->GetController().LoadURL((*url), content::Referrer(), 
			content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR), std::string());
	}	
}
//Reload for current's web_content.
void KTabManager::Reload(){
	if((current != NULL) && (current->web_content.get())){
		current->web_content->GetController().Reload(false);
		current->web_content->Focus();
	}	
}
int KTabManager::GetTabXSumLimit(){
	return tab_x_sum_limit + 137;
}

void KTabManager::Render(){
	UI_LOG(0, "Running Rendering Loop")
	if((current != NULL) && (current->web_content.get())){
		RECT rc;
		if(!current->is_window_active){
			ShowWindow(current->web_content->GetNativeView(), SW_SHOW);
			current->is_window_active = true;
			current->web_content->WasShown();
		}
		GetWindowRect(current->web_content->GetNativeView(), &rc);
		if((rc.top != TAB_CAPTION_WIDTH) || (rc.left != 1) || ((rc.right - rc.left) != ((region.right - region.left)-2)) ||
			(((rc.bottom - rc.top)+TAB_CAPTION_WIDTH+1) != (region.bottom - region.top))){
			MoveWindow(current->web_content->GetNativeView(), 1, TAB_CAPTION_WIDTH , region.right-2, region.bottom - TAB_CAPTION_WIDTH - 1, TRUE);
		}
		current->web_content->Focus();
	}
	if(SUCCEEDED(InitDeviceDependantResource())){
		timer.Start();
		while(1){
			bool should_break = true; //This shit is included to have the animation effect of add tab below up thing.
			direct2d_render_target.Get()->BeginDraw();
			direct2d_render_target.Get()->SetTransform(D2D1::Matrix3x2F::Identity());
			direct2d_render_target.Get()->FillRectangle(D2D1::RectF(0.0f, 0.0f, static_cast<FLOAT>(region.right - region.left), 40.0f), background_brush.Get());
			if(back_button != NULL){
				back_button->Render(direct2d_render_target.Get());
			}
			if(min_max_close_button != NULL){
				min_max_close_button->Render(direct2d_render_target.Get());
			}
			if(KaminoGlobal::g_is_incognito_mode){			
				direct2d_render_target.Get()->DrawBitmap(incognito_image.Get(), D2D1::RectF(region.right - 115.0f, 5.0f,  region.right - 75.0f, 35.0f), 1.0f);
			}
			//Render All tabs except Current.
			for(KTab *temp = main; temp != NULL; temp = temp->next){ 
				temp->left_pos = (temp->prev) ? temp->prev->left_pos + TAB_WIDTH : TAB_START_X_OFFSET;
				if((temp != current)){
					temp->OnPaint(temp, reinterpret_cast<ID2D1HwndRenderTarget *>(direct2d_render_target.Get()));
				}
				if(temp->top_pos > 2)
					should_break = false;
				else
					temp->top_pos = 2;
			}
			if(current != NULL){
				current->OnPaint(current, reinterpret_cast<ID2D1HwndRenderTarget *>(direct2d_render_target.Get()));
				if(current->top_pos > 2)
					should_break = false;
				else
					current->top_pos = 2;
			}
			if(add_tab_button != NULL){
				add_tab_button->Render(direct2d_render_target.Get());
			}
			direct2d_render_target.Get()->DrawLine(D2D1::Point2F(0.0f, static_cast<FLOAT>(TAB_CAPTION_WIDTH - 4)), D2D1::Point2F(static_cast<FLOAT>(region.right), 
				static_cast<FLOAT>(TAB_CAPTION_WIDTH - 4)), tab_border_line_brush3.Get(), 2.0f);
			direct2d_render_target.Get()->FillRectangle(
				D2D1::RectF(0.0f, static_cast<FLOAT>(TAB_CAPTION_WIDTH - 4), static_cast<FLOAT>(region.right), static_cast<FLOAT>(TAB_CAPTION_WIDTH)), 
				background_brush2.Get()
			); 
			direct2d_render_target.Get()->DrawLine(D2D1::Point2F(0.0f, static_cast<FLOAT>(TAB_CAPTION_WIDTH)), D2D1::Point2F(static_cast<FLOAT>(region.right), 
				static_cast<FLOAT>(TAB_CAPTION_WIDTH)), tab_border_line_brush3.Get(), 2.0f);
			direct2d_render_target.Get()->DrawLine(D2D1::Point2F(0.0f, 0.0f), D2D1::Point2F(0.0f, static_cast<FLOAT>(TAB_CAPTION_WIDTH)), 
				tab_border_line_brush2.Get(), 2.0f);
			direct2d_render_target.Get()->DrawLine(D2D1::Point2F(static_cast<FLOAT>(region.right), 0.0f), D2D1::Point2F(static_cast<FLOAT>(region.right), static_cast<FLOAT>(TAB_CAPTION_WIDTH)), 
				tab_border_line_brush2.Get(), 2.0f);
			HRESULT hr = direct2d_render_target.Get()->EndDraw();
			if(hr == D2DERR_RECREATE_TARGET || FAILED(hr)){
				DestroyDeiviceDependantResource();
				InitDeviceDependantResource();
			}
			if(should_break)
				break;
		}

		return;
	}
}
