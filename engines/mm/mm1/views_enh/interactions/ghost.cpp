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

#include "mm/mm1/views_enh/interactions/ghost.h"
#include "mm/mm1/maps/map37.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Ghost::Ghost() : Interaction("Ghost", 33) {
	_title = "Okrim";
}

bool Ghost::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);
	addText("A ghostlike figure rises from the body of Okrim and speaks, \"In exchange for my ring, "
		"forfeit a life. Accept (Y/N)?\"");

	clearButtons();
	addButton("Accept", 'Y');
	addButton("Decline", 'N');

	return true;
}

bool Ghost::msgKeypress(const KeypressMessage &msg) {
	if (_buttons.empty()) {
		viewAction();
		return true;
	}

	MM1::Maps::Map37 &map = *static_cast<MM1::Maps::Map37 *>(g_maps->_currentMap);

	if (msg.keycode == Common::KEYCODE_y) {
		g_globals->_party[0]._condition = ERADICATED;

		// Match Classic/original MM1: Okrim's Ring is awarded whether
		// the party accepts or declines the ghost's demand.
		g_globals->_treasure._items[2] = RING_OF_OKRIM_ID;
		close();
		g_events->addAction(KEYBIND_SEARCH);
		return true;

	} else if (msg.keycode == Common::KEYCODE_n) {
		map[MM1::Maps::MAP_29] = 32;
		map[MM1::Maps::MAP_47] = 8;

		// Preserve Enhanced's acknowledgement text, but stage the same
		// Ring reward that Classic places in treasure immediately.
		g_globals->_treasure._items[2] = RING_OF_OKRIM_ID;
		addText("\"May the shadow of death greet you with open arms!\"");
		clearButtons();
		return true;
	}

	return true;
}

void Ghost::viewAction() {
	if (_buttons.empty()) {
		close();
		g_events->addAction(KEYBIND_SEARCH);
	}
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
