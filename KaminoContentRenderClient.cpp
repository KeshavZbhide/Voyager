// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoIPCMessages.h"
#include "KaminoContentRenderClient.h"
#include "base\memory\ref_counted.h"
#include "content\public\renderer\render_view.h"
#include "content\public\renderer\render_view_observer.h"
#include "content\public\renderer\render_thread.h"
#include "content\public\renderer\render_process_observer.h"
#include "v8\include\v8.h"

#include "third_party\WebKit\public\web\WebView.h"
#include "third_party\WebKit\public\web\WebFrame.h"

#include <vector>
#include <map>

namespace v8{

void KaminoGetRecentHistory(const FunctionCallbackInfo<v8::Value>& args){
	HandleScope handle_scope(args.GetIsolate());	
	std::vector<std::wstring> tittels;
	std::vector<std::string> urls;
	content::KaminoContentRenderClient *client = reinterpret_cast<content::KaminoContentRenderClient *>(v8::External::Cast(*args.Data())->Value());
	content::RenderView *view = client->GetRenderViewForFrame(WebKit::WebFrame::frameForCurrentContext()->top());
	view->Send(new KaminoMsg_GetRecentHistoryTittles(view->GetRoutingID(), &tittels));
	view->Send(new KaminoMsg_GetRecentHistoryURLs(view->GetRoutingID(), &urls));
	Local<Array> history = Array::New(tittels.size() * 2);
	if(tittels.size() != urls.size()){
		args.GetReturnValue().Set(Undefined());
		return;
	}
	for(unsigned int i = 0, j = 0; i < tittels.size(); i++, j++){
		history->Set(j, String::New((uint16_t *)tittels[i].c_str()));
		history->Set(++j, String::New(urls[i].c_str()));
	}
	args.GetReturnValue().Set(history);
}

void KaminoSetSymLink(const FunctionCallbackInfo<v8::Value>& args){
	HandleScope handle_scope(args.GetIsolate());
	String::Utf8Value key(args[0]);
	String::Utf8Value link(args[1]);
	content::KaminoContentRenderClient *client = reinterpret_cast<content::KaminoContentRenderClient *>(v8::External::Cast(*args.Data())->Value());
	content::RenderView *view = client->GetRenderViewForFrame(WebKit::WebFrame::frameForCurrentContext()->top());
	if(view != NULL)
		view->Send(new KaminoMsg_NewSymLink(view->GetRoutingID(), std::string(*key), std::string(*link)));
}

void KaminoLOG(const FunctionCallbackInfo<v8::Value>& args){
	HandleScope handle_scope(args.GetIsolate());
	String::Utf8Value msg(args[0]);
	content::KaminoContentRenderClient *client = reinterpret_cast<content::KaminoContentRenderClient *>(v8::External::Cast(*args.Data())->Value());
	content::RenderView *view = client->GetRenderViewForFrame(WebKit::WebFrame::frameForCurrentContext()->top());
	if(view != NULL)
		view->Send(new KaminoMsg_LOG(view->GetRoutingID(), std::string(*msg)));
};

void KaminoILike(const FunctionCallbackInfo<v8::Value>& args){
	HandleScope handle_scope(args.GetIsolate());	
	String::Utf8Value msg(args[0]);
	content::KaminoContentRenderClient *client = reinterpret_cast<content::KaminoContentRenderClient *>(v8::External::Cast(*args.Data())->Value());
	content::RenderView *view = client->GetRenderViewForFrame(WebKit::WebFrame::frameForCurrentContext()->top());
	if(view != NULL)
		view->Send(new KaminoMsg_ILIKE(view->GetRoutingID(), std::string(*msg)));
};

void KaminoShouldShowIcons(const FunctionCallbackInfo<v8::Value>& args){
	HandleScope handle_scope(args.GetIsolate());
	content::KaminoContentRenderClient *client = reinterpret_cast<content::KaminoContentRenderClient *>(v8::External::Cast(*args.Data())->Value());
	content::RenderView *view = client->GetRenderViewForFrame(WebKit::WebFrame::frameForCurrentContext()->top());
	bool should_show_icons = false;
	if(view != NULL)
		view->Send(new KaminoMsg_ShouldShowIcons(view->GetRoutingID(), args[0]->BooleanValue(), &should_show_icons));
	args.GetReturnValue().Set(Boolean::New(should_show_icons));
}

class AwesomeKaminoJSExtensioAPI: public Extension{
public:
	AwesomeKaminoJSExtensioAPI(char *name, char *js_code, content::KaminoContentRenderClient *client)
		:Extension(name, js_code){
			client_ = client;
	}

