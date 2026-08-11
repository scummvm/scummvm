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

#include "got/views/dialogs/save_game.h"
#include "got/got.h"

namespace Got {
namespace Views {
namespace Dialogs {

SaveGame::SaveGame() : SelectOption("SaveGame", "Save Game?", YES_NO) {
}

bool SaveGame::msgGame(const GameMessage &msg) {
	if (msg._name == "TITLE" || msg._name == "QUIT") {
		_isQuit = msg._name == "QUIT";
		open();
		return true;
	}

	return false;
}

void SaveGame::selected() {
	if (_selectedItem == 0)
		g_engine->saveGameDialog();

	if (_isQuit) {
		g_engine->quitGame();
	} else {
		fadeOut();
		send("TitleBackground", GameMessage("MAIN_MENU"));
	}
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
