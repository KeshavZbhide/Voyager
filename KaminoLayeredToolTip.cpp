#include "KaminoLayeredToolTip.h"
#include "KaminoGlobal.h"

KaminoLayeredToolTip::KaminoLayeredToolTip(KTabManager *m){
	manager = m;
	target = NULL;
	layered_window_bitmap = NULL;
	app_window = NULL;
	is_tooltip_shown = false;
	text = NULL;
	current_tab = NULL;

	p_GlyphIndices = NULL;
	p_CodePoints = NULL;
	font_face = NULL;
	font_file = NULL;

	tooltip_wnd_cls.cbClsExtra = 0;
	tooltip_wnd_cls.cbSize = sizeof(WNDCLASSEX);
	tooltip_wnd_cls.cbWndExtra = 0;
	tooltip_wnd_cls.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	tooltip_wnd_cls.hCursor = LoadCursor(NULL, IDC_ARROW);
	tooltip_wnd_cls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	tooltip_wnd_cls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	tooltip_wnd_cls.hInstance = GetModuleHandle(NULL);
	tooltip_wnd_cls.lpfnWndProc = ToolTipWndProc;
	tooltip_wnd_cls.lpszClassName = L"URLToolTip";
	tooltip_wnd_cls.lpszMenuName = NULL;
	tooltip_wnd_cls.style = (CS_HREDRAW|CS_VREDRAW);
	RegisterClassEx(&tooltip_wnd_cls);	

	blend_func.AlphaFormat =  AC_SRC_ALPHA;
	blend_func.BlendFlags = 0;
	blend_func.BlendOp = AC_SRC_OVER;
	blend_func.SourceConstantAlpha = 255;
}

KaminoLayeredToolTip::~KaminoLayeredToolTip(void){
	DeleteObject(layered_window_bitmap);
	if(p_GlyphIndices)
		delete p_GlyphIndices;
	if(p_CodePoints)
		delete p_CodePoints;
	if(font_face)
		font_face->Release();
	if(font_file)
		font_file->Release();
	if(target)
		target->Release();
	if(tooltip_window)
		DestroyWindow(tooltip_window);
	if(text)
		delete text;
}

HRESULT KaminoLayeredToolTip::Initialize(HWND hWnd){
	HRESULT hr = E_FAIL;
	tooltip_window = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED, tooltip_wnd_cls.lpszClassName, L"", WS_POPUP, 0, 
			0, 0, 0, manager->core_parent, NULL, tooltip_wnd_cls.hInstance, this);
	if(tooltip_window != NULL)
		hr = S_OK;
	hr = InitDeviceIndependantResource();
	return hr;
}

LRESULT CALLBACK KaminoLayeredToolTip::ToolTipWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){	
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

HRESULT KaminoLayeredToolTip::InitDeviceIndependantResource(){
	//SetLayeredWindowAttributes(tooltip_window, 0, (255 * 70) / 100, LWA_ALPHA);
	HRESULT hr = S_OK;
	KaminoDirect2DFactory factory;	
	const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
	const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, format);
	factory.Get()->CreateDCRenderTarget(&properties, &target);
	text_brush.Reset(0.145f, 0.145f, 0.145f, 0.9f, target);
	text_border_brush.Reset(0.666f, 0.666f, 0.666f, 0.15f, target);
	void *bits_holder;
	width = KaminoGlobal::g_ideal_rect.right - KaminoGlobal::g_ideal_rect.left;
	height = (KaminoGlobal::g_screen_height / 3);
	BITMAPINFO bitmap_info = { };
	bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
	bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
	bitmap_info.bmiHeader.biWidth = KaminoGlobal::g_ideal_rect.right - KaminoGlobal::g_ideal_rect.left;
	bitmap_info.bmiHeader.biHeight = 0 - (KaminoGlobal::g_screen_height / 3);
	layered_window_bitmap = CreateDIBSection(0, &bitmap_info, DIB_RGB_COLORS, &bits_holder, NULL, NULL);
	if(layered_window_bitmap == NULL){
		UI_LOG(0, "LayredWindowToolTip Bitmap == NULL")
		hr = E_FAIL;
	}
	font_size = KaminoGlobal::g_screen_height / 8.0f;
	//if(text_format.Get() == NULL)
		//hr = text_format.Reset(L"Ubuntu", font_size, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
	return hr;
}

HRESULT KaminoLayeredToolTip::InitDeviceDependantResource(){
	HRESULT hr = S_OK;		
	return hr;
}

void KaminoLayeredToolTip::DestroyDeviceDependantResource(){

}

