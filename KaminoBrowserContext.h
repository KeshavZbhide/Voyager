// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_BROWSER_CONTEXT_H
#define KAMINO_BROWSER_CONTEXT_H
#include "content\public\browser\browser_context.h"
#include "content\public\browser\resource_context.h"
#include "net\url_request\url_request_context_getter.h"
#include "net\url_request\url_request_context.h"
#include "net\url_request\url_request_context_storage.h"
#include "net\proxy\proxy_config_service.h"
#include "KaminoNetworkDelegate.h"
#include "KaminoDownloadManagerDelegate.h"

namespace content{

	class KaminoURLRequestContextGetter : public net::URLRequestContextGetter{
	public:
		KaminoURLRequestContextGetter(MessageLoop* io, MessageLoop *file);
		~KaminoURLRequestContextGetter();
		virtual net::URLRequestContext* GetURLRequestContext() override;
		virtual scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const override;
		net::HostResolver* get_host_resolver();
		MessageLoop* io_loop;
		MessageLoop* file_loop;
		scoped_ptr<net::URLRequestContext> url_request_context;
		scoped_ptr<KaminoNetworkDelegate> network_delegate;
		scoped_ptr<net::URLRequestContextStorage> url_request_context_storage;
		scoped_ptr<net::ProxyConfigService> proxy_config_service;
		DISALLOW_COPY_AND_ASSIGN(KaminoURLRequestContextGetter);
	};
	class KaminoResourceContext : public ResourceContext{
	public:
		explicit KaminoResourceContext(KaminoURLRequestContextGetter *getter);
		virtual ~KaminoResourceContext();
		virtual net::HostResolver* GetHostResolver() override;
		virtual net::URLRequestContext* GetRequestContext() override;
		scoped_refptr<KaminoURLRequestContextGetter> url_request_context_getter;
		DISALLOW_COPY_AND_ASSIGN(KaminoResourceContext);
	};	
	class KaminoBrowserContext : public BrowserContext
	{
	public:
		KaminoBrowserContext(void);
		~KaminoBrowserContext(void);
		virtual FilePath GetPath() override;
		virtual bool IsOffTheRecord() const override;
		virtual net::URLRequestContextGetter* GetRequestContext() override;
		virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(int renderer_child_id) override;
		virtual net::URLRequestContextGetter* GetRequestContextForStoragePartition( const FilePath& partition_path, bool in_memory) override;
		virtual net::URLRequestContextGetter* GetMediaRequestContext() override;
		virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(int renderer_child_id) override;
		virtual net::URLRequestContextGetter* GetMediaRequestContextForStoragePartition(const FilePath& partition_path, bool in_memory) override;
		virtual ResourceContext* GetResourceContext() override;
		virtual GeolocationPermissionContext* GetGeolocationPermissionContext() override;
		virtual SpeechRecognitionPreferences* GetSpeechRecognitionPreferences() override;
		virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() override;
		virtual DownloadManagerDelegate* GetDownloadManagerDelegate() override;
		scoped_refptr<net::URLRequestContextGetter> url_request_context_getter;
		scoped_ptr<ResourceContext> resource_context;
		scoped_refptr<KaminoDownloadManagerDelegate> download_manager_delegate;
		DISALLOW_COPY_AND_ASSIGN(KaminoBrowserContext);
	};
}
#endif