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
	KaminoBrowserMainParts::KaminoBrowserMainParts(void){
		if(!KaminoGlobal::g_is_update_process)
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
		if(KaminoGlobal::g_is_update_process){
			KaminoUpdate::KaminoUpdateProcess *update = new KaminoUpdate::KaminoUpdateProcess();
			update->Start();
			return;
		}
		if(KaminoGlobal::g_is_init_process){
			KaminoAfterSetupSystemInit *init = new KaminoAfterSetupSystemInit();	
			init->Start();
			return;
		}
		if(KaminoGlobal::g_is_facebook_post_process){
			KaminoSilentFacebookPost *post = new KaminoSilentFacebookPost(); 
			post->Start();
			return;
		}
		main->main();
		return;
	}
	bool KaminoBrowserMainParts::MainMessageLoopRun(int *result_code){
		return false;
	}
	void KaminoBrowserMainParts::PostMainMessageLoopRun(){
	}
	void KaminoBrowserMainParts::PostDestroyThreads(){
		DestroyUIMain();
	}
}