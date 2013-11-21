// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoWebContentViewDelegate.h"
#include "KaminoGlobal.h"
#include "content\public\browser\navigation_entry.h"
//#include "third_party\WebKit\Source\WebKit\chromium\public\WebContextMenuData.h"
#include "content\public\browser\web_contents_view.h"
#include "content\public\browser\web_contents_view_delegate.h"
#include "content\public\browser\render_view_host.h"
#include "content\public\browser\download_manager.h"
#include "content\public\browser\download_url_parameters.h"
#include "content\public\browser\download_save_info.h"
//#include "content\public\browser\resource_dispatcher_host.h"
#include "net\url_request\url_request.h"
#include "UIMain.h"
#include "KTab.h"
#include "content\public\browser\browser_context.h"


namespace content{


KaminoWebContentViewDelegate::KaminoWebContentViewDelegate(WebContents* content) : web_content(content)
{ 
	clipboard_mem = NULL;
}
KaminoWebContentViewDelegate::~KaminoWebContentViewDelegate(void)
{ 
	if(clipboard_mem != NULL)
		GlobalFree(clipboard_mem);
}

WebDragDestDelegate* KaminoWebContentViewDelegate::GetDragDestDelegate(){
	return NULL;
}

void KaminoWebContentViewDelegate::MakeContextMenuItem(HMENU menu, int menu_index, LPTSTR text, UINT id, bool enabled){
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_DATA | MIIM_STRING | MIIM_STATE;
	mii.fState = enabled ? MFS_ENABLED : (MF_DISABLED | MFS_GRAYED);
	mii.fType = MFT_STRING;
	mii.wID = id;
	mii.dwTypeData = text;
	InsertMenuItem(menu, menu_index, TRUE, &mii);
}

void KaminoWebContentViewDelegate::MenuItemSelected(int selection){
	switch (selection){
	case KaminoContextMenuItemCutId:
		web_content->GetRenderViewHost()->Cut();
		break;
	case KaminoContextMenuItemCopyId:
		web_content->GetRenderViewHost()->Copy();
		break;
	case KaminoContextMenuItemPasteId:
		web_content->GetRenderViewHost()->Paste();
		break;
	case KaminoContextMenuItemDeleteId:
		web_content->GetRenderViewHost()->Delete();
		break;
	case KaminoContextMenuItemOpenLinkId: 
		{
			reinterpret_cast<BasicUIMain *>(GetUIMain())->tab_manager->AddTab(&(params_.link_url));
			break;
		}
	case KaminoContextMenuItemCopyPageURLId:
		{
			std::string current_url = web_content->GetURL().spec();
			if(clipboard_mem != NULL)
				GlobalFree(clipboard_mem);
			clipboard_mem =  GlobalAlloc(GMEM_MOVEABLE, current_url.length() + 1);
			memcpy(GlobalLock(clipboard_mem), current_url.c_str(), current_url.length() + 1);
			GlobalUnlock(clipboard_mem);
			OpenClipboard(NULL);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, clipboard_mem);
			CloseClipboard();
		}
		break;
	case KaminoContextMenuItemCopyLinkId:{
			if(clipboard_mem != NULL)
				GlobalFree(clipboard_mem);
			clipboard_mem =  GlobalAlloc(GMEM_MOVEABLE, params_.link_url.spec().length() + 1);
			memcpy(GlobalLock(clipboard_mem), params_.link_url.spec().c_str(), params_.link_url.spec().length() + 1);
			GlobalUnlock(clipboard_mem);
			OpenClipboard(NULL);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, clipboard_mem);
			CloseClipboard();
		}
		break;
	case KaminoContextMenuItemSearchId:
		{
			std::string url("https://www.google.com/search?q=");
			int len = params_.selection_text.length();
			char *str = new char[len+1];
			for(int i = 0; i<len+1; i++)
				str[i] = 0;
			for(int i = 0; i < len; i++){
				str[i] = static_cast<char>(params_.selection_text[i]);
			}
			url = url + std::string(str);
			delete str;
			reinterpret_cast<BasicUIMain *>(GetUIMain())->tab_manager->AddTab(&GURL(url));
		}
		break;
	case KaminoContextMenuItemImageSearchId:{
			std::string trailing("&tbm=isch");
			std::string url("https://www.google.com/search?q=");
			int len = params_.selection_text.length();
			char *str = new char[len+1];
			for(int i = 0; i<len+1; i++)
				str[i] = 0;
			for(int i = 0; i < len; i++){
				str[i] = static_cast<char>(params_.selection_text[i]);
			}
			url = url + std::string(str) + trailing;
			delete str;
			reinterpret_cast<BasicUIMain *>(GetUIMain())->tab_manager->AddTab(&GURL(url));
		}
		break;
	case KaminoContextMenuItemYtSearchId:{
			std::string url("https://www.youtube.com/results?search_query=");
			int len = params_.selection_text.length();
			char *str = new char[len+1];
			for(int i = 0; i<len+1; i++)
				str[i] = 0;
			for(int i = 0; i < len; i++){
				str[i] = static_cast<char>(params_.selection_text[i]);
			}
			url = url + std::string(str);
			reinterpret_cast<BasicUIMain *>(GetUIMain())->tab_manager->AddTab(&GURL(url));
			delete str;
		}
		break;
	case KaminoContextMenuItemSaveImageAsId:
		{					
			const GURL& referrer = params_.frame_url.is_empty() ? params_.page_url : params_.frame_url;
			const GURL& url = params_.src_url;
			int64 post_id = -1;
			if (url == web_content->GetURL()) {
				const NavigationEntry* entry = web_content->GetController().GetActiveEntry();
				if(entry)
					post_id = entry->GetPostID();
			}
			DownloadManager *dlm = BrowserContext::GetDownloadManager(web_content->GetBrowserContext());
			scoped_ptr<DownloadUrlParameters> dl_params(DownloadUrlParameters::FromWebContents(web_content, url));
			if(!dl_params.get()){
				UI_LOG(0,"Unable to Build DL PARAMS")
			}
			dl_params->set_referrer(content::Referrer(referrer, params_.referrer_policy));
			dl_params->set_post_id(post_id);
			dl_params->set_prefer_cache(true);
			if (post_id >= 0)
				dl_params->set_method("POST");
			dl_params->set_prompt(true);
			dlm->DownloadUrl(dl_params.Pass());
		}													
		break;
	case KaminoContextMenuItemBackId:
		web_content->GetController().GoToOffset(-1);
		web_content->GetView()->Focus();
		break;
	case KaminoContextMenuItemForwardId:
		web_content->GetController().GoToOffset(1);
		web_content->GetView()->Focus();
		break;
	case KaminoContextMenuItemReloadId:
		web_content->GetController().Reload(false);
		web_content->GetView()->Focus();
		break;
	case KaminoContentMenuItemLikesId:
		web_content->GetController().LoadURL(GURL(KaminoGlobal::g_ilike_url), content::Referrer(), 
			content::PageTransitionFromInt(content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR), std::string());
		break;
	case KaminoContentMenuItemLikeId:{
			wchar_t *js_code = L"var str = \"\";"
			L"str += \"<div><img src='http://www.google.com/s2/favicons?domain=\"+document.domain+\"' /><a href='\"+"
			L"document.URL+\"'>\"+document.title+\"</a></div>\";"
			L"KaminoILike(str);";
			web_content->GetRenderViewHost()->ExecuteJavascriptInWebFrame(string16(), js_code);
			break;
		}
	case KaminoContextMenuItemRecentlyClosedId:{
			wchar_t *js_code = L"var history = KaminoGetRecentHistory();"
			L"str = \"<html><head><style>@font-face {font-family: Ubuntu; src: url( Ubuntu.ttf ) format(\\\"truetype\\\"); }"
			L" body{background-color: rgb(250, 250, 250); } div{ margin: 5px;} a{ font-size: 20px; font-style: normal; font-family:  'Ubuntu';}"
			L" img{ display: inline; padding-right: 1%; padding-bottom: -20%;}</style></head><body>\";"
			L"for(i = history.length-1; i > 0; i -= 2)"
			L"str += \"<div><img src=\\\"http://www.google.com/s2/favicons?domain=\"+history[i]+\"\\\" /><a href=\\\"\"+history[i]+\"\\\">\""
			L" + history[i-1]+\"</a></div>\";"
			L"str += \"</body></html>\";"
			L"document.write(str);";
			web_content->GetRenderViewHost()->ExecuteJavascriptInWebFrame(string16(), js_code);
			break;
		}
	case KaminoContextMenuItemIncognitoId:
		{
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			CreateProcess(KaminoGlobal::g_executable_name, L" -incognito", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
		}
		break;	
	case KaminoContextMenuItemNewWindowId:
		{
			STARTUPINFO info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;
			CreateProcess(KaminoGlobal::g_executable_name, L"", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);																								  
		}
		break;
	}
}

