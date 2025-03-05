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
#include "m4/gui/gui_menu_items.h"
#include "m4/adv_r/other.h"
#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_player.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/game_menu.h"
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
#include "m4/platform/timer.h"

namespace M4 {
namespace GUI {

void gui_DrawSprite(Sprite *mySprite, Buffer *myBuff, int32 x, int32 y) {
	DrawRequest spriteDrawReq;
	Buffer drawSpriteBuff;

	if ((!mySprite) || (!myBuff)) {
		return;
	}

	if (mySprite->sourceHandle) {
		HLock(mySprite->sourceHandle);
		mySprite->data = (uint8 *)((intptr) * (mySprite->sourceHandle) + mySprite->sourceOffset);

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

//-----------------------------  MENU DIALOG FUNCTIONS    ---------------------------------//

bool guiMenu::initialize(RGB8 *myPalette) {
	int32 i;

	// This procedure is called before *any* menu is created - the following global var is used
	// By the guiMenu::eventHandler() to trap events - it must be cleared.
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

	// Allocate space for the thumbnail sprites
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

void guiMenu::shutdown(bool fadeToColor) {
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

GrBuff *guiMenu::copyBackground(guiMenu *myMenu, int32 x, int32 y, int32 w, int32 h) {
	// Verify params
	if ((!myMenu) || (!myMenu->menuBuffer)) {
		return nullptr;
	}

	// Create a new grbuff struct
	GrBuff *copyOfBackground = new GrBuff(w, h);
	if (!copyOfBackground) {
		return nullptr;
	}

	// Get the source and destination buffers
	Buffer *srcBuff = myMenu->menuBuffer->get_buffer();
	Buffer *destBuff = copyOfBackground->get_buffer();
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

void guiMenu::show(void *s, void *r, void *b, int32 destX, int32 destY) {
	ScreenContext *myScreen = (ScreenContext *)s;
	RectList *myRectList = (RectList *)r;
	Buffer *destBuffer = (Buffer *)b;
	RectList *myRect;

	// Parameter verification
	if (!myScreen) {
		return;
	}
	guiMenu *myMenu = (guiMenu *)(myScreen->scrnContent);
	if (!myMenu) {
		return;
	}
	GrBuff *myMenuBuffer = myMenu->menuBuffer;
	if (!myMenuBuffer) {
		return;
	}
	Buffer *myBuffer = myMenuBuffer->get_buffer();
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

guiMenu *guiMenu::create(Sprite *backgroundSprite, int32 x1, int32 y1, int32 scrnFlags) {
	Buffer drawSpriteBuff;
	DrawRequest spriteDrawReq;

	// Verify params
	if (!backgroundSprite) {
		return nullptr;
	}

	guiMenu *newMenu = new guiMenu();

	newMenu->menuBuffer = new GrBuff(backgroundSprite->w, backgroundSprite->h);
	newMenu->itemList = nullptr;
	newMenu->cb_return = nullptr;
	newMenu->cb_esc = nullptr;
	newMenu->menuEventHandler = (EventHandler)guiMenu::eventHandler;

	// Draw the background in to the menuBuffer
	Buffer *tempBuff = newMenu->menuBuffer->get_buffer();

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
		(RefreshFunc)guiMenu::show, (EventHandler)guiMenu::eventHandler)) {
		return nullptr;
	}

	return newMenu;
}

void guiMenu::destroy(guiMenu *myMenu) {
	// Verify params
	if (!myMenu) {
		return;
	}

	// Destroy the items
	menuItem *myItem = myMenu->itemList;
	while (myItem) {
		myMenu->itemList = myItem->next;
		(myItem->destroy)(myItem);
		myItem = myMenu->itemList;
	}

	// Destroy the buffer
	delete myMenu->menuBuffer;

	// Destroy the menu
	delete myMenu;
}

void guiMenu::configure(guiMenu *myMenu, CALLBACK cb_return, CALLBACK cb_esc) {
	if (!myMenu) {
		return;
	}
	myMenu->cb_return = cb_return;
	myMenu->cb_esc = cb_esc;
}

bool guiMenu::eventHandler(guiMenu *theMenu, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen) {
	guiMenu *myMenu = (guiMenu *)theMenu;
	menuItem *myItem;
	int32 status;
	static int32 movingX;
	static int32 movingY;
	static bool movingScreen = false;

	// Initialize the vars
	bool handled = false;
	if (currScreen)
		*currScreen = false;

	// Make sure the screen exists and is active
	ScreenContext *myScreen = vmng_screen_find(theMenu, &status);
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
	const int32 menuX = parm2 - myScreen->x1;
	const int32 menuY = parm3 - myScreen->y1;

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
	default:
		break;
	}
	return true;
}

menuItem *guiMenu::getItem(int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	menuItem *myItem = myMenu->itemList;
	while (myItem && (myItem->tag != tag)) {
		myItem = myItem->next;
	}

	return myItem;
}

void guiMenu::itemDelete(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem)
		myItem = guiMenu::getItem(tag, myMenu);
	if (!myItem)
		return;

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
		Buffer *backgroundBuff = myItem->background->get_buffer();
		if (!backgroundBuff) {
			return;
		}

		// Get the menu buffer and draw the sprite to it
		Buffer *myBuff = myMenu->menuBuffer->get_buffer();
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
		myItem->destroy(myItem);
	}
}

void guiMenu::itemRefresh(menuItem *myItem, int32 tag, guiMenu *myMenu) {
	int32 status;

	// Verify params
	if (!myMenu) {
		return;
	}

	if (!myItem) {
		myItem = guiMenu::getItem(tag, myMenu);
	}
	if (!myItem) {
		return;
	}

	// Draw myItem
	(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);

	// Update the video
	ScreenContext *myScreen = vmng_screen_find((void *)myItem->myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
			myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
	}
}

bool guiMenu::loadSprites(const char *series, int32 numSprites) {
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
	for (int32 i = 0; i < _GM(spriteCount); i++) {
		if ((_GM(menuSprites)[i] = CreateSprite(_GM(menuSeriesHandle), _GM(menuSeriesOffset), i, nullptr, nullptr)) == nullptr) {
			return false;
		}
	}

	return true;
}

void guiMenu::unloadSprites() {
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
	for (int32 i = 0; i < _GM(spriteCount); i++) {
		mem_free((void *)_GM(menuSprites)[i]);
	}

	// Turf the sprites array
	mem_free((void *)_GM(menuSprites));
	_GM(menuSprites) = nullptr;
	_GM(spriteCount) = 0;
}

//-----------------------------  GENERAL ITEM FUNCTIONS    ---------------------------------//

void menuItem::destroyItem(menuItem *theItem) {
	// Verify params
	if (!theItem) {
		return;
	}

	delete theItem->background;
	delete theItem;
}

bool menuItem::cursorInsideItem(menuItem *myItem, int32 cursorX, int32 cursorY) {
	if ((cursorX >= myItem->x1) && (cursorX <= myItem->x2) && (cursorY >= myItem->y1) && (cursorY <= myItem->y2)) {
		return true;
	}

	return false;
}

//-----------------------------  BUTTON FUNCTIONS    ---------------------------------//

void menuItemButton::drawButton(menuItemButton *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;
	char tempStr[32];

	// Verify params
	if (!myItem || !myMenu) {
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

	// Select the sprite
	switch (myItem->buttonType) {
	case BTN_TYPE_GM_GENERIC:
		switch (myItem->itemFlags) {
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

	case BTN_TYPE_SL_TEXT:
		switch (myItem->itemFlags) {
		case BTN_STATE_OVER:
			if (IS_RIDDLE)
				gr_font_set_color(96);
			else
				font_set_colors(TEXT_COLOR_OVER_SHADOW, TEXT_COLOR_OVER_FOREGROUND, TEXT_COLOR_OVER_HILITE);

			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LINE_OVER];
			break;

		case BTN_STATE_PRESS:
			if (IS_RIDDLE)
				gr_font_set_color(96);
			else
				font_set_colors(TEXT_COLOR_PRESS_SHADOW, TEXT_COLOR_PRESS_FOREGROUND, TEXT_COLOR_PRESS_HILITE);

			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LINE_PRESS];
			break;

		case BTN_STATE_GREY:
			if (IS_RIDDLE)
				gr_font_set_color(202);
			else
				font_set_colors(TEXT_COLOR_GREY_SHADOW, TEXT_COLOR_GREY_FOREGROUND, TEXT_COLOR_GREY_HILITE);

			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LINE_NORM];
			break;

		default:
		case BTN_STATE_NORM:
			if (IS_RIDDLE)
				gr_font_set_color(96);
			else
				font_set_colors(TEXT_COLOR_NORM_SHADOW, TEXT_COLOR_NORM_FOREGROUND, TEXT_COLOR_NORM_HILITE);

			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LINE_NORM];
			break;
		}
		break;

	/** ORION BURGER BUTTON TYPES **/
	case BTN_TYPE_SL_SAVE:
		switch (myItem->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SAVE_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SAVE_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SAVE_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SAVE_BTN_GREY];
			break;
		}
		break;

