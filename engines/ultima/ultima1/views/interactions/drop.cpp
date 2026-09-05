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

#include "ultima/ultima1/views/interactions/drop.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

constexpr int COLOR_TEXT = 11; // light cyan

// Returns how many of items[1..maxIndex] are actually owned (index 0,
// the innate/default state, is never droppable)
static int countOwned(const int16 *items, int maxIndex) {
	int count = 0;
	for (int idx = 1; idx <= maxIndex; ++idx) {
		if (items[idx])
			++count;
	}

	return count;
}

Drop::Drop() : Interaction("Drop") {
}

bool Drop::msgFocus(const FocusMessage &msg) {
	_mode = DROP_AREA;
	setBounds(Common::Rect(0, 0, 0, 0));
	writeString("Drop Pence,Weapon,Armour:");
	showCursor();
	return true;
}

bool Drop::msgAction(const ActionMessage &msg) {
	none();
	return true;
}

bool Drop::msgKeypress(const KeypressMessage &msg) {
	switch (_mode) {
	case DROP_AREA:
		switch (msg.keycode) {
		case Common::KEYCODE_p:
			_mode = DROP_PENCE;
			_amount.clear();
			resetLine();
			writeString("\x10""Drop pence: ");
			showCursor();
			break;

		case Common::KEYCODE_w:
			if (!countOwned(_G(savegame)._weapons, Data::WEAPON_COUNT - 1)) {
				none();
				break;
			}

			_mode = DROP_WEAPON;
			resetLine();
			writeString("\x10""Drop weapon: ");
			showCursor();

			// Draw a proper dialog with the list of selections
			setBounds(TextRect(0, 0, 39, 19));
			redraw();
			break;

		case Common::KEYCODE_a:
			if (!countOwned(_G(savegame)._armor, Data::ARMOR_COUNT - 1)) {
				none();
				break;
			}

			_mode = DROP_ARMOUR;
			resetLine();
			writeString("\x10""Drop armour: ");
			showCursor();

			// Draw a proper dialog with the list of selections
			setBounds(TextRect(0, 0, 39, 19));
			redraw();
			break;

		default:
			// Everything else aborts drop
			none();
			break;
		}
		break;

	case DROP_PENCE:
		penceKeypress(msg);
		break;

	case DROP_WEAPON:
		selectItem(_G(savegame)._weapons, Data::WEAPON_NAMES, Data::WEAPON_COUNT - 1,
			_G(savegame)._equippedWeapon, msg.keycode);
		break;

	case DROP_ARMOUR:
		selectItem(_G(savegame)._armor, Data::ARMOR_NAMES, Data::ARMOR_COUNT - 1,
			_G(savegame)._equippedArmor, msg.keycode);
		break;

	default:
		break;
	}

	return true;
}

void Drop::draw() {
	// No initial drawing when waiting for drop area, or during pence
	// entry, which stays entirely within the Commands text area
	if (_mode == DROP_AREA || _mode == DROP_PENCE)
		return;

	// Display title
	auto s = getSurface();
	s.writeString(Point(16, 0), "\x10 Drop \x11");

	// Clear dialog content
	s.fillRect(TextRect(1, 1, 38, 18), 0);

	switch (_mode) {
	case DROP_WEAPON:
		drawItemList(_G(savegame)._weapons, Data::WEAPON_NAMES, Data::WEAPON_COUNT - 1, 15);
		break;
	case DROP_ARMOUR:
		drawItemList(_G(savegame)._armor, Data::ARMOR_NAMES, Data::ARMOR_COUNT - 1, 13);
		break;
	default:
		break;
	}
}

void Drop::drawItemList(const int16 *items, const char *const *names, int maxIndex, int col) {
	auto s = getSurface();
	int count = countOwned(items, maxIndex);
	int y = 10 - count / 2;

	s.setColor(COLOR_TEXT);
	for (int idx = 1; idx <= maxIndex; ++idx) {
		if (items[idx]) {
			s.writeString(Point(col, y), "%c) %s", 'a' + idx, names[idx]);
			++y;
		}
	}
}

