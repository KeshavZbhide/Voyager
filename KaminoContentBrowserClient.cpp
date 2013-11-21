// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "UIMain.h"
#include "KTab.h"
#include "KaminoWebContentViewDelegate.h"
#include "KaminoRenderViewHost.h"

#include <windows.h>
#include "base\file_path.h"
#include "base\path_service.h"
#include "KaminoContentBrowserClient.h"
#include "content\public\browser\web_contents.h"
#include "webkit\glue\webpreferences.h"
#include "content\public\browser\render_view_host.h"
#include "content\public\browser\browser_url_handler.h"


namespace content{

	KaminoContentBrowserClient::KaminoContentBrowserClient(void){
		browser_main_parts = NULL; 
	}
	KaminoContentBrowserClient::~KaminoContentBrowserClient(void){ 
	}
	BrowserMainParts* KaminoContentBrowserClient::CreateBrowserMainParts(const MainFunctionParams& parameters){
		browser_main_parts = new KaminoBrowserMainParts();
		return browser_main_parts;
	}
	void KaminoContentBrowserClient::RenderViewHostCreated(RenderViewHost* render_view_host){
		new KaminoRenderViewHostObserver(render_view_host);
	}
	void KaminoContentBrowserClient::RenderProcessHostCreated(RenderProcessHost* host){
		host->GetChannel()->AddFilter(new KaminoSpecialCommunicator);
	}
	void KaminoContentBrowserClient::BrowserChildProcessHostCreated(BrowserChildProcessHost* host) {
	}
	std::string KaminoContentBrowserClient::GetDefaultDownloadName(){
		return std::string("download.file");
	}
	const wchar_t* KaminoContentBrowserClient::GetResourceDllName(){
		static wchar_t file_path[MAX_PATH+1] = {0};
		if(file_path[0] = 0){
			FilePath module;
			PathService::Get(base::FILE_MODULE, &module);
			const std::wstring wstr = module.value();
			int count  = std::min(static_cast<size_t>(MAX_PATH), wstr.size());
			wcsncpy(file_path, wstr.c_str(), count);
			file_path[count] = 0;
		}
		return file_path;
	}
	void KaminoContentBrowserClient::OverrideWebkitPrefs(RenderViewHost* render_view_host, const GURL& url, webkit_glue::WebPreferences* prefs) {
		prefs->allow_file_access_from_file_urls = true;
		prefs->dns_prefetching_enabled = true;
		prefs->allow_universal_access_from_file_urls = true;
	}
	WebContentsViewDelegate* KaminoContentBrowserClient::GetWebContentsViewDelegate(WebContents* web_contents){
		return new KaminoWebContentViewDelegate(web_contents);
	}
	bool KaminoLauchNewBrowserProcess(GURL *url, BrowserContext* browser_context){
		std::string window = url->possibly_invalid_spec();
		if(window.compare("kamino://incognito/") == 0){
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			CreateProcess(KaminoGlobal::g_executable_name, L" -incognito", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);													
		}
		if(window.compare("kamino://new/") == 0){
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			CreateProcess(KaminoGlobal::g_executable_name, L"", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);																
		}
		return false;
	}
	void KaminoContentBrowserClient::BrowserURLHandlerCreated(BrowserURLHandler* handler){
		handler->AddHandlerPair(KaminoLauchNewBrowserProcess, BrowserURLHandler::null_handler());		
	}
}