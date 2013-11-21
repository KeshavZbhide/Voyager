// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoDownloadManagerDelegate.h"
#include "base\bind.h"
#include "base\strings\string_util.h"
#include "base\files\file_path.h"
#include "base\file_util.h"
#include "base\path_service.h"
#include "content\public\browser\browser_thread.h"
#include "net\base\net_util.h"
#include "content\public\browser\download_item.h"
#include "content\public\browser\web_contents.h"
#include "UIMain.h"
#include <Windows.h>
#include <Commdlg.h>
#include "KaminoGlobal.h"
#include <tchar.h>

namespace content{
	bool KaminoDownloadItemObserver::is_class_registerd = false;
	KaminoDownloadItemObserver::KaminoDownloadItemObserver(){
		if(!is_class_registerd){
			wnd_cls.cbClsExtra = 0;
			wnd_cls.cbSize = sizeof(wnd_cls);
			wnd_cls.cbWndExtra = 0;
			wnd_cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wnd_cls.hCursor = LoadCursor(NULL, IDC_ARROW);
			wnd_cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			wnd_cls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
			wnd_cls.hInstance = GetModuleHandle(NULL);
			wnd_cls.lpfnWndProc  = KaminoDownloadItemObserver::WndProc;
			wnd_cls.lpszClassName  = L"DownloadWindow";
			wnd_cls.lpszMenuName = NULL;
			wnd_cls.style = CS_VREDRAW | CS_HREDRAW;
			RegisterClassEx(&wnd_cls);
			is_class_registerd = true;
		}
		download_item = NULL;		
		percent_complete[0]  = 0;
		percent_complete[1]  = 0;
		percent_complete[2]  = 0;
		percent_complete[3]  = 0;
		percent_complete[4]  = 0;
		int width = static_cast<int>((KaminoGlobal::g_ideal_rect.right - KaminoGlobal::g_ideal_rect.left) / 3.2);
		int height = static_cast<int>((KaminoGlobal::g_ideal_rect.bottom - KaminoGlobal::g_ideal_rect.top) / 3.2);
		int x = KaminoGlobal::g_ideal_rect.right - width;
		int y = KaminoGlobal::g_ideal_rect.top + 40;
		InitDeviceIndependantResource();
		download_window = CreateWindowEx(NULL, L"DownloadWindow", L"", WS_OVERLAPPEDWINDOW, 
			x, y, width, height, NULL, NULL, GetModuleHandle(NULL), this);
		ShowWindow(download_window, SW_SHOW);
		UpdateWindow(download_window);
	}
	KaminoDownloadItemObserver::~KaminoDownloadItemObserver(){	
	}
	void KaminoDownloadItemObserver::OnDownloadDestroyed(DownloadItem *download){
		download_item = NULL;
	};
	void KaminoDownloadItemObserver::OnDownloadOpened(DownloadItem *download){};
	void KaminoDownloadItemObserver::OnDownloadRemoved(DownloadItem *download){};
	void KaminoDownloadItemObserver::OnDownloadUpdated(DownloadItem *download){
		percent_complete[0] = 0;
		percent_complete[1] = 0;
		percent_complete[2] = 0;
		percent_complete[3] = 0;
		percent_complete[4] = 0;
		wsprintf(percent_complete, L"%d%%", download->PercentComplete());
		download_item = download;
		Render();
	}

