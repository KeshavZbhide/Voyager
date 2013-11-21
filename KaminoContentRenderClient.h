// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_CONTENT_RENDER_CLIENT_H
#define KAMINO_CONTENT_RENDER_CLIENT_H

#include "content\public\renderer\content_renderer_client.h"

class WebKit::WebFrame;

namespace content{

class KaminoRenderProcessObserver;

class KaminoContentRenderClient : public  ContentRendererClient{
	public:

		struct RenderViewList{
			RenderViewList(RenderView *v, RenderViewList *p);
			~RenderViewList();
			RenderView *view;
			RenderViewList *next;
			RenderViewList *prev;
		};

		KaminoContentRenderClient();
		virtual ~KaminoContentRenderClient();
		virtual void RenderThreadStarted() OVERRIDE;
		virtual void RenderViewCreated(RenderView* render_view) OVERRIDE;
		RenderView *GetRenderViewForFrame(WebKit::WebFrame *frame);
		RenderViewList *assosiated_render_views;
private:
		scoped_ptr<KaminoRenderProcessObserver> render_process_observer;
};

}



#endif;