// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_UPDATE_PROCESS_H
#define KAMINO_UPDATE_PROCESS_H

#include "KaminoBrowserContext.h"
#include <windows.h>
#include "base\files\file_path.h"
#include "base\message_loop\message_loop.h"
#include "third_party\libxml\chromium\libxml_utils.h"
#include "KaminoBrowserMainParts.h"


namespace KaminoUpdate{

class content::KaminoBrowserMainParts;
class KaminoUpdateProcess{
public:
	KaminoUpdateProcess(content::KaminoBrowserMainParts *main_parts);
	~KaminoUpdateProcess();
	void Start();
	void InitialExecution(std::string version);
	void InitialExecution2(std::string stateXml);
	//Should Bring File From Server... Yeah;
	void BuildCurrentState();

	//Check to see if we need to Build A new App Directory for the main Executable.
	bool KaminoUpdateProcess::ShouldUnderGoAppUpdateProcedure(std::string&);
	//Should Bring the Patch form s3 and apply it. 
	bool ShouldPatchExecutable(std::string &name, std::string &version);
	//Should Bring the raw executable file form S3 , Else Copy it from the prev version.
	bool ShouldBringRawExecutable(std::string &name, std::string &version);

	bool PatchExecutable(std::string &in_dir, std::string &patch_src);
	//
	void StartPatchingAndUpdatingBinaryExecutables(std::string stateXml);

	static void BuildAllShortCuts(std::string &target);
	
	bool ShouldBringFile(std::string&, std::string&);
	void BringFile(std::string &file, std::string &url, base::Callback<void(base::FilePath)> callback);
	WNDCLASSEX wnd_cls;
	HWND hWnd;
	scoped_refptr<net::URLRequestContextGetter> url_request_context_getter;
	XmlReader reader_current_state;
	std::string current_state_xml;
	content::KaminoBrowserMainParts *main_owner;
};

}
#endif

