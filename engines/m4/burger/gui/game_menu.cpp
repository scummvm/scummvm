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

#include "m4/burger/gui/game_menu.h"
#include "m4/burger/gui/interface.h"
#include "m4/adv_r/adv_player.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace GUI {

void CreateGameMenuMain(RGB8 *myPalette) {
	error("TODO: CreateGameMenuMain");
}

void CreateSaveLoadMenu(RGB8 *myPalette, bool saveMenu) {
	error("TODO: CreateSaveLoadMenu");
}

void CreateGameMenu(RGB8 *myPalette) {
	if (player_commands_allowed() && _GI(visible) && !_G(pal_fade_in_progress)
			&& !_G(menuSystemInitialized)) {
		_G(gameMenuFromMain) = false;
		CreateGameMenuMain(myPalette);
	}
}

void CreateGameMenuFromMain(RGB8 *myPalette) {
	if (!_G(pal_fade_in_progress) && _G(menuSystemInitialized)) {
		_G(gameMenuFromMain) = true;
		CreateGameMenuMain(myPalette);
	}
}

void CreateLoadMenuFromMain(RGB8 *myPalette) {
	if (_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_G(gui).saveLoadFromHotkey = true;
	_G(gui).gameMenuFromMain = true;
	CreateSaveLoadMenu(myPalette, false);
}

} // namespace GUI
} // namespace Burger
} // namespace M4
