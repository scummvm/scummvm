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

#include "ultima/ultima0/views/info.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

Info::Info(const char *viewName) : View(viewName) {
}

bool Info::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MENUS);
	return true;
}

bool Info::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return true;
}

void Info::draw() {
	const auto &player = g_engine->_player;
	auto s = getSurface();
	int i;

	s.clear();
	View::draw();

	// Stats
	for (i = 0; i < MAX_ATTR; ++i) {
		Common::String line = ATTRIB_NAMES[i];
		while (line.size() < 15)
			line += '.';
		s.writeString(Common::Point(0, 4 + i), line);
	}

	// Price/Damage/Item
	for (i = 0; i < MAX_OBJ; ++i) {
		s.writeString(Common::Point(5, 18 + i),
			Common::String::format("%d", OBJECT_INFO[i]._cost));

		if (i == 0) {
			s.writeString(" For 10");
			s.writeString(Common::Point(15, 18 + i), "N/A");
		} else if (i == 5) {
			s.writeString(Common::Point(15, 18 + i), "?????");
		} else {
			s.writeString(Common::Point(15, 18 + i),
				Common::String::format("1-%d", OBJECT_INFO[i]._maxDamage));
		}

		s.writeString(Common::Point(25, 18 + i), OBJECT_INFO[i]._name);
	}

	// Headers
	s.setColor(255, 0, 128);
	s.writeString(Common::Point(6, 2), "Stat's");
	s.writeString(Common::Point(21, 2), "Weapons");
	s.writeString(Common::Point(5, 16), "Price     Damage    Item");

	// Amounts
	s.setColor(C_VIOLET);
	for (i = 0; i < MAX_ATTR; ++i)
		s.writeString(Common::Point(15, 4 + i),
			Common::String::format("%d", player._attr[i]));
	for (i = 0; i < MAX_OBJ; ++i)
		s.writeString(Common::Point(21, 4 + i),
			Common::String::format("%4d-", (int)player._object[i]));
	s.writeString(Common::Point(18, 10), "Q-Quit");
}

void Info::leave() {
	const auto &player = g_engine->_player;
	replaceView(player._level == 0 ? "WorldMap" : "DungeonMap");
}

bool Info::msgAction(const ActionMessage &msg) {
	if (isDelayActive())
		return false;

	if (msg._action == KEYBIND_ESCAPE || msg._action == KEYBIND_QUIT) {
		leave();
		return true;
	}

	for (int i = 0; i < MAX_OBJ; ++i) {
		if (msg._action == OBJECT_INFO[i]._action) {
			selectObject(i);
			return true;
		}
	}

	return false;
}

bool Info::msgGame(const GameMessage &msg) {
	if (msg._name == "SELECTION") {
		selectObject(msg._value);
		return true;
	}

	return false;
}

/*-------------------------------------------------------------------*/

Info::InfoObject::InfoObject(Info *parent, const Common::Point &pt, int id,
		const Common::String &text) :
		UIElement("InfoObject", parent), _id(id), _text(text) {
	setBounds(Gfx::TextRect(pt.x, pt.y, pt.x + text.size(), pt.y));
}

void Info::InfoObject::draw() {
	auto s = getSurface();
	s.writeString(_text);
}

bool Info::InfoObject::msgMouseDown(const MouseDownMessage &msg) {
	_parent->send(GameMessage("SELECTION", _id));
	return true;
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
