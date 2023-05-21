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

#include "mm/mm1/views_enh/interactions/alamar.h"
#include "mm/mm1/maps/map49.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

#define VAL1 952
#define HAS_EYE 154

Alamar::Alamar() : Interaction("Alamar", 22) {
}

bool Alamar::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);
	clearButtons();

	MM1::Maps::Map49 &map = *static_cast<MM1::Maps::Map49 *>(g_maps->_currentMap);
	_succeeded = false;
	for (uint i = 0; i < g_globals->_party.size() && !_succeeded; ++i)
		_succeeded = (g_globals->_party[i]._flags[13] & CHARFLAG13_ALAMAR) != 0;
	map[HAS_EYE] = g_globals->_party.hasItem(EYE_OF_GOROS_ID) ? 1 : 0;

	if (!_succeeded && !map[HAS_EYE]) {
		for (uint i = 0; i < g_globals->_party.size() && !_succeeded; ++i)
			g_globals->_party[i]._quest = 255;
	}

	_title = STRING["maps.emap49.king_alamar"];

	if (_succeeded) {
		addText(Common::String::format("%s%s",
			STRING["maps.map49.alamar1"].c_str(),
			STRING["maps.map49.alamar3"].c_str()
		));

	} else if (map[HAS_EYE]) {
		_title = STRING["maps.emap49.sheltem"];
		addText(Common::String::format("%s%s",
			STRING["maps.map49.alamar1"].c_str(),
			STRING["maps.map49.alamar4"].c_str()
		));

		for (int i = 0; i < 6; ++i)
			Sound::sound(SOUND_2);

	} else {
		addText(Common::String::format("%s%s",
			STRING["maps.map49.alamar1"].c_str(),
			STRING["maps.map49.alamar2"].c_str()
		));
	}

	return true;
}

void Alamar::viewAction() {
	MM1::Maps::Map49 &map = *static_cast<MM1::Maps::Map49 *>(g_maps->_currentMap);
	close();

	if (map[HAS_EYE]) {
		map[VAL1]++;
		map.updateGame();

	} else {
		g_maps->_mapPos.x = 8;
		map.updateGame();
	}
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
