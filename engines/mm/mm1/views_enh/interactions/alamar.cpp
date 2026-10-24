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

	_title = "King Alamar";

	if (_succeeded) {
		addText(Common::String::format("The omnipotent King Alamar speaks,  %s",
			"\"My saviors, you're always welcome here! You should find the Inner Sanctum... Live long "
				"and prosper.\""
		));

	} else if (map[HAS_EYE]) {
		_title = "Imposter King Alamar";
		addText("The omnipotent King Alamar speaks,  \"You've discovered my true identity! Into the soul maze you go...\"");

		for (int i = 0; i < 6; ++i)
			Sound::sound(SOUND_2);

	} else {
		addText("The omnipotent King Alamar speaks,  \"Varnlings, I quest thee to find the Crypt of Carmenca. Away with you...\"");
	}

	return true;
}

void Alamar::viewAction() {
	MM1::Maps::Map49 &map = *static_cast<MM1::Maps::Map49 *>(g_maps->_currentMap);
	close();

	// Alamar has already been defeated, so there's nothing more to do
	if (_succeeded)
		return;

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
