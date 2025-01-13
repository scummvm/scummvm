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
#include "got/game/init.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

static const char *OPTIONS[] = {"Play Game", "Load Game", "High Scores", "Credits", "Demo", "Quit", nullptr};
static const char *OPTIONS_NO_DEMO[] = {"Play Game", "Load Game", "High Scores", "Credits", "Quit", nullptr};

MainMenu::MainMenu() : SelectOption("MainMenu", "God of Thunder Menu",
									gDebugLevel > 0 ? OPTIONS : OPTIONS_NO_DEMO) {
}

bool MainMenu::msgFocus(const FocusMessage &msg) {
	g_vars->resetEndgameFlags();
	return SelectOption::msgFocus(msg);
}

void MainMenu::closed() {
	_selectedItem = 4; // Quit game
	selected();
}

void MainMenu::selected() {
	switch (_selectedItem) {
	case 0:
		_G(demo) = false;
		addView("SelectGame");
		break;

	case 1:
		if (!g_engine->loadGameDialog())
			addView("SelectGame");
		break;

	case 2:
		replaceView("HighScores", true, true);
		break;

	case 3:
		addView("Credits");
		break;

	case 4:
		if (gDebugLevel > 0) {
			_G(demo) = true;
			initialize_game();
			replaceView("PartTitle", true, true);
		} else {
			addView("Quit");
		}
		break;

	case 5:
		addView("Quit");
		break;

	default:
		break;
	}
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
