// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_UPDATE_STATUS_H
#define KAMINO_UPDATE_STATUS_H

#include "net\url_request\url_fetcher.h"
#include "net\url_request\url_fetcher_delegate.h"
#include "third_party\libxml\chromium\libxml_utils.h"
#include "base\callback.h"


class KaminoUpdateStatus : public net::URLFetcherDelegate{
public:
	net::URLFetcher *fetcher;
	base::Callback<void(std::string)> exec;
	KaminoUpdateStatus(net::URLRequestContextGetter *getter);
	~KaminoUpdateStatus();
	void Execute(base::Callback<void(std::string)> callback);
	virtual void OnURLFetchComplete(const net::URLFetcher* source) OVERRIDE;
	virtual void OnURLFetchDownloadProgress(const net::URLFetcher* source, int64 current, int64 total) OVERRIDE;
	virtual void OnURLFetchDownloadData(const net::URLFetcher* source, scoped_ptr<std::string> download_data) OVERRIDE;
	virtual bool ShouldSendDownloadData() OVERRIDE;
	virtual void OnURLFetchUploadProgress(const net::URLFetcher* source, int64 current, int64 total) OVERRIDE;
};


#endif