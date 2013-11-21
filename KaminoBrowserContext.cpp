// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "base\files\file_path.h"
#include "base\path_service.h"
#include "base\strings\string_number_conversions.h"
#include "base\strings\string_split.h"
#include "base\strings\string_util.h"
#include "base\threading\worker_pool.h"
#include "base\path_service.h"

#include "content\browser\net\sqlite_persistent_cookie_store.h"
#include "content\public\browser\browser_thread.h"
#include "content\public\browser\download_manager_delegate.h"
#include "content\public\browser\storage_partition.h"
#include "content\public\common\url_constants.h"
#include "net\dns\host_resolver.h"
#include "net\cert\cert_verifier.h"
#include "net\proxy\proxy_service.h"
#include "net\proxy\proxy_config.h"
#include "net\proxy\proxy_config_service.h"
#include "net\cookies\cookie_monster.h"
#include "net\ssl\server_bound_cert_service.h"
#include "net\ssl\default_server_bound_cert_store.h"
#include "net\ssl\ssl_config_service_defaults.h"
#include "net\http\http_network_session.h"
#include "net\http\http_auth_handler_factory.h"
#include "net\http\http_server_properties_impl.h"
#include "net\http\http_cache.h"
#include "net\url_request\static_http_user_agent_settings.h"
#include "net\url_request\url_request_job_factory_impl.h"

#include "net\url_request\data_protocol_handler.h"
#include "net\url_request\file_protocol_handler.h"

#include "base\threading\sequenced_worker_pool.h"
#include "base\threading\worker_pool.h"

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
	KaminoURLRequestContextGetter::KaminoURLRequestContextGetter(base::MessageLoop *io, base::MessageLoop *file, 
		net::NetLog *net_log, ProtocolHandlerMap *protocol_handlers){
		io_loop = io;
		file_loop = file;
		net_log_ = net_log;
		std::swap(protocol_handlers_, *protocol_handlers);
		proxy_config_service.reset(net::ProxyService::CreateSystemProxyConfigService(
            io_loop->message_loop_proxy().get(), file_loop));
	}
	
	KaminoURLRequestContextGetter::~KaminoURLRequestContextGetter(){
	}
	
	net::URLRequestContext* KaminoURLRequestContextGetter::GetURLRequestContext(){
		if(!url_request_context.get()){
			
			network_delegate.reset(new KaminoNetworkDelegate);
			url_request_context.reset(new net::URLRequestContext());
			url_request_context->set_network_delegate(network_delegate.get());
			url_request_context->set_net_log(net_log_);

			storage_.reset(new net::URLRequestContextStorage(url_request_context.get()));
			base::FilePath cache_path;
			PathService::Get(base::DIR_MODULE, &cache_path);
			if(!KaminoGlobal::g_is_incognito_mode){
				scoped_refptr<SQLitePersistentCookieStore> persistent_cookie_store;
				base::FilePath cookie_path;
				PathService::Get(base::DIR_MODULE, &cookie_path);
				cookie_path = cookie_path.Append(FILE_PATH_LITERAL("../Cache/Cookies"));
				persistent_cookie_store = new SQLitePersistentCookieStore(cookie_path, 
											BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO),
											BrowserThread::GetMessageLoopProxyForThread(BrowserThread::DB),
											false, NULL);
				storage_->set_cookie_store(new net::CookieMonster(persistent_cookie_store.get(), NULL));
			}
			else
				storage_->set_cookie_store(new net::CookieMonster(NULL, NULL));
			const char** arr = new const char*[2];
			arr[0] = "http";
			arr[1] = "https";
			url_request_context->cookie_store()->GetCookieMonster()->SetCookieableSchemes(arr, 2);
			delete arr;
			storage_->set_server_bound_cert_service(new net::ServerBoundCertService(new net::DefaultServerBoundCertStore(NULL), 
																		base::WorkerPool::GetTaskRunner(true)));
			storage_->set_http_user_agent_settings(new net::StaticHttpUserAgentSettings("en-us,en", EmptyString()));
			
			scoped_ptr<net::HostResolver> host_resolver = net::HostResolver::CreateDefaultResolver(NULL);
			storage_->set_cert_verifier(net::CertVerifier::CreateDefault());
			storage_->set_transport_security_state(new net::TransportSecurityState);
			
			//TODO Check the proxy resolver...
			storage_->set_proxy_service(net::ProxyService::CreateWithoutProxyResolver(new KaminoProxyConfigService(), NULL));
			
			storage_->set_ssl_config_service(new net::SSLConfigServiceDefaults);
			storage_->set_http_auth_handler_factory(net::HttpAuthHandlerFactory::CreateDefault(host_resolver.get()));
			storage_->set_http_server_properties(scoped_ptr<net::HttpServerProperties>(new net::HttpServerPropertiesImpl()));
			
			cache_path = cache_path.Append(FILE_PATH_LITERAL("../Cache"));
			net::HttpCache::DefaultBackend* main_backend = NULL;
			if(KaminoGlobal::g_is_incognito_mode)
				main_backend = new net::HttpCache::DefaultBackend(net::MEMORY_CACHE, net::CACHE_BACKEND_DEFAULT,
					cache_path, 0, BrowserThread::GetMessageLoopProxyForThread(BrowserThread::CACHE).get());
			else
				main_backend = new net::HttpCache::DefaultBackend(net::DISK_CACHE, net::CACHE_BACKEND_DEFAULT,
					cache_path, 0, BrowserThread::GetMessageLoopProxyForThread(BrowserThread::CACHE).get());

			net::HttpNetworkSession::Params network_session_params;
			network_session_params.cert_verifier = url_request_context->cert_verifier();
			network_session_params.server_bound_cert_service = url_request_context->server_bound_cert_service();
			network_session_params.proxy_service = url_request_context->proxy_service();
			network_session_params.ssl_config_service = url_request_context->ssl_config_service();
			network_session_params.http_auth_handler_factory = url_request_context->http_auth_handler_factory();
			network_session_params.http_server_properties = url_request_context->http_server_properties();
			network_session_params.ignore_certificate_errors = true;
			network_session_params.transport_security_state = url_request_context->transport_security_state();
			network_session_params.network_delegate = url_request_context->network_delegate();
			network_session_params.net_log = url_request_context->net_log();

			storage_->set_host_resolver(host_resolver.Pass());
			network_session_params.host_resolver = url_request_context->host_resolver();
			net::HttpCache *main_cache = new net::HttpCache(network_session_params, main_backend);
			storage_->set_http_transaction_factory(main_cache);
			
			scoped_ptr<net::URLRequestJobFactoryImpl> job_factory(new net::URLRequestJobFactoryImpl());
			for(ProtocolHandlerMap::iterator it = protocol_handlers_.begin(); it != protocol_handlers_.end(); ++it){
				job_factory->SetProtocolHandler(it->first, it->second.release());
			}
		
			bool set_protocol = job_factory->SetProtocolHandler(chrome::kDataScheme, new net::DataProtocolHandler);
			set_protocol = job_factory->SetProtocolHandler(chrome::kFileScheme, 
				new net::FileProtocolHandler(
					content::BrowserThread::GetBlockingPool()->GetTaskRunnerWithShutdownBehavior(base::SequencedWorkerPool::SKIP_ON_SHUTDOWN)
				)
			);
			storage_->set_job_factory(job_factory.release());																										
		}
		return url_request_context.get();
	}

	scoped_refptr<base::SingleThreadTaskRunner> KaminoURLRequestContextGetter::GetNetworkTaskRunner() const {
		return BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO);
	}
	
	net::HostResolver* KaminoURLRequestContextGetter::host_resolver(){
		return url_request_context->host_resolver();
	}



	//------------------------------------KaminoResourceContext.cpp---------------------------------------------------------

