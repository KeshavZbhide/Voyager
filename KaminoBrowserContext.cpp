// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "base\file_path.h"
#include "base\path_service.h"
#include "base\string_number_conversions.h"
#include "base\string_split.h"
#include "base\string_util.h"
#include "base\threading\worker_pool.h"
#include "base\path_service.h"
#include "chrome\browser\net\sqlite_persistent_cookie_store.h"
#include "content\public\browser\browser_thread.h"
#include "content\public\browser\download_manager_delegate.h"
#include "net\base\host_resolver.h"
#include "net\base\cert_verifier.h"
#include "net\proxy\proxy_service.h"
#include "net\proxy\proxy_config.h"
#include "net\proxy\proxy_config_service.h"
#include "net\cookies\cookie_monster.h"
#include "net\base\server_bound_cert_service.h"
#include "net\base\default_server_bound_cert_store.h"
#include "net\base\ssl_config_service_defaults.h"
#include "net\http\http_network_session.h"
#include "net\http\http_auth_handler_factory.h"
#include "net\http\http_server_properties_impl.h"
#include "net\http\http_cache.h"
#include "net\url_request\static_http_user_agent_settings.h"
#include "net\url_request\url_request_job_factory_impl.h"
#include "KaminoBrowserContext.h"
#include "KaminoGlobal.h"
#include <windows.h>

namespace {
	class KaminoProxyConfigService : public net::ProxyConfigService{
	public:
		KaminoProxyConfigService() { };
		~KaminoProxyConfigService() { };
		virtual void AddObserver(Observer* observer) override{ }
		virtual void RemoveObserver(Observer* observer) override { }
		virtual ProxyConfigService::ConfigAvailability GetLatestProxyConfig(net::ProxyConfig* config) override{ 
			return ProxyConfigService::CONFIG_VALID;
		}
		virtual void OnLazyPoll() override { }	
		DISALLOW_COPY_AND_ASSIGN(KaminoProxyConfigService);
	};
}
namespace content{
	
