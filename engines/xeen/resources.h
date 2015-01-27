/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef XEEN_RESOURCES_H
#define XEEN_RESOURCES_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Xeen {

extern const char *const CREDITS;

extern const char *const OPTIONS_TITLE;

extern const char *const THE_PARTY_NEEDS_REST;

extern const char *const WHO_WILL;

extern const char *const WHATS_THE_PASSWORD;

extern const char *const IN_NO_CONDITION;

extern const char *const NOTHING_HERE;

extern const char *const TERRAIN_TYPES[6];

extern const char *const SURFACE_TYPE_NAMES[15];

extern const char *const SURFACE_NAMES[16];

extern const char *const WHO_ACTIONS[32];

extern const char *const WHO_WILL_ACTIONS[4];

extern const byte SYMBOLS[20][64];

extern const byte TEXT_COLORS[40][4];

extern const char *const DIRECTION_TEXT[4];

extern const char *const RACE_NAMES[5];

extern const int RACE_HP_BONUSES[5];

extern const int RACE_SP_BONUSES[5][2];

extern const char *const CLASS_NAMES[11];

extern const uint CLASS_EXP_LEVELS[10];

extern const char *const ALIGNMENT_NAMES[3];

extern const char *const SEX_NAMES[2];

extern const char *const CONDITION_NAMES[18];

extern const char *const IN_PARTY;

extern const char *const PARTY_DETAILS;

extern const int FACE_CONDITION_FRAMES[17];

extern const int CHAR_FACES_X[6];

extern const int HP_BARS_X[6];

extern const char *const NO_ONE_TO_ADVENTURE_WITH;

extern const byte BACKGROUND_XLAT[];

extern const char *const YOUR_ROSTER_IS_FULL;

extern const char *const PLEASE_WAIT;

extern const char *const OOPS;

extern const int8 SCREEN_POSITIONING_X[4][48];

extern const int8 SCREEN_POSITIONING_Y[4][48];

extern const int INDOOR_OBJECT_X[2][12];

extern const int MAP_OBJECT_Y[2][12];

extern const int INDOOR_MONSTERS_Y[4];

extern const int OUTDOOR_OBJECT_X[2][12];

extern const int OUTDOOR_MONSTER_INDEXES[26];

extern const int OUTDOOR_MONSTERS_Y[26];

extern const int DIRECTION_ANIM_POSITIONS[4][4];

extern const byte WALL_SHIFTS[4][48];

extern const int DRAW_NUMBERS[25];

extern const int DRAW_FRAMES[25][2];

extern const int COMBAT_FLOAT_X[8];

extern const int COMBAT_FLOAT_Y[8];

extern const int MONSTER_EFFECT_FLAGS[15][8];

extern const uint SPELLS_ALLOWED[3][40];

extern const int BASE_HP_BY_CLASS[10];

extern const int AGE_RANGES[10];

extern const int AGE_RANGES_ADJUST[2][10];

extern const int STAT_VALUES[24];

extern const int STAT_BONUSES[24];

extern const int ELEMENTAL_CATEGORIES[6];

extern const int ATTRIBUTE_CATEGORIES[10];

extern const int ATTRIBUTE_BONUSES[72];

extern const int ELEMENTAL_RESISTENCES[37];

extern const int ELEMENTAL_DAMAGE[37];

extern const int METAL_LAC[9];

extern const int ARMOR_STRENGTHS[14];

extern const int OUTDOOR_DRAWSTRCT_INDEXES[44];

extern const int TOWN_MAXES[2][11];

extern const char *const TOWN_ACTION_MUSIC[14];

extern const char *const TOWN_ACTION_SHAPES[4];

extern const int TOWN_ACTION_FILES[2][7];

extern const char *const BANK_TEXT;

extern const char *const BLACKSMITH_TEXT;

extern const char *const GUILD_NOT_MEMBER_TEXT;

extern const char *const GUILD_TEXT;

extern const char *const TAVERN_TEXT;

extern const char *const FOOD_AND_DRINK;

extern const char *const TEMPLE_TEXT;

extern const char *const EXPERIENCE_FOR_LEVEL;

extern const char *const LEARNED_ALL;

extern const char *const ELIGIBLE_FOR_LEVEL;

extern const char *const TRAINING_TEXT;

extern const char *const GOLD_GEMS;

extern const char *const DEPOSIT_WITHDRAWL[2];

extern const char *const NOT_ENOUGH_X_IN_THE_Y;

extern const char *const NO_X_IN_THE_Y;

extern const char *const STAT_NAMES[16];

extern const char *const CONSUMABLE_NAMES[4];

extern const char *const WHERE_NAMES[2];

extern const char *const AMOUNT;

} // End of namespace Xeen

#endif	/* XEEN_RESOURCES_H */
