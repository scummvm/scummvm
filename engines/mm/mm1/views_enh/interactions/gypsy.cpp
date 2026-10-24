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

#include "mm/mm1/views_enh/interactions/gypsy.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/globals.h"


namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

Gypsy::Gypsy() : Interaction("Gypsy", 9) {
	_title = "Gypsy";
}

bool Gypsy::msgFocus(const FocusMessage &msg) {
	PartyView::msgFocus(msg);

	_charSelected = false;
	addText("A toothless Gypsy seer asks:\nWho would like to hear their sign (F1-F6)?");
	return true;
}

void Gypsy::viewAction() {
	// When already showing Gypsy, any click/key will close view
	if (_charSelected)
		close();
}

void Gypsy::charSwitched(Character *priorChar) {
	Interaction::charSwitched(priorChar);

	Character &c = *g_globals->_currCharacter;
	if (!(c._flags[4] & CHARFLAG4_ASSIGNED)) {
		c._flags[4] = CHARFLAG4_ASSIGNED |
			(getRandomNumber(8) - 1);
	}

	Common::String line = Common::String::format(
		"\"Your sign is the %s\"",
		COLOR_NAMES[c._flags[4] & CHARFLAG4_SIGN]
	);
	addText(line);

	_charSelected = true;
	redraw();
}

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
