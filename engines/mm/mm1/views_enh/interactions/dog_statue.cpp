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

#include "mm/mm1/views_enh/interactions/dog_statue.h"
#include "mm/mm1/maps/map42.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

DogStatue::DogStatue() : Interaction("DogStatue") {
	_title = "Dog Statue";
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

	clearButtons();

	if (_completedQuests) {
		addText(Common::String::format("%s%s",
			"Standing atop a pedestal in the center of the room is a statue of a small sable and white "
				"dog. ",
			"It comes to life and speaks,\"Congratulations, you've completed a major quest (+10000 "
				"exp) Remember B.J.!\""
		));
	} else {
		addText(Common::String::format("%sDesecrate it (Y/N)?",
			"Standing atop a pedestal in the center of the room is a statue of a small sable and white "
				"dog. "));

		addButton("\x01""37Yes", 'Y');
		addButton("\x01""37No", 'N');
	}

	return TextView::msgFocus(msg);
}

bool DogStatue::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map42 &map = *static_cast<MM1::Maps::Map42 *>(g_maps->_currentMap);

	if (_completedQuests) {
		close();
		map.dogSuccess();
		g_events->addAction(KEYBIND_SEARCH);

	} else if (msg.keycode == Common::KEYCODE_y || msg.keycode == Common::KEYCODE_n) {
		close();

		if (msg.keycode == Common::KEYCODE_y)
			map.dogDesecrate();
	}

	return true;
}

void DogStatue::viewAction() {
	MM1::Maps::Map42 &map = *static_cast<MM1::Maps::Map42 *>(g_maps->_currentMap);

	close();
	if (_completedQuests) {
		map.dogSuccess();
		g_events->addAction(KEYBIND_SEARCH);
	}
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
