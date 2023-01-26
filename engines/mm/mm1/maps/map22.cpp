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

#include "mm/mm1/maps/map22.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define WHISTLE_GIVEN 107

static const byte MONSTER_ID1[5] = { 13, 12, 13, 12, 11 };
static const byte MONSTER_ID2[5] = { 6, 6, 5, 5, 4 };

void Map22::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 14; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[65 + i]) {		
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

void Map22::special00() {
	bool hasFlags = false;
	for (uint i = 0; i < g_globals->_party.size() && !hasFlags; ++i) {
		g_globals->_currCharacter = &g_globals->_party[i];
		hasFlags = ((g_globals->_currCharacter->_flags[0] &
			(CHARFLAG0_ZAM_CLUE | CHARFLAG0_ZOM_CLUE)) ==
			(CHARFLAG0_ZAM_CLUE | CHARFLAG0_ZOM_CLUE));
	}

	if (!hasFlags) {
		none160();
		return;
	}

	Sound::sound(SOUND_3);
	InfoMessage msg(STRING["maps.map22.chest"]);
	msg._largeMessage = true;
	send(msg);

	for (uint i = 0; i < g_globals->_party.size() && !hasFlags; ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;

		// Remove the clue flags
		c._flags[0] = (c._flags[0] & ~(CHARFLAG0_ZAM_CLUE | CHARFLAG0_ZOM_CLUE)) |
			CHARFLAG0_FOUND_CHEST;

		// Add amulet to player
		if (!c._backpack.full()) {
			_data[WHISTLE_GIVEN]++;
			c._backpack.add(RUBY_WHISTLE_ID, 200);
			break;
		}
	}

	if (!_data[WHISTLE_GIVEN]) {
		// Entire party's inventory full. So screw it, replace
		// the last item in the last character's backpack
		g_globals->_currCharacter->_backpack.removeAt(INVENTORY_COUNT - 1);
		g_globals->_currCharacter->_backpack.add(RUBY_WHISTLE_ID, 192);
	}

	g_globals->_currCharacter->_gold += 5000;
}

void Map22::special01() {
	send(SoundMessage(STRING["maps.map22.roadsign"]));
}

void Map22::special02() {
	send(SoundMessage(
		STRING["maps.map22.wagons"],
		[]() {
			g_maps->clearSpecial();
			if (g_maps->_mapPos.x == 5) {
				g_globals->_treasure._items[2] = MERCHANTS_PASS_ID;
				g_events->addAction(KEYBIND_SEARCH);

			} else {
				InfoMessage msg1(
					16, 2, STRING["maps.map22.ambush"],
					[]() {
						Game::Encounter &enc = g_globals->_encounters;
						int monsterCount = g_events->getRandomNumber(3);
						int idx = g_events->getRandomNumber(5) - 1;
						const int id1 = MONSTER_ID1[idx];
						const int id2 = MONSTER_ID2[idx];

						enc.clearMonsters();
						for (int i = 0; i < monsterCount; ++i)
							enc.addMonster(id1, id2);

						enc._levelIndex = 32;
						enc._manual = true;
						enc._encounterType = Game::FORCE_SURPRISED;
						enc.execute();
					}
				);
				msg1._delaySeconds = 2;
				g_events->send(msg1);
			}
		}
	));
}

void Map22::special08() {
	send(SoundMessage(
		STRING["maps.map22.fountain"],
		[]() {
			switch (g_maps->_mapPos.x) {
			case 8:
				// Add might
				for (uint i = 0; i < g_globals->_party.size(); ++i)
					g_globals->_party[i]._might._current += 50;

				Sound::sound(SOUND_3);
				g_events->send(SoundMessage(STRING["maps.map22.today_might"]));
				break;

			case 9:
				// Add spell levels
				for (uint i = 0; i < g_globals->_party.size(); ++i)
					g_globals->_party[i]._spellLevel._current += 7;

				Sound::sound(SOUND_3);
				g_events->send(SoundMessage(STRING["maps.map22.today_spells"]));
				break;

			default:
				// Poison
				for (uint i = 0; i < g_globals->_party.size(); ++i) {
					Character &c = g_globals->_party[i];
					if (!(c._condition & BAD_CONDITION))
						c._condition |= POISONED;
				}

				Sound::sound(SOUND_3);
				g_events->send(SoundMessage(STRING["maps.map22.poison"]));
				break;
			}
		}
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
