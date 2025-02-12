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

#include "graphics/thumbnail.h"
#include "m4/burger/gui/game_menu.h"
#include "m4/burger/gui/interface.h"
#include "m4/adv_r/other.h"
#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_player.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/hotkeys.h"
#include "m4/graphics/gr_line.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/graphics/krn_pal.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/platform/keys.h"
#include "m4/burger/hotkeys.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace GUI {

static bool buttonClosesDialog;

void UpdateThumbNails(int32 firstSlot, guiMenu *myMenu);

Sprite *menu_CreateThumbnail(int32 *spriteSize) {
	Sprite *thumbNailSprite;
	GrBuff *thumbNail;
	Buffer *scrnBuff, *intrBuff, *destBuff, RLE8Buff;
	uint8 *srcPtr, *srcPtr2, *srcPtr3, *srcRowPtr, *destPtr;
	ScreenContext *gameScreen;
	int32 i, status;
	int32 currRow, beginRow, endRow;

	// Create a Sprite for the thumbNail
	if ((thumbNailSprite = (Sprite *)mem_alloc(sizeof(Sprite), "sprite")) == nullptr) {
		return nullptr;
	}

	thumbNail = new GrBuff((MAX_VIDEO_X + 1) / 3, (MAX_VIDEO_Y + 1) / 3);
	if (!thumbNail) {
		return nullptr;
	}

	destBuff = thumbNail->get_buffer();
	if (!destBuff) {
		return nullptr;
	}

	gameScreen = vmng_screen_find(_G(gameDrawBuff), &status);
	if ((!gameScreen) || (status != SCRN_ACTIVE)) {
		return nullptr;
	}

	scrnBuff = _G(gameDrawBuff)->get_buffer();
	if (!scrnBuff) {
		return nullptr;
	}

	// Grab the interface buffer
	intrBuff = _G(gameInterfaceBuff)->get_buffer();

	if (gameScreen->y1 > 0) {
		// Paint the top of the thumbnail black
		beginRow = gameScreen->y1;
		memset(destBuff->data, 21, (beginRow / 3) * destBuff->stride);
		srcRowPtr = (uint8 *)(scrnBuff->data + (-gameScreen->x1));
		destPtr = (uint8 *)(destBuff->data + ((beginRow / 3) * destBuff->stride));
	} else {
		srcRowPtr = (uint8 *)(scrnBuff->data + ((-gameScreen->y1) * scrnBuff->stride) + (-gameScreen->x1));
		beginRow = 0;
		destPtr = destBuff->data;
	}
	endRow = imath_min(MAX_VIDEO_Y, gameScreen->y2);

	for (currRow = beginRow; currRow <= endRow; currRow += 3) {

		// Set the src pointers
		srcPtr = srcRowPtr;
		srcPtr2 = srcRowPtr + scrnBuff->stride;
		srcPtr3 = srcRowPtr + (scrnBuff->stride << 1);

		for (i = 0; i < (MAX_VIDEO_X + 1) / 3; i++) {

			// Calculate the average - make sure not to extend past the end of the buffer
			if (endRow - currRow < 1) {
				*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2)) / 3));
			} else if (endRow - currRow < 2) {
				*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
					*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2)) / 6));
			} else {
				*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
					*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2) +
					*srcPtr3 + *(srcPtr3 + 1) + *(srcPtr3 + 2)) / 9));
			}

			if (*destPtr == 0) {
				*destPtr = 21;
			}

			// Increment the pointers
			srcPtr += 3;
			srcPtr2 += 3;
			srcPtr3 += 3;
			destPtr++;
		}

		// Update the row pointer
		srcRowPtr += scrnBuff->stride * 3;
	}

	// Reset the currRow
	beginRow = currRow;

	// Paint the interface section of the thumbnail
	if (currRow < MAX_VIDEO_Y) {
		// If the interface is visible, grab it
		if (intrBuff) {
			srcRowPtr = intrBuff->data;
			endRow = imath_min(MAX_VIDEO_Y, beginRow + intrBuff->h - 1);
			for (currRow = beginRow; currRow <= endRow; currRow += 3) {
				// Set the src pointers
				srcPtr = srcRowPtr;
				srcPtr2 = srcRowPtr + intrBuff->stride;
				srcPtr3 = srcRowPtr + (intrBuff->stride << 1);

				for (i = 0; i < (MAX_VIDEO_X + 1) / 3; i++) {
					// If the pix is outside of the inventory objects in the interface, set to black
					// If ((srcPtr - srcRowPtr < 180) || (srcPtr - srcRowPtr > 575)) {
					if (true) {			// for now make everything in the interface black
						*destPtr = 21;
					}

					// Else calculate the average - make sure not to extend past the end of the buffer
					else {
						if (endRow - currRow < 1) {
							*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2)) / 3));
						} else if (endRow - currRow < 2) {
							*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
								*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2)) / 6));
						} else {
							*destPtr = (uint8)((uint32)((*srcPtr + *(srcPtr + 1) + *(srcPtr + 2) +
								*srcPtr2 + *(srcPtr2 + 1) + *(srcPtr2 + 2) +
								*srcPtr3 + *(srcPtr3 + 1) + *(srcPtr3 + 2)) / 9));
						}
						if (*destPtr == 0) {
							*destPtr = 21;
						}
					}

					// Increment the pointers
					srcPtr += 3;
					srcPtr2 += 3;
					srcPtr3 += 3;
					destPtr++;
				}

				// Update the row pointer
				srcRowPtr += intrBuff->stride * 3;
			}
		} else {
			// Else paint the bottom of the thumbnail black
			destPtr = (uint8 *)(destBuff->data + ((currRow / 3) * destBuff->stride));
			memset(destPtr, 21, (destBuff->h - (currRow / 3)) * destBuff->stride);
		}
	}

	// Reset the currRow
	beginRow = currRow;

	if (currRow < MAX_VIDEO_Y) {
		// Paint the bottom of the thumbnail black
		destPtr = (uint8 *)(destBuff->data + ((currRow / 3) * destBuff->stride));
		memset(destPtr, 21, (destBuff->h - (currRow / 3)) * destBuff->stride);
	}

	// Compress the thumbNail data into the RLE8Buff
	if ((*spriteSize = (int32)gr_sprite_RLE8_encode(destBuff, &RLE8Buff)) <= 0) {
		return nullptr;
	}

	// Fill in the Sprite structure
	thumbNailSprite->w = destBuff->w;
	thumbNailSprite->h = destBuff->h;
	thumbNailSprite->encoding = RLE8;
	thumbNailSprite->data = nullptr;
	if ((thumbNailSprite->sourceHandle = NewHandle(*spriteSize, "thumbNail source")) == nullptr) {
		return nullptr;
	}
	thumbNailSprite->sourceOffset = 0;

	// Now copy the RLE8Buff into the thumbNail source handle
	HLock(thumbNailSprite->sourceHandle);
	thumbNailSprite->data = (uint8 *)(*(thumbNailSprite->sourceHandle));
	memcpy(thumbNailSprite->data, RLE8Buff.data, *spriteSize);
	HUnLock(thumbNailSprite->sourceHandle);

	// Release all buffers
	_G(gameDrawBuff)->release();
	if (intrBuff) {
		_G(gameInterfaceBuff)->release();
	}
	thumbNail->release();

	// Free up both the thumbNail and the RLE8Buff
	delete thumbNail;
	mem_free((void *)RLE8Buff.data);

	return thumbNailSprite;
}


bool menu_CursorInsideItem(menuItem *myItem, int32 cursorX, int32 cursorY) {
	if ((cursorX >= myItem->x1) && (cursorX <= myItem->x2) && (cursorY >= myItem->y1) && (cursorY <= myItem->y2)) {
		return true;
	} else {
		return false;
	}
}


void gui_DrawSprite(Sprite *mySprite, Buffer *myBuff, int32 x, int32 y) {
	DrawRequest spriteDrawReq;
	Buffer drawSpriteBuff;

	if ((!mySprite) || (!myBuff)) {
		return;
	}

	if (mySprite->sourceHandle) {
		HLock(mySprite->sourceHandle);
		mySprite->data = (uint8 *)((intptr)*(mySprite->sourceHandle) + mySprite->sourceOffset);

		drawSpriteBuff.w = mySprite->w;
		drawSpriteBuff.stride = mySprite->w;
		drawSpriteBuff.h = mySprite->h;
		drawSpriteBuff.encoding = (mySprite->encoding) & (uint8)0x7f;
		drawSpriteBuff.data = mySprite->data;

		spriteDrawReq.Src = &drawSpriteBuff;
		spriteDrawReq.Dest = myBuff;
		spriteDrawReq.x = x;
		spriteDrawReq.y = y;
		spriteDrawReq.scaleX = 100;
		spriteDrawReq.scaleY = 100;
		spriteDrawReq.srcDepth = 0;
		spriteDrawReq.depthCode = nullptr;
		spriteDrawReq.Pal = nullptr;
		spriteDrawReq.ICT = nullptr;

		gr_sprite_draw(&spriteDrawReq);

		// Unlock the handle
		HUnLock(mySprite->sourceHandle);
	}
}


void item_Destroy(void *theItem) {
	menuItem *myItem = (menuItem *)theItem;

	// Verify params
	if (!myItem) {
		return;
	}
	if (myItem->background) {
		delete myItem->background;
	}

	if (myItem->itemInfo) {
		mem_free((void *)myItem->itemInfo);
	}
	mem_free((void *)myItem);
}


//-------------------------------    MESSAGE MENU ITEM    ---------------------------------//


void menu_DrawMsg(void *theItem, void *theMenu, int32 x, int32 y, int32, int32) {
	menuItem *myItem = (menuItem *)theItem;
	guiMenu *myMenu = (guiMenu *)theMenu;
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo) || (!myMenu)) {
		return;
	}

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	// Get the button info and select the sprite
	//myMsg = (menuItemMsg *)myItem->itemInfo;
	switch (myItem->tag) {
	case SL_TAG_SAVE_LABEL:
		mySprite = _GM(menuSprites)[SL_SAVE_LABEL];
		break;
	case SL_TAG_LOAD_LABEL:
		mySprite = _GM(menuSprites)[SL_LOAD_LABEL];
		break;
	case SL_TAG_THUMBNAIL:
		mySprite = _GM(saveLoadThumbNail);
		break;
	}

	// Get the menu buffer and draw the sprite to it
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	} else if (myItem->tag == SL_TAG_THUMBNAIL && mySprite->w == 160) {
		// Hack for handling smaller ScummVM thumbnails
		for (int yp = y; yp < (y + SL_THUMBNAIL_H); ++yp) {
			byte *line = myBuff->data + myBuff->stride * yp + x;
			Common::fill(line, line + SL_THUMBNAIL_W, 0);
		}

		x += 25;
		y += 25;
	}

	// Draw the sprite in
	gui_DrawSprite(mySprite, myBuff, x, y);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}