	case BTN_TYPE_SL_LOAD:
		switch (myItem->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LOAD_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LOAD_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LOAD_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LOAD_BTN_GREY];
			break;
		}
		break;

	case BTN_TYPE_SL_CANCEL:
		switch (myItem->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_CANCEL_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_CANCEL_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_CANCEL_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_CANCEL_BTN_NORM];
			break;
		}
		break;

	case BTN_TYPE_OM_DONE:
		switch (myItem->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_DONE_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_DONE_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_DONE_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_DONE_BTN_GREY];
			break;
		}
		break;

	case BTN_TYPE_OM_CANCEL:
		switch (myItem->itemFlags) {
		case BTN_STATE_NORM:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_CANCEL_BTN_NORM];
			break;
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_CANCEL_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_CANCEL_BTN_PRESS];
			break;
		default:
		case BTN_STATE_GREY:
			mySprite = _GM(menuSprites)[(int)Burger::GUI::OM_CANCEL_BTN_NORM];
			break;
		}
		break;

	/** RIDDLE BUTTON TYPES **/
	case BTN_TYPE_OM_SCROLLING_ON:
		switch (myItem->itemFlags) {
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[(int)Riddle::GUI::OM_SCROLLING_ON_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[(int)Riddle::GUI::OM_SCROLLING_ON_BTN_PRESS];
			break;
		case BTN_STATE_NORM:
		default:
			mySprite = _GM(menuSprites)[(int)Riddle::GUI::OM_SCROLLING_ON_BTN_NORM];
			break;
		}
		break;

	case BTN_TYPE_OM_SCROLLING_OFF:
		switch (myItem->itemFlags) {
		case BTN_STATE_OVER:
			mySprite = _GM(menuSprites)[(int)Riddle::GUI::OM_SCROLLING_OFF_BTN_OVER];
			break;
		case BTN_STATE_PRESS:
			mySprite = _GM(menuSprites)[(int)Riddle::GUI::OM_SCROLLING_OFF_BTN_PRESS];
			break;
		case BTN_STATE_NORM:
		default:
			mySprite = _GM(menuSprites)[(int)Riddle::GUI::OM_SCROLLING_OFF_BTN_NORM];
			break;
		}
		break;

	default:
		break;
	}

	// Get the menu buffer
	Buffer *myBuff = myMenu->menuBuffer->get_buffer();
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
	if ((myItem->buttonType == BTN_TYPE_SL_TEXT) && (myItem->prompt)) {
		// Write in the special tag
		Common::sprintf_s(tempStr, 32, "%02d", myItem->tag - 1000 + _GM(firstSlotIndex));

		gr_font_set(_GM(menuFont));
		gr_font_write(myBuff, tempStr, x + 4, y + 1, 0, -1);
		gr_font_write(myBuff, myItem->prompt, x + 26, y + 1, 0, -1);
	}

	// Release the menu buffer
	myMenu->menuBuffer->release();
}

