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

#include "mm/mm1/maps/map20.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define CASTLE_STATE 177

void Map20::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 9; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[60 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_walls[g_maps->_mapOffset] != 0xff) {
		g_maps->clearSpecial();
		enc.execute();

	} else if (getRandomNumber(100) != 100) {
		none160();

	} else {
		int id1 = getRandomNumber(5);
		int monsterCount = getRandomNumber(13);

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, 11);

		enc._manual = true;
		enc._levelIndex = 80;
		enc.execute();
	}
}

void Map20::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map20.stairs_down"],
		[]() {
			g_maps->_mapPos = Common::Point(1, 15);
			g_maps->changeMap(0xc03, 1);
		}
	));
}

void Map20::special01() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map20.cave"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 0);
			g_maps->changeMap(0x51b, 1);
		}
	));
}

void Map20::special02() {
	visitedExit();
	if (_data[CASTLE_STATE] & 0x80) {
		goToCastle();

	} else if (_data[CASTLE_STATE]) {
		_data[CASTLE_STATE] = 0;

	} else {
		send("View", DrawGraphicMessage(6 + 65));

		send(SoundMessage(
			STRING["maps.map20.castle"],
			[]() {
				Map20 &map = *static_cast<Map20 *>(g_maps->_currentMap);
				map[CASTLE_STATE] = 0xff;
				map.goToCastle();
			},
			[]() {
				Map20 &map = *static_cast<Map20 *>(g_maps->_currentMap);
				map[CASTLE_STATE]++;
				map.updateGame();
			}
		));
	}
}

void Map20::special03() {
	bool hasWhistle = g_globals->_party.hasItem(RUBY_WHISTLE_ID);

	if (!hasWhistle) {
		send(SoundMessage(STRING["maps.map20.temple"],
			[](const Common::KeyState &) {
				g_events->focusedView()->close();
			}
		));
		return;
	}

	send(
		SoundMessage(STRING["maps.map20.temple"],
		[](const Common::KeyState &) {
			g_events->focusedView()->close();
			g_events->send(SoundMessage(
				STRING["maps.map20.whistle"],
				[](const Common::KeyState &ks) {
					if (ks.keycode == Common::KEYCODE_0) {
						g_events->focusedView()->close();
						g_maps->_currentMap->none160();
					} else if (ks.keycode == Common::KEYCODE_2) {
						g_events->focusedView()->close();
						g_events->send(SoundMessage(
							STRING["maps.map20.stairs_down"],
							[]() {
								for (uint i = 0; i < g_globals->_party.size(); ++i) {
									g_globals->_currCharacter = &g_globals->_party[i];
									g_globals->_currCharacter->_flags[0] |= CHARFLAG0_FOUND_CHEST | CHARFLAG0_40;
								}

								g_maps->_mapPos = Common::Point(8, 8);
								g_maps->changeMap(0xf04, 3);
							}
						));

					} else if (ks.keycode >= Common::KEYCODE_1 &&
							ks.keycode <= Common::KEYCODE_9) {
						g_events->focusedView()->close();
						g_maps->_mapPos = Common::Point(8, 5);
						g_maps->changeMap(0x604, 1);
					}
				}
			));
		}
	));
}

void Map20::special04() {
	send(SoundMessage(STRING["maps.map20.sign1"]));
}

void Map20::special05() {
	send(SoundMessage(STRING["maps.map20.sign2"]));
}

void Map20::special06() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_currCharacter = &g_globals->_party[i];
		g_globals->_currCharacter->_flags[5] |= CHARFLAG5_2;
	}

	SoundMessage info(0, 0, STRING["maps.map20.peak"]);
	info._largeMessage = true;
	send(info);
}

void Map20::special07() {
	g_maps->_mapPos.y = 6;
	updateGame();
}

void Map20::special08() {
	g_maps->_mapPos.x = 9;
	updateGame();
}

void Map20::goToCastle() {
	g_maps->_mapPos = Common::Point(15, 8);
	g_maps->changeMap(0xa11, 3);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
