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

#include "mm/mm1/maps/map_desert.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void MapDesert::desert() {
	SoundMessage msg;
	msg._lines.push_back(Line(0, 1, STRING["maps.desert.its_hot"]));

	// Check whether party has the desert map
	bool hasMap = g_globals->_party.hasItem(MAP_OF_DESERT_ID);
	if (!hasMap) {
		msg._lines.push_back(Line(0, 2, STRING["maps.desert.lost"]));
		lost();
	}

	byte &deadCount = _data[_deadCountOffset];
	deadCount = 0;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];

		if (!(c._condition & BAD_CONDITION)) {
			if (c._food) {
				// Take away food first
				c._food--;

			} else if (c._endurance) {
				// Then decrease endurance when the food runs out
				c._endurance._current--;

			} else {
				// When endurance reaches, character becomes living impaired
				c._condition = DEAD | BAD_CONDITION;
				deadCount++;
			}
		}
	}

	if (deadCount)
		g_events->findView("GameParty")->redraw();

	switch (_randomMode) {
	case RND_BASIC:
		if (getRandomNumber(100) == 100) {
			msg._callback = []() {
				g_globals->_encounters.execute();
			};
			msg._delaySeconds = 3;
		}
		break;

	case RND_FULL:
		switch (getRandomNumber(200)) {
		case 20:
			msg._lines.push_back(Line(0, msg._lines.back().y + 1,
				STRING["maps.desert.whirlwind"]));
			Sound::sound(SOUND_3);
			g_maps->_mapPos = Common::Point(
				getRandomNumber(15), getRandomNumber(15));

			send(msg);
			updateGame();
			return;

		case 30:
			msg._lines.push_back(Line(0, msg._lines.back().y + 1,
				STRING["maps.desert.sandstorm"]));
			Sound::sound(SOUND_3);
			reduceHP();
			break;

		case 199:
		case 200:
			g_globals->_encounters.execute();
			break;

		default:
			break;
		}
		break;
	}

	send(msg);
}

void MapDesert::lost() {
	if (getRandomNumber(2) == 1) {
		g_maps->turnLeft();
	} else {
		g_maps->turnRight();
	}
}

bool MapDesert::mappingAllowed() const {
	return g_globals->_party.hasItem(MAP_OF_DESERT_ID);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