bool menuItemButton::handler(menuItemButton *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	ScreenContext *myScreen;
	int32 status;

	// Verify params
	if (!myItem) {
		return false;
	}

	if (!(eventType == EVENT_MOUSE)) {
		return false;
	}

	if (myItem->itemFlags == BTN_STATE_GREY) {
		return false;
	}

	bool redrawItem = false;
	bool execCallback = false;
	bool handled = true;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			myItem->itemFlags = BTN_STATE_PRESS;
			*currItem = myItem;
			redrawItem = true;
		} else {
			*currItem = nullptr;
			if (myItem->itemFlags != BTN_STATE_NORM) {
				myItem->itemFlags = BTN_STATE_NORM;
				redrawItem = true;
			}
		}
		break;

	case _ME_L_drag:
	case _ME_doubleclick_drag:
		if (!*currItem) {
			return true;
		}
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			if (myItem->itemFlags != BTN_STATE_PRESS) {
				myItem->itemFlags = BTN_STATE_PRESS;
				redrawItem = true;
			}
		} else {
			if (myItem->itemFlags != BTN_STATE_OVER) {
				myItem->itemFlags = BTN_STATE_OVER;
				redrawItem = true;
			}
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			if (*currItem) {
				execCallback = true;

				if (myItem->buttonType == BTN_TYPE_OM_SCROLLING_ON)
					myItem->buttonType = BTN_TYPE_OM_SCROLLING_OFF;
				else if (myItem->buttonType == BTN_TYPE_OM_SCROLLING_OFF)
					myItem->buttonType = BTN_TYPE_OM_SCROLLING_ON;
			} else {
				*currItem = myItem;
			}
			myItem->itemFlags = BTN_STATE_OVER;
			redrawItem = true;
		} else {
			*currItem = nullptr;
			myItem->itemFlags = BTN_STATE_NORM;
			redrawItem = true;
			handled = false;
		}
		break;

	case _ME_move:
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			*currItem = myItem;
			if (myItem->itemFlags != BTN_STATE_OVER) {
				myItem->itemFlags = BTN_STATE_OVER;
				redrawItem = true;
			}
		} else {
			*currItem = nullptr;
			if (myItem->itemFlags != BTN_STATE_NORM) {
				myItem->itemFlags = BTN_STATE_NORM;
				redrawItem = true;
				handled = false;
			}
		}
		break;

	case _ME_L_hold:
	case _ME_doubleclick_hold:
	default:
		break;
	}

	// See if we need to redraw the button
	if (redrawItem) {
		(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		if (IS_RIDDLE)
			digi_play("950_s51", 2, 255, -1, 950);

		guiMenu *currMenu = myItem->myMenu;
		int32 currTag = myItem->tag;
		_GM(buttonClosesDialog) = false;

		(myItem->callback)(myItem, myItem->myMenu);

		status = 0;
		myScreen = _GM(buttonClosesDialog) ? nullptr : vmng_screen_find(myItem->myMenu, &status);

		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		} else {
			menuItem *tempItem = guiMenu::getItem(currTag, currMenu);
			if (!tempItem) {
				*currItem = nullptr;
			}
		}
	}

	return handled;
}

menuItemButton *menuItemButton::add(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, CALLBACK callback, int32 buttonType,
	bool ghosted, bool transparent, const char *prompt, ItemHandlerFunction i_handler) {
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	menuItemButton *newItem = new menuItemButton();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
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
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
	}

	if (ghosted) {
		newItem->itemFlags = BTN_STATE_GREY;
	} else {
		newItem->itemFlags = BTN_STATE_NORM;
	}
	newItem->buttonType = buttonType;

	// Note: prompt is not duplicated, therefore, make sure the name is stored in non-volatile memory
	newItem->prompt = prompt;
	newItem->specialTag = tag - 1000;

	newItem->redraw = (DrawFunction)menuItemButton::drawButton;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = i_handler;

	// Draw the button in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	ScreenContext *myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}

void menuItemButton::disableButton(menuItemButton *myItem, int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu)
		return;

	if (!myItem)
		myItem = (menuItemButton *)guiMenu::getItem(tag, myMenu);
	if (!myItem)
		return;

	myItem->itemFlags = BTN_STATE_GREY;
}

