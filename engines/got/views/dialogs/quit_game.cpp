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

#include "got/views/dialogs/quit_game.h"
#include "got/got.h"

namespace Got {
namespace Views {
namespace Dialogs {

static const char *OPTIONS[] = { "Continue Game", "Quit to Opening Screen", "Quit to DOS", nullptr };

QuitGame::QuitGame() : SelectOption("QuitGame", "Quit Game?", OPTIONS) {
}

void QuitGame::selected() {
	switch (_selectedItem) {
	case 0:
		break;
	case 1:
		// Prompt for saving game before returning to title
		send("SaveGame", GameMessage("TITLE"));
		break;
	case 2:
		// Prompt for saving game before quitting
		send("SaveGame", GameMessage("QUIT"));
		break;
	default:
		break;
	}
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
