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

#include "mm/mm1/maps/map47.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

static const char *const TONES[3] = {
	"A loud tone resounds\n"
		"throughout the room!",
	"A sharp tone assaults your senses!",
	"A mellow tone relaxes you."
};

#define ARRAY1 250
#define QUEST_CLERICS_TONE_OFFSET (15 * MAP_W + 10)
#define QUEST_CLERICS_COMPLETE_OFFSET (15 * MAP_W + 13)
#define QUEST_CLERICS_ACTIVE_OFFSET (15 * MAP_W + 15)
#define QUEST_LOG_FLAG 0x80

void Map47::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 26; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[77 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (getRandomNumber(100) < 10) {
		g_globals->_encounters.execute();

	} else {
		g_maps->_mapPos = Common::Point(
			getRandomNumber(16) - 1, getRandomNumber(16) - 1);
		updateGame();
		send(SoundMessage("Poof!"));
	}
}

void Map47::special00() {
	send(SoundMessage(g_engine->isEnhanced() ?
		"Etched in gold, message 5 reads:\n"
			"Scoop:-the-discoveries-your-to-\n"
			"return-the-your-for-dreams-is" :
		"Etched in gold, message 5 reads:\n"
			"Scoop:-the-discoveries-your-to-return-\n"
			"the-your-for-dreams-is"));
}

void Map47::special01() {
	send(SoundMessage(
		mapText("Covered with cryptic symbols, a large\n"
			"brass gong hangs from the ceiling.\n"
			"Strike it (Y/N)?"),
		[]() {
			Map47 &map = *static_cast<Map47 *>(g_maps->_currentMap);
			int toneNum = g_events->getRandomNumber(6);
			if (toneNum < 4) {
				// Record the tone for the quest and its Quest Log progress
				map._data[ARRAY1 + toneNum - 1] = 1;
				map._visited[QUEST_CLERICS_TONE_OFFSET +
					toneNum - 1] |= QUEST_LOG_FLAG;
				g_events->send(SoundMessage(mapText(TONES[toneNum - 1])));
			} else {
				map.poof();
			}
		}
	));
}

void Map47::special02() {
	// Tones are also kept in the saved visit data, since the overlay
	// data is reloaded from disk when a save is loaded
	bool heard[3];
	for (int i = 0; i < 3; ++i) {
		heard[i] = _data[ARRAY1 + i] ||
			(_visited[QUEST_CLERICS_TONE_OFFSET + i] & QUEST_LOG_FLAG);
	}

	if (heard[0] && heard[1] && heard[2]) {
		send(SoundMessage("\"We deem you worthy of advancement!\""));
		_visited[QUEST_CLERICS_COMPLETE_OFFSET] |= QUEST_LOG_FLAG;
		_visited[QUEST_CLERICS_ACTIVE_OFFSET] &= ~QUEST_LOG_FLAG;

		// Vanilla MM1 behavior: becoming worthy recharges all seven
		// permanent-stat locations by clearing their GOT_* bits.
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[11] = CHARFLAG11_CLERICS;
		}

		// Quest Log persistence must describe only the current tone cycle.
		// After a successful Clerics turn-in, both temporary and persisted
		// Loud/Sharp/Mellow state are cleared so another recharge requires
		// hearing all three tones again.
		for (int i = 0; i < 3; ++i) {
			_data[ARRAY1 + i] = 0;
			_visited[QUEST_CLERICS_TONE_OFFSET + i] &= ~QUEST_LOG_FLAG;
		}
	} else {
		// Speaking to the Clerics starts the optional advancement quest.
		_visited[QUEST_CLERICS_ACTIVE_OFFSET] |= QUEST_LOG_FLAG;
		send(SoundMessage(
			0, 1, mapText("The Clerics of the South speak:\n"),
			0, 2, "\"Hear the 3 tones and return!\""
		));
	}
}

void Map47::special03() {
	visitedExit();

	send(SoundMessage(
		"Stairs going up! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0xa00, 3);
		}
	));
}

void Map47::special04() {
	send(SoundMessage(g_engine->isEnhanced() ?
		"On the right wall is a painting of a box, on the left a painting of a curtain." :
		"On the right wall is a painting of a\n"
			"box,on the left a painting of a curtain."));
}

void Map47::special05() {
	encounter(&_data[587], &_data[594]);
}

void Map47::special06() {
	encounter(&_data[601], &_data[608]);
}

void Map47::special07() {
	encounter(&_data[615], &_data[624]);
}

void Map47::special08() {
	encounter(&_data[633], &_data[644]);
}

void Map47::special09() {
	encounter(&_data[655], &_data[665]);
}

void Map47::special10() {
	encounter(&_data[675], &_data[684]);
}

void Map47::special11() {
	encounter(&_data[693], &_data[700]);
}

void Map47::special12() {
	encounter(&_data[707], &_data[718]);
}

void Map47::special13() {
	encounter(&_data[729], &_data[740]);
}

void Map47::special14() {
	encounter(&_data[751], &_data[762]);
}

void Map47::special15() {
	encounter(&_data[773], &_data[782]);
}

void Map47::special16() {
	encounter(&_data[791], &_data[801]);
}

void Map47::special17() {
	encounter(&_data[811], &_data[821]);
}

void Map47::special23() {
	Common::String line = Common::String::format(
		"Door number %c",
		'0' + (g_maps->_mapPos.x - 5)
	);
	send(SoundMessage(line));
}

void Map47::poof() {
	if (getRandomNumber(100) < 10) {
		g_globals->_encounters.execute();

	} else {
		g_maps->_mapPos = Common::Point(
			getRandomNumber(16) - 1, getRandomNumber(16) - 1);
		updateGame();

		send(SoundMessage("Poof!"));
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
