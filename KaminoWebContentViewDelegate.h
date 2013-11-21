// Copyright (c) 2013 The Voyjor.inc Authors. All rights reserved.
// Use of this source code is governed by a GNU-public-style license that can be
// found in the LICENSE file.

#ifndef KAMINO_WEB_CONTENT_VIEW_DELEGATE_H
#define KAMINO_WEB_CONTENT_VIEW_DELEGATE_H
#include "content\public\browser\web_contents.h"
#include "content\public\browser\web_contents_view_delegate.h"
#include "content\public\common\context_menu_params.h"
#include "content\public\browser\web_drag_dest_delegate.h"

#include <WinUser.h>

namespace content{
class KaminoWebContentViewDelegate :public WebContentsViewDelegate{
public:
	enum {
		KaminoContextMenuItemCutId = 10001,
		KaminoContextMenuItemCopyId,
		KaminoContextMenuItemSearchId,
		KaminoContextMenuItemImageSearchId,
		KaminoContextMenuItemYtSearchId,
		KaminoContextMenuItemPasteId,
		KaminoContextMenuItemDeleteId,
		KaminoContextMenuItemOpenLinkId,
		KaminoContextMenuItemSaveImageAsId,
		KaminoContextMenuItemSaveLinkAsId,
		KaminoContextMenuItemCopyLinkId,
		KaminoContextMenuItemBackId,
		KaminoContextMenuItemForwardId,
		KaminoContextMenuItemReloadId,
		KaminoContextMenuItemRecentlyClosedId,
		KaminoContentMenuItemLikeId,
		KaminoContentMenuItemLikesId,
		KaminoContextMenuItemCopyPageURLId,
		KaminoContextMenuItemIncognitoId,
		KaminoContextMenuItemNewWindowId
	};

	KaminoWebContentViewDelegate(WebContents *content);
	~KaminoWebContentViewDelegate(void);
	WebDragDestDelegate* GetDragDestDelegate();
	void MakeContextMenuItem(HMENU menu, int menu_index, LPTSTR text, UINT id, bool enabled);
	void MenuItemSelected(int selection);
	void ShowContextMenu(const ContextMenuParams& params) OVERRIDE;
	void StoreFocus() OVERRIDE;
	void RestoreFocus() OVERRIDE;
	bool Focus() OVERRIDE;
	void TakeFocus(bool reverse) OVERRIDE;
	void SizeChanged(const gfx::Size& size) OVERRIDE;
	WebContents *web_content;
	ContextMenuParams params_;
	HANDLE clipboard_mem;
	DISALLOW_COPY_AND_ASSIGN(KaminoWebContentViewDelegate);
};
}
#endif;