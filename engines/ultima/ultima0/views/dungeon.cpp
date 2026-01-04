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
#include "ultima/ultima0/metaengine.h"
#include "ultima/ultima0/gfx/font.h"
#include "ultima/ultima0/gfx/dungeon.h"
#include "ultima/ultima0/data/monster_logic.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

Dungeon::Dungeon() : View("Dungeon") {
}

bool Dungeon::msgFocus(const FocusMessage &msg) {
	showMessage("");
	showLines("");
	MetaEngine::setKeybindingMode(KBMODE_DUNGEONS);

	if (!g_engine->isMidiPlaying())
		g_engine->playMidi("dungeon.mid");

	return true;
}

bool Dungeon::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return true;
}

void Dungeon::draw() {
	auto s = getSurface();
	s.clear();

	// Draw the dungeon view
	Graphics::ManagedSurface dungArea(s, Common::Rect(0, 0, s.w, s.h - Gfx::GLYPH_HEIGHT * 4));
	Gfx::Dungeon::draw(&dungArea);

	// Allow the status area to draw
	View::draw();

	if (g_engine->_showMinimap) {
		s.frameRect(Common::Rect(s.w - DUNGEON_MINIMAP_SIZE - 4, 0, s.w, DUNGEON_MINIMAP_SIZE + 4), C_GREY);
		s.frameRect(Common::Rect(s.w - DUNGEON_MINIMAP_SIZE - 3, 1, s.w - 1, DUNGEON_MINIMAP_SIZE + 3), C_GREY);
		Graphics::ManagedSurface minimapArea(s, Common::Rect(s.w - DUNGEON_MINIMAP_SIZE - 2, 2, s.w - 2, DUNGEON_MINIMAP_SIZE + 2));
		drawMinimap(minimapArea);
	}
}

void Dungeon::drawMinimap(Graphics::ManagedSurface &mapArea) {
	const auto &player = g_engine->_player;
	const auto &dungeon = g_engine->_dungeon;
	int tile;

	for (int y = 0; y < DUNGEON_MAP_SIZE; ++y) {
		for (int x = 0; x < DUNGEON_MAP_SIZE; ++x) {
			const Common::Rect r(x * MINIMAP_TILE_SIZE, y * MINIMAP_TILE_SIZE,
				(x + 1) * MINIMAP_TILE_SIZE, (y + 1) * MINIMAP_TILE_SIZE);
			if (x == player._dungeonPos.x && y == player._dungeonPos.y) {
				mapArea.fillRect(r, C_CYAN);
			} else if (dungeon.findMonster(Common::Point(x, y)) >= 0) {
				mapArea.fillRect(r, C_RED);
			} else {
				tile = dungeon._map[x][y];

				if (tile == DT_SPACE || tile == DT_DOOR || tile == DT_HIDDENDOOR)
					mapArea.fillRect(r, C_BLACK);
				else if (tile == DT_SOLID)
					mapArea.fillRect(r, C_WHITE);
				else if (tile == DT_LADDERUP || tile == DT_LADDERDN)
					mapArea.fillRect(r, C_GREEN);
				else
					mapArea.fillRect(r, C_ROSE);
			}
		}
	}
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
	case KEYBIND_PASS:
		showMessage("");
		break;
	case KEYBIND_ATTACK:
		showMessage(" \x9""Attack!");
		_status.draw();		// Render the message before we switch views
		addView("Attack");
		break;
	case KEYBIND_QUIT:
		// "Quit" in the original which merely saves the game. For ScummVM,
		// we open the GMM, allowing the user to either save or quit
		g_engine->openMainMenuDialog();
		return true;
	case KEYBIND_MINIMAP:
		g_engine->_showMinimap = !g_engine->_showMinimap;
		redraw();
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

	showMessage("Huh???");
	endOfTurn();
	return true;
}