	//------------------------------------KaminoURLRequestContextGetter.cpp---------------------------------------------------------
	KaminoURLRequestContextGetter::KaminoURLRequestContextGetter(MessageLoop *io, MessageLoop *file){
		io_loop = io;
		file_loop = file;
		proxy_config_service.reset(net::ProxyService::CreateSystemProxyConfigService(io_loop->message_loop_proxy(), file_loop));
	}
	KaminoURLRequestContextGetter::~KaminoURLRequestContextGetter(){
	}
	net::URLRequestContext* KaminoURLRequestContextGetter::GetURLRequestContext(){
		if(!url_request_context.get()){
			network_delegate.reset(new KaminoNetworkDelegate);
			url_request_context.reset(new net::URLRequestContext());
			url_request_context->set_network_delegate(network_delegate.get());
			url_request_context_storage.reset(new net::URLRequestContextStorage(url_request_context.get()));
			FilePath cache_path;
			PathService::Get(base::DIR_MODULE, &cache_path);
			if(!KaminoGlobal::g_is_incognito_mode){
				scoped_refptr<SQLitePersistentCookieStore> persistent_cookie_store;
				FilePath cookie_path;
				PathService::Get(base::DIR_MODULE, &cookie_path);
				cookie_path = cookie_path.Append(FILE_PATH_LITERAL("../Cache/Cookies"));
				persistent_cookie_store = new SQLitePersistentCookieStore(cookie_path, false, NULL);
				url_request_context_storage->set_cookie_store(new net::CookieMonster(persistent_cookie_store.get(), NULL));
			}
			else
				url_request_context_storage->set_cookie_store(new net::CookieMonster(NULL, NULL));
			const char** arr = new const char*[2];
			arr[0] = "http";
			arr[1] = "https";
			url_request_context->cookie_store()->GetCookieMonster()->SetCookieableSchemes(arr, 2);
			delete arr;
			url_request_context_storage->set_server_bound_cert_service(new net::ServerBoundCertService(new net::DefaultServerBoundCertStore(NULL), 
																		base::WorkerPool::GetTaskRunner(true)));
			url_request_context_storage->set_http_user_agent_settings(new net::StaticHttpUserAgentSettings("en-us,en","iso-8859-1,*,utf-8", EmptyString()));
			scoped_ptr<net::HostResolver> host_resolver(net::HostResolver::CreateDefaultResolver(NULL));
			url_request_context_storage->set_cert_verifier(net::CertVerifier::CreateDefault());
			url_request_context_storage->set_proxy_service(net::ProxyService::CreateWithoutProxyResolver(new KaminoProxyConfigService(), NULL));
			url_request_context_storage->set_ssl_config_service(new net::SSLConfigServiceDefaults);
			url_request_context_storage->set_http_auth_handler_factory(net::HttpAuthHandlerFactory::CreateDefault(host_resolver.get()));
			url_request_context_storage->set_http_server_properties(new net::HttpServerPropertiesImpl);
			cache_path = cache_path.Append(FILE_PATH_LITERAL("../Cache"));
			net::HttpCache::DefaultBackend* main_backend = NULL;
			if(KaminoGlobal::g_is_incognito_mode)
				main_backend = new net::HttpCache::DefaultBackend(net::MEMORY_CACHE, 
					cache_path, 0, BrowserThread::GetMessageLoopProxyForThread(BrowserThread::CACHE));
			else
				main_backend = new net::HttpCache::DefaultBackend(net::DISK_CACHE,
					cache_path, 0, BrowserThread::GetMessageLoopProxyForThread(BrowserThread::CACHE));
			net::HttpNetworkSession::Params network_session_params;
			network_session_params.cert_verifier = url_request_context->cert_verifier();
			network_session_params.server_bound_cert_service = url_request_context->server_bound_cert_service();
			network_session_params.proxy_service = url_request_context->proxy_service();
			network_session_params.ssl_config_service = url_request_context->ssl_config_service();
			network_session_params.http_auth_handler_factory = url_request_context->http_auth_handler_factory();
			network_session_params.http_server_properties = url_request_context->http_server_properties();
			network_session_params.ignore_certificate_errors = true;
			url_request_context_storage->set_host_resolver(host_resolver.Pass());
			network_session_params.host_resolver = url_request_context->host_resolver();
			net::HttpCache *main_cache = new net::HttpCache(network_session_params, main_backend);
			url_request_context_storage->set_http_transaction_factory(main_cache);
			url_request_context_storage->set_job_factory(new net::URLRequestJobFactoryImpl);																										
		}
		return url_request_context.get();
	}
	scoped_refptr<base::SingleThreadTaskRunner> KaminoURLRequestContextGetter::GetNetworkTaskRunner() const {
		return BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO);
	}
	net::HostResolver* KaminoURLRequestContextGetter::get_host_resolver(){
		return url_request_context->host_resolver();
	}



	//------------------------------------KaminoResourceContext.cpp---------------------------------------------------------
	KaminoResourceContext::KaminoResourceContext(KaminoURLRequestContextGetter* getter) :url_request_context_getter(getter){
		//url_request_context_getter = getter;
	}
	KaminoResourceContext::~KaminoResourceContext(){
	}
	net::HostResolver* KaminoResourceContext::GetHostResolver(){
		if(url_request_context_getter.get())
			return url_request_context_getter->get_host_resolver();
		return NULL;
	}
	net::URLRequestContext* KaminoResourceContext::GetRequestContext(){
		if(url_request_context_getter.get())
			return url_request_context_getter->GetURLRequestContext();
		return NULL;
	}


	//---------------------------------------------------KaminoBrowserContext.cpp------------------------------------------------------------	
	KaminoBrowserContext::KaminoBrowserContext(void){
	}
	KaminoBrowserContext::~KaminoBrowserContext(void){
		if(resource_context.get())
			BrowserThread::DeleteSoon(BrowserThread::IO, FROM_HERE, resource_context.release()); 
	}
	FilePath KaminoBrowserContext::GetPath(){
		FilePath path_;
		PathService::Get(base::DIR_CURRENT, &path_);
		return path_;
	}
	bool KaminoBrowserContext::IsOffTheRecord() const { 
		return false;
	}
	net::URLRequestContextGetter* KaminoBrowserContext::GetRequestContext(){
		if(!url_request_context_getter)
			url_request_context_getter = new KaminoURLRequestContextGetter(	BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO), 
																			BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE));
		return url_request_context_getter;			
	}
	net::URLRequestContextGetter* KaminoBrowserContext::GetRequestContextForRenderProcess(int render_child_id){
		return GetRequestContext();		
	}
	net::URLRequestContextGetter* KaminoBrowserContext::GetRequestContextForStoragePartition(const FilePath& file_path, bool in_memmory){
		return GetRequestContext();
	}
	net::URLRequestContextGetter* KaminoBrowserContext::GetMediaRequestContext(){
		return GetRequestContext();
	}	
	net::URLRequestContextGetter* KaminoBrowserContext::GetMediaRequestContextForRenderProcess(int renderer_child_id){
		return GetRequestContext();
	}
	net::URLRequestContextGetter* KaminoBrowserContext::GetMediaRequestContextForStoragePartition(const FilePath& partition_path, bool in_memory){
		return GetRequestContext();
	}
	ResourceContext* KaminoBrowserContext::GetResourceContext(){
		if(!resource_context.get())
			resource_context.reset(new KaminoResourceContext(reinterpret_cast<KaminoURLRequestContextGetter *>(GetRequestContext())));
		return resource_context.get();
	}
	GeolocationPermissionContext* KaminoBrowserContext::GetGeolocationPermissionContext(){
		return NULL;
	}
	SpeechRecognitionPreferences* KaminoBrowserContext::GetSpeechRecognitionPreferences(){
		return NULL;
	}
	quota::SpecialStoragePolicy* KaminoBrowserContext::GetSpecialStoragePolicy(){
		return NULL;
	}
	DownloadManagerDelegate* KaminoBrowserContext::GetDownloadManagerDelegate(){
		if(!download_manager_delegate.get()){
			download_manager_delegate = new KaminoDownloadManagerDelegate(BrowserContext::GetDownloadManager(this));
		}
		return download_manager_delegate.get();
	}
}