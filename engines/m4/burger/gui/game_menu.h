
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

#ifndef M4_BURGER_GUI_GAME_MENU_H
#define M4_BURGER_GUI_GAME_MENU_H

#include "graphics/surface.h"
#include "m4/m4_types.h"
#include "m4/graphics/gr_buff.h"
#include "m4/gui/gui_menu_items.h"
#include "m4/gui/game_menu.h"
#include "m4/gui/gui_univ.h"

namespace M4 {
namespace Burger {
namespace GUI {

using M4::GUI::guiMenu;
using M4::GUI::menuItemButton;
using M4::GUI::menuItemMsg;
using M4::GUI::menuItemHSlider;
using M4::GUI::menuItemVSlider;
using M4::GUI::menuItemTextField;
using M4::GUI::Sprite;
using M4::GUI::CALLBACK;
using M4::GUI::ItemHandlerFunction;

class SaveLoadMenu : public M4::GUI::SaveLoadMenuBase {
private:
	static void destroyMenu(bool saveMenu);
	static bool load_Handler(menuItemButton *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem);

	static void cb_SaveLoad_Save(void *, guiMenu *myMenu);
	static void cb_SaveLoad_Load(menuItemButton *, guiMenu *);
	static void cb_SaveLoad_Cancel(menuItemButton *, guiMenu *myMenu);
	static void cb_SaveLoad_Slot(menuItemButton *myButton, guiMenu *myMenu);
	static void cb_SaveLoad_VSlider(menuItemVSlider *myItem, guiMenu *myMenu);

public:
	static void show(RGB8 *myPalette, bool saveMenu);
};

//GAME MENU FUNCTIONS
extern void CreateGameMenu(RGB8 *myPalette);
extern void CreateOptionsMenu(RGB8 *myPalette);
extern void CreateF2SaveMenu(RGB8 *myPalette);
extern void CreateLoadMenu(RGB8 *myPalette);
extern void CreateF3LoadMenu(RGB8 *myPalette);

//routines used by the main menu
void CreateLoadMenuFromMain(RGB8 *myPalette);
void CreateGameMenuFromMain(RGB8 *myPalette);

//======================================
//
//		Game menu enums and defines
//

enum game_menu_button_tags {
	GM_TAG_QUIT = 1,
	GM_TAG_OPTIONS = 2,
	GM_TAG_RESUME = 3,
	GM_TAG_SAVE = 4,
	GM_TAG_LOAD = 5,
	GM_TAG_MAIN = 6
};

/**
 * Error menu enums and defines
 */
enum error_menu_sprites {
	EM_DIALOG_BOX,

	EM_RETURN_BTN_NORM,
	EM_RETURN_BTN_OVER,
	EM_RETURN_BTN_PRESS,

	EM_TOTAL_SPRITES
};

enum error_menu_tags {
	EM_TAG_RETURN = 1
};
#define ERROR_MENU_X	 100
#define ERROR_MENU_Y	 100
#define ERROR_MENU_W	 100
#define ERROR_MENU_H	 100

#define EM_RETURN_X	  15
#define EM_RETURN_Y	  15
#define EM_RETURN_W	  15
#define EM_RETURN_H	  15


void CreateGameMenuMain(RGB8 *myPalette);

} // namespace GUI
} // namespace Burger
} // namespace M4

#endif