void menuItemButton::enableButton(menuItemButton *myItem, int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu)
		return;

	if (!myItem)
		myItem = (menuItemButton *)guiMenu::getItem(tag, myMenu);
	if (!myItem)
		return;

	myItem->itemFlags = BTN_STATE_NORM;
}


//-----------------------------  MSG FUNCTIONS    ---------------------------------//

menuItemMsg *menuItemMsg::msgAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, bool transparent) {
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	menuItemMsg *newItem = new menuItemMsg();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
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
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
	}

	newItem->redraw = (DrawFunction)menuItemMsg::drawMsg;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = nullptr;

	// Draw the message in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	ScreenContext *myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}

void menuItemMsg::drawMsg(menuItemMsg *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite = nullptr;

	// Verify params
	if (!myItem || !myMenu) {
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

	// Select the sprite
	switch (myItem->tag) {
	case SL_TAG_SAVE_TITLE_LABEL:
		mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SAVE_TITLE];
		break;
	case SL_TAG_LOAD_TITLE_LABEL:
		mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LOAD_TITLE];
		break;
	case SL_TAG_SAVE_LABEL:
		mySprite = _GM(menuSprites)[myItem->itemFlags ?
			SaveLoadMenuBase::SL_SAVE_LABEL_GREY : SaveLoadMenuBase::SL_SAVE_LABEL_7];
		break;
	case SL_TAG_LOAD_LABEL:
		mySprite = _GM(menuSprites)[myItem->itemFlags ?
			SaveLoadMenuBase::SL_LOAD_LABEL_GREY : SaveLoadMenuBase::SL_LOAD_LABEL_9];
		break;
	case SL_TAG_THUMBNAIL:
		mySprite = _GM(saveLoadThumbNail);
		break;
	}

	// Get the menu buffer and draw the sprite to it
	Buffer *myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	} else if (myItem->tag == SL_TAG_THUMBNAIL && mySprite->w == 160) {
		// Hack for handling smaller ScummVM thumbnails
		for (int yp = y; yp < (y + SaveLoadMenuBase::SL_THUMBNAIL_H); ++yp) {
			byte *line = myBuff->data + myBuff->stride * yp + x;
			Common::fill(line, line + SaveLoadMenuBase::SL_THUMBNAIL_W, 0);
		}

		x += 25;
		y += 25;
	}

	// Draw the sprite in
	gui_DrawSprite(mySprite, myBuff, x, y);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}


//-------------------------------    HSLIDER MENU ITEM    ---------------------------------//

void menuItemHSlider::drawHSlider(menuItemHSlider *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite;

	// Verify params
	if (!myItem || !myMenu)
		return;

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
	Buffer *myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Get the slider info and select the thumb sprite
	switch (myItem->itemFlags) {
	case H_THUMB_OVER:
		mySprite = _GM(menuSprites)[IS_RIDDLE ? (int)Riddle::GUI::OM_SLIDER_BTN_OVER :
			(int)Burger::GUI::OM_SLIDER_BTN_OVER];
		break;
	case H_THUMB_PRESS:
		mySprite = _GM(menuSprites)[IS_RIDDLE ? (int)Riddle::GUI::OM_SLIDER_BTN_PRESS :
			(int)Burger::GUI::OM_SLIDER_BTN_PRESS];
		break;
	default:
	case H_THUMB_NORM:
		mySprite = _GM(menuSprites)[IS_RIDDLE ? (int)Riddle::GUI::OM_SLIDER_BTN_NORM :
			(int)Burger::GUI::OM_SLIDER_BTN_NORM];
		break;
	}

	// Fill in everything left of the thumb with a hilite color
	if (myItem->thumbX > 2) {
		if (IS_RIDDLE) {
			gr_color_set(120);
			gr_buffer_rect_fill(myBuff, myItem->x1 + 2, myItem->y1 + 3,
				myItem->thumbX - 2, myItem->thumbH - 6);
		} else {
			gr_color_set(129);
			gr_buffer_rect_fill(myBuff, myItem->x1 + 3, myItem->y1 + 9,
				myItem->thumbX, myItem->thumbH - 18);
		}
	}

	// Draw in the thumb
	gui_DrawSprite(mySprite, myBuff, myItem->x1 + myItem->thumbX, myItem->y1);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}