class KaminoResourceContext : public ResourceContext {
public:
	KaminoResourceContext() : getter_(NULL) {}
	virtual ~KaminoResourceContext() {}
	
	// ResourceContext implementation:
	virtual net::HostResolver* GetHostResolver() OVERRIDE {
		CHECK(getter_);
		return getter_->host_resolver();
	}
	
	virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
		CHECK(getter_);
		return getter_->GetURLRequestContext();
	}
	
	virtual bool AllowMicAccess(const GURL& origin) OVERRIDE {
	    return false;
	}
	
	virtual bool AllowCameraAccess(const GURL& origin) OVERRIDE {
		return false;
	}

	void set_url_request_context_getter(KaminoURLRequestContextGetter* getter) {
		getter_ = getter;
	}
 private:
  KaminoURLRequestContextGetter* getter_;
  DISALLOW_COPY_AND_ASSIGN(KaminoResourceContext);
};


	//---------------------------------------------------KaminoBrowserContext.cpp------------------------------------------------------------	
	KaminoBrowserContext::KaminoBrowserContext(bool off_record, net::NetLog *net_log) 
		:resource_context(new KaminoResourceContext), net_log_(net_log){		
	}

	KaminoBrowserContext::~KaminoBrowserContext(void){
		if(resource_context.get())
			BrowserThread::DeleteSoon(BrowserThread::IO, FROM_HERE, resource_context.release()); 
	}

	base::FilePath KaminoBrowserContext::GetPath() const{
		base::FilePath path_;
		PathService::Get(base::DIR_CURRENT, &path_);
		return path_;
	}

	bool KaminoBrowserContext::IsOffTheRecord() const { 
		return false;
	}
	
	net::URLRequestContextGetter* KaminoBrowserContext::GetRequestContext(){
		return GetDefaultStoragePartition(this)->GetURLRequestContext();
	}
	
	net::URLRequestContextGetter* KaminoBrowserContext::GetRequestContextForRenderProcess(int render_child_id){
		return GetRequestContext();		
	}
	
	net::URLRequestContextGetter* KaminoBrowserContext::GetMediaRequestContext(){
		return GetRequestContext();
	}	
	
	net::URLRequestContextGetter* KaminoBrowserContext::GetMediaRequestContextForRenderProcess(int renderer_child_id){
		return GetRequestContext();
	}
	
	net::URLRequestContextGetter* KaminoBrowserContext::GetMediaRequestContextForStoragePartition(const base::FilePath& partition_path, bool in_memory){
		return GetRequestContext();
	}
	
	ResourceContext* KaminoBrowserContext::GetResourceContext(){
		return resource_context.get();
	}
	
	GeolocationPermissionContext* KaminoBrowserContext::GetGeolocationPermissionContext(){
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

	void KaminoBrowserContext::RequestMIDISysExPermission(int render_process_id, int render_view_id, const GURL& requesting_frame,
		const MIDISysExPermissionCallback& callback){
			callback.Run(false);	
	}


	net::URLRequestContextGetter* KaminoBrowserContext::CreateRequestContext(ProtocolHandlerMap* protocol_handlers){	
		if(!url_request_context_getter){
			url_request_context_getter = new KaminoURLRequestContextGetter(
				BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO), 
				BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
				net_log_,
				protocol_handlers);
			resource_context->set_url_request_context_getter(url_request_context_getter.get());
		}
		return url_request_context_getter.get();
	}

	net::URLRequestContextGetter* KaminoBrowserContext::CreateRequestContextForStoragePartition(const base::FilePath& partition_path, 
		bool in_memory, ProtocolHandlerMap* protocol_handlers){	
		return NULL;
	}
}