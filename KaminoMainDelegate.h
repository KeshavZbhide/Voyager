// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_MAIN_DELEGATE_H
#define KAMINO_MAIN_DELEGATE_H
#include "base\basictypes.h"
#include "base\compiler_specific.h"
#include "content\public\app\content_main_delegate.h"
#include "base\memory\scoped_ptr.h"
#include "content\public\common\content_client.h"
#include "content\public\browser\browser_main_runner.h"
#include "KaminoContentBrowserClient.h"
#include "KaminoContentRenderClient.h"
#include "KaminoGlobal.h"


namespace content{

	class KaminoContentClient : public ContentClient{
		public:
			virtual ~KaminoContentClient();
			virtual std::string GetUserAgent() const OVERRIDE;
			virtual string16 GetLocalizedString(int message_id) const OVERRIDE;
			virtual base::StringPiece GetDataResource(int resource_id, ui::ScaleFactor scale_factor) const OVERRIDE;
			virtual base::RefCountedStaticMemory* GetDataResourceBytes(int resource_id) const OVERRIDE;
			virtual gfx::Image& GetNativeImageNamed(int resource_id) const OVERRIDE;
	};

	class KaminoMainDelegate : public ContentMainDelegate
	{
	public:
		KaminoMainDelegate(void);
		~KaminoMainDelegate(void);
		virtual bool BasicStartupComplete(int* exit_code) override;
		virtual void PreSandboxStartup() override;
		virtual int RunProcess( const std::string& process_type, const MainFunctionParams& main_function_params) override;
		virtual ContentBrowserClient* CreateContentBrowserClient() override;
		virtual ContentRendererClient* CreateContentRendererClient() override;

	private:
		scoped_ptr<KaminoContentBrowserClient> browser_client;
		scoped_ptr<KaminoContentRenderClient> render_client;
		scoped_ptr<BrowserMainRunner> browser_main_runner;
		KaminoContentClient content_client;
		DISALLOW_COPY_AND_ASSIGN(KaminoMainDelegate);
	};
}
#endif