bool menuItemHSlider::handler(menuItemHSlider *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	ScreenContext *myScreen;
	int32 status;
	int32 deltaSlide;
	static bool movingFlag;
	static int32 movingX;

	// Verify params
	if (!myItem)
		return false;

	if (!(eventType == EVENT_MOUSE)) {
		return false;
	}

	bool redrawItem = false;
	bool handled = true;
	bool execCallback = false;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menuItem::cursorInsideItem(myItem, x, y) && (x - myItem->x1 >= myItem->thumbX) &&
			(x - myItem->x1 <= myItem->thumbX + myItem->thumbW - 1)) {
			myItem->itemFlags = H_THUMB_PRESS;
			movingFlag = true;
			movingX = x;
			*currItem = myItem;
			redrawItem = true;
		} else {
			*currItem = nullptr;
			myItem->itemFlags = 0;
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
				deltaSlide = imath_min(myItem->thumbX, movingX - x);
				if (deltaSlide > 0) {
					myItem->thumbX -= deltaSlide;
					redrawItem = true;
					myItem->percent = myItem->thumbX * 100 / myItem->maxThumbX;
					execCallback = true;
				}
			} else if (x > movingX) {
				deltaSlide = imath_min(myItem->maxThumbX - myItem->thumbX, x - movingX);
				if (deltaSlide > 0) {
					myItem->thumbX += deltaSlide;
					redrawItem = true;
					myItem->percent = myItem->thumbX * 100 / myItem->maxThumbX;
					execCallback = true;
				}
			}
			movingX = x;
			if (movingX < (myItem->thumbX + myItem->x1)) {
				movingX = myItem->thumbX + myItem->x1;
			} else if (movingX > (myItem->thumbX + myItem->thumbW - 1 + myItem->x1)) {
				movingX = myItem->thumbX + myItem->thumbW - 1 + myItem->x1;
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
		if (menuItem::cursorInsideItem(myItem, x, y) && (x - myItem->x1 >= myItem->thumbX) &&
			(x - myItem->x1 <= myItem->thumbX + myItem->thumbW - 1)) {
			myItem->itemFlags = H_THUMB_OVER;
			*currItem = myItem;
		} else {
			myItem->itemFlags = H_THUMB_NORM;
			*currItem = nullptr;
		}
		redrawItem = true;
		execCallback = true;
		break;

	case _ME_move:
		if (menuItem::cursorInsideItem(myItem, x, y) && (x - myItem->x1 >= myItem->thumbX) &&
			(x - myItem->x1 <= myItem->thumbX + myItem->thumbW - 1)) {
			if (myItem->itemFlags != H_THUMB_OVER) {
				myItem->itemFlags = H_THUMB_OVER;
				*currItem = myItem;
				redrawItem = true;
			}
		} else {
			if (myItem->itemFlags != H_THUMB_NORM) {
				myItem->itemFlags = H_THUMB_NORM;
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
		(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		(myItem->callback)((void *)myItem, myItem->myMenu);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}

menuItemHSlider *menuItemHSlider::add(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
	int32 initPercent, CALLBACK callback, bool transparent) {
	int32 status;

	// Verify params
	if (!myMenu) {
		return nullptr;
	}

	// Allocate a new one
	menuItemHSlider *newItem = new menuItemHSlider();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
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
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
	}

	// Intialize the new slider
	newItem->itemFlags = H_THUMB_NORM;
	auto *thumb = _GM(menuSprites)[IS_RIDDLE ? (int)Riddle::GUI::OM_SLIDER_BTN_NORM :
		(int)Burger::GUI::OM_SLIDER_BTN_NORM];
	newItem->thumbW = thumb->w;
	newItem->thumbH = thumb->h;
	newItem->maxThumbX = w - thumb->w;

	if (initPercent < 0) {
		initPercent = 0;
	} else if (initPercent > 100) {
		initPercent = 100;
	}

	// Calculate the initial thumbX
	newItem->percent = initPercent;
	newItem->thumbX = initPercent * newItem->maxThumbX / 100;

	newItem->redraw = (DrawFunction)menuItemHSlider::drawHSlider;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = (ItemHandlerFunction)menuItemHSlider::handler;

	// Draw the slider in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	ScreenContext *myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}


//-------------------------------    VSLIDER MENU ITEM    ---------------------------------//

menuItemVSlider *menuItemVSlider::add(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
	int32 initPercent, CALLBACK callback, bool transparent) {
	int32 status;

	// Verify params
	if (!myMenu)
		return nullptr;

	// Allocate a new one
	menuItemVSlider *newItem = new menuItemVSlider();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
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
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
	}

	newItem->itemFlags = menuItemVSlider::VS_NORM;

	if (IS_RIDDLE) {
		newItem->thumbW = _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_NORM_21]->w;
		newItem->thumbH = _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_NORM_21]->h;

		newItem->minThumbY = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_NORM_13]->h;
		newItem->maxThumbY = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_NORM_13]->h +
			_GM(menuSprites)[SaveLoadMenuBase::SL_SCROLL_BAR_24]->h
			- _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_NORM_21]->h - 1;
	} else {
		newItem->thumbW = _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_NORM_21]->w;
		newItem->thumbH = _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_NORM_21]->h;

		newItem->minThumbY = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_NORM_13]->h + 1;
		newItem->maxThumbY = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_NORM_13]->h +
			_GM(menuSprites)[SaveLoadMenuBase::SL_SCROLL_BAR_24]->h
			- _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_NORM_21]->h - 1;
	}

	// Calculate the initial thumbY
	newItem->percent = imath_max(imath_min(initPercent, 100), 0);
	newItem->thumbY = newItem->minThumbY +
		((newItem->percent * (newItem->maxThumbY - newItem->minThumbY)) / 100);

	newItem->redraw = (DrawFunction)menuItemVSlider::drawVSlider;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = (ItemHandlerFunction)menuItemVSlider::handler;

	// Draw the vslider in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	ScreenContext *myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}