menuItem *menu_MsgAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, bool transparent) {
	menuItem *newItem;
	menuItemMsg *msgInfo;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	if ((newItem = (menuItem *)mem_alloc(sizeof(menuItem), "gui menu item")) == nullptr) {
		return nullptr;
	}

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = (void *)myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = nullptr;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = menu_CopyBackground(myMenu, x, y, w, h);
	}

	if ((msgInfo = (menuItemMsg *)mem_alloc(sizeof(menuItemMsg), "menu item message")) == nullptr) {
		return nullptr;
	}
	newItem->itemInfo = (void *)msgInfo;

	newItem->redraw = menu_DrawMsg;
	newItem->destroy = item_Destroy;
	newItem->itemEventHandler = nullptr;

	// Draw the message in now
	(newItem->redraw)(newItem, (void *)myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find((void *)myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}



//-------------------------------    BUTTON MENU ITEM    ----------------------------------//

enum button_states {
	BTN_STATE_NORM = 0,
	BTN_STATE_OVER = 1,
	BTN_STATE_PRESS = 2,
	BTN_STATE_GREY = 3
};

enum button_types {
	BTN_TYPE_GM_GENERIC,
	BTN_TYPE_SL_SAVE,
	BTN_TYPE_SL_LOAD,
	BTN_TYPE_SL_CANCEL,
	BTN_TYPE_SL_TEXT,
	BTN_TYPE_OM_DONE,
	BTN_TYPE_OM_CANCEL,

	BTN_TYPE_TOTAL_NUMBER
};


void menu_DrawButton(void *theItem, void *theMenu, int32 x, int32 y, int32, int32) {
	menuItem *myItem = (menuItem *)theItem;
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItemButton *myButton = nullptr;
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;
	char tempStr[32];

	// Verify params
	if ((!myItem) || (!myItem->itemInfo) || (!myMenu)) {
		return;
	}

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	// Get the button info and select the sprite
	myButton = (menuItemButton *)myItem->itemInfo;

	switch (myButton->buttonType) {
	case BTN_TYPE_GM_GENERIC:
		switch (myButton->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[GM_BUTTON_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[GM_BUTTON_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[GM_BUTTON_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[GM_BUTTON_GREY];
			break;
		}
		break;

	case BTN_TYPE_SL_SAVE:
		switch (myButton->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[SL_SAVE_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[SL_SAVE_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[SL_SAVE_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[SL_SAVE_BTN_GREY];
			break;
		}
		break;

	case BTN_TYPE_SL_LOAD:
		switch (myButton->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[SL_LOAD_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[SL_LOAD_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[SL_LOAD_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[SL_LOAD_BTN_GREY];
			break;
		}
		break;

	case BTN_TYPE_SL_TEXT:
		switch (myButton->itemFlags) {
		case BTN_STATE_OVER:
			font_set_colors(TEXT_COLOR_OVER_SHADOW, TEXT_COLOR_OVER_FOREGROUND, TEXT_COLOR_OVER_HILITE);
			// Gr_font_set_color(TEXT_COLOR_OVER);
			mySprite = _GM(menuSprites)[SL_LINE_OVER];
			break;
		case BTN_STATE_PRESS:
			font_set_colors(TEXT_COLOR_PRESS_SHADOW, TEXT_COLOR_PRESS_FOREGROUND, TEXT_COLOR_PRESS_HILITE);
			// Gr_font_set_color(TEXT_COLOR_PRESS);
			mySprite = _GM(menuSprites)[SL_LINE_PRESS];
			break;
		case BTN_STATE_GREY:
			font_set_colors(TEXT_COLOR_GREY_SHADOW, TEXT_COLOR_GREY_FOREGROUND, TEXT_COLOR_GREY_HILITE);
			// Gr_font_set_color(TEXT_COLOR_GREY);
			mySprite = _GM(menuSprites)[SL_LINE_NORM];
			break;
		default:
		case BTN_STATE_NORM:
			font_set_colors(TEXT_COLOR_NORM_SHADOW, TEXT_COLOR_NORM_FOREGROUND, TEXT_COLOR_NORM_HILITE);
			// Gr_font_set_color(TEXT_COLOR_NORM);
			mySprite = _GM(menuSprites)[SL_LINE_NORM];
			break;
		}
		break;

	case BTN_TYPE_SL_CANCEL:
		switch (myButton->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[SL_CANCEL_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[SL_CANCEL_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[SL_CANCEL_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[SL_CANCEL_BTN_NORM];
			break;
		}
		break;

	case BTN_TYPE_OM_DONE:
		switch (myButton->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[OM_DONE_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[OM_DONE_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[OM_DONE_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[OM_DONE_BTN_GREY];
			break;
		}
		break;

	case BTN_TYPE_OM_CANCEL:
		switch (myButton->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[OM_CANCEL_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[OM_CANCEL_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[OM_CANCEL_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[OM_CANCEL_BTN_NORM];
			break;
		}
		break;
	}

	// Get the menu buffer
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Draw the button sprite in
	gui_DrawSprite(mySprite, myBuff, x, y);

	// If the button is a textbutton, write in the text
	if ((myButton->buttonType == BTN_TYPE_SL_TEXT) && (myButton->prompt)) {
		// Write in the special tag
		Common::sprintf_s(tempStr, 32, "%02d", myItem->tag - 1000 + _GM(firstSlotIndex));

		gr_font_set(_GM(menuFont));
		gr_font_write(myBuff, tempStr, x + 4, y + 1, 0, -1);
		gr_font_write(myBuff, myButton->prompt, x + 26, y + 1, 0, -1);
	}

	// Release the menu buffer
	myMenu->menuBuffer->release();
}


bool button_Handler(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	menuItem *myItem = (menuItem *)theItem;
	menuItemButton *myButton;
	bool redrawItem, execCallback, handled;
	ScreenContext *myScreen;
	int32 status;
	int32 currTag;
	guiMenu *currMenu;
	menuItem *tempItem;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo)) {
		return false;
	}
	myButton = (menuItemButton *)myItem->itemInfo;

	if (!(eventType == EVENT_MOUSE)) {
		return false;
	}

	if (myButton->itemFlags == BTN_STATE_GREY) {
		return false;
	}

	redrawItem = false;
	execCallback = false;
	handled = true;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menu_CursorInsideItem(myItem, x, y)) {
			myButton->itemFlags = BTN_STATE_PRESS;
			*currItem = theItem;
			redrawItem = true;
		} else {
			*currItem = nullptr;
			if (myButton->itemFlags != BTN_STATE_NORM) {
				myButton->itemFlags = BTN_STATE_NORM;
				redrawItem = true;
			}
		}
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (!*currItem) {
			return true;
		}
		if (menu_CursorInsideItem(myItem, x, y)) {
			if (myButton->itemFlags != BTN_STATE_PRESS) {
				myButton->itemFlags = BTN_STATE_PRESS;
				redrawItem = true;
			}
		} else {
			if (myButton->itemFlags != BTN_STATE_OVER) {
				myButton->itemFlags = BTN_STATE_OVER;
				redrawItem = true;
			}
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		if (menu_CursorInsideItem(myItem, x, y)) {
			if (*currItem) {
				execCallback = true;
			} else {
				*currItem = theItem;
			}
			myButton->itemFlags = BTN_STATE_OVER;
			redrawItem = true;
		} else {
			*currItem = nullptr;
			myButton->itemFlags = BTN_STATE_NORM;
			redrawItem = true;
			handled = false;
		}
		break;

	case _ME_move:
		if (menu_CursorInsideItem(myItem, x, y)) {
			*currItem = theItem;
			if (myButton->itemFlags != BTN_STATE_OVER) {
				myButton->itemFlags = BTN_STATE_OVER;
				redrawItem = true;
			}
		} else {
			*currItem = nullptr;
			if (myButton->itemFlags != BTN_STATE_NORM) {
				myButton->itemFlags = BTN_STATE_NORM;
				redrawItem = true;
				handled = false;
			}
		}
		break;

	case _ME_L_hold:
	case _ME_doubleclick_hold:
		break;
	}

	// See if we need to redraw the button
	if (redrawItem) {
		(myItem->redraw)((void *)myItem, (void *)myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		//		  digi_play(inv_click_snd, 2, 255, -1, inv_click_snd_room_lock);
		currMenu = (guiMenu *)myItem->myMenu;
		currTag = myItem->tag;
		buttonClosesDialog = false;

		(myItem->callback)((void *)myItem, (void *)myItem->myMenu);

		status = 0;
		myScreen = buttonClosesDialog ? nullptr : vmng_screen_find((void *)myItem->myMenu, &status);

		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		} else {
			tempItem = menu_GetItem(currTag, currMenu);
			if (!tempItem) {
				*currItem = nullptr;
			}
		}
	}

	return handled;
}


menuItem *menu_ButtonAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, CALLBACK callback, int32 buttonType,
	bool greyed, bool transparent, const char *prompt, ItemHandlerFunction i_handler) {
	menuItem *newItem;
	menuItemButton *buttonInfo;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	if ((newItem = (menuItem *)mem_alloc(sizeof(menuItem), "gui menu item")) == nullptr) {
		return nullptr;
	}

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = (void *)myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = callback;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = menu_CopyBackground(myMenu, x, y, w, h);
	}

	if ((buttonInfo = (menuItemButton *)mem_alloc(sizeof(menuItemButton), "menu item button")) == nullptr) {
		return nullptr;
	}
	if (greyed) {
		buttonInfo->itemFlags = BTN_STATE_GREY;
	} else {
		buttonInfo->itemFlags = BTN_STATE_NORM;
	}
	buttonInfo->buttonType = buttonType;

	// Note: prompt is not duplicated, therefore, make sure the name is stored in non-volatile memory
	buttonInfo->prompt = prompt;
	buttonInfo->specialTag = tag - 1000;

	newItem->itemInfo = (void *)buttonInfo;

	newItem->redraw = menu_DrawButton;
	newItem->destroy = item_Destroy;
	newItem->itemEventHandler = i_handler;

	// Draw the button in now
	(newItem->redraw)(newItem, (void *)myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find((void *)myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


void menu_DisableButton(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	menuItemButton *myButton;

	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem) {
		myItem = menu_GetItem(tag, myMenu);
	}
	if ((!myItem) || (!myItem->itemInfo)) {
		return;
	}

	myButton = (menuItemButton *)myItem->itemInfo;
	myButton->itemFlags = BTN_STATE_GREY;
}


void menu_EnableButton(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	menuItemButton *myButton;

	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem) {
		myItem = menu_GetItem(tag, myMenu);
	}
	if ((!myItem) || (!myItem->itemInfo)) {
		return;
	}

	myButton = (menuItemButton *)myItem->itemInfo;
	myButton->itemFlags = BTN_STATE_NORM;
}


//-------------------------------    HSLIDER MENU ITEM    ---------------------------------//

enum {
	H_THUMB_NORM = 0,
	H_THUMB_OVER = 1,
	H_THUMB_PRESS = 2
};


void menu_DrawHSlider(void *theItem, void *theMenu, int32 x, int32 y, int32, int32) {
	menuItem *myItem = (menuItem *)theItem;
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItemHSlider *mySlider = nullptr;
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo) || (!myMenu)) {
		return;
	}

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	// Get the menu buffer and draw the sprite to it
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Get the slider info and select the thumb sprite
	mySlider = (menuItemHSlider *)myItem->itemInfo;
	switch (mySlider->itemFlags) {
	case H_THUMB_OVER:
		mySprite = _GM(menuSprites)[OM_SLIDER_BTN_OVER];
		break;
	case H_THUMB_PRESS:
		mySprite = _GM(menuSprites)[OM_SLIDER_BTN_PRESS];
		break;
	default:
	case H_THUMB_NORM:
		mySprite = _GM(menuSprites)[OM_SLIDER_BTN_NORM];
		break;
	}

	// Fill in everything left of the thumb with a hilite color
	if (mySlider->thumbX > 2) {
		gr_color_set(SLIDER_BAR_COLOR);
		gr_buffer_rect_fill(myBuff, myItem->x1 + 3, myItem->y1 + 9, mySlider->thumbX, mySlider->thumbH - 18);
	}

	// Draw in the thumb
	gui_DrawSprite(mySprite, myBuff, myItem->x1 + mySlider->thumbX, myItem->y1);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}


bool hslider_Handler(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	menuItem *myItem = (menuItem *)theItem;
	menuItemHSlider *mySlider;
	bool redrawItem, execCallback, handled;
	ScreenContext *myScreen;
	int32 status;
	int32 deltaSlide;
	static bool movingFlag;
	static int32 movingX;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo)) {
		return false;
	}
	mySlider = (menuItemHSlider *)myItem->itemInfo;

	if (!(eventType == EVENT_MOUSE)) {
		return false;
	}

	redrawItem = false;
	handled = true;
	execCallback = false;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menu_CursorInsideItem(myItem, x, y) && (x - myItem->x1 >= mySlider->thumbX) &&
			(x - myItem->x1 <= mySlider->thumbX + mySlider->thumbW - 1)) {
			//				  digi_play(inv_click_snd, 2, 255, -1, inv_click_snd_room_lock);
			mySlider->itemFlags = H_THUMB_PRESS;
			movingFlag = true;
			movingX = x;
			*currItem = theItem;
			redrawItem = true;
		} else {
			*currItem = nullptr;
			mySlider->itemFlags = 0;
			redrawItem = true;
		}
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (!*currItem) {
			return true;
		}
		if (movingFlag) {
			if (x < movingX) {
				deltaSlide = imath_min(mySlider->thumbX, movingX - x);
				if (deltaSlide > 0) {
					mySlider->thumbX -= deltaSlide;
					redrawItem = true;
					mySlider->percent = mySlider->thumbX * 100 / mySlider->maxThumbX;
					execCallback = true;
				}
			} else if (x > movingX) {
				deltaSlide = imath_min(mySlider->maxThumbX - mySlider->thumbX, x - movingX);
				if (deltaSlide > 0) {
					mySlider->thumbX += deltaSlide;
					redrawItem = true;
					mySlider->percent = mySlider->thumbX * 100 / mySlider->maxThumbX;
					execCallback = true;
				}
			}
			movingX = x;
			if (movingX < (mySlider->thumbX + myItem->x1)) {
				movingX = mySlider->thumbX + myItem->x1;
			} else if (movingX > (mySlider->thumbX + mySlider->thumbW - 1 + myItem->x1)) {
				movingX = mySlider->thumbX + mySlider->thumbW - 1 + myItem->x1;
			}
		} else {
			*currItem = nullptr;
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		if (!*currItem) {
			return true;
		}
		movingFlag = false;
		if (menu_CursorInsideItem(myItem, x, y) && (x - myItem->x1 >= mySlider->thumbX) &&
			(x - myItem->x1 <= mySlider->thumbX + mySlider->thumbW - 1)) {
			mySlider->itemFlags = H_THUMB_OVER;
			*currItem = theItem;
		} else {
			mySlider->itemFlags = H_THUMB_NORM;
			*currItem = nullptr;
		}
		redrawItem = true;
		execCallback = true;
		break;

	case _ME_move:
		if (menu_CursorInsideItem(myItem, x, y) && (x - myItem->x1 >= mySlider->thumbX) &&
			(x - myItem->x1 <= mySlider->thumbX + mySlider->thumbW - 1)) {
			if (mySlider->itemFlags != H_THUMB_OVER) {
				mySlider->itemFlags = H_THUMB_OVER;
				*currItem = theItem;
				redrawItem = true;
			}
		} else {
			if (mySlider->itemFlags != H_THUMB_NORM) {
				mySlider->itemFlags = H_THUMB_NORM;
				*currItem = nullptr;
				redrawItem = true;
				handled = false;
			}
		}
		break;

	case _ME_L_hold:
	case _ME_doubleclick_hold:
		break;
	}

	// See if we need to redraw the hslider
	if (redrawItem) {
		(myItem->redraw)((void *)myItem, (void *)myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		(myItem->callback)((void *)myItem, (void *)myItem->myMenu);
		myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}


menuItem *menu_HSliderAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
	int32 initPercent, CALLBACK callback, bool transparent) {
	menuItem *newItem;
	menuItemHSlider *sliderInfo;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	if ((newItem = (menuItem *)mem_alloc(sizeof(menuItem), "gui menu item")) == nullptr) {
		return nullptr;
	}

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = (void *)myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = callback;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = menu_CopyBackground(myMenu, x, y, w, h);
	}

	// Create a new slider
	if ((sliderInfo = (menuItemHSlider *)mem_alloc(sizeof(menuItemHSlider), "menu item hslider")) == nullptr) {
		return nullptr;
	}

	// Intialize the new slider
	sliderInfo->itemFlags = H_THUMB_NORM;
	sliderInfo->thumbW = _GM(menuSprites)[OM_SLIDER_BTN_NORM]->w;
	sliderInfo->thumbH = _GM(menuSprites)[OM_SLIDER_BTN_NORM]->h;
	sliderInfo->maxThumbX = w - _GM(menuSprites)[OM_SLIDER_BTN_NORM]->w;

	if (initPercent < 0) {
		initPercent = 0;
	} else if (initPercent > 100) {
		initPercent = 100;
	}

	// Calculate the initial thumbX
	sliderInfo->percent = initPercent;
	sliderInfo->thumbX = initPercent * sliderInfo->maxThumbX / 100;

	newItem->itemInfo = (void *)sliderInfo;

	newItem->redraw = menu_DrawHSlider;
	newItem->destroy = item_Destroy;
	newItem->itemEventHandler = hslider_Handler;

	// Draw the slider in now
	(newItem->redraw)(newItem, (void *)myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find((void *)myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


//-------------------------------    VSLIDER MENU ITEM    ---------------------------------//

enum {
	VS_NORM = 0x0000,
	VS_OVER = 0x0001,
	VS_PRESS = 0x0002,
	VS_GREY = 0x0003,
	VS_STATUS = 0x000f,
	VS_UP = 0x0010,
	VS_PAGE_UP = 0x0020,
	VS_THUMB = 0x0030,
	VS_PAGE_DOWN = 0x0040,
	VS_DOWN = 0x0050,
	VS_COMPONENT = 0x00f0
};


void menu_DrawVSlider(void *theItem, void *theMenu, int32 x, int32 y, int32, int32) {
	menuItem *myItem = (menuItem *)theItem;
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItemVSlider *myVSlider = nullptr;
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *upSprite = nullptr;
	Sprite *thumbSprite = nullptr;
	Sprite *downSprite = nullptr;
	Sprite *vbarSprite = nullptr;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo) || (!myMenu)) {
		return;
	}

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	myVSlider = (menuItemVSlider *)myItem->itemInfo;

	// Get the menu buffer
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Set the different sprite components
	vbarSprite = _GM(menuSprites)[SL_SCROLL_BAR];
	upSprite = _GM(menuSprites)[SL_UP_BTN_NORM];
	thumbSprite = _GM(menuSprites)[SL_SLIDER_BTN_NORM];
	downSprite = _GM(menuSprites)[SL_DOWN_BTN_NORM];

	if ((myVSlider->itemFlags & VS_STATUS) == VS_GREY) {
		upSprite = _GM(menuSprites)[SL_UP_BTN_GREY];
		thumbSprite = nullptr;
		downSprite = _GM(menuSprites)[SL_DOWN_BTN_GREY];
	} else if ((myVSlider->itemFlags & VS_STATUS) == VS_OVER) {
		if ((myVSlider->itemFlags & VS_COMPONENT) == VS_UP) {
			upSprite = _GM(menuSprites)[SL_UP_BTN_OVER];
		} else if ((myVSlider->itemFlags & VS_COMPONENT) == VS_THUMB) {
			thumbSprite = _GM(menuSprites)[SL_SLIDER_BTN_OVER];
		} else if ((myVSlider->itemFlags & VS_COMPONENT) == VS_DOWN) {
			downSprite = _GM(menuSprites)[SL_DOWN_BTN_OVER];
		}
	} else if ((myVSlider->itemFlags & VS_STATUS) == VS_PRESS) {
		if ((myVSlider->itemFlags & VS_COMPONENT) == VS_UP) {
			upSprite = _GM(menuSprites)[SL_UP_BTN_PRESS];
		} else if ((myVSlider->itemFlags & VS_COMPONENT) == VS_THUMB) {
			thumbSprite = _GM(menuSprites)[SL_SLIDER_BTN_PRESS];
		} else if ((myVSlider->itemFlags & VS_COMPONENT) == VS_DOWN) {
			downSprite = _GM(menuSprites)[SL_DOWN_BTN_PRESS];
		}
	}

	// Draw the sprite comonents
	gui_DrawSprite(vbarSprite, myBuff, x, y + upSprite->h);
	gui_DrawSprite(upSprite, myBuff, x, y);
	gui_DrawSprite(thumbSprite, myBuff, x, y + myVSlider->thumbY);
	gui_DrawSprite(downSprite, myBuff, x, y + upSprite->h + vbarSprite->h);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}

int32 vslider_WhereIsCursor(menuItemVSlider *myVSlider, int32 y) {
	if (y < myVSlider->minThumbY) {
		return VS_UP;
	} else if (y < myVSlider->thumbY) {
		return VS_PAGE_UP;
	} else if (y < myVSlider->thumbY + myVSlider->thumbH) {
		return VS_THUMB;
	} else if (y < myVSlider->maxThumbY + myVSlider->thumbH) {
		return VS_PAGE_DOWN;
	} else {
		return VS_DOWN;
	}
}

bool vslider_Handler(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	menuItem *myItem = (menuItem *)theItem;
	menuItemVSlider *myVSlider;
	bool redrawItem, execCallback, handled;
	int32 tempFlags;
	ScreenContext *myScreen;
	int32 status;
	int32 deltaSlide;
	int32 currTime;
	static bool movingFlag;
	static int32 movingY;
	static int32 callbackTime;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo)) {
		return false;
	}

	if (!(eventType == EVENT_MOUSE)) {
		return false;
	}

	myVSlider = (menuItemVSlider *)myItem->itemInfo;
	if ((myVSlider->itemFlags & VS_STATUS) == VS_GREY) {
		*currItem = nullptr;
		return false;
	}

	currTime = timer_read_60();
	redrawItem = false;
	handled = true;
	execCallback = false;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menu_CursorInsideItem(myItem, x, y)) {
			//				  digi_play(inv_click_snd, 2, 255, -1, inv_click_snd_room_lock);
			*currItem = theItem;
			tempFlags = vslider_WhereIsCursor(myVSlider, y - myItem->y1);
			if (tempFlags == VS_THUMB) {
				movingFlag = true;
				movingY = y;
			}
			if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
				myVSlider->itemFlags = tempFlags + VS_NORM;
			} else {
				myVSlider->itemFlags = tempFlags + VS_PRESS;
				redrawItem = true;
			}
			execCallback = true;
		} else {
			*currItem = nullptr;
			myVSlider->itemFlags = VS_NORM;
			redrawItem = true;
		}
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (!*currItem) {
			return true;
		}
		if (movingFlag) {
			if (y < movingY) {
				deltaSlide = imath_min(myVSlider->thumbY - myVSlider->minThumbY, movingY - y);
				if (deltaSlide > 0) {
					myVSlider->thumbY -= deltaSlide;
					myVSlider->percent = ((myVSlider->thumbY - myVSlider->minThumbY) * 100) /
						(myVSlider->maxThumbY - myVSlider->minThumbY);
					redrawItem = true;
					execCallback = true;
				}
			} else if (y > movingY) {
				deltaSlide = imath_min(myVSlider->maxThumbY - myVSlider->thumbY, y - movingY);
				if (deltaSlide > 0) {
					myVSlider->thumbY += deltaSlide;
					myVSlider->percent = ((myVSlider->thumbY - myVSlider->minThumbY) * 100) /
						(myVSlider->maxThumbY - myVSlider->minThumbY);
					redrawItem = true;
					execCallback = true;
				}
			}
			movingY = y;
			if (movingY < (myVSlider->thumbY + myItem->y1)) {
				movingY = myVSlider->thumbY + myItem->y1;
			} else if (movingY > (myVSlider->thumbY + myVSlider->thumbH - 1 + myItem->y1)) {
				movingY = myVSlider->thumbY + myVSlider->thumbH - 1 + myItem->y1;
			}
		} else {
			if (menu_CursorInsideItem(myItem, x, y)) {
				tempFlags = vslider_WhereIsCursor(myVSlider, y - myItem->y1);
				if ((myVSlider->itemFlags & VS_COMPONENT) == tempFlags) {
					if ((tempFlags != VS_PAGE_UP) && (tempFlags != VS_PAGE_DOWN) &&
						((myVSlider->itemFlags & VS_STATUS) != VS_PRESS)) {
						myVSlider->itemFlags = tempFlags + VS_PRESS;
						redrawItem = true;
					}
					if (currTime - callbackTime > 6) {
						execCallback = true;
					}
				} else {
					if ((myVSlider->itemFlags & VS_STATUS) != VS_OVER) {
						myVSlider->itemFlags = (myVSlider->itemFlags & VS_COMPONENT) + VS_OVER;
						redrawItem = true;
					}
				}
				execCallback = true;
			} else {
				if ((myVSlider->itemFlags & VS_STATUS) != VS_OVER) {
					myVSlider->itemFlags = (myVSlider->itemFlags & VS_COMPONENT) + VS_OVER;
					redrawItem = true;
				}
			}
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		movingFlag = false;
		if (menu_CursorInsideItem(myItem, x, y)) {
			tempFlags = vslider_WhereIsCursor(myVSlider, y - myItem->y1);
			if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
				myVSlider->itemFlags = VS_NORM;
			} else {
				myVSlider->itemFlags = tempFlags + VS_OVER;
				*currItem = theItem;
			}
		} else {
			myVSlider->itemFlags = VS_NORM;
			*currItem = nullptr;
		}
		redrawItem = true;
		if (!_GM(currMenuIsSave)) {
			UpdateThumbNails(_GM(firstSlotIndex), (guiMenu *)myItem->myMenu);
		}
		break;

	case _ME_move:
		if (menu_CursorInsideItem(myItem, x, y)) {
			*currItem = theItem;
			tempFlags = vslider_WhereIsCursor(myVSlider, y - myItem->y1);
			if ((myVSlider->itemFlags & VS_COMPONENT) != tempFlags) {
				if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
					myVSlider->itemFlags = VS_NORM;
				} else {
					myVSlider->itemFlags = tempFlags + VS_OVER;
				}
				redrawItem = true;
			}
		} else {
			*currItem = nullptr;
			if (myVSlider->itemFlags != VS_NORM) {
				myVSlider->itemFlags = VS_NORM;
				redrawItem = true;
				handled = false;
			}
		}
		break;

	case _ME_L_hold:
	case _ME_doubleclick_hold:
		if (!*currItem) {
			return true;
		}
		if (menu_CursorInsideItem(myItem, x, y)) {
			tempFlags = vslider_WhereIsCursor(myVSlider, y - myItem->y1);
			if ((myVSlider->itemFlags & VS_COMPONENT) == tempFlags) {
				if (currTime - callbackTime > 6) {
					execCallback = true;
				}
			}
		}
		break;
	}

	// See if we need to redraw the vslider
	if (redrawItem) {
		(myItem->redraw)((void *)myItem, (void *)myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		callbackTime = currTime;
		(myItem->callback)((void *)myItem, (void *)myItem->myMenu);
		myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}


menuItem *menu_VSliderAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
	int32 initPercent, CALLBACK callback, bool transparent) {

	menuItem *newItem;
	menuItemVSlider *vsliderInfo;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	if ((newItem = (menuItem *)mem_alloc(sizeof(menuItem), "gui menu item")) == nullptr) {
		return nullptr;
	}

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = (void *)myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = callback;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = menu_CopyBackground(myMenu, x, y, w, h);
	}

	if ((vsliderInfo = (menuItemVSlider *)mem_alloc(sizeof(menuItemVSlider), "menu item vslider")) == nullptr) {
		return nullptr;
	}
	vsliderInfo->itemFlags = VS_NORM;

	vsliderInfo->thumbW = _GM(menuSprites)[SL_SLIDER_BTN_NORM]->w;
	vsliderInfo->thumbH = _GM(menuSprites)[SL_SLIDER_BTN_NORM]->h;

	vsliderInfo->minThumbY = _GM(menuSprites)[SL_UP_BTN_NORM]->h + 1;
	vsliderInfo->maxThumbY = _GM(menuSprites)[SL_UP_BTN_NORM]->h + _GM(menuSprites)[SL_SCROLL_BAR]->h
		- _GM(menuSprites)[SL_SLIDER_BTN_NORM]->h - 1;

	// Calculate the initial thumbY
	vsliderInfo->percent = imath_max(imath_min(initPercent, 100), 0);
	vsliderInfo->thumbY = vsliderInfo->minThumbY +
		((vsliderInfo->percent * (vsliderInfo->maxThumbY - vsliderInfo->minThumbY)) / 100);

	newItem->itemInfo = (void *)vsliderInfo;

	newItem->redraw = menu_DrawVSlider;
	newItem->destroy = item_Destroy;
	newItem->itemEventHandler = vslider_Handler;

	// Draw the vslider in now
	(newItem->redraw)(newItem, (void *)myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find((void *)myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


void menu_DisableVSlider(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	menuItemVSlider *mySlider;

	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem) {
		myItem = menu_GetItem(tag, myMenu);
	}
	if ((!myItem) || (!myItem->itemInfo)) {
		return;
	}

	mySlider = (menuItemVSlider *)myItem->itemInfo;
	mySlider->itemFlags = VS_GREY;
}


void menu_EnableVSlider(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	menuItemVSlider *mySlider;

	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem) {
		myItem = menu_GetItem(tag, myMenu);
	}
	if ((!myItem) || (!myItem->itemInfo)) {
		return;
	}

	mySlider = (menuItemVSlider *)myItem->itemInfo;
	mySlider->itemFlags = VS_NORM;
}


//-----------------------------    TEXTFIELD MENU ITEM    ---------------------------------//

enum {
	TF_NORM = 0,
	TF_OVER = 1,
	TF_GREY = 2
};

void menu_DrawTextField(void *theItem, void *theMenu, int32 x, int32 y, int32, int32) {
	menuItem *myItem = (menuItem *)theItem;
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItemTextField *myText = nullptr;
	Buffer *myBuff = nullptr;
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;
	char tempStr[64], tempChar;
	int32 cursorX;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo) || (!myMenu)) {
		return;
	}

	// If the item is marked transparent, get the background buffer
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}
	}

	// Get the button info and select the sprite
	myText = (menuItemTextField *)myItem->itemInfo;

	switch (myText->itemFlags) {
	case TF_GREY:
		mySprite = _GM(menuSprites)[SL_LINE_NORM];
		break;

	case TF_OVER:
		mySprite = _GM(menuSprites)[SL_LINE_OVER];
		break;

	case TF_NORM:
	default:
		mySprite = _GM(menuSprites)[SL_LINE_OVER];
		break;
	}

	// Get the menu buffer and draw the sprite to it
	myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Draw the item sprite in
	gui_DrawSprite(mySprite, myBuff, x, y);

	//write in the special tag
	gr_font_set_color(TEXT_COLOR_NORM_FOREGROUND);
	Common::sprintf_s(tempStr, 64, "%02d", myText->specialTag);
	gr_font_set(_GM(menuFont));
	gr_font_write(myBuff, tempStr, x + 4, y + 1, 0, -1);

	//write in the text
	gr_font_write(myBuff, &myText->prompt[0], x + 26, y + 1, 0, -1);

	if (myText->itemFlags == TF_OVER) {
		// Draw in the cursor
		if (myText->cursor) {
			tempChar = *myText->cursor;
			*myText->cursor = '\0';
			cursorX = gr_font_string_width(&myText->prompt[0], -1);
			*myText->cursor = tempChar;

			gr_color_set(TEXT_COLOR_OVER_FOREGROUND);
			gr_vline(myBuff, x + cursorX + 26, y + 1, y + 12);
		}
	}

	// Release the menu buffer
	myMenu->menuBuffer->release();
}


bool textfield_Handler(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	menuItem *myItem = (menuItem *)theItem;
	menuItemTextField *myText;
	bool redrawItem, execCallback, handled;
	ScreenContext *myScreen;
	int32 status, temp;
	char tempStr[80], *tempPtr;

	// Verify params
	if ((!myItem) || (!myItem->itemInfo)) {
		return false;
	}
	myText = (menuItemTextField *)myItem->itemInfo;

	if (myText->itemFlags == TF_GREY) {
		return false;
	}

	redrawItem = false;
	execCallback = false;
	handled = true;

	if (eventType == EVENT_MOUSE) {
		switch (event) {
		case _ME_L_click:
		case _ME_doubleclick:
			_GM(deleteSaveDesc) = false;
			if (menu_CursorInsideItem(myItem, x, y)) {
				*currItem = theItem;
			}
			break;

		case _ME_L_drag:
		case _ME_doubleclick_drag:
			break;

		case _ME_L_release:
		case _ME_doubleclick_release:
			if (!*currItem) {
				return true;
			}
			*currItem = nullptr;
			if (menu_CursorInsideItem(myItem, x, y)) {
				if (myText->itemFlags == TF_OVER) {
					temp = strlen(myText->prompt);
					if (temp > 0) {
						Common::strcpy_s(tempStr, myText->prompt);
						tempPtr = &tempStr[temp];
						gr_font_set(_GM(menuFont));
						temp = gr_font_string_width(tempStr, -1);
						while ((tempPtr != &tempStr[0]) && (temp > x - myItem->x1 - 26)) {
							*--tempPtr = '\0';
							temp = gr_font_string_width(tempStr, -1);
						}
						myText->cursor = &myText->prompt[tempPtr - &tempStr[0]];
						redrawItem = true;
					}
				} else if (event == _ME_doubleclick_release) {
					execCallback = true;
				}
			}
			break;

		case _ME_move:
		case _ME_L_hold:
		case _ME_doubleclick_hold:
			break;
		}
	} else if ((eventType == EVENT_KEY) && (myText->itemFlags == TF_OVER)) {
		switch (event) {
		case KEY_RETURN:
			_GM(deleteSaveDesc) = false;
			execCallback = true;
			break;

		case KEY_HOME:
			_GM(deleteSaveDesc) = false;
			myText->cursor = &myText->prompt[0];
			redrawItem = true;
			break;

		case KEY_END:
			_GM(deleteSaveDesc) = false;
			myText->cursor = myText->promptEnd;
			redrawItem = true;
			break;

		case KEY_LEFT:
			_GM(deleteSaveDesc) = false;
			if (myText->cursor > &myText->prompt[0]) {
				myText->cursor--;
				redrawItem = true;
			}
			break;

		case KEY_RIGHT:
			_GM(deleteSaveDesc) = false;
			if (myText->cursor < myText->promptEnd) {
				myText->cursor++;
				redrawItem = true;
			}
			break;

		case KEY_DELETE:
			if (_GM(deleteSaveDesc)) {
				myText->prompt[0] = '\0';
				myText->promptEnd = &myText->prompt[0];
				myText->cursor = myText->promptEnd;
				redrawItem = true;
			} else if (myText->cursor < myText->promptEnd) {
				Common::strcpy_s(tempStr, (char *)(myText->cursor + 1));
				Common::strcpy_s(myText->cursor, 80, tempStr);
				myText->promptEnd--;
				redrawItem = true;
			}
			break;

		case KEY_BACKSP:
			_GM(deleteSaveDesc) = false;
			if (myText->cursor > &myText->prompt[0]) {
				Common::strcpy_s(tempStr, myText->cursor);
				myText->promptEnd--;
				myText->cursor--;
				Common::strcpy_s(myText->cursor, 80, tempStr);
				redrawItem = true;
			}
			break;

		default:
			_GM(deleteSaveDesc) = false;
			gr_font_set(_GM(menuFont));
			temp = gr_font_string_width(&myText->prompt[0], -1);
			if ((strlen(&myText->prompt[0]) < 79) && (temp < myText->pixWidth - 12) && (event >= 32) && (event <= 127)) {
				if (myText->cursor < myText->promptEnd) {
					Common::strcpy_s(tempStr, (char *)myText->cursor);
					Common::sprintf_s(myText->cursor, 80, "%c%s", (char)event, tempStr);
				} else {
					*myText->cursor = (char)event;
					*(myText->cursor + 1) = '\0';
				}
				myText->cursor++;
				myText->promptEnd++;

				redrawItem = true;
			}
			break;
		}
	} else if ((eventType == EVENT_KEY) && (event == KEY_RETURN)) {
		// The only events a NORM textfield can respond to are doubleclick_release and <return> keypress
		execCallback = true;
	} else {
		// Otherwise the event will not be handled
		return false;
	}

	// See if we need to redraw the button
	if (redrawItem) {
		(myItem->redraw)((void *)myItem, (void *)myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		(myItem->callback)((void *)myItem, (void *)myItem->myMenu);
		myScreen = vmng_screen_find((void *)myItem->myMenu, &status);

		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}


menuItem *menu_TextFieldAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, int32 initFlags,
		const char *prompt, int32 specialTag, CALLBACK callback, bool transparent) {
	menuItem *newItem;
	menuItemTextField *textInfo;
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	if ((newItem = (menuItem *)mem_alloc(sizeof(menuItem), "gui menu item")) == nullptr) {
		return nullptr;
	}

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = (void *)myMenu;
	newItem->tag = tag;
	newItem->x1 = x;
	newItem->y1 = y;
	newItem->x2 = x + w - 1;
	newItem->y2 = y + h - 1;
	newItem->callback = callback;

	if (!transparent) {
		newItem->transparent = false;
		newItem->background = nullptr;
	} else {
		newItem->transparent = true;
		newItem->background = menu_CopyBackground(myMenu, x, y, w, h);
	}

	if ((textInfo = (menuItemTextField *)mem_alloc(sizeof(menuItemTextField), "menu item textfield")) == nullptr) {
		return nullptr;
	}
	textInfo->itemFlags = initFlags;

	textInfo->specialTag = specialTag;
	textInfo->pixWidth = w - 27;
	if (prompt) {
		Common::strcpy_s(&textInfo->prompt[0], 80, prompt);
		textInfo->promptEnd = &textInfo->prompt[strlen(prompt)];
	} else {
		textInfo->prompt[0] = '\0';
		textInfo->promptEnd = &textInfo->prompt[0];
	}
	textInfo->cursor = textInfo->promptEnd;

	newItem->itemInfo = (void *)textInfo;

	newItem->redraw = menu_DrawTextField;
	newItem->destroy = item_Destroy;
	newItem->itemEventHandler = textfield_Handler;

	// Draw the vslider in now
	(newItem->redraw)(newItem, (void *)myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	myScreen = vmng_screen_find((void *)myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


//-------------------------------------   GAME MENU   -------------------------------------//


#define GAME_MENU_X		190
#define GAME_MENU_Y		100
#define GAME_MENU_W		260
#define GAME_MENU_H		198

#define GM_MAIN_X			 45
#define GM_MAIN_Y			 53
#define GM_MAIN_W			 24
#define GM_MAIN_H			 24

#define GM_OPTIONS_X		 45
#define GM_OPTIONS_Y		 94
#define GM_OPTIONS_W		 24
#define GM_OPTIONS_H		 24

#define GM_RESUME_X		 45
#define GM_RESUME_Y		135
#define GM_RESUME_W		 24
#define GM_RESUME_H		 24

#define GM_QUIT_X			141
#define GM_QUIT_Y			135
#define GM_QUIT_W			 24
#define GM_QUIT_H		    24

#define GM_SAVE_X			141
#define GM_SAVE_Y			 53
#define GM_SAVE_W			 24
#define GM_SAVE_H		    24

#define GM_LOAD_X			141
#define GM_LOAD_Y			 94
#define GM_LOAD_W			 24
#define GM_LOAD_H			 24

static void DestroyGameMenu();

void cb_Game_Quit(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();

	// Shutdown the menu system
	menu_Shutdown(false);

	// Set the global that will cause the entire game to exit to dos
	_G(kernel).going = false;
}

void cb_Game_Resume(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();

	// Shutdown the menu system
	menu_Shutdown(true);
}

void cb_Game_Save(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();
	menu_Shutdown(true);
	buttonClosesDialog = true;

	// Create the save game menu
	g_engine->showSaveScreen();
}

void cb_Game_Load(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();
	menu_Shutdown(true);
	buttonClosesDialog = true;

	// Create the save game menu
	g_engine->showLoadScreen(M4Engine::kLoadFromGameDialog);
}

void cb_Game_Main(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();

	if (!_GM(gameMenuFromMain)) {
		// Save the game so we can resume from here if possible
		if (_GM(interfaceWasVisible) && player_commands_allowed()) {
			other_save_game_for_resurrection();
		}

		// Make sure the interface does not reappear
		_GM(interfaceWasVisible) = false;

		// Shutdown the menu system
		menu_Shutdown(false);
	} else {
		menu_Shutdown(true);
	}

	// Go to the main menu
	_G(game).setRoom(_G(executing) == WHOLE_GAME ? 903 : 901);
}

void cb_Game_Options(void *, void *) {
	// Destroy the game menu
	DestroyGameMenu();
	buttonClosesDialog = true;

	// Create the options menu
	CreateOptionsMenu(nullptr);
}

void DestroyGameMenu(void) {
	if (!_GM(gameMenu)) {
		return;
	}

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(gameMenu));

	// Destroy the menu resources
	menu_Destroy(_GM(gameMenu));

	// Unload the menu sprites
	menu_UnloadSprites();
}

void CreateGameMenuMain(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		menu_Initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!menu_LoadSprites("gamemenu", GM_TOTAL_SPRITES)) {
		return;
	}

	_GM(gameMenu) = menu_Create(_GM(menuSprites)[GM_DIALOG_BOX], GAME_MENU_X, GAME_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(gameMenu)) {
		return;
	}

	menu_ButtonAdd(_GM(gameMenu), GM_TAG_MAIN, GM_MAIN_X, GM_MAIN_Y, GM_MAIN_W, GM_MAIN_H, cb_Game_Main);
	menu_ButtonAdd(_GM(gameMenu), GM_TAG_OPTIONS, GM_OPTIONS_X, GM_OPTIONS_Y, GM_OPTIONS_W, GM_OPTIONS_H, cb_Game_Options);
	menu_ButtonAdd(_GM(gameMenu), GM_TAG_RESUME, GM_RESUME_X, GM_RESUME_Y, GM_RESUME_W, GM_RESUME_H, cb_Game_Resume);
	menu_ButtonAdd(_GM(gameMenu), GM_TAG_QUIT, GM_QUIT_X, GM_QUIT_Y, GM_QUIT_W, GM_QUIT_H, cb_Game_Quit);

	if (!_GM(gameMenuFromMain)) {
		menu_ButtonAdd(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cb_Game_Save);
	} else {
		menu_ButtonAdd(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cb_Game_Save, BTN_TYPE_GM_GENERIC, true);
	}

	// See if there are any games to load
	if (g_engine->savesExist()) {
		menu_ButtonAdd(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cb_Game_Load);
	} else {
		menu_ButtonAdd(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cb_Game_Load, BTN_TYPE_GM_GENERIC, true);
	}

	// Configure the game so pressing <esc> will cause the menu to disappear and the game to resume
	menu_Configure(_GM(gameMenu), cb_Game_Resume, cb_Game_Resume);

	vmng_screen_show((void *)_GM(gameMenu));
	LockMouseSprite(0);
}


//----------------------------------   OPTIONS MENU   -------------------------------------//

void DestroyOptionsMenu();


void cb_Options_Game_Cancel(void *, void *) {
	// Reset values of items to what they were when options menu came up
	digi_set_overall_volume(_GM(remember_digi_volume));
	_G(flags)[digestability] = _GM(remember_digestability);

	// Destroy the options menu
	DestroyOptionsMenu();
	buttonClosesDialog = true;

	// Create the options menu
	CreateGameMenuMain(nullptr);
}

void cb_Options_Game_Done(void *, void *) {
	// Destroy the options menu
	DestroyOptionsMenu();
	buttonClosesDialog = true;

	// Create the options menu
	CreateGameMenuMain(nullptr);
}

void cb_Options_Digi(void *theItem, void *theMenu) {
	menuItem *myItem = (menuItem *)theItem;
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItemHSlider *mySlider;

	mySlider = (menuItemHSlider *)myItem->itemInfo;
	// Set the digi volume
	digi_set_overall_volume(mySlider->percent);
	term_message("digi volume: %d", mySlider->percent);

	// This scroller control has been moved, so make sure that the DONE button is not greyed out
	menu_EnableButton(nullptr, OM_TAG_DONE, myMenu);
	menu_ItemRefresh(nullptr, OM_TAG_DONE, myMenu);

}

void cb_Options_Digestability(void *theItem, void *theMenu) {
	menuItem *myItem = (menuItem *)theItem;
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItemHSlider *mySlider;

	mySlider = (menuItemHSlider *)myItem->itemInfo;
	// Set the midi volume
	term_message("digestability: %d", mySlider->percent);
	_G(flags)[digestability] = mySlider->percent;

	// This scroller control has been moved, so make sure that the DONE button is not greyed out
	menu_EnableButton(nullptr, OM_TAG_DONE, myMenu);
	menu_ItemRefresh(nullptr, OM_TAG_DONE, myMenu);
}

void DestroyOptionsMenu(void) {

	if (!_GM(opMenu)) {
		return;
	}

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(opMenu));

	// Destroy the menu resources
	menu_Destroy(_GM(opMenu));

	// Unload the menu sprites
	menu_UnloadSprites();
}


void CreateOptionsMenu(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		menu_Initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!menu_LoadSprites("opmenu", OM_TOTAL_SPRITES)) {
		return;
	}

	_GM(opMenu) = menu_Create(_GM(menuSprites)[OM_DIALOG_BOX], OPTIONS_MENU_X, OPTIONS_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(opMenu)) {
		return;
	}

	menu_ButtonAdd(_GM(opMenu), OM_TAG_CANCEL, OM_CANCEL_X, OM_CANCEL_Y, OM_CANCEL_W, OM_CANCEL_H, cb_Options_Game_Cancel, BTN_TYPE_OM_CANCEL);
	menu_ButtonAdd(_GM(opMenu), OM_TAG_DONE, OM_DONE_X, OM_DONE_Y, OM_DONE_W, OM_DONE_H, cb_Options_Game_Done, BTN_TYPE_OM_DONE, true);
	menu_HSliderAdd(_GM(opMenu), OM_TAG_DIGI, OM_DIGI_X, OM_DIGI_Y, OM_DIGI_W, OM_DIGI_H, digi_get_overall_volume(), cb_Options_Digi, true);
	menu_HSliderAdd(_GM(opMenu), OM_TAG_DIGESTABILITY, OM_DIGESTABILITY_X, OM_DIGESTABILITY_Y,
		OM_DIGESTABILITY_W, OM_DIGESTABILITY_H, _G(flags)[digestability], cb_Options_Digestability, true);

	// Remember the values of the items in case the user cancels
	_GM(remember_digi_volume) = digi_get_overall_volume();
	_GM(remember_digestability) = _G(flags)[digestability];

	// Configure the game so pressing <esc> will cause the menu to disappear and the gamemenu to reappear
	menu_Configure(_GM(opMenu), cb_Options_Game_Done, cb_Options_Game_Cancel);

	vmng_screen_show((void *)_GM(opMenu));
	LockMouseSprite(0);
}

//------------------------------------------------------------------------------------//
//--------------------------------   ERR MENU   --------------------------------------//
//------------------------------------------------------------------------------------//

void DestroyErrMenu(void);

void cb_Err_Done(void *, void *) {
	// Destroy the game menu
	DestroyErrMenu();

	// Shutdown the menu system
	menu_Shutdown(true);
}


void DestroyErrMenu(void) {
	if (!_GM(errMenu)) {
		return;
	}

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(errMenu));

	// Destroy the menu resources
	menu_Destroy(_GM(errMenu));

	// Unload the menu sprites
	menu_UnloadSprites();
}


void CreateErrMenu(RGB8 *myPalette) {
	Buffer *myBuff;

	if (!_G(menuSystemInitialized)) {
		menu_Initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!menu_LoadSprites("errmenu", 5)) {
		return;
	}

	_GM(errMenu) = menu_Create(_GM(menuSprites)[EM_DIALOG_BOX], ERROR_MENU_X, ERROR_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(errMenu)) {
		return;
	}

	// Get the menu buffer
	myBuff = _GM(errMenu)->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	//write the err message
	gr_font_set_color(TEXT_COLOR_NORM_FOREGROUND);
	gr_font_write(myBuff, "Save game failed!", 48, 8, 0, -1);

	gr_font_write(myBuff, "A disk error has", 48, 23, 0, -1);
	gr_font_write(myBuff, "occurred.", 48, 33, 0, -1);

	gr_font_write(myBuff, "Please ensure you", 48, 48, 0, -1);
	gr_font_write(myBuff, "have write access", 48, 58, 0, -1);
	gr_font_write(myBuff, "and sufficient", 48, 68, 0, -1);
	gr_font_write(myBuff, "disk space (40k).", 48, 78, 0, -1);

	_GM(errMenu)->menuBuffer->release();

	// Add the done button
	menu_ButtonAdd(_GM(errMenu), EM_TAG_RETURN, EM_RETURN_X, EM_RETURN_Y, EM_RETURN_W, EM_RETURN_H, cb_Err_Done);

	// Configure the game so pressing <esc> will cause the menu to disappear and the gamemenu to reappear
	menu_Configure(_GM(errMenu), cb_Err_Done, cb_Err_Done);

	vmng_screen_show((void *)_GM(errMenu));
	LockMouseSprite(0);
}


//--------------------------------   SAVE / LOAD MENU   -----------------------------------//

void DestroySaveLoadMenu(bool saveMenu);
void cb_SaveLoad_Slot(void *theItem, void *theMenu);
bool load_Handler(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem);


bool LoadThumbNail(int32 slotNum) {
	Sprite *&thumbNailSprite = _GM(thumbNails)[slotNum];
	return g_engine->loadSaveThumbnail(slotNum + 1, thumbNailSprite);
}


void UnloadThumbNail(int32 slotNum) {
	if (_GM(thumbNails)[slotNum]->sourceHandle) {
		HUnLock(_GM(thumbNails)[slotNum]->sourceHandle);
		DisposeHandle(_GM(thumbNails)[slotNum]->sourceHandle);
		_GM(thumbNails)[slotNum]->sourceHandle = nullptr;
	}
}


void UpdateThumbNails(int32 firstSlot, guiMenu *myMenu) {
	int32 i, startIndex, endIndex;

	// Make sure there is something to update
	if (firstSlot == _GM(thumbIndex)) {
		return;
	}

	// Ensure firstSlot is in a valid range
	firstSlot = imath_max(imath_min(firstSlot, 89), 0);

	if (firstSlot > _GM(thumbIndex)) {
		// Dump Out all thumbnails in slots which don't overlap
		startIndex = _GM(thumbIndex);
		endIndex = imath_min(_GM(thumbIndex) + 9, firstSlot - 1);
		for (i = startIndex; i <= endIndex; i++) {
			UnloadThumbNail(i);
		}

		// Load in all thumbnails missing thumbnails
		startIndex = imath_max(_GM(thumbIndex) + 10, firstSlot);
		endIndex = imath_min(firstSlot + 9, 98);
		for (i = startIndex; i <= endIndex; i++) {
			if (_GM(slotInUse)[i]) {
				if (!LoadThumbNail(i)) {
					_GM(slotInUse)[i] = false;
					menu_DisableButton(nullptr, 1001 + i - firstSlot, myMenu);
					menu_ItemRefresh(nullptr, 1001 + i - firstSlot, myMenu);
				}
			}
		}
	} else {
		// Else firstSlot < _GM(thumbIndex)
		// Dump Out all thumbnails in slots which don't overlap
		startIndex = imath_max(firstSlot + 10, _GM(thumbIndex));
		endIndex = imath_min(_GM(thumbIndex) + 9, 98);
		for (i = startIndex; i <= endIndex; i++) {
			UnloadThumbNail(i);
		}

		// Load in all thumbnails missing thumbnails
		startIndex = firstSlot;
		endIndex = imath_min(firstSlot + 9, _GM(thumbIndex) - 1);
		for (i = startIndex; i <= endIndex; i++) {
			if (_GM(slotInUse)[i]) {
				if (!LoadThumbNail(i)) {
					_GM(slotInUse)[i] = false;
					menu_DisableButton(nullptr, 1001 + i - firstSlot, myMenu);
					menu_ItemRefresh(nullptr, 1001 + i - firstSlot, myMenu);
				}
			}
		}
	}

	// Set the var
	_GM(thumbIndex) = firstSlot;
}


void SetFirstSlot(int32 firstSlot, guiMenu *myMenu) {
	menuItem *tempItem;
	menuItemButton *myButton;
	int32 i;

	if (!myMenu) {
		return;
	}

	// Ensure firstSlot is in a valid range
	firstSlot = imath_max(imath_min(firstSlot, 89), 0);

	// Change the prompt and special tag of each of the slot buttons
	for (i = 0; i < MAX_SLOTS_SHOWN; i++) {
		tempItem = menu_GetItem(i + 1001, myMenu);
		myButton = (menuItemButton *)tempItem->itemInfo;
		myButton->prompt = _GM(slotTitles)[firstSlot + i];
		if (_GM(currMenuIsSave) || _GM(slotInUse)[firstSlot + i]) {
			myButton->itemFlags = BTN_STATE_NORM;
		} else {
			myButton->itemFlags = BTN_STATE_GREY;
		}
		myButton->specialTag = firstSlot + i + 1;
		menu_ItemRefresh(tempItem, i + 1001, myMenu);
	}
}


void cb_SaveLoad_VSlider(void *theItem, void *theMenu) {
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItem *myItem = (menuItem *)theItem;
	menuItemVSlider *mySlider;
	bool redraw;

	if ((!myMenu) || (!myItem) || (!myItem->itemInfo)) {
		return;
	}

	// Get my slider
	mySlider = (menuItemVSlider *)myItem->itemInfo;

	if ((mySlider->itemFlags & VS_COMPONENT) != VS_THUMB) {

		redraw = false;
		switch (mySlider->itemFlags & VS_COMPONENT) {

		case VS_UP:
			if (_GM(firstSlotIndex) > 0) {
				_GM(firstSlotIndex)--;
				redraw = true;
			}
			break;

		case VS_PAGE_UP:
			if (_GM(firstSlotIndex) > 0) {
				_GM(firstSlotIndex) = imath_max(_GM(firstSlotIndex) - 10, 0);
				redraw = true;
			}
			break;

		case VS_PAGE_DOWN:
			if (_GM(firstSlotIndex) < 89) {
				_GM(firstSlotIndex) = imath_min(_GM(firstSlotIndex) + 10, 89);
				redraw = true;
			}
			break;

		case VS_DOWN:
			if (_GM(firstSlotIndex) < 89) {
				_GM(firstSlotIndex)++;
				redraw = true;
			}
			break;
		}

		// See if we were able to set a new first slot index
		if (redraw) {
			SetFirstSlot(_GM(firstSlotIndex), myMenu);

			// Calculate the new percent
			mySlider->percent = (_GM(firstSlotIndex) * 100) / 89;

			// Calculate the new thumbY
			mySlider->thumbY = mySlider->minThumbY +
				((mySlider->percent * (mySlider->maxThumbY - mySlider->minThumbY)) / 100);

			// Redraw the slider
			menu_ItemRefresh(myItem, -1, myMenu);
		}
	}

	// Else the callback came from the thumb - set the _GM(firstSlotIndex) based on the slider percent
	else {
		_GM(firstSlotIndex) = (mySlider->percent * 89) / 100;
		SetFirstSlot(_GM(firstSlotIndex), myMenu);
	}
}


void cb_SaveLoad_Save(void *, void *theMenu) {
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItem *myTextItem;
	menuItemTextField *myText;
	bool saveGameFailed;

	// If (slotSelected < 0) this callback is being executed by pressing return prematurely
	if (_GM(slotSelected) < 0) {
		return;
	}

	// First make the textfield NORM
	myTextItem = menu_GetItem(2000, myMenu);
	if ((!myTextItem) || (!myTextItem->itemInfo)) {
		return;
	}
	myText = (menuItemTextField *)myTextItem->itemInfo;
	myText->itemFlags = TF_NORM;

	// Set the vars
	_GM(slotInUse)[_GM(slotSelected) - 1] = true;
	Common::strcpy_s(_GM(slotTitles)[_GM(slotSelected) - 1], 80, myText->prompt);

	// Save the game
	saveGameFailed = !g_engine->saveGameFromMenu(_GM(slotSelected),
		myText->prompt, _GM(_thumbnail));

	// If the save game failed, bring up the err menu
	if (saveGameFailed) {
		// Kill the save menu
		DestroySaveLoadMenu(true);

		// Create the err menu
		CreateErrMenu(nullptr);

		// Abort this procedure
		return;
	}

	// Kill the save menu
	DestroySaveLoadMenu(true);

	// Shutdown the menu system
	menu_Shutdown(true);
}


void cb_SaveLoad_Load(void *, void *theMenu) {
	KernelTriggerType oldMode;

	// If (slotSelected < 0) this callback is being executed by pressing return prematurely
	if (_GM(slotSelected) < 0) {
		return;
	}

	// Kill the menu
	DestroySaveLoadMenu(false);

	// Shutdown the menu system
	menu_Shutdown(false);

	// See if we need to reset the ESC, F2, and F3 hotkeys
	if (_GM(gameMenuFromMain)) {
		AddSystemHotkey(KEY_ESCAPE, Burger::Hotkeys::escape_key_pressed);
		AddSystemHotkey(KEY_F2, M4::Hotkeys::saveGame);
		AddSystemHotkey(KEY_F3, M4::Hotkeys::loadGame);
	}

	// Start the restore process
	_G(kernel).restore_slot = _GM(slotSelected);
	oldMode = _G(kernel).trigger_mode;

	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(TRIG_RESTORE_GAME);
	_G(kernel).trigger_mode = oldMode;
}


void cb_SaveLoad_Cancel(void *, void *theMenu) {
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItem *myItem;
	int32 i, x, y, w, h;

	// If a slot has been selected, cancel will re-enable all slots
	if (_GM(slotSelected) >= 0) {
		// Enable the prev buttons
		for (i = 1001; i <= 1010; i++) {
			if (_GM(currMenuIsSave) || _GM(slotInUse)[i - 1001 + _GM(firstSlotIndex)]) {
				menu_EnableButton(nullptr, i, myMenu);
				menu_ItemRefresh(nullptr, i, myMenu);
			}
		}

		// Find the textfield and use it's coords to place the button
		myItem = menu_GetItem(2000, myMenu);
		x = myItem->x1;
		y = myItem->y1;
		w = myItem->x2 - myItem->x1 + 1;
		h = myItem->y2 - myItem->y1 + 1;

		// Delete the textfield
		menu_ItemDelete(myItem, 2000, myMenu);

		// Add the button back in
		if (_GM(currMenuIsSave)) {
			menu_ButtonAdd(myMenu, 1000 + _GM(slotSelected) - _GM(firstSlotIndex), x, y, w, h,
				cb_SaveLoad_Slot, BTN_TYPE_SL_TEXT, false, true, _GM(slotTitles)[_GM(slotSelected) - 1], button_Handler);
		} else {
			menu_ButtonAdd(myMenu, 1000 + _GM(slotSelected) - _GM(firstSlotIndex), x, y, w, h,
				cb_SaveLoad_Slot, BTN_TYPE_SL_TEXT, false, true, _GM(slotTitles)[_GM(slotSelected) - 1], load_Handler);

			// Remove the thumbnail
			if (_GM(saveLoadThumbNail)) {
				_GM(saveLoadThumbNail) = _GM(menuSprites)[SL_EMPTY_THUMB];
				menu_ItemRefresh(nullptr, SL_TAG_THUMBNAIL, myMenu);
			}
		}
		SetFirstSlot(_GM(firstSlotIndex), myMenu);

		// Enable the slider
		menu_EnableVSlider(nullptr, SL_TAG_VSLIDER, myMenu);
		menu_ItemRefresh(nullptr, SL_TAG_VSLIDER, myMenu);

		// Disable the save/load button
		if (_GM(currMenuIsSave)) {
			menu_DisableButton(nullptr, SL_TAG_SAVE, myMenu);
			menu_ItemRefresh(nullptr, SL_TAG_SAVE, myMenu);
		} else {
			menu_DisableButton(nullptr, SL_TAG_LOAD, myMenu);
			menu_ItemRefresh(nullptr, SL_TAG_LOAD, myMenu);
		}

		// Reset the slot selected var
		_GM(slotSelected) = -1;

	} else {
		// Otherwise, back to the game menu

		// Destroy the menu
		DestroySaveLoadMenu(_GM(currMenuIsSave));

		if (_GM(saveLoadFromHotkey)) {
			// Shutdown the menu system
			menu_Shutdown(true);
		} else {
			// Create the game menu
			CreateGameMenuMain(nullptr);
		}
	}

	buttonClosesDialog = true;
}


void cb_SaveLoad_Slot(void *theItem, void *theMenu) {
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItem *myItem = (menuItem *)theItem;
	menuItemButton *myButton;
	int32 i, x, y, w, h;
	char prompt[80];
	int32 specialTag;

	// Verify params
	if ((!myMenu) || (!myItem) || (!myItem->itemInfo)) {
		return;
	}

	// Get the button
	myButton = (menuItemButton *)myItem->itemInfo;
	Common::strcpy_s(prompt, 80, myButton->prompt);
	specialTag = myButton->specialTag;

	// Set the globals
	_GM(slotSelected) = myButton->specialTag;
	_GM(deleteSaveDesc) = true;

	// Disable all other buttons
	for (i = 1001; i <= 1010; i++) {
		if (i != myItem->tag) {
			menu_DisableButton(nullptr, i, myMenu);
			menu_ItemRefresh(nullptr, i, myMenu);
		}
	}

	// Get the slot coords, and delete it
	x = myItem->x1;
	y = myItem->y1;
	w = myItem->x2 - myItem->x1 + 1;
	h = myItem->y2 - myItem->y1 + 1;
	menu_ItemDelete(myItem, -1, myMenu);

	if (_GM(currMenuIsSave)) {
		// Replace the current button with a textfield
		if (!strcmp(prompt, "<empty>")) {
			menu_TextFieldAdd(myMenu, 2000, x, y, w, h, TF_OVER,
				nullptr, specialTag, cb_SaveLoad_Save, true);
		} else {
			menu_TextFieldAdd(myMenu, 2000, x, y, w, h, TF_OVER,
				prompt, specialTag, cb_SaveLoad_Save, true);
		}
	} else {
		menu_TextFieldAdd(myMenu, 2000, x, y, w, h, TF_NORM,
			prompt, specialTag, cb_SaveLoad_Load, true);
	}

	// Disable the slider
	menu_DisableVSlider(nullptr, SL_TAG_VSLIDER, myMenu);
	menu_ItemRefresh(nullptr, SL_TAG_VSLIDER, myMenu);

	// Enable the save/load button
	if (_GM(currMenuIsSave)) {
		menu_EnableButton(nullptr, SL_TAG_SAVE, myMenu);
		menu_ItemRefresh(nullptr, SL_TAG_SAVE, myMenu);
	} else {
		menu_EnableButton(nullptr, SL_TAG_LOAD, myMenu);
		menu_ItemRefresh(nullptr, SL_TAG_LOAD, myMenu);
	}
}

void InitializeSlotTables(void) {
	const SaveStateList saves = g_engine->listSaves();

	// First reset all the slots to empty
	for (int i = 0; i < MAX_SLOTS; ++i) {
		Common::strcpy_s(_GM(slotTitles)[i], 80, "<empty>");
		_GM(slotInUse)[i] = false;
	}

	for (const auto &save : saves) {
		if (save.getSaveSlot() != 0) {
			Common::String desc = save.getDescription();
			Common::strcpy_s(_GM(slotTitles)[save.getSaveSlot() - 1], 80, desc.c_str());
			_GM(slotInUse)[save.getSaveSlot() - 1] = true;
		}
	}
}

bool load_Handler(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	menuItem *myItem = (menuItem *)theItem;
	menuItemButton *myButton;
	bool handled;

	// Handle the event just like any other button
	handled = button_Handler(theItem, eventType, event, x, y, currItem);

	// If we've selected a slot, we want the thumbNail to remain on the menu permanently
	if (_GM(slotSelected) >= 0) {
		return handled;
	}

	// But if the event moved the mouse, we want to display the correct thumbNail;
	if ((eventType == EVENT_MOUSE) && ((event == _ME_move) || (event == _ME_L_drag) || (event == _ME_L_release) ||
		(event == _ME_doubleclick_drag) || (event == _ME_doubleclick_release))) {

		// Get the button
		if ((!myItem) || (!myItem->itemInfo)) {
			return handled;
		}
		myButton = (menuItemButton *)myItem->itemInfo;

		// This determines that we are over the button
		if ((myButton->itemFlags == BTN_STATE_OVER) || (myButton->itemFlags == BTN_STATE_PRESS)) {
			// See if the current _GM(saveLoadThumbNail) is pointing to the correct sprite
			if (_GM(saveLoadThumbNail) != _GM(thumbNails)[myButton->specialTag - 1]) {
				_GM(saveLoadThumbNail) = _GM(thumbNails)[myButton->specialTag - 1];
				menu_ItemRefresh(nullptr, SL_TAG_THUMBNAIL, (guiMenu *)myItem->myMenu);
			}
		}

		// Else we must determine whether the thumbnail needs to be replaced with the empty thumbnail.
		else {

			// If the mouse has moved outside of the entire range of all 10 buttons,
			//or it is over a button which is not hilited it is to be removed.
			if (menu_CursorInsideItem(myItem, x, y)
				|| (x < SL_SCROLL_FIELD_X)
				|| (x > SL_SCROLL_FIELD_X + SL_SCROLL_FIELD_W)
				|| (y < SL_SCROLL_FIELD_Y)
				|| (y > SL_SCROLL_FIELD_Y + SL_SCROLL_FIELD_H)) {

				// Remove the thumbnail
				if (_GM(saveLoadThumbNail)) {
					_GM(saveLoadThumbNail) = _GM(menuSprites)[SL_EMPTY_THUMB];
					menu_ItemRefresh(nullptr, SL_TAG_THUMBNAIL, (guiMenu *)myItem->myMenu);
				}
			}
		}
	}
	return handled;
}


void DestroySaveLoadMenu(bool saveMenu) {
	int32 i;

	if (!_GM(slMenu)) {
		return;
	}

	// Determine whether the screen was the SAVE or the LOAD menu
	if (saveMenu) {

		// If SAVE, there should be a thumbnail to unload
		if (_GM(saveLoadThumbNail)) {
			DisposeHandle(_GM(saveLoadThumbNail)->sourceHandle);
			mem_free(_GM(saveLoadThumbNail));
			_GM(saveLoadThumbNail) = nullptr;
		}
	} else {
		// Else there may be up to 10 somewhere in the list to be unloaded
		for (i = 0; i < MAX_SLOTS; i++) {
			UnloadThumbNail(i);
		}
		_GM(saveLoadThumbNail) = nullptr;
	}

	// Destroy the screen
	vmng_screen_dispose(_GM(slMenu));
	menu_Destroy(_GM(slMenu));

	// Unload the save/load menu sprites
	menu_UnloadSprites();
}


void CreateSaveLoadMenu(RGB8 *myPalette, bool saveMenu) {
	ItemHandlerFunction	i_handler;
	bool buttonGreyed;

	if (!_G(menuSystemInitialized)) {
		menu_Initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!menu_LoadSprites("slmenu", SL_TOTAL_SPRITES)) {
		return;
	}

	// Initialize some global vars
	_GM(firstSlotIndex) = 0;
	_GM(slotSelected) = -1;
	_GM(saveLoadThumbNail) = nullptr;
	_GM(thumbIndex) = 100;
	_GM(currMenuIsSave) = saveMenu;

	_GM(slMenu) = menu_Create(_GM(menuSprites)[SL_DIALOG_BOX], SAVE_LOAD_MENU_X, SAVE_LOAD_MENU_Y,
		MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(slMenu)) {
		return;
	}

	if (_GM(currMenuIsSave)) {
		menu_MsgAdd(_GM(slMenu), SL_TAG_SAVE_LABEL, SL_SAVE_LABEL_X, SL_SAVE_LABEL_Y, SL_SAVE_LABEL_W, SL_SAVE_LABEL_H);
		menu_ButtonAdd(_GM(slMenu), SL_TAG_SAVE, SL_SAVE_X, SL_SAVE_Y, SL_SAVE_W, SL_SAVE_H,
			cb_SaveLoad_Save, BTN_TYPE_SL_SAVE, true);
	} else {
		menu_MsgAdd(_GM(slMenu), SL_TAG_LOAD_LABEL, SL_LOAD_LABEL_X, SL_LOAD_LABEL_Y, SL_LOAD_LABEL_W, SL_LOAD_LABEL_H);
		menu_ButtonAdd(_GM(slMenu), SL_TAG_LOAD, SL_LOAD_X, SL_LOAD_Y, SL_LOAD_W, SL_LOAD_H,
			cb_SaveLoad_Load, BTN_TYPE_SL_LOAD, true);
	}

	menu_ButtonAdd(_GM(slMenu), SL_TAG_CANCEL, SL_CANCEL_X, SL_CANCEL_Y, SL_CANCEL_W, SL_CANCEL_H,
		cb_SaveLoad_Cancel, BTN_TYPE_SL_CANCEL);

	menu_VSliderAdd(_GM(slMenu), SL_TAG_VSLIDER, SL_SLIDER_X, SL_SLIDER_Y, SL_SLIDER_W, SL_SLIDER_H,
		0, cb_SaveLoad_VSlider);

	InitializeSlotTables();

	if (_GM(currMenuIsSave)) {
		buttonGreyed = false;
		i_handler = button_Handler;
	} else {
		buttonGreyed = true;
		i_handler = load_Handler;
	}

	for (int32 i = 0; i < MAX_SLOTS_SHOWN; i++) {
		menu_ButtonAdd(_GM(slMenu), 1001 + i,
			SL_SCROLL_FIELD_X, SL_SCROLL_FIELD_Y + i * SL_SCROLL_LINE_H,
			SL_SCROLL_LINE_W, SL_SCROLL_LINE_H,
			cb_SaveLoad_Slot, BTN_TYPE_SL_TEXT,
			buttonGreyed && (!_GM(slotInUse)[i]), true, _GM(slotTitles)[i], i_handler);
	}

	if (_GM(currMenuIsSave)) {
		// Create thumbnails. One in the original game format for displaying,
		// and the other in the ScummVM format for actually using in the save files
		_GM(saveLoadThumbNail) = menu_CreateThumbnail(&_GM(sizeofThumbData));
		_GM(_thumbnail).free();
		Graphics::createThumbnail(_GM(_thumbnail));

	} else {
		UpdateThumbNails(0, _GM(slMenu));
		_GM(saveLoadThumbNail) = _GM(menuSprites)[SL_EMPTY_THUMB];
	}

	menu_MsgAdd(_GM(slMenu), SL_TAG_THUMBNAIL, SL_THUMBNAIL_X, SL_THUMBNAIL_Y, SL_THUMBNAIL_W, SL_THUMBNAIL_H, false);

	if (_GM(currMenuIsSave)) {
		//<return> - if a slot has been selected, saves the game
		//<esc> - cancels and returns to the game menu
		menu_Configure(_GM(slMenu), cb_SaveLoad_Save, cb_SaveLoad_Cancel);
	} else {
		//<return> - if a slot has been selected, loads the selected game
		//<esc> - cancels and returns to the game menu
		menu_Configure(_GM(slMenu), cb_SaveLoad_Load, cb_SaveLoad_Cancel);
	}

	vmng_screen_show((void *)_GM(slMenu));
	LockMouseSprite(0);
}

void CreateGameMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(gameMenuFromMain) = false;
	CreateGameMenuMain(myPalette);
}

void CreateGameMenuFromMain(RGB8 *myPalette) {
	if (_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(gameMenuFromMain) = true;
	CreateGameMenuMain(myPalette);
}

void CreateSaveMenu(RGB8 *myPalette) {
	_GM(saveLoadFromHotkey) = false;
	CreateSaveLoadMenu(myPalette, true);
}

void CreateF2SaveMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = false;
	CreateSaveLoadMenu(myPalette, true);
}

void CreateLoadMenu(RGB8 *myPalette) {
	_GM(saveLoadFromHotkey) = false;
	CreateSaveLoadMenu(myPalette, false);
}

void CreateF3LoadMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = false;
	CreateSaveLoadMenu(myPalette, false);
}

void CreateLoadMenuFromMain(RGB8 *myPalette) {
	if (_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = true;
	CreateSaveLoadMenu(myPalette, false);
}

} // namespace GUI
} // namespace Burger
} // namespace M4
