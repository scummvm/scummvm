
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

#ifndef M4_GUI_GUI_MENU_H
#define M4_GUI_GUI_MENU_H

#include "graphics/surface.h"
#include "m4/m4_types.h"
#include "m4/graphics/gr_buff.h"
#include "m4/gui/gui_univ.h"

namespace M4 {
namespace GUI {

#define _GM(X) ::M4::g_vars->_menu.X
#define LockMouseSprite mouse_lock_sprite
#define UnlockMouseSprite mouse_unlock_sprite

typedef bool (*ItemHandlerFunction)(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem);
typedef void (*DrawFunction)(void *source, void *dest, int32 x1, int32 y1, int32 x2, int32 y2);
typedef void (*DestroyFunction)(void *theItem);
typedef M4CALLBACK CALLBACK;

typedef M4sprite Sprite;

enum game_menu_sprites {
	GM_DIALOG_BOX,

	GM_BUTTON_GREY,
	GM_BUTTON_NORM,
	GM_BUTTON_OVER,
	GM_BUTTON_PRESS,

	GM_TOTAL_SPRITES
};


struct menuItem {
	menuItem *next;
	menuItem *prev;

	void *myMenu;
	int32 tag;

	int32 x1, y1, x2, y2;

	bool transparent;
	GrBuff *background;

	void *itemInfo;

	CALLBACK callback;
	DrawFunction redraw;
	DestroyFunction destroy;
	ItemHandlerFunction	itemEventHandler;
};


struct menuItemMsg {
	int32 itemFlags;
};

struct menuItemButton {
	int32 itemFlags;
	int32 buttonType;
	const char *prompt;
	menuItem *assocItem;
	int32 specialTag;
};

struct menuItemHSlider {
	int32 itemFlags;

	int32 thumbW, thumbH;
	int32 thumbX, maxThumbX;

	int32 percent;
};

struct menuItemVSlider {
	int32 itemFlags;

	int32 thumbW, thumbH;
	int32 thumbY, minThumbY, maxThumbY;

	int32 percent;
};

struct menuItemTextField {
	int32 itemFlags;

	int32 specialTag;
	int32 pixWidth;

	char prompt[80];
	char *promptEnd;

	char *cursor;
};

struct guiMenu {
	GrBuff *menuBuffer;
	menuItem *itemList;
	CALLBACK cb_return;
	CALLBACK cb_esc;
	EventHandler menuEventHandler;
};

struct MenuGlobals {
	//GLOBAL VARS
	bool menuSystemInitialized = false;
	bool interfaceWasVisible = false;
	RGB8 *menuPalette = nullptr;
	bool dumpedCodes = false;
	bool dumpedBackground = false;

	menuItem *menuCurrItem = nullptr;

	guiMenu *gameMenu = nullptr;
	guiMenu *opMenu = nullptr;
	guiMenu *slMenu = nullptr;
	guiMenu *errMenu = nullptr;

	//menu sprite series vars
	char *menuSeriesResource = nullptr;
	MemHandle menuSeriesHandle = nullptr;
	int32 menuSeriesOffset = 0;
	int32 menuSeriesPalOffset = 0;

	Font *menuFont = nullptr;

	// menu sprites array (used to hold all the sprites for the current menu, spriteCount is set tot he number of sprites in the series)
	int32 spriteCount = 0;
	Sprite **menuSprites = nullptr;

	// VARS SPECIFIC TO THE GAME MENUS SYSTEM
	// An array of slot titles used by the save/load menus
	char **slotTitles = nullptr;
	bool *slotInUse = nullptr;
	int32 firstSlotIndex = 0;	// Slot at the top of the list on menu
	int32 slotSelected = -1;	// Slot currently selected 
	bool deleteSaveDesc = false;

	Sprite **thumbNails = nullptr;
	Sprite *saveLoadThumbNail = nullptr;	// Original used for menu display
	Graphics::Surface _thumbnail;			// ScummVM version used for savegame
	int32 sizeofThumbData = -1;
	int32 thumbIndex = 0;

	bool currMenuIsSave = true;			// Used to determine load or save menu
	bool saveLoadFromHotkey = false;	// Come from hotkey, not through game menu
	bool gameMenuFromMain = false;		// Come from main menu, not through escape

	int32 remember_digi_volume = 0;		// For cancelling out of the options menu
	int32 remember_digestability = 0;	// For cancelling out of the options menu

	~MenuGlobals() {
		_thumbnail.free();
	}
};

//======================================
//
//		gamemenu module defines
//
#define MEMORY_NEEDED		0	// bytes needed for menus to work
#define MENU_DEPTH 			9 	// video depth for menu popup boxes
#define MAX_SLOTS			99	// number of save games you can have
#define MAX_SLOTS_SHOWN 	8	// number of slots in the scrolling field

// GENERAL MENU FUNCTIONS
extern bool menu_Initialize(RGB8 *myPalette);
extern void menu_Shutdown(bool fadeToColor);
extern guiMenu *menu_Create(Sprite *backgroundSprite, int32 x1, int32 y1, int32 scrnFlags);
extern void menu_Destroy(guiMenu *myMenu);
extern void menu_Configure(guiMenu *myMenu, CALLBACK cb_return, CALLBACK cb_esc);
extern GrBuff *menu_CopyBackground(guiMenu *myMenu, int32 x, int32 y, int32 w, int32 h);
extern menuItem *menu_GetItem(int32 tag, guiMenu *myMenu);
extern void menu_ItemDelete(menuItem *myItem, int32 tag, guiMenu *myMenu);
extern void menu_ItemRefresh(menuItem *myItem, int32 tag, guiMenu *myMenu);

extern bool menu_LoadSprites(const char *series, int32 numSprites);
extern void menu_UnloadSprites();

} // namespace GUI
} // namespace M4

#endif
