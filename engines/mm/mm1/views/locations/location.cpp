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

#include "mm/mm1/views/locations/location.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

#define DISPLAY_SECONDS 5

Location::Location(const Common::String &name) : TextView(name) {
	_bounds = getLineBounds(17, 24);
	_modeString = STRING["dialogs.location.gather"];
}

bool Location::msgGame(const GameMessage &msg) {
	if (msg._name == "DISPLAY") {
		addView();
		return true;
	}

	return TextView::msgGame(msg);
}

void Location::draw() {
	clearSurface();
	writeString(0, 0, g_globals->_currCharacter->_name);
	newLine();
	writeString(STRING["dialogs.location.gold"]);
	writeNumber(g_globals->_currCharacter->_gold);
	newLine();
	newLine();
	writeString(_modeString);
	writeString(0, 6, STRING["dialogs.misc.go_back"]);
}

void Location::displayMessage(int x, const Common::String &msg) {
	clearLines(3, 7);
	writeString(x, 5, msg);
	delaySeconds(DISPLAY_SECONDS);
}

void Location::newLine() {
	_textPos.x = 0;
	if (++_textPos.y >= 24)
		_textPos.y = 0;
}

bool Location::subtractGold(uint amount) {
	if (g_globals->_currCharacter->_gold < amount) {
		notEnoughGold();
		return false;
	} else {
		g_globals->_currCharacter->_gold -= amount;
		return true;
	}
}

void Location::notEnoughGold() {
	Sound::sound(SOUND_2);
	displayMessage(STRING["dialogs.misc.not_enough_gold"]);
}

void Location::backpackFull() {
	Sound::sound(SOUND_2);
	displayMessage(STRING["dialogs.misc.backpack_full"]);
}

void Location::changeCharacter(uint index) {
	if (index >= g_globals->_party.size())
		return;

	g_globals->_currCharacter = &g_globals->_party[index];
	redraw();
}

void Location::leave() {
	g_maps->turnAround();
	close();
	g_events->redraw();
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
