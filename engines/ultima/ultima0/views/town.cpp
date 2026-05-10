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
//static const char *DONT_HAVE_THAT = "I'm Sorry We Don't have that.";
static const char *NOT_ENOUGH = "M'Lord thou can not afford that item.";
static const char *MAGES_CANT_USE = "I'm sorry, Mages can't use that.";
static const char *BYE = "Bye";

Town::Town() : Info("Town") {
}

bool Town::msgFocus(const FocusMessage &msg) {
	_message = WELCOME;
	g_engine->playMidi("shop.mid");
	return Info::msgFocus(msg);
}

bool Town::msgUnfocus(const UnfocusMessage &msg) {
	g_engine->stopMidi();
	return Info::msgUnfocus(msg);
}

void Town::draw() {
	Info::draw();

	// General message
	auto s = getSurface();
	s.writeString(Common::Point(1, 12), _message.empty() ? THANK_YOU : _message);
	s.writeString(Common::Point(1, 13), "Which item shallt thou buy");
	_message.clear();
}

void Town::selectObject(int item) {
	auto &player = g_engine->_player;
	const auto &obj = OBJECT_INFO[item];

	// Some things mages can't use
	if (player._class == 'M') {
		if (item == OB_BOW || item == OB_RAPIER) {
			_message = MAGES_CANT_USE;
			redraw();
			return;
		}
	}

	if (obj._cost > player._attr[AT_GOLD]) {
		_message = NOT_ENOUGH;

	} else {
		player._attr[AT_GOLD] -= obj._cost;	// Lose the money
		player._object[item] = MIN<int>(player._object[item] + (item == OB_FOOD ? 10 : 1), 999);
		_message = THANK_YOU;
	}

	redraw();
}

void Town::leave() {
	_message = BYE;
	delaySeconds(1);
	redraw();
}

void Town::timeout() {
	replaceView("WorldMap");
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
