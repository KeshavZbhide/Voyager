// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include "content\public\app\startup_helper_win.h"
#include "content/public/app/content_main.h"
#include "sandbox\win\src\sandbox_types.h"
#include "content\public\app\content_main_runner.h"
#include "KaminoMainDelegate.h"
#include "KaminoGlobal.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR cmd, int nCmdShow){
	if(KaminoGlobal::Initialize(cmd)){
		sandbox::SandboxInterfaceInfo sandbox = {0};
		content::InitializeSandboxInfo(&sandbox);
		content::ContentMainRunner *main_runner = content::ContentMainRunner::Create();
		content::KaminoMainDelegate main_delegate;
		return content::ContentMain(hInstance, &sandbox, &main_delegate);
	}
	return 0;
}
