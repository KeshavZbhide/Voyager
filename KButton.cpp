// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KButton.h"
#include "UIMain.h"
#include "KaminoGlobal.h"

#define LINE_WIDTH 1.2f

KBackButton::KBackButton(KTabManager *m, int l, int w){
	manager = m;
}
KBackButton::~KBackButton(){}
HRESULT KBackButton::InitializeDeviceDependantResource(ID2D1RenderTarget *target){
	HRESULT hr = S_OK;
	if(image.Get() == NULL)
		hr = image.Reset(target, L"../KaminoResource/back6.png");
	return hr;
}


void KBackButton::OnClick(){
	manager->GoBack();
}
void KBackButton::Render(ID2D1RenderTarget *target){
	D2D1_SIZE_U size = image.Get()->GetPixelSize();
	float startX  = 15.0f;
	float startY = -1.0f * (size.height/4.5f);
	float endX = size.width+startX;
	float endY = size.height+startY;
	target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(((size.width)/2 + 5.0f), 17.5), 37.0f, 37.0f), manager->GiveMeTabBorderLineBrush(), LINE_WIDTH);
	target->DrawBitmap(image.Get(), D2D1::RectF(startX,startY, endX, endY), 1.0f); 	
}

KLikeButton::KLikeButton(KTabManager *m, int l, int w){
	manager = m;
}
KLikeButton::~KLikeButton(){
}
HRESULT KLikeButton::InitializeDeviceDependantResource(ID2D1RenderTarget *target){
	HRESULT hr = S_OK;
	if(image.Get() == NULL)
		hr = image.Reset(target, L"../KaminoResource/heart.png");
	return hr;
}
void KLikeButton::OnClick(){
}
void KLikeButton::Render(ID2D1RenderTarget *target){
	D2D1_SIZE_U size = image.Get()->GetPixelSize();
	float startX  = 1120.0f;
	float startY = (6.5f);
	float endX = static_cast<FLOAT>(size.width)+startX;
	float endY = static_cast<FLOAT>(size.height)+startY;
	target->DrawBitmap(image.Get(), D2D1::RectF(startX,startY, endX, endY), 1.0f); 	
}






KAddTabButton::KAddTabButton(KTabManager *m){
	hovering_over = false;
	manager = m;
}

KAddTabButton::~KAddTabButton(){}

HRESULT KAddTabButton::InitializeDeviceDependantResource(ID2D1RenderTarget *target){
	HRESULT hr = S_OK;
	return hr;
}

void KAddTabButton::Render(ID2D1RenderTarget *target){
	target->SetTransform(D2D1::Matrix3x2F::Identity());
	target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(static_cast<FLOAT>(manager->GetTabXSumLimit()), 20.0f), 8.0f, 8.0f), manager->GiveMeTabBorderLineBrush(), LINE_WIDTH);
	if(hovering_over)
		target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(static_cast<FLOAT>(manager->GetTabXSumLimit()), 20.0f), 5.0f,5.0f), manager->GiveMeTabBorderLineBrush(), LINE_WIDTH);
}

bool KAddTabButton::IfMouseIsHoveringOver(int x, bool &should_render_again){
	bool prev = hovering_over;
	if((x >= manager->GetTabXSumLimit() - 18) && (x <= manager->GetTabXSumLimit()+53))
		 hovering_over = true;
	else
		hovering_over = false;
	should_render_again = (prev != hovering_over);
	return hovering_over;
}

void KAddTabButton::MouseOut(){
	hovering_over = false;
}

bool KAddTabButton::IfClicked(int x){
	if(hovering_over)
		manager->AddTab(KaminoGlobal::g_start_url);
	return hovering_over;
}


KMinMaxCloseButton::KMinMaxCloseButton(KTabManager *m){				
	manager = m;
}

KMinMaxCloseButton::~KMinMaxCloseButton(){}

bool KMinMaxCloseButton::IfMouseIsHoveringOver(int x, bool &should_render_again){
	HoveringOver prev = hovering_over;
	if(x <= startX){
		hovering_over = HoveringOverNone;
		should_render_again = (prev != hovering_over);
		return false;
	}
	if((x >= startX) && (x <= startX + 22)){
		hovering_over = HoveringOverMin;
		should_render_again = (prev != hovering_over);
		return true;
	}
	if((x >= startX + 22) && (x <= startX + 44)){
		hovering_over = HoveringOverMax;
		should_render_again =  (prev != hovering_over);
		return true;
	}
	if((x >= startX + 44) && (x <= startX + 66)){
		hovering_over = HoveringOverClose;
		should_render_again = (prev != hovering_over);
		return true;
	}
	hovering_over = HoveringOverNone;
	return false;
}

bool KMinMaxCloseButton::IfClicked(int x){
	switch(hovering_over){
		case HoveringOverNone:
			return false;
		case HoveringOverMin:
			ShowWindow(manager->core_parent, SW_MINIMIZE);
			return true;
		case HoveringOverMax:
			if(KaminoGlobal::g_is_window_fullscreen){
				MoveWindow(manager->core_parent, KaminoGlobal::g_ideal_rect.left, KaminoGlobal::g_ideal_rect.top, 
					KaminoGlobal::g_ideal_rect.right, KaminoGlobal::g_ideal_rect.bottom, FALSE);
				ShowWindow(manager->core_parent, SW_RESTORE);
			}
			else{
				ShowWindow(manager->core_parent, SW_MAXIMIZE);
				KaminoGlobal::g_is_window_fullscreen = true;
			}
			return true;
		case HoveringOverClose:
			manager->CloseAllTabs();
			return true;
		default:
			return false;
	}
}

void KMinMaxCloseButton::MouseOut(){
	hovering_over = HoveringOverNone;
	return;
}

HRESULT KMinMaxCloseButton::InitializeDeviceDependantResource(ID2D1RenderTarget *target){
	HRESULT hr = S_OK;	
	if(red.Get() == NULL)
		hr = red.Reset(0.799f, 0.1627859f, 0.0f, 0.687f, target);
	if(green.Get() == NULL)
		hr = green.Reset(0.0f, 0.55972f, 0.0f, 0.687f, target);
	if(yello.Get() == NULL)
		hr = yello.Reset(0.959f, 0.7597f,  0.0f, 0.687f, target);
	return hr;
}

void KMinMaxCloseButton::Render(ID2D1RenderTarget *target){
	D2D1_SIZE_U size = target->GetPixelSize();
	float start = static_cast<FLOAT>(size.width - 65.0f);
	startX = static_cast<int>(start - 12);
	target->SetTransform(D2D1::Matrix3x2F::Identity());
	target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(start, 20.0f), 8.0f, 8.0f), manager->GiveMeTabBorderLineBrush(), LINE_WIDTH);
	if(hovering_over == HoveringOverMin)
		target->FillEllipse(D2D1::Ellipse(D2D1::Point2F(start, 20.0f), 5.0f, 5.0f), green.Get());
	start += 22;
	target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(start, 20.0f), 8.0f, 8.0f), manager->GiveMeTabBorderLineBrush(), LINE_WIDTH);
	if(hovering_over == HoveringOverMax)
		target->FillEllipse(D2D1::Ellipse(D2D1::Point2F(start, 20.0f), 5.0f, 5.0f), yello.Get());
	start += 22;
	target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(start, 20.0f), 8.0f, 8.0f), manager->GiveMeTabBorderLineBrush(), LINE_WIDTH);
	if(hovering_over == HoveringOverClose)
		target->FillEllipse(D2D1::Ellipse(D2D1::Point2F(start, 20.0f), 5.0f, 5.0f), red.Get());
}