void menuItemVSlider::drawVSlider(menuItemVSlider *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	Buffer *backgroundBuff = nullptr;

	// Verify params
	if (!myItem || !myMenu)
		return;

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

	// Get the menu buffer
	Buffer *myBuff = myMenu->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	// If the item is tagged as transparent, we need to fill in it's background behind it
	if (backgroundBuff) {
		gr_buffer_rect_copy_2(backgroundBuff, myBuff, 0, 0, x, y, backgroundBuff->w, backgroundBuff->h);
		myItem->background->release();
	}

	// Set the different sprite components
	Sprite *vbarSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SCROLL_BAR_24];
	Sprite *upSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_NORM_13];
	Sprite *thumbSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_NORM_21];
	Sprite *downSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_DOWN_BTN_NORM_14];

	if ((myItem->itemFlags & VS_STATUS) == VS_GREY) {
		upSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_GREY_19];
		thumbSprite = nullptr;
		downSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_DOWN_BTN_GREY_20];
	} else if ((myItem->itemFlags & VS_STATUS) == VS_OVER) {
		if ((myItem->itemFlags & VS_COMPONENT) == VS_UP) {
			upSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_OVER_15];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_THUMB) {
			thumbSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_OVER_22];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_DOWN) {
			downSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_DOWN_BTN_OVER_16];
		}
	} else if ((myItem->itemFlags & VS_STATUS) == VS_PRESS) {
		if ((myItem->itemFlags & VS_COMPONENT) == VS_UP) {
			upSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_UP_BTN_PRESS_17];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_THUMB) {
			thumbSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_SLIDER_BTN_PRESS_23];
		} else if ((myItem->itemFlags & VS_COMPONENT) == VS_DOWN) {
			downSprite = _GM(menuSprites)[SaveLoadMenuBase::SL_DOWN_BTN_PRESS_18];
		}
	}

	// Draw the sprite comonents
	gui_DrawSprite(vbarSprite, myBuff, x, y + upSprite->h);
	gui_DrawSprite(upSprite, myBuff, x, y);
	gui_DrawSprite(thumbSprite, myBuff, x, y + myItem->thumbY);
	gui_DrawSprite(downSprite, myBuff, x, y + upSprite->h + vbarSprite->h);

	// Release the menu buffer
	myMenu->menuBuffer->release();
}

