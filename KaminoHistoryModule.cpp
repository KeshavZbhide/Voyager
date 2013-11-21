// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoHistoryModule.h"
#include "base\path_service.h"
#include "base\platform_file.h"

#include "content\public\browser\browser_thread.h"

void CleanUpStack();


namespace KaminoHistoryStack{

	struct History{
	public:
		History(char *data)
			:url(data), tittle((wchar_t *)(data + url.length() + 1)){
			next = NULL;
		}
		History(const std::string &u, const std::wstring &t): url(u), tittle(t){
			next = NULL;
		};
		std::string url;
		std::wstring tittle;
		History *next;
	};

//Global Data Set	
	History *start = NULL;
	History *end = NULL;
	History *init_start = NULL;
	int num = 0;
	char *file_content = NULL;
	int64 file_size = 0;
	wchar_t* file_path = NULL;
	std::string *recent_history_json;
}


void KaminoHistoryModule::PreInitialize(){
	KaminoHistoryStack::start = NULL;
	KaminoHistoryStack::init_start = NULL;
	KaminoHistoryStack::end = NULL;
	KaminoHistoryStack::num = 0;
	KaminoHistoryStack::recent_history_json = NULL;
	KaminoHistoryStack::file_content = NULL;	
	wchar_t *file_name = L"KaminoResource\\History.file";
	int64 len = GetCurrentDirectory(0, NULL);
	KaminoHistoryStack::file_path = (wchar_t *)malloc((len + wcslen(file_name))*sizeof(wchar_t));
	GetCurrentDirectory(len, KaminoHistoryStack::file_path);
	wcscpy(wcsrchr(KaminoHistoryStack::file_path, '\\')+1, file_name);
	FilePath path(KaminoHistoryStack::file_path);
	base::PlatformFile file = base::CreatePlatformFile(path, base::PLATFORM_FILE_OPEN_ALWAYS | base::PLATFORM_FILE_WRITE | base::PLATFORM_FILE_READ,
		NULL, NULL);
	base::PlatformFileInfo info;
	base::GetPlatformFileInfo(file, &info);
	KaminoHistoryStack::file_size = info.size;
	if(info.size != 0){
		KaminoHistoryStack::file_content = (char *)malloc(info.size);
		base::ReadPlatformFile(file, 0, KaminoHistoryStack::file_content, info.size);				
	}
	base::ClosePlatformFile(file);
	BuildStack();
}

void KaminoHistoryModule::BuildStack(){
	if(KaminoHistoryStack::file_content == NULL)
		return;
	KaminoHistoryStack::start = new KaminoHistoryStack::History(KaminoHistoryStack::file_content);
	KaminoHistoryStack::init_start = KaminoHistoryStack::start;
	KaminoHistoryStack::num++;
	int pointer = KaminoHistoryStack::start->url.length() + 1 + (KaminoHistoryStack::start->tittle.length() * sizeof(wchar_t)) + sizeof(wchar_t);
	KaminoHistoryStack::History *it = KaminoHistoryStack::start;
	for(int i = 0; (pointer <= KaminoHistoryStack::file_size) && (i < 24); i++){
		it->next = new KaminoHistoryStack::History(KaminoHistoryStack::file_content + pointer);
		it = it->next;
		KaminoHistoryStack::end = it;
		pointer += it->url.length() + 1 + (it->tittle.length() * sizeof(wchar_t)) + sizeof(wchar_t);
		KaminoHistoryStack::num++;
	}
}

#ifdef UI_DEBUG_LOG
void KaminoHistoryModule::CatStack(){
	KaminoHistoryStack::History *it = NULL;
	for(it = KaminoHistoryStack::start; it != NULL; it = it->next)
		UI_LOG(0, ">> %s ", it->url.c_str());
}
#endif