void Dungeon::endOfTurn() {
	auto &player = g_engine->_player;
	auto &dungeon = g_engine->_dungeon;

	if (player._attr[AT_HP] <= 0) {
		g_engine->stopMidi();
		replaceView("Dead");
		return;
	}

	// Check for monster attacks
	MonsterLogic::checkForAttacks(player, dungeon);

	player._object[OB_FOOD] = MAX(player._object[OB_FOOD] - 0.1, 0.0);
	if (player._object[OB_FOOD] == 0) {
		showMessage("You have starved...");
		g_engine->stopMidi();
		delaySeconds(1);
	}

	redraw();
}

void Dungeon::moveForward() {
	auto &dungeon = g_engine->_dungeon;
	auto &player = g_engine->_player;
	Common::Point newPos = player._dungeonPos + player._dungeonDir;

	if (!ISWALKTHRU(dungeon._map[newPos.x][newPos.y]) || dungeon.findMonster(newPos) >= 0)
		return;

	// Set new position
	player._dungeonPos = newPos;

	// What's here ?
	int n = dungeon._map[player._dungeonPos.x][player._dungeonPos.y];

	if (n == DT_PIT) {
		// Fell in a pit
		player._level++;					// Down a level
		showMessage("Aaarrrgghhh! A Trap !");
		showLines(Common::String::format("Falling to Level %d.", player._level));

		player._attr[AT_HP] -= (3 + urand() % (3 * player._level));
		dungeon.create(player);		// Create the new level
	} else if (n == DT_GOLD) {
		// Gold here
		// Remove the gold
		dungeon._map[player._dungeonPos.x][player._dungeonPos.y] = DT_SPACE;
		int gold = (urand() % (5 * player._level)) + player._level;	// Calculate amount

		showMessage("Gold !!!!!");
		Common::String msg = Common::String::format("%d pieces of eight ", gold);
		player._attr[AT_GOLD] = MIN<int>(player._attr[AT_GOLD] + gold, 9999);	// Add to total

		if (gold > 0) {
			int objNum = urand() % MAX_OBJ;		// Decide which object
			const char *name = OBJECT_INFO[objNum]._name;
			const char *prefix = "a";			// Decide a,an or some
			if (strchr("aeiou", tolower(*name)))
				prefix = "an";
			if (objNum == 0)
				prefix = "some";

			msg += Common::String::format("\nand %s %s.", prefix, name);
			player._object[objNum] = MIN<int>(player._object[objNum] + 1, 9999);	// Bump the total
		}

		showLines(msg);
	}
}

void Dungeon::interact() {
	auto &dungeon = g_engine->_dungeon;
	auto &player = g_engine->_player;

	// Identify what's there
	int t = dungeon._map[player._dungeonPos.x][player._dungeonPos.y];
	bool done = false;

	if (t == DT_LADDERUP) {
		// Climbing up a ladder
		player._level--;
		done = true;

		if (player._level == 0) {
			showMessage("Leave Dungeon.");
			if (player._hpGain > 0)
				showLines(Common::String::format("Thou has gained %d HP", player._hpGain));
			player._attr[AT_HP] += player._hpGain;
			player._hpGain = 0;
			delaySeconds(1);	// Brief delay to show text before leaving dungeon

		} else {
			showMessage("Use Ladder");
			showLines(Common::String::format("Go up to Level %d.", player._level));
		}
	} else if (t == DT_LADDERDN) {
		// Climbing down a ladder
		player._level++;
		done = true;

		showMessage("Use Ladder");
		showLines(Common::String::format("Go down to Level %d.\n", player._level));
	}

	if (done) {
		if (player._level > 0)
			// New Dungeon Map Required
			dungeon.create(player);
	} else {
		showMessage("Huh???");
	}
}

void Dungeon::timeout() {
	const auto &player = g_engine->_player;
	g_engine->stopMidi();

	replaceView((player._level == 0) ? "WorldMap" : "Dead");
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
