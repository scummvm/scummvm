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
	_title = STRING["maps.emap42.title"];
}

bool DogStatue::msgFocus(const FocusMessage &msg) {
	Sound::sound(SOUND_2);

	_completedQuests = false;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if ((g_globals->_party[i]._flags[0] &
			(CHARFLAG0_COURIER3 | CHARFLAG0_FOUND_CHEST | CHARFLAG0_40)) ==
			(CHARFLAG0_COURIER3 | CHARFLAG0_FOUND_CHEST | CHARFLAG0_40)) {
			_completedQuests = true;
			break;
		}
	}

	clearButtons();

	if (_completedQuests) {
		addText(Common::String::format("%s%s",
			STRING["maps.map42.statue1"].c_str(),
			STRING["maps.map42.statue2"].c_str()
		));
	} else {
		addText(Common::String::format("%s%s",
			STRING["maps.map42.statue1"].c_str(),
			STRING["maps.map42.statue3"].c_str()
		));

		addButton(STRING["maps.yes"], 'Y');
		addButton(STRING["maps.no"], 'N');
	}

	return TextView::msgFocus(msg);
}

bool DogStatue::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map42 &map = *static_cast<MM1::Maps::Map42 *>(g_maps->_currentMap);

	if (_completedQuests) {
		close();
		map.dogSuccess();

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
	if (_completedQuests)
		map.dogSuccess();
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
