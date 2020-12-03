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
#define STRING_BUFFER_SIZE 32768

class XeenEngine;

class Resources {
	/**
	 * Derived file class with sync method aliases so that the same
	 * code from create_xeen can be re-used for both reading and
	 * writing the resource data
	 */	
	class ResFile : public File {
	private:
		char *_buffer;
		char *_buffStart;
	public:
		ResFile(char *buffer) : File("CONSTANTS"), _buffer(buffer), _buffStart(buffer) {}

		void syncString(const char *&str) {
			str = _buffer;
			strcpy(_buffer, readString().c_str());
			_buffer += strlen(_buffer) + 1;
			assert((_buffer - _buffStart) < STRING_BUFFER_SIZE);
		}
		void syncStrings(const char **str, int count) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count, 0, 0, 0));
			for (int idx = 0; idx < count; ++idx)
				syncString(str[idx]);
		}
		void syncStrings2D(const char **str, int count1, int count2) {
			uint tag = readUint32LE();
			assert(tag == MKTAG(count1, count2, 0, 0));
			for (int idx = 0; idx < count1 * count2; ++idx)
				syncString(str[idx]);
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
	char _buffer[STRING_BUFFER_SIZE];
private:
	/**
	 * Loads all the constants data stored in the xeen.ccs archive shipped with ScummVM
	 */
	void loadData();
public:
	SpriteResource _globalSprites;
	SpriteResource _logoSprites;
	Common::StringArray _maeNames;			// Magic and equipment names
	Common::StringArray _cloudsMapNames;	// Clouds of Xeen map names
	const char **ITEM_NAMES[4];

	// Data loaded from xeen.ccs
	const char *CLOUDS_CREDITS;
	const char *DARK_SIDE_CREDITS;
	const char *SWORDS_CREDITS1;
	const char *SWORDS_CREDITS2;
	const char *OPTIONS_MENU;
	const char *GAME_NAMES[3];
	const char *THE_PARTY_NEEDS_REST;
	const char *WHO_WILL;
	const char *HOW_MUCH;
	const char *WHATS_THE_PASSWORD;
	const char *PASSWORD_INCORRECT;
	const char *IN_NO_CONDITION;
	const char *NOTHING_HERE;
	const char *TERRAIN_TYPES[6];
	const char *OUTDOORS_WALL_TYPES[16];
	const char *SURFACE_NAMES[16];
	const char *WHO_ACTIONS[32];
	const char *WHO_WILL_ACTIONS[4];
	byte SYMBOLS[20][64];
	byte TEXT_COLORS[40][4];
	byte TEXT_COLORS_STARTUP[40][4];
	const char *DIRECTION_TEXT_UPPER[4];
	const char *DIRECTION_TEXT[4];
	const char *RACE_NAMES[5];
	int RACE_HP_BONUSES[5];
	int RACE_SP_BONUSES[5][2];
	const char *CLASS_NAMES[11];
	int CLASS_EXP_LEVELS[10];
	const char *ALIGNMENT_NAMES[3];
	const char *SEX_NAMES[2];
	const char *SKILL_NAMES[18];
	const char *CONDITION_NAMES[17];
	int CONDITION_COLORS[17];
	const char *GOOD;
	const char *BLESSED;
	const char *POWER_SHIELD;
	const char *HOLY_BONUS;
	const char *HEROISM;
	const char *IN_PARTY;
	const char *PARTY_DETAILS;
	const char *PARTY_DIALOG_TEXT;
	int FACE_CONDITION_FRAMES[17];
	int CHAR_FACES_X[6];
	int HP_BARS_X[6];
	const char *NO_ONE_TO_ADVENTURE_WITH;
	byte DARKNESS_XLAT[3][256];
	const char *YOUR_ROSTER_IS_FULL;
	const char *PLEASE_WAIT;
	const char *OOPS;
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
	int METAL_LAC[22];
	int ARMOR_STRENGTHS[14];
	int MAKE_ITEM_ARR1[6];
	int MAKE_ITEM_ARR2[6][7][2];
	int MAKE_ITEM_ARR3[10][7][2];
	int MAKE_ITEM_ARR4[2][7][2];
	int MAKE_ITEM_ARR5[8][2];
	int OUTDOOR_DRAWSTRUCT_INDEXES[44];
	int TOWN_MAXES[2][11];
	const char *TOWN_ACTION_MUSIC[2][7];
	const char *TOWN_ACTION_SHAPES[7];
	int TOWN_ACTION_FILES[2][7];
	const char *BANK_TEXT;
	const char *BLACKSMITH_TEXT;
	const char *GUILD_NOT_MEMBER_TEXT;
	const char *GUILD_TEXT;
	const char *TAVERN_TEXT;
	const char *GOOD_STUFF;
	const char *HAVE_A_DRINK;
	const char *YOURE_DRUNK;
	int TAVERN_EXIT_LIST[2][6][5][2];
	const char *FOOD_AND_DRINK;
	const char *TEMPLE_TEXT;
	const char *EXPERIENCE_FOR_LEVEL;
	const char *TRAINING_LEARNED_ALL;
	const char *ELIGIBLE_FOR_LEVEL;
	const char *TRAINING_TEXT;
	const char *GOLD_GEMS;
	const char *GOLD_GEMS_2;
	const char *DEPOSIT_WITHDRAWL[2];
	const char *NOT_ENOUGH_X_IN_THE_Y;
	const char *NO_X_IN_THE_Y;
	const char *STAT_NAMES[16];
	const char *CONSUMABLE_NAMES[4];
	const char *WHERE_NAMES[2];
	const char *AMOUNT;
	const char *FOOD_PACKS_FULL;
	const char *BUY_SPELLS;
	const char *GUILD_OPTIONS;
	int MISC_SPELL_INDEX[74];
	int SPELL_COSTS[77];
	int CLOUDS_GUILD_SPELLS[5][20];
	int DARK_SPELL_OFFSETS[3][39];
	int DARK_SPELL_RANGES[12][2];
	int SWORDS_SPELL_RANGES[12][2];
	int SPELL_GEM_COST[77];
	const char *NOT_A_SPELL_CASTER;
	const char *SPELLS_LEARNED_ALL;
	const char *SPELLS_FOR;
	const char *SPELL_LINES_0_TO_9;
	const char *SPELLS_DIALOG_SPELLS;
	const char *SPELL_PTS;
	const char *GOLD;
	const char *SPELL_INFO;
	const char *SPELL_PURCHASE;
	const char *MAP_TEXT;
	const char *LIGHT_COUNT_TEXT;
	const char *FIRE_RESISTENCE_TEXT;
	const char *ELECRICITY_RESISTENCE_TEXT;
	const char *COLD_RESISTENCE_TEXT;
	const char *POISON_RESISTENCE_TEXT;
	const char *CLAIRVOYANCE_TEXT;
	const char *LEVITATE_TEXT;
	const char *WALK_ON_WATER_TEXT;
	const char *GAME_INFORMATION;
	const char *WORLD_GAME_TEXT;
	const char *DARKSIDE_GAME_TEXT;
	const char *CLOUDS_GAME_TEXT;
	const char *SWORDS_GAME_TEXT;
	const char *WEEK_DAY_STRINGS[10];
	const char *CHARACTER_DETAILS;
	const char *PARTY_GOLD;
	const char *PLUS_14;
	const char *CHARACTER_TEMPLATE;
	const char *EXCHANGING_IN_COMBAT;
	const char *CURRENT_MAXIMUM_RATING_TEXT;
	const char *CURRENT_MAXIMUM_TEXT;
	const char *RATING_TEXT[24];
	const char *AGE_TEXT;
	const char *LEVEL_TEXT;
	const char *RESISTENCES_TEXT;
	const char *NONE;
	const char *EXPERIENCE_TEXT;
	const char *ELIGIBLE;
	const char *IN_PARTY_IN_BANK;
	const char *FOOD_TEXT;
	const char *EXCHANGE_WITH_WHOM;
	const char *QUICK_REF_LINE;
	const char *QUICK_REFERENCE;
	int BLACKSMITH_MAP_IDS[2][4];
	const char *ITEMS_DIALOG_TEXT1;
	const char *ITEMS_DIALOG_TEXT2;
	const char *ITEMS_DIALOG_LINE1;
	const char *ITEMS_DIALOG_LINE2;
	const char *BTN_BUY;
	const char *BTN_SELL;
	const char *BTN_IDENTIFY;
	const char *BTN_FIX;
	const char *BTN_USE;
	const char *BTN_EQUIP;
	const char *BTN_REMOVE;
	const char *BTN_DISCARD;
	const char *BTN_QUEST;
	const char *BTN_ENCHANT;
	const char *BTN_RECHARGE;
	const char *BTN_GOLD;
	const char *ITEM_BROKEN;
	const char *ITEM_CURSED;
	const char *ITEM_OF;
	const char *BONUS_NAMES[7];
	const char *WEAPON_NAMES[41];
	const char *ARMOR_NAMES[14];
	const char *ACCESSORY_NAMES[11];
	const char *MISC_NAMES[22];
	const char *SPECIAL_NAMES[74];
	const char *ELEMENTAL_NAMES[6];
	const char *ATTRIBUTE_NAMES[10];
	const char *EFFECTIVENESS_NAMES[7];
	const char *QUEST_ITEM_NAMES[85];
	const char *QUEST_ITEM_NAMES_SWORDS[51];
	int WEAPON_BASE_COSTS[35];
	int ARMOR_BASE_COSTS[14];
	int ACCESSORY_BASE_COSTS[11];
	int MISC_MATERIAL_COSTS[22];
	int MISC_BASE_COSTS[76];
	int METAL_BASE_MULTIPLIERS[22];
	int ITEM_SKILL_DIVISORS[4];
	int RESTRICTION_OFFSETS[4];
	int ITEM_RESTRICTIONS[86];
	const char *NOT_PROFICIENT;
	const char *NO_ITEMS_AVAILABLE;
	const char *CATEGORY_NAMES[4];
	const char *X_FOR_THE_Y;
	const char *X_FOR_Y;
	const char *X_FOR_Y_GOLD;
	const char *FMT_CHARGES;
	const char *AVAILABLE_GOLD_COST;
	const char *CHARGES;
	const char *COST;
	const char *ITEM_ACTIONS[7];
	const char *WHICH_ITEM;
	const char *WHATS_YOUR_HURRY;
	const char *USE_ITEM_IN_COMBAT;
	const char *NO_SPECIAL_ABILITIES;
	const char *CANT_CAST_WHILE_ENGAGED;
	const char *EQUIPPED_ALL_YOU_CAN;
	const char *REMOVE_X_TO_EQUIP_Y;
	const char *RING;
	const char *MEDAL;
	const char *CANNOT_REMOVE_CURSED_ITEM;
	const char *CANNOT_DISCARD_CURSED_ITEM;
	const char *PERMANENTLY_DISCARD;
	const char *BACKPACK_IS_FULL;
	const char *CATEGORY_BACKPACK_IS_FULL[4];
	const char *BUY_X_FOR_Y_GOLD;
	const char *SELL_X_FOR_Y_GOLD;
	const char *NO_NEED_OF_THIS;
	const char *NOT_RECHARGABLE;
	const char *SPELL_FAILED;
	const char *NOT_ENCHANTABLE;
	const char *ITEM_NOT_BROKEN;
	const char *FIX_IDENTIFY[2];
	const char *FIX_IDENTIFY_GOLD;
	const char *IDENTIFY_ITEM_MSG;
	const char *ITEM_DETAILS;
	const char *ALL;
	const char *FIELD_NONE;
	const char *DAMAGE_X_TO_Y;
	const char *ELEMENTAL_XY_DAMAGE;
	const char *ATTR_XY_BONUS;
	const char *EFFECTIVE_AGAINST;
	const char *QUESTS_DIALOG_TEXT;
	const char *CLOUDS_OF_XEEN_LINE;
	const char *DARKSIDE_OF_XEEN_LINE;
	const char *SWORDS_OF_XEEN_LINE;
	const char *NO_QUEST_ITEMS;
	const char *NO_CURRENT_QUESTS;
	const char *NO_AUTO_NOTES;
	const char *QUEST_ITEMS_DATA;
	const char *CURRENT_QUESTS_DATA;
	const char *AUTO_NOTES_DATA;
	const char *REST_COMPLETE;
	const char *PARTY_IS_STARVING;
	const char *HIT_SPELL_POINTS_RESTORED;
	const char *TOO_DANGEROUS_TO_REST;
	const char *SOME_CHARS_MAY_DIE;
	const char *DISMISS_WHOM;
	const char *CANT_DISMISS_LAST_CHAR;
	const char *DELETE_CHAR_WITH_ELDER_WEAPON;
	const char *REMOVE_DELETE[2];
	const char *REMOVE_OR_DELETE_WHICH;
	const char *YOUR_PARTY_IS_FULL;
	const char *HAS_SLAYER_SWORD;
	const char *SURE_TO_DELETE_CHAR;
	const char *CREATE_CHAR_DETAILS;
	const char *NEW_CHAR_STATS;
	const char *NAME_FOR_NEW_CHARACTER;
	const char *SELECT_CLASS_BEFORE_SAVING;
	const char *EXCHANGE_ATTR_WITH;
	int NEW_CHAR_SKILLS[10];
	int NEW_CHAR_SKILLS_OFFSET[10];
	int NEW_CHAR_SKILLS_LEN[10];
	int NEW_CHAR_RACE_SKILLS[10];
	int RACE_MAGIC_RESISTENCES[5];
	int RACE_FIRE_RESISTENCES[5];
	int RACE_ELECTRIC_RESISTENCES[5];
	int RACE_COLD_RESISTENCES[5];
	int RACE_ENERGY_RESISTENCES[5];
	int RACE_POISON_RESISTENCES[5];
	int NEW_CHARACTER_SPELLS[10][4];
	const char *COMBAT_DETAILS;
	const char *NOT_ENOUGH_TO_CAST;
	const char *SPELL_CAST_COMPONENTS[2];
	const char *CAST_SPELL_DETAILS;
	const char *PARTY_FOUND;
	const char *BACKPACKS_FULL_PRESS_KEY;
	const char *HIT_A_KEY;
	const char *GIVE_TREASURE_FORMATTING;
	const char *X_FOUND_Y;
	const char *ON_WHO;
	const char *WHICH_ELEMENT1;
	const char *WHICH_ELEMENT2;
	const char *DETECT_MONSTERS;
	const char *LLOYDS_BEACON;
	const char *HOW_MANY_SQUARES;
	const char *TOWN_PORTAL;
	const char *TOWN_PORTAL_SWORDS;
	int TOWN_MAP_NUMBERS[3][5];
	const char *MONSTER_DETAILS;
	const char *MONSTER_SPECIAL_ATTACKS[23];
	const char *IDENTIFY_MONSTERS;
	const char *EVENT_SAMPLES[6];
	const char *MOONS_NOT_ALIGNED;
	const char *AWARDS_FOR;
	const char *AWARDS_TEXT;
	const char *NO_AWARDS;
	const char *WARZONE_BATTLE_MASTER;
	const char *WARZONE_MAXED;
	const char *WARZONE_LEVEL;
	const char *WARZONE_HOW_MANY;
	const char *PICKS_THE_LOCK;
	const char *UNABLE_TO_PICK_LOCK;
	const char *CONTROL_PANEL_TEXT;
	const char *CONTROL_PANEL_BUTTONS;
	const char *ON;
	const char *OFF;
	const char *CONFIRM_QUIT;
	const char *MR_WIZARD;
	const char *NO_LOADING_IN_COMBAT;
	const char *NO_SAVING_IN_COMBAT;
	const char *QUICK_FIGHT_TEXT;
	const char *QUICK_FIGHT_OPTIONS[4];
	const char *WORLD_END_TEXT[9];
	const char *WORLD_CONGRATULATIONS;
	const char *WORLD_CONGRATULATIONS2;
	const char *CLOUDS_CONGRATULATIONS1;
	const char *CLOUDS_CONGRATULATIONS2;
	const char *GOOBER[3];
	const char *MUSIC_FILES1[5];
	const char *MUSIC_FILES2[6][7];
	const char *DIFFICULTY_TEXT;
	const char *SAVE_OFF_LIMITS;
	const char *CLOUDS_INTRO1;
	const char *DARKSIDE_ENDING1;
	const char *DARKSIDE_ENDING2;
	const char *PHAROAH_ENDING_TEXT1;
	const char *PHAROAH_ENDING_TEXT2;
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
