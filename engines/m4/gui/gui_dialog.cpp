/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/gui/gui_dialog.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/gui/gui.h"
#include "m4/core/imath.h"
#include "m4/platform/keys.h"
#include "m4/graphics/gr_pal.h"
#include "m4/graphics/graphics.h"
#include "m4/graphics/krn_pal.h"
#include "m4/mem/mem.h"
#include "m4/mem/memman.h"
#include "m4/vars.h"

namespace M4 {

#define _GD(X) _G(dialog).X
#define STR_DIALOG "dialog box"
#define STR_TEXTSCRN "text screen"

static void DialogShow(void *s, void *r, void *b, int32 destX, int32 destY);
static bool Dialog_EventHandler(void *myDialog, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen);
static bool TextScrn_EventHandler(void *theTextScrn, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen);

void Dialog::destroy() {
	DialogDestroy(this);
}

void Dialog::refresh() {
	Dialog_Refresh(this);
}

void Dialog::resize(int32 newW, int32 newH) {
	Dialog_Resize(this, newW, newH);
}

void Dialog::configure(int32 defaultTag, int32 returnTag, int32 cancelTag) {
	Dialog_Configure(this, defaultTag, returnTag, cancelTag);
}

void Dialog::setDefault(int32 tag) {
	Dialog_SetDefault(this, tag);
}

bool Dialog::setPressed(int32 tag) {
	return Dialog_SetPressed(this, tag);
}

void Dialog::show() {
	vmng_screen_show(this);
}

bool Dialog::addMessage(int32 x, int32 y, const char *prompt, int32 tag) {
	return Dialog_Add_Message(this, x, y, prompt, tag);
}

bool Dialog::addPicture(int32 x, int32 y, Buffer *myBuff, int32 tag) {
	return Dialog_Add_Picture(this, x, y, myBuff, tag);
}

bool Dialog::addButton(int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag) {
	return Dialog_Add_Button(this, x, y, prompt, cb, tag);
}

bool Dialog::addRepeatButton(int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag) {
	return Dialog_Add_RepeatButton(this, x, y, prompt, cb, tag);
}

bool Dialog::addList(int32 x1, int32 y1, int32 x2, int32 y2, M4CALLBACK cb, int32 tag) {
	return Dialog_Add_List(this, x1, y1, x2, y2, cb, tag);
}

bool Dialog::addTextField(int32 x1, int32 y1, int32 x2, const char *defaultPrompt, M4CALLBACK cb, int32 tag, int32 fieldLength) {
	return Dialog_Add_TextField(this, x1, y1, x2, defaultPrompt, cb, tag, fieldLength);
}

void Dialog::registerTextField() {
	Dialog_RegisterTextField(this);
}

Item *Dialog::getItem(int32 tag) {
	return Dialog_Get_Item(this, tag);
}

void Dialog::changeItemPrompt(const char *newPrompt, Item *myItem, int32 tag) {
	Dialog_Change_Item_Prompt(this, newPrompt, myItem, tag);
}

bool Dialog::removeItem(Item *myItem, int32 tag) {
	return Dialog_Remove_Item(this, myItem, tag);
}

void Dialog::refreshItem(Item *myItem, int32 tag) {
	Dialog_Refresh_Item(this, myItem, tag);
}


bool gui_dialog_init() {
	_GD(listboxSearchStr)[0] = '\0';
	return true;
}

void gui_dialog_shutdown() {
}

void vmng_TextScrn_Destroy(TextScrn *myTextScrn) {
	TextItem *myTextItems;
	TextItem *tempTextItem;
	tempTextItem = myTextItems = myTextScrn->myTextItems;

	while (tempTextItem) {
		myTextItems = myTextItems->next;
		mem_free(tempTextItem->prompt);
		mem_free((void *)tempTextItem);
		tempTextItem = myTextItems;
	}

	delete myTextScrn->textScrnBuffer;
	mem_free(myTextScrn);
}

Dialog *DialogCreateAbsolute(int32 x1, int32 y1, int32 x2, int32 y2, uint32 scrnFlags) {
	Buffer *tempBuffer;
	Dialog *dialog;
	ButtonDrawRec bdr;

	if ((dialog = (Dialog *)mem_alloc(sizeof(Dialog), STR_DIALOG)) == nullptr) {
		return nullptr;
	}

	dialog->w = x2 - x1 + 1;
	dialog->h = y2 - y1 + 1;

	dialog->dlgBuffer = new GrBuff(x2 - x1 + 1, y2 - y1 + 1);

	if (!dialog->dlgBuffer) {
		return nullptr;
	}

	dialog->num_items = 0;
	dialog->itemList = nullptr;

	dialog->cancel_item = dialog->return_item = dialog->default_item = nullptr;

	if (!vmng_screen_create(x1, y1, x2, y2, SCRN_DLG, scrnFlags, (void *)dialog,
		(RefreshFunc)DialogShow, (EventHandler)Dialog_EventHandler)) return nullptr;

	// Draw dialog box bounding rectangle
	tempBuffer = dialog->dlgBuffer->get_buffer();
	bdr.dialog = (void *)dialog;
	bdr.scrBuf = tempBuffer;
	bdr.fillMe = FILL_INTERIOR;
	bdr.pressed = false; bdr.el_type = DIALOGBOX;
	bdr.x1 = 0; bdr.y1 = 0; bdr.x2 = dialog->w - 1; bdr.y2 = dialog->h - 1;
	drawGUIelement(&bdr, nullptr);
	dialog->dlgBuffer->release();

	return dialog;
}

Dialog *DialogCreate(M4Rect *r, uint32 scrnFlags) {
	return DialogCreateAbsolute(r->x1, r->y1, r->x2, r->y2, scrnFlags);
}

void vmng_Dialog_Destroy(Dialog *d) {
	Item *myItems;
	Item *tempItem;
	tempItem = myItems = d->itemList;
	while (tempItem) {
		myItems = myItems->next;
		Item_destroy(tempItem);
		tempItem = myItems;
	}

	delete d->dlgBuffer;
	mem_free((void *)d);
}

void DialogDestroy(Dialog *d, M4Rect *r) {
	ScreenContext *myScreen;

	if (!d)
		return;

	if (r) {
		if ((myScreen = vmng_screen_find((void *)d, nullptr)) != nullptr) {
			r->x1 = myScreen->x1;
			r->x2 = myScreen->x2;
			r->y1 = myScreen->y1;
			r->y2 = myScreen->y2;
		}
	}

	vmng_screen_dispose((void *)d);
	vmng_Dialog_Destroy(d);
}

bool GetDialogCoords(Dialog *d, M4Rect *r) {
	ScreenContext *myScreen;
	if ((!d) || (!r))
		return false;
	if ((myScreen = vmng_screen_find((void *)d, nullptr)) == nullptr)
		return false;

	r->x1 = myScreen->x1;
	r->x2 = myScreen->x2;
	r->y1 = myScreen->y1;
	r->y2 = myScreen->y2;
	return true;
}

bool Dialog_Add_Message(Dialog *d, int32 x, int32 y, const char *prompt, int32 tag) {
	Item *myItem;
	if ((myItem = ItemAdd(d->itemList, x, y, 0, 0, prompt, tag, MESSAGE, nullptr, 0)) == nullptr) {
		return false;
	}

	if (!d->itemList)
		d->itemList = myItem;
	d->listBottom = myItem;
	Dialog_Refresh_Item(d, myItem, myItem->tag);
	return true;
}

bool Dialog_Add_Picture(Dialog *d, int32 x, int32 y, Buffer *myBuff, int32 tag) {
	Item *myItem;
	if ((myItem = ItemAdd(d->itemList, x, y,
		myBuff->w, myBuff->h, (char *)myBuff->data, tag, PICTURE, nullptr, 0)) == nullptr) {
		return false;
	}

	if (!d->itemList)
		d->itemList = myItem;

	d->listBottom = myItem;
	Dialog_Refresh_Item(d, myItem, myItem->tag);
	return true;
}

bool Dialog_Add_Button(Dialog *d, int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag) {
	Item *myItem;
	if ((myItem = ItemAdd(d->itemList, x, y, 0, 0, prompt, tag, BUTTON, cb, 0)) == nullptr) {
		return false;
	}
	if (!d->itemList)
		d->itemList = myItem;

	d->listBottom = myItem;
	Dialog_Refresh_Item(d, myItem, myItem->tag);
	return true;
}

bool Dialog_Add_RepeatButton(Dialog *d, int32 x, int32 y, const char *prompt, M4CALLBACK cb, int32 tag) {
	Item *myItem;
	if ((myItem = ItemAdd(d->itemList, x, y, 0, 0, prompt, tag, REPEAT_BUTTON, cb, 0)) == nullptr) {
		return false;
	}

	if (!d->itemList)
		d->itemList = myItem;

	d->listBottom = myItem;
	Dialog_Refresh_Item(d, myItem, myItem->tag);
	return true;
}

bool Dialog_Add_List(Dialog *d, int32 x1, int32 y1, int32 x2, int32 y2,
	M4CALLBACK cb, int32 tag) {
	Item *myItem;
	if ((myItem = ItemAdd(d->itemList, x1, y1, x2 - x1 + 1, y2 - y1 + 1, nullptr, tag, LISTBOX, cb, 0)) == nullptr) {
		return false;
	}
	if (!d->itemList)
		d->itemList = myItem;

	d->listBottom = myItem;
	Dialog_Refresh_Item(d, myItem, myItem->tag);
	return true;
}

bool Dialog_Add_TextField(Dialog *d, int32 x1, int32 y1, int32 x2,
		const char *defaultPrompt, M4CALLBACK cb, int32 tag, int32 fieldLength) {
	Item *myItem;
	if ((myItem = ItemAdd(d->itemList, x1, y1, x2 - x1 + 1, 0, defaultPrompt, tag, TEXTFIELD, cb, fieldLength)) == nullptr) {
		return false;
	}
	if (!d->itemList)
		d->itemList = myItem;

	d->listBottom = myItem;
	Dialog_Refresh_Item(d, myItem, myItem->tag);
	return true;
}

bool Dialog_Add_Key(Dialog *d, long myKey, HotkeyCB cb) {
	return AddScreenHotkey((void *)d, myKey, cb);
}

bool Dialog_Remove_Key(Dialog *d, long myKey) {
	return RemoveScreenHotkey((void *)d, myKey);
}

bool Dialog_Remove_Item(Dialog *d, Item *myItem, int32 tag) {
	Buffer *tempBuffer;
	ScreenContext *myScreen;
	int32 status;

	myScreen = vmng_screen_find((void *)d, &status);
	if (!myScreen)
		return false;
	if (!myItem)
		myItem = ItemFind(d->itemList, tag);
	if (!myItem)
		return false;

	// NOTE: if the item removed is the default, cancel, or return item, problems could happen...
	if (myItem->next)
		myItem->next->prev = myItem->prev;
	else
		d->listBottom = myItem;
	if (myItem->prev)
		myItem->prev->next = myItem->next;
	else
		d->itemList = myItem->next;

	gr_color_set(__LTGRAY);
	tempBuffer = d->dlgBuffer->get_buffer();
	gr_buffer_rect_fill(tempBuffer, myItem->x, myItem->y, myItem->w, myItem->h);
	d->dlgBuffer->release();
	if (status == SCRN_ACTIVE) {
		RestoreScreens(myScreen->x1 + myItem->x, myScreen->y1 + myItem->y,
			myScreen->x1 + myItem->x + myItem->w - 1,
			myScreen->y1 + myItem->y + myItem->h - 1);
	}

	Item_destroy(myItem);
	return true;
}

bool Dialog_ListItemExists(Dialog *d, Item *myItem, int32 tag, char *prompt, int32 listTag) {
	if ((!myItem) && (!d))
		return false;
	if (!myItem)
		myItem = ItemFind(d->itemList, tag);
	if (!myItem)
		return false;

	return ListItemExists(myItem, prompt, listTag);
}

bool Dialog_Add_List_Item(Dialog *d, Item *myItem, char *prompt, int32 tag, int32 listTag, int32 addMode, bool refresh) {
	ScreenContext *myScreen;
	int32 status;
	bool retValue;
	myScreen = vmng_screen_find((void *)d, &status);

	if (!myScreen)
		return false;
	if (!myItem)
		myItem = ItemFind(d->itemList, tag);

	retValue = ListItemAdd(myItem, prompt, listTag, addMode, nullptr);

	if (retValue && refresh) {
		Dialog_Refresh_Item(d, myItem, myItem->tag);
	}

	return true;
}

bool Dialog_Delete_List_Item(Dialog *d, Item *myItem, int32 tag, ListItem *myListItem, int32 listTag) {
	ScreenContext *myScreen;
	int32 status;
	if ((myScreen = vmng_screen_find((void *)d, &status)) == nullptr)
		return false;

	if (!myItem) {
		myItem = ItemFind(d->itemList, tag);
		myListItem = nullptr;
	}
	if (ListItemDelete(myItem, myListItem, listTag)) {
		Dialog_Refresh_Item(d, myItem, myItem->tag);
	}

	return true;
}

bool Dialog_Change_List_Item(Dialog *d, Item *myItem, int32 tag, ListItem *myListItem,
	int32 listTag, char *newPrompt, int32 newListTag, int32 changeMode, bool refresh) {
	ScreenContext *myScreen;
	int32 status;
	bool retValue;
	myScreen = vmng_screen_find((void *)d, &status);

	if (!myScreen)
		return false;
	if (!myItem) {
		myItem = ItemFind(d->itemList, tag);
		myListItem = nullptr;
	}

	retValue = ListItemChange(myItem, myListItem, listTag, newPrompt, newListTag, changeMode);
	if (retValue && refresh) {
		Dialog_Refresh_Item(d, myItem, myItem->tag);
	}

	return retValue;
}

void Dialog_Change_Item_Prompt(Dialog *d, const char *newPrompt, Item *myItem, int32 tag) {
	Buffer *tempBuffer;
	ScreenContext *myScreen;
	int32 status, tempWidth, tempHeight, itemType;

	if ((myScreen = vmng_screen_find((void *)d, &status)) == nullptr) {
		return;
	}
	if (!myItem) {
		myItem = ItemFind(d->itemList, tag);
	}

	if (myItem) {
		tempWidth = myItem->w;
		tempHeight = myItem->h;

		if (Item_change_prompt(myItem, newPrompt)) {
			tempWidth = imath_max(tempWidth, myItem->w);
			tempHeight = imath_max(tempHeight, myItem->h);
			gr_color_set(__LTGRAY);
			tempBuffer = d->dlgBuffer->get_buffer();
			gr_buffer_rect_fill(tempBuffer, myItem->x, myItem->y, tempWidth, tempHeight);

			if (myItem == d->default_item) {
				itemType = ITEM_DEFAULT;
			} else if (myItem == d->return_item) {
				itemType = ITEM_RETURN;
			} else {
				itemType = ITEM_NORMAL;
			}

			if (Item_show(myItem, (void *)d, tempBuffer, itemType)) {
				if (status == SCRN_ACTIVE) {
					RestoreScreens(myScreen->x1 + myItem->x, myScreen->y1 + myItem->y,
						myScreen->x1 + myItem->x + tempWidth - 1,
						myScreen->y1 + myItem->y + tempHeight - 1);
				}
			}

			d->dlgBuffer->release();
		}
	}
}

bool Dialog_ListboxSearch(Dialog *d, Item *myItem, int32 tag, int32 searchMode, char *searchStr, int32 parm1) {
	ScreenContext *myScreen;
	int32 status;
	bool returnValue;
	myScreen = vmng_screen_find((void *)d, &status);

	if (!myScreen)
		return false;
	if (!myItem)
		myItem = ItemFind(d->itemList, tag);
	if (!myItem || (myItem->type != LISTBOX))
		return false;

	returnValue = ListItemSearch(myItem, searchMode, searchStr, parm1);
	Dialog_Refresh_Item(d, myItem, myItem->tag);
	return returnValue;
}

Item *Dialog_Get_Item(Dialog *d, int32 tag) {
	if (!d)
		return nullptr;

	return ItemFind(d->itemList, tag);
}

void Dialog_Refresh_Item(Dialog *d, Item *myItem, int32 tag) {
	Buffer *tempBuffer;
	ScreenContext *myScreen;
	int32 status, itemType;

	if (!d)
		return;
	if ((myScreen = vmng_screen_find((void *)d, &status)) == nullptr)
		return;
	if (!myItem)
		myItem = ItemFind(d->itemList, tag);
	if (!myItem)
		return;

	if (myItem == d->default_item)
		itemType = ITEM_DEFAULT;
	else if (myItem == d->return_item)
		itemType = ITEM_RETURN;
	else
		itemType = ITEM_NORMAL;

	tempBuffer = d->dlgBuffer->get_buffer();
	if (Item_show(myItem, (void *)d, tempBuffer, itemType)) {
		if (status == SCRN_ACTIVE) {
			RestoreScreens(myScreen->x1 + myItem->x, myScreen->y1 + myItem->y,
				myScreen->x1 + myItem->x + myItem->w - 1,
				myScreen->y1 + myItem->y + myItem->h - 1);
		}
	}

	d->dlgBuffer->release();
}

void Dialog_Refresh(Dialog *d) {
	Buffer *tempBuffer;
	ScreenContext *myScreen;
	Item *i;
	int32 status, itemType;
	ButtonDrawRec bdr;

	if (!d) return;

	tempBuffer = d->dlgBuffer->get_buffer();
	bdr.dialog = (void *)d;
	bdr.scrBuf = tempBuffer; bdr.fillMe = FILL_INTERIOR;
	bdr.pressed = false; bdr.el_type = DIALOGBOX;
	bdr.x1 = 0; bdr.y1 = 0; bdr.x2 = d->w - 1; bdr.y2 = d->h - 1;
	drawGUIelement(&bdr, nullptr);

	for (i = d->itemList; i != nullptr; i = i->next) {
		if (i == d->default_item) itemType = ITEM_DEFAULT;
		else if (i == d->return_item) itemType = ITEM_RETURN;
		else itemType = ITEM_NORMAL;
		Item_show(i, (void *)d, tempBuffer, itemType);
	}

	d->dlgBuffer->release();

	if ((myScreen = vmng_screen_find((void *)d, &status)) == nullptr)
		return;
	if (status == SCRN_ACTIVE) {
		RestoreScreens(myScreen->x1, myScreen->y1, myScreen->x2, myScreen->y2);
	}
}

void Dialog_Refresh_All(void) {
	ScreenContext *myScreen;
	myScreen = _G(frontScreen);

	while (myScreen) {
		if (myScreen->scrnType == SCRN_DLG) {
			Dialog_Refresh((Dialog *)myScreen->scrnContent);
		}
		myScreen = myScreen->behind;
	}
}

void Dialog_Resize(Dialog *d, int32 newW, int32 newH) {
	if (!d) {
		return;
	}
	if (newW <= 0) {
		newW = d->w;
	}
	if (newH <= 0) {
		newH = d->h;
	}

	delete d->dlgBuffer;
	d->dlgBuffer = new GrBuff(newW, newH);

	d->w = newW;
	d->h = newH;

	ResizeScreen(d, newW, newH);
	Dialog_Refresh(d);
	return;
}

static void DialogShow(void *s, void *r, void *b, int32 destX, int32 destY) {
	ScreenContext *myScreen = (ScreenContext *)s;
	RectList *myRectList = (RectList *)r;
	Buffer *destBuffer = (Buffer *)b;
	Buffer *tempBuffer;
	Dialog *d;
	RectList *myRect;

	// Parameter verification
	if (!myScreen)
		return;
	d = (Dialog *)(myScreen->scrnContent);
	if (!d)
		return;

	// If no destBuffer, then draw directly to video
	if (!destBuffer) {
		myRect = myRectList;
		while (myRect) {
			d->dlgBuffer->refresh_video(myRect->x1, myRect->y1, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				myRect->x2 - myScreen->x1, myRect->y2 - myScreen->y1);

			myRect = myRect->next;
		}
	} else {
		// Else draw to the dest buffer
		tempBuffer = d->dlgBuffer->get_buffer();
		myRect = myRectList;

		while (myRect) {
			gr_buffer_rect_copy_2(tempBuffer, destBuffer, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				destX, destY, myRect->x2 - myRect->x1 + 1, myRect->y2 - myRect->y1 + 1);
			myRect = myRect->next;
		}

		d->dlgBuffer->release();
	}
}

void Dialog_Configure(Dialog *d, int32 defaultTag, int32 returnTag, int32 cancelTag) {
	if (!d)
		return;

	d->return_item = ItemFind(d->itemList, returnTag);
	d->cancel_item = ItemFind(d->itemList, cancelTag);
	Dialog_SetDefault(d, defaultTag);

	if (d->default_item) {
		Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
	}
}

void Dialog_SetDefault(Dialog *d, int32 tag) {
	ScreenContext *myScreen;
	Item *origItem, *newDefault;
	int32 status;
	myScreen = vmng_screen_find((void *)d, &status);
	if (!myScreen)
		return;

	origItem = d->default_item;
	newDefault = Item_set_default(d->itemList, d->default_item, tag);

	if ((!newDefault) || ((newDefault->type != LISTBOX) && (newDefault->type != TEXTFIELD))) {
		d->default_item = nullptr;
	} else {
		d->default_item = newDefault;
		Item_set_pressed(nullptr, d->default_item, 0);
	}
	if (origItem && (origItem != d->default_item)) {
		Item_set_unpressed(nullptr, origItem, 0);
		Dialog_Refresh_Item(d, origItem, origItem->tag);
	}
}

bool Dialog_SetPressed(Dialog *d, int32 tag) {
	ScreenContext *myScreen;
	Item *myItem;
	int32 status;
	myScreen = vmng_screen_find((void *)d, &status);

	if (!myScreen)
		return false;

	myItem = Item_set_pressed(d->itemList, nullptr, tag);

	if (myItem) {
		Dialog_Refresh_Item(d, myItem, myItem->tag);
		return true;
	} else {
		return false;
	}
}

static bool Dialog_SetUnpressed(Dialog *d, int32 tag) {
	ScreenContext *myScreen;
	Item *myItem;
	int32 status;

	myScreen = vmng_screen_find((void *)d, &status);
	if (!myScreen)
		return false;

	myItem = Item_set_unpressed(d->itemList, nullptr, tag);
	if (myItem) {
		Dialog_Refresh_Item(d, myItem, myItem->tag);
		return true;
	} else {
		return false;
	}
}

static bool Dialog_SetNextDefault(ScreenContext *myScreen, Dialog *d) {
	int32 status;
	Item *origItem;

	if (!myScreen)
		myScreen = vmng_screen_find((void *)d, &status);
	else
		status = SCRN_ACTIVE;
	if (!myScreen)
		return false;

	origItem = d->default_item;
	d->default_item = Item_set_next_default(d->default_item, d->itemList);
	if (status == SCRN_ACTIVE) {
		if (origItem && (origItem != d->default_item)) {
			Dialog_Refresh_Item(d, origItem, origItem->tag);
		}
		if (d->default_item) {
			Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
		}
	}

	if (d->default_item)
		return true;
	else
		return false;
}

static bool Dialog_SetPrevDefault(ScreenContext *myScreen, Dialog *d) {
	int32 status;
	Item *origItem;

	if (!myScreen)
		myScreen = vmng_screen_find((void *)d, &status);
	else
		status = SCRN_ACTIVE;
	if (!myScreen)
		return false;

	origItem = d->default_item;
	d->default_item = Item_set_prev_default(d->default_item, d->listBottom);

	if (status == SCRN_ACTIVE) {
		if (origItem && (origItem != d->default_item)) {
			Dialog_Refresh_Item(d, origItem, origItem->tag);
		}
		if (d->default_item) {
			Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
		}
	}

	if (d->default_item)
		return true;
	else
		return false;
}

void Dialog_GetNextListItem(Dialog *d) {
	if ((!d->default_item) || (d->default_item->type != LISTBOX))
		return;
	if (GetNextListItem(d->default_item)) {
		Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
	}
}

static void Dialog_GetNextPageList(Dialog *d) {
	if ((!d->default_item) || (d->default_item->type != LISTBOX))
		return;
	if (GetNextPageList(d->default_item)) {
		Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
	}
}

void Dialog_GetPrevListItem(Dialog *d) {
	if ((!d->default_item) || (d->default_item->type != LISTBOX))
		return;
	if (GetPrevListItem(d->default_item)) {
		Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
	}
}

static void Dialog_GetPrevPageList(Dialog *d) {
	if ((!d->default_item) || (d->default_item->type != LISTBOX))
		return;
	if (GetPrevPageList(d->default_item)) {
		Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
	}
}

ListItem *Dialog_GetCurrListItem(Dialog *d, Item *i, int32 tag) {
	if ((!i) && (!d))
		return nullptr;
	if (!i)
		i = ItemFind(d->itemList, tag);
	if (!i)
		return nullptr;

	return i->currItem;
}

char *Dialog_GetCurrListItemPrompt(Dialog *d, Item *i, int32 tag) {
	ListItem *myListItem;
	if ((myListItem = Dialog_GetCurrListItem(d, i, tag)) == nullptr)
		return nullptr;

	return myListItem->prompt;
}

bool Dialog_GetCurrListItemTag(Dialog *d, Item *i, int32 tag, int32 *listTag) {
	ListItem *myListItem;
	if ((myListItem = Dialog_GetCurrListItem(d, i, tag)) == nullptr)
		return false;

	*listTag = myListItem->tag;
	return true;
}

char *Dialog_GetListItemPrompt(Dialog *d, Item *i, int32 tag, int32 listTag) {
	ListItem *myListItem;

	if (!i)
		i = ItemFind(d->itemList, tag);
	if (!i)
		return nullptr;
	if ((myListItem = ListItemFind(i, LIST_BY_TAG, nullptr, listTag)) == nullptr)
		return nullptr;

	return myListItem->prompt;
}

void Dialog_EmptyListBox(Dialog *d, Item *i, int32 tag) {
	int32 status;

	if ((!i) && (!d))
		return;
	if (!i)
		i = ItemFind(d->itemList, tag);
	if (!i)
		return;

	Item_empty_list(i);
	(void)vmng_screen_find((void *)d, &status);
	Dialog_Refresh_Item(d, i, i->tag);
}

void Dialog_RegisterTextField(Dialog *d) {
	Item *myItem = nullptr;
	ScreenContext *myScreen;
	int32 status;

	myScreen = vmng_screen_find((void *)d, &status);
	if ((!myScreen) || (status != SCRN_ACTIVE))
		return;
	if ((myItem = Item_CheckTextField()) == nullptr)
		return;
	if (myItem->callback) {
		(myItem->callback)((void *)myItem, (void *)d);
		myScreen = vmng_screen_find((void *)d, &status);
		Dialog_Refresh_Item(d, myItem, myItem->tag);
	}
}

static void SystemErrCallback(void *, void *) {
	_GD(okButton) = true;
}

void Dialog_SystemError(char *s) {
	Dialog *aDlog = DialogCreateAbsolute(0, 0, MAX_VIDEO_X, MAX_VIDEO_Y, SF_ALERT);
	Dialog_Add_Message(aDlog, 20, 60, s, 2);
	Dialog_Add_Button(aDlog, 50, 80, " OK ", SystemErrCallback, 100);

	Dialog_Configure(aDlog, 0, 100, 0);
	vmng_screen_show((void *)aDlog);
	_GD(okButton) = false;

	while (!_GD(okButton)) {
		gui_system_event_handler();
	}

	DialogDestroy(aDlog, (M4Rect *)nullptr);
}

void Dialog_KeyMouseCollision(void) {
	Item *textItem;
	_GD(movingScreen) = false;
	_GD(clickItem) = nullptr;
	_GD(doubleClickItem) = nullptr;
	_GD(listboxSearchStr)[0] = '\0';

	if ((textItem = Item_CheckTextField()) != nullptr) {
		if (textItem->callback) {
			(textItem->callback)((void *)textItem, nullptr);
		}
	}
}

static bool Dialog_EventHandler(void *myDialog, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen) {
	ScreenContext *myScreen;
	Dialog *d = (Dialog *)myDialog;
	Item *myItem, *textItem;
	int32 status;
	bool handled;
	bool clearClickItem;
	static uint32 repeatTime;
	int32 scrollable = 0;
	uint32 tempTime;
	static int32 movingX;
	static int32 movingY;
	char tempStr[2];

	myScreen = vmng_screen_find(myDialog, &status);
	if ((!myScreen) || (status != SCRN_ACTIVE))
		return false;

	if (eventType == EVENT_KEY) {
		handled = false;
		if (d->default_item) {
			if (d->default_item->type == TEXTFIELD) {
				_GD(listboxSearchStr)[0] = '\0';
				if (parm1 == KEY_ESCAPE) {
					if ((textItem = Item_RestoreTextField()) != nullptr) {
						Dialog_Refresh_Item(d, textItem, textItem->tag);
					}
				} else if ((parm1 == KEY_RETURN) || (parm1 == KEY_TAB) || (parm1 == KEY_SHFTTAB)) {
					if ((textItem = Item_CheckTextField()) != nullptr) {
						if (textItem->callback) {
							(textItem->callback)((void *)textItem, (void *)d);
							myScreen = vmng_screen_find((void *)d, &status);
							if ((!myScreen) || (status != SCRN_ACTIVE)) handled = true;
						}
					}
				} else {
					// will fall out to catch the "set next/prev default"
					handled = Item_TextEdit(d->default_item, parm1);
					Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
				}
			} else if (d->default_item->type == LISTBOX) {
				handled = true;
				switch (parm1) {
				case KEY_RETURN:
					_GD(listboxSearchStr)[0] = '\0';
					if (d->default_item->callback) {
						handled = true;
						(d->default_item->callback)((void *)(d->default_item), (void *)d);
					} else handled = false;
					break;
				case KEY_PAGE_UP:
					_GD(listboxSearchStr)[0] = '\0';
					if (Item_SetViewBottom(d->default_item)) {
						Dialog_GetPrevPageList(d);
					} else ViewCurrListItem(d->default_item);
					break;
				case KEY_PAGE_DOWN:
					_GD(listboxSearchStr)[0] = '\0';
					if (Item_SetViewBottom(d->default_item)) {
						Dialog_GetNextPageList(d);
					} else ViewCurrListItem(d->default_item);
					break;
				case KEY_UP:
					_GD(listboxSearchStr)[0] = '\0';
					if (Item_SetViewBottom(d->default_item)) {
						Dialog_GetPrevListItem(d);
					} else ViewCurrListItem(d->default_item);
					break;
				case KEY_DOWN:
					_GD(listboxSearchStr)[0] = '\0';
					if (Item_SetViewBottom(d->default_item)) {
						Dialog_GetNextListItem(d);
					} else ViewCurrListItem(d->default_item);
					break;
				case KEY_LEFT:
					_GD(listboxSearchStr)[0] = '\0';
					if (Item_SetViewBottom(d->default_item)) {
						Dialog_GetPrevListItem(d);
					} else ViewCurrListItem(d->default_item);
					break;
				case KEY_RIGHT:
					_GD(listboxSearchStr)[0] = '\0';
					if (Item_SetViewBottom(d->default_item)) {
						Dialog_GetNextListItem(d);
					} else ViewCurrListItem(d->default_item);
					break;
				default:
					if ((parm1 >= 32) && (parm1 <= 127)) {
						if (strlen(_GD(listboxSearchStr)) < 79) {
							tempStr[0] = (char)parm1;
							tempStr[1] = '\0';
							Common::strcat_s(_GD(listboxSearchStr), 80, tempStr);
						}
						if (!ListItemSearch(d->default_item, LIST_ALPH, _GD(listboxSearchStr), 0)) {
							_GD(listboxSearchStr)[0] = (char)parm1;
							_GD(listboxSearchStr)[1] = '\0';
							ListItemSearch(d->default_item, LIST_ALPH, _GD(listboxSearchStr), 0);
						}
					} else handled = false;
					break;
				}
				if (handled) {
					Dialog_Refresh_Item(d, d->default_item, d->default_item->tag);
				}
			}
		}
		if (!handled) {
			_GD(listboxSearchStr)[0] = '\0';
			switch (parm1) {
			case KEY_ESCAPE:
				if ((d->cancel_item) && (d->cancel_item->callback)) {
					handled = true;
					(d->cancel_item->callback)((void *)(d->cancel_item), (void *)d);
				}
				break;
			case KEY_RETURN:
				if ((d->return_item) && (d->return_item->callback)) {
					handled = true;
					(d->return_item->callback)((void *)(d->return_item), (void *)d);
				}
				break;
			case KEY_TAB:
				handled = Dialog_SetNextDefault(myScreen, d);
				break;
			case KEY_SHFTTAB:
				handled = Dialog_SetPrevDefault(myScreen, d);
				break;
			}
		}
		return handled;
	} else if (eventType == EVENT_MOUSE) {
		if (parm1 != _ME_move) _GD(listboxSearchStr)[0] = '\0';
		myItem = d->itemList;
		while (myItem && (!((parm2 - myScreen->x1 >= myItem->x) &&
			(parm2 - myScreen->x1 <= (myItem->x + myItem->w - 1)) &&
			(parm3 - myScreen->y1 >= myItem->y) &&
			(parm3 - myScreen->y1 <= (myItem->y + myItem->h - 1))))) {
			myItem = myItem->next;
		}
		clearClickItem = false;
		switch (parm1) {
		case _ME_L_click:
			if (currScreen) *currScreen = true;
			repeatTime = timer_read_60();
			if ((textItem = Item_CheckTextField()) != nullptr) {
				if (textItem->callback) {
					(textItem->callback)((void *)textItem, (void *)d);
					myScreen = vmng_screen_find((void *)d, &status);
					if ((!myScreen) || (status != SCRN_ACTIVE)) {
						if (currScreen) *currScreen = false;
						_GD(clickItem) = nullptr;
						return true;
					}
				}
			}
			vmng_screen_show(myScreen->scrnContent);
			if (myItem && (myItem->type != MESSAGE) && (myItem->type != PICTURE)) _GD(clickItem) = myItem;
			else {
				_GD(clickItem) = nullptr;
				Dialog_SetDefault(d, 0);
			}
			if (_GD(clickItem)) {
				if (_GD(clickItem)->type == LISTBOX) {
					ClickOnListBox(_GD(clickItem), parm2 - myScreen->x1 - _GD(clickItem)->x, parm3 - myScreen->y1 - _GD(clickItem)->y, SCROLLABLE | PAGEABLE);
				} else if (_GD(clickItem)->type == TEXTFIELD) {
					SetTextBlockBegin(_GD(clickItem), parm2 - myScreen->x1 - _GD(clickItem)->x);
				}
				Dialog_SetDefault(d, _GD(clickItem)->tag);
				Dialog_SetPressed(d, _GD(clickItem)->tag);
			} else if (!(myScreen->scrnFlags & SF_IMMOVABLE)) {
				_GD(movingScreen) = true;
				movingX = parm2;
				movingY = parm3;
			}
			break;
		case _ME_L_hold:
		case _ME_doubleclick_hold:
			tempTime = timer_read_60();
			if (_GD(clickItem)) {
				if (_GD(clickItem)->type == LISTBOX) {
					if (tempTime > (repeatTime + 2)) {
						repeatTime = tempTime;
						ClickOnListBox(_GD(clickItem), parm2 - myScreen->x1 - _GD(clickItem)->x, parm3 - myScreen->y1 - _GD(clickItem)->y, SCROLLABLE);
					}
				} else if (_GD(clickItem)->type == REPEAT_BUTTON) {
					if (tempTime > (repeatTime + 2)) {
						repeatTime = tempTime;
						if (_GD(clickItem)->callback) {
							(_GD(clickItem)->callback)((void *)_GD(clickItem), (void *)d);
							myScreen = vmng_screen_find((void *)d, &status);
							if ((!myScreen) || (status != SCRN_ACTIVE)) {
								_GD(clickItem) = nullptr;
								return true;
							}
						}
					}
				}
			}
			break;
		case _ME_L_drag:
			if (_GD(movingScreen)) {
				MoveScreenDelta(myScreen, parm2 - movingX, parm3 - movingY);
				movingX = parm2;
				movingY = parm3;
			} else if (_GD(clickItem)) {
				if (_GD(clickItem)->type == LISTBOX) {
					tempTime = timer_read_60();
					if (tempTime > (repeatTime + 2)) {
						scrollable = SCROLLABLE;
						repeatTime = tempTime;
					}
					ClickOnListBox(_GD(clickItem), parm2 - myScreen->x1 - _GD(clickItem)->x,
						parm3 - myScreen->y1 - _GD(clickItem)->y, scrollable);
				} else if (_GD(clickItem)->type == TEXTFIELD) {
					SetTextBlockEnd(_GD(clickItem), parm2 - myScreen->x1 - _GD(clickItem)->x);
				} else if (_GD(clickItem) == myItem) {
					Dialog_SetPressed(d, _GD(clickItem)->tag);
				} else {
					Dialog_SetUnpressed(d, _GD(clickItem)->tag);
				}
			}
			break;
		case _ME_doubleclick:
			if (myItem == _GD(doubleClickItem)) {
				_GD(clickItem) = _GD(doubleClickItem);
				_GD(doubleClickItem) = nullptr;
			}
			if (_GD(clickItem) && (_GD(clickItem)->type == LISTBOX)) {
				if (DoubleClickOnListBox(_GD(clickItem), parm2 - myScreen->x1 - _GD(clickItem)->x,
					parm3 - myScreen->y1 - _GD(clickItem)->y)) {
					if (_GD(clickItem)->callback) {
						(_GD(clickItem)->callback)((void *)_GD(clickItem), (void *)d);
						clearClickItem = true;
						myScreen = vmng_screen_find((void *)d, &status);
						if ((!myScreen) || (status != SCRN_ACTIVE)) {
							if (currScreen) *currScreen = false;
							_GD(clickItem) = nullptr;
							return true;
						}
					}
				} else {
					ClickOnListBox(_GD(clickItem), parm2 - myScreen->x1 - _GD(clickItem)->x,
						parm3 - myScreen->y1 - _GD(clickItem)->y, SCROLLABLE | PAGEABLE);
					Dialog_SetDefault(d, _GD(clickItem)->tag);
					Dialog_SetPressed(d, _GD(clickItem)->tag);
				}
			} else if (_GD(clickItem) && (_GD(clickItem)->type == TEXTFIELD)) {
				SetTextBlockBegin(_GD(clickItem), 0);
				SetTextBlockEnd(_GD(clickItem), 32767);
			}
			break;
		case _ME_L_release:
			if (currScreen) *currScreen = false;
			if (_GD(movingScreen)) {
				_GD(movingScreen) = false;
				clearClickItem = true;
			} else if (_GD(clickItem)) {
				if (_GD(clickItem)->type == LISTBOX) {
					ResetDefaultListBox(_GD(clickItem));
					clearClickItem = true;
				} else if (_GD(clickItem)->type != TEXTFIELD) {
					Dialog_SetUnpressed(d, _GD(clickItem)->tag);
					clearClickItem = true;
					if ((myItem == _GD(clickItem)) && (myItem->callback)) {
						(myItem->callback)((void *)myItem, (void *)d);
						myScreen = vmng_screen_find((void *)d, &status);
						if ((!myScreen) || (status != SCRN_ACTIVE)) {
							_GD(clickItem) = nullptr;
							return true;
						}
					}
				} else clearClickItem = true;
			}
			break;
		case _ME_doubleclick_release:
			if (_GD(clickItem) && (_GD(clickItem)->type == LISTBOX)) {
				ResetDefaultListBox(_GD(clickItem));
				clearClickItem = true;
			}
			break;
		}
		if (_GD(clickItem)) {
			if ((_GD(clickItem) = Dialog_Get_Item(d, _GD(clickItem)->tag)) != nullptr) {
				Dialog_Refresh_Item(d, _GD(clickItem), _GD(clickItem)->tag);
			}
		}
		if (clearClickItem) {
			_GD(doubleClickItem) = _GD(clickItem);
			_GD(clickItem) = nullptr;
		}

		return true;
	}

	return false;
}

// TEXTSCRN STUFF...

static void TextScrn_Show(void *s, void *r, void *b, int32 destX, int32 destY) {
	ScreenContext *myScreen = (ScreenContext *)s;
	TextScrn *myTextScrn;
	TextItem *myItem;
	matte *myRectList = (matte *)r;
	Buffer *destBuffer = (Buffer *)b;
	Buffer *myBuff;
	GrBuff *myGrBuff;
	ScreenContext *tempScreen;
	matte *myMatte, tempMatte;
	RectList *updateList, *updateRect;
	RectList *newUpdateList;
	Font *currFont;

	// Parameter verification
	if (!myScreen)
		return;
	myTextScrn = (TextScrn *)(myScreen->scrnContent);
	if (!myTextScrn)
		return;

	myGrBuff = (GrBuff *)(myTextScrn->textScrnBuffer);

	if (!myGrBuff)
		return;

	// If no destBuffer, then draw directly to video
	if (!destBuffer) {
		tempMatte.nextMatte = nullptr;

		// Create an updateRectList to catch the black areas afterwards
		updateList = vmng_CreateNewRect(myScreen->x1, myScreen->y1, myScreen->x2, myScreen->y2);
		updateList->prev = nullptr;
		updateList->next = nullptr;

		// Now loop through all the screens behind myScreen
		tempScreen = myScreen->behind;
		while (tempScreen && updateList) {
			// Duplicate the updateList
			newUpdateList = vmng_DuplicateRectList(updateList);

			// Loop through the updateList
			updateRect = updateList;
			while (updateRect) {
				// See if it intersects
				tempMatte.x1 = imath_max(updateRect->x1, tempScreen->x1);
				tempMatte.y1 = imath_max(updateRect->y1, tempScreen->y1);
				tempMatte.x2 = imath_min(updateRect->x2, tempScreen->x2);
				tempMatte.y2 = imath_min(updateRect->y2, tempScreen->y2);

				if (tempScreen->redraw && (tempMatte.x1 <= tempMatte.x2) && (tempMatte.y1 <= tempMatte.y2)) {
					// Draw the intersected part of tempScreen onto myBuffer
					myBuff = myGrBuff->get_buffer();
					(tempScreen->redraw)(tempScreen, (void *)&tempMatte, myBuff, tempMatte.x1 - myScreen->x1, tempMatte.y1 - myScreen->y1);
					myGrBuff->release();
					// Remove that rectangle from the update list
					vmng_RemoveRectFromRectList(&newUpdateList, tempMatte.x1, tempMatte.y1, tempMatte.x2, tempMatte.y2);
				}

				// Get the next updateRect
				updateRect = updateRect->next;
			}

			// The newUpdateList now contains all the pieces not covered by tempScreen;
			// Turf the update list, and replace it with the newupdateList
			vmng_DisposeRectList(&updateList);
			updateList = newUpdateList;

			// Now get the next screen
			tempScreen = tempScreen->behind;
		}

		// Now we've gone through all the screens, whatever is left in the updateList should be filled in with black
		gr_color_set(__BLACK);
		updateRect = updateList;
		myBuff = myGrBuff->get_buffer();
		while (updateRect) {
			gr_buffer_rect_fill(myBuff, updateRect->x1 - myScreen->x1, updateRect->y1 - myScreen->y1,
				updateRect->x2 - updateRect->x1 + 1, updateRect->y2 - updateRect->y1 + 1);
			updateRect = updateRect->next;
		}
		myGrBuff->release();

		// Now dispose of the updateList
		vmng_DisposeRectList(&updateList);

		// Now we darken (or lighten) the screen by the luminance percentage	
		myBuff = myGrBuff->get_buffer();
		krn_ChangeBufferLuminance(myBuff, myTextScrn->luminance);
		myGrBuff->release();

		// Now myBuff should contain a copy of everything on the screen, except the actual contents of this transparent screen
		// Now would be the time to draw the contents
		currFont = gr_font_get();
		gr_font_set(myTextScrn->myFont);
		myItem = myTextScrn->myTextItems;
		myBuff = myGrBuff->get_buffer();

		while (myItem) {
			if (myItem == myTextScrn->hiliteItem) {
				font_set_colors(myTextScrn->hiliteColor_alt1, myTextScrn->hiliteColor_alt2, myTextScrn->hiliteColor);
			} else {
				font_set_colors(myTextScrn->textColor_alt1, myTextScrn->textColor_alt2, myTextScrn->textColor);
			}
			gr_font_write(myBuff, myItem->prompt, myItem->x, myItem->y, 0, 0); // MattP no auto spacing
			myItem = myItem->next;
		}

		myGrBuff->release();
		gr_font_set(currFont);

		// Now dump the matte list out to video
		myMatte = myRectList;
		while (myMatte) {
			myGrBuff->refresh_video(myMatte->x1, myMatte->y1, myMatte->x1 - myScreen->x1, myMatte->y1 - myScreen->y1,
				myMatte->x2 - myScreen->x1, myMatte->y2 - myScreen->y1);
			//vmng_refresh_video(myMatte->x1, myMatte->y1, myMatte->x1 - myScreen->x1, myMatte->y1 - myScreen->y1,
			//							myMatte->x2 - myScreen->x1, myMatte->y2 - myScreen->y1, myBuff);
			myMatte = myMatte->nextMatte;
		}
	} else {
		// Else draw to the dest buffer
		myMatte = myRectList;
		myBuff = myGrBuff->get_buffer();
		while (myMatte) {
			gr_buffer_rect_copy_2(myBuff, destBuffer, myMatte->x1 - myScreen->x1, myMatte->y1 - myScreen->y1,
				destX, destY, myMatte->x2 - myMatte->x1 + 1, myMatte->y2 - myMatte->y1 + 1);
			myMatte = myMatte->nextMatte;
		}

		myGrBuff->release();
	}
}



TextScrn *TextScrn_Create(int32 x1, int32 y1, int32 x2, int32 y2, int32 luminance, uint32 scrnFlags,
	int32 textColor, int32 hiliteColor,
	int32 textColorAlt1, int32 hiliteColorAlt1, int32 textColorAlt2, int32 hiliteColorAlt2) {

	TextScrn *myTextScrn;

	if ((myTextScrn = (TextScrn *)mem_alloc(sizeof(TextScrn), STR_TEXTSCRN)) == nullptr)
		return nullptr;

	myTextScrn->w = x2 - x1 + 1;
	myTextScrn->h = y2 - y1 + 1;
	myTextScrn->textColor = textColor;
	myTextScrn->textColor_alt1 = textColorAlt1;
	myTextScrn->textColor_alt2 = textColorAlt2;
	myTextScrn->hiliteColor = hiliteColor;
	myTextScrn->hiliteColor_alt1 = hiliteColorAlt1;
	myTextScrn->hiliteColor_alt2 = hiliteColorAlt2;
	myTextScrn->luminance = luminance;
	myTextScrn->myFont = gr_font_get();
	myTextScrn->myTextItems = nullptr;
	myTextScrn->hiliteItem = nullptr;

	myTextScrn->textScrnBuffer = new GrBuff(myTextScrn->w, myTextScrn->h);

	if (!myTextScrn->textScrnBuffer) {
		return nullptr;
	}

	if (!vmng_screen_create(x1, y1, x2, y2, SCRN_TEXT, scrnFlags | SF_TRANSPARENT, (void *)myTextScrn, (RefreshFunc)TextScrn_Show, TextScrn_EventHandler)) {
		return nullptr;
	}
	return myTextScrn;
}

void TextScrn_Destroy(TextScrn *myTextScrn) {
	if (!myTextScrn) return;
	vmng_screen_dispose((void *)myTextScrn);
	vmng_TextScrn_Destroy(myTextScrn);
	_G(currScreen) = false;
}

void TextScrn_Activate(TextScrn *myTextScrn) {
	vmng_screen_show((void *)myTextScrn);
}

bool TextScrn_Add_Key(TextScrn *myTextScrn, long myKey, HotkeyCB cb) {
	return AddScreenHotkey((void *)myTextScrn, myKey, cb);
}

bool TextScrn_Add_TextItem(TextScrn *myTextScrn, int32 x, int32 y, int32 tag,
	int32 justification, const char *prompt, M4CALLBACK callback) {
	TextItem *myTextItem;

	if (!myTextScrn) {
		return false;
	}
	if ((myTextItem = (TextItem *)mem_alloc(sizeof(TextItem), "text item")) == nullptr)
		return false;

	myTextItem->w = gr_font_string_width(prompt, 0); // No auto spacing
	myTextItem->h = gr_font_get_height() + 1;
	myTextItem->y = y;
	myTextItem->justification = justification;
	switch (justification) {
	case TS_JUST_LEFT:
		myTextItem->x = 0;
		break;
	case TS_JUST_RIGHT:
		myTextItem->x = myTextScrn->w - myTextItem->w - 1;
		break;
	case TS_CENTRE:
		myTextItem->x = (myTextScrn->w - myTextItem->w) >> 1;
		break;
	case TS_GIVEN:
	default:
		myTextItem->x = x;
		break;
	}
	myTextItem->tag = tag;
	myTextItem->type = BUTTON;
	myTextItem->prompt = mem_strdup(prompt);
	myTextItem->callback = callback;
	myTextItem->next = myTextScrn->myTextItems;
	myTextScrn->myTextItems = myTextItem;

	return true;
}

bool TextScrn_Add_Message(TextScrn *myTextScrn, int32 x, int32 y, int32 tag,
	int32 justification, const char *prompt) {
	TextItem *myTextItem;

	if (!myTextScrn) {
		return false;
	}
	if ((myTextItem = (TextItem *)mem_alloc(sizeof(TextItem), "textscrn msg")) == nullptr)
		return false;

	myTextItem->w = gr_font_string_width(prompt, 0); // No auto spacing
	myTextItem->h = gr_font_get_height() + 1;
	myTextItem->y = y;
	myTextItem->justification = justification;

	switch (justification) {
	case TS_JUST_LEFT:
		myTextItem->x = 0;
		break;
	case TS_JUST_RIGHT:
		myTextItem->x = myTextScrn->w - myTextItem->w - 1;
		break;
	case TS_CENTRE:
		myTextItem->x = (myTextScrn->w - myTextItem->w) >> 1;
		break;
	case TS_GIVEN:
	default:
		myTextItem->x = x;
		break;
	}
	myTextItem->tag = tag;
	myTextItem->type = MESSAGE;
	myTextItem->prompt = mem_strdup(prompt);
	myTextItem->next = myTextScrn->myTextItems;
	myTextScrn->myTextItems = myTextItem;

	return true;
}

void TextScrn_Delete_TextItem(TextScrn *myTextScrn, int32 tag) {
	ScreenContext *myScreen;
	int32 status, x, y, w, h;
	TextItem *myTextItem, *tempTextItem;

	if ((myScreen = vmng_screen_find((void *)myTextScrn, &status)) == nullptr)
		return;

	myTextItem = myTextScrn->myTextItems;

	if (myTextItem->tag == tag) {
		myTextScrn->myTextItems = myTextItem->next;
		tempTextItem = myTextItem;
	} else {
		while (myTextItem->next && (myTextItem->next->tag != tag)) myTextItem = myTextItem->next;
		if ((tempTextItem = myTextItem->next) == nullptr) return;
		myTextItem->next = tempTextItem->next;
	}

	x = tempTextItem->x;
	y = tempTextItem->y;
	w = tempTextItem->w;
	h = tempTextItem->h;
	mem_free(tempTextItem->prompt);
	mem_free((void *)tempTextItem);

	if (status == SCRN_ACTIVE) {
		RestoreScreens(myScreen->x1 + x, myScreen->y1 + y, myScreen->x1 + x + w - 1, myScreen->y1 + y + h - 1);
	}
}

static bool TextScrn_EventHandler(void *theTextScrn, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen) {
	ScreenContext *myScreen;
	TextScrn *myTextScrn = (TextScrn *)theTextScrn;
	TextItem *myTextItem, *oldHiliteItem;
	int32 status;
	static int32 movingX;
	static int32 movingY;

	myScreen = vmng_screen_find(theTextScrn, &status);

	if ((!myScreen) || (status != SCRN_ACTIVE))
		return false;
	if (!(eventType == EVENT_MOUSE))
		return false;

	myTextItem = myTextScrn->myTextItems;
	while (myTextItem && (!((parm2 >= myScreen->x1) &&
		(parm2 <= myScreen->x2) &&
		(parm3 - myScreen->y1 >= myTextItem->y) &&
		(parm3 - myScreen->y1 <= (myTextItem->y + myTextItem->h - 1))))) {
		myTextItem = myTextItem->next;
	}

	if (myTextItem && (myTextItem->type == MESSAGE)) {
		myTextItem = nullptr;
	}

	if (myTextItem != myTextScrn->hiliteItem) {
		oldHiliteItem = myTextScrn->hiliteItem;
		myTextScrn->hiliteItem = myTextItem;
		if (oldHiliteItem) {
			RestoreScreens(myScreen->x1 + oldHiliteItem->x, myScreen->y1 + oldHiliteItem->y,
				myScreen->x1 + oldHiliteItem->x + oldHiliteItem->w - 1,
				myScreen->y1 + oldHiliteItem->y + oldHiliteItem->h - 1);
		}
		if (myTextItem) {
			RestoreScreens(myScreen->x1 + myTextItem->x, myScreen->y1 + myTextItem->y,
				myScreen->x1 + myTextItem->x + myTextItem->w - 1,
				myScreen->y1 + myTextItem->y + myTextItem->h - 1);
		}
	}

	switch (parm1) {
	case _ME_L_click:
		if (currScreen) {
			*currScreen = true;
		}
		if (!(myScreen->scrnFlags & SF_IMMOVABLE)) {
			_GD(movingScreen) = true;
			movingX = parm2;
			movingY = parm3;
		}
		break;
	case _ME_L_hold:
	case _ME_doubleclick_hold:
		break;
	case _ME_L_drag:
		if (_GD(movingScreen)) {
			MoveScreenDelta(myScreen, parm2 - movingX, parm3 - movingY);
			movingX = parm2;
			movingY = parm3;
		}
		break;
	case _ME_move:
		if ((parm2 < myScreen->x1) || (parm2 > myScreen->x2) ||
			(parm3 < myScreen->y1) || (parm3 > myScreen->y2)) {
			if (currScreen) {
				*currScreen = false;
			}
			return false;
		}
		break;
	case _ME_doubleclick:
		break;
	case _ME_L_release:
		if (currScreen) {
			*currScreen = false;
		}
		_GD(movingScreen) = false;
		if (myTextItem && myTextItem->callback) {
			(myTextItem->callback)((void *)myTextItem, (void *)myTextScrn);
		}
		myScreen = vmng_screen_find(theTextScrn, &status);
		break;

	case _ME_doubleclick_release:
		break;
	}

	if (myScreen && myTextScrn->hiliteItem && currScreen) {
		*currScreen = true;
	}

	return true;
}

} // namespace M4
