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
static const byte MATCH_FLAGS[8] = {
	1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80
};
static const uint16 QUEST_EXPERIENCE[7] = {
	1000, 2000, 3000, 4000, 6000, 8000, 10000
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
	visitedExit();

	send(SoundMessage(
		STRING["maps.map35.exit"],
		[]() {
			g_maps->_mapPos = Common::Point(14, 10);
			g_maps->changeMap(0xa00, 2);
		}
	));
}

void Map35::special02() {
	visitedExit();

	g_maps->_mapPos = Common::Point(14, 9);
	g_maps->changeMap(0xa00, 2);
	send(SoundMessage(STRING["maps.map35.slide"]));
}

void Map35::special03() {
	send(SoundMessage(STRING["maps.map35.message"]));
}

void Map35::special04() {
	if (!g_globals->_party.hasItem(MERCHANTS_PASS_ID)) {
		g_maps->_mapPos.y++;
		updateGame();
		send(SoundMessage(STRING["maps.map35.merchant_pass"]));
	}
}

void Map35::special05() {
	updateFlags();
	send("View", DrawGraphicMessage(7 + 65));
	send("Inspectron", GameMessage("DISPLAY"));
}

void Map35::special06() {
	g_maps->clearSpecial();
	g_globals->_treasure.setGems(30);
	g_events->addAction(KEYBIND_SEARCH);
}

void Map35::special07() {
	send(SoundMessage(STRING["maps.map35.vault"]));
	_data[VAL1] = 30;
	_data[VAL2] = 7;
}

void Map35::special09() {
	g_maps->clearSpecial();
	g_globals->_treasure.setGems(50);
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

Common::String Map35::checkQuestComplete() {
	Character &leader = g_globals->_party[0];
	int qIndex = leader._quest - 8;

	if (leader._flags[5] & MATCH_FLAGS[qIndex] & 0x7f) {
		// The quest was complete
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._quest = 0;
			c._flags[8] |= MATCH_FLAGS[qIndex];
			c._exp += QUEST_EXPERIENCE[qIndex];
		}

		return Common::String::format(
			STRING["maps.map35.inspectron5"].c_str(),
			QUEST_EXPERIENCE[qIndex]);
	} else {
		// The quest isn't yet complete
		return STRING["maps.map35.inspectron3"];
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
