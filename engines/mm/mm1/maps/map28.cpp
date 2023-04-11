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

#include "mm/mm1/maps/map28.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/data/locations.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 63
#define VAL2 64

void Map28::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 3; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[54 + i]) {			
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (g_maps->_mapPos.x >= 7) {
		g_maps->clearSpecial();
		g_globals->_encounters.execute();

	} else {
		send(SoundMessage(
			STRING["maps.map28.tree"],
			[]() {
				Map28 &map = *static_cast<Map28 *>(g_maps->_currentMap);

				if (!map[VAL2]) {
					g_events->send(SoundMessage(STRING["maps.map28.nothing"]));

				} else {
					g_maps->clearSpecial();
					map[VAL1]++;

					int val = g_events->getRandomNumber(10);
					switch (val) {
					case 1:
						g_events->send(SoundMessage(STRING["maps.map28.nothing"]));
						break;

					case 2:
						for (uint i = 0; i < g_globals->_party.size(); ++i) {
							Character &c = g_globals->_party[i];
							c._food = MIN((int)c._food + 5, MAX_FOOD);
						}
						g_events->send(SoundMessage(STRING["maps.map28.food"]));
						break;

					case 3:
						map.setCondition(POISONED);
						map.reduceHpBase();
						g_events->send(SoundMessage(STRING["maps.map28.thorns"]));
						break;

					case 4:
						if (g_globals->_activeSpells._s.levitate) {
							g_events->send(SoundMessage(STRING["maps.map28.nothing"]));
						} else {
							map.reduceHpBase();
							map.reduceHpBase();
							g_events->send(SoundMessage(STRING["maps.map28.you_fell"]));
						}
						break;

					case 5:
						g_events->send(SoundMessage(STRING["maps.map28.lightning"]));
						break;

					case 6:
						map.setCondition(DISEASED);
						map.reduceHpBase();
						g_events->send(SoundMessage(STRING["maps.map28.lightning"]));
						break;

					case 7:
						if (!g_globals->_activeSpells._s.psychic_protection) {
							for (uint i = 0; i < g_globals->_party.size(); ++i) {
								Character &c = g_globals->_party[i];
								c._sp._current = 0;
							}
						}

						g_events->send(SoundMessage(STRING["maps.map28.flash"]));
						break;

					case 8:
						g_events->send(SoundMessage(STRING["maps.map28.poof"]));
						g_maps->_mapPos = Common::Point(
							g_events->getRandomNumber(15),
							g_events->getRandomNumber(15));
						map.updateGame();
						break;

					default:
						g_globals->_activeSpells._s.cursed = val;
						break;
					}
				}
			}
		));
	}
}

void Map28::special00() {
	g_events->addView("Arenko");
}

void Map28::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map28.cave"],
		[]() {
			g_maps->_mapPos = Common::Point(8, 8);
			g_maps->changeMap(0x601, 1);
		}
	));
}

void Map28::special02() {
	send(SoundMessage(STRING["maps.map28.sign"]));
}

void Map28::setCondition(byte condition) {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (getRandomNumber(3) == 2 && !(c._condition & BAD_CONDITION))
			c._condition = condition;
	}
}

void Map28::reduceHpBase() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._hpCurrent /= 2;
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
