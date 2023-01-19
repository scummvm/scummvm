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

#include "mm/mm1/maps/map35.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 29
#define VAL2 47

static const byte MATCH_ITEMS[7] = {
	0,  0,241,  0,  0,  0,  0
};
static const byte MATCH_FLAGS[7] = {
	CHARFLAG5_1, CHARFLAG5_2, CHARFLAG5_4, CHARFLAG5_8,
	CHARFLAG5_10, CHARFLAG5_20, CHARFLAG5_40
};

void Map35::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 11; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[62 + i]) {
				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	// All other cells on the map are encounters
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map35::special00() {
	g_events->addView("ManPrisoner");
}

void Map35::special01() {
	send(SoundMessage(
		STRING["maps.map35.exit"],
		[]() {
			g_maps->_mapPos = Common::Point(14, 10);
			g_maps->changeMap(0xa00, 2);
		}
	));
}

void Map35::special02() {
	send(SoundMessage(STRING["maps.map35.slide"]));
	g_maps->_mapPos = Common::Point(14, 9);
	g_maps->changeMap(0xa00, 2);
}

void Map35::special03() {
	send(SoundMessage(STRING["maps.map35.message"]));
}

void Map35::special04() {
	if (!g_globals->_party.hasItem(MERCHANTS_PASS_ID)) {
		send(SoundMessage(STRING["maps.map35.merchants"]));
		g_maps->_mapPos.y++;
		updateGame();
	}
}

void Map35::special05() {
	updateFlags();
	warning("TODO: drawMonster");
	g_events->addView("Inspectron");
}

void Map35::special06() {
	g_maps->clearSpecial();
	g_globals->_treasure[8] = 30;
	g_events->addAction(KEYBIND_SEARCH);
}

void Map35::special07() {
	send(SoundMessage(STRING["maps.map35.vault"]));
	_data[VAL1] = 30;
	_data[VAL2] = 7;
}

void Map35::special09() {
	g_maps->clearSpecial();
	g_globals->_treasure[8] = 50;
	special07();
}

void Map35::updateFlags() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		int counte = c._equipped.size();
		int count = counte + c._backpack.size();
		for (int itemIdx = 0; itemIdx < count; ++itemIdx) {
			byte itemId = (itemIdx < counte) ?
				c._equipped[itemIdx]._id : c._backpack[itemIdx - counte]._id;

			// Scan list of items to match against
			for (int arrIdx = 0; arrIdx < 7; ++arrIdx) {
				if (itemId == MATCH_ITEMS[arrIdx]) {
					c._flags[5] |= MATCH_FLAGS[arrIdx];
					break;
				}
			}
		}
	}
}

void Map35::acceptQuest() {
	Character &leader = g_globals->_party[0];
	byte flags = leader._flags[8];

	// Find quest that hasn't been done yet
	int questNum;
	for (questNum = 8; flags && questNum < 15; ++questNum, flags >>= 1) {
		if (!(flags & 1))
			break;
	}
	if (questNum == 15) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[8] = CHARFLAG8_80;
			c._flags[5] = CHARFLAG5_80;
		}
	}

	// Assign the quest to all party characters
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._quest = questNum;
	}

	// Draw the scene
	g_maps->_mapPos.y++;
	redrawGame();
}

bool Map35::matchQuest(Common::String &line) {
	// TODO
	return false;
}

} // namespace Maps
} // namespace MM1
} // namespace MM
