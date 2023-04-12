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

#include "mm/mm1/maps/map27.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map27::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 6; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[57 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_walls[g_maps->_mapOffset] == 0xff) {
		desert();
	} else {
		g_maps->clearSpecial();
		g_globals->_encounters.execute();
	}
}

void Map27::special00() {
	send(SoundMessage(STRING["maps.map27.pool"]));

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._flags[11] & CHARFLAG11_PERSONALITY)) {
			c._flags[11] |= CHARFLAG11_PERSONALITY;
			if (c._personality._base < 30) {
				c._personality._current = c._personality._base =
					c._personality._base + 4;
			}
		}
	}
}

void Map27::special01() {
	send(SoundMessage(STRING["maps.map27.retreat"]));
}

void Map27::special02() {
	send(SoundMessage(
		0, 1, clerics('N'),
		0, 2, STRING["maps.map27.cured"]
	));

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (c._condition != ERADICATED) {
			c._condition = 0;
			c._hpCurrent = c._hpMax = c._hp;
		}
	}

	g_maps->clearSpecial();
}

void Map27::special03() {
	send(SoundMessage(
		0, 1, clerics('E'),
		0, 2, STRING["maps.map27.alignment"]
	));

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._alignment = c._alignmentInitial;
	}
}

void Map27::special04() {
	send(SoundMessage(
		0, 1, clerics('W'),
		0, 2, STRING["maps.map27.curses"]
	));

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];

		for (int invNum = 0; invNum < 2; ++invNum) {
			Inventory &inv = (invNum == 0) ? c._equipped : c._backpack;

			for (int itemNum = (int)inv.size() - 1; itemNum >= 0; --itemNum) {
				Item *item = g_globals->_items.getItem(c._equipped[itemNum]._id);
				if (item->_constBonus_id == EQUIP_CURSED)
					inv.removeAt(itemNum);
			}
		}
	}

	g_globals->_activeSpells._s.cursed = 0;
}

void Map27::special05() {
	send(SoundMessage(STRING["maps.map27.sign"]));
}

Common::String Map27::clerics(char name) {
	return Common::String::format("%s %c.",
		STRING["maps.map27.clerics"].c_str(), name);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
