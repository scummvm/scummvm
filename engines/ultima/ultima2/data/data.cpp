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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "ultima/ultima2/data/data.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

const byte CGA_PALETTE1[4 * 3] = {
	0x00, 0x00, 0x00, // 0: black
	0x55, 0xff, 0xff, // 1: light cyan
	0xff, 0x55, 0xff, // 2: light magenta
	0xff, 0xff, 0xff  // 3: white
};

const char *const PLANET_NAMES[PLANET_COUNT] = {
	"EARTH", "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN", "URANUS", "NEPTUNE", "PLUTO"
};

const byte PLANET_COORDS[PLANET_COUNT + 1][3] = {
	{ 6, 6, 6 }, { 5, 4, 5 }, { 3, 3, 4 }, { 6, 2, 3 }, { 1, 3, 4 },
	{ 2, 8, 5 }, { 9, 4, 6 }, { 4, 0, 5 }, { 0, 1, 4 }, { 9, 9, 9 }
};

void setCGAPalette() {
	Graphics::Palette palette(PALETTE_PLAYER_MARKER + 1);
	palette.set(CGA_PALETTE1, 0, 4);

	const byte green[3] = { 0x55, 0xff, 0x55 };
	palette.set(green, PALETTE_PLAYER_MARKER, 1);
	g_system->getPaletteManager()->setPalette(palette);
}

const char *const RACE_NAMES[RACE_COUNT] = {
	"HUMAN", "ELF", "DWARF", "HOBBIT"
};

const char *const CLASS_NAMES[CLASS_COUNT] = {
	"FIGHTER", "CLERIC", "WIZARD", "THIEF"
};

const char *const WEAPON_NAMES[WEAPON_COUNT] = {
	"HANDS", "DAGGER", "MACE", "AXE", "BOW", "SWORD",
	"GREAT SWORD", "LIGHT SWORD", "PHASER", "QUICK SWORD"
};

const char *const ARMOR_NAMES[ARMOR_COUNT] = {
	"SKIN", "CLOTH", "LEATHER", "CHAIN", "PLATE", "REFLECT", "POWER"
};

const char *const SPELL_NAMES[SPELL_COUNT] = {
	"NONE", "LIGHT", "DOWN LADDER", "UP LADDER", "PASSWALL",
	"SURFACE", "PRAYER", "MAGIC MISSILE", "BLINK", "KILL"
};

const char *const ITEM_NAMES[ITEM_COUNT] = {
	"RING", "WAND", "STAFF", "BOOTS", "CLOAK", "HELM", "GEM", "ANKH",
	"RED GEM", "SKULL KEY", "GREEN GEM", "BRASS BUTTON", "BLUE TASSLE",
	"STRANGE COIN", "GREEN IDOL", "TRI LITHIUM"
};

const char *raceName(Race race) {
	return (race >= 0 && race < RACE_COUNT) ? RACE_NAMES[race] : "";
}

const char *className(CharClass charClass) {
	return (charClass >= 0 && charClass < CLASS_COUNT) ? CLASS_NAMES[charClass] : "";
}

const char *weaponName(WeaponType weapon) {
	return (weapon >= 0 && weapon < WEAPON_COUNT) ? WEAPON_NAMES[weapon] : "";
}

const char *armorName(ArmorType armor) {
	return (armor >= 0 && armor < ARMOR_COUNT) ? ARMOR_NAMES[armor] : "";
}

const char *spellName(SpellType spell) {
	return (spell >= 0 && spell < SPELL_COUNT) ? SPELL_NAMES[spell] : "";
}

const char *itemName(int item) {
	return (item >= 0 && item < ITEM_COUNT) ? ITEM_NAMES[item] : "";
}

} // namespace Data
} // namespace Ultima2
} // namespace Ultima
