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

#include "mm/mm1/maps/map29.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define ANSWER_OFFSET 67
#define BEAST_FLAG 110

void Map29::special() {
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

	switch (_walls[g_maps->_mapOffset]) {
	case 0:
		if (getRandomNumber(50) == 50) {
			int id1 = getRandomNumber(16);
			int monsterCount = getRandomNumber(id1 < 15 ? 13 : 4);

			enc.clearMonsters();
			for (int i = 0; i < monsterCount; ++i)
				enc.addMonster(id1, 11);

			enc._manual = true;
			enc._levelIndex = 80;
			enc.execute();

		} else {
			none160();
		}
		break;

	case 0xaa:
		if (g_maps->_forwardMask == DIRMASK_W)
			checkPartyDead();
		else
			// TODO: This key doesn't seem to be used by game
			g_events->addKeypress((Common::KeyCode)149);
		break;

	default:
		g_maps->clearSpecial();
		enc.execute();
		break;
	}
}

void Map29::special00() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map29.algary"],
		[]() {
			g_maps->_mapPos = Common::Point(8, 15);
			g_maps->changeMap(0x203, 1);
		}
	));
}

void Map29::special01() {
	if (_data[BEAST_FLAG]) {
		g_maps->clearSpecial();
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[2] |= CHARFLAG2_4;
		}
	} else {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			if (c._flags[2] & CHARFLAG2_4)
				return;
		}

		send(SoundMessage(
			STRING["maps.map29.beast"],
			[]() {
				g_maps->_mapPos = Common::Point(12, 12);
				g_maps->_currentMap->updateGame();
			},
			[]() {
				Game::Encounter &enc = g_globals->_encounters;
				Map29 &map = *static_cast<Map29 *>(g_maps->_currentMap);
				g_events->close();
				map[BEAST_FLAG]++;

				enc.clearMonsters();
				enc.addMonster(3, 12);
				for (int i = 1; i < 13; ++i)
					enc.addMonster(3, 5);

				enc._manual = true;
				enc._encounterType = Game::FORCE_SURPRISED;
				enc._levelIndex = 80;
				enc.execute();
			}
		));
	}
}

void Map29::special02() {
	if (!g_globals->_party.hasItem(B_QUEEN_IDOL_ID) ||
			!g_globals->_party.hasItem(W_QUEEN_IDOL_ID)) {
		begone();
	} else {
		g_events->addView("Chess");
	}
}

void Map29::special03() {
	SoundMessage msg(
		STRING["maps.map29.attack"],
		[]() {
			Game::Encounter &enc = g_globals->_encounters;
			int monsterCount = g_events->getRandomNumber(5) + 6;
			g_maps->clearSpecial();

			enc.clearMonsters();
			enc.addMonster(11, 12);
			for (int i = 1; i < monsterCount; ++i)
				enc.addMonster(1, 7);

			enc._manual = true;
			enc._encounterType = Game::FORCE_SURPRISED;
			enc._levelIndex = 80;
			enc.execute();
		}
	);
	msg._delaySeconds = 4;
	send(msg);
}

void Map29::chessAnswer(const Common::String &answer) {
	Common::String properAnswer;

	for (int i = 0; i < 22; ++i)
		properAnswer += _data[ANSWER_OFFSET + i] - 48;

	if (answer.equalsIgnoreCase(properAnswer)) {
		redrawGame();

		InfoMessage msg(
			16, 2, STRING["maps.map19.correct"],
			[]() {
				MM1::Maps::Map29 &map29 = *static_cast<MM1::Maps::Map29 *>(g_maps->_currentMap);

				for (uint i = 0; i < g_globals->_party.size(); ++i) {
					Character &c = g_globals->_party[i];
					c._exp += 25000;
				}

				g_maps->_mapPos.y = 7;
				map29.updateGame();
			}
		);

		msg._delaySeconds = 2;
		send(msg);
		Sound::sound(SOUND_3);
		Sound::sound(SOUND_3);

	} else {
		begone();
	}

}

void Map29::begone() {
	g_maps->_mapPos.y = 7;
	SoundMessage msg(STRING["maps.map29.begone"],
		[]() {
			g_maps->_currentMap->updateGame();
		}
	);
	msg._delaySeconds = 2;
	send(msg);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