void KaminoHistoryModule::Flush(){
	if((KaminoHistoryStack::start == NULL) && (KaminoHistoryStack::file_content == NULL))
		return;
	if((KaminoHistoryStack::start == NULL) && (KaminoHistoryStack::file_content != NULL)){
		FilePath path(KaminoHistoryStack::file_path);
		base::PlatformFile file = base::CreatePlatformFile(path, base::PLATFORM_FILE_OPEN_ALWAYS | base::PLATFORM_FILE_WRITE, NULL, NULL);
		base::TruncatePlatformFile(file, KaminoHistoryStack::file_size);
		base::WritePlatformFile(file, 0, KaminoHistoryStack::file_content, KaminoHistoryStack::file_size);
		base::ClosePlatformFile(file);
		return;
	}
	FilePath path(KaminoHistoryStack::file_path);
	base::PlatformFile file = base::CreatePlatformFile(path, base::PLATFORM_FILE_OPEN_ALWAYS | base::PLATFORM_FILE_WRITE, NULL, NULL);
	base::TruncatePlatformFile(file, 0);
	int position = 0;
//CleanUp.
	CleanUpStack();
	KaminoHistoryStack::History *it = KaminoHistoryStack::start;
	KaminoHistoryStack::History *prev = it;
	for(it = it->next; it != NULL; it = it->next){
		int len = prev->url.length() + 1;
		base::WritePlatformFile(file, position, prev->url.c_str(), len);
		int len2 = ((prev->tittle.length()+1) * sizeof(wchar_t));
		position += len;
		base::WritePlatformFile(file, position, (char *)(prev->tittle.c_str()), len2);
		position += len2;
		delete prev;
		prev = it;
	}
	if(prev != NULL){
		int len = prev->url.length() + 1;
		base::WritePlatformFile(file, position, prev->url.c_str(), len);
		int len2 = ((prev->tittle.length()+1) * sizeof(wchar_t));
		position += len;
		base::WritePlatformFile(file, position, (char *)(prev->tittle.c_str()), len2);
		position += len2;
		delete prev;
	}
	base::ClosePlatformFile(file);
	if(KaminoHistoryStack::file_content != NULL)
		free(KaminoHistoryStack::file_content);
	if(KaminoHistoryStack::file_path != NULL)
		free(KaminoHistoryStack::file_path);
}

void CleanUpStack(){
	if(KaminoHistoryStack::init_start == KaminoHistoryStack::start)
		return;
	KaminoHistoryStack::History *temp = KaminoHistoryStack::init_start;
	for(KaminoHistoryStack::History *it = temp->next; it != KaminoHistoryStack::start; it = it->next){
		KaminoHistoryStack::num--;
		delete temp;
		temp = it;
	}
	if(temp != NULL){
		KaminoHistoryStack::num--;
		delete temp;
	}
	KaminoHistoryStack::init_start = KaminoHistoryStack::start;	
}

void KaminoHistoryModule::push(const std::string &url, const std::wstring &tittle){
	if((KaminoHistoryStack::start == NULL) && (KaminoHistoryStack::end == NULL))
	{
		KaminoHistoryStack::start = new KaminoHistoryStack::History(url, tittle);
		KaminoHistoryStack::end = KaminoHistoryStack::start;
		KaminoHistoryStack::init_start = KaminoHistoryStack::start;
		KaminoHistoryStack::num++;
		return;
	}
	KaminoHistoryStack::end->next = new KaminoHistoryStack::History(url, tittle);
	KaminoHistoryStack::end = KaminoHistoryStack::end->next;
	KaminoHistoryStack::num++;
	if(KaminoHistoryStack::num > 25)
		KaminoHistoryStack::start = KaminoHistoryStack::start->next;
	if(KaminoHistoryStack::num > 40)
		content::BrowserThread::PostTask(content::BrowserThread::DB, FROM_HERE, base::Bind(CleanUpStack));
}

void KaminoHistoryModule::getTittle(std::vector<std::wstring> *vec){
	for(KaminoHistoryStack::History *it = KaminoHistoryStack::start; it != NULL; it = it->next)
		vec->push_back(it->tittle);
}

void KaminoHistoryModule::getURLs(std::vector<std::string> *vec){
	for(KaminoHistoryStack::History *it = KaminoHistoryStack::start; it != NULL; it = it->next)
		vec->push_back(it->url);
}
