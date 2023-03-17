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

#include "mm/mm1/views_enh/locations/blacksmith.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

Blacksmith::Blacksmith() : Location("Blacksmith", LOC_BLACKSMITH) {
	addButton(&g_globals->_escSprites, Common::Point(24, 100), 0, KEYBIND_ESCAPE);
}

void Blacksmith::draw() {
	Location::draw();

	const Character &c = *g_globals->_currCharacter;

	setReduced(false);
	writeLine(0, STRING["enhdialogs.location.store"], ALIGN_MIDDLE);
	writeLine(1, STRING["enhdialogs.location.options_for"], ALIGN_MIDDLE);
	writeLine(3, c._name, ALIGN_MIDDLE);

	writeLine(5, STRING["enhdialogs.blacksmith.browse"], ALIGN_LEFT, 10);

	writeLine(10, STRING["enhdialogs.location.gold"]);
	writeLine(10, Common::String::format("%d",
		g_globals->_currCharacter->_gold), ALIGN_RIGHT);

	setReduced(true);
	writeString(27, 122, STRING["enhdialogs.location.esc"]);
}

bool Blacksmith::msgKeypress(const KeypressMessage &msg) {
	// If a delay is active, end it
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_b:
		addView("BlackSmithItems");
		break;
	default:
		return Location::msgKeypress(msg);
		break;
	}

	return true;
}

bool Blacksmith::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		leave();
		return true;
	}

	return Location::msgAction(msg);
}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
