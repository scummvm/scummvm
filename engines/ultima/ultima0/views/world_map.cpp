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

#include "ultima/ultima0/views/world_map.h"
#include "ultima/ultima0/ultima0.h"
#include "ultima/ultima0/metaengine.h"
#include "ultima/ultima0/gfx/font.h"
#include "ultima/ultima0/gfx/map.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

WorldMap::WorldMap() : View("WorldMap") {
}

bool WorldMap::msgFocus(const FocusMessage &msg) {
	showMessage("");
	MetaEngine::setKeybindingMode(KBMODE_OVERWORLD);

	if (!g_engine->isMidiPlaying())
		g_engine->playMidi("over.mid");

	return true;
}

bool WorldMap::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return true;
}

void WorldMap::draw() {
	auto s = getSurface();

	// Draw the map
	Graphics::ManagedSurface mapArea(s, Common::Rect(0, 0, s.w, s.h - Gfx::GLYPH_HEIGHT * 4));
	Gfx::Map::draw(&mapArea);

	// Allow the status area to draw
	View::draw();

	if (g_engine->_showMinimap) {
		s.frameRect(Common::Rect(s.w - WORLD_MINIMAP_SIZE - 4, 0, s.w, WORLD_MINIMAP_SIZE + 4), C_GREY);
		s.frameRect(Common::Rect(s.w - WORLD_MINIMAP_SIZE - 3, 1, s.w - 1, WORLD_MINIMAP_SIZE + 3), C_GREY);
		Graphics::ManagedSurface minimapArea(s, Common::Rect(s.w - WORLD_MINIMAP_SIZE - 2, 2, s.w - 2, WORLD_MINIMAP_SIZE + 2));
		Gfx::Map::draw(&minimapArea, true);
	}
}

bool WorldMap::msgAction(const ActionMessage &msg) {
	if (isDelayActive())
		return false;

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
		enter();
		break;
	case KEYBIND_PASS:
		showMessage("");
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
		showMessage("");
		break;
	}

	endOfTurn();
	return true;
}

bool WorldMap::msgKeypress(const KeypressMessage &msg) {
	if (isDelayActive())
		return false;

	endOfTurn();
	return true;
}

void WorldMap::endOfTurn() {
	auto &player = g_engine->_player;

	if (player._attr[AT_HP] <= 0) {
		g_engine->stopMidi();
		replaceView("Dead");

	} else {
		player._object[OB_FOOD] = MAX(player._object[OB_FOOD] - 1.0, 0.0);
		if (player._object[OB_FOOD] == 0) {
			showMessage("You have starved...");
			delaySeconds(1);
		}
	}
}

void WorldMap::timeout() {
	const auto &map = g_engine->_worldMap;
	auto &player = g_engine->_player;
	auto &dungeon = g_engine->_dungeon;

	g_engine->stopMidi();

	if (player._attr[AT_HP] <= 0 || player._object[OB_FOOD] <= 0) {
		// Timeout from displaying player was killed
		replaceView("Dead");
	} else {
		// Otherwise a timeout from entering a location
		int t = map.read(player._worldPos.x, player._worldPos.y);
		switch (t) {
		case WT_TOWN:
			replaceView("Town");
			break;
		case WT_DUNGEON:
			player._level = 1;				// Go to level 1
			player._dungeonPos.x = 1;			// Set initial position
			player._dungeonPos.y = 1;
			player._dungeonDir.x = 1;			// And direction
			player._dungeonDir.y = 0;
			dungeon.create(player);

			replaceView("Dungeon");
			break;
		case WT_BRITISH:
			replaceView("Castle");
			break;
		default:
			break;
		}
	}
}

void WorldMap::move(int xi, int yi) {
	auto &player = g_engine->_player;
	auto &map = g_engine->_worldMap;

	// Calculate new position
	int x1 = player._worldPos.x + xi;
	int y1 = player._worldPos.y + yi;

	if (map.read(x1, y1) == WT_MOUNTAIN) {
		showMessage("You can't pass the mountains.");
	} else {
		// Move
		player._worldPos.x = x1;
		player._worldPos.y = y1;
		redraw();
	}
}

void WorldMap::enter() {
	const auto &player = g_engine->_player;
	const auto &map = g_engine->_worldMap;

	int t = map.read(player._worldPos.x, player._worldPos.y);
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
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
