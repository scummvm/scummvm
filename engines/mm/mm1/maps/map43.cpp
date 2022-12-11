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

#include "mm/mm1/maps/map43.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define TREASURE_STATE 142

static const byte MATCH_ITEMS[7] = {
	0, 0, MAP_OF_DESERT_ID, 0, 0, 0, 0
};
static const byte MATCH_FLAGS[8] = {
	1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80
};
static const uint16 QUEST_EXPERIENCE[7] = {
	1000, 2000, 3000, 4000, 6000, 8000, 10000
};

void Map43::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 9; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[60 + i]) {			
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

void Map43::special00() {
	g_events->addView("DemonPrisoner");
}

void Map43::special01() {
	_data[TREASURE_STATE] = 0;
	if (g_maps->_forwardMask == DIRMASK_E) {
		visitedExit();

		send(SoundMessage(
			STRING["maps.map43.exit"],
			[]() {
				g_maps->_mapPos = Common::Point(9, 13);
				g_maps->changeMap(0x101, 2);
			}
		));
	} else {
		none160();
	}
}

void Map43::special02() {
	send(SoundMessage(
		STRING["maps.map43.button"],
		[]() {
			Map43 &map = *static_cast<Map43 *>(g_maps->_currentMap);
			map._states[101] = 17;
			map.none160();
		}
	));
}

void Map43::special03() {
	send(SoundMessage(STRING["maps.map43.message_b"]));
}

void Map43::special04() {
	if (!g_globals->_party.hasItem(MERCHANTS_PASS_ID)) {
		send(SoundMessage(
			STRING["maps.map43.guards"],
			[](const Common::KeyState &) {
				g_events->focusedView()->close();
				g_maps->_mapPos = Common::Point(9, 13);
				g_maps->changeMap(0x101, 2);
			}
		));
	}
}

void Map43::special05() {
	updateFlags();
	send("View", DrawGraphicMessage(7 + 65));
	g_events->addView("LordIronfist");
}

void Map43::special06() {
	if (_data[TREASURE_STATE]) {
		g_globals->_treasure.setGold(18000);
		g_maps->clearSpecial();
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		Game::Encounter &enc = g_globals->_encounters;
		_data[TREASURE_STATE]++;

		enc.clearMonsters();
		for (int i = 0; i < 8; ++i)
			enc.addMonster(13, 7);

		enc._manual = true;
		enc._levelIndex = 80;
		enc.execute();
	}
}

void Map43::special07() {
	send(SoundMessage(STRING["maps.map43.tower"]));
}

void Map43::special08() {
	send(SoundMessage(STRING["maps.map43.throne_room"]));
}

void Map43::updateFlags() {
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
					c._flags[7] |= MATCH_FLAGS[arrIdx];
					break;
				}
			}
		}
	}
}

void Map43::acceptQuest() {
	Character &leader = g_globals->_party[0];
	byte flags = leader._flags[10];

	// Find quest that hasn't been done yet
	int questNum = 1;
	if (flags) {
		for (questNum = 1; flags && questNum < 8; ++questNum, flags >>= 1) {
			if (!(flags & 1))
				break;
		}
		if (questNum == 8) {
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				c._flags[10] = CHARFLAG8_80;
				c._flags[7] = CHARFLAG5_80;
			}

			questNum = 1;
		}
	}

	// Assign the quest to all party characters
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._quest = questNum;
	}

	// Draw the scene
	g_maps->_mapPos.x++;
	updateGame();
}

Common::String Map43::checkQuestComplete() {
	Character &leader = g_globals->_party[0];
	int qIndex = leader._quest - 1;

	if (leader._flags[7] & MATCH_FLAGS[qIndex] & 0x7f) {
		// The quest was complete
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._quest = 0;
			c._flags[10] |= MATCH_FLAGS[qIndex];
			c._exp += QUEST_EXPERIENCE[qIndex];
		}

		return Common::String::format(
			STRING["maps.map43.ironfist5"].c_str(),
			QUEST_EXPERIENCE[qIndex]);
	} else {
		// The quest isn't yet complete
		return STRING["maps.map43.ironfist3"];
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
