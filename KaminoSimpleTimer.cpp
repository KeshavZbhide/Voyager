// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "KaminoSimpleTimer.h"

KaminoTimer::KaminoTimer(){
	LARGE_INTEGER tickFrequncy;
	QueryPerformanceFrequency(&tickFrequncy);
	freq = static_cast<double>(tickFrequncy.QuadPart);
}

KaminoTimer::~KaminoTimer(){}

void KaminoTimer::Start(){
	QueryPerformanceCounter(&before);
	is_running = TRUE;
}

double KaminoTimer::getTimeElapsed(){
	QueryPerformanceCounter(&after);
	return static_cast<double>(after.QuadPart - before.QuadPart)/freq;
}





