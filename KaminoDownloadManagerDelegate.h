// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_DOWNLOAD_MANAGER_DELEGATE_H
#define KAMINO_DOWNLOAD_MANAGER_DELEGATE_H

#include "base\compiler_specific.h"
#include "base\memory\ref_counted.h"
#include "content\public\browser\download_manager.h"
#include "content\public\browser\download_manager_delegate.h"
#include "KaminoDirectResource.h"


namespace content{

class  KaminoDownloadItemObserver : public DownloadItem::Observer{
public:
	static bool is_class_registerd;
	KaminoDownloadItemObserver();
	~KaminoDownloadItemObserver();
	virtual void OnDownloadUpdated(DownloadItem* download) OVERRIDE;
    virtual void OnDownloadOpened(DownloadItem* download) OVERRIDE;
    virtual void OnDownloadRemoved(DownloadItem* download) OVERRIDE;
    virtual void OnDownloadDestroyed(DownloadItem* download) OVERRIDE;

private:
	WNDCLASSEX wnd_cls;
	HWND download_window;
	wchar_t percent_complete[5];
	DownloadItem *download_item;
	KaminoTextFormat text_format_num;
	KaminoTextFormat text_format_file;
	KaminoDirect2DRenderTarget target;
	KaminoDirect2DBrush brush;
	HRESULT InitDeviceIndependantResource();
	HRESULT InitDeviceDependantResource();
	void DestroyDeviceDependantResource();
	void Render();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
	
class KaminoDownloadManagerDelegate : public DownloadManagerDelegate,
	public base::RefCountedThreadSafe<KaminoDownloadManagerDelegate> {
public:
	KaminoDownloadManagerDelegate(DownloadManager *manager);
	~KaminoDownloadManagerDelegate(void);
	virtual void Shutdown() OVERRIDE;
	virtual bool DetermineDownloadTarget(DownloadItem* download, const DownloadTargetCallback& callback) OVERRIDE;
	void GenerateFilename(int32 download_id, const DownloadTargetCallback& callback, const FilePath& generated_name, const FilePath& suggested_directory);
	void OnDownloadPathGenerated(int32 download_id, const DownloadTargetCallback& callback, const FilePath& suggested_path);
	FilePath default_download_path_;
	DownloadManager *download_manager;
	DISALLOW_COPY_AND_ASSIGN(KaminoDownloadManagerDelegate);
};

}
#endif