int32 menuItemVSlider::whereIsCursor(menuItemVSlider *myVSlider, int32 y) {
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

bool menuItemVSlider::handler(menuItemVSlider *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	int32 tempFlags;
	ScreenContext *myScreen;
	int32 status;
	int32 deltaSlide;
	static bool movingFlag;
	static int32 movingY;
	static int32 callbackTime;

	// Verify params
	if (!myItem)
		return false;

	if (!(eventType == EVENT_MOUSE))
		return false;

	if ((myItem->itemFlags & VS_STATUS) == VS_GREY) {
		*currItem = nullptr;
		return false;
	}

	const int32 currTime = timer_read_60();
	bool redrawItem = false;
	bool handled = true;
	bool execCallback = false;

	switch (event) {
	case _ME_L_click:
	case _ME_doubleclick:
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			//				  digi_play(inv_click_snd, 2, 255, -1, inv_click_snd_room_lock);
			*currItem = myItem;
			tempFlags = menuItemVSlider::whereIsCursor(myItem, y - myItem->y1);
			if (tempFlags == VS_THUMB) {
				movingFlag = true;
				movingY = y;
			}
			if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
				myItem->itemFlags = tempFlags + VS_NORM;
			} else {
				myItem->itemFlags = tempFlags + VS_PRESS;
				redrawItem = true;
			}
			execCallback = true;
		} else {
			*currItem = nullptr;
			myItem->itemFlags = VS_NORM;
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
				deltaSlide = imath_min(myItem->thumbY - myItem->minThumbY, movingY - y);
				if (deltaSlide > 0) {
					myItem->thumbY -= deltaSlide;
					myItem->percent = ((myItem->thumbY - myItem->minThumbY) * 100) /
						(myItem->maxThumbY - myItem->minThumbY);
					redrawItem = true;
					execCallback = true;
				}
			} else if (y > movingY) {
				deltaSlide = imath_min(myItem->maxThumbY - myItem->thumbY, y - movingY);
				if (deltaSlide > 0) {
					myItem->thumbY += deltaSlide;
					myItem->percent = ((myItem->thumbY - myItem->minThumbY) * 100) /
						(myItem->maxThumbY - myItem->minThumbY);
					redrawItem = true;
					execCallback = true;
				}
			}
			movingY = y;
			if (movingY < (myItem->thumbY + myItem->y1)) {
				movingY = myItem->thumbY + myItem->y1;
			} else if (movingY > (myItem->thumbY + myItem->thumbH - 1 + myItem->y1)) {
				movingY = myItem->thumbY + myItem->thumbH - 1 + myItem->y1;
			}
		} else {
			if (menuItem::cursorInsideItem(myItem, x, y)) {
				tempFlags = menuItemVSlider::whereIsCursor(myItem, y - myItem->y1);
				if ((myItem->itemFlags & VS_COMPONENT) == tempFlags) {
					if ((tempFlags != VS_PAGE_UP) && (tempFlags != VS_PAGE_DOWN) &&
						((myItem->itemFlags & VS_STATUS) != VS_PRESS)) {
						myItem->itemFlags = tempFlags + VS_PRESS;
						redrawItem = true;
					}
					if (currTime - callbackTime > 6) {
						execCallback = true;
					}
				} else {
					if ((myItem->itemFlags & VS_STATUS) != VS_OVER) {
						myItem->itemFlags = (myItem->itemFlags & VS_COMPONENT) + VS_OVER;
						redrawItem = true;
					}
				}
				execCallback = true;
			} else {
				if ((myItem->itemFlags & VS_STATUS) != VS_OVER) {
					myItem->itemFlags = (myItem->itemFlags & VS_COMPONENT) + VS_OVER;
					redrawItem = true;
				}
			}
		}
		break;

	case _ME_L_release:
	case _ME_doubleclick_release:
		movingFlag = false;
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			tempFlags = menuItemVSlider::whereIsCursor(myItem, y - myItem->y1);
			if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
				myItem->itemFlags = VS_NORM;
			} else {
				myItem->itemFlags = tempFlags + VS_OVER;
				*currItem = myItem;
			}
		} else {
			myItem->itemFlags = VS_NORM;
			*currItem = nullptr;
		}
		redrawItem = true;
		if (!_GM(currMenuIsSave)) {
			SaveLoadMenuBase::updateThumbnails(_GM(firstSlotIndex), (guiMenu *)myItem->myMenu);
		}
		break;

	case _ME_move:
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			*currItem = myItem;
			tempFlags = menuItemVSlider::whereIsCursor(myItem, y - myItem->y1);
			if ((myItem->itemFlags & VS_COMPONENT) != tempFlags) {
				if ((tempFlags == VS_PAGE_UP) || (tempFlags == VS_PAGE_DOWN)) {
					myItem->itemFlags = VS_NORM;
				} else {
					myItem->itemFlags = tempFlags + VS_OVER;
				}
				redrawItem = true;
			}
		} else {
			*currItem = nullptr;
			if (myItem->itemFlags != VS_NORM) {
				myItem->itemFlags = VS_NORM;
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
		if (menuItem::cursorInsideItem(myItem, x, y)) {
			tempFlags = menuItemVSlider::whereIsCursor(myItem, y - myItem->y1);
			if ((myItem->itemFlags & VS_COMPONENT) == tempFlags) {
				if (currTime - callbackTime > 6) {
					execCallback = true;
				}
			}
		}
		break;
	}

	// See if we need to redraw the vslider
	if (redrawItem) {
		(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		callbackTime = currTime;
		(myItem->callback)((void *)myItem, myItem->myMenu);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}

void menuItemVSlider::disableVSlider(menuItemVSlider *myItem, int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu)
		return;

	if (!myItem)
		myItem = (menuItemVSlider *)guiMenu::getItem(tag, myMenu);
	if (!myItem)
		return;

	myItem->itemFlags = menuItemVSlider::VS_GREY;
}

void menuItemVSlider::enableVSlider(menuItemVSlider *myItem, int32 tag, guiMenu *myMenu) {
	// Verify params
	if (!myMenu)
		return;

	if (!myItem)
		myItem = (menuItemVSlider *)guiMenu::getItem(tag, myMenu);
	if (!myItem)
		return;

	myItem->itemFlags = menuItemVSlider::VS_NORM;
}


//-----------------------------    TEXTFIELD MENU ITEM    ---------------------------------//

void menuItemTextField::drawTextField(menuItemTextField *myItem, guiMenu *myMenu, int32 x, int32 y, int32, int32) {
	Buffer *backgroundBuff = nullptr;
	Sprite *mySprite;
	char tempStr[64];

	// Verify params
	if (!myItem || !myMenu)
		return;

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

	// Select the sprite
	switch (myItem->itemFlags) {
	case TF_GREY:
		mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LINE_NORM];
		break;

	case TF_OVER:
		mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LINE_OVER];
		break;

	case TF_NORM:
	default:
		mySprite = _GM(menuSprites)[SaveLoadMenuBase::SL_LINE_OVER];
		break;
	}

	// Get the menu buffer and draw the sprite to it
	Buffer *myBuff = myMenu->menuBuffer->get_buffer();
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
	gr_font_set_color(menuItem::TEXT_COLOR_NORM_FOREGROUND);
	Common::sprintf_s(tempStr, 64, "%02d", myItem->specialTag);
	gr_font_set(_GM(menuFont));
	gr_font_write(myBuff, tempStr, x + 4, y + 1, 0, -1);

	//write in the text
	gr_font_write(myBuff, &myItem->prompt[0], x + 26, y + 1, 0, -1);

	if (myItem->itemFlags == TF_OVER) {
		// Draw in the cursor
		if (myItem->cursor) {
			const char tempChar = *myItem->cursor;
			*myItem->cursor = '\0';
			const int32 cursorX = gr_font_string_width(&myItem->prompt[0], -1);
			*myItem->cursor = tempChar;

			gr_color_set(menuItem::TEXT_COLOR_OVER_FOREGROUND);
			gr_vline(myBuff, x + cursorX + 26, y + 1, y + 12);
		}
	}

	// Release the menu buffer
	myMenu->menuBuffer->release();
}

