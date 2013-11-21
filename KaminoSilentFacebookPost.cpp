#include "KaminoSilentFacebookPost.h"
#include "content\public\browser\render_view_host.h"
#include "content\public\browser\browser_thread.h"
#include "content\public\browser\notification_source.h"
#include "content\public\browser\notification_types.h"
#include "base\message_loop.h"
#include "base\time.h"
#include "KaminoGlobal.h"
#include "UIMain.h"

KaminoSilentFacebookPost::KaminoSilentFacebookPost(){
	browser_context.reset(new content::KaminoBrowserContext());
}

KaminoSilentFacebookPost::~KaminoSilentFacebookPost(){
	browser_context.reset(NULL);	
}

void KaminoSilentFacebookPost::ExitFaceBookPostProcess(){
	UI_LOG(0, "Exiting Facebook Post Process")
	web_content->Close();
	web_content.reset(NULL);
	MessageLoop::current()->QuitWhenIdle();
}

void KaminoSilentFacebookPost::PostComment(){
	static bool is_first = true;
	std::wstring js_code = L"var txt = document.getElementsByTagName(\"textarea\");"
		L"var at_txt;"
		L"for(i = 0; i < txt.length; i++){ if(txt[i].value == \"What's on your mind?\") at_txt = txt[i]; }"
		L"var buttons = document.getElementsByTagName(\"button\");"
		L"var at_button;"
		L"for(i = 0; i < buttons.length; i++){ if(buttons[i].innerHTML == \"Post\") at_button = buttons[i]; }"
		L"at_txt.value = \"I use Voyjor Web Browser (http://voyjor.com)\";"
		L"at_button.click();";
	web_content->GetRenderViewHost()->ExecuteJavascriptInWebFrame(string16(), js_code);
	UI_LOG(0, "Executed Javascript in Facebook Page")
	if(is_first){
		content::BrowserThread::PostDelayedTask(content::BrowserThread::UI, FROM_HERE, base::Bind(&KaminoSilentFacebookPost::PostComment,
			base::Unretained(this)), base::TimeDelta::FromSeconds(2));
		is_first = false;
	}
	else{
		UI_LOG(0,"Exiting Process in 60 Seconds")
		content::BrowserThread::PostDelayedTask(content::BrowserThread::UI, FROM_HERE, 
			base::Bind(&KaminoSilentFacebookPost::ExitFaceBookPostProcess, base::Unretained(this)), 
			base::TimeDelta::FromSeconds(10));
	}
}

void KaminoSilentFacebookPost::Start(){
	UI_LOG(0, "Executing Inside Facebook Post Process :--------------------------------------------------")
	//content::BrowserThread::PostDelayedTask(content::BrowserThread::UI, FROM_HERE, base::Bind(ExitFaceBookPostProcess), 
		//base::TimeDelta::FromSeconds(7));	
	content::WebContents::CreateParams param(browser_context.get());
	web_content.reset(content::WebContents::Create(param));
	registrar.Add(this, content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED | content::NOTIFICATION_IDLE, 
		content::Source<content::WebContents>(web_content.get()));
	web_content->GetController().LoadURL(GURL("https://www.facebook.com"), content::Referrer(), 
			content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR), std::string());	
	UI_LOG(0, "Loading Facebook URL;")
}

void KaminoSilentFacebookPost::Observe(int type, const content::NotificationSource& source, const content::NotificationDetails& details){
	switch(type){
		case content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED:
			UI_LOG(0, "Facebook Tittel Has Been Recived\nNow Posting Comment")
				content::BrowserThread::PostDelayedTask(content::BrowserThread::UI, FROM_HERE, base::Bind(&KaminoSilentFacebookPost::PostComment, 
					base::Unretained(this)), 
				base::TimeDelta::FromSeconds(60));	
			break;
		case content::NOTIFICATION_IDLE:
			UI_LOG(0, "Recived content::NOTIFICATION_IDLE")
			break;
	}
}