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

#include "got/views/dialogs/main_menu.h"

namespace Got {
namespace Views {
namespace Dialogs {

static const char *OPTIONS[] = {
	"Play Game", "High Scores", "Credits", "Demo", "Quit", nullptr
};

MainMenu::MainMenu() : SelectOption("MainMenu", "God of Thunder Menu", OPTIONS) {
}

void MainMenu::closed() {
	_selectedItem = 4; // Quit game
	selected();
}

void MainMenu::selected() {
	switch (_selectedItem) {
	case 0:
		// FIXME: Dummy test of fading code
		replaceView("QuitGame", true, true);
		break;

	case 4:
		addView("Quit");
		break;

	default:
		break;
	}
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
