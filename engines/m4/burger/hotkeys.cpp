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

#include "m4/burger/hotkeys.h"
#include "m4/burger/burger_globals.h"
#include "m4/burger/gui/game_menu.h"
#include "m4/burger/other.h"

namespace M4 {
namespace Burger {

void escape_key_pressed(void *, void *) {
	// Decide what to do depending on what kind of game is playing
	switch (_G(executing)) {
	case JUST_OVERVIEW:
		// Stop playing the overview
		_G(game).going = false;
		break;

	case INTERACTIVE_DEMO:
	case MAGAZINE_DEMO:
		if (_G(game).room_id == 901) {
			// Already on the demo menu screen, so quit game
			_G(game).going = false;
		} else {
			// In-game, so go back to demo menu screen
			_G(game).new_section = 9;
			_G(game).new_room = 901;
		}
		break;

	case WHOLE_GAME:
		// The real thing is playing 
		switch (_G(game).room_id) {
		case 902:
		case 904:
		case 951:
		case 971:
			// Switch to main menu
			_G(game).new_section = 9;
			_G(game).new_room = 901;
			break;

		case 903:
			// Quit game
			_G(game).going = false;
			break;

		default:
			if (player_commands_allowed()) {
				// Bring up the main menu
				other_save_game_for_resurrection();
				GUI::CreateGameMenu(&_G(master_palette)[0]);
			}
			break;
		}
		break;
	}
}

} // namespace Burger
} // namespace M4
