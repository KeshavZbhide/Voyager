// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoMainDelegate.h"
#include "base\string_piece.h"
#include "webkit\user_agent\user_agent_util.h"


namespace content{
	KaminoContentClient::~KaminoContentClient(){
	}
	std::string KaminoContentClient::GetUserAgent() const{
		std::string product = "Chrome/25.0.1364.36";
		return webkit_glue::BuildUserAgentFromProduct(product);
	}
	KaminoMainDelegate::KaminoMainDelegate(void){
	}
	KaminoMainDelegate::~KaminoMainDelegate(void){
	}
	bool KaminoMainDelegate::BasicStartupComplete(int* exit_code){
		SetContentClient(&content_client);
		return false;
	}
	void KaminoMainDelegate::PreSandboxStartup(){
	}
	void KaminoMainDelegate::SandboxInitialized(const std::string& process_type){
	}
	int KaminoMainDelegate::RunProcess( const std::string& process_type, const MainFunctionParams& main_function_params){
		if(process_type.empty()){
			browser_main_runner.reset(BrowserMainRunner::Create());
			browser_main_runner->Initialize(main_function_params);
			browser_main_runner->Run();
			browser_main_runner->Shutdown();
			return 0;
		}
		return -1;
	}
	
	void KaminoMainDelegate::ProcessExiting(const std::string& process_type){
	}
	
	ContentBrowserClient* KaminoMainDelegate::CreateContentBrowserClient() {
		if(!browser_client.get()){
			browser_client.reset(new KaminoContentBrowserClient());
		}
		return browser_client.get();
	}
	
	ContentPluginClient* KaminoMainDelegate::CreateContentPluginClient(){
		return NULL;
	}
	
	ContentRendererClient* KaminoMainDelegate::CreateContentRendererClient(){
		if(!render_client.get())
			render_client.reset(new KaminoContentRenderClient());
		return render_client.get();
	}
	
	ContentUtilityClient* KaminoMainDelegate::CreateContentUtilityClient(){
		return NULL;
	}
}
