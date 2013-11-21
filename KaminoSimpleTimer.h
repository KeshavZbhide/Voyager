// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_SIMPLE_TIMER_H
#define KAMINO_SIMPLE_TIMER_H
#include "Windows.h"
class KaminoTimer
{
public:
	LARGE_INTEGER before;
	LARGE_INTEGER after;
	BOOL is_running;
	double freq;
	KaminoTimer(void);
	~KaminoTimer(void);
	void Start();
	double getTimeElapsed();
};
#endif
