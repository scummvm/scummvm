
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

class GameMenu {
private:
	static void destroyMenu();
	static void cb_Game_Quit(void *, void *);
	static void cb_Game_Resume(void *, void *);
	static void cb_Game_Save(void *, void *);
	static void cb_Game_Load(void *, void *);
	static void cb_Game_Main(void *, void *);
	static void cb_Game_Options(void *, void *);

public:
	static void show(RGB8 *myPalette);
};

class OptionsMenu {
private:
	static void destroyMenu();
	static void cb_Options_Game_Cancel(void *, void *);
	static void cb_Options_Game_Done(void *, void *);
	static void cb_Options_Digi(menuItemHSlider *myItem, guiMenu *myMenu);
	static void cb_Options_Digestability(menuItemHSlider *myItem, guiMenu *myMenu);
	static void setDigiVolume(uint16 volume);
	static void setDigiVolumePerc(uint8 volumePerc);
	static uint16 getDigiVolume();
	static uint8 getDigiVolumePerc();

public:
	static void show(RGB8 *myPalette);
};

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

class ErrorMenu {
private:
	static void destroyMenu();
	static void cb_Err_Done(void *, void *);

public:
	static void show(RGB8 *myPalette);
};

// GAME MENU FUNCTIONS
extern void CreateGameMenu(RGB8 *myPalette);
extern void CreateF2SaveMenu(RGB8 *myPalette);
extern void CreateLoadMenu(RGB8 *myPalette);
extern void CreateF3LoadMenu(RGB8 *myPalette);

// Routines used by the main menu
void CreateLoadMenuFromMain(RGB8 *myPalette);
void CreateGameMenuFromMain(RGB8 *myPalette);

} // namespace GUI
} // namespace Burger
} // namespace M4

#endif
