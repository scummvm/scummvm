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

#include "mm/mm1/maps/map39.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define ANSWER_OFFSET 477

void Map39::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 19; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[70 + i]) {
				
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

void Map39::special00() {
	send(SoundMessage(g_engine->isEnhanced() ?
		"Etched in gold, message 6 reads:\n"
			"Book-done.-A-self-that-from-\n"
			"key-you-be-to-seems" :
		"Etched in gold, message 6 reads:\n"
			"book-done.-a-self-that-from-key-you-\n"
			"be-to-seems"));
}

void Map39::special01() {
	visitedExit();

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_party[i]._flags[7] |= CHARFLAG7_1;
	}

	send(SoundMessage(
		"Stairs to surface, take them (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(9, 9);
			g_maps->changeMapFromExit(0x703, 2);
		}
	));
}

void Map39::special02() {
	visitedExit();

	send(SoundMessage(
		"Stairs going down! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0x702, 3);
		}
	));
}

void Map39::special03() {
	g_events->addView("Ruby");
}

void Map39::special04() {
	if (g_globals->_party.hasItem(SILVER_KEY_ID)) {
		send(SoundMessage("A silver door, your silver key glows!"));
	} else {
		g_maps->_mapPos.y++;
		updateGame();
		send(SoundMessage("A silver door repels you!"));
	}
}

void Map39::special05() {
	encounter(&_data[615], &_data[622]);
}

void Map39::special06() {
	encounter(&_data[629], &_data[637]);
}

void Map39::special07() {
	encounter(&_data[645], &_data[656]);
}

void Map39::special08() {
	encounter(&_data[667], &_data[676]);
}

void Map39::special09() {
	encounter(&_data[685], &_data[696]);
}

void Map39::special10() {
	encounter(&_data[707], &_data[718]);
}

void Map39::special11() {
	encounter(&_data[729], &_data[742]);
}

void Map39::special12() {
	encounter(&_data[755], &_data[762]);
}

void Map39::special13() {
	encounter(&_data[769], &_data[782]);
}

void Map39::special14() {
	encounter(&_data[795], &_data[807]);
}

void Map39::special18() {
	send(SoundMessage(mapText("The wall is painted from ceiling to\n"
		"floor in a black and white checkered\n"
		"pattern!")));
}

void Map39::riddleAnswered(const Common::String &answer) {
	Common::String properAnswer;

	for (int i = 0; i < 12 && _data[ANSWER_OFFSET + i]; ++i)
		properAnswer += _data[ANSWER_OFFSET + i] - 64;

	if (answer.equalsIgnoreCase(properAnswer)) {
		g_maps->clearSpecial();
		Sound::sound(SOUND_3);
		redrawGame();

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			g_globals->_party[i]._flags[5] |= CHARFLAG5_20;
		}

		g_globals->_treasure._items[2] = CRYSTAL_KEY_ID;
		g_events->addAction(KEYBIND_SEARCH);

	} else {
		g_maps->_mapPos.x = 9;
		updateGame();

		send(InfoMessage("Wrong!  A bright flash..."));
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
