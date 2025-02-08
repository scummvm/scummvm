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
#include "m4/gui/gui_menu.h"
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
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace GUI {

bool menu_Initialize(RGB8 *myPalette) {
	int32 i, memAvail;

	// This procedure is called before *any* menu is created - the following global var is used
	// By the menu_eventhandler() to trap events - it must be cleared.
	_GM(menuCurrItem) = nullptr;

	if (_G(menuSystemInitialized)) {
		return true;
	}

	// Set this now to prevent re-entry into the menu system
	_G(menuSystemInitialized) = true;

	// Pause the game
	game_pause(true);

	// Hide the telegram window
	// Hide_telegram_dialog(); // Ripley shit again!

	// Hide the interface
	if (INTERFACE_VISIBLE) {
		_GM(interfaceWasVisible) = true;
		interface_hide();
	} else {
		_GM(interfaceWasVisible) = false;
	}

	_GM(menuPalette) = myPalette;
	krn_fade_to_grey(_GM(menuPalette), 5, 1);

	_GM(dumpedCodes) = false;
	_GM(dumpedBackground) = false;

	// Make sure we have enough memory
	PurgeMem();
	CompactMem();
	memAvail = mem_avail();

	// Dump the screen codes if necessary
	if (memAvail < MEMORY_NEEDED) {
		adv_GetCodeMemory();
		_GM(dumpedCodes) = true;
		memAvail = mem_avail();
	}

	// Dump the background if necessary
	if (memAvail < MEMORY_NEEDED) {
		adv_GetBackgroundMemory();
		_GM(dumpedBackground) = true;
		memAvail = mem_avail();
	}

	// If we still don't have enough memory, we are hosed
	if (memAvail < MEMORY_NEEDED) {
		return false;
	}

	// Load in the font
	_GM(menuFont) = gr_font_load("FONTMENU.FNT");

	// Alloc space for the save/load tables
	if ((_GM(slotTitles) = (char **)mem_alloc(sizeof(char *) * MAX_SLOTS, "slot desc array")) == nullptr) {
		return false;
	}
	for (i = 0; i < MAX_SLOTS; i++) {
		if ((_GM(slotTitles)[i] = (char *)mem_alloc(80, "slot title")) == nullptr) {
			return false;
		}
	}
	if ((_GM(slotInUse) = (bool *)mem_alloc(sizeof(bool) * MAX_SLOTS, "slotUnUse array")) == nullptr) {
		return false;
	}

	// Allocate space for the thumnail sprites
	if ((_GM(thumbNails) = (Sprite **)mem_alloc(sizeof(Sprite *) * MAX_SLOTS, "thumbNail array")) == nullptr) {
		return false;
	}
	for (i = 0; i < MAX_SLOTS; i++) {
		if ((_GM(thumbNails)[i] = (Sprite *)mem_alloc(sizeof(Sprite), "thumbNail")) == nullptr) {
			return false;
		}
		_GM(thumbNails)[i]->sourceHandle = nullptr;
	}

	return true;
}

void menu_Shutdown(bool fadeToColor) {
	int32 i;

	// Verify that we need to shutdown
	if (!_G(menuSystemInitialized)) {
		return;
	}

	_GM(menuCurrItem) = nullptr;

	// Turf the font
	gr_font_dealloc(_GM(menuFont));
	_GM(menuFont) = nullptr;

	// Turf the slot arrays
	for (i = 0; i < MAX_SLOTS; i++) {
		if (_GM(slotTitles)[i]) {
			mem_free((void *)_GM(slotTitles)[i]);
		}
	}
	mem_free((void *)_GM(slotTitles));
	mem_free((void *)_GM(slotInUse));

	// Turf the thumbnail sprites
	for (i = 0; i < MAX_SLOTS; i++) {
		if (_GM(thumbNails)[i]) {
			mem_free((void *)_GM(thumbNails)[i]);
		}
	}
	mem_free((void *)_GM(thumbNails));

	// Restore the background and codes if necessary
	PurgeMem();
	CompactMem();
	if (_GM(dumpedBackground)) {
		if (!adv_restoreBackground()) {
			error_show(FL, 0, "unable to restore background");
		}
	}
	if (_GM(dumpedCodes)) {
		if (!adv_restoreCodes()) {
			error_show(FL, 0, "unable to restore screen codes");
		}
	}

	// Fade the screen - up to color if the game resumes, down to black if a new game was loaded
	if (fadeToColor) {
		krn_fade_from_grey(_GM(menuPalette), 5, 1, TO_COLOR);
	} else {
		krn_fade_from_grey(_GM(menuPalette), 5, 1, TO_BLACK);
	}

	// See if the interface needs to be restored
	if (_GM(interfaceWasVisible)) {
		interface_show();
	}

	// Allow the mouse to change from a clock
	UnlockMouseSprite();

	// Unpause the game
	game_pause(false);

	// Menu is now uninitialized
	_G(menuSystemInitialized) = false;
}

bool menu_EventHandler(void *theMenu, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen);

GrBuff *menu_CopyBackground(guiMenu *myMenu, int32 x, int32 y, int32 w, int32 h) {
	GrBuff *copyOfBackground;
	Buffer *srcBuff, *destBuff;

	// Verify params
	if ((!myMenu) || (!myMenu->menuBuffer)) {
		return nullptr;
	}

	// Create a new grbuff struct
	copyOfBackground = new GrBuff(w, h);
	if (!copyOfBackground) {
		return nullptr;
	}

	// Get the source and destination buffers
	srcBuff = myMenu->menuBuffer->get_buffer();
	destBuff = copyOfBackground->get_buffer();
	if ((!srcBuff) || (!destBuff)) {
		delete copyOfBackground;

		return nullptr;
	}

	// Copy the rect
	gr_buffer_rect_copy_2(srcBuff, destBuff, x, y, 0, 0, w, h);

	// Now release the buffers
	myMenu->menuBuffer->release();
	copyOfBackground->release();

	return copyOfBackground;
}

void menu_Show(void *s, void *r, void *b, int32 destX, int32 destY) {
	ScreenContext *myScreen = (ScreenContext *)s;
	RectList *myRectList = (RectList *)r;
	Buffer *destBuffer = (Buffer *)b;
	guiMenu *myMenu;
	GrBuff *myMenuBuffer;
	Buffer *myBuffer;
	RectList *myRect;

	// Parameter verification
	if (!myScreen) {
		return;
	}
	myMenu = (guiMenu *)(myScreen->scrnContent);
	if (!myMenu) {
		return;
	}
	myMenuBuffer = myMenu->menuBuffer;
	if (!myMenuBuffer) {
		return;
	}
	myBuffer = myMenuBuffer->get_buffer();
	if (!myBuffer) {
		return;
	}

	// If no destBuffer, then draw directly to video
	if (!destBuffer) {
		myRect = myRectList;
		while (myRect) {
			vmng_refresh_video(myRect->x1, myRect->y1, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				myRect->x2 - myScreen->x1, myRect->y2 - myScreen->y1, myBuffer);
			myRect = myRect->next;
		}
	}

	// Else draw to the dest buffer
	else {
		myRect = myRectList;
		while (myRect) {
			gr_buffer_rect_copy_2(myBuffer, destBuffer, myRect->x1 - myScreen->x1, myRect->y1 - myScreen->y1,
				destX, destY, myRect->x2 - myRect->x1 + 1, myRect->y2 - myRect->y1 + 1);
			myRect = myRect->next;
		}
	}

	// Release myBuffer
	myMenuBuffer->release();
}

guiMenu *menu_Create(Sprite *backgroundSprite, int32 x1, int32 y1, int32 scrnFlags) {
	guiMenu *newMenu;
	Buffer *tempBuff, drawSpriteBuff;
	DrawRequest			spriteDrawReq;

	// Verify params
	if (!backgroundSprite) {
		return nullptr;
	}

	if ((newMenu = (guiMenu *)mem_alloc(sizeof(guiMenu), "gui menu")) == nullptr) {
		return nullptr;
	}
	newMenu->menuBuffer = new GrBuff(backgroundSprite->w, backgroundSprite->h);
	newMenu->itemList = nullptr;
	newMenu->cb_return = nullptr;
	newMenu->cb_esc = nullptr;
	newMenu->menuEventHandler = menu_EventHandler;

	// Draw the background in to the menuBuffer
	tempBuff = newMenu->menuBuffer->get_buffer();

	// Copy background into menu-buffer because it is not rectangular (matte ink effect)
	Buffer *matte = _G(gameDrawBuff)->get_buffer(); // get a pointer to the game background buffer
	if (tempBuff->h > (_G(gameDrawBuff)->h - y1)) {  // if temp buffer is going to hang off the bottom of the game buffer
		gr_buffer_rect_copy_2(matte, tempBuff, x1, y1, 0, 0, tempBuff->w, _G(gameDrawBuff)->h - y1);	// copy the differnce
	} else {
		gr_buffer_rect_copy_2(matte, tempBuff, x1, y1, 0, 0, tempBuff->w, tempBuff->h);			  // copy all of it
	}
	_G(gameDrawBuff)->release();	 // Release the buffer so it can be moved if nesessary

	// draw the sprite
	if (backgroundSprite->sourceHandle) {
		HLock(backgroundSprite->sourceHandle);
		backgroundSprite->data = (uint8 *)((intptr) * (backgroundSprite->sourceHandle) + backgroundSprite->sourceOffset);

		drawSpriteBuff.w = backgroundSprite->w;
		drawSpriteBuff.stride = backgroundSprite->w;
		drawSpriteBuff.h = backgroundSprite->h;
		drawSpriteBuff.encoding = (backgroundSprite->encoding) & (uint8)0x7f;
		drawSpriteBuff.data = backgroundSprite->data;

		spriteDrawReq.Src = &drawSpriteBuff;
		spriteDrawReq.Dest = tempBuff;
		spriteDrawReq.x = 0;
		spriteDrawReq.y = 0;
		spriteDrawReq.scaleX = 100;
		spriteDrawReq.scaleY = 100;
		spriteDrawReq.srcDepth = 0;
		spriteDrawReq.depthCode = nullptr;
		spriteDrawReq.Pal = nullptr;
		spriteDrawReq.ICT = nullptr;

		gr_sprite_draw(&spriteDrawReq);

		// Unlock the handle
		HUnLock(backgroundSprite->sourceHandle);
	}
	// Release the tempBuffer
	newMenu->menuBuffer->release();

	if (!vmng_screen_create(x1, y1, x1 + backgroundSprite->w - 1, y1 + backgroundSprite->h - 1, 69, scrnFlags, (void *)newMenu,
		(RefreshFunc)menu_Show, menu_EventHandler)) {
		return nullptr;
	}

	return newMenu;
}

void menu_Destroy(guiMenu *myMenu) {
	menuItem *myItem;

	// Verify params
	if (!myMenu) {
		return;
	}

	// Destroy the items
	myItem = myMenu->itemList;
	while (myItem) {
		myMenu->itemList = myItem->next;
		(myItem->destroy)((void *)myItem);
		myItem = myMenu->itemList;
	}

	// Destroy the buffer
	delete myMenu->menuBuffer;

	// Destroy the menu
	mem_free((void *)myMenu);
}

void menu_Configure(guiMenu *myMenu, CALLBACK cb_return, CALLBACK cb_esc) {
	if (!myMenu) {
		return;
	}
	myMenu->cb_return = cb_return;
	myMenu->cb_esc = cb_esc;
}

bool menu_EventHandler(void *theMenu, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen) {
	ScreenContext *myScreen;
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItem *myItem;
	int32 status, menuX, menuY;
	bool handled;
	static int32 movingX;
	static int32 movingY;
	static bool movingScreen = false;

	// Initialize the vars
	handled = false;
	if (currScreen)
		*currScreen = false;

	// Make sure the screen exists and is active
	myScreen = vmng_screen_find(theMenu, &status);
	if ((!myScreen) || (status != SCRN_ACTIVE)) {
		return false;
	}

	// If the escape key was pressed, it takes priority over items handling the event
	if ((eventType == EVENT_KEY) && (parm1 == KEY_ESCAPE)) {
		if (myMenu->cb_esc) {
			_GM(menuCurrItem) = nullptr;
			(myMenu->cb_esc)(nullptr, theMenu);
			return true;
		}
	}

	// If the return key was pressed, it takes priority over items handling the event
	if ((eventType == EVENT_KEY) && (parm1 == KEY_RETURN)) {
		if (myMenu->cb_return) {
			_GM(menuCurrItem) = nullptr;
			(myMenu->cb_return)(nullptr, theMenu);
			return true;
		}
	}

	// Convert the global coordinates to coords relative to the menu
	menuX = parm2 - myScreen->x1;
	menuY = parm3 - myScreen->y1;

	// If we are currently handling the events for an item, continue until that item releases control
	if (_GM(menuCurrItem)) {
		handled = (_GM(menuCurrItem)->itemEventHandler)(_GM(menuCurrItem), eventType, parm1, menuX, menuY, (void **)&_GM(menuCurrItem));
		if (_GM(menuCurrItem)) {
			*currScreen = true;
		}
		if (handled) {
			return true;
		}
	}

	// See what kind of event we have
	if (eventType == EVENT_MOUSE) {
		// Scroll through the list of items until we find one that the cursor is on top of
		myItem = myMenu->itemList;
		while (myItem && (!((menuX >= myItem->x1) && (menuX <= myItem->x2) &&
			(menuY >= myItem->y1) && (menuY <= myItem->y2)))) {
			myItem = myItem->next;
		}

		// If an item is found, then if it has an event handler, handle the event and return true
		if (myItem) {
			if (myItem->itemEventHandler) {
				(myItem->itemEventHandler)(myItem, eventType, parm1, menuX, menuY, (void **)&_GM(menuCurrItem));
				if (_GM(menuCurrItem)) {
					*currScreen = true;
				}
				return true;
			}
		}
	}

	else if (eventType == EVENT_KEY) {
		// Else the event is a key event - loop through - see if anyone grabs it
		myItem = myMenu->itemList;
		while (myItem && (!handled)) {
			if (myItem->itemEventHandler) {
				handled = (myItem->itemEventHandler)(myItem, eventType, parm1, -1, -1, nullptr);
			}
			myItem = myItem->next;
		}
		return handled;
	}

	// Otherwise the event is not handled by any of the menu items.  Let the menu screen itself handle the event

	switch (parm1) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (!(myScreen->scrnFlags & SF_IMMOVABLE)) {
			*currScreen = true;
			movingScreen = true;
			movingX = parm2;
			movingY = parm3;
		}
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (movingScreen) {
			MoveScreenDelta(myScreen, parm2 - movingX, parm3 - movingY);
			movingX = parm2;
			movingY = parm3;
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		*currScreen = false;
		movingScreen = false;
		break;

	case _ME_move:
	case _ME_L_hold:
	case _ME_doubleclick_hold:
		break;
	}
	return true;
}

menuItem *menu_GetItem(int32 tag, guiMenu *myMenu) {
	menuItem *myItem;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	myItem = myMenu->itemList;
	while (myItem && (myItem->tag != tag)) {
		myItem = myItem->next;
	}

	return myItem;
}


void menu_ItemDelete(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	Buffer *myBuff, *backgroundBuff;

	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem) {
		myItem = menu_GetItem(tag, myMenu);
	}
	if (!myItem) {
		return;
	}

	// Remove myItem from the item list
	if (myItem->next) {
		myItem->next->prev = myItem->prev;
	}
	if (myItem->prev) {
		myItem->prev->next = myItem->next;
	} else {
		myMenu->itemList = myItem->next;
	}

	// If the item is marked transparent, we can remove it from the menu
	if (myItem->transparent) {
		if (!myItem->background) {
			return;
		}
		backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}

		// Get the menu buffer and draw the sprite to it
		myBuff = myMenu->menuBuffer->get_buffer();
		if (!myBuff) {
			return;
		}

		// Copy the clean piece of the background to the menu buffer
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, myItem->x1, myItem->y1, backgroundBuff->w, backgroundBuff->h);

		// Release both buffers
		myMenu->menuBuffer->release();
		myItem->background->release();
	}

	// Destroy the item;
	if (myItem->destroy) {
		myItem->destroy((void *)myItem);
	}
}

