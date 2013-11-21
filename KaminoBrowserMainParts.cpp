// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoBrowserMainParts.h"
#include "KaminoGlobal.h"
#include "KaminoUpdateProcess.h"
#include "KaminoAfterSetupSystemInit.h"
#include "KaminoSilentFacebookPost.h"
#include "base\callback.h"


namespace content{

	KaminoNetLog::KaminoNetLog(){}
	KaminoNetLog::~KaminoNetLog(){}

	KaminoBrowserMainParts::KaminoBrowserMainParts(const MainFunctionParams &params) 
		:params_(params), run_message_loop_(true){
		if((!KaminoGlobal::g_is_update_process) && (!KaminoGlobal::g_is_facebook_post_process) && (!KaminoGlobal::g_is_init_process))
			main = GetUIMain();
	}
	
	KaminoBrowserMainParts::~KaminoBrowserMainParts(void){
	}
	
	void KaminoBrowserMainParts::PreEarlyInitialization() {
	}
	
	void KaminoBrowserMainParts::PostEarlyInitialization(){
	}
	
	void KaminoBrowserMainParts::PreMainMessageLoopStart(){
	}
	
	void KaminoBrowserMainParts::PostMainMessageLoopStart(){	
	}
	
	void KaminoBrowserMainParts::ToolkitInitialized(){
	}
	
	int KaminoBrowserMainParts::PreCreateThreads(){
		return 0;
	}
	
	void KaminoBrowserMainParts::PreMainMessageLoopRun(){
		net_log_.reset(new KaminoNetLog);
		browser_context_.reset(new KaminoBrowserContext(false, net_log_.get()));
		if(KaminoGlobal::g_is_update_process){
			KaminoUpdate::KaminoUpdateProcess *update = new KaminoUpdate::KaminoUpdateProcess(this);
			update->Start();
			return;
		}
		else 
			if(KaminoGlobal::g_is_init_process){
				KaminoAfterSetupSystemInit *init = new KaminoAfterSetupSystemInit(this);	
				init->Start();
				return;
			}
			else 
			if(KaminoGlobal::g_is_facebook_post_process){			
				KaminoSilentFacebookPost *post = new KaminoSilentFacebookPost(this); 
				post->Start();
				return;
			}
			else{
				main->SetBrowserContext(browser_context_.get());
				main->main();
			}
		if (params_.ui_task) {
			params_.ui_task->Run();
			delete params_.ui_task;
			run_message_loop_ = false;
		}
		return;
	}
	
	bool KaminoBrowserMainParts::MainMessageLoopRun(int *result_code){
		return !run_message_loop_;
	}
	
	void KaminoBrowserMainParts::PostMainMessageLoopRun(){
	}

	void KaminoBrowserMainParts::PostDestroyThreads(){
		if((!KaminoGlobal::g_is_update_process) && (!KaminoGlobal::g_is_facebook_post_process) && (!KaminoGlobal::g_is_init_process))
			DestroyUIMain();
	}
}