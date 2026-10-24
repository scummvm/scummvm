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

#include "mm/mm1/maps/map38.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map38::special() {
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

	g_maps->_mapPos = Common::Point(getRandomNumber(16) - 1,
		getRandomNumber(16) - 1);
	// This random relocation bypasses updateGame()/redrawGame().
	g_maps->visitedTile();
	send(SoundMessage(mapText("A strange ringing sound startles\nthe party. You are teleported!")));
}

void Map38::special00() {
	send(SoundMessage(g_engine->isEnhanced() ?
		"Etched in gold, message 4 reads:\n"
			"Of-be-has-true-knowledge-plane-\n"
			"a-level-to-reality-it" :
		"Etched in gold, message 4 reads:'\n"
			"of-be-has-true-knowledge-plane-a-level-'\n"
			"to-reality-it"));
}

void Map38::special01() {
	send(SoundMessage(mapText("The wall is painted from ceiling to\n"
		"floor in a black and white checkered\n"
		"pattern!")));
}

void Map38::special02() {
	special03();
}

void Map38::special03() {
	visitedExit();

	send(SoundMessage(
		"Stairs going up! Take them (Y/N)?",
		[]() {
			g_maps->_mapPos.x = 0;
			g_maps->changeMapFromExit(0xf03, 3);
		}
	));
}

void Map38::special04() {
	send(SoundMessage(mapText("A banner reads:\n"
		"the Labyrinth of Lazzeruth...\n"
		"Few have entered, none have returned!")));
}

void Map38::special05() {
	encounter(&_data[669], &_data[682]);
}

void Map38::special06() {
	encounter(&_data[690], &_data[705]);
}

void Map38::special07() {
	encounter(&_data[715], &_data[724]);
}

void Map38::special08() {
	encounter(&_data[733], &_data[743]);
}

void Map38::special09() {
	encounter(&_data[753], &_data[764]);
}

void Map38::special10() {
	encounter(&_data[775], &_data[788]);
}

void Map38::special11() {
	encounter(&_data[801], &_data[816]);
}

void Map38::special12() {
	encounter(&_data[831], &_data[840]);
}

void Map38::special13() {
	encounter(&_data[849], &_data[862]);
}

void Map38::special14() {
	encounter(&_data[875], &_data[890]);
}

void Map38::special15() {
	send(SoundMessage(mapText("Inset in the wall, a stone face\nyells: \"My brain hurts!\"")));
}

void Map38::special16() {
	send(SoundMessage(mapText("Inset in the wall, a stone face\n"
		"speaks: \"I would pay highly for a green\n"
		"handled, pearl encrusted abelnuski.\"")));
}

void Map38::special17() {
	send(SoundMessage(mapText("Inset in the wall, a stone face\n"
		"speaks: \"I see all, hear all,\n"
		"know nothing...\"")));
}

void Map38::special18() {
	send(SoundMessage(mapText("Inset in the wall, a stone face\n"
		"proclaims: \"Okrim is watching.\n"
		"He studies your weaknesses!\"")));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
