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

#include "mm/mm1/views/interactions/alamar.h"
#include "mm/mm1/maps/map49.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

#define VAL1 952
#define HAS_EYE 154

Alamar::Alamar() : TextView("Alamar") {
	_bounds = getLineBounds(17, 24);
}

bool Alamar::msgFocus(const FocusMessage &msg) {
	MM1::Maps::Map49 &map = *static_cast<MM1::Maps::Map49 *>(g_maps->_currentMap);

	_succeeded = false;
	for (uint i = 0; i < g_globals->_party.size() && !_succeeded; ++i)
		_succeeded = (g_globals->_party[i]._flags[13] & CHARFLAG13_ALAMAR) != 0;
	map[HAS_EYE] = g_globals->_party.hasItem(EYE_OF_GOROS_ID) ? 1 : 0;

	if (!_succeeded && !map[HAS_EYE]) {
		for (uint i = 0; i < g_globals->_party.size() && !_succeeded; ++i)
			g_globals->_party[i]._quest = 255;
	}

	Sound::sound(SOUND_2);
	Sound::sound(SOUND_2);

	return TextView::msgFocus(msg);
}

void Alamar::draw() {
	MM1::Maps::Map49 &map = *static_cast<MM1::Maps::Map49 *>(g_maps->_currentMap);
	clearSurface();

	if (_succeeded) {
		writeString(0, 0, STRING["maps.map49.alamar1"]);
		writeString(STRING["maps.map49.alamar3"]);

	} else if (map[HAS_EYE]) {
		send("View", DrawGraphicMessage(65 + 7));
		send("View", DrawGraphicMessage(65 + 8));

		writeString(0, 0, STRING["maps.map49.alamar1"]);
		writeString(STRING["maps.map49.alamar4"]);

		for (int i = 0; i < 6; ++i)
			Sound::sound(SOUND_2);

	} else {
		writeString(0, 0, STRING["maps.map49.alamar1"]);
		writeString(STRING["maps.map49.alamar2"]);
	}
}

bool Alamar::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map49 &map = *static_cast<MM1::Maps::Map49 *>(g_maps->_currentMap);
	close();

	if (map[HAS_EYE]) {
		map[VAL1]++;
		map.updateGame();

	} else {
		g_maps->_mapPos.x = 8;
		map.updateGame();
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
