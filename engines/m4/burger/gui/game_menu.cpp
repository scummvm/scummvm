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
#include "m4/burger/burger_globals.h"

namespace M4 {
namespace Burger {

void CreateGameMenu(RGB8 *myPalette) {
#ifdef TODO
	if ((!player_commands_allowed()) || (!interface_visible) ||
		pal_fade_in_progress || menuSystemInitialized) {
		return;
	}
	gameMenuFromMain = FALSE;
	CreateGameMenuMain(myPalette);
#else
	error("TODO: CreateGameMenu");
#endif
}

void CreateGameMenuFromMain(RGB8 *myPalette) {
#ifdef TODO
	if (pal_fade_in_progress || menuSystemInitialized) {
		return;
	}
	gameMenuFromMain = TRUE;
	CreateGameMenuMain(myPalette);
#else
	error("TODO: CreateGameMenuFromMain");
#endif
}

} // namespace Burger
} // namespace M4
