// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_BROWSER_MAIN_PARTS_H
#define KAMINO_BROWSER_MAIN_PARTS_H

#include <windows.h>
#include <string>
#include "base\message_loop\message_loop.h"
#include "content\public\browser\browser_main_parts.h"
#include "net\base\net_log_logger.h"
#include "base\basictypes.h"
#include "base\memory\scoped_ptr.h"
#include "base\compiler_specific.h"
#include "content\public\common\main_function_params.h"
#include "KaminoBrowserContext.h"
#include "KaminoGlobal.h"
#include "UIMain.h"

namespace content{
	
	class KaminoNetLog : public net::NetLog{
		public:
			KaminoNetLog();
			virtual ~KaminoNetLog();
		private:
			scoped_ptr<net::NetLogLogger> net_log_logger_;
			DISALLOW_COPY_AND_ASSIGN(KaminoNetLog);	
	};

	class KaminoBrowserMainParts : public BrowserMainParts
	{
	public:
		KaminoBrowserMainParts(const MainFunctionParams &params);
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
		KaminoBrowserContext *browser_context() {return browser_context_.get();}
		net::NetLog *net_log(){ return net_log_.get(); }
	private:
		UIMain *main;
		scoped_ptr<KaminoBrowserContext> browser_context_;
		scoped_ptr<net::NetLog> net_log_;
		bool run_message_loop_;
		MainFunctionParams params_;
	};
}
#endif