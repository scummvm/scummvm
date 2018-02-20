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
#include "xeen/files.h"
#include "xeen/party.h"
#include "xeen/spells.h"

namespace Xeen {

#define Res (*g_resources)

class XeenEngine;

class Resources {
	/**
	 * Derived string class to fix automatic type conversion to const char *
	 */
	class String : public Common::String {
	public:
		String() : Common::String() {}
		String(const Common::String &s) : Common::String(s) {}
		operator const char *() { return c_str(); }
	};

	/**
	 * Derived file class with sync method aliases so that the same
	 * code from create_xeen can be re-used for both reading and
	 * writing the resource data
	 */	
	class ResFile : public File {
	public:
		ResFile() : File("CONSTANTS") {}

		void syncString(String &str) {
			str = readString();
		}
		void syncStrings(String *str, int count) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count, 0, 0, 0));
			for (int idx = 0; idx < count; ++idx)
				str[idx] = readString();
		}
		void syncStrings2D(String *str, int count1, int count2) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count1, count2, 0, 0));
			for (int idx = 0; idx < count1 * count2; ++idx)
				str[idx] = readString();
		}
		void syncNumber(int &val) {
			val = readSint32LE();
		}
		void syncNumbers(int *vals, int count) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count, 0, 0, 0));
			for (int idx = 0; idx < count; ++idx)
				vals[idx] = readSint32LE();
		}
		void syncNumbers2D(int *vals, int count1, int count2) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count1, count2, 0, 0));
			for (int idx = 0; idx < count1 * count2; ++idx)
				vals[idx] = readSint32LE();
		}
		void syncNumbers3D(int *vals, int count1, int count2, int count3) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count1, count2, count3, 0));
			for (int idx = 0; idx < count1 * count2 * count3; ++idx)
				vals[idx] = readSint32LE();
		}
		void syncNumbers4D(int *vals, int count1, int count2, int count3, int count4) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count1, count2, count3, count4));
			for (int idx = 0; idx < count1 * count2 * count3 * count4; ++idx)
				vals[idx] = readSint32LE();
		}
		void syncBytes2D(byte *vals, int count1, int count2) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count1, count2, 0, 0));
			read(vals, count1 * count2);
		}
	};
private:
	/**
	 * Loads all the constants data stored in the xeen.css
	 */
	void loadData();
