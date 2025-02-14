
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

public:
	static void show();
};

extern void CreateGameMenu(RGB8 *myPalette);

} // namespace GUI
} // namespace Riddle
} // namespace M4

#endif
