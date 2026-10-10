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
#include "ultima/ultima3/data/data.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

const byte CGA_PALETTE1[4 * 3] = {
	0x00, 0x00, 0x00, // 0: black
	0x55, 0xff, 0xff, // 1: light cyan
	0xff, 0x55, 0xff, // 2: light magenta
	0xff, 0xff, 0xff  // 3: white
};

void setCGAPalette() {
	Graphics::Palette palette(4);
	palette.set(CGA_PALETTE1, 0, 4);
	g_system->getPaletteManager()->setPalette(palette);
}

const char SEX_KEYS[] = "MFO";
const char RACE_KEYS[] = "HEDBF";
const char CLASS_KEYS[] = "FCWTPLBDIAR";
const char STATUS_KEYS[] = "GPDA";

const char *const SEX_NAMES[SEX_COUNT] = { "Male", "Female", "Other" };
const char *const RACE_NAMES[RACE_COUNT] = { "Human", "Elf", "Dwarf", "Bobbit", "Fuzzy" };
const char *const CLASS_NAMES[CLASS_COUNT] = {
	"Fighter", "Cleric", "Wizard", "Thief", "Paladin", "Lark", "Barbarian",
	"Druid", "Illusionist", "Alchemist", "Ranger"
};
const char *const STATUS_NAMES[STATUS_COUNT] = { "Good", "Poisoned", "Dead", "Ashes" };
const char *const WEAPON_NAMES[WEAPON_COUNT] = {
	"Hand", "Dagger", "Mace", "Sling", "Axe", "Bow", "Sword", "2-H-Swd",
	"+2 Axe", "+2 Bow", "+2 Swd", "Gloves", "+4 Axe", "+4 Bow", "+4 Swd", "Exotic"
};
const char *const ARMOUR_NAMES[ARMOUR_COUNT] = {
	"Skin", "Cloth", "Leather", "Chain", "Plate", "+2 Chain", "+2 Plate", "Exotic"
};

const char *const NAMES[NAME_COUNT] = {
	"Water", "Grass", "Brush", "Forest", "Mountains", "Dungeon", "Towne",
	"Castle", "Floor", "Chest", "Horse", "Frigate", "Whirlpool", "Serpent",
	"Man-O-War", "Pirate", "Merchant", "Jester", "Guard", "Lord British",
	"Fighter", "Cleric", "Wizard", "Thief", "Orc", "Skeleton", "Giant",
	"Daemon", "Pincher", "Dragon", "Balron", "Exodus", "Force Field", "Lava",
	"Moon Gate", "Wall", "Void", "Wall", "A", "B", "C", "D", "E", "F", "G", "H",
	"I", "U", "Y", "L", "M", "N", "O", "P", "W", "R", "S", "T", "Snake",
	"Snake", "Magic", "Fire", "Shrine", "Ranger", "Hand", "Dagger", "Mace",
	"Sling", "Axe", "Bow", "Sword", "2-H-Swd", "+2 Axe", "+2 Bow", "+2 Swd",
	"Gloves", "+4 Axe", "+4 Bow", "+4 Swd", "Exotic", "Skin", "Cloth",
	"Leather", "Chain", "Plate", "+2 Chain", "+2 Plate", "Exotic", "Repond",
	"Mittar", "Lorum", "Dor Acron", "Sur Acron", "Fulgar", "Dag Acron",
	"Mentar", "Dag Lorum", "Fal Divi", "Noxum", "Decorp", "Altair",
	"Dag Mentar", "Necorp", "", "Pontori", "Appar Unem", "Sanctu", "Luminae",
	"Rec Su", "Rec Du", "Lib Rec", "Alcort", "Sequitu", "Sominae",
	"Sanctu Mani", "Vieda", "Excuun", "Surmandum", "Zxkuqyb", "Anju Sermani",
	"Brigand", "Cutpurse", "Goblin", "Troll", "Ghoul", "Zombie", "Golem",
	"Titan", "Gargoyle", "Mane", "Snatch", "Bradle", "Griffon", "Wyvern",
	"Orcus", "Devil"
};

const byte MONSTER_HIT_POINTS[16] = {
	0x20, 0x20, 0xF0, 0xF0, 0xC0, 0x60, 0xA0, 0x80, 0x30, 0x50, 0x70, 0xA0, 0xC0, 0xE0, 0xF0, 0xF0
};
const byte MONSTER_EXPERIENCE[16] = {
	0x01, 0x02, 0x15, 0x20, 0x08, 0x06, 0x10, 0x05, 0x03, 0x04, 0x06, 0x08, 0x10, 0x15, 0x20, 0x05
};

// Classes in the order the equipment limits are given in
static const char EQUIPMENT_CLASSES[] = "FCWTPBLIDAR";
static const char WEAPON_LIMITS[] = "QDCHQQQDDCL";
static const char ARMOUR_LIMITS[] = "IECDFDCDCCH";

static int equipmentClass(char classKey) {
	const char *pos = classKey ? strchr(EQUIPMENT_CLASSES, classKey) : nullptr;
	return pos ? pos - EQUIPMENT_CLASSES : 0;
}

char weaponLimit(char classKey) {
	return WEAPON_LIMITS[equipmentClass(classKey)];
}

char armourLimit(char classKey) {
	return ARMOUR_LIMITS[equipmentClass(classKey)];
}

byte fightingTile(char classKey) {
	static const char CLASSES[] = "FCWTPBLIDA";
	static const byte TILES[] = { 0x14, 0x15, 0x16, 0x17, 0x14, 0x14, 0x11, 0x16, 0x15, 0x16 };

	const char *pos = classKey ? strchr(CLASSES, classKey) : nullptr;
	return pos ? TILES[pos - CLASSES] : 0x3F;
}

int lookupIndex(char key, const char *keys, int count) {
	for (int i = 0; i < count; ++i) {
		if (keys[i] == key)
			return i;
	}

	return count - 1;
}

} // namespace Data
} // namespace Ultima3
} // namespace Ultima
