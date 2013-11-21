// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoUpdateStatus.h"
#include "googleurl\src\gurl.h"
#include "UIMain.h"

KaminoUpdateStatus::KaminoUpdateStatus(net::URLRequestContextGetter *getter) : net::URLFetcherDelegate(){
	fetcher = NULL;
	fetcher = net::URLFetcher::Create(0, GURL("https://s3-us-west-2.amazonaws.com/voyjor/current_version.txt"), net::URLFetcher::GET, this);	
	fetcher->SetRequestContext(getter);
}

KaminoUpdateStatus::~KaminoUpdateStatus(){
	if(fetcher)
		delete fetcher;
}

void KaminoUpdateStatus::Execute(base::Callback<void(std::string)> callback){
	exec = callback;
	fetcher->Start();
}

void KaminoUpdateStatus::OnURLFetchComplete(const net::URLFetcher* source){
	std::string response;
	source->GetResponseAsString(&response);
	UI_LOG(0, "Running Executinon Callback")
	exec.Run(response);
	delete this;
}

void KaminoUpdateStatus::OnURLFetchDownloadProgress(const net::URLFetcher* source, int64 current, int64 total){
						
}

void KaminoUpdateStatus::OnURLFetchDownloadData(const net::URLFetcher* source, scoped_ptr<std::string> download_data){

}

bool KaminoUpdateStatus::ShouldSendDownloadData(){
	return false;
}

void KaminoUpdateStatus::OnURLFetchUploadProgress(const net::URLFetcher* source, int64 current, int64 total){

}







