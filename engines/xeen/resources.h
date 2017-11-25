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

#define Res (*g_resources)

class XeenEngine;

class Resources {
protected:
	Resources();
public:
	SpriteResource _globalSprites;
	Common::StringArray _maeNames;		// Magic and equipment names
	static const char *const CREDITS;
	static const char *const OPTIONS_TITLE;
	static const char *const THE_PARTY_NEEDS_REST;
	static const char *const WHO_WILL;
	static const char *const HOW_MUCH;
	static const char *const WHATS_THE_PASSWORD;
	static const char *const IN_NO_CONDITION;
	static const char *const NOTHING_HERE;
	static const char *const TERRAIN_TYPES[6];
	static const char *const SURFACE_TYPE_NAMES[15];
	static const char *const SURFACE_NAMES[16];
	static const char *const WHO_ACTIONS[32];
	static const char *const WHO_WILL_ACTIONS[4];
	static const byte SYMBOLS[20][64];
	static const byte TEXT_COLORS[40][4];
	static const char *const DIRECTION_TEXT_UPPER[4];
	static const char *const DIRECTION_TEXT[4];
	static const char *const RACE_NAMES[5];
	static const int RACE_HP_BONUSES[5];
	static const int RACE_SP_BONUSES[5][2];
	static const char *const CLASS_NAMES[11];
	static const uint CLASS_EXP_LEVELS[10];
	static const char *const ALIGNMENT_NAMES[3];
	static const char *const SEX_NAMES[2];
	static const char *const SKILL_NAMES[18];
	static const char *const CONDITION_NAMES[17];
	static const int CONDITION_COLORS[17];
	static const char *const GOOD;
	static const char *const BLESSED;
	static const char *const POWER_SHIELD;
	static const char *const HOLY_BONUS;
	static const char *const HEROISM;
	static const char *const IN_PARTY;
	static const char *const PARTY_DETAILS;
	static const char *const PARTY_DIALOG_TEXT;
	static const int FACE_CONDITION_FRAMES[17];
	static const int CHAR_FACES_X[6];
	static const int HP_BARS_X[6];
	static const char *const NO_ONE_TO_ADVENTURE_WITH;
	static const byte BACKGROUND_XLAT[];
	static const char *const YOUR_ROSTER_IS_FULL;
	static const char *const PLEASE_WAIT;
	static const char *const OOPS;
	static const int8 SCREEN_POSITIONING_X[4][48];
	static const int8 SCREEN_POSITIONING_Y[4][48];
	static const int MONSTER_GRID_BITMASK[12];
	static const int INDOOR_OBJECT_X[2][12];
	static const int MAP_OBJECT_Y[2][12];
	static const int INDOOR_MONSTERS_Y[4];
	static const int OUTDOOR_OBJECT_X[2][12];
	static const int OUTDOOR_MONSTER_INDEXES[26];
	static const int OUTDOOR_MONSTERS_Y[26];
	static const int DIRECTION_ANIM_POSITIONS[4][4];
	static const byte WALL_SHIFTS[4][48];
	static const int DRAW_NUMBERS[25];
	static const int DRAW_FRAMES[25][2];
	static const int COMBAT_FLOAT_X[8];
	static const int COMBAT_FLOAT_Y[8];
	static const int MONSTER_EFFECT_FLAGS[15][8];
	static const uint SPELLS_ALLOWED[3][40];
	static const int BASE_HP_BY_CLASS[10];
	static const int AGE_RANGES[10];
	static const int AGE_RANGES_ADJUST[2][10];
	static const uint STAT_VALUES[24];
	static const int STAT_BONUSES[24];
	static const int ELEMENTAL_CATEGORIES[6];
	static const int ATTRIBUTE_CATEGORIES[10];
	static const int ATTRIBUTE_BONUSES[72];
	static const int ELEMENTAL_RESISTENCES[37];
	static const int ELEMENTAL_DAMAGE[37];
	static const int WEAPON_DAMAGE_BASE[35];
	static const int WEAPON_DAMAGE_MULTIPLIER[35];
	static const int METAL_DAMAGE[22];
	static const int METAL_DAMAGE_PERCENT[22];
	static const int METAL_LAC[9];
	static const int ARMOR_STRENGTHS[14];
	static const int MAKE_ITEM_ARR1[6];
	static const int MAKE_ITEM_ARR2[6][7][2];
	static const int MAKE_ITEM_ARR3[10][7][2];
	static const int MAKE_ITEM_ARR4[2][7][2];
	static const int MAKE_ITEM_ARR5[8][2];
	static const int OUTDOOR_DRAWSTRCT_INDEXES[44];
	static const int TOWN_MAXES[2][11];
	static const char *const TOWN_ACTION_MUSIC[2][7];
	static const char *const TOWN_ACTION_SHAPES[7];
	static const int TOWN_ACTION_FILES[2][7];
	static const char *const BANK_TEXT;
	static const char *const BLACKSMITH_TEXT;
	static const char *const GUILD_NOT_MEMBER_TEXT;
	static const char *const GUILD_TEXT;
	static const char *const TAVERN_TEXT;
	static const char *const GOOD_STUFF;
	static const char *const HAVE_A_DRINK;
	static const char *const YOURE_DRUNK;
	static const int TAVERN_EXIT_LIST[2][6][5][2];
	static const char *const FOOD_AND_DRINK;
	static const char *const TEMPLE_TEXT;
	static const char *const EXPERIENCE_FOR_LEVEL;
	static const char *const LEARNED_ALL;
	static const char *const ELIGIBLE_FOR_LEVEL;
	static const char *const TRAINING_TEXT;
	static const char *const GOLD_GEMS;
	static const char *const GOLD_GEMS_2;
	static const char *const DEPOSIT_WITHDRAWL[2];
	static const char *const NOT_ENOUGH_X_IN_THE_Y;
	static const char *const NO_X_IN_THE_Y;
	static const char *const STAT_NAMES[16];
	static const char *const CONSUMABLE_NAMES[4];
	static const char *const WHERE_NAMES[2];
	static const char *const AMOUNT;
	static const char *const FOOD_PACKS_FULL;
	static const char *const BUY_SPELLS;
	static const char *const GUILD_OPTIONS;
	static const int MISC_SPELL_INDEX[74];
	static const int SPELL_COSTS[77];
	static const int CLOUDS_SPELL_OFFSETS[5][20];
	static const uint DARK_SPELL_OFFSETS[3][39];
	static const int DARK_SPELL_RANGES[12][2];
	static const int SPELL_LEVEL_OFFSETS[3][39];
	static const int SPELL_GEM_COST[77];
	static const char *const NOT_A_SPELL_CASTER;
	static const char *const SPELLS_FOR;
	static const char *const SPELL_LINES_0_TO_9;
	static const char *const SPELLS_DIALOG_SPELLS;
	static const char *const SPELL_PTS;
	static const char *const GOLD;
	static const char *const SPELLS_PRESS_A_KEY;
	static const char *const SPELLS_PURCHASE;
	static const char *const MAP_TEXT;
	static const char *const LIGHT_COUNT_TEXT;
	static const char *const FIRE_RESISTENCE_TEXT;
	static const char *const ELECRICITY_RESISTENCE_TEXT;
	static const char *const COLD_RESISTENCE_TEXT;
	static const char *const POISON_RESISTENCE_TEXT;
	static const char *const CLAIRVOYANCE_TEXT;
	static const char *const LEVITATE_TEXT;
	static const char *const WALK_ON_WATER_TEXT;
	static const char *const GAME_INFORMATION;
	static const char *const WORLD_GAME_TEXT;
	static const char *const DARKSIDE_GAME_TEXT;
	static const char *const CLOUDS_GAME_TEXT;
	static const char *const SWORDS_GAME_TEXT;
	static const char *const WEEK_DAY_STRINGS[10];
	static const char *const CHARACTER_DETAILS;
	static const char *const PARTY_GOLD;
	static const char *const PLUS_14;
	static const char *const CHARACTER_TEMPLATE;
	static const char *const EXCHANGING_IN_COMBAT;
	static const char *const CURRENT_MAXIMUM_RATING_TEXT;
	static const char *const CURRENT_MAXIMUM_TEXT;
	static const char *const RATING_TEXT[24];
	static const char *const AGE_TEXT;
	static const char *const LEVEL_TEXT;
	static const char *const RESISTENCES_TEXT;
	static const char *const NONE;
	static const char *const EXPERIENCE_TEXT;
	static const char *const ELIGIBLE;
	static const char *const IN_PARTY_IN_BANK;
	static const char *const FOOD_TEXT;
	static const char *const EXCHANGE_WITH_WHOM;
	static const char *const QUICK_REF_LINE;
	static const char *const QUICK_REFERENCE;
	static const uint BLACKSMITH_MAP_IDS[2][4];
	static const char *const ITEMS_DIALOG_TEXT1;
	static const char *const ITEMS_DIALOG_TEXT2;
	static const char *const ITEMS_DIALOG_LINE1;
	static const char *const ITEMS_DIALOG_LINE2;
	static const char *const BTN_BUY;
	static const char *const BTN_SELL;
	static const char *const BTN_IDENTIFY;
	static const char *const BTN_FIX;
	static const char *const BTN_USE;
	static const char *const BTN_EQUIP;
	static const char *const BTN_REMOVE;
	static const char *const BTN_DISCARD;
	static const char *const BTN_QUEST;
	static const char *const BTN_ENCHANT;
	static const char *const BTN_RECHARGE;
	static const char *const BTN_GOLD;
	static const char *const ITEM_BROKEN;
	static const char *const ITEM_CURSED;
	static const char *const BONUS_NAMES[7];
	static const char *const WEAPON_NAMES[35];
	static const char *const ARMOR_NAMES[14];
	static const char *const ACCESSORY_NAMES[11];
	static const char *const MISC_NAMES[22];
	static const char *const *ITEM_NAMES[4];
	static const char *const ELEMENTAL_NAMES[6];
	static const char *const ATTRIBUTE_NAMES[10];
	static const char *const EFFECTIVENESS_NAMES[7];
	static const char *const QUEST_ITEM_NAMES[85];
	static const int WEAPON_BASE_COSTS[35];
	static const int ARMOR_BASE_COSTS[14];
	static const int ACCESSORY_BASE_COSTS[11];
	static const int MISC_MATERIAL_COSTS[22];
	static const int MISC_BASE_COSTS[76];
	static const int METAL_BASE_MULTIPLIERS[22];
	static const int ITEM_SKILL_DIVISORS[4];
	static const int RESTRICTION_OFFSETS[4];
	static const int ITEM_RESTRICTIONS[86];
	static const char *const NOT_PROFICIENT;
	static const char *const NO_ITEMS_AVAILABLE;
	static const char *const CATEGORY_NAMES[4];
	static const char *const X_FOR_THE_Y;
	static const char *const X_FOR_Y;
	static const char *const X_FOR_Y_GOLD;
	static const char *const FMT_CHARGES;
	static const char *const AVAILABLE_GOLD_COST;
	static const char *const CHARGES;
	static const char *const COST;
	static const char *const ITEM_ACTIONS[7];
	static const char *const WHICH_ITEM;
	static const char *const WHATS_YOUR_HURRY;