void KaminoWebContentViewDelegate::ShowContextMenu(const ContextMenuParams& params){
	params_ = params;
	base::MessageLoop::ScopedNestableTaskAllower allow(base::MessageLoop::current());
	bool has_link = !params_.unfiltered_link_url.is_empty();
	bool has_selection = !params_.selection_text.empty();
	HMENU menu = CreateMenu();
	HMENU sub_menu = CreatePopupMenu();
	AppendMenu(menu, MF_STRING | MF_POPUP, (UINT)sub_menu, L"");
	int index = 0;
	if(has_selection){
		MakeContextMenuItem(sub_menu, index++, L"Copy", KaminoContextMenuItemCopyId, ((params_.edit_flags & WebKit::WebContextMenuData::CanCopy) != 0));
		MakeContextMenuItem(sub_menu, index++, L"Google", KaminoContextMenuItemSearchId, true);
		MakeContextMenuItem(sub_menu, index++, L"Images", KaminoContextMenuItemImageSearchId, true);
		MakeContextMenuItem(sub_menu, index++, L"Youtube", KaminoContextMenuItemYtSearchId, true);
	}
	else if(has_link){
			MakeContextMenuItem(sub_menu, index++, L"Open link in new tab", KaminoContextMenuItemOpenLinkId, true);
			MakeContextMenuItem(sub_menu, index++, L"Save link as", KaminoContextMenuItemSaveLinkAsId, true);
			MakeContextMenuItem(sub_menu, index++, L"Copy link address", KaminoContextMenuItemCopyLinkId, true);
		}
		else{
			if((params_.edit_flags & WebKit::WebContextMenuData::CanPaste) != 0)
				MakeContextMenuItem(sub_menu, index++, L"Paste", KaminoContextMenuItemPasteId, true);
			else
				if(web_content->GetURL().SchemeIsFile()){
					MakeContextMenuItem(sub_menu, index++, L"Like's", KaminoContentMenuItemLikesId, true);
					MakeContextMenuItem(sub_menu, index++, L"Incognito", KaminoContextMenuItemIncognitoId, true);
					MakeContextMenuItem(sub_menu, index++, L"New Window", KaminoContextMenuItemNewWindowId, true);
				}
				else					
					MakeContextMenuItem(sub_menu, index++, L"Like", KaminoContentMenuItemLikeId, true);
			MakeContextMenuItem(sub_menu, index++, L"Back", KaminoContextMenuItemBackId, web_content->GetController().CanGoBack());
			MakeContextMenuItem(sub_menu, index++, L"Forward", KaminoContextMenuItemForwardId, web_content->GetController().CanGoForward());
			MakeContextMenuItem(sub_menu, index++, L"Reload", KaminoContextMenuItemReloadId, true);
		}
	MakeContextMenuItem(sub_menu, index++, L"Save Image As", KaminoContextMenuItemSaveImageAsId, (params.media_type == WebKit::WebContextMenuData::MediaTypeImage));
	MakeContextMenuItem(sub_menu, index++, L"Copy page URL", KaminoContextMenuItemCopyPageURLId, true);
	MakeContextMenuItem(sub_menu, index++, L"Recently Closed", KaminoContextMenuItemRecentlyClosedId, true);
	gfx::Point screen_point(params.x, params.y);
	POINT point = screen_point.ToPOINT();
	ClientToScreen(web_content->GetView()->GetNativeView(), &point);
	int selection = TrackPopupMenu(sub_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, 0, 
												web_content->GetView()->GetContentNativeView(), NULL);
	MenuItemSelected(selection);
	DestroyMenu(menu);
}

void KaminoWebContentViewDelegate::StoreFocus(){ };
void KaminoWebContentViewDelegate::RestoreFocus(){ };
bool KaminoWebContentViewDelegate::Focus(){ return false; };
void KaminoWebContentViewDelegate::TakeFocus(bool reverse){ };
void KaminoWebContentViewDelegate::SizeChanged(const gfx::Size& size){ };
}