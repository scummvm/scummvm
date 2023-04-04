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

#include "mm/mm1/maps/map11.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define MAX_ANSWER_LENGTH 8
#define ANSWER_OFFSET 636
#define VAL1 641
#define VAL2 642
#define VAL3 643

void Map11::special() {
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

	g_globals->_treasure._items[2] = 0;

	if (getRandomNumber(50) == 10) {
		pit();

	} else {
		if (_data[VAL1] == 66 && _data[VAL2] == 74) {
			g_maps->_mapPos = Common::Point(7, 11);
		} else {
			g_maps->_mapPos.x = getRandomNumber(15);
			g_maps->_mapPos.y = getRandomNumber(15);
		}

		updateGame();
		send(SoundMessage(STRING["maps.map11.poof"]));
	}
}

void Map11::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map11.ladder"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 2);
			g_maps->changeMap(0xf04, 2);
		}
	));
}

void Map11::special01() {
	if (_data[VAL1]) {
		// Teleport to outside the Inn of Sorpigal
		g_maps->_mapPos = Common::Point(8, 5);
		g_maps->changeMap(0x604, 1);
	} else {
		g_events->addView("VolcanoGod");
	}
}

void Map11::special02() {
	g_maps->clearSpecial();
	pit();
}

void Map11::special03() {
	send(SoundMessage(STRING["maps.map11.sign"]));
}

void Map11::special04() {
	g_maps->clearSpecial();

	send(SoundMessage(STRING["maps.map11.virgin"],
		[](const Common::KeyState &ks) {
			switch (ks.keycode) {
			case Common::KEYCODE_a:
				g_events->close();
				g_events->send(SoundMessage(STRING["maps.map11.tip1"]));
				break;
			case Common::KEYCODE_b:
				static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap)->challenge();
				break;
			case Common::KEYCODE_c:
			case Common::KEYCODE_ESCAPE:
				g_events->close();
				break;
			default:
				break;
			}
		}
	));
}

void Map11::special05() {
	send(SoundMessage(STRING["maps.map11.tip2"]));
}

void Map11::special06() {
	selectDial(0);
}

void Map11::special07() {
	selectDial(1);
}

void Map11::special08() {
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map11::pit() {
	if (g_globals->_activeSpells._s.levitate) {
		Common::String msg = Common::String::format("%s %s",
			STRING["maps.map10.pit"].c_str(),
			STRING["maps.map10.levitation"].c_str());
		send(SoundMessage(msg));
		Sound::sound(SOUND_3);

	} else {
		reduceHP();
		reduceHP();
		send(SoundMessage(STRING["maps.map10.pit"]));
		Sound::sound(SOUND_3);
	}
}

void Map11::selectDial(int dialIndex) {
	_dialIndex = dialIndex;
	Common::String msg = Common::String::format(
		STRING["maps.map11.dial"].c_str(), '1' + dialIndex);

	send(SoundMessage(msg,
		[](const Common::KeyState &ks) {
			if (ks.keycode >= Common::KEYCODE_a &&
					ks.keycode <= Common::KEYCODE_z) {
				Map11 &map = *static_cast<Map11 *>(g_maps->_currentMap);
				g_events->close();
				map.setDialChar(ks.ascii);
				map.none160();
			}
		}
	));
}

void Map11::challenge() {
	Game::Encounter &enc = g_globals->_encounters;
	g_events->close();

	enc.clearMonsters();
	enc.addMonster(10, 12);
	for (int i = 1; i < 10; ++i)
		enc.addMonster(7, 8);

	enc._levelIndex = 96;
	enc._manual = true;
	enc.execute();
}

void Map11::setDialChar(char c) {
	_data[VAL2 + _dialIndex] = c;
}

void Map11::clue() {
	g_maps->_mapPos = Common::Point(0, 5);
	updateGame();
}

void Map11::riddleAnswer(const Common::String &answer) {
	Common::String properAnswer;
	for (int i = 0; i < 8 && _data[ANSWER_OFFSET + i]; ++i)
		properAnswer += _data[ANSWER_OFFSET + i] + 30;

	if (answer.equalsIgnoreCase(properAnswer)) {
		_data[VAL1]++;
		g_globals->_treasure._items[2] = KEY_CARD_ID;
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		g_maps->_mapPos = Common::Point(7, 2);
		g_maps->changeMap(0xf04, 2);
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
