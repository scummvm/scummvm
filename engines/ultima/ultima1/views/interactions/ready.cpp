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
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

constexpr int COLOR_TEXT = 11; // light cyan

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
		selectItem(_G(savegame)._weapons, Data::WEAPON_NAMES, Data::WEAPON_COUNT - 1,
			_G(savegame)._equippedWeapon, "weapon", msg.keycode);
		break;

	case READY_ARMOUR:
		selectItem(_G(savegame)._armor, Data::ARMOR_NAMES, Data::ARMOR_COUNT - 1,
			_G(savegame)._equippedArmor, "armour", msg.keycode);
		break;

	case READY_SPELL:
		selectItem(_G(savegame)._spells, Data::SPELL_NAMES, Data::SPELL_COUNT - 1,
			_G(savegame)._equippedSpell, "spell", msg.keycode);
		break;

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
	switch (_mode) {
	case READY_WEAPON:
		drawItemList(_G(savegame)._weapons, Data::WEAPON_NAMES, Data::WEAPON_COUNT - 1);
		break;
	case READY_ARMOUR:
		drawItemList(_G(savegame)._armor, Data::ARMOR_NAMES, Data::ARMOR_COUNT - 1);
		break;
	case READY_SPELL:
		drawItemList(_G(savegame)._spells, Data::SPELL_NAMES, Data::SPELL_COUNT - 1);
		break;
	default:
		break;
	}
}

void Ready::drawItemList(const int16 *items, const char *const *names, int maxIndex) {
	auto s = getSurface();

	// Count the owned items so the list can be vertically centered
	int count = 0;
	for (int idx = 0; idx <= maxIndex; ++idx) {
		if (items[idx])
			++count;
	}

	int y = 10 - count / 2;
	s.setColor(COLOR_TEXT);

	for (int idx = 0; idx <= maxIndex; ++idx) {
		if (items[idx]) {
			s.writeString(Point(15, y), "%c) %s", 'a' + idx, names[idx]);
			++y;
		}
	}
}

void Ready::selectItem(const int16 *items, const char *const *names, int maxIndex,
		int16 &equippedIndex, const char *label, Common::KeyCode keycode) {
	int idx = keycode - Common::KEYCODE_a;

	// An out of range letter, or one for an item not owned, leaves
	// whatever was already readied unchanged
	if (idx < 0 || idx > maxIndex || !items[idx])
		idx = equippedIndex;

	equippedIndex = idx;

	// Display readied selection
	Common::String msg = Common::String::format("%s: ", label);
	if (_mode == READY_SPELL)
		msg += ' ';
	msg += names[idx];
	ready(msg.c_str());
}

void Ready::nothing() {
	ready("nothing");
}

void Ready::ready(const char *name) {
	resetLine();
	writeString("\x10Ready %s\n", name);
	prompt();
	close();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
