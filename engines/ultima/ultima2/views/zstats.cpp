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

#include "ultima/ultima2/views/zstats.h"
#include "ultima/ultima2/gfx/text.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

ZStats::ZStats() : View("ZStats") {
	setBounds(Graphics::Views::TextRect(0, 0, 39, 24));
}

bool ZStats::msgFocus(const FocusMessage &msg) {
	// Clear the entire screen
	setBounds(Graphics::Views::TextRect(0, 0, 39, 24));
	getSurface().clear();

	// Set up a slightly reduced bounds to exclude columns 0 and 39
	setBounds(Graphics::Views::TextRect(1, 0, 38, 24));
	return Graphics::Views::View::msgFocus(msg);
}

void ZStats::draw() {
	auto s = getSurface();
	s.clear();

	Data::Savegame &sg = _G(savegame);

	// Write text in one of the colors, at a position or from where the last text ended
	auto put = [&](Gfx::TextColor color, const Common::String &text) {
		s.setColor(Gfx::textColor(color));
		s.writeString(text);
	};
	auto putAt = [&](int x, int y, Gfx::TextColor color, const Common::String &text) {
		s.setTextPos(Common::Point(x, y));
		put(color, text);
	};
	auto number = [](int value) {
		return Common::String::format("%.2d", value);
	};

	// A label followed by a number, ending at the right edge when x is negative
	auto labelled = [&](int x, int y, const Common::String &label, int value) {
		if (x < 0)
			x = 38 - (int)(label.size() + 2);
		putAt(x, y, Gfx::TEXT_NORMAL, label);
		put(Gfx::TEXT_NUMBER, number(value));
	};

	putAt(0, 0, Gfx::TEXT_NAME, sg._name);
	putAt(0, 1, Gfx::TEXT_NAME, Common::String::format("A Level %.2d %s %s %s", sg._experience,
		sg._sex == Data::SEX_MALE ? "Male" : "Female",
		Data::raceName(sg._race),
		Data::className(sg._class)));

	putAt(2, 4, Gfx::TEXT_NORMAL, "Weapon-");
	put(Gfx::TEXT_LIST, Data::weaponName(sg._readiedWeapon));
	putAt(2, 5, Gfx::TEXT_NORMAL, "Armour-");
	put(Gfx::TEXT_LIST, Data::armorName(sg._readiedArmor));
	putAt(2, 6, Gfx::TEXT_NORMAL, " Spell-");
	put(Gfx::TEXT_LIST, Data::spellName(sg._readiedSpell));

	labelled(-1, 4, "Torches-", sg._torches);
	labelled(-1, 5, "Keys-", sg._keys);
	labelled(-1, 6, "Tools-", sg._thievesTools);

	labelled(0, 7, "Strength-", sg._strength);
	labelled(14, 7, " Stamina-", sg._stamina);
	labelled(-1, 7, " Wisdom-", sg._wisdom);
	labelled(0, 8, " Agility-", sg._agility);
	labelled(14, 8, "Charisma-", sg._charisma);
	labelled(-1, 8, "Intell.-", sg._intelligence);

	int row = 10;
	putAt(0, row, Gfx::TEXT_HEADING, "Weapons: ");
	for (int i = 0; i < Data::WEAPON_COUNT; ++i) {
		if (sg._weaponOwned[i] != 0) {
			put(Gfx::TEXT_NORMAL, Common::String::format("%ss-", Data::weaponName((Data::WeaponType)i)));
			put(Gfx::TEXT_NUMBER, number(sg._weaponOwned[i]));
			put(Gfx::TEXT_NORMAL, " ");
		}
	}
	row += 3; // two blank rows below WEAPONS

	putAt(0, row, Gfx::TEXT_HEADING, "Armour: ");
	for (int i = 0; i < Data::ARMOR_COUNT; ++i) {
		if (sg._armorOwned[i] != 0) {
			put(Gfx::TEXT_NORMAL, Common::String::format("%s-", Data::armorName((Data::ArmorType)i)));
			put(Gfx::TEXT_NUMBER, number(sg._armorOwned[i]));
			put(Gfx::TEXT_NORMAL, " ");
		}
	}
	row += 2; // one blank row below ARMOUR

	putAt(0, row, Gfx::TEXT_HEADING, "Spells: ");
	for (int i = 0; i < Data::SPELL_COUNT; ++i) {
		if (sg._spellCharges[i] != 0) {
			put(Gfx::TEXT_NORMAL, Common::String::format("%ss-", Data::spellName((Data::SpellType)i)));
			put(Gfx::TEXT_NUMBER, number(sg._spellCharges[i]));
			put(Gfx::TEXT_NORMAL, " ");
		}
	}
	row += 3; // two blank rows below SPELLS

	putAt(0, row, Gfx::TEXT_HEADING, "Items: ");
	for (int i = 0; i < Data::ITEM_COUNT; ++i) {
		if (sg._items[i] != 0) {
			put(Gfx::TEXT_NORMAL, Common::String::format("%s%s-", Data::itemName(i),
				i == Data::ITEM_BOOTS ? "" : "s"));
			put(Gfx::TEXT_NUMBER, number(sg._items[i]));
			put(Gfx::TEXT_NORMAL, " ");
		}
	}

	// Show the blinking cursor
	_textCursor.setPosition(s.getTextPos());
	_textCursor.show();
}

bool ZStats::msgKeypress(const KeypressMessage &msg) {
	close();
	_G(logic)->resumeTurn();
	return true;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
