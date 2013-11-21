// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoGlobal.h"
#include <window.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include "UIMain.h"
#include "KaminoUpdateProcess.h"
#include "base\file_util.h"
#include "third_party\libxml\chromium\libxml_utils.h"

namespace KaminoGlobal{

HWND g_taskbar_window = NULL;
bool g_is_taskbar_shown = true;
int g_screen_width = 0;
int g_screen_height = 0;
int g_taskbar_width  = 0;
int g_taskbar_height = 0;
bool g_is_window_fullscreen = false;
bool g_is_window_wide = true;
char *g_start_url = NULL;
char *g_ilike_url = NULL;
RECT g_ideal_rect;
bool g_is_incognito_mode = false;
bool g_is_update_process = false;
bool g_is_test_run = false;
bool g_is_init_process = false;
bool g_is_facebook_post_process = false;

wchar_t *g_executable_name = NULL;
char *g_current_version = NULL;
int g_current_version_int = INT_MAX;
void *ptr_a = NULL;


UINT* g_pCodePoints = NULL;
UINT16* g_pGlyphIndices = NULL;
DWRITE_GLYPH_METRICS *g_pGlyphMatrics = NULL;
FLOAT *g_pGlyphAdvances = NULL;

//Brush Informations

FLOAT g_active_tab_brush_r = 0.0f;
FLOAT g_active_tab_brush_g = 0.0f;
FLOAT g_active_tab_brush_b = 0.0f;
FLOAT g_active_tab_brush_a = 0.0f;

FLOAT g_inactive_tab_brush_r = 0.0f;
FLOAT g_inactive_tab_brush_g = 0.0f;
FLOAT g_inactive_tab_brush_b = 0.0f;
FLOAT g_inactive_tab_brush_a = 0.0f;

FLOAT g_tab_border_line_brush_r = 0.0f;
FLOAT g_tab_border_line_brush_g = 0.0f;
FLOAT g_tab_border_line_brush_b = 0.0f;
FLOAT g_tab_border_line_brush_a = 0.0f;

FLOAT g_tab_border_line_brush2_r = 0.0f;
FLOAT g_tab_border_line_brush2_g = 0.0f;
FLOAT g_tab_border_line_brush2_b = 0.0f;
FLOAT g_tab_border_line_brush2_a = 0.0f;

FLOAT g_tab_text_brush_r = 0.0f;
FLOAT g_tab_text_brush_g = 0.0f;
FLOAT g_tab_text_brush_b = 0.0f;
FLOAT g_tab_text_brush_a = 0.0f;

FLOAT g_text_active_opacity = 0.0f;
FLOAT g_text_inactive_opacity = 0.0f;

FLOAT g_background_brush_r = 1.0f;
FLOAT g_background_brush_g = 1.0f;
FLOAT g_background_brush_b = 1.0f;


//Back Button File...
char *g_back_button_file = NULL;

void RegisterVoidPointerA(void *ptr){
	ptr_a = ptr;
}

void ParseCurrentVersion();

void BuildStartUpUrl(){
	char *scheme = "file:///";
	char *htmlFile = "KaminoResource\\voyager.html";
	DWORD len = 0;
	len = GetCurrentDirectory(0, NULL);
	g_start_url = (char *)malloc((strlen(scheme))+(len)+(strlen(htmlFile)));
	strcpy(g_start_url, scheme);
	GetCurrentDirectoryA(len, g_start_url+(strlen(scheme)));
	strcpy(strrchr(g_start_url, '\\')+1, htmlFile);
}

void BuildILikeUrl(){
	char *scheme = "file:///";
	char *htmlFile = "KaminoResource\\ilikeHtm.html";
	DWORD len = 0;
	len = GetCurrentDirectory(0, NULL);
	g_ilike_url = (char *)malloc((strlen(scheme))+(len)+(strlen(htmlFile)));
	strcpy(g_ilike_url, scheme);
	GetCurrentDirectoryA(len, g_ilike_url+(strlen(scheme)));
	strcpy(strrchr(g_ilike_url, '\\')+1, htmlFile);		
}

bool CheckSwitch(wchar_t *cmd, wchar_t *swtch, int cmd_len, int swtch_len){
	
	if(cmd_len != 0){
		for(int i = 0; i < cmd_len; i++){
			bool is_match = false;
			for(int j = 0; (j < swtch_len); j++){
				if(i+j >= cmd_len){
					is_match = false;
					break;
				}
				is_match = (cmd[i + j] == swtch[j]);
				if(!is_match)
					break;
			}
			if(is_match){
				return true;
				break;
			}
		}	
	}
	return false;	
}

bool Initialize(wchar_t *cmd){
	SYSTEMTIME time;
	GetSystemTime(&time);
	int len = wcslen(cmd);
	ptr_a = NULL;
	wchar_t *incognito_switch = L"-incognito";
	RECT task_bar_rect;
	MONITORINFO mi = { sizeof(MONITORINFO) };
	g_taskbar_window = FindWindow(L"Shell_traywnd", NULL);
	g_is_taskbar_shown = true;
    GetWindowRect(g_taskbar_window, &task_bar_rect);
	g_taskbar_width = (task_bar_rect.right - task_bar_rect.left);
	g_taskbar_height = (task_bar_rect.bottom - task_bar_rect.top);
	HMONITOR hmon = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST);
	GetMonitorInfo(hmon, &mi);
	g_screen_width = mi.rcMonitor.right - mi.rcMonitor.left;
	g_screen_height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	g_is_window_fullscreen = false;
	g_is_window_wide = true;

