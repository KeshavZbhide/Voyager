// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#define IPC_MESSAGE_IMPL
#include "KaminoIPCMessages.h"
//     // Generate constructors.
#include "ipc/struct_constructor_macros.h"
#include "KaminoIPCMessages.h"
//     // Generate destructors.
#include "ipc/struct_destructor_macros.h"
#include "KaminoIPCMessages.h"
//     // Generate param traits write methods.
#include "ipc/param_traits_write_macros.h"
namespace IPC {
	#include "KaminoIPCMessages.h"
}  // namespace IPC
//     // Generate param traits read methods.
#include "ipc/param_traits_read_macros.h"
namespace IPC {
#include "KaminoIPCMessages.h"
}  // namespace IPC
//     // Generate param traits log methods.
#include "ipc/param_traits_log_macros.h"
namespace IPC {
	#include "KaminoIPCMessages.h"
}  // namespace IPC
//