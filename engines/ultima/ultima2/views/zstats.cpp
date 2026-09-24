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
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

constexpr int COLOR_TEXT = 3;

static const char *weaponName(Data::WeaponType w) {
	return (w >= 0 && w < Data::WEAPON_COUNT) ? Data::WEAPON_NAMES[w] : "";
}

static const char *armorName(Data::ArmorType a) {
	return (a >= 0 && a < Data::ARMOR_COUNT) ? Data::ARMOR_NAMES[a] : "";
}

static const char *spellName(Data::SpellType s) {
	return (s >= 0 && s < Data::SPELL_COUNT) ? Data::SPELL_NAMES[s] : "";
}

static const char *itemName(int i) {
	return (i >= 0 && i < Data::ITEM_COUNT) ? Data::ITEM_NAMES[i] : "";
}

static const char *raceName(Data::Race race) {
	switch (race) {
	case Data::RACE_HUMAN: return "HUMAN";
	case Data::RACE_ELF: return "ELF";
	case Data::RACE_DWARF: return "DWARF";
	case Data::RACE_HOBBIT: return "HOBBIT";
	default: return "";
	}
}

static const char *className(Data::CharClass charClass) {
	switch (charClass) {
	case Data::CLASS_FIGHTER: return "FIGHTER";
	case Data::CLASS_CLERIC: return "CLERIC";
	case Data::CLASS_WIZARD: return "WIZARD";
	case Data::CLASS_THIEF: return "THIEF";
	default: return "";
	}
}

ZStats::ZStats() : View("ZStats") {
	setBounds(TextRect(0, 0, 39, 24));
}

bool ZStats::msgFocus(const FocusMessage &msg) {
	// Clear the entire screen
	setBounds(TextRect(0, 0, 39, 24));
	getSurface().clear();

	// Set up a slightly reduced bounds to exclude columns 0 and 39
	setBounds(TextRect(1, 0, 38, 24));
	return Shared::Gfx::View::msgFocus(msg);
}

void ZStats::draw() {
	auto s = getSurface();
	s.clear();
	s.setColor(COLOR_TEXT);

	Data::Savegame &sg = _G(savegame);

	s.writeString(Common::Point(0, 0), "%s", sg._name);
	s.writeString(Common::Point(0, 1), "A LEVEL %.2d %s %s %s", sg._experience,
		sg._sex == Data::SEX_MALE ? "MALE" : "FEMALE",
		raceName(sg._race),
		className(sg._class));

	s.writeString(Common::Point(2, 4), "WEAPON-%s", weaponName(sg._readiedWeapon));
	s.writeString(Common::Point(2, 5), "ARMOUR-%s", armorName(sg._readiedArmor));
	s.writeString(Common::Point(2, 6), " SPELL-%s", spellName(sg._readiedSpell));

	s.writeString(Common::Point(38, 4), Common::String::format("TORCHES-%.2d", sg._torches), Graphics::kTextAlignRight);
	s.writeString(Common::Point(38, 5), Common::String::format("KEYS-%.2d", sg._keys), Graphics::kTextAlignRight);
	s.writeString(Common::Point(38, 6), Common::String::format("TOOLS-%.2d", sg._thievesTools), Graphics::kTextAlignRight);

	s.writeString(Common::Point(0, 7), "STRENGTH-%.2d", sg._strength);
	s.writeString(Common::Point(14, 7), " STAMINA-%.2d", sg._stamina);
	s.writeString(Common::Point(38, 7), Common::String::format(" WISDOM-%.2d", sg._wisdom), Graphics::kTextAlignRight);
	s.writeString(Common::Point(0, 8), " AGILITY-%.2d", sg._agility);
	s.writeString(Common::Point(14, 8), "CHARISMA-%.2d", sg._charisma);
	s.writeString(Common::Point(38, 8), Common::String::format("INTELL.-%.2d", sg._intelligence), Graphics::kTextAlignRight);

	int row = 10;
	Common::String line = "WEAPONS: ";
	for (int i = 0; i < Data::WEAPON_COUNT; ++i) {
		if (sg._weaponOwned[i] != 0)
			line += Common::String::format("%sS-%.2d ", weaponName((Data::WeaponType)i), sg._weaponOwned[i]);
	}
	s.writeString(Common::Point(0, row), line);
	row += 3; // two blank rows below WEAPONS

	line = "ARMOUR: ";
	for (int i = 0; i < Data::ARMOR_COUNT; ++i) {
		if (sg._armorOwned[i] != 0)
			line += Common::String::format("%s-%.2d ", armorName((Data::ArmorType)i), sg._armorOwned[i]);
	}
	s.writeString(Common::Point(0, row), line);
	row += 2; // one blank row below ARMOUR

	line = "SPELLS: ";
	for (int i = 0; i < Data::SPELL_COUNT; ++i) {
		if (sg._spellCharges[i] != 0)
			line += Common::String::format("%sS-%.2d ", spellName((Data::SpellType)i), sg._spellCharges[i]);
	}
	s.writeString(Common::Point(0, row), line);
	row += 3; // two blank rows below SPELLS

	line = "ITEMS: ";
	for (int i = 0; i < Data::ITEM_COUNT; ++i) {
		if (sg._items[i] != 0) {
			line += Common::String::format("%s%s-%.2d ", itemName(i),
				i == Data::ITEM_BOOTS ? "" : "S", sg._items[i]);
		}
	}
	s.writeString(Common::Point(0, row), line);

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
