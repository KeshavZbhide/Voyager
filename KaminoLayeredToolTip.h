#ifndef KAMINO_LAYERED_TOOLTIP_H
#define KAMINO_LAYERED_TOOLTIP_H


#include "KTab.h"
#include "KaminoDirectResource.h"
#include <Windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include "windowsx.h"
#include <string>


class KTabManager;
class KTab;

class KaminoLayeredToolTip
{
public:
	KaminoLayeredToolTip(KTabManager *);
	~KaminoLayeredToolTip(void);
	HRESULT Initialize(HWND);
	HRESULT RenderText(const char *str, size_t len, KTab *);
	HRESULT BuildTextGeometry();
	void Hide();
	bool is_tooltip_shown;
	static LRESULT CALLBACK ToolTipWndProc(HWND, UINT, WPARAM, LPARAM);
	KTabManager *manager;
	KTab *current_tab;
	ID2D1DCRenderTarget *target;

private :
	HBITMAP layered_window_bitmap;
	BLENDFUNCTION blend_func;
	float font_size;
	KaminoTextFormat text_format;
	KaminoDirect2DBrush text_brush;
	KaminoDirect2DBrush text_border_brush;
	
	KaminoDirect2DPathGeometry text_geometry;

	int width;
	int height;

	UINT16* p_GlyphIndices;
	UINT* p_CodePoints;
	IDWriteFontFace *font_face;
	IDWriteFontFile *font_file;

	HWND tooltip_window;
	HWND app_window;
	WNDCLASSEX tooltip_wnd_cls;
	wchar_t *text;
	size_t input_len;
	RECT client_rect;
	HRESULT InitDeviceIndependantResource();
	HRESULT InitDeviceDependantResource();
	void DestroyDeviceDependantResource();
};




#endif