	g_pCodePoints = new UINT[4];
	g_pGlyphIndices = new UINT16[4];
	g_pGlyphMatrics = new DWRITE_GLYPH_METRICS[4];
	g_pGlyphAdvances = new FLOAT[4];

	int width = static_cast<int>(KaminoGlobal::g_screen_width  - (KaminoGlobal::g_screen_width * 0.016)) ;
	int height = height = static_cast<int>((KaminoGlobal::g_screen_height - KaminoGlobal::g_taskbar_height) - 
		((KaminoGlobal::g_screen_height - KaminoGlobal::g_taskbar_height) * 0.016));
	g_ideal_rect.left = (KaminoGlobal::g_screen_width/2) - (width/2);
	g_ideal_rect.top = (KaminoGlobal::g_screen_height - KaminoGlobal::g_taskbar_height)/2 - (height/2);
	g_ideal_rect.bottom = height;
	g_ideal_rect.right = width;

	BuildStartUpUrl();
	BuildILikeUrl();
	//Check If Incognito ....
	if(len > 0){
		g_is_incognito_mode = CheckSwitch(cmd, incognito_switch, len, wcslen(incognito_switch));
		//Check if Update Porcess ...
		g_is_update_process = CheckSwitch(cmd, L"-update", len, wcslen(L"-update"));
		//Check if Test Run ....
		g_is_test_run = CheckSwitch(cmd, L"-test_run", len, wcslen(L"-test_run"));
		//Check if Init Process ...
		g_is_init_process = CheckSwitch(cmd, L"-system_init", len, wcslen(L"-system_init"));
		//Check if we need to make a facebook post
		g_is_facebook_post_process = CheckSwitch(cmd, L"-facebook_post", len, wcslen(L"-facebook_post"));
	}
	//GetModuleName
	g_executable_name = new wchar_t[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), g_executable_name, MAX_PATH);
	g_current_version = NULL;
	std::string current_version;	
	if(file_util::ReadFileToString(FilePath(L"../current_version.txt"), &current_version)){
		g_current_version = (char *)malloc(current_version.length()+2);
		ZeroMemory(g_current_version, current_version.length()+2);
		current_version.copy(g_current_version, current_version.length(), 0);
		g_current_version_int = atoi(g_current_version);
	}
	//ParseCurrentVersion();
	return true;
}

}