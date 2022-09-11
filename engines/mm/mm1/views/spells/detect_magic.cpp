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

#include "mm/mm1/views/spells/detect_magic.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

void DetectMagic::show() {
	UIElement *view = dynamic_cast<DetectMagic *>(g_events->findView("DetectMagic"));
	assert(view);

	view->open();
}

DetectMagic::DetectMagic() : SpellView("DetectMagic") {
	_bounds = getLineBounds(20, 24);
}

void DetectMagic::draw() {
	clearSurface();
	escToGoBack(0);

	writeString(0, 0, STRING["dialogs.spells.detect_charges"]);
	_textPos.x = 20;
	_textPos.y = 0;

	Inventory &inv = g_globals->_currCharacter->_backpack;
	for (uint i = 0; i < inv.size(); ++i) {
		writeChar('A' + i);
		writeChar(')');

		int itemId = inv[i]._id;
		bool flag = false;
		if (itemId < 12)
			flag = false;
		else if (itemId < 61)
			flag = true;
		else if (itemId < 66)
			flag = false;
		else if (itemId < 86)
			flag = true;
		else if (itemId < 93)
			flag = false;
		else if (itemId < 121)
			flag = true;
		else if (itemId < 128)
			flag = false;
		else if (itemId < 156)
			flag = true;
		else if (itemId < 158)
			flag = false;
		else if (itemId < 255)
			flag = true;
		else
			flag = false;

		if (flag) {
			writeString("Y (");
			writeNumber(inv[i]._charges);
			writeChar(')');

		} else {
			writeChar('N');
		}

		// Move to write position for next item (if any)
		if (_textPos.x < 30) {
			_textPos.x = 30;
		} else {
			_textPos.x = 20;
			_textPos.y++;
		}
	}
}

bool DetectMagic::msgKeypress(const KeypressMessage &msg) {
	close();
	return true;
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
