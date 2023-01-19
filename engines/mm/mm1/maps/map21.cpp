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

#include "mm/mm1/maps/map21.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 159
#define VAL2 160
#define TRIVIA_COST 500

void Map21::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 8; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[59 + i]) {	
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_states[g_maps->_mapOffset] != 0xff) {
		g_maps->clearSpecial();
		enc.execute();

	} else if (getRandomNumber(100) != 100) {
		if (getRandomNumber(200) != 192)
			none160();
		return;

	} else {
		int id1 = getRandomNumber(14);
		int monsterCount = getRandomNumber(13);

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, 11);

		enc._flag = true;
		enc._levelIndex = 80;
		enc.execute();
	}
}

void Map21::special00() {
	g_maps->clearSpecial();

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_currCharacter = &g_globals->_party[i];
		g_globals->_currCharacter->_flags[7] |= CHARFLAG7_20;
	}

	SoundMessage msg(
		STRING["maps.map21.ghostship"],
		[]() {
			Game::Encounter &enc = g_globals->_encounters;
			enc.clearMonsters();
			for (int i = 0; i < 8; ++i)
				enc.addMonster(12, 12);
			enc.addMonster(13, 12);
			for (int i = 9; i < 13; ++i)
				enc.addMonster(3, 8);

			enc._levelIndex = 80;
			enc._encounterType = Game::FORCE_SURPRISED;
			enc._flag = true;
			enc.execute();
		}
	);
	msg._delaySeconds = 2;
	send(msg);
}

void Map21::special01() {
	send(SoundMessage(
		STRING["maps.map21.free_trivia"],
		[]() {
			Map21 &map = *static_cast<Map21 *>(g_maps->_currentMap);
			map[VAL2]++;
		}
	));
}

void Map21::special02() {
	send(SoundMessage(
		STRING["maps.map21.trivia_island"],
		[](const Common::KeyState &ks) {
			Map21 &map = *static_cast<Map21 *>(g_maps->_currentMap);

			if (ks.keycode == Common::KEYCODE_y) {
				g_events->close();

				for (uint i = 0; i < g_globals->_party.size(); ++i) {
					Character &c = g_globals->_party[i];
					g_globals->_currCharacter = &c;
					if (g_globals->_currCharacter->_gold >= TRIVIA_COST) {
						c._gold -= 500;
						g_maps->clearSpecial();
						return;
					}
				}

				g_events->send(SoundMessage(STRING["maps.map21.not_enough_gold"]));
				g_maps->_mapPos.y++;
				updateGame();

			} else if (ks.keycode == Common::KEYCODE_n) {
				g_events->close();
				g_maps->_mapPos.y++;
				updateGame();
			}
		}
	));
}

void Map21::special03() {
	send("Trivia", 0);
}

void Map21::special04() {
	send("Trivia", 0);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
