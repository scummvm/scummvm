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

#include "ultima/ultima0/views/town.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

static const char *WELCOME = "Welcome to the Adventure Shop";
static const char *THANK_YOU = "Thank you m'lord";
static const char *DONT_HAVE_THAT = "I'm Sorry We Don't have that.";
static const char *NOT_ENOUGH = "M'Lord thou can not afford that item.";
static const char *MAGES_CANT_USE = "I'm sorry, Mages can't use that.";
static const char *BYE = "Bye";

Town::Town() : View("Town") {
}

bool Town::msgFocus(const FocusMessage &msg) {
	_message = WELCOME;
	return true;
}

void Town::draw() {
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

	// General message
	s.writeString(Common::Point(1, 12), _message.empty() ? THANK_YOU : _message);
	s.writeString(Common::Point(1, 13), "Which item shallt thou buy");
	_message.clear();

	// Price/Damage/Item
	for (i = 0; i < MAX_OBJ; ++i) {
		s.writeString(Common::Point(5, 18 + i),
			Common::String::format("%d", OBJECT_INFO[i].Cost));

		if (i == 0) {
			s.writeString(" For 10");
			s.writeString(Common::Point(15, 18 + i), "N/A");
		} else if (i == 5) {
			s.writeString(Common::Point(15, 18 + i), "?????");
		} else {
			s.writeString(Common::Point(15, 18 + i),
				Common::String::format("1-%d", OBJECT_INFO[i].MaxDamage));
		}

		s.writeString(Common::Point(25, 18 + i), OBJECT_INFO[i].Name);
	}

	// Headers
	s.setColor(255, 0, 128);
	s.writeString(Common::Point(6, 2), "Stat's");
	s.writeString(Common::Point(21, 2), "Weapons");
	s.writeString(Common::Point(5, 16), "Price     Damage    Item");

	// Amounts
	s.setColor(C_PURPLE);
	for (i = 0; i < MAX_ATTR; ++i)
		s.writeString(Common::Point(15, 4 + i),
			Common::String::format("%d", player.Attr[i]));
	for (i = 0; i < MAX_OBJ; ++i)
		s.writeString(Common::Point(22, 4 + i),
			Common::String::format("%3d-", (int)player.Object[i]));
	s.writeString(Common::Point(18, 10), "Q-Quit");
}

bool Town::msgKeypress(const KeypressMessage &msg) {
	if (isDelayActive())
		return false;

	for (int i = 0; i < MAX_OBJ; ++i) {
		if (toupper(msg.ascii) == OBJECT_INFO[i].Key ||
				(Common::isDigit(msg.ascii) && (msg.ascii - '0') == OBJECT_INFO[i].Cost)) { 
			selectObject(i);
			return true;
		}
	}

	if (msg.keycode == Common::KEYCODE_q) {
		_message = BYE;
		delaySeconds(1);
		redraw();
	}

	return true;
}

bool Town::msgAction(const ActionMessage &msg) {
	if (isDelayActive())
		return false;

	if (msg._action == KEYBIND_ESCAPE) {
		_message = BYE;
		delaySeconds(1);
		redraw();
		return true;
	}

	return false;
}

bool Town::msgGame(const GameMessage &msg) {
	if (msg._name == "SELECTION") {
		selectObject(msg._value);
		return true;
	}

	return false;
}

void Town::selectObject(int item) {
	auto &player = g_engine->_player;
	const auto &obj = OBJECT_INFO[item];

	// Some things mages can't use
	if (player.Class == 'M') {
		if (item == OB_BOW || item == OB_RAPIER) {
			_message = MAGES_CANT_USE;
			redraw();
			return;
		}
	}

	if (obj.Cost > player.Attr[AT_GOLD]) {
		_message = NOT_ENOUGH;

	} else {
		player.Attr[AT_GOLD] -= obj.Cost;	// Lose the money
		player.Object[item] = MIN<int>(player.Object[item] + (item == OB_FOOD ? 10 : 1), 999);
		_message = THANK_YOU;
	}

	redraw();
}

void Town::timeout() {
	replaceView("WorldMap");
}

/*-------------------------------------------------------------------*/

Town::TitleOption::TitleOption(Town *parent, const Common::Point &pt, int id,
		const Common::String &text) :
		UIElement("TitleOption", parent), _id(id), _text(text) {
	setBounds(Gfx::TextRect(pt.x, pt.y, pt.x + text.size(), pt.y));
}

void Town::TitleOption::draw() {
	auto s = getSurface();
	s.writeString(_text);
}

bool Town::TitleOption::msgMouseDown(const MouseDownMessage &msg) {
	_parent->send(GameMessage("SELECTION", _id));
	return true;
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
