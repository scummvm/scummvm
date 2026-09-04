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

#include "ultima/ultima1/views/interactions/ready.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

Ready::Ready() : Interaction("Ready") {
}

bool Ready::msgFocus(const FocusMessage &msg) {
	// Start by getting them to select the area
	_mode = READY_AREA;
	setBounds(Common::Rect(0, 0, 0, 0));
	writeString("Ready Weapon,Armour,Spell:");
	showCursor();
	return true;
}

bool Ready::msgAction(const ActionMessage &msg) {
	nothing();
	return true;
}

bool Ready::msgKeypress(const KeypressMessage &msg) {
	switch (_mode) {
	case READY_AREA:
		switch (msg.keycode) {
		case Common::KEYCODE_w:
			// Ready weapon
			_mode = READY_WEAPON;
			resetLine();
			writeString("\x10Ready weapon: ");
			showCursor();

			// Draw a proper dialog with the list of selections
			setBounds(TextRect(0, 0, 39, 19));
			redraw();
			break;

		case Common::KEYCODE_a:
			// Ready armor
			_mode = READY_ARMOUR;
			resetLine();
			writeString("\x10Ready armour: ");
			showCursor();

			// Draw a proper dialog with the list of selections
			setBounds(TextRect(0, 0, 39, 19));
			redraw();
			break;

		case Common::KEYCODE_s:
			// Ready spell
			_mode = READY_SPELL;
			resetLine();
			writeString("\x10Ready spell:  ");
			showCursor();

			// Draw a proper dialog with the list of selections
			setBounds(TextRect(0, 0, 39, 19));
			redraw();
			break;

		default:
			// Everything else aborts ready
			nothing();
			break;
		}
		break;

	case READY_WEAPON:
		break;

	case READY_ARMOUR:
		break;

	case READY_SPELL:
	default:
		break;
	}

	return true;
}

void Ready::draw() {
	// No initial drawing when waiting for ready area
	if (_mode == READY_AREA)
		return;

	// Display title
	auto s = getSurface();
	s.writeString(Point(16, 0), "\x10 Ready \x11");

	// Clear dialog content
	s.fillRect(TextRect(1, 1, 38, 18), 0);

	// Draw list of selections
}

void Ready::nothing() {
	resetLine();
	writeString("\x10Ready nothing\n");
	prompt();
	close();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
