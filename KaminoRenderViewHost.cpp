// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoRenderViewHost.h"
#include "UIMain.h"
#include "KaminoHistoryModule.h"
#include "KaminoIPCMessages.h"
#include "base\synchronization\waitable_event.h"
#include "ipc\ipc_message_macros.h"
#include "content\public\browser\render_process_host.h"
#include "ipc\ipc_sync_message_filter.h"
#include "KTab.h"
#include "KaminoGlobal.h"
#include "base\file_util.h"

class KTabManager;

namespace content{

KaminoSpecialCommunicator::KaminoSpecialCommunicator()
	:IPC::SyncMessageFilter(new base::WaitableEvent(false, true)){}
	
bool KaminoSpecialCommunicator::OnMessageReceived(const IPC::Message& message){
	bool handled = true;
	IPC_BEGIN_MESSAGE_MAP(KaminoSpecialCommunicator, message)
		IPC_MESSAGE_HANDLER(KaminoMsg_GetRecentHistoryTittles, ForwardRecentHistoryTittels)
		IPC_MESSAGE_HANDLER(KaminoMsg_GetRecentHistoryURLs, ForwardRecentHistoryURLs)
		IPC_MESSAGE_HANDLER(KaminoMsg_ShouldShowIcons, ForwardShouldShowIconsStaticVar);
		IPC_MESSAGE_UNHANDLED(handled = false)
	IPC_END_MESSAGE_MAP()
	return handled;
}

void KaminoSpecialCommunicator::ForwardRecentHistoryTittels(std::vector<std::wstring> *vec){
	KaminoHistoryModule::getTittle(vec);		
}
void KaminoSpecialCommunicator::ForwardRecentHistoryURLs(std::vector<std::string> *vec){
	KaminoHistoryModule::getURLs(vec);		
}
void KaminoSpecialCommunicator::ForwardShouldShowIconsStaticVar(bool one, bool *two){
	static bool initial_val = false;
	if(one)
		initial_val = true;
	*two = initial_val;
}


KaminoRenderViewHostObserver::KaminoRenderViewHostObserver(RenderViewHost *render_view_host)
	:RenderViewHostObserver(render_view_host){}

bool KaminoRenderViewHostObserver::OnMessageReceived(const IPC::Message& message){
	bool handled = true;
	IPC_BEGIN_MESSAGE_MAP(KaminoRenderViewHostObserver, message)
		IPC_MESSAGE_HANDLER(KaminoMsg_LOG, LogFromRenderer);
		IPC_MESSAGE_HANDLER(KaminoMsg_ILIKE, ILikeFromRenderer);
		IPC_MESSAGE_HANDLER(KaminoMsg_NewSymLink, NewSymLinkFromRenderer);
		IPC_MESSAGE_UNHANDLED(handled = false)
	IPC_END_MESSAGE_MAP()
  return handled;
}

void KaminoRenderViewHostObserver::LogFromRenderer(std::string msg){
	UI_LOG(0, "MSG From Renderer >>>> %s", msg.c_str());
}

void KaminoRenderViewHostObserver::NewSymLinkFromRenderer(std::string key, std::string link){
	std::string js_content;
	UI_LOG(0, "Executin Inside >>> NewSymLinkFromRenderer");
	if(base::PathExists(base::FilePath(L"..\\KaminoResource\\simlinks.js"))){
		base::ReadFileToString(base::FilePath(L"..\\KaminoResource\\simlinks.js"), &js_content);
		size_t index = js_content.find(key);
		if(index == -1 || index == js_content.npos){
			js_content.resize(js_content.length() - 2);
			js_content += key + ":" + link + ",};";
			file_util::WriteFile(base::FilePath(L"..\\KaminoResource\\simlinks.js"), js_content.c_str(), js_content.length());
		}
		else{
			base::DeleteFile(base::FilePath(L"..\\KaminoResource\\simlinks.js"), false);
			index += key.length() + 1;
			std::string rest = js_content.substr(index, js_content.length());
			rest = rest.substr(rest.find(","), rest.length());
			js_content.resize(index);
			std::string final = js_content.append(link).append(rest);
			file_util::WriteFile(base::FilePath(L"..\\KaminoResource\\simlinks.js"), js_content.c_str(), js_content.length());			
		}
	}	
}


void KaminoRenderViewHostObserver::ILikeFromRenderer(std::string content){
	UI_LOG(0, "Recived I LIKE from Renderer :: %s", content.c_str())
	char *str = "<html><body><head><link href='styler.css' rel='stylesheet' type='text/css'></head>";
	char *str2 = "</body></html>";
	if(!base::PathExists(base::FilePath(L"..\\KaminoResource\\IlikeHtm.html"))){
		file_util::WriteFile(base::FilePath(L"..\\KaminoResource\\IlikeHtm.html"), str, strlen(str));
		file_util::AppendToFile(base::FilePath(L"..\\KaminoResource\\IlikeHtm.html"), content.c_str(), content.length());
		file_util::AppendToFile(base::FilePath(L"..\\KaminoResource\\IlikeHtm.html"), str2, strlen(str2));
	}
	else{
		base::FilePath path = base::FilePath(L"..\\KaminoResource\\IlikeHtm.html");
		char *present_content = NULL;
		int64 content_size;
		file_util::GetFileSize(path, &content_size);
		present_content = (char *)malloc(content_size);
		//file_util::AppendToFile(FilePath(L"..\\KaminoResource\\IlikeHtm.html"), content.c_str(), content.length());		
		file_util::ReadFile(path, present_content, content_size);
		file_util::WriteFile(path, str, strlen(str));
		file_util::AppendToFile(path, content.c_str(), content.length());
		file_util::AppendToFile(path, present_content + strlen(str), content_size - strlen(str));
		free(present_content);
	}
	KTabManager *main_manager = (KTabManager *)(KaminoGlobal::ptr_a);
	main_manager->AddTab(KaminoGlobal::g_ilike_url);
}

}
