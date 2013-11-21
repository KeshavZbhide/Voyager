// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#include "ipc\ipc_message_macros.h"
#include <vector>

#define IPC_MESSAGE_START ExtensionMsgStart

IPC_MESSAGE_ROUTED1(KaminoMsg_LOG, std::string)
IPC_MESSAGE_ROUTED1(KaminoMsg_ILIKE, std::string)
IPC_MESSAGE_ROUTED0(KaminoMsg_StartIcognitoMode)
IPC_MESSAGE_ROUTED2(KaminoMsg_NewSymLink, std::string, std::string);
IPC_SYNC_MESSAGE_ROUTED0_1(KaminoMsg_GetRecentHistoryTittles, std::vector<std::wstring>);
IPC_SYNC_MESSAGE_ROUTED0_1(KaminoMsg_GetRecentHistoryURLs, std::vector<std::string>);
IPC_SYNC_MESSAGE_ROUTED1_1(KaminoMsg_ShouldShowIcons, bool, bool)
