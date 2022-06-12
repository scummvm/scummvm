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

#include "mm/mm1/views/businesses/business.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Businesses {

#define DISPLAY_TIMEOUT (5 * FRAME_RATE)

Business::Business(const Common::String &name) : TextView(name) {
	_bounds = getLineBounds(17, 24);
	_modeString = STRING["dialogs.business.gather"];
}

void Business::draw() {
	clearSurface();
	writeString(0, 0, g_globals->_currCharacter->_name);
	newLine();
	writeString(STRING["dialogs.business.gold"]);
	writeNumber(g_globals->_currCharacter->_gold);
	newLine();
	newLine();
	writeString(_modeString);
	writeString(0, 6, STRING["dialogs.misc.go_back"]);
}

void Business::displayMessage(int x, const Common::String &msg) {
	clearLines(3, 7);
	writeString(x, 5, msg);
	_timeoutCtr = DISPLAY_TIMEOUT;
}

void Business::newLine() {
	_textPos.x = 0;
	if (++_textPos.y >= 24)
		_textPos.y = 0;
}

void Business::gatherGold() {
	uint total = 0;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		total += g_globals->_party[i]._gold;
		g_globals->_party[i]._gold = 0;
	}

	g_globals->_currCharacter->_gold = total;
}

bool Business::subtractGold(uint amount) {
	if (g_globals->_currCharacter->_gold < amount) {
		notEnoughGold();
		return false;
	} else {
		g_globals->_currCharacter->_gold -= amount;
		return true;
	}
}

void Business::notEnoughGold() {
	Sound::sound(SOUND_2);
	displayMessage(STRING["dialogs.misc.not_enough_gold"]);
}

void Business::changeCharacter(uint index) {
	if (index >= g_globals->_party.size())
		return;

	g_globals->_currCharacter = &g_globals->_party[index];
	redraw();
}

void Business::leave() {
	SWAP(g_maps->_forwardMask, g_maps->_backwardsMask);
	SWAP(g_maps->_leftMask, g_maps->_rightMask);
	SWAP(g_maps->_forwardOffset, g_maps->_backwardsOffset);
	SWAP(g_maps->_leftOffset, g_maps->_rightOffset);

	close();
	g_events->redraw();
}

bool Business::tick() {
	if (_timeoutCtr && --_timeoutCtr == 0) {
		redraw();
	}

	return TextView::tick();
}

} // namespace Businesses
} // namespace Views
} // namespace MM1
} // namespace MM
