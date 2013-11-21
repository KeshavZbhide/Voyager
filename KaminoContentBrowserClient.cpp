// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "UIMain.h"
#include "KTab.h"
#include "KaminoWebContentViewDelegate.h"
#include "KaminoRenderViewHost.h"

#include <windows.h>
#include "base\files\file_path.h"
#include "base\path_service.h"
#include "KaminoContentBrowserClient.h"
#include "content\public\browser\web_contents.h"
#include "content\public\browser\render_view_host.h"
#include "content\public\browser\browser_url_handler.h"
#include "webkit\common\webpreferences.h"


namespace content{

	KaminoContentBrowserClient::KaminoContentBrowserClient(void){
		browser_main_parts = NULL; 
	}
	
	KaminoContentBrowserClient::~KaminoContentBrowserClient(void){ 
	}
	
	BrowserMainParts* KaminoContentBrowserClient::CreateBrowserMainParts(const MainFunctionParams& parameters){
		browser_main_parts = new KaminoBrowserMainParts(parameters);
		return browser_main_parts;
	}
	
	void KaminoContentBrowserClient::RenderProcessHostCreated(RenderProcessHost* host){
		host->GetChannel()->AddFilter(new KaminoSpecialCommunicator);
	}
		
	net::URLRequestContextGetter* KaminoContentBrowserClient::CreateRequestContext(BrowserContext* browser_context, ProtocolHandlerMap* protocol_handlers)
	{	
		if(browser_main_parts->browser_context() == browser_context)
			return browser_main_parts->browser_context()->CreateRequestContext(protocol_handlers);
		else
			return NULL;
	}
	
	net::URLRequestContextGetter* KaminoContentBrowserClient::CreateRequestContextForStoragePartition(BrowserContext* browser_context, 
		const base::FilePath& partition_path, bool in_memory, ProtocolHandlerMap* protocol_handlers){	
			if(browser_main_parts->browser_context() == browser_context)
				return browser_main_parts->browser_context()->CreateRequestContextForStoragePartition(
																partition_path, in_memory, protocol_handlers);
			else
				return NULL;
	}

	bool KaminoContentBrowserClient::IsHandledURL(const GURL& url) {
		return true;
	}

	void KaminoContentBrowserClient::AppendExtraCommandLineSwitches(CommandLine* command_line, int child_process_id){		
	
	}

	void KaminoContentBrowserClient::ResourceDispatcherHostCreated(){
	
	}

	AccessTokenStore* KaminoContentBrowserClient::CreateAccessTokenStore(){
		return NULL;
	}

	bool KaminoContentBrowserClient::SupportsBrowserPlugin(content::BrowserContext* browser_context, const GURL& url){
		return true;
	}

	QuotaPermissionContext* KaminoContentBrowserClient::CreateQuotaPermissionContext(){
		return NULL;	
	}
	
	SpeechRecognitionManagerDelegate* KaminoContentBrowserClient::GetSpeechRecognitionManagerDelegate(){
		return NULL;
	}

	net::NetLog* KaminoContentBrowserClient::GetNetLog(){	
		return browser_main_parts->net_log();
	}
	
	bool KaminoContentBrowserClient::ShouldSwapProcessesForRedirect(ResourceContext* resource_context, const GURL& current_url,
		const GURL& new_url){
		return false;
	}

	std::string KaminoContentBrowserClient::GetDefaultDownloadName(){
		return std::string("download.file");
	}
	
	const wchar_t* KaminoContentBrowserClient::GetResourceDllName(){
		static wchar_t file_path[MAX_PATH+1] = {0};
		if(file_path[0] = 0){
			base::FilePath module;
			PathService::Get(base::FILE_MODULE, &module);
			const std::wstring wstr = module.value();
			int count  = std::min(static_cast<size_t>(MAX_PATH), wstr.size());
			wcsncpy(file_path, wstr.c_str(), count);
			file_path[count] = 0;
		}
		return file_path;
	}

	//TODO Understant And Solve WebPreferences....
	void KaminoContentBrowserClient::OverrideWebkitPrefs(RenderViewHost* render_view_host,
                                   const GURL& url,
                                   WebPreferences* prefs) {
		prefs->allow_file_access_from_file_urls = true;
		prefs->dns_prefetching_enabled = true;
		prefs->allow_universal_access_from_file_urls = true;
	}
	
	WebContentsViewDelegate* KaminoContentBrowserClient::GetWebContentsViewDelegate(WebContents* web_contents){
		return new KaminoWebContentViewDelegate(web_contents);
	}

	bool KaminoLauchNewBrowserProcess(GURL *url, BrowserContext* browser_context){
		/*std::string window = url->possibly_invalid_spec();
		if(window.compare("file://launch_incognito/") == 0){
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			CreateProcess(KaminoGlobal::g_executable_name, L" -incognito", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);													
		}
		if(window.compare("file://launch_new_window/") == 0){
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			CreateProcess(KaminoGlobal::g_executable_name, L"", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);																
		}
		return false;*/
		return false;
	}
	
	void KaminoContentBrowserClient::BrowserURLHandlerCreated(BrowserURLHandler* handler){
		UI_LOG(0, "Browser URL Handler Created..")
		//handler->AddHandlerPair(KaminoLauchNewBrowserProcess, BrowserURLHandler::null_handler());		
	}
}