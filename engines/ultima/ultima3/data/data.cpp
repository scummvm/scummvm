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
