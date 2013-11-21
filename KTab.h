// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KTAB_H
#define KTAB_H
#include "UIMain.h"
#include "KaminoBrowserContext.h"
#include "KaminoLayeredToolTip.h"
#include "KButton.h"
#include "KaminoDirectResource.h"
#include "KaminoWaitWindow.h"
#include "KaminoFindWindow.h"
#include "KaminoSimpleTimer.h"
#include "base\memory\ref_counted.h"
#include "base\memory\scoped_ptr.h"
#include "content\public\browser\web_contents.h"
#include "content\public\browser\web_contents_delegate.h"
#include "content\public\browser\notification_registrar.h"
#include "content\public\browser\notification_observer.h"

#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <Wincodec.h>

class KTabManager;
class KaminoLayeredToolTip;
class KBackButton;
class KAddTabButton;
class KLikeButton;
class KMinMaxCloseButton;
class KTab;
class KaminoWaitWindow;
class KaminoFindWindow;

//The main tab manager, in charge of various tabs, and the main tab_window.
class KTabManager{
public:
	KTabManager();
	~KTabManager();
	bool Initialize(HWND);
	bool AddTab(GURL *);
	bool AddTab(char *);
	bool AddTab(content::WebContents *);
	bool AddTabNextTo(KTab *tab, content::WebContents *);
	void CloseTab(KTab *, bool);
	void GoBack();
	void GoForward();
	void Focus();
	void GoToUrl(char *);
	void GoToUrl(GURL *);
	void SwitchTo(KTab *);
	void Reload();
	void Render();
	void CloseAllTabs();
	void RecivedHwndMessage(UINT, WPARAM, LPARAM);
	
	//Called By KTab Callbacks. Mostly OnPaint
	ID2D1PathGeometry *GiveMeTabGeometry();
	ID2D1PathGeometry *GiveMeTabBorderLineGeometry();
	ID2D1Brush *GiveMeTabBorderLineBrush();
	ID2D1Brush *GiveMeTabBorderLineBrush2();
	ID2D1Brush *GiveMeBackgroundBrush(KTab *);
	ID2D1Brush *GiveMeTextBrush();
	ID2D1Brush *GiveMeTabCloseBrush();

	KTab* main;
	KTab* current;
	KBackButton *back_button;
	KAddTabButton *add_tab_button;
	KMinMaxCloseButton *min_max_close_button;
	KaminoLayeredToolTip *tool_tip;
	KaminoFindWindow *find_window;

	bool is_initialized;
	HWND tab_window;

	RECT region;
	HWND core_parent;
	scoped_ptr<content::KaminoBrowserContext> browser_context;
	static LRESULT CALLBACK TabManagerWndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK TabManagerWndProcStub(HWND, UINT, WPARAM, LPARAM, KTabManager *myself);
	int GetTabXSumLimit();
//private:
	KaminoDirect2DFactory direct2d_factory;
	KaminoWriteFactory write_factory;
	KaminoWicImagingFactory wic_factory;
	KaminoDirect2DRenderTarget direct2d_render_target;
	KaminoDirect2DPathGeometry tab_geometry;
	KaminoDirect2DPathGeometry  tab_border_line_geometry;
	KaminoDirect2DBrush active_tab_brush;
	KaminoDirect2DBrush inactive_tab_brush;
	KaminoDirect2DBrush tab_border_line_brush;
	KaminoDirect2DBrush tab_border_line_brush2;
	KaminoDirect2DBrush tab_border_line_brush3;
	KaminoDirect2DBrush tab_text_brush;
	KaminoDirect2DBrush background_brush;
	KaminoDirect2DBrush background_brush2;
	KaminoDirect2DBrush tab_close_button_brush;
	KaminoDirect2DBitmap incognito_image;
	KaminoTimer timer;
	//Temp
	KaminoTextFormat tooltip_text_format;
	int tab_x_sum_limit; //this is the sum of all widget length;
	HRESULT InitDeviceIndependantResource();
	HRESULT InitDeviceDependantResource();
	void DestroyDeviceIndependantResource();
	void DestroyDeiviceDependantResource(); //called only when we get D2D RECREATE_ HR signal.
	void KTabSwap(KTab *first, KTab *second); //swap the tabs in the linked list
};

//This Open-Structure represents a basic tab widget.
class KTab : public content::WebContentsDelegate,
public content::NotificationObserver{
public:
	KTab(){ }; //does'nt do anything //should be fixed.
	~KTab(){ };// does'nt do anything // should be fixed.
	//Pointers to critical function for runtime modification.
	void (*OnPaint)(KTab*, ID2D1HwndRenderTarget *);
	void (*OnClick)(KTab*, int, int);
	HRESULT (*InitDeviceDependantResource)(KTab *, ID2D1RenderTarget *);
	void (*DestoryDeviceDependantResource)(KTab *);
	//Maybe should be a pointer
	HRESULT BuildTextGeometry();

	//Favicon Download Callback;
	void KTabFaviconDownloadCallBack(int id, const GURL& url, bool errors, int size,
                              const std::vector<SkBitmap>& bitmaps);


	//WebContentDelegate Methods
	virtual void LoadingStateChanged(content::WebContents* source) OVERRIDE;
	virtual content::WebContents* OpenURLFromTab(content::WebContents* source, const content::OpenURLParams& params) OVERRIDE;
	virtual void WebContentsCreated(content::WebContents* source_contents, int64 source_frame_id, const GURL& target_url, 
									content::WebContents* new_contents) OVERRIDE;
	virtual void DidNavigateMainFramePostCommit(content::WebContents* source) OVERRIDE;
	virtual void HandleKeyboardEvent(content::WebContents* source,
                                   const content::NativeWebKeyboardEvent& event) OVERRIDE;

	//Notification Observer for SetTittle
	void Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details) OVERRIDE;


	int left_pos;
	int top_pos;
	bool is_mouse_over;
	bool should_repaint;// flag to set, to rebuild the text_geometry and a new text_bitmap.
	bool is_tittle_set;
	bool is_window_active;
	int draged_left_pos;
	KTabManager *manager;
	KTab* prev;
	KTab* next;
	scoped_ptr<content::WebContents> web_content;		
	std::string current_url;
	KaminoDirect2DPathGeometry text_geometry;
	KaminoDirect2DRenderTarget bitmap_render_target;
	KaminoDirect2DBitmap text_bitmap;
	KaminoDirect2DBitmap favicon;
	content::NotificationRegistrar registrar;
};


#endif