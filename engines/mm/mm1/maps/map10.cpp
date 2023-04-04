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

#include "mm/mm1/maps/map10.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

static const byte MAP_DEST_X[15] = {
	7, 7, 3, 3, 7, 13, 1, 7, 9, 13, 12, 1, 8, 12, 4
};
static const byte MAP_DEST_Y[15] = {
	3, 14, 7, 7, 12, 8, 13, 3, 0, 7, 2, 14, 13, 7, 7
};
static const uint16 MAP_DEST_ID[15] = {
	0x706, 0x107, 0xb07, 0x508, 0xf08, 0xa11, 0xa11, 1,
	0xc01, 0x202, 0x604, 0xc03, 0x203, 0x802, 0xb1a
};
static const byte MAP_DEST_SECTION[15] = {
	3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

void Map10::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 34; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[85 + i]) {
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

void Map10::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.passage_outside1"],
		[]() {
			g_maps->_mapPos = Common::Point(0, 7);
			g_maps->changeMap(0x101, 2);
		}
	));
}

void Map10::special01() {
	send(SoundMessage(
		STRING["maps.map10.ranalou1"],
		[](const Common::KeyState &ks) {
			if (ks.keycode == Common::KEYCODE_y) {
				g_events->send(InfoMessage(STRING["maps.map10.ranalou2"]));
				Sound::sound(SOUND_3);

				for (uint i = 0; i < g_globals->_party.size(); ++i)
					g_globals->_party[i]._flags[1] |= CHARFLAG1_1;

				g_maps->clearSpecial();

			} else if (ks.keycode == Common::KEYCODE_n) {
				g_events->send(SoundMessage(STRING["maps.map10.ranalou3"]));
				g_maps->_mapPos = Common::Point(15, 0);
				g_maps->_currentMap->updateGame();
			}
		}
	));
}

void Map10::special02() {
	int idx = g_maps->_mapPos.x;
	g_maps->_mapPos = Common::Point(MAP_DEST_X[idx], MAP_DEST_Y[idx]);
	g_maps->changeMap(MAP_DEST_ID[idx], MAP_DEST_SECTION[idx]);
	send(SoundMessage(STRING["maps.map10.poof"]));
}

void Map10::special17() {
	g_maps->_mapPos.y++;
	updateGame();
}

void Map10::special18() {
	g_maps->_mapPos.x++;
	updateGame();
}

void Map10::special19() {
	g_maps->_mapPos.x--;
	updateGame();
}

void Map10::special20() {
	g_maps->clearSpecial();
	g_globals->_treasure._items[2] = 252;
	g_globals->_treasure.setGems(20);
	g_globals->_treasure._container = SILVER_CHEST;
	g_events->addAction(KEYBIND_SEARCH);
}

void Map10::special23() {
	g_maps->clearSpecial();

	if (g_globals->_activeSpells._s.levitate) {
		Common::String msg = Common::String::format("%s %s",
			STRING["maps.map10.pit"].c_str(),
			STRING["maps.map10.levitation"].c_str());
		send(SoundMessage(msg));
		Sound::sound(SOUND_3);

	} else {
		reduceHP();

		if (getRandomNumber(4) == 4) {
			SoundMessage msg(
				0, 1, STRING["maps.map10.pit"],
				[]() {
					g_globals->_encounters.execute();
				}
			);
			msg._delaySeconds = 2;
			send(msg);
			Sound::sound(SOUND_3);

		} else {
			send(SoundMessage(STRING["maps.map10.pit"]));
			Sound::sound(SOUND_3);
		}
	}
}

void Map10::special29() {
	send(SoundMessage(STRING["maps.map10.sign1"]));
}

void Map10::special30() {
	Game::Encounter &enc = g_globals->_encounters;
	int monsterCount = getRandomNumber(4) + 3;
	g_globals->_treasure._items[2] = THUNDRANIUM_ID;

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(9, 9);

	enc._levelIndex = 64;
	enc._manual = true;
	enc.execute();
}

void Map10::special31() {
	send(SoundMessage(STRING["maps.map10.sign2"]));
}

void Map10::special32() {
	Game::Encounter &enc = g_globals->_encounters;
	int monsterCount = getRandomNumber(4) + 3;
	g_globals->_treasure._items[2] = LASER_BLASTER_ID;

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(8, 12);

	enc._levelIndex = 64;
	enc._manual = true;
	enc.execute();
}

void Map10::special33() {
	g_maps->_mapPos.x = 15;
	updateGame();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
