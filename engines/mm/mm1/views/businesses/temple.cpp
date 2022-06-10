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

#include "mm/mm1/views/businesses/temple.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Businesses {

static const uint16 HEAL_COST1[5] = { 2000, 5000, 5000, 2000, 8000 };
static const uint16 HEAL_COST2[5] = { 200, 500, 500, 200, 1000 };
static const uint16 HEAL_COST3[5] = { 25, 50, 50, 25, 100 };
static const uint16 UNCURSE_COST[5] = { 500, 1000, 1000, 1012, 1500 };
static const uint16 ALIGNMENT_COST[5] = { 250, 200, 200, 200, 250 };
static const uint16 DONATE_COST[5] = { 100, 100, 100, 25, 200 };

Temple::Temple() : Business("Temple") {
	_modeString = STRING["dialogs.business.gather"];
}

bool Temple::msgFocus(const FocusMessage &msg) {
	g_events->msgBusiness(BusinessMessage(2));
	selectCharacter(0);

	return true;
}

bool Temple::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_ESCAPE:
		leave();
		break;
	default:
		break;
	}

	return true;
}

void Temple::selectCharacter(uint charIndex) {
	Maps::Map &map = *g_maps->_currentMap;
	int i;

	g_globals->_currCharacter = &g_globals->_party[charIndex];
	_isEradicated = false;

	int townNum = map[0];
	if (townNum < 1 || townNum >= 6)
		townNum = 1;
	--townNum;
	/*
	_healCost = HEAL_COST[townNum];
	_uncurseCost = UNCURSE_COST[townNum];
	_donateCost = DONATE_COST[townNum];
	*/

	Character &c = *g_globals->_currCharacter;

	_healCost = 0;
	if (c._condition == ERADICATED) {
		_healCost = HEAL_COST1[townNum];
		_isEradicated = true;
	} else if (c._condition & BAD_CONDITION) {
		_healCost = HEAL_COST2[townNum];
	} else if (c._condition || c._hp < c._hpMax) {
		_healCost = HEAL_COST3[townNum];
	}

	_uncurseCost = UNCURSE_COST[townNum];
	for (i = 0;  i < INVENTORY_COUNT; ++i) {
		if (c._equipped[i]) {
			if (getItem(c._equipped[i])->_field10 == 0xff)
				break;
		}
	}
	if (i == INVENTORY_COUNT)
		_uncurseCost = 0;

	_alignmentCost = ALIGNMENT_COST[townNum];
	if (c._alignment == c._alignmentInitial)
		_alignmentCost = 0;

	_donateCost = DONATE_COST[townNum];
}

void Temple::draw() {
	Business::draw();

	writeString(21, 0, STRING["dialogs.temple.service_cost"]);

	writeString(18, 2, STRING["dialogs.temple.a"]);
	_textPos.x = 36;
	if (_healCost)
		writeNumber(_healCost);
	else
		writeString("----");

	writeString(18, 3, STRING["dialogs.temple.b"]);
	_textPos.x = 36;
	if (_uncurseCost)
		writeNumber(_uncurseCost);
	else
		writeString("----");

	writeString(18, 4, STRING["dialogs.temple.c"]);
	_textPos.x = 36;
	if (_alignmentCost)
		writeNumber(_alignmentCost);
	else
		writeString("----");

	writeString(18, 5, STRING["dialogs.temple.d"]);
	_textPos.x = 36;
	writeNumber(_donateCost);
}

} // namespace Businesses
} // namespace Views
} // namespace MM1
} // namespace MM
