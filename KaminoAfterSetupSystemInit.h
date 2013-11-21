// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_AFTER_SETUP_SYSTEM_INIT_H
#define KAMINO_AFTER_SETUP_SYSTEM_INIT_H

#include "KaminoBrowserMainParts.h"

class content::KaminoBrowserMainParts;
class KaminoAfterSetupSystemInit{
public:
	KaminoAfterSetupSystemInit(content::KaminoBrowserMainParts *main);
	~KaminoAfterSetupSystemInit();
	void Start();
	void CallBackLooperTillEnd();
	content::KaminoBrowserMainParts *main_owner;
};



#endif