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

#include "ultima/ultima1/views/overworld.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

Overworld::Overworld() : View("Overworld") {
	_map.setBounds(TextRect(0, 0, 40, 20));
	_commands.setBounds(TextRect(0, 20, 30, 24));
	_stats.setBounds(TextRect(30, 20, 40, 24));
}

void Overworld::draw() {
	// Clear map tiles so entire map is redrawn
	auto &map = g_engine->_map;
	map.clearTiles();

	// Allow the child controls to draw
	return View::draw();
}

bool Overworld::msgFocus(const FocusMessage &msg) {
	const auto &player = g_engine->_player;
	auto &map = g_engine->_map;
	map.load(player._mapNum);

	return View::msgFocus(msg);
}

bool Overworld::msgGame(const GameMessage &msg) {
	const auto &player = g_engine->_player;

	if (msg._name == "COMMAND") {
		// Scroll the command area, and pause briefly so users can tell
		// when multiple commands in sequence are happening
		_mode = kModePreCommand;
		_commands.writeString("\n");
		delayFrames(1);
		return true;
	} else if (msg._name == "INFO") {
		_commands.writeString(msg._stringValue);
		return true;
	} else if (msg._name == "DEAD") {
		// Update the stats view immediately to reflect the deadness
		_stats.draw();

		// Add message to the commands view that the player died
		_commands.writeString("\0x8d\0x8d\0x8d");
		_commands.writeString(player._name);
		_commands.writeString(" IS DEAD!");

		// Switch to the invisible Dead view to freeze the screen
		g_engine->replaceView("Dead");
		return true;
	} else if (msg._name == "CIRCLE") {
		if (msg._stringValue == "UP")
			_map.flashCircle(0, -1);
		else if (msg._stringValue == "DOWN")
			_map.flashCircle(0, 1);
		else if (msg._stringValue == "LEFT")
			_map.flashCircle(-1, 0);
		else if (msg._stringValue == "RIGHT")
			_map.flashCircle(1, 0);
		else
			_map.flashCircle(0, 0);
		return true;
	}

	return View::msgGame(msg);
}

void Overworld::timeout() {
	switch (_mode) {
	case kModePreCommand:
		// Display new command prompt
		_mode = kModeCommand;
		_commands.prompt();
		delayFrames(Data::FRAMES_BEFORE_COMMAND_TIMEOUT);
		break;
	case kModeCommand:
		// Timeout happened, so automatically select pass command
		g_engine->_game.doCommand(KEYBIND_PASS);
		break;
	default:
		break;
	}
}

bool Overworld::msgAction(const ActionMessage &msg) {
	auto &game = g_engine->_game;

	// Stop command timeout
	cancelDelay();

	if (_mode == kModeAttackDirection) {
		// TODO
	} else {
		game.doCommand(msg._action);
	}

	return true;
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
