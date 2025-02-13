
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

namespace Burger {
namespace GUI {

enum save_load_menu_sprites {
	SL_DIALOG_BOX,
	SL_EMPTY_THUMB,

	SL_SAVE_BTN_GREY,
	SL_SAVE_BTN_NORM,
	SL_SAVE_BTN_OVER,
	SL_SAVE_BTN_PRESS,

	SL_LOAD_BTN_GREY,
	SL_LOAD_BTN_NORM,
	SL_LOAD_BTN_OVER,
	SL_LOAD_BTN_PRESS,

	SL_CANCEL_BTN_NORM,
	SL_CANCEL_BTN_OVER,
	SL_CANCEL_BTN_PRESS,

	SL_UP_BTN_GREY,
	SL_UP_BTN_NORM,
	SL_UP_BTN_OVER,
	SL_UP_BTN_PRESS,

	SL_DOWN_BTN_GREY,
	SL_DOWN_BTN_NORM,
	SL_DOWN_BTN_OVER,
	SL_DOWN_BTN_PRESS,

	SL_SAVE_LABEL,
	SL_LOAD_LABEL,

	SL_SLIDER_BTN_NORM,
	SL_SLIDER_BTN_OVER,
	SL_SLIDER_BTN_PRESS,

	SL_LINE_NORM,
	SL_LINE_OVER,
	SL_LINE_PRESS,

	SL_SCROLL_BAR,

	SL_TOTAL_SPRITES
};

enum options_menu_sprites {
	OM_DIALOG_BOX,

	OM_SLIDER_BTN_NORM,
	OM_SLIDER_BTN_OVER,
	OM_SLIDER_BTN_PRESS,

	OM_SLIDER_BAR,

	OM_DONE_BTN_GREY,
	OM_DONE_BTN_NORM,
	OM_DONE_BTN_OVER,
	OM_DONE_BTN_PRESS,

	OM_CANCEL_BTN_NORM,
	OM_CANCEL_BTN_OVER,
	OM_CANCEL_BTN_PRESS,

	OM_TOTAL_SPRITES
};

} // namespace GUI
} // namespace Burger

namespace GUI {

#define _GM(X) ::M4::g_vars->_menu.X
#define LockMouseSprite mouse_lock_sprite
#define UnlockMouseSprite mouse_unlock_sprite

struct menuItem;
struct guiMenu;

typedef bool (*ItemHandlerFunction)(menuItem *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem);
typedef void (*DrawFunction)(void *source, guiMenu *dest, int32 x1, int32 y1, int32 x2, int32 y2);
typedef void (*DestroyFunction)(menuItem *theItem);
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

struct guiMenu;

struct menuItem {
	enum {
		TEXT_COLOR_GREY_HILITE		= 192,
		TEXT_COLOR_GREY_FOREGROUND  = 210,
		TEXT_COLOR_GREY_SHADOW		= 229,

		TEXT_COLOR_NORM_HILITE		= 3,
		TEXT_COLOR_NORM_FOREGROUND	= 2,
		TEXT_COLOR_NORM_SHADOW		= 1,

		TEXT_COLOR_OVER_HILITE		= 3,
		TEXT_COLOR_OVER_FOREGROUND	= 2,
		TEXT_COLOR_OVER_SHADOW		= 1,

		TEXT_COLOR_PRESS_HILITE		= 3,
		TEXT_COLOR_PRESS_FOREGROUND = 2,
		TEXT_COLOR_PRESS_SHADOW		= 1,

		SLIDER_BAR_COLOR			= 129
	};

	menuItem *next = nullptr;
	menuItem *prev = nullptr;

	guiMenu *myMenu = nullptr;
	int32 tag = 0;

	int32 x1 = 0, y1 = 0, x2 = 0, y2 = 0;

	bool transparent = false;
	GrBuff *background = nullptr;

	CALLBACK callback = nullptr;
	DrawFunction redraw = nullptr;
	DestroyFunction destroy = nullptr;
	ItemHandlerFunction	itemEventHandler = nullptr;

