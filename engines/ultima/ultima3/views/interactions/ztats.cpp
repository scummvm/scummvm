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

#include "ultima/ultima3/views/interactions/ztats.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

// The marks and cards a character can hold, from the highest bit down
static const char *const MARK_NAMES[8] = {
	"Mark of Kings", "Mark of Snake", "Mark of Fire", "Mark of Force",
	"Card of Death", "Card of Moons", "Card of Sol", "Card of Love"
};

Ztats::~Ztats() {
	_G(effects)._highlight = -1;
}

void Ztats::buildPages(const Data::RosterEntry &e) {
	using Common::String;
	_pages.clear();

	_pages.push_back(String::format("%s\nStr...%02X\nDex...%02X\nInt...%02X\nWis...%02X\nH.P...%04X",
		e._name, e._strength, e._dexterity, e._intelligence, e._wisdom, e._hitPoints));
	_pages.push_back(String::format("\nH.M...%04X", e._maxHitPoints));
	_pages.push_back(String::format("\nGold: %04X", e._gold));
	_pages.push_back(String::format("\nExp...%04X", e._experience));
	_pages.push_back(String::format("\nGems..%02X", e._gems));
	_pages.push_back(String::format("\nKeys..%02X", e._keys));
	_pages.push_back(String::format("\nPowd..%02X", e._powder));
	_pages.push_back(String::format("\nTrch..%02X", e._torches));

	for (int bit = 0; bit < 8; ++bit) {
		if (e._marksAndCards & (0x80 >> bit))
			_pages.push_back(String("\n") + MARK_NAMES[bit]);
	}

	_pages.push_back(String("\nWeapon:") + Data::WEAPON_NAMES[e._weaponIndex]);
	_pages.push_back(String("\nArmour:") + Data::ARMOUR_NAMES[e._armourIndex]);

	// The headings are shown along with the first item under them, and the
	// fixed hands and skin entries aren't waited on
	String pending = "\n***Weapons***";
	for (int i = Data::WEAPON_COUNT - 1; i >= 1; --i) {
		if (e._weaponOwned[i - 1] == 0)
			continue;

		_pages.push_back(pending + String::format("\n%02X-%s-(%c)", e._weaponOwned[i - 1],
			Data::WEAPON_NAMES[i], 'A' + i));
		pending.clear();
	}

	pending += "\n02-Hands-(A)\n**Armour**";
	for (int i = Data::ARMOUR_COUNT - 1; i >= 1; --i) {
		if (e._armourOwned[i - 1] == 0)
			continue;

		_pages.push_back(pending + String::format("\n%02X-%s-(%c)", e._armourOwned[i - 1],
			Data::ARMOUR_NAMES[i], 'A' + i));
		pending.clear();
	}

	_tail = pending + "\n01-Skin-(A)\n";
}

void Ztats::finish() {
	_G(effects)._highlight = -1;
}

bool Ztats::keypress(const KeypressMessage &msg) {
	if (!_showing) {
		PlayerChooser::Result result = _chooser.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		_showing = true;
		_G(effects)._highlight = _chooser.slot();
		buildPages(_G(savegame).partyMember(_chooser.slot()));
		_G(messages).print(_pages[0].c_str());
		return false;
	}

	switch (msg.keycode) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_SPACE:
		break;
	case Common::KEYCODE_ESCAPE:
		_G(messages).print("\n");
		finish();
		return true;
	default:
		return false;
	}

	if (++_page < _pages.size()) {
		_G(messages).print(_pages[_page].c_str());
		return false;
	}

	_G(messages).print(_tail.c_str());
	finish();
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
