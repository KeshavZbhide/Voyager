// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_HISTORY_MODULE_H
#define KAMINO_HISTORY_MODULE_H

#include <window.h>
#include "base\files\file_path.h"
#include "UIMain.h"
#include <vector>
#include <map>

class KaminoHistoryModule{

public:
	//Opens the file Handle, only, to avoid slow boot time.
	static void PreInitialize();

	//Starts Loading History form file at start.
	static void BuildStack();

	//Flush history to File. Called at shutdown.
	static void Flush();

#ifdef  UI_DEBUG_LOG
	//Test if stack is built properly
	static void CatStack();
#endif

	//Push a url on the history stack;
	static void push(const std::string &url, const std::wstring &tittle);

	//Returns the last num of urls visited, max num is 13; string format is url url url url ..... or in JSON format
	static void getTittle(std::vector<std::wstring> *vec);
	static void getURLs(std::vector<std::string> *vec);
};



#endif