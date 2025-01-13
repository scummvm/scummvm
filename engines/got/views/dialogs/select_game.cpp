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

#include "got/views/dialogs/select_game.h"
#include "got/game/init.h"
#include "got/got.h"

namespace Got {
namespace Views {
namespace Dialogs {

static const char *OPTIONS[] = {
	"Part 1: Serpent Surprise!",
	"Part 2: Non-stick Nognir",
	"Part 3: Lookin' for Loki",
	nullptr
};

SelectGame::SelectGame() : SelectOption("SelectGame", "Play Which Game?", OPTIONS) {
}

void SelectGame::selected() {
	// Select the game area to start from
	g_vars->setArea(_selectedItem + 1);

	// Switch to the story view for the selected game area
	initialize_game();
	replaceView("Story", true, true);
}

void SelectGame::closed() {
	addView("MainMenu");
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
