// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoMainDelegate.h"
#include "base\file_util.h"
#include "base\files\file_path.h"
#include "base\path_service.h"
#include "webkit\common\user_agent\user_agent_util.h"
#include "ui\base\l10n\l10n_util.h"
#include "ui\base\resource\resource_bundle.h"


namespace content{
	KaminoContentClient::~KaminoContentClient(){
	}
	
	std::string KaminoContentClient::GetUserAgent() const {
		std::string product = "Chrome/31.0.1650.34";
		return webkit_glue::BuildUserAgentFromProduct(product);
	}

	string16 KaminoContentClient::GetLocalizedString(int message_id) const{
		return l10n_util::GetStringUTF16(message_id);		
	}

	base::StringPiece KaminoContentClient::GetDataResource( int resource_id, ui::ScaleFactor scale_factor) const {
		return ResourceBundle::GetSharedInstance().GetRawDataResourceForScale(resource_id, scale_factor);	
	}

	base::RefCountedStaticMemory* KaminoContentClient::GetDataResourceBytes(
		int resource_id) const {
			return ResourceBundle::GetSharedInstance().LoadDataResourceBytes(resource_id);
	}
	
	gfx::Image& KaminoContentClient::GetNativeImageNamed(int resource_id) const {
		return ResourceBundle::GetSharedInstance().GetNativeImageNamed(resource_id);
	}

	//----------------------------------------Main-Delegte-----------------------------------
	
	KaminoMainDelegate::KaminoMainDelegate(void){
	}
	KaminoMainDelegate::~KaminoMainDelegate(void){
	}
	
	bool KaminoMainDelegate::BasicStartupComplete(int* exit_code){
		SetContentClient(&content_client);
		return false;
	}
	
	void KaminoMainDelegate::PreSandboxStartup(){
		base::FilePath pak_file;
		base::FilePath pak_dir;
		PathService::Get(base::DIR_MODULE, &pak_dir);
		pak_file = pak_dir.Append(FILE_PATH_LITERAL("content_shell.pak"));
		ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
	}

	int KaminoMainDelegate::RunProcess( const std::string& process_type, const MainFunctionParams& main_function_params){
		if(process_type.empty() || KaminoGlobal::g_is_incognito_mode 
			|| KaminoGlobal::g_is_init_process 
			|| KaminoGlobal::g_is_facebook_post_process
			|| KaminoGlobal::g_is_test_run){
			browser_main_runner.reset(BrowserMainRunner::Create());
			browser_main_runner->Initialize(main_function_params);
			browser_main_runner->Run();
			browser_main_runner->Shutdown();
			return 0;
		}
		return -1;
	}

	ContentBrowserClient* KaminoMainDelegate::CreateContentBrowserClient() {
		if(!browser_client.get()){
			browser_client.reset(new KaminoContentBrowserClient());
		}
		return browser_client.get();
	}
	
	
	ContentRendererClient* KaminoMainDelegate::CreateContentRendererClient(){
		if(!render_client.get())
			render_client.reset(new KaminoContentRenderClient());
		return render_client.get();
	}	
}
