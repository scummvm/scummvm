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

#include "mm/mm1/maps/map25.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map25::special() {
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

	switch (_walls[g_maps->_mapOffset]) {
	case 0:
		if (getRandomNumber(100) == 100) {
			Game::Encounter &enc = g_globals->_encounters;
			Character &c = g_globals->_party[0];
			g_globals->_currCharacter = &c;
			int val = getRandomNumber((c._level >= 12 ? 14 : c._level) + 2);
			int monsterCount = getRandomNumber((val < 15) ? 13 : 4);

			enc.clearMonsters();
			for (int i = 0; i < monsterCount; ++i)
				enc.addMonster(val, 11);

			enc._manual = true;
			enc._levelIndex = 80;
			enc.execute();

		} else {
			switch (getRandomNumber(100)) {
			case 99:
				send(SoundMessage(STRING["maps.map25.volcano"]));
				reduceHP();
				Sound::sound(SOUND_3);
				break;

			case 100: {
				SoundMessage msg(
					STRING["maps.map25.pirates"],
					[]() {
						Game::Encounter &enc = g_globals->_encounters;
						int monsterCount = g_events->getRandomNumber(8) + 4;
						enc.clearMonsters();
						for (int i = 0; i < monsterCount; ++i)
							enc.addMonster(12, 12);

						enc._levelIndex = 64;
						enc._manual = true;
						enc.execute();
					}
				);
				msg._delaySeconds = 3;
				send(msg);
				break;
			}

			default:
				none160();
				break;
			}
		}
		break;

	case 0x55:
		if (g_maps->_mapPos.y < 9) {
			nativesAttack();
		} else {
			if (g_maps->_mapPos.x >= 10) {
				SoundMessage msg(STRING["maps.map25.weeping"]);
				if (getRandomNumber(100) >= 99) {
					msg._delaySeconds = 3;
					msg._callback = []() {
						g_globals->_encounters.execute();
					};
				}

				send(msg);
			} else if (getRandomNumber(100) >= 99) {
				g_globals->_encounters.execute();
			}
		}
		break;

	default:
		send(SoundMessage(
			STRING["maps.map25.ship"],
			[]() {
				g_maps->clearSpecial();
				if (g_maps->_mapOffset == 216) {
					g_events->send(SoundMessage(STRING["maps.map25.jolly_raven"]));

					for (uint i = 0; i < g_globals->_party.size(); ++i) {
						g_globals->_currCharacter = &g_globals->_party[i];
						g_globals->_currCharacter->_flags[7] |= CHARFLAG7_10;
					}

					g_globals->_treasure.setGems(20);
				} else {
					g_globals->_activeSpells._s.cursed = 10;
					g_globals->_encounters.execute();
				}
			}
		));
		break;
	}
}

void Map25::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map25.portal"],
		[]() {
			g_maps->_mapPos = Common::Point(12, 4);
			g_maps->changeMap(0x201, 3);
		}
	));
}

void Map25::special01() {
	visitedExit();

	if (g_globals->_party.hasItem(CORAL_KEY_ID)) {
		send(InfoMessage(
			STRING["maps.map25.key"],
			[]() {
				g_maps->_mapPos = Common::Point(7, 5);
				g_maps->changeMap(0x212, 1);
			}
		));
	}
}

void Map25::special02() {
	nativesAttack();
}

void Map25::nativesAttack() {
	SoundMessage msg(
		STRING["maps.map25.natives"],
		[]() {
			Game::Encounter &enc = g_globals->_encounters;
			int monsterCount = g_events->getRandomNumber(6) + 7;
			enc.clearMonsters();
			for (int i = 0; i < monsterCount; ++i)
				enc.addMonster(9, 12);

			g_maps->clearSpecial();
			enc._levelIndex = 64;
			enc._manual = true;
			enc.execute();
		}
	);
	msg._delaySeconds = 3;
	send(msg);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