void Drop::selectItem(int16 *items, const char *const *names, int maxIndex,
		int16 &equippedIndex, Common::KeyCode keycode) {
	int idx = keycode - Common::KEYCODE_a;

	// Unlike Ready, an out of range or unowned choice just drops nothing,
	// rather than falling back to a previous selection
	if (idx < 1 || idx > maxIndex || !items[idx]) {
		writeString("none");
	} else {
		writeString(names[idx]);
		--items[idx];

		// If the last of the currently readied one was just dropped, unready it
		if (idx == equippedIndex && items[idx] == 0)
			equippedIndex = 0;
	}

	finish();
}

void Drop::penceKeypress(const KeypressMessage &msg) {
	if (msg.ascii >= '0' && msg.ascii <= '9') {
		// No leading zero, and no more than 4 digits
		if ((_amount.empty() && msg.ascii == '0') || _amount.size() >= 4)
			return;

		_amount += (char)msg.ascii;
		writeString(Common::String(1, (char)msg.ascii));

	} else if (msg.keycode == Common::KEYCODE_BACKSPACE) {
		if (!_amount.empty()) {
			_amount.deleteLastChar();
			resetLine();
			writeString(Common::String::format("\x10""Drop pence: %s", _amount.c_str()));
			showCursor();
		}

	} else {
		// Any other key finalizes the amount entered so far
		dropPence();
	}
}

void Drop::dropPence() {
	auto &sg = _G(savegame);
	int total = _amount.empty() ? 0 : atoi(_amount.c_str());

	if (!total) {
		writeString("none");
		finish();
		return;
	}

	// Blank line, moving past the entered amount
	writeString("\n");

	if (total > sg._coins) {
		writeString("Thou hast not that much!");
		g_engine->playFX(1);
		finish();
		return;
	}

	sg._coins -= total;
	redrawStats();

	if (_G(map)._mapType == Data::MAPTYPE_CASTLE)
		dropPenceCastleBonus(total);
	else
		dropPenceCityBonus(total);

	finish();
}

void Drop::dropPenceCityBonus(int total) {
	auto &sg = _G(savegame);
	int tile = _G(map).getTileAt(sg._locationPosition.x, sg._locationPosition.y);

	if (tile != Data::CTILE_DROP1) {
		writeString("Ok!");
		return;
	}

	sg._hits += (total * 3) / 2;
	redrawStats();

	if (g_engine->getRandomNumber(1, 255) > 16) {
		writeString("Shazam!");
	} else {
		// WORKAROUND: The original computes this random roll but then (apparently by mistake)
		// indexes its bonus array by the pence amount dropped instead, which would read/write
		// out of bounds for anything but a tiny amount. Applying the roll to the spell it clearly
		// intended to pick instead
		int spellIdx = g_engine->getRandomNumber(1, 7);
		if (spellIdx == 3)
			spellIdx = 4;
		++sg._spells[spellIdx];
		writeString("Alakazot!");
	}

	g_engine->playFX(5);
}

void Drop::dropPenceCastleBonus(int total) {
	auto &sg = _G(savegame);
	int tile = _G(map).getTileAt(sg._locationPosition.x, sg._locationPosition.y);

	if (tile != Data::CTILE_DROP1 && tile != Data::CTILE_DROP2 && tile != Data::CTILE_DROP3) {
		writeString("Ok!");
		return;
	}

	writeString("Shazam!");
	g_engine->playFX(5);

	if (tile == Data::CTILE_DROP3) {
		sg._food += total * 3;
		redrawStats();

	} else if (tile == Data::CTILE_DROP2) {
		if (total >= 10) {
			int weaponIdx = g_engine->getRandomNumber(1, 15);
			if (sg._weapons[weaponIdx] < 255)
				++sg._weapons[weaponIdx];
		}

	} else {
		// CTILE_51 - boost a random attribute (never Hits), capped at 99
		int attrIdx = g_engine->getRandomNumber(1, 6);
		int16 *attr;
		switch (attrIdx) {
		case 1: attr = &sg._strength; break;
		case 2: attr = &sg._agility; break;
		case 3: attr = &sg._stamina; break;
		case 4: attr = &sg._charisma; break;
		case 5: attr = &sg._wisdom; break;
		default: attr = &sg._intelligence; break;
		}

		*attr += total / 10;
		if (*attr > 99)
			*attr = 99;
	}
}

void Drop::none() {
	writeString("nothing");
	finish();
}

void Drop::finish() {
	writeString("\n");
	prompt();
	close();
}

void Drop::redrawStats() {
	g_engine->findView("Stats")->draw();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