	HRESULT KaminoDownloadItemObserver::InitDeviceIndependantResource(){
		HRESULT hr = E_FAIL;
		FLOAT font_size = KaminoGlobal::g_screen_height / 10.0f;
		hr = text_format_num.Reset(L"Ubuntu", font_size, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
		hr = text_format_file.Reset(L"Ubuntu", font_size / 4.8f, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
		return hr;
	}

	HRESULT KaminoDownloadItemObserver::InitDeviceDependantResource(){
		HRESULT hr = S_OK;
		if(target.Get() == NULL){
			RECT rc;
			GetClientRect(download_window, &rc);
			hr = target.Reset(download_window, rc);
		}
		if(SUCCEEDED(hr) && (brush.Get() == NULL))
			hr = brush.Reset(0.176f, 0.176f, 0.176f, 1.0f, target.Get());
		return hr;
	}

	void KaminoDownloadItemObserver::DestroyDeviceDependantResource(){
		target.Clean();
		brush.Clean();
	}

	void KaminoDownloadItemObserver::Render(){
		if(SUCCEEDED(InitDeviceDependantResource())){
			RECT rc;
			GetClientRect(download_window, &rc);
			target.Get()->BeginDraw();
			target.Get()->Clear(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
			target.Get()->DrawTextW(percent_complete, wcslen(percent_complete), text_format_num.Get(), 
				D2D1::RectF(((rc.right - rc.left)* 0.032f), (rc.bottom - rc.top) * 0.032f, rc.right, rc.bottom), brush.Get());
			if(download_item != NULL){
				std::wstring file_name = download_item->GetFileNameToReportUser().value();
				target.Get()->DrawTextW(file_name.c_str(),file_name.length(), text_format_file.Get(), 
					D2D1::RectF(((rc.right - rc.left)* 0.032f), (rc.bottom - rc.top) * 0.032f + 100.0f, rc.right, rc.bottom), brush.Get());
			}
			if(target.Get()->EndDraw() == D2DERR_RECREATE_TARGET)
				DestroyDeviceDependantResource();
		}
		ValidateRect(download_window, NULL);
	}

	LRESULT CALLBACK KaminoDownloadItemObserver::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
		KaminoDownloadItemObserver *myself = NULL;
		if(msg == WM_CREATE){
			CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
			myself = (KaminoDownloadItemObserver *)pcs->lpCreateParams;
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(myself));
			return 0;
		}
		myself = reinterpret_cast<KaminoDownloadItemObserver *>(static_cast<LONG_PTR>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
		if(myself == NULL)
			return DefWindowProc(hWnd, msg, wParam, lParam);
		switch(msg){
			case WM_PAINT:
				myself->Render();
				return 0;
			case WM_SIZE:{
					RECT rc;
					GetClientRect(hWnd, &rc);
					((ID2D1HwndRenderTarget *)myself->target.Get())->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));		
				}
				return 0;
			case WM_CLOSE:
				if((myself->download_item != NULL) && (!myself->download_item->IsDone()))
					myself->download_item->Cancel(true);
				DestroyWindow(hWnd);
				return 0;
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}

	//---------------------------------------------- DownloadManager ----------------------------------------------------

	KaminoDownloadManagerDelegate::KaminoDownloadManagerDelegate(DownloadManager *manager) :download_manager(manager){ 
		AddRef();
	}
	KaminoDownloadManagerDelegate::~KaminoDownloadManagerDelegate(void){ }
	void KaminoDownloadManagerDelegate::Shutdown(){
		Release();
	}
	bool KaminoDownloadManagerDelegate::DetermineDownloadTarget(DownloadItem* download, const DownloadTargetCallback& callback) {
		download->AddObserver(new KaminoDownloadItemObserver());
		if (default_download_path_.empty()) {
			PathService::Get(base::DIR_USER_DESKTOP, &default_download_path_);
			default_download_path_ = default_download_path_.DirName();
			default_download_path_ = default_download_path_.Append(L"Downloads");			
		}
		if (!download->GetForcedFilePath().empty()) {
			callback.Run(download->GetForcedFilePath(), DownloadItem::TARGET_DISPOSITION_OVERWRITE,DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, download->GetForcedFilePath());
			return true;
		}
		base::FilePath generated_name = net::GenerateFileName(download->GetURL(), download->GetContentDisposition(), EmptyString(), download->GetSuggestedFilename(),
			download->GetMimeType(), "download");
		BrowserThread::PostTask(BrowserThread::FILE, FROM_HERE, 
								base::Bind(&KaminoDownloadManagerDelegate::GenerateFilename, 
								this, download->GetId(), 
								callback, generated_name,default_download_path_));
		return true;
	}

	void KaminoDownloadManagerDelegate::GenerateFilename(int32 download_id, const DownloadTargetCallback& callback, const base::FilePath& generated_name,
			const base::FilePath& suggested_directory) {
		DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
		if (!base::PathExists(suggested_directory))
			file_util::CreateDirectory(suggested_directory);
		base::FilePath suggested_path(suggested_directory.Append(generated_name));
		BrowserThread::PostTask(BrowserThread::UI, FROM_HERE, base::Bind(&KaminoDownloadManagerDelegate::OnDownloadPathGenerated,
								this, download_id, callback, suggested_path));
	}

	void KaminoDownloadManagerDelegate::OnDownloadPathGenerated(int32 download_id, const DownloadTargetCallback& callback, const base::FilePath& suggested_path) {
		DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
		wprintf(L"\nDownloading to Path > %s", suggested_path.value());
		callback.Run(suggested_path, DownloadItem::TARGET_DISPOSITION_PROMPT, DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, suggested_path.AddExtension(L"crdownload"));		
	}
}
