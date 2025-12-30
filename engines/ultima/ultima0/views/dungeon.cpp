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

#include "ultima/ultima0/views/dungeon.h"
#include "ultima/ultima0/ultima0.h"
#include "ultima/ultima0/gfx/font.h"
#include "ultima/ultima0/gfx/dungeon.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

Dungeon::Dungeon() : View("Dungeon") {
}

bool Dungeon::msgFocus(const FocusMessage &msg) {
	return true;
}

void Dungeon::draw() {
	auto s = getSurface();
	s.clear();

	// Draw the dungeon view
	Graphics::ManagedSurface dungArea(s, Common::Rect(0, 0, s.w, s.h - Gfx::CHAR_HEIGHT * 4));
	Gfx::Dungeon::draw(&dungArea);

	// Allow the status area to draw
	View::draw();
}

bool Dungeon::msgAction(const ActionMessage &msg) {
	if (isDelayActive())
		return false;
#ifdef TODO
	switch (msg._action) {
	case KEYBIND_UP:
		showMessage("North");
		move(0, -1);
		break;
	case KEYBIND_DOWN:
		showMessage("South");
		move(0, 1);
		break;
	case KEYBIND_LEFT:
		showMessage("West");
		move(-1, 0);
		break;
	case KEYBIND_RIGHT:
		showMessage("East");
		move(1, 0);
		break;
	case KEYBIND_INFO:
		// Show character info screen
		showMessage("");
		replaceView("Info");
		break;
	case KEYBIND_ENTER:
	case KEYBIND_SELECT:
		enter();
		break;
	default:
		showMessage("");
		break;
	}
#endif
	endOfTurn();
	return true;
}

bool Dungeon::msgKeypress(const KeypressMessage &msg) {
	if (isDelayActive())
		return false;

	switch (msg.keycode) {
	case Common::KEYCODE_q:
		// "Quit" in the original which merely saves the game. For ScummVM,
		// we open the GMM, allowing the user to either save or quit
		g_engine->openMainMenuDialog();
		return true;

	default:
		break;
	}

	endOfTurn();
	return true;
}

void Dungeon::endOfTurn() {
#ifdef TODO
	auto &player = g_engine->_player;

	if (player.Attr[AT_HP] <= 0)
		replaceView("Dead");

	player.Object[OB_FOOD] = MAX(player.Object[OB_FOOD] - 1.0, 0.0);
	if (player.Object[OB_FOOD] == 0) {
		showMessage("You have starved...");
		delaySeconds(1);
	}
#endif
}

void Dungeon::timeout() {
	auto &player = g_engine->_player;

	if (player.Attr[AT_HP] <= 0 || player.Object[OB_FOOD] <= 0) {
		// Timeout from displaying player was killed
		replaceView("Dead");
	} else {
		// TODO: timeout actions
	}
}

void Dungeon::move(int xi, int yi) {
#ifdef TODO
	auto &player = g_engine->_player;
	auto &map = g_engine->_worldMap;

	// Calculate new position
	int x1 = player.World.x + xi;
	int y1 = player.World.y + yi;

	if (map.read(x1, y1) == WT_MOUNTAIN) {
		showMessage("You can't pass the mountains.");
	} else {
		// Move
		player.World.x = x1;
		player.World.y = y1;
		redraw();
	}
#endif
}

void Dungeon::interact() {
#ifdef TODO
	const auto &player = g_engine->_player;
	const auto &map = g_engine->_worldMap;

	int t = map.read(player.World.x, player.World.y);
	switch (t) {
	case WT_TOWN:
		showMessage("Enter Town.");
		delaySeconds(1);
		break;
	case WT_DUNGEON:
		showMessage("Enter Dungeon.");
		delaySeconds(1);
		break;
	case WT_BRITISH:
		showMessage("Enter Castle.");
		delaySeconds(1);
		break;
	default:
		// Nope....
		showMessage("Huh???");
		break;
	}
#endif
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