public:
	SpriteResource _globalSprites;
	Common::StringArray _maeNames;		// Magic and equipment names
	String *ITEM_NAMES[4];

	// Data loaded from xeen.ccs
	String CREDITS;
	String OPTIONS_TITLE;
	String THE_PARTY_NEEDS_REST;
	String WHO_WILL;
	String HOW_MUCH;
	String WHATS_THE_PASSWORD;
	String IN_NO_CONDITION;
	String NOTHING_HERE;
	String TERRAIN_TYPES[6];
	String OUTDOORS_WALL_TYPES[16];
	String SURFACE_NAMES[16];
	String WHO_ACTIONS[32];
	String WHO_WILL_ACTIONS[4];
	byte SYMBOLS[20][64];
	byte TEXT_COLORS[40][4];
	String DIRECTION_TEXT_UPPER[4];
	String DIRECTION_TEXT[4];
	String RACE_NAMES[5];
	int RACE_HP_BONUSES[5];
	int RACE_SP_BONUSES[5][2];
	String CLASS_NAMES[11];
	int CLASS_EXP_LEVELS[10];
	String ALIGNMENT_NAMES[3];
	String SEX_NAMES[2];
	String SKILL_NAMES[18];
	String CONDITION_NAMES[17];
	int CONDITION_COLORS[17];
	String GOOD;
	String BLESSED;
	String POWER_SHIELD;
	String HOLY_BONUS;
	String HEROISM;
	String IN_PARTY;
	String PARTY_DETAILS;
	String PARTY_DIALOG_TEXT;
	int FACE_CONDITION_FRAMES[17];
	int CHAR_FACES_X[6];
	int HP_BARS_X[6];
	String NO_ONE_TO_ADVENTURE_WITH;
	byte DARKNESS_XLAT[3][256];
	String YOUR_ROSTER_IS_FULL;
	String PLEASE_WAIT;
	String OOPS;
	int SCREEN_POSITIONING_X[4][48];
	int SCREEN_POSITIONING_Y[4][48];
	int MONSTER_GRID_BITMASK[12];
	int INDOOR_OBJECT_X[2][12];
	int MAP_OBJECT_Y[2][12];
	int INDOOR_MONSTERS_Y[4];
	int OUTDOOR_OBJECT_X[2][12];
	int OUTDOOR_MONSTER_INDEXES[26];
	int OUTDOOR_MONSTERS_Y[26];
	int DIRECTION_ANIM_POSITIONS[4][4];
	byte WALL_SHIFTS[4][48];
	int DRAW_NUMBERS[25];
	int DRAW_FRAMES[25][2];
	int COMBAT_FLOAT_X[8];
	int COMBAT_FLOAT_Y[8];
	int MONSTER_EFFECT_FLAGS[15][8];
	int SPELLS_ALLOWED[3][40];
	int BASE_HP_BY_CLASS[10];
	int AGE_RANGES[10];
	int AGE_RANGES_ADJUST[2][10];
	int STAT_VALUES[24];
	int STAT_BONUSES[24];
	int ELEMENTAL_CATEGORIES[6];
	int ATTRIBUTE_CATEGORIES[10];
	int ATTRIBUTE_BONUSES[72];
	int ELEMENTAL_RESISTENCES[37];
	int ELEMENTAL_DAMAGE[37];
	int WEAPON_DAMAGE_BASE[35];
	int WEAPON_DAMAGE_MULTIPLIER[35];
	int METAL_DAMAGE[22];
	int METAL_DAMAGE_PERCENT[22];
	int METAL_LAC[9];
	int ARMOR_STRENGTHS[14];
	int MAKE_ITEM_ARR1[6];
	int MAKE_ITEM_ARR2[6][7][2];
	int MAKE_ITEM_ARR3[10][7][2];
	int MAKE_ITEM_ARR4[2][7][2];
	int MAKE_ITEM_ARR5[8][2];
	int OUTDOOR_DRAWSTRUCT_INDEXES[44];
	int TOWN_MAXES[2][11];
	String TOWN_ACTION_MUSIC[2][7];
	String TOWN_ACTION_SHAPES[7];
	int TOWN_ACTION_FILES[2][7];
	String BANK_TEXT;
	String BLACKSMITH_TEXT;
	String GUILD_NOT_MEMBER_TEXT;
	String GUILD_TEXT;
	String TAVERN_TEXT;
	String GOOD_STUFF;
	String HAVE_A_DRINK;
	String YOURE_DRUNK;
	int TAVERN_EXIT_LIST[2][6][5][2];
	String FOOD_AND_DRINK;
	String TEMPLE_TEXT;
	String EXPERIENCE_FOR_LEVEL;
	String LEARNED_ALL;
	String ELIGIBLE_FOR_LEVEL;
	String TRAINING_TEXT;
	String GOLD_GEMS;
	String GOLD_GEMS_2;
	String DEPOSIT_WITHDRAWL[2];
	String NOT_ENOUGH_X_IN_THE_Y;
	String NO_X_IN_THE_Y;
	String STAT_NAMES[16];
	String CONSUMABLE_NAMES[4];
	String WHERE_NAMES[2];
	String AMOUNT;
	String FOOD_PACKS_FULL;
	String BUY_SPELLS;
	String GUILD_OPTIONS;
	int MISC_SPELL_INDEX[74];
	int SPELL_COSTS[77];
	int CLOUDS_SPELL_OFFSETS[5][20];
	int DARK_SPELL_OFFSETS[3][39];
	int DARK_SPELL_RANGES[12][2];
	int SPELL_LEVEL_OFFSETS[3][39];
	int SPELL_GEM_COST[77];
	String NOT_A_SPELL_CASTER;
	String SPELLS_FOR;
	String SPELL_LINES_0_TO_9;
	String SPELLS_DIALOG_SPELLS;
	String SPELL_PTS;
	String GOLD;
	String SPELLS_PRESS_A_KEY;
	String SPELLS_PURCHASE;
	String MAP_TEXT;
	String LIGHT_COUNT_TEXT;
	String FIRE_RESISTENCE_TEXT;
	String ELECRICITY_RESISTENCE_TEXT;
	String COLD_RESISTENCE_TEXT;
	String POISON_RESISTENCE_TEXT;
	String CLAIRVOYANCE_TEXT;
	String LEVITATE_TEXT;
	String WALK_ON_WATER_TEXT;
	String GAME_INFORMATION;
	String WORLD_GAME_TEXT;
	String DARKSIDE_GAME_TEXT;
	String CLOUDS_GAME_TEXT;
	String SWORDS_GAME_TEXT;
	String WEEK_DAY_STRINGS[10];
	String CHARACTER_DETAILS;
	String PARTY_GOLD;
	String PLUS_14;
	String CHARACTER_TEMPLATE;
	String EXCHANGING_IN_COMBAT;
	String CURRENT_MAXIMUM_RATING_TEXT;
	String CURRENT_MAXIMUM_TEXT;
	String RATING_TEXT[24];
	String AGE_TEXT;
	String LEVEL_TEXT;
	String RESISTENCES_TEXT;
	String NONE;
	String EXPERIENCE_TEXT;
	String ELIGIBLE;
	String IN_PARTY_IN_BANK;
	String FOOD_TEXT;
	String EXCHANGE_WITH_WHOM;
	String QUICK_REF_LINE;
	String QUICK_REFERENCE;
	int BLACKSMITH_MAP_IDS[2][4];
	String ITEMS_DIALOG_TEXT1;
	String ITEMS_DIALOG_TEXT2;
	String ITEMS_DIALOG_LINE1;
	String ITEMS_DIALOG_LINE2;
	String BTN_BUY;
	String BTN_SELL;
	String BTN_IDENTIFY;
	String BTN_FIX;
	String BTN_USE;
	String BTN_EQUIP;
	String BTN_REMOVE;
	String BTN_DISCARD;
	String BTN_QUEST;
	String BTN_ENCHANT;
	String BTN_RECHARGE;
	String BTN_GOLD;
	String ITEM_BROKEN;
	String ITEM_CURSED;
	String BONUS_NAMES[7];
	String WEAPON_NAMES[35];
	String ARMOR_NAMES[14];
	String ACCESSORY_NAMES[11];
	String MISC_NAMES[22];
	String ELEMENTAL_NAMES[6];
	String ATTRIBUTE_NAMES[10];
	String EFFECTIVENESS_NAMES[7];
	String QUEST_ITEM_NAMES[85];
	int WEAPON_BASE_COSTS[35];
	int ARMOR_BASE_COSTS[14];
	int ACCESSORY_BASE_COSTS[11];
	int MISC_MATERIAL_COSTS[22];
	int MISC_BASE_COSTS[76];
	int METAL_BASE_MULTIPLIERS[22];
	int ITEM_SKILL_DIVISORS[4];
	int RESTRICTION_OFFSETS[4];
	int ITEM_RESTRICTIONS[86];
	String NOT_PROFICIENT;
	String NO_ITEMS_AVAILABLE;
	String CATEGORY_NAMES[4];
	String X_FOR_THE_Y;
	String X_FOR_Y;
	String X_FOR_Y_GOLD;
	String FMT_CHARGES;
	String AVAILABLE_GOLD_COST;
	String CHARGES;
	String COST;
	String ITEM_ACTIONS[7];
	String WHICH_ITEM;
	String WHATS_YOUR_HURRY;
	String USE_ITEM_IN_COMBAT;
	String NO_SPECIAL_ABILITIES;
	String CANT_CAST_WHILE_ENGAGED;
	String EQUIPPED_ALL_YOU_CAN;
	String REMOVE_X_TO_EQUIP_Y;
	String RING;
	String MEDAL;
	String CANNOT_REMOVE_CURSED_ITEM;
	String CANNOT_DISCARD_CURSED_ITEM;
	String PERMANENTLY_DISCARD;
	String BACKPACK_IS_FULL;
	String CATEGORY_BACKPACK_IS_FULL[4];
	String BUY_X_FOR_Y_GOLD;
	String SELL_X_FOR_Y_GOLD;
	String NO_NEED_OF_THIS;
	String NOT_RECHARGABLE;
	String SPELL_FAILED;
	String NOT_ENCHANTABLE;
	String ITEM_NOT_BROKEN;
	String FIX_IDENTIFY[2];
	String FIX_IDENTIFY_GOLD;
	String IDENTIFY_ITEM_MSG;
	String ITEM_DETAILS;
	String ALL;
	String FIELD_NONE;
	String DAMAGE_X_TO_Y;
	String ELEMENTAL_XY_DAMAGE;
	String ATTR_XY_BONUS;
	String EFFECTIVE_AGAINST;
	String QUESTS_DIALOG_TEXT;
	String CLOUDS_OF_XEEN_LINE;
	String DARKSIDE_OF_XEEN_LINE;
	String NO_QUEST_ITEMS;
	String NO_CURRENT_QUESTS;
	String NO_AUTO_NOTES;
	String QUEST_ITEMS_DATA;
	String CURRENT_QUESTS_DATA;
	String AUTO_NOTES_DATA;
	String REST_COMPLETE;
	String PARTY_IS_STARVING;
	String HIT_SPELL_POINTS_RESTORED;
	String TOO_DANGEROUS_TO_REST;
	String SOME_CHARS_MAY_DIE;
	String CANT_DISMISS_LAST_CHAR;
	String REMOVE_DELETE[2];
	String REMOVE_OR_DELETE_WHICH;
	String YOUR_PARTY_IS_FULL;
	String HAS_SLAYER_SWORD;
	String SURE_TO_DELETE_CHAR;
	String CREATE_CHAR_DETAILS;
	String NEW_CHAR_STATS;
	String NAME_FOR_NEW_CHARACTER;
	String SELECT_CLASS_BEFORE_SAVING;
	String EXCHANGE_ATTR_WITH;
	int NEW_CHAR_SKILLS[10];
	int NEW_CHAR_SKILLS_LEN[10];
	int NEW_CHAR_RACE_SKILLS[10];
	int RACE_MAGIC_RESISTENCES[5];
	int RACE_FIRE_RESISTENCES[5];
	int RACE_ELECTRIC_RESISTENCES[5];
	int RACE_COLD_RESISTENCES[5];
	int RACE_ENERGY_RESISTENCES[5];
	int RACE_POISON_RESISTENCES[5];
	int NEW_CHARACTER_SPELLS[10][4];
	String COMBAT_DETAILS;
	String NOT_ENOUGH_TO_CAST;
	String SPELL_CAST_COMPONENTS[2];
	String CAST_SPELL_DETAILS;
	String PARTY_FOUND;
	String BACKPACKS_FULL_PRESS_KEY;
	String HIT_A_KEY;
	String GIVE_TREASURE_FORMATTING;
	String X_FOUND_Y;
	String ON_WHO;
	String WHICH_ELEMENT1;
	String WHICH_ELEMENT2;
	String DETECT_MONSTERS;
	String LLOYDS_BEACON;
	String HOW_MANY_SQUARES;
	String TOWN_PORTAL;
	int TOWN_MAP_NUMBERS[2][5];
	String MONSTER_DETAILS;
	String MONSTER_SPECIAL_ATTACKS[23];
	String IDENTIFY_MONSTERS;
	String EVENT_SAMPLES[6];
	String MOONS_NOT_ALIGNED;
	String AWARDS_FOR;
	String AWARDS_TEXT;
	String NO_AWARDS;
	String WARZONE_BATTLE_MASTER;
	String WARZONE_MAXED;
	String WARZONE_LEVEL;
	String WARZONE_HOW_MANY;
	String PICKS_THE_LOCK;
	String UNABLE_TO_PICK_LOCK;
	String CONTROL_PANEL_TEXT;
	String CONTROL_PANEL_BUTTONS;
	String ON;
	String OFF;
	String CONFIRM_QUIT;
	String MR_WIZARD;
	String NO_LOADING_IN_COMBAT;
	String NO_SAVING_IN_COMBAT;
	String QUICK_FIGHT_TEXT;
	String QUICK_FIGHT_OPTIONS[4];
	String WORLD_END_TEXT[9];
	String WORLD_CONGRATULATIONS;
	String WORLD_CONGRATULATIONS2;
	String CLOUDS_CONGRATULATIONS1;
	String CLOUDS_CONGRATULATIONS2;
	String GOOBER[3];
	String MUSIC_FILES1[5];
	String MUSIC_FILES2[6][7];
public:
	/**
	 * Constructor
	 */
	Resources();

	/**
	 * Initializes an instnace of the resources
	 */
	Resources *init(XeenEngine *vm);
};

extern Resources *g_resources;

} // End of namespace Xeen

#endif	/* XEEN_RESOURCES_H */