bool menuItemTextField::handler(menuItemTextField *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	ScreenContext *myScreen;
	int32 status, temp;
	char tempStr[80];

	// Verify params
	if (!myItem)
		return false;

	if (myItem->itemFlags == TF_GREY) {
		return false;
	}

	bool redrawItem = false;
	bool execCallback = false;
	const bool handled = true;

	if (eventType == EVENT_MOUSE) {
		switch (event) {
		case _ME_L_click:
		case _ME_doubleclick:
			_GM(deleteSaveDesc) = false;
			if (menuItem::cursorInsideItem(myItem, x, y)) {
				*currItem = myItem;
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
			if (menuItem::cursorInsideItem(myItem, x, y)) {
				if (myItem->itemFlags == TF_OVER) {
					temp = strlen(myItem->prompt);
					if (temp > 0) {
						Common::strcpy_s(tempStr, myItem->prompt);
						char *tempPtr = &tempStr[temp];
						gr_font_set(_GM(menuFont));
						temp = gr_font_string_width(tempStr, -1);
						while ((tempPtr != &tempStr[0]) && (temp > x - myItem->x1 - 26)) {
							*--tempPtr = '\0';
							temp = gr_font_string_width(tempStr, -1);
						}
						myItem->cursor = &myItem->prompt[tempPtr - &tempStr[0]];
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
	} else if ((eventType == EVENT_KEY) && (myItem->itemFlags == TF_OVER)) {
		switch (event) {
		case KEY_RETURN:
			_GM(deleteSaveDesc) = false;
			execCallback = true;
			break;

		case KEY_HOME:
			_GM(deleteSaveDesc) = false;
			myItem->cursor = &myItem->prompt[0];
			redrawItem = true;
			break;

		case KEY_END:
			_GM(deleteSaveDesc) = false;
			myItem->cursor = myItem->promptEnd;
			redrawItem = true;
			break;

		case KEY_LEFT:
			_GM(deleteSaveDesc) = false;
			if (myItem->cursor > &myItem->prompt[0]) {
				myItem->cursor--;
				redrawItem = true;
			}
			break;

		case KEY_RIGHT:
			_GM(deleteSaveDesc) = false;
			if (myItem->cursor < myItem->promptEnd) {
				myItem->cursor++;
				redrawItem = true;
			}
			break;

		case KEY_DELETE:
			if (_GM(deleteSaveDesc)) {
				myItem->prompt[0] = '\0';
				myItem->promptEnd = &myItem->prompt[0];
				myItem->cursor = myItem->promptEnd;
				redrawItem = true;
			} else if (myItem->cursor < myItem->promptEnd) {
				Common::strcpy_s(tempStr, (char *)(myItem->cursor + 1));
				Common::strcpy_s(myItem->cursor, 80, tempStr);
				myItem->promptEnd--;
				redrawItem = true;
			}
			break;

		case KEY_BACKSP:
			_GM(deleteSaveDesc) = false;
			if (myItem->cursor > &myItem->prompt[0]) {
				Common::strcpy_s(tempStr, myItem->cursor);
				myItem->promptEnd--;
				myItem->cursor--;
				Common::strcpy_s(myItem->cursor, 80, tempStr);
				redrawItem = true;
			}
			break;

		default:
			_GM(deleteSaveDesc) = false;
			gr_font_set(_GM(menuFont));
			temp = gr_font_string_width(&myItem->prompt[0], -1);
			if ((strlen(&myItem->prompt[0]) < 79) && (temp < myItem->pixWidth - 12) && (event >= 32) && (event <= 127)) {
				if (myItem->cursor < myItem->promptEnd) {
					Common::strcpy_s(tempStr, (char *)myItem->cursor);
					Common::sprintf_s(myItem->cursor, 80, "%c%s", (char)event, tempStr);
				} else {
					*myItem->cursor = (char)event;
					*(myItem->cursor + 1) = '\0';
				}
				myItem->cursor++;
				myItem->promptEnd++;

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
		(myItem->redraw)(myItem, myItem->myMenu, myItem->x1, myItem->y1, 0, 0);
		myScreen = vmng_screen_find(myItem->myMenu, &status);
		if (myScreen && (status == SCRN_ACTIVE)) {
			RestoreScreens(myScreen->x1 + myItem->x1, myScreen->y1 + myItem->y1,
				myScreen->x1 + myItem->x2, myScreen->y1 + myItem->y2);
		}
	}

	// See if we need to call the callback function
	if (execCallback && myItem->callback) {
		(myItem->callback)((void *)myItem, myItem->myMenu);
		myScreen = vmng_screen_find(myItem->myMenu, &status);

		if ((!myScreen) || (status != SCRN_ACTIVE)) {
			*currItem = nullptr;
		}
	}

	return handled;
}

menuItemTextField *menuItemTextField::add(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, int32 initFlags,
	const char *prompt, int32 specialTag, CALLBACK callback, bool transparent) {
	menuItemTextField *textInfo;
	int32 status;

	// Verify params
	if (!myMenu)
		return nullptr;

	// Allocate a new one
	menuItemTextField *newItem = new menuItemTextField();

	// Initialize the struct
	newItem->next = myMenu->itemList;
	newItem->prev = nullptr;
	if (myMenu->itemList) {
		myMenu->itemList->prev = newItem;
	}
	myMenu->itemList = newItem;

	newItem->myMenu = myMenu;
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
		newItem->background = guiMenu::copyBackground(myMenu, x, y, w, h);
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

	newItem->redraw = (DrawFunction)menuItemTextField::drawTextField;
	newItem->destroy = (DestroyFunction)menuItem::destroyItem;
	newItem->itemEventHandler = (ItemHandlerFunction)menuItemTextField::handler;

	// Draw the vslider in now
	(newItem->redraw)(newItem, myMenu, x, y, 0, 0);

	// See if the screen is currently visible
	ScreenContext *myScreen = vmng_screen_find(myMenu, &status);
	if (myScreen && (status == SCRN_ACTIVE)) {
		RestoreScreens(myScreen->x1 + newItem->x1, myScreen->y1 + newItem->y1,
			myScreen->x1 + newItem->x2, myScreen->y1 + newItem->y2);
	}

	return newItem;
}

} // namespace GUI
} // namespace M4
