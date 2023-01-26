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

#include "mm/mm1/maps/map33.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 71
#define VAL2 72

static const byte MONSTER_IDS11[10] = {
	15, 16, 15, 16, 14, 15,  4,  5,  3,  2
};
static const byte MONSTER_IDS12[10] = {
	7,  7,  8,  8,  9,  9, 10, 10, 10, 10
};
static const byte MONSTER_IDS21[12] = {
	2, 12, 7, 5, 5, 1, 2, 3, 1, 4, 5, 2
};
static const byte MONSTER_IDS22[12] = {
	5, 18, 10, 10, 11, 11, 11, 14, 13, 16, 15, 16
};

void Map33::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 5; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[56 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_walls[g_maps->_mapOffset] == 0xff) {
		if (g_maps->_mapPos.x >= 6 || g_maps->_mapPos.y < 5 ||
				g_maps->_mapPos.y >= 8) {
			if (g_maps->_mapPos.y < 5 && g_maps->_forwardMask != DIRMASK_N) {
				g_events->addKeypress((Common::KeyCode)149);
			} else {
				InfoMessage msg(0, 1, STRING["maps.map33.slime"]);
				Sound::sound(SOUND_2);

				switch (getRandomNumber(200)) {
				case 198: {
					Character &c = g_globals->_party[
						getRandomNumber(g_globals->_party.size() - 1)
					];
					if (!(c._condition & BAD_CONDITION)) {
						c._condition |= SILENCED | PARALYZED | UNCONSCIOUS;
						c._hpBase = 0;
						msg._lines.push_back(Line(0, 2, STRING["maps.map33.quicksand"]));
						Sound::sound(SOUND_3);
					}
					break;
				}

				case 199:
				case 200: {
					int monsterCount = getRandomNumber(10) + 3;
					enc.clearMonsters();
					for (int i = 0; i < monsterCount; ++i) {
						int idx = getRandomNumber(12) - 1;
						enc.addMonster(MONSTER_IDS21[idx], MONSTER_IDS22[idx]);
					}

					enc._manual = true;
					enc._levelIndex = 80;
					enc.execute();
					break;
				}

				default:
					break;
				}
			}
		} else {
			send(SoundMessage(STRING["maps.map33.tombstones"]));
		}
	} else {
		if (g_maps->_mapPos.y < 5) {
			int monsterCount = getRandomNumber(6) + 2;
			int idx = getRandomNumber(6) - 1;
			int id1 = MONSTER_IDS11[idx];
			int id2 = MONSTER_IDS12[idx];

			enc.clearMonsters();
			for (int i = 0; i < monsterCount; ++i)
				enc.addMonster(id1, id2);

			enc._manual = true;
			enc._levelIndex = 64;
		}

		g_maps->clearSpecial();
		enc.execute();
	}
}

void Map33::special00() {
	send(SoundMessage(
		STRING["maps.map33.meeting"],
		[]() {
			Game::Encounter &enc = g_globals->_encounters;
			g_maps->clearSpecial();

			enc.clearMonsters();
			for (int i = 0; i < 10; ++i)
				enc.addMonster(MONSTER_IDS11[i], MONSTER_IDS12[i]);

			enc._manual = true;
			enc._levelIndex = 80;
			enc.execute();
		}
	));
}

void Map33::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map33.building"],
		[]() {
			g_maps->_mapPos = Common::Point(0, 0);
			g_maps->changeMap(0xf01, 3);
		}
	));
}

void Map33::special02() {
	_data[VAL1] = 12;
	_data[VAL2] = 8;
	search(STRING["maps.map33.coffin"]);
}

void Map33::special03() {
	_data[VAL1] = 10;
	_data[VAL2] = 10;
	search(STRING["maps.map33.crypt"]);
}

void Map33::special04() {
	g_maps->clearSpecial();

	send(SoundMessage(
		STRING["maps.map33.corpse"],
		[]() {
			SoundMessage msg(
				STRING["maps.map33.thanks"],
				[](const Common::KeyState &) {
					g_events->close();
					g_globals->_treasure.setGems(50);
					g_events->addAction(KEYBIND_SEARCH);
				}
			);
			g_events->send(msg);
		}
	));
}

void Map33::search(const Common::String &msg) {
	send(SoundMessage(
		msg,
		[]() {
			Map33 &map = *static_cast<Map33 *>(g_maps->_currentMap);
			Game::Encounter &enc = g_globals->_encounters;
			int monsterCount = g_events->getRandomNumber(6) + 7;

			enc.clearMonsters();
			enc.addMonster(map[VAL1], map[VAL2]);

			for (int i = 1; i < monsterCount; ++i)
				enc.addMonster(10, 7);

			enc._manual = true;
			enc._levelIndex = 80;
			enc.execute();
		}
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
