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

#include "got/views/dialogs/options_menu.h"
#include "got/game/back.h"
#include "got/vars.h"

namespace Got {
namespace Views {
namespace Dialogs {

static const char *OPTIONS[] = {"Sound/Music", "Skill Level", "Save Game", "Load Game", "Die", "Turbo Mode", "Help", "Quit", nullptr};

OptionsMenu::OptionsMenu() : SelectOption("OptionsMenu", "Options Menu", OPTIONS) {
}

void OptionsMenu::selected() {
	switch (_selectedItem) {
	case 0:
		addView("SetSound");
		break;
	case 1:
		addView("SkillLevel");
		break;
	case 2:
		g_engine->saveGameDialog();
		break;
	case 3:
		g_engine->loadGameDialog();
		break;
	case 4:
		g_events->send(GameMessage("THOR_DIES"));
		break;
	case 5:
		addView("SelectSlow");
		break;
	case 6:
		// Help
		odinSpeaks(2008, -1);
		break;
	case 7:
		addView("QuitGame");
		break;
	default:
		break;
	}
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
