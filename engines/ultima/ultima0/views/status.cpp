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

#include "ultima/ultima0/views/status.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

Status::Status(const Common::String &name, UIElement *parent) : View(name, parent) {
	setBounds(Common::Rect(0, DEFAULT_SCY - 4 * Gfx::GLYPH_HEIGHT, DEFAULT_SCX, DEFAULT_SCY));
}

bool Status::msgFocus(const FocusMessage &msg) {
	_message.clear();
	return View::msgFocus(msg);
}

void Status::draw() {
	const auto &player = g_engine->_player;
	auto s = getSurface();
	s.clear();

	if (!_message.empty())
		s.writeString(Common::Point(1, 0), _message);

	s.writeString(Common::Point(28, 1), "Food=");
	s.writeString(Common::Point(28, 2), "H.P.=");
	s.writeString(Common::Point(28, 3), "Gold=");
	s.setColor(C_VIOLET);
	s.writeString(Common::Point(33, 1), Common::String::format("%d", (int)player._object[OB_FOOD]));
	s.writeString(Common::Point(33, 2), Common::String::format("%d", player._attr[AT_HP]));
	s.writeString(Common::Point(33, 3), Common::String::format("%d", player._attr[AT_GOLD]));
}

bool Status::msgGame(const GameMessage &msg) {
	if (msg._name == "MSG") {
		_message = msg._stringValue;
		redraw();
		return true;
	}

	return false;
}

/*-------------------------------------------------------------------*/

bool DungeonStatus::msgFocus(const FocusMessage &msg) {
	_lines.clear();
	return Status::msgFocus(msg);
}

void DungeonStatus::draw() {
	Status::draw();

	const auto &player = g_engine->_player;
	auto s = getSurface();

	// Display the current direction
	if (player._level > 0)
		s.writeString(Common::Point(15, 0), DIRECTION_NAMES[player.dungeonDir()]);

	// Draw any extra lines
	for (uint i = 0; i < _lines.size(); ++i)
		s.writeString(Common::Point(1, 1 + i), _lines[i]);
}

bool DungeonStatus::msgGame(const GameMessage &msg) {
	if (msg._name == "LINES") {
		_lines.clear();

		Common::String str = msg._stringValue;
		uint p;
		while ((p = str.findFirstOf('\n')) != Common::String::npos) {
			_lines.push_back(Common::String(str.c_str(), str.c_str() + p));
			str = Common::String(str.c_str() + p + 1);
		}
		if (!str.empty())
			_lines.push_back(str);

		redraw();
		return true;

	} else {
		if (msg._name == "MSG")
			// Changing message also resets any message lines
			_lines.clear();

		return Status::msgGame(msg);
	}
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
