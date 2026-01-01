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
	showMessage("");
	showLines("");
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
	auto &player = g_engine->_player;

	if (isDelayActive())
		return false;

	switch (msg._action) {
	case KEYBIND_UP:
		showMessage("Move Forward");
		moveForward();
		break;
	case KEYBIND_DOWN:
		showMessage("Turn Around");
		player.dungeonTurnLeft();
		player.dungeonTurnLeft();
		break;
	case KEYBIND_LEFT:
		showMessage("Turn Left");
		player.dungeonTurnLeft();
		break;
	case KEYBIND_RIGHT:
		showMessage("Turn Right");
		player.dungeonTurnRight();
		break;
	case KEYBIND_INFO:
		// Show character info screen
		showMessage("");
		replaceView("Info");
		break;
	case KEYBIND_ENTER:
		interact();
		break;
	default:
		showMessage("Huh???");
		break;
	}

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
		showMessage("Huh???");
		break;
	}

	endOfTurn();
	return true;
}

void Dungeon::endOfTurn() {
	auto &player = g_engine->_player;

	if (player.Attr[AT_HP] <= 0)
		replaceView("Dead");

	player.Object[OB_FOOD] = MAX(player.Object[OB_FOOD] - 0.1, 0.0);
	if (player.Object[OB_FOOD] == 0) {
		showMessage("You have starved...");
		delaySeconds(1);
	}

	redraw();
}

void Dungeon::moveForward() {
	auto &dungeon = g_engine->_dungeon;
	auto &player = g_engine->_player;
	COORD New = player.Dungeon + player.DungDir;

	if (!ISWALKTHRU(dungeon.Map[New.x][New.y]) || dungeon.findMonster(New) >= 0)
		return;

	// Set new position
	player.Dungeon = New;

	// What's here ?
	int n = dungeon.Map[player.Dungeon.x][player.Dungeon.y];

	if (n == DT_PIT) {
		// Fell in a pit
		player.Level++;					// Down a level
		showMessage("Aaarrrgghhh! A Trap !");
		showLines(Common::String::format("Falling to Level %d.", player.Level));

		player.Attr[AT_HP] -= (3 + urand() % (3 * player.Level));
		dungeon.create(player);		// Create the new level
	} else if (n == DT_GOLD) {
		// Gold here
		// Remove the gold
		dungeon.Map[player.Dungeon.x][player.Dungeon.y] = DT_SPACE;
		int gold = (urand() % (5 * player.Level)) + player.Level;	// Calculate amount

		showMessage("Gold !!!!!");
		Common::String msg = Common::String::format("%d pieces of eight ", gold);
		player.Attr[AT_GOLD] = MIN<int>(player.Attr[AT_GOLD] + gold, 9999);	// Add to total

		if (gold > 0) {
			int objNum = urand() % MAX_OBJ;		// Decide which object
			const char *name = OBJECT_INFO[objNum].Name;
			const char *prefix = "a";			// Decide a,an or some
			if (strchr("aeiou", tolower(*name)))
				prefix = "an";
			if (objNum == 0)
				prefix = "some";

			msg += Common::String::format("\nand %s %s.", prefix, name);
			player.Object[objNum] = MIN<int>(player.Object[objNum] + 1, 9999);	// Bump the total
		}

		showLines(msg);
	}
}

void Dungeon::interact() {
	auto &dungeon = g_engine->_dungeon;
	auto &player = g_engine->_player;

	// Identify what's there
	int t = dungeon.Map[player.Dungeon.x][player.Dungeon.y];
	bool done = false;

	if (t == DT_LADDERUP) {
		// Climbing up a ladder
		player.Level--;
		done = true;

		if (player.Level == 0) {
			showMessage("Leave Dungeon.");
			if (player.HPGain > 0)
				showLines(Common::String::format("Thou has gained %d HP", player.HPGain));
			player.Attr[AT_HP] += player.HPGain;
			player.HPGain = 0;
			delaySeconds(1);	// Brief delay to show text before leaving dungeon

		} else {
			showMessage("Use Ladder");
			showLines(Common::String::format("Go up to Level %d.", player.Level));
		}
	} else if (t == DT_LADDERDN) {
		// Climbing down a ladder
		player.Level++;
		done = true;

		showMessage("Use Ladder");
		showLines(Common::String::format("Go down to Level %d.\n", player.Level));
	}

	if (done) {
		if (player.Level > 0)
			// New Dungeon Map Required
			dungeon.create(player);
	} else {
		showMessage("Huh???");
	}
}

void Dungeon::timeout() {
	const auto &player = g_engine->_player;

	replaceView((player.Level == 0) ? "WorldMap" : "Dead");
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