HRESULT KaminoLayeredToolTip::RenderText(const char *str, size_t len, KTab *tab){

	if(len <= 1 || str[0] == '\0')
		return S_OK;
	if(text == NULL)
		text = new wchar_t[50];
	for(int i = 0; i<50; i++)
		text[i] = 0;
	int start = 0;
	if(str[0] == 'w' && str[1] == 'w' && str[2] == 'w' && str[3] == '.')
		start += 4;
	int limit = start;
	for(input_len = 0; input_len < (len - limit); input_len++, start++)
		text[input_len] = str[start];
	
	current_tab = tab;

	if(!is_tooltip_shown){
		ShowWindow(tooltip_window, SW_SHOW);
		is_tooltip_shown = true;
	}

	BuildTextGeometry();

	POINT p1; SIZE sz;
	POINT p2; p2.x = 0; p2.y = 0;
	sz.cx = width;
	sz.cy = height;
	
	if(KaminoGlobal::g_is_window_fullscreen){
		p1.x = 0;
		p1.y = KaminoGlobal::g_screen_height - height;
	}
	else{
		p1.x = KaminoGlobal::g_ideal_rect.left;
		p1.y = (KaminoGlobal::g_ideal_rect.bottom  - height) + KaminoGlobal::g_ideal_rect.top;
	}

	HDC bitmap_dc = CreateCompatibleDC(GetDC(NULL));
	HBITMAP old = (HBITMAP)SelectObject(bitmap_dc, layered_window_bitmap);

	if(target != NULL){	
		RECT sub_rect; sub_rect.left = 0; sub_rect.top = 0;
		sub_rect.right = KaminoGlobal::g_screen_width;
		sub_rect.bottom = height;
		target->BindDC(bitmap_dc, &sub_rect);
		target->BeginDraw();
		target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
		if((tab != NULL) && (tab->favicon.Get() != NULL)){
			D2D1_RECT_F rect;
			D2D1_SIZE_F size = current_tab->favicon.Get()->GetSize();
			float x = 16.0f - (size.width/2) + 16.0f;
			float y = 16.0f - (size.height/2) + (height/2) + (font_size/1.7);
			rect = D2D1::RectF(x, y, x+size.width, y+size.height);
			//rect = D2D1::RectF(10.0f, (height/2) - (size.height/2), size.width, (height/2) + (size.height/2));
			target->DrawBitmap(current_tab->favicon.Get(), rect, 1.0f);
		}
		target->SetTransform(D2D1::Matrix3x2F::Translation(64, (height/2) + (font_size)));
		target->DrawGeometry(text_geometry.Get(), text_border_brush.Get(), 2.0f);
		target->FillGeometry(text_geometry.Get(), text_brush.Get());	
		target->SetTransform(D2D1::Matrix3x2F::Identity());
		target->EndDraw();

		UpdateLayeredWindow(tooltip_window, NULL, &p1, &sz, bitmap_dc, &p2, 0, &blend_func, ULW_ALPHA);
	}
	ELSE_UI_LOG(0, "Target Is Null Hence Cant Paint KaminoLayeredToolTip")
	SelectObject(bitmap_dc, old);
	return S_OK;
}


void KaminoLayeredToolTip::Hide(){
	if(is_tooltip_shown){
		if(manager)
			manager->Focus();
		ShowWindow(tooltip_window, SW_HIDE);
		is_tooltip_shown = false;
		current_tab = NULL;
	}
}

HRESULT KaminoLayeredToolTip::BuildTextGeometry(){
	HRESULT hr = S_OK;
	KaminoWriteFactory write_factory;
	KaminoDirect2DFactory factory;
	
	if(p_CodePoints == NULL)
		p_CodePoints = new UINT[50];
	if(p_GlyphIndices == NULL)
		p_GlyphIndices = new UINT16[50];

	for(int i = 0; i < 50; i++){
		p_CodePoints[i] = 0;
		p_GlyphIndices[i] = 0;
	}

	if((write_factory.Get() == NULL) || (factory.Get() == NULL)){
		return E_FAIL;
	}
	if(font_file == NULL)
		hr = write_factory.Get()->CreateFontFileReference(L"../KaminoResource/Oxygen-Bold.ttf", NULL, &font_file);
	if(font_file == NULL){
		UI_LOG(0, "Font File Non Existant")
		return E_FAIL;
	}
	if(font_face == NULL)
		hr = write_factory.Get()->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE, 1, &font_file, 0, DWRITE_FONT_SIMULATIONS_NONE, &font_face);
	if(font_face == NULL){
		return E_FAIL;
	}
	size_t count;
	for(count = 0; (count < input_len) && (count < 50); count++)
		p_CodePoints[count] = static_cast<UINT>(text[count]);
	hr = font_face->GetGlyphIndicesW(p_CodePoints, count, p_GlyphIndices);
	if(FAILED(hr))
		return hr;
	hr = text_geometry.Reset();
	if(SUCCEEDED(hr)){
		ID2D1GeometrySink *sink = NULL;
		text_geometry.Get()->Open(&sink);
		hr = font_face->GetGlyphRunOutline(KaminoGlobal::g_screen_height / 10.0f, p_GlyphIndices, NULL, NULL, input_len, FALSE, FALSE, sink);
		if(FAILED(hr)){					
			return E_FAIL;
		}
		sink->Close();
	}
	return S_OK;
}