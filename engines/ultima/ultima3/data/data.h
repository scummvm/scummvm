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

#ifndef ULTIMA3_DATA_DATA_H
#define ULTIMA3_DATA_DATA_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

// The original CGA 4-color palette, used by the original and composite modes
extern const byte CGA_PALETTE1[4 * 3];

// The renderers from the Ultima III Upgrade patch
enum RenderMode {
	RENDER_CGA = 0, RENDER_CGA_COMPOSITE = 1, RENDER_EGA = 2, RENDER_VGA = 3,
	RENDER_MODE_COUNT = 4
};

// Descriptions of the modes for the launcher's graphics mode drop-down, in
// the same order. Marked for translation, but not translated
extern const char *const RENDER_MODE_LABELS[RENDER_MODE_COUNT];

// EGA and VGA draw everything from the upgrade's coloured graphics files
inline bool isEnhancedMode(RenderMode mode) {
	return mode == RENDER_EGA || mode == RENDER_VGA;
}

// The composite renderer's 16 artifact colors, loaded from CGACOMP.PAL, are
// kept clear of the CGA colors. Gfx::compositeDecode maps each decoded CGA
// pixel to one of these
constexpr int COMPOSITE_PALETTE_BASE = 32;
constexpr int COMPOSITE_PALETTE_SIZE = 16;

// The colors the game draws with, which differ between the modes
inline byte whiteColor(RenderMode mode) {
	return isEnhancedMode(mode) ? 15 : 3;
}

// The color of the frame around the screens in the EGA renderer, which is made of solid cells
constexpr byte EGA_FRAME_COLOR = 9;

// What the pixels flipped to show something inverted are XORed with
inline byte invertMask(RenderMode mode) {
	return isEnhancedMode(mode) ? 0x0F : 3;
}

/**
 * Sets the screen palette for a mode, which is the CGA colors, the EGA colors
 * or the VGA palette file
 */
void setPalette(RenderMode mode);

// The letters a character's sex, race and class are stored as, with the
// names shown for each
constexpr int SEX_COUNT = 3;
constexpr int RACE_COUNT = 5;
constexpr int CLASS_COUNT = 11;
constexpr int STATUS_COUNT = 4;
constexpr int WEAPON_COUNT = 16;
constexpr int ARMOUR_COUNT = 8;
constexpr int NAME_COUNT = 136;

extern const char SEX_KEYS[];
extern const char RACE_KEYS[];
extern const char CLASS_KEYS[];
extern const char STATUS_KEYS[];
extern const char *const SEX_NAMES[SEX_COUNT];
extern const char *const RACE_NAMES[RACE_COUNT];
extern const char *const CLASS_NAMES[CLASS_COUNT];
extern const char *const STATUS_NAMES[STATUS_COUNT];
extern const char *const WEAPON_NAMES[WEAPON_COUNT];
extern const char *const ARMOUR_NAMES[ARMOUR_COUNT];

// The game's table of terrain, creature, equipment and spell names, which
// are referred to by a number starting at 1
extern const char *const NAMES[NAME_COUNT];

// The most hit points each kind of monster starts out with, and the
// experience it gives (in BCD) when killed, found from the low four bits of
// its tile number
extern const byte MONSTER_HIT_POINTS[16];
extern const byte MONSTER_EXPERIENCE[16];

/**
 * Returns the letter after those of the weapons and armour that a class may
 * use, apart from the exotic ones which anyone may
 */
char weaponLimit(char classKey);
char armourLimit(char classKey);

/**
 * Returns the tile showing a party member of a class in a fight
 */
byte fightingTile(char classKey);

/**
 * Returns the index of a key within the first count entries of a key list.
 * As in the original, a key that isn't present maps to the last entry.
 */
int lookupIndex(char key, const char *keys, int count);

} // namespace Data
} // namespace Ultima3
} // namespace Ultima

#endif
