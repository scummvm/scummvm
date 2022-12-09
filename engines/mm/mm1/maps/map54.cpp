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

#include "mm/mm1/maps/map54.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define PERF_TOTAL 1293
#define VAL2 1296

static byte FLAGS[5] = { 1, 2, 4, 8, 0x10 };

void Map54::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 8; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[59 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	checkPartyDead();
}

void Map54::special00() {
	if (!g_globals->_party.hasItem(KEY_CARD_ID)) {
		g_maps->_mapPos.y++;
		updateGame();

		send(SoundMessage(
			0, 1, STRING["maps.map54.slot1"],
			0, 2, STRING["maps.map54.slot2"]
		));
		return;
	}

	// Check for flag
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if ((c._flags[13] & (CHARFLAG13_1 | CHARFLAG13_2 | CHARFLAG13_4 | CHARFLAG13_8 | CHARFLAG13_10)) !=
			(CHARFLAG13_1 | CHARFLAG13_2 | CHARFLAG13_4 | CHARFLAG13_8 | CHARFLAG13_10)) {
			g_maps->_mapPos.y++;
			updateGame();

			send(SoundMessage(
				0, 1, STRING["maps.map54.slot1"],
				0, 2, STRING["maps.map54.slot3"]
			));
			return;
		}
	}

	send(SoundMessage(
		0, 1, STRING["maps.map54.slot1"],
		0, 2, STRING["maps.map54.slot4"]
	));
}

void Map54::special01() {
	projector(0);
}

void Map54::special02() {
	projector(1);
}

void Map54::special03() {
	projector(2);
}

void Map54::special04() {
	projector(3);
}

void Map54::special05() {
	projector(4);
}

void Map54::special06() {
	send(SoundMessage(STRING["maps.map54.glow"]));
}

void Map54::special07() {
	send("View", DrawGraphicMessage(5 + 65));
	g_events->addView("Keeper");
}

void Map54::projector(int index) {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_party[i]._flags[13] |= FLAGS[index];
	}

	sorpigalInn();

	Common::String line = Common::String::format(
		STRING["maps.map54.projector"].c_str(),
		'1' + index);
	send(SoundMessage(line));
}

bool Map54::isWorthy(uint32 &perfTotal) {
	perfTotal = 0;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (c._flags[13] & CHARFLAG13_80) {
			_data[VAL2]++;
		} else if (c._flags[13] & CHARFLAG13_ALAMAR) {
			c._exp += 500000;
			c._flags[13] = 0xff;
			_data[VAL2]++;
		}

		// Calculate performance totals for party
		perfTotal += c.getPerformanceTotal();
		if (c._flags[13] & CHARFLAG13_80)
			perfTotal += 65536;
	}

	_data[PERF_TOTAL + 0] = perfTotal & 0xff;
	_data[PERF_TOTAL + 1] = (perfTotal >> 8) & 0xff;
	_data[PERF_TOTAL + 2] = (perfTotal >> 16) & 0xff;

	return perfTotal >= 65536;
}

void Map54::sorpigalInn() {
	g_maps->_mapPos = Common::Point(8, 5);
	g_maps->changeMap(0x604, 1);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
