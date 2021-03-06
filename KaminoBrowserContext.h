// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_BROWSER_CONTEXT_H
#define KAMINO_BROWSER_CONTEXT_H

#include "base\basictypes.h"
#include "base\compiler_specific.h"
#include "content\public\browser\browser_context.h"
#include "content\public\browser\resource_context.h"
#include "content\public\browser\content_browser_client.h"
#include "net\url_request\url_request_context_getter.h"
#include "net\url_request\url_request_context.h"
#include "net\url_request\url_request_context_storage.h"
#include "net\proxy\proxy_config_service.h"
#include "base\files\file_path.h"
#include "KaminoNetworkDelegate.h"
#include "KaminoDownloadManagerDelegate.h"

namespace content{

	class KaminoResourceContext;

	class KaminoURLRequestContextGetter : public net::URLRequestContextGetter{	
	public:
		KaminoURLRequestContextGetter(base::MessageLoop* io, base::MessageLoop *file, 
			net::NetLog *net_log, ProtocolHandlerMap *protocol_handlers);
		//For UpdateProcess
		//KaminoURLRequestContextGetter(base::MessageLoop* io, base::MessageLoop *file);
		~KaminoURLRequestContextGetter();
		virtual net::URLRequestContext* GetURLRequestContext() override;
		virtual scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const override;
		net::HostResolver* host_resolver();
	private:
		base::MessageLoop* io_loop;
		base::MessageLoop* file_loop;
		net::NetLog *net_log_;
		ProtocolHandlerMap protocol_handlers_;
		scoped_ptr<net::URLRequestContext> url_request_context;
		scoped_ptr<KaminoNetworkDelegate> network_delegate;
		scoped_ptr<net::URLRequestContextStorage> storage_;
		scoped_ptr<net::ProxyConfigService> proxy_config_service;
		DISALLOW_COPY_AND_ASSIGN(KaminoURLRequestContextGetter);
	};


	class KaminoBrowserContext : public BrowserContext
	{
	public:
		KaminoBrowserContext(bool off_record, net::NetLog *net_log);
		~KaminoBrowserContext(void);
		virtual base::FilePath GetPath() const OVERRIDE;
		virtual bool IsOffTheRecord() const override;
		virtual net::URLRequestContextGetter* GetRequestContext() override;
		virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(int renderer_child_id) override;
		virtual net::URLRequestContextGetter* GetMediaRequestContext() override;
		virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(int renderer_child_id) override;
		virtual net::URLRequestContextGetter* GetMediaRequestContextForStoragePartition(const base::FilePath& partition_path, bool in_memory) override;
		virtual void RequestMIDISysExPermission(int render_process_id, int render_view_id, const GURL& requesting_frame,
			const MIDISysExPermissionCallback& callback) OVERRIDE;
		virtual ResourceContext* GetResourceContext() override;
		virtual GeolocationPermissionContext* GetGeolocationPermissionContext() override;
		virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
		virtual DownloadManagerDelegate* GetDownloadManagerDelegate() override;
		net::URLRequestContextGetter* CreateRequestContext(ProtocolHandlerMap* protocol_handlers);
		net::URLRequestContextGetter* CreateRequestContextForStoragePartition(const base::FilePath& partition_path, 
										bool in_memory, ProtocolHandlerMap* protocol_handlers);
	private:
		scoped_refptr<KaminoURLRequestContextGetter> url_request_context_getter;
		scoped_ptr<KaminoResourceContext> resource_context;
		net::NetLog *net_log_;
		scoped_refptr<KaminoDownloadManagerDelegate> download_manager_delegate;
		DISALLOW_COPY_AND_ASSIGN(KaminoBrowserContext);
	};
}
#endif