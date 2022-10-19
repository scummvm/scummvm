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

	g_globals->_treasure[5] = 0;

	if (getRandomNumber(50) == 10) {
		pit();

	} else {
		if (_data[VAL1] == 66 && _data[VAL2] == 74) {
			g_maps->_mapPos = Common::Point(7, 11);
		} else {
			g_maps->_mapPos.x = getRandomNumber(15);
			g_maps->_mapPos.y = getRandomNumber(15);
		}

		send(SoundMessage(STRING["maps.map11.poof"]));
		g_events->send("Game", GameMessage("UPDATE"));
	}
}

void Map11::special00() {
	send(SoundMessage(
		STRING["maps.ladder_up"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 2);
			g_maps->changeMap(0xf04, 2);
		}
	));
}

void Map11::special01() {
	if (_data[VAL1]) {
		g_maps->_mapPos = Common::Point(8, 5);
		g_maps->changeMap(0x604, 1);
	} else {
		g_events->addView("VolcanoGod");
	}
}

void Map11::special02() {
}

void Map11::special03() {
}

void Map11::special04() {
}

void Map11::special05() {
}

void Map11::special06() {
}

void Map11::special07() {
}

void Map11::special08() {
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

} // namespace Maps
} // namespace MM1
} // namespace MM
