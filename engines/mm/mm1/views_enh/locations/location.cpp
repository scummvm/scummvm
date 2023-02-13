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

#include "mm/mm1/views_enh/locations/location.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"
#include "mm/shared/utils/strings.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

Location::Location(const Common::String &name) :
		ScrollView(name) {
	_bounds = Common::Rect(232, 0, 320, 146);
}

void Location::leave() {
	g_maps->turnAround();
	close();
	g_events->redraw();
}

void Location::displayMessage(const Common::String &msg) {
	Location::draw();

	writeLine(3, msg, ALIGN_MIDDLE);
}

void Location::changeCharacter(uint index) {
	if (index >= g_globals->_party.size())
		return;

	g_globals->_currCharacter = &g_globals->_party[index];
	redraw();
}

bool Location::subtractGold(uint amount) {
	if (g_globals->_currCharacter->_gold < amount) {
		notEnoughGold();
		return false;
	} else {
		g_globals->_currCharacter->_gold -= amount;
		return true;
	}
}

void Location::notEnoughGold() {
	Sound::sound(SOUND_2);
	displayMessage(STRING["dialogs.misc.not_enough_gold"]);
}

void Location::backpackFull() {
	Sound::sound(SOUND_2);
	displayMessage(STRING["dialogs.misc.backpack_full"]);
}

} // namespace Locations
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