void menu_ItemRefresh(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem) {
		myItem = menu_GetItem(tag, myMenu);
	}
	if (!myItem) {
		return;
	}

	// Draw myItem
	(myItem->redraw)(myItem, (void *)myItem->myMenu, myItem->x1, myItem->y1, 0, 0);

	// Update the video
	myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
			myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
	}
}


//-----------------------------    GAME MENU FUNCTIONS    ---------------------------------//

bool menu_LoadSprites(const char *series, int32 numSprites) {
	int32 i;

	// Load in the game menu series
	if (LoadSpriteSeries(series, &_GM(menuSeriesHandle), &_GM(menuSeriesOffset),
		&_GM(menuSeriesPalOffset), _GM(menuPalette)) <= 0) {
		return false;
	}
	_GM(menuSeriesResource) = mem_strdup(series);

	// Update the palette for the menu
	if (IS_BURGER)
		gr_pal_set_range(_GM(menuPalette), 59, 197);
	else
		gr_pal_set_range(_GM(menuPalette), 128, 128);

	_GM(spriteCount) = numSprites;

	// Create the _GM(menuSprites) array
	if ((_GM(menuSprites) = (Sprite **)mem_alloc(sizeof(Sprite *) * _GM(spriteCount), "sprites array")) == nullptr) {
		return false;
	}

	// Create the menu sprites
	for (i = 0; i < _GM(spriteCount); i++) {
		if ((_GM(menuSprites)[i] = CreateSprite(_GM(menuSeriesHandle), _GM(menuSeriesOffset), i, nullptr, nullptr)) == nullptr) {
			return false;
		}
	}

	return true;
}

void menu_UnloadSprites() {
	int32 i;

	if (!_GM(menuSeriesResource)) {
		return;
	}

	// Unload the sprites from memory
	rtoss(_GM(menuSeriesResource));
	mem_free(_GM(menuSeriesResource));
	_GM(menuSeriesResource) = nullptr;
	_GM(menuSeriesHandle) = nullptr;
	_GM(menuSeriesOffset) = -1;
	_GM(menuSeriesPalOffset) = -1;

	// Turf the sprites
	for (i = 0; i < _GM(spriteCount); i++) {
		mem_free((void *)_GM(menuSprites)[i]);
	}

	// Turf the sprites array
	mem_free((void *)_GM(menuSprites));
	_GM(menuSprites) = nullptr;
	_GM(spriteCount) = 0;
}

} // namespace GUI
} // namespace M4
