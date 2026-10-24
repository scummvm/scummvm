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

#include "mm/mm1/maps/map48.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define ANSWER_OFFSET 274

void Map48::special() {
	g_events->addView("Orango");
}

void Map48::orangoAnswer(const Common::String &answer) {
	Common::String properAnswer;
	for (int i = 0; i < 15 && _data[ANSWER_OFFSET + i]; ++i)
		properAnswer += (_data[ANSWER_OFFSET + i] & 0x7f) + 29;

 	if (answer.equalsIgnoreCase(properAnswer)) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[13] |= CHARFLAG13_ALAMAR;
		}

		g_maps->_mapPos = Common::Point(8, 5);
		g_maps->changeMap(0x604, 1);

		SoundMessage msg(mapText("Agent Orango Seventeen reporting:\n"
			"Imposter xx21a7-3 voided!\n"
			"Starphase 5281.6 at 120-varn-161 pod #41\n"
			"You are now rank 1 and eligible for\n"
			"transfer. Find Inner Sanctum for new\n"
			"assignment. The walls begin to fade...\n"
			"(press space)"));
		msg._largeMessage = true;
		send(msg);

	} else {
		g_maps->_mapPos.x++;
		updateGame();
		send(SoundMessage(13, 2, "Keep looking!"));
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