	virtual Handle<FunctionTemplate> GetNativeFunction(Handle<v8::String> name) OVERRIDE{
		if(name->Equals(String::New("KaminoLOG")))
			return FunctionTemplate::New(KaminoLOG, External::New(client_));
		if(name->Equals(String::New("KaminoILike")))
			return FunctionTemplate::New(KaminoILike, External::New(client_));
		if(name->Equals(String::New("KaminoGetRecentHistory")))
			return FunctionTemplate::New(KaminoGetRecentHistory, External::New(client_));
		if(name->Equals(String::New("KaminoShouldShowIcons")))
			return FunctionTemplate::New(KaminoShouldShowIcons, External::New(client_));
		if(name->Equals(String::New("KaminoSetSymLink")))
			return FunctionTemplate::New(KaminoSetSymLink, External::New(client_));
		return FunctionTemplate::New();
	}

private:
	content::KaminoContentRenderClient *client_;
};

}

namespace content{

//------------------------------------------KaminoRenderProcessObserver-----------------------------------------

class KaminoRenderProcessObserver : public RenderProcessObserver{
public:
	KaminoRenderProcessObserver(KaminoContentRenderClient *client){
		client_ = client;
	}
	virtual void WebKitInitialized() OVERRIDE{ //This is all we need this class for V8 Extension API.
		RenderThread::Get()->RegisterExtension(new v8::AwesomeKaminoJSExtensioAPI("app/x", "native function KaminoILike(x); native function KaminoLOG(x);" 
			"native function KaminoGetRecentHistory(); native function KaminoShouldShowIcons(x); native function KaminoSetSymLink(k,l);", client_));
	}
private:
	KaminoContentRenderClient *client_;
	DISALLOW_COPY_AND_ASSIGN(KaminoRenderProcessObserver);
};
 
//--------------------------------------------KaminoRenderViewObserver------------------------------------------

class KaminoRenderViewObserver : public RenderViewObserver{ //We Need this for OnMessageRecived.
public:
	KaminoRenderViewObserver(RenderView *render_view, KaminoContentRenderClient *client)
		:RenderViewObserver(render_view){ 
			client_ = client;
	}
	virtual void OnDestruct() OVERRIDE{
		if(client_)
			for(KaminoContentRenderClient::RenderViewList *it = client_->assosiated_render_views; it != NULL; it = it->next)
				if(it->view == render_view()){
					delete it;
				}
		delete this;
	}
private:
	KaminoContentRenderClient *client_;
};

//-----------------------------------------KaminoContentRenderClient-----------------------------------------------

KaminoContentRenderClient::RenderViewList::RenderViewList(RenderView *v, RenderViewList *p){ view = v; next = NULL; prev = p; }
KaminoContentRenderClient::RenderViewList::~RenderViewList(){ view = NULL; if(prev) prev->next = next; }

KaminoContentRenderClient::KaminoContentRenderClient(){
	assosiated_render_views = NULL;
	render_process_observer.reset(NULL);
}

KaminoContentRenderClient::~KaminoContentRenderClient(){
}

void KaminoContentRenderClient::RenderThreadStarted(){
	if(render_process_observer.get() == NULL)
		render_process_observer.reset(new KaminoRenderProcessObserver(this));
	RenderThread::Get()->AddObserver(render_process_observer.get());
}

void KaminoContentRenderClient::RenderViewCreated(RenderView* render_view){
	new KaminoRenderViewObserver(render_view, this);
	if(assosiated_render_views == NULL)
		assosiated_render_views = new RenderViewList(render_view, NULL);
	else{
		RenderViewList *it = assosiated_render_views;
		for(; it->next != NULL; it = it->next);
		it->next = new RenderViewList(render_view, it);
	}
}

RenderView *KaminoContentRenderClient::GetRenderViewForFrame(WebKit::WebFrame *frame){
	if(assosiated_render_views){
		if(assosiated_render_views->next == NULL)
			return assosiated_render_views->view;
		else{
			for(RenderViewList *it = assosiated_render_views->next; it != NULL; it = it->next)
				if(it->view->GetWebView()->mainFrame() == frame)
					return it->view;
		}
	}
	return NULL;
}

}