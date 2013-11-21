// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_CONTENT_BROWSER_CLIENT_H
#define KAMINO_CONTENT_BROWSER_CLIENT_H
#include "base\compiler_specific.h"
#include "content\public\browser\content_browser_client.h"
#include "KaminoBrowserMainParts.h"
#include "KaminoGlobal.h"


namespace content{

	class KaminoContentBrowserClient : public ContentBrowserClient
	{
	public:
		KaminoContentBrowserClient(void);
		~KaminoContentBrowserClient(void);
		virtual BrowserMainParts* CreateBrowserMainParts(const MainFunctionParams& parameters) OVERRIDE;

		//virtual void RenderViewHostCreated(RenderViewHost* render_view_host) OVERRIDE;
		virtual void RenderProcessHostCreated(RenderProcessHost* host) OVERRIDE;
		
		//[..Yet Unimplemented.....
		virtual net::URLRequestContextGetter* CreateRequestContext(BrowserContext* browser_context, ProtocolHandlerMap* protocol_handlers) OVERRIDE;
		virtual net::URLRequestContextGetter* CreateRequestContextForStoragePartition(BrowserContext* browser_context, 
			const base::FilePath& partition_path, bool in_memory, ProtocolHandlerMap* protocol_handlers) OVERRIDE;
		virtual bool IsHandledURL(const GURL& url) OVERRIDE;
		virtual void AppendExtraCommandLineSwitches(CommandLine* command_line, int child_process_id) OVERRIDE;
		virtual void ResourceDispatcherHostCreated() OVERRIDE;
		virtual AccessTokenStore* CreateAccessTokenStore() OVERRIDE;
		virtual bool SupportsBrowserPlugin(content::BrowserContext* browser_context, const GURL& url) OVERRIDE;
		virtual QuotaPermissionContext* CreateQuotaPermissionContext() OVERRIDE;
		virtual SpeechRecognitionManagerDelegate* GetSpeechRecognitionManagerDelegate() OVERRIDE;
		virtual net::NetLog* GetNetLog() OVERRIDE;
		virtual bool ShouldSwapProcessesForRedirect(ResourceContext* resource_context, const GURL& current_url,
			const GURL& new_url) OVERRIDE;
		//Implement Fast..]
	
		virtual std::string GetDefaultDownloadName() OVERRIDE;
		virtual const wchar_t* GetResourceDllName() OVERRIDE;
		virtual void OverrideWebkitPrefs(RenderViewHost* render_view_host, const GURL& url, WebPreferences* prefs) OVERRIDE;
		virtual WebContentsViewDelegate* GetWebContentsViewDelegate(WebContents* web_contents) OVERRIDE;
		virtual void BrowserURLHandlerCreated(BrowserURLHandler* handler) OVERRIDE;
		//virtual FilePath GetDefaultDownloadDirectory();
		KaminoBrowserMainParts* browser_main_parts;
	};
}
#endif
