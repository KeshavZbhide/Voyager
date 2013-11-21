// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_BROWSER_MAIN_PARTS_H
#define KAMINO_BROWSER_MAIN_PARTS_H
#include <windows.h>
#include "base\message_loop.h"
#include "content\public\browser\browser_main_parts.h"
#include "KaminoGlobal.h"
#include "UIMain.h"


namespace content{
	class KaminoBrowserMainParts : public BrowserMainParts
	{
	public:
		KaminoBrowserMainParts(void);
		~KaminoBrowserMainParts(void);
		virtual void PreEarlyInitialization() override;
		virtual void PostEarlyInitialization() override;
		virtual void PreMainMessageLoopStart() override;
		virtual void PostMainMessageLoopStart() override;
		virtual void ToolkitInitialized() override;
		virtual int PreCreateThreads() override;
		virtual void PreMainMessageLoopRun() override;
		virtual bool MainMessageLoopRun(int* result_code) override;
		virtual void PostMainMessageLoopRun() override;
		virtual void PostDestroyThreads() override;
	private:
		UIMain *main;
		//scoped_ptr<MessageLoop> chrome_message_loop;
	};
}
#endif