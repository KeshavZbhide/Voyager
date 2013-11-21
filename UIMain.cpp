// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "UIMain.h"
#include <stdio.h>
#include <stdarg.h>

void ui_log(int funcEntryOrExit, char *format, ...){
	static int tab_pos = 0;
	if(strlen(format) > 1)
		printf("\n");
	for(int i = 0; i <= tab_pos; i++)
		printf("  ");		
	va_list argptr;
    va_start(argptr, format);
	vfprintf(stdout, format, argptr);
	va_end(argptr);
	if(funcEntryOrExit == 1){
		printf("{");
		tab_pos++;
	}
	if(funcEntryOrExit == -1){
		tab_pos--;
		printf("\n");
		for(int i = 0; i <= tab_pos; i++)
			printf("  ");
		printf("}");
	}
}