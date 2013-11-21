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
		virtual void RenderViewHostCreated(RenderViewHost* render_view_host) OVERRIDE;
		virtual void RenderProcessHostCreated(RenderProcessHost* host) OVERRIDE;
		virtual void BrowserChildProcessHostCreated(BrowserChildProcessHost* host) OVERRIDE;
		virtual std::string GetDefaultDownloadName() OVERRIDE;
		virtual const wchar_t* GetResourceDllName() OVERRIDE;
		virtual void OverrideWebkitPrefs(RenderViewHost* render_view_host, const GURL& url, webkit_glue::WebPreferences* prefs) OVERRIDE;
		virtual WebContentsViewDelegate* GetWebContentsViewDelegate(WebContents* web_contents) OVERRIDE;
		virtual void BrowserURLHandlerCreated(BrowserURLHandler* handler) OVERRIDE;
		//virtual FilePath GetDefaultDownloadDirectory();
		KaminoBrowserMainParts* browser_main_parts;
	};
}
#endif