	static const char *const USE_ITEM_IN_COMBAT;

	static const char *const NO_SPECIAL_ABILITIES;

	static const char *const CANT_CAST_WHILE_ENGAGED;

	static const char *const EQUIPPED_ALL_YOU_CAN;
	static const char *const REMOVE_X_TO_EQUIP_Y;
	static const char *const RING;
	static const char *const MEDAL;
	static const char *const CANNOT_REMOVE_CURSED_ITEM;
	static const char *const CANNOT_DISCARD_CURSED_ITEM;
	static const char *const PERMANENTLY_DISCARD;
	static const char *const BACKPACK_IS_FULL;
	static const char *const CATEGORY_BACKPACK_IS_FULL[4];
	static const char *const BUY_X_FOR_Y_GOLD;
	static const char *const SELL_X_FOR_Y_GOLD;
	static const char *const NO_NEED_OF_THIS;
	static const char *const NOT_RECHARGABLE;
	static const char *const SPELL_FAILED;
	static const char *const NOT_ENCHANTABLE;
	static const char *const ITEM_NOT_BROKEN;
	static const char *const FIX_IDENTIFY[2];
	static const char *const FIX_IDENTIFY_GOLD;
	static const char *const IDENTIFY_ITEM_MSG;
	static const char *const ITEM_DETAILS;
	static const char *const ALL;
	static const char *const FIELD_NONE;
	static const char *const DAMAGE_X_TO_Y;
	static const char *const ELEMENTAL_XY_DAMAGE;
	static const char *const ATTR_XY_BONUS;
	static const char *const EFFECTIVE_AGAINST;
	static const char *const QUESTS_DIALOG_TEXT;
	static const char *const CLOUDS_OF_XEEN_LINE;
	static const char *const DARKSIDE_OF_XEEN_LINE;
	static const char *const NO_QUEST_ITEMS;
	static const char *const NO_CURRENT_QUESTS;
	static const char *const NO_AUTO_NOTES;
	static const char *const QUEST_ITEMS_DATA;
	static const char *const CURRENT_QUESTS_DATA;
	static const char *const AUTO_NOTES_DATA;
	static const char *const REST_COMPLETE;
	static const char *const PARTY_IS_STARVING;
	static const char *const HIT_SPELL_POINTS_RESTORED;
	static const char *const TOO_DANGEROUS_TO_REST;
	static const char *const SOME_CHARS_MAY_DIE;
	static const char *const CANT_DISMISS_LAST_CHAR;
	static const char *const REMOVE_DELETE[2];
	static const char *const REMOVE_OR_DELETE_WHICH;
	static const char *const YOUR_PARTY_IS_FULL;
	static const char *const HAS_SLAYER_SWORD;
	static const char *const SURE_TO_DELETE_CHAR;
	static const char *const CREATE_CHAR_DETAILS;
	static const char *const NEW_CHAR_STATS;
	static const char *const NAME_FOR_NEW_CHARACTER;
	static const char *const SELECT_CLASS_BEFORE_SAVING;
	static const char *const EXCHANGE_ATTR_WITH;
	static const int NEW_CHAR_SKILLS[10];
	static const int NEW_CHAR_SKILLS_LEN[10];
	static const int NEW_CHAR_RACE_SKILLS[10];
	static const int RACE_MAGIC_RESISTENCES[5];
	static const int RACE_FIRE_RESISTENCES[5];
	static const int RACE_ELECTRIC_RESISTENCES[5];
	static const int RACE_COLD_RESISTENCES[5];
	static const int RACE_ENERGY_RESISTENCES[5];
	static const int RACE_POISON_RESISTENCES[5];
	static const int NEW_CHARACTER_SPELLS[10][4];
	static const char *const COMBAT_DETAILS;
	static const char *NOT_ENOUGH_TO_CAST;
	static const char *SPELL_CAST_COMPONENTS[2];
	static const char *const CAST_SPELL_DETAILS;
	static const char *const PARTY_FOUND;
	static const char *const BACKPACKS_FULL_PRESS_KEY;
	static const char *const HIT_A_KEY;
	static const char *const GIVE_TREASURE_FORMATTING;
	static const char *const X_FOUND_Y;
	static const char *const ON_WHO;
	static const char *const WHICH_ELEMENT1;
	static const char *const WHICH_ELEMENT2;
	static const char *const DETECT_MONSTERS;
	static const char *const LLOYDS_BEACON;
	static const char *const HOW_MANY_SQUARES;
	static const char *const TOWN_PORTAL;
	static const int TOWN_MAP_NUMBERS[2][5];
	static const char *const MONSTER_DETAILS;
	static const char *const MONSTER_SPECIAL_ATTACKS[23];
	static const char *const IDENTIFY_MONSTERS;
	static const char *const EVENT_SAMPLES[6];
public:
	/**
	 * Initializes an instnace of the resources
	 */
	static Resources *init(XeenEngine *vm);
};

extern Resources *g_resources;

} // End of namespace Xeen

#endif	/* XEEN_RESOURCES_H */
