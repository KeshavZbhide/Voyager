// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_GLOBAL_H
#define KAMINO_GLOBAL_H

#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <Wincodec.h>


namespace KaminoGlobal{
	extern HWND g_taskbar_window;
	extern bool g_is_taskbar_shown;
	extern int g_screen_width;
	extern int g_screen_height;
	extern int g_taskbar_height;
	extern int g_taskbar_width;
	extern bool g_is_window_fullscreen;
	extern bool g_is_window_wide;
	extern void *ptr_a;
	extern UINT* g_pCodePoints;
	extern UINT16* g_pGlyphIndices;
	extern DWRITE_GLYPH_METRICS* g_pGlyphMatrics;
	extern FLOAT* g_pGlyphAdvances;
	extern char *g_start_url;
	extern char *g_ilike_url;
	extern RECT g_ideal_rect;
	extern bool g_is_incognito_mode;
	extern bool g_is_update_process;
	extern bool g_is_test_run;
	extern bool g_is_init_process;
	extern bool g_is_facebook_post_process;
	extern wchar_t *g_executable_name;

	extern int g_current_version_int;
	extern char *g_current_version;
	
	extern FLOAT g_active_tab_brush_r;
	extern FLOAT g_active_tab_brush_g;
	extern FLOAT g_active_tab_brush_b;
	extern FLOAT g_active_tab_brush_a;
	
	extern FLOAT g_inactive_tab_brush_r;
	extern FLOAT g_inactive_tab_brush_g;
	extern FLOAT g_inactive_tab_brush_b;
	extern FLOAT g_inactive_tab_brush_a;
	
	extern FLOAT g_tab_border_line_brush_r;
	extern FLOAT g_tab_border_line_brush_g;
	extern FLOAT g_tab_border_line_brush_b;
	extern FLOAT g_tab_border_line_brush_a;
	
	extern FLOAT g_tab_border_line_brush2_r;
	extern FLOAT g_tab_border_line_brush2_g;
	extern FLOAT g_tab_border_line_brush2_b;
	extern FLOAT g_tab_border_line_brush2_a;

	extern FLOAT g_tab_text_brush_r;
	extern FLOAT g_tab_text_brush_g;
	extern FLOAT g_tab_text_brush_b;
	extern FLOAT g_tab_text_brush_a;

	//Back Button File...
	extern char *g_back_button_file;

	extern FLOAT g_text_active_opacity;
	extern FLOAT g_text_inactive_opacity;

	extern FLOAT g_background_brush_r;
	extern FLOAT g_background_brush_g;
	extern FLOAT g_background_brush_b;

	bool Initialize(wchar_t *cmd);
	void RegisterVoidPointerA(void *ptr);
	void BuildAllShortcuts();
}

#endif