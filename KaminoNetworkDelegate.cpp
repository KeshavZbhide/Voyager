// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoNetworkDelegate.h"
#include "net\base\net_errors.h"

namespace content{
	KaminoNetworkDelegate::KaminoNetworkDelegate(void)
	{ }
	KaminoNetworkDelegate::~KaminoNetworkDelegate(void)
	{ }
	int KaminoNetworkDelegate::OnBeforeURLRequest(net::URLRequest* request, const net::CompletionCallback& callback, GURL* new_url){
		return net::OK;
	}
	int KaminoNetworkDelegate::OnBeforeSendHeaders(net::URLRequest* request, const net::CompletionCallback& callback, net::HttpRequestHeaders* headers){
		return net::OK;
	}
	void KaminoNetworkDelegate::OnSendHeaders(net::URLRequest* request, const net::HttpRequestHeaders& headers){
	}
	int KaminoNetworkDelegate::OnHeadersReceived(net::URLRequest* request,
												const net::CompletionCallback& callback,
												const net::HttpResponseHeaders* original_response_headers,
												scoped_refptr<net::HttpResponseHeaders>*
												override_response_headers){
		return net::OK;
	}
	void KaminoNetworkDelegate::OnBeforeRedirect(net::URLRequest* request, const GURL& new_location){
	}
	void KaminoNetworkDelegate::OnResponseStarted(net::URLRequest* request){
	}
	void KaminoNetworkDelegate::OnRawBytesRead(const net::URLRequest& request, int bytes_read){
	}
	void KaminoNetworkDelegate::OnCompleted(net::URLRequest* request, bool started){
	}
	void KaminoNetworkDelegate::OnURLRequestDestroyed(net::URLRequest* request){	
	}
	void KaminoNetworkDelegate::OnPACScriptError(int line_number, const string16& error){
	}
	KaminoNetworkDelegate::AuthRequiredResponse KaminoNetworkDelegate::OnAuthRequired(net::URLRequest* request, 
							const net::AuthChallengeInfo& auth_info, const AuthCallback& callback, 
							net::AuthCredentials* credentials){
		return AUTH_REQUIRED_RESPONSE_NO_ACTION;
	}
	bool KaminoNetworkDelegate::OnCanGetCookies(const net::URLRequest& request, const net::CookieList& cookie_list){
		return true;
	}
	bool KaminoNetworkDelegate::OnCanSetCookie(const net::URLRequest& request, const std::string& cookie_line, net::CookieOptions* options){
		return true;
	}
	bool KaminoNetworkDelegate::OnCanAccessFile(const net::URLRequest& request, const base::FilePath& path) const {
		return true;
	}
	bool KaminoNetworkDelegate::OnCanThrottleRequest(const net::URLRequest& request) const {
		return true;
	}
	int KaminoNetworkDelegate::OnBeforeSocketStreamConnect(net::SocketStream* stream, const net::CompletionCallback& callback){
		return net::OK;
	}
	void KaminoNetworkDelegate::OnRequestWaitStateChange(const net::URLRequest& request, RequestWaitState state){
	}
}