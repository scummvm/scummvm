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

#include "mm/mm1/maps/map24.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 91

void Map24::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 10; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[61 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map24::special00() {
	send(SoundMessage(STRING["maps.map24.roadsign"]));
}

void Map24::special01() {
	SoundMessage msg(
		STRING["maps.map24.kilburn"],
		[]() {
			Map24 &map = *static_cast<Map24 *>(g_maps->_currentMap);
			if (!map.addItem(MAP_OF_DESERT_ID)) {
				g_maps->clearSpecial();
				map.none160();
			}
		}
	);
	msg._largeMessage = true;

	send(msg);
}

void Map24::special02() {
	if (_data[VAL1]) {
		send(SoundMessage(
			STRING["maps.map24.wyvern_eye"],
			[]() {
				Map24 &map = *static_cast<Map24 *>(g_maps->_currentMap);
				map.addItem(WYVERN_EYE_ID);
			}
		));
	} else {
		send(SoundMessage(
			STRING["maps.map24.lair"],
			[]() {
				Map24 &map = *static_cast<Map24 *>(g_maps->_currentMap);
				Game::Encounter &enc = g_globals->_encounters;
				map[VAL1]++;

				int monsterCount = g_events->getRandomNumber(4) + 3;
				enc.clearMonsters();
				enc.addMonster(6, 8);
				for (int i = 1; i < monsterCount; ++i)
					enc.addMonster(14, 7);

				enc._manual = true;
				enc._encounterType = Game::FORCE_SURPRISED;
				enc._levelIndex = 40;
				enc.execute();
			}
		));
	}
}

void Map24::special03() {
	g_maps->clearSpecial();

	SoundMessage msg(
		STRING["maps.map24.wyverns"],
		[]() {
			Game::Encounter &enc = g_globals->_encounters;
			int monsterCount = g_events->getRandomNumber(4) + 3;
			enc.clearMonsters();
			for (int i = 1; i < monsterCount; ++i)
				enc.addMonster(14, 7);

			enc._manual = true;
			enc._encounterType = Game::FORCE_SURPRISED;
			enc._levelIndex = 40;
			enc.execute();
		}
	);
	msg._delaySeconds = 3;
	send(msg);
}

void Map24::special08() {
	send(SoundMessage(STRING["maps.map24.sign"]));
}

void Map24::special09() {
	send(SoundMessage(
		STRING["maps.map24.hermit"],
		[]() {
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				c._backpack.clear();
				for (int j = 0; j < INVENTORY_COUNT; ++j)
					c._backpack.add(USELESS_ITEM_ID, 0);
			}

			Character &c = g_globals->_party[0];
			g_globals->_currCharacter = &c;
			c._backpack[0]._id = PIRATES_MAP_A_ID;
			c._backpack[1]._id = PIRATES_MAP_B_ID;

			g_maps->clearSpecial();
			g_maps->_currentMap->none160();
		}
	));
}

bool Map24::addItem(byte itemId) {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;

		if (!c._backpack.full()) {
			c._backpack.add(itemId, 20);
			return false;
		}
	}

	g_events->send(SoundMessage(STRING["maps.map24.backpacks_full"]));
	Sound::sound(SOUND_3);
	return true;
}

} // namespace Maps
} // namespace MM1
} // namespace MM
