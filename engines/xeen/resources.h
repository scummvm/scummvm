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
#include "common/str-array.h"
#include "gui/debugger.h"
#include "xeen/party.h"
#include "xeen/spells.h"

namespace Xeen {

class Resources {
public:
	SpriteResource _globalSprites;
	Common::StringArray _maeNames;		// Magic and equipment names
public:
	Resources();
};

#define Res (*_vm->_resources)

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

extern const char *const DIRECTION_TEXT_UPPER[4];

extern const char *const DIRECTION_TEXT[4];

extern const char *const RACE_NAMES[5];

extern const int RACE_HP_BONUSES[5];

extern const int RACE_SP_BONUSES[5][2];

extern const char *const CLASS_NAMES[11];

extern const uint CLASS_EXP_LEVELS[10];

extern const char *const ALIGNMENT_NAMES[3];

extern const char *const SEX_NAMES[2];

extern const char *const SKILL_NAMES[18];

extern const char *const CONDITION_NAMES[17];

extern const int CONDITION_COLORS[17];

extern const char *const GOOD;

extern const char *const BLESSED;

extern const char *const POWER_SHIELD;

extern const char *const HOLY_BONUS;

extern const char *const HEROISM;
extern const char *const IN_PARTY;

extern const char *const PARTY_DETAILS;
extern const char *const PARTY_DIALOG_TEXT;

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

extern const uint STAT_VALUES[24];

extern const int STAT_BONUSES[24];

extern const int ELEMENTAL_CATEGORIES[6];

extern const int ATTRIBUTE_CATEGORIES[10];

extern const int ATTRIBUTE_BONUSES[72];

extern const int ELEMENTAL_RESISTENCES[37];

extern const int ELEMENTAL_DAMAGE[37];

extern const int WEAPON_DAMAGE_BASE[35];
extern const int WEAPON_DAMAGE_MULTIPLIER[35];
extern const int METAL_DAMAGE[22];
extern const int METAL_DAMAGE_PERCENT[22];

extern const int METAL_LAC[9];

extern const int ARMOR_STRENGTHS[14];

extern const int MAKE_ITEM_ARR1[6];

extern const int MAKE_ITEM_ARR2[6][7][2];

extern const int MAKE_ITEM_ARR3[10][7][2];

extern const int MAKE_ITEM_ARR4[2][7][2];

extern const int MAKE_ITEM_ARR5[8][2];

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

extern const char *const GOOD_STUFF;

extern const char *const HAVE_A_DRINK;

extern const char *const YOURE_DRUNK;

extern const int TAVERN_EXIT_LIST[2][6][5][2];

extern const char *const FOOD_AND_DRINK;

extern const char *const TEMPLE_TEXT;

extern const char *const EXPERIENCE_FOR_LEVEL;

extern const char *const LEARNED_ALL;

extern const char *const ELIGIBLE_FOR_LEVEL;

extern const char *const TRAINING_TEXT;

extern const char *const GOLD_GEMS;

extern const char *const GOLD_GEMS_2;

extern const char *const DEPOSIT_WITHDRAWL[2];

extern const char *const NOT_ENOUGH_X_IN_THE_Y;

extern const char *const NO_X_IN_THE_Y;

extern const char *const STAT_NAMES[16];

extern const char *const CONSUMABLE_NAMES[4];

extern const char *const WHERE_NAMES[2];

extern const char *const AMOUNT;

extern const char *const FOOD_PACKS_FULL;

extern const char *const BUY_SPELLS;

extern const char *const GUILD_OPTIONS;

extern const int MISC_SPELL_INDEX[74];

extern const int SPELL_COSTS[77];

extern const int CLOUDS_SPELL_OFFSETS[5][20];

extern const uint DARK_SPELL_OFFSETS[3][39];

extern const int DARK_SPELL_RANGES[12][2];

extern const int SPELL_LEVEL_OFFSETS[3][39];

extern const int SPELL_GEM_COST[77];

extern const char *const NOT_A_SPELL_CASTER;

extern const char *const SPELLS_FOR;

extern const char *const SPELL_LINES_0_TO_9;

extern const char *const SPELLS_DIALOG_SPELLS;

extern const char *const SPELL_PTS;

extern const char *const GOLD;

extern const char *const SPELLS_PRESS_A_KEY;

extern const char *const SPELLS_PURCHASE;

extern const char *const SPELL_DETAILS;

extern const char *const MAP_TEXT;

extern const char *const LIGHT_COUNT_TEXT;

extern const char *const FIRE_RESISTENCE_TEXT;

extern const char *const ELECRICITY_RESISTENCE_TEXT;

extern const char *const COLD_RESISTENCE_TEXT;

extern const char *const POISON_RESISTENCE_TEXT;

extern const char *const CLAIRVOYANCE_TEXT;

extern const char *const LEVITATE_TEXT;

extern const char *const WALK_ON_WATER_TEXT;

extern const char *const GAME_INFORMATION;

extern const char *const WORLD_GAME_TEXT;

extern const char *const DARKSIDE_GAME_TEXT;

extern const char *const CLOUDS_GAME_TEXT;

extern const char *const SWORDS_GAME_TEXT;

extern const char *const WEEK_DAY_STRINGS[10];

extern const char *const CHARACTER_DETAILS;

extern const char *const PARTY_GOLD;

extern const char *const PLUS_14;

extern const char *const CHARACTER_TEMPLATE;

extern const char *const EXCHANGING_IN_COMBAT;

extern const char *const CURRENT_MAXIMUM_RATING_TEXT;

extern const char *const CURRENT_MAXIMUM_TEXT;

extern const char *const RATING_TEXT[24];

extern const char *const AGE_TEXT;

extern const char *const LEVEL_TEXT;

extern const char *const RESISTENCES_TEXT;

extern const char *const NONE;

extern const char *const EXPERIENCE_TEXT;

extern const char *const ELIGIBLE;

extern const char *const IN_PARTY_IN_BANK;

extern const char *const FOOD_TEXT;

extern const char *const EXCHANGE_WITH_WHOM;

extern const char *const QUICK_REF_LINE;

extern const char *const QUICK_REFERENCE;

extern const uint BLACKSMITH_MAP_IDS[2][4];

extern const char *const ITEMS_DIALOG_TEXT1;
extern const char *const ITEMS_DIALOG_TEXT2;
extern const char *const ITEMS_DIALOG_LINE1;
extern const char *const ITEMS_DIALOG_LINE2;

extern const char *const BTN_BUY;
extern const char *const BTN_SELL;
extern const char *const BTN_IDENTIFY;
extern const char *const BTN_FIX;
extern const char *const BTN_USE;
extern const char *const BTN_EQUIP;
extern const char *const BTN_REMOVE;
extern const char *const BTN_DISCARD;
extern const char *const BTN_EQUIP;
extern const char *const BTN_QUEST;
extern const char *const BTN_ENCHANT;
extern const char *const BTN_RECHARGE;
extern const char *const BTN_GOLD;

extern const char *const ITEM_BROKEN;
extern const char *const ITEM_CURSED;
extern const char *const BONUS_NAMES[7];
extern const char *const WEAPON_NAMES[35];
extern const char *const ARMOR_NAMES[14];
extern const char *const ACCESSORY_NAMES[11];
extern const char *const MISC_NAMES[22];
extern const char *const ELEMENTAL_NAMES[6];
extern const char *const ATTRIBUTE_NAMES[10];
extern const char *const EFFECTIVENESS_NAMES[7];
extern const char *const QUEST_ITEM_NAMES[85];

extern const int WEAPON_BASE_COSTS[35];
extern const int ARMOR_BASE_COSTS[25];
extern const int ACCESSORY_BASE_COSTS[11];
extern const int MISC_MATERIAL_COSTS[22];
extern const int MISC_BASE_COSTS[76];
extern const int METAL_BASE_MULTIPLIERS[22];
extern const int ITEM_SKILL_DIVISORS[4];

extern const int RESTRICTION_OFFSETS[4];
extern const int ITEM_RESTRICTIONS[86];

extern const char *const NOT_PROFICIENT;

extern const char *const NO_ITEMS_AVAILABLE;

extern const char *const CATEGORY_NAMES[4];

extern const char *const X_FOR_THE_Y;

extern const char *const X_FOR_Y;

extern const char *const X_FOR_Y_GOLD;

extern const char *const FMT_CHARGES;

extern const char *const AVAILABLE_GOLD_COST;

extern const char *const CHARGES;

extern const char *const COST;

extern const char *const ITEM_ACTIONS[7];
extern const char *const WHICH_ITEM;

extern const char *const WHATS_YOUR_HURRY;

extern const char *const USE_ITEM_IN_COMBAT;

extern const char *const NO_SPECIAL_ABILITIES;

extern const char *const CANT_CAST_WHILE_ENGAGED;

extern const char *const EQUIPPED_ALL_YOU_CAN;
extern const char *const REMOVE_X_TO_EQUIP_Y;
extern const char *const RING;
extern const char *const MEDAL;

extern const char *const CANNOT_REMOVE_CURSED_ITEM;

extern const char *const CANNOT_DISCARD_CURSED_ITEM;

extern const char *const PERMANENTLY_DISCARD;

extern const char *const BACKPACK_IS_FULL;

extern const char *const CATEGORY_BACKPACK_IS_FULL[4];

extern const char *const BUY_X_FOR_Y_GOLD;

extern const char *const SELL_X_FOR_Y_GOLD;

extern const char *const NO_NEED_OF_THIS;

extern const char *const NOT_RECHARGABLE;

extern const char *const SPELL_FAILED;

extern const char *const NOT_ENCHANTABLE;

extern const char *const ITEM_NOT_BROKEN;

extern const char *const FIX_IDENTIFY[2];

extern const char *const FIX_IDENTIFY_GOLD;

extern const char *const IDENTIFY_ITEM_MSG;

extern const char *const ITEM_DETAILS;

extern const char *const ALL;
extern const char *const FIELD_NONE;
extern const char *const DAMAGE_X_TO_Y;
extern const char *const ELEMENTAL_XY_DAMAGE;
extern const char *const ATTR_XY_BONUS;
extern const char *const EFFECTIVE_AGAINST;

extern const char *const QUESTS_DIALOG_TEXT;
extern const char *const CLOUDS_OF_XEEN_LINE;
extern const char *const DARKSIDE_OF_XEEN_LINE;

extern const char *const NO_QUEST_ITEMS;
extern const char *const NO_CURRENT_QUESTS;
extern const char *const NO_AUTO_NOTES;
extern const char *const QUEST_ITEMS_DATA;
extern const char *const CURRENT_QUESTS_DATA;
extern const char *const AUTO_NOTES_DATA;

extern const char *const REST_COMPLETE;
extern const char *const PARTY_IS_STARVING;
extern const char *const HIT_SPELL_POINTS_RESTORED;
extern const char *const TOO_DANGEROUS_TO_REST;
extern const char *const SOME_CHARS_MAY_DIE;

extern const char *const CANT_DISMISS_LAST_CHAR;

extern const char *const REMOVE_DELETE[2];
extern const char *const REMOVE_OR_DELETE_WHICH;

extern const char *const YOUR_PARTY_IS_FULL;

extern const char *const HAS_SLAYER_SWORD;
extern const char *const SURE_TO_DELETE_CHAR;

extern const char *const CREATE_CHAR_DETAILS;

extern const char *const NEW_CHAR_STATS;
extern const char *const NAME_FOR_NEW_CHARACTER;
extern const char *const SELECT_CLASS_BEFORE_SAVING;
extern const char *const EXCHANGE_ATTR_WITH;
extern const int NEW_CHAR_SKILLS[10];
extern const int NEW_CHAR_SKILLS_LEN[10];
extern const int NEW_CHAR_RACE_SKILLS[10];

extern const int RACE_MAGIC_RESISTENCES[5];
extern const int RACE_FIRE_RESISTENCES[5];
extern const int RACE_ELECTRIC_RESISTENCES[5];
extern const int RACE_COLD_RESISTENCES[5];
extern const int RACE_ENERGY_RESISTENCES[5];
extern const int RACE_POISON_RESISTENCES[5];
extern const int NEW_CHARACTER_SPELLS[10][4];

extern const char *const COMBAT_DETAILS;

} // End of namespace Xeen

#endif	/* XEEN_RESOURCES_H */
