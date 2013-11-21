// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_RENDER_VIEW_HOST_H
#define KAMINO_REDNER_VIEW_HOST_H
#include "content\public\browser\render_view_host_observer.h"
#include "ipc\ipc_message_macros.h"
#include "content\public\browser\render_process_host.h"
#include "ipc\ipc_sync_message_filter.h"


#include <window.h>
#include <map>

namespace content{

//This Filter Runs on the IO Thread. remeber.
class KaminoSpecialCommunicator : public IPC::SyncMessageFilter{
public:
	KaminoSpecialCommunicator();
	inline void ForwardRecentHistoryTittels(std::vector<std::wstring> *vec);
	inline void ForwardRecentHistoryURLs(std::vector<std::string> *vec);
	void ForwardShouldShowIconsStaticVar(bool one, bool *two);
	virtual bool OnMessageReceived(const IPC::Message& message);
};

class KaminoRenderViewHostObserver : public RenderViewHostObserver{
public:
	KaminoRenderViewHostObserver(RenderViewHost *render_view_host);
	virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
	void LogFromRenderer(std::string msg);
	void ILikeFromRenderer(std::string content);
	void NewSymLinkFromRenderer(std::string key, std::string link);
};

}





#endif