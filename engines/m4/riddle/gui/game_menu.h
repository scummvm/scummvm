
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

#ifndef M4_RIDDLE_GUI_GAME_MENU_H
#define M4_RIDDLE_GUI_GAME_MENU_H

#include "m4/gui/gui_menu_items.h"
#include "m4/gui/game_menu.h"

namespace M4 {
namespace Riddle {
namespace GUI {

class GameMenu {
private:
	static void destroyGameMenu();
	static void cbQuitGame(void *, void *);
	static void cbMainMenu(void *, void *);
	static void cbResume(void *, void *);
	static void cbOptions(void *, void *);
	static void cbSave(void *, void *);
	static void cbLoad(void *, void *);

public:
	static void show(RGB8 *myPalette);
};

class OptionsMenu {
private:
	static void destroyOptionsMenu();
	static void cbGameMenu(void *, void *);
	static void cbScrolling(M4::GUI::menuItemButton *myItem, M4::GUI::guiMenu *);
	static void cbSetDigi(M4::GUI::menuItemHSlider *myItem, M4::GUI::guiMenu *);
	static void cbSetMidi(M4::GUI::menuItemHSlider *myItem, M4::GUI::guiMenu *);

public:
	static void show();
};

class SaveLoadMenu : public M4::GUI::SaveLoadMenuBase {
private:
	static void destroyMenu(bool saveMenu);
	static bool load_Handler(M4::GUI::menuItemButton *myItem, int32 eventType,
		int32 event, int32 x, int32 y, void **currItem);
	static void cbCancel(M4::GUI::menuItemButton *, M4::GUI::guiMenu *myMenu);
	static void cbSave(void *, M4::GUI::guiMenu *myMenu);
	static void cbLoad(void *, M4::GUI::guiMenu *myMenu);
	static void cbSlot(M4::GUI::menuItemButton *myButton, M4::GUI::guiMenu *myMenu);
	static void cbVSlider(M4::GUI::menuItemVSlider *myItem, M4::GUI::guiMenu *myMenu);

public:
	static void show(RGB8 *myPalette, bool saveMenu);
};

class ErrorMenu {
private:
	static void destroyMenu();
	static void cbDone(void *, void *);

public:
	static void show(RGB8 *myPalette);
};

extern void CreateGameMenu(RGB8 *myPalette);
extern void CreateF2SaveMenu(RGB8 *myPalette);
extern void CreateLoadMenu(RGB8 *myPalette);
extern void CreateF3LoadMenu(RGB8 *myPalette);
// Routines used by the main menu
void CreateLoadMenuFromMain(RGB8 *myPalette);
void CreateGameMenuFromMain(RGB8 *myPalette);

} // namespace GUI
} // namespace Riddle
} // namespace M4

#endif
