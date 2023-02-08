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

#include "mm/mm1/views/locations/temple.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

Temple::Temple() : Location("Temple") {
}

bool Temple::msgFocus(const FocusMessage &msg) {
	send("View", ValueMessage(LOC_TEMPLE));
	changeCharacter(0);

	return true;
}

bool Temple::msgKeypress(const KeypressMessage &msg) {
	// If a delay is active, end it
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		restoreHealth();
		break;
	case Common::KEYCODE_b:
		uncurseItems();
		break;
	case Common::KEYCODE_c:
		restoreAlignment();
		break;
	case Common::KEYCODE_d:
		donate();
		break;
	case Common::KEYCODE_g:
		g_globals->_currCharacter->gatherGold();
		redraw();
		break;
	case Common::KEYCODE_1:
	case Common::KEYCODE_2:
	case Common::KEYCODE_3:
	case Common::KEYCODE_4:
	case Common::KEYCODE_5:
	case Common::KEYCODE_6:
		changeCharacter(msg.keycode - Common::KEYCODE_1);
		break;
	default:
		break;
	}

	return true;
}

bool Temple::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		leave();
		return true;
	}

	return false;
}

void Temple::changeCharacter(uint index) {
	Maps::Map &map = *g_maps->_currentMap;
	int i;

	if (index >= g_globals->_party.size())
		return;
	Location::changeCharacter(index);

	_isEradicated = false;

	int townNum = map[Maps::MAP_ID];
	if (townNum < 1 || townNum >= 6)
		townNum = 1;
	--townNum;

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
			if (g_globals->_items.getItem(c._equipped[i])->_equipMode == EQUIP_CURSED)
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
	Location::draw();

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

void Temple::restoreHealth() {
	if (subtractGold(_healCost)) {
		Character &c = *g_globals->_currCharacter;
		c._condition = FINE;
		c._hpCurrent = c._hp;

		if (_isEradicated) {
			c._age._current += 10;
			--c._endurance;
		}

		redraw();
	}
}

void Temple::uncurseItems() {
	if (subtractGold(_uncurseCost)) {
		Character &c = *g_globals->_currCharacter;
		for (int i = 0; i < INVENTORY_COUNT; ++i) {
			if (c._equipped[i]) {
				g_globals->_items.getItem(c._equipped[i]);
				if (g_globals->_currItem._equipMode == EQUIP_CURSED) {
					c._equipped.removeAt(i);
					--i;
				}
			}
		}

		redraw();
	}
}

void Temple::restoreAlignment() {
	if (subtractGold(_alignmentCost)) {
		Character &c = *g_globals->_currCharacter;
		c._alignment = c._alignmentInitial;
		c._alignmentCtr = ALIGNMENT_VALS[c._alignment];

		redraw();
	}
}

void Temple::donate() {
	if (subtractGold(_donateCost)) {
		Maps::Map &map = *g_maps->_currentMap;
		Character &c = *g_globals->_currCharacter;

		int townNum = map[Maps::MAP_ID];
		if (townNum < 1 || townNum >= 6)
			townNum = 1;
		--townNum;

		c._worthiness |= DONATE_VALS[townNum];
		draw();

		if (g_engine->getRandomNumber(15) == 10) {
			for (int i = 0; i < 13; ++i)
				g_globals->_activeSpells._arr[i] = 75;

			Sound::sound(SOUND_3);
			displayMessage(STRING["dialogs.temple.protected"]);
		} else {
			displayMessage(STRING["dialogs.temple.thankyou"]);
		}
	}
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
