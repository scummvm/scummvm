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

#include "mm/mm1/views/interactions/dog_statue.h"
#include "mm/mm1/maps/map42.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

DogStatue::DogStatue() : TextView("DogStatue") {
	_bounds = getLineBounds(17, 24);
}

bool DogStatue::msgFocus(const FocusMessage &msg) {
	Sound::sound(SOUND_2);

	bool alreadyCompleted = false;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if (g_globals->_party[i]._flags[0] & CHARFLAG0_DOG_STATUE) {
			alreadyCompleted = true;
			break;
		}
	}

	_completedQuests = alreadyCompleted &&
		!g_globals->_party.hasItem(GOLD_KEY_ID);

	if (!alreadyCompleted) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			if ((g_globals->_party[i]._flags[0] &
				(CHARFLAG0_COURIER3 | CHARFLAG0_FOUND_CHEST | CHARFLAG0_40)) ==
				(CHARFLAG0_COURIER3 | CHARFLAG0_FOUND_CHEST | CHARFLAG0_40)) {
				_completedQuests = true;
				break;
			}
		}
	}

	return TextView::msgFocus(msg);
}

void DogStatue::draw() {
	MM1::Maps::Map42 &map = *static_cast<MM1::Maps::Map42 *>(g_maps->_currentMap);

	clearSurface();
	writeString(0, 0, "Standing atop a pedestal in the center\n"
		"of the room is a statue of a small sable\n"
		"and white dog. ");

	if (_completedQuests) {
		writeString(0, 2, "It comes to life and\n"
			"speaks,\"Congratulations, you've\n"
			"completed a major quest (+10000 exp)\n"
			"Remember B.J.!\"");
		map.dogSuccess();

	} else {
		writeString(0, 3, "Desecrate it (Y/N)?");
	}
}

bool DogStatue::msgKeypress(const KeypressMessage &msg) {
	if (_completedQuests) {
		close();
		g_events->addAction(KEYBIND_SEARCH);
		return true;
	}

	if (msg.keycode == Common::KEYCODE_y || msg.keycode == Common::KEYCODE_n) {
		MM1::Maps::Map42 &map = *static_cast<MM1::Maps::Map42 *>(g_maps->_currentMap);
		close();

		if (msg.keycode == Common::KEYCODE_y) {
			map.dogDesecrate();
		}
	}

	return true;
}

bool DogStatue::msgAction(const ActionMessage &msg) {
	if (_completedQuests) {
		close();
		g_events->addAction(KEYBIND_SEARCH);
		return true;
	}

	if (msg._action == KEYBIND_ESCAPE || msg._action == KEYBIND_SELECT) {
		MM1::Maps::Map42 &map = *static_cast<MM1::Maps::Map42 *>(g_maps->_currentMap);
		close();

		if (msg._action == KEYBIND_SELECT)
			map.dogDesecrate();

		return true;
	}

	return false;
}


} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