	static void destroyItem(menuItem *theItem);
	static bool cursorInsideItem(menuItem *myItem, int32 cursorX, int32 cursorY);
};


struct menuItemMsg : public menuItem {
	int32 itemFlags = 0;
};

struct menuItemButton : public menuItem {
private:
	static void drawButton(menuItemButton *myItem, guiMenu *myMenu,
		int32 x, int32 y, int32, int32);

public:
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

	int32 itemFlags = 0;
	int32 buttonType = 0;
	const char *prompt = nullptr;
	menuItem *assocItem = nullptr;
	int32 specialTag = 0;

	static menuItemButton *buttonAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, CALLBACK callback = nullptr,
		int32 buttonType = 0, bool ghosted = false, bool transparent = false,
		const char *prompt = nullptr, ItemHandlerFunction i_handler = (ItemHandlerFunction)handler);
	static void disableButton(menuItemButton *myItem, int32 tag, guiMenu *myMenu);
	static void enableButton(menuItemButton *myItem, int32 tag, guiMenu *myMenu);
	static bool handler(menuItemButton *theItem, int32 eventType, int32 event,
		int32 x, int32 y, void **currItem);
};

struct menuItemHSlider : public menuItem {
	int32 itemFlags = 0;

	int32 thumbW = 0, thumbH = 0;
	int32 thumbX = 0, maxThumbX = 0;

	int32 percent = 0;
};

struct menuItemVSlider : public menuItem {
	int32 itemFlags = 0;

	int32 thumbW = 0, thumbH = 0;
	int32 thumbY = 0, minThumbY = 0, maxThumbY = 0;

	int32 percent = 0;
};

struct menuItemTextField : public menuItem {
	int32 itemFlags = 0;

	int32 specialTag = 0;
	int32 pixWidth = 0;

	char prompt[80] = { 0 };
	char *promptEnd = nullptr;

	char *cursor = nullptr;
};

struct guiMenu {
private:
	static void show(void *s, void *r, void *b, int32 destX, int32 destY);
	static bool eventHandler(guiMenu *theMenu, int32 eventType, int32 parm1, int32 parm2, int32 parm3, bool *currScreen);

public:
	GrBuff *menuBuffer = nullptr;
	menuItem *itemList = nullptr;
	CALLBACK cb_return = nullptr;
	CALLBACK cb_esc = nullptr;
	EventHandler menuEventHandler = nullptr;

	static bool initialize(RGB8 *myPalette);
	static void shutdown(bool fadeToColor);
	static guiMenu *create(Sprite *backgroundSprite, int32 x1, int32 y1, int32 scrnFlags);
	static void destroy(guiMenu *myMenu);
	static void configure(guiMenu *myMenu, CALLBACK cb_return, CALLBACK cb_esc);
	static GrBuff *copyBackground(guiMenu *myMenu, int32 x, int32 y, int32 w, int32 h);
	static menuItem *getItem(int32 tag, guiMenu *myMenu);
	static void itemDelete(menuItem *myItem, int32 tag, guiMenu *myMenu);
	static void itemRefresh(menuItem *myItem, int32 tag, guiMenu *myMenu);

	static bool loadSprites(const char *series, int32 numSprites);
	static void unloadSprites();
};

struct MenuGlobals {
	//GLOBAL VARS
	bool menuSystemInitialized = false;
	bool buttonClosesDialog = false;
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

extern void gui_DrawSprite(Sprite *mySprite, Buffer *myBuff, int32 x, int32 y);

//======================================
//
//		gamemenu module defines
//
#define MEMORY_NEEDED		0	// bytes needed for menus to work
#define MENU_DEPTH 			9 	// video depth for menu popup boxes
#define MAX_SLOTS			99	// number of save games you can have
#define MAX_SLOTS_SHOWN 	8	// number of slots in the scrolling field

} // namespace GUI
} // namespace M4

#endif
