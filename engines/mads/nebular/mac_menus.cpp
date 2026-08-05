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

#include "common/config-manager.h"
#include "common/translation.h"
#include "gui/chooser.h"
#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/kernel.h"
#include "mads/mads.h"
#include "mads/nebular/mac_menus.h"

namespace MADS {
namespace RexNebular {

void selectMacintoshDifficulty() {
	const int configuredDifficulty = ConfMan.getInt("difficulty");
	if (configuredDifficulty >= DIFFICULTY_HARD && configuredDifficulty <= DIFFICULTY_EASY) {
		game.difficulty = configuredDifficulty;
		return;
	}

	Common::U32StringArray choices;
	choices.push_back(_("Novice - Easy"));
	choices.push_back(_("Advanced - Difficult"));
	choices.push_back(_("Expert - Very Difficult"));
	// The generic browser layout is tied to the launcher's game-list widget.
	// Use its screen-based counterpart because this chooser runs in-engine.
	GUI::ChooserDialog dialog(_("Select a Difficulty Level:"), "FileBrowser");
	dialog.setList(choices);

	switch (dialog.runModal()) {
	case 0:
		game.difficulty = DIFFICULTY_EASY;
		break;
	case 1:
		game.difficulty = DIFFICULTY_MEDIUM;
		break;
	case 2:
		game.difficulty = DIFFICULTY_HARD;
		break;
	default:
		game.difficulty = DIFFICULTY_MEDIUM;
		break;
	}
}

void macintoshGameMenu() {
	g_engine->flushKeys();

	if (kernel.activate_menu == GAME_DIFFICULTY_MENU)
		selectMacintoshDifficulty();
	else if (kernel.activate_menu != GAME_NO_MENU)
		g_engine->openMainMenuDialog();

	kernel.activate_menu = GAME_NO_MENU;
}

} // namespace RexNebular
} // namespace MADS
