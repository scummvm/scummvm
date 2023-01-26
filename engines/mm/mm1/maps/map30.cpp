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

#include "mm/mm1/maps/map30.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

static const byte MONSTER_ID1[8] = { 2, 3, 8, 10, 14, 16, 17, 7 };
static const byte MONSTER_ID2[8] = { 5, 6, 3, 8, 10, 12, 12, 12 };

void Map30::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 4; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[55 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_walls[g_maps->_mapOffset] != 0xff) {
		g_maps->clearSpecial();
		int monsterCount = getRandomNumber(8) + 6;
		int idx = getRandomNumber(8) - 1;
		byte id1 = MONSTER_ID1[idx];
		byte id2 = MONSTER_ID2[idx];

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, id2);

		enc._manual = true;
		enc._levelIndex = 96;
		enc.execute();
		return;
	}

	desert();
}

void Map30::special00() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map30.passage"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 7);
			g_maps->changeMap(0x802, 1);
		}
	));
}

void Map30::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map30.ruins"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 15);
			g_maps->changeMap(0x107, 3);
		}
	));
}

void Map30::special02() {
	g_events->addView("Giant");
}

void Map30::special03() {
	send(SoundMessage(
		STRING["maps.map30.hourglass"],
		[]() {
			g_maps->clearSpecial();
			Sound::sound(SOUND_3);

			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				c._age._current = c._age._base =
					MAX((int)c._age._base - 20, 18);
			}

			g_maps->_currentMap->none160();
		}
	));
}

Common::String Map30::worthiness() {
	Character &c = *g_globals->_currCharacter;
	if ((c._flags[1] & ~CHARFLAG1_WORTHY) == ~CHARFLAG1_WORTHY)
		return worthy();
	else
		return unworthy();
}

Common::String Map30::worthy() {
	Character &c = *g_globals->_currCharacter;
	int val = ((c._worthiness + 1) / 2) * 256;
	c._exp += val;

	Common::String line = Common::String::format(
		STRING["maps.map30.worthy"].c_str(), '0' + (c._worthiness / 5));
	line = Common::String::format("%s%d %s",
		line.c_str(), val, STRING["maps.map30.experience"].c_str());

	if (c._worthiness & 0x80) {
		int attrNum = getRandomNumber(7) - 1;
		line += Common::String::format(", +3 %s",
			STRING[Common::String::format("maps.map30.attributes.%d", attrNum)].c_str());

		AttributePair &attrib = c.getAttribute(attrNum);
		if (attrib._base < 43)
			attrib._current = attrib._base = attrib._base + 3;
	}

	c._worthiness = 0;
	return line;
}

Common::String Map30::unworthy() {
	return STRING["maps.map30.unworthy"];
}

} // namespace Maps
} // namespace MM1
} // namespace MM
