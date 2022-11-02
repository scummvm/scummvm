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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "common/scummsys.h"
#include "cc.h"

enum MagicSpell {
	MS_AcidSpray = 0,
	MS_Awaken = 1,
	MS_BeastMaster = 2,
	MS_Bless = 3,
	MS_Clairvoyance = 4,
	MS_ColdRay = 5,
	MS_CreateFood = 6,
	MS_CureDisease = 7,
	MS_CureParalysis = 8,
	MS_CurePoison = 9,
	MS_CureWounds = 10,
	MS_DancingSword = 11,
	MS_DayOfProtection = 12,
	MS_DayOfSorcery = 13,
	MS_DeadlySwarm = 14,
	MS_DetectMonster = 15,
	MS_DivineIntervention = 16,
	MS_DragonSleep = 17,
	MS_ElementalStorm = 18,
	MS_EnchantItem = 19,
	MS_EnergyBlast = 20,
	MS_Etheralize = 21,
	MS_FantasticFreeze = 22,
	MS_FieryFlail = 23,
	MS_FingerOfDeath = 24,
	MS_Fireball = 25,
	MS_FirstAid = 26,
	MS_FlyingFist = 27,
	MS_FrostBite = 28,
	MS_GolemStopper = 29,
	MS_Heroism = 30,
	MS_HolyBonus = 31,
	MS_HolyWord = 32,
	MS_Hynotize = 33,
	MS_IdentifyMonster = 34,
	MS_Implosion = 35,
	MS_Incinerate = 36,
	MS_Inferno = 37,
	MS_InsectSpray = 38,
	MS_ItemToGold = 39,
	MS_Jump = 40,
	MS_Levitate = 41,
	MS_Light = 42,
	MS_LightningBolt = 43,
	MS_LloydsBeacon = 44,
	MS_MagicArrow = 45,
	MS_MassDistortion = 46,
	MS_MegaVolts = 47,
	MS_MoonRay = 48,
	MS_NaturesCure = 49,
	MS_Pain = 50,
	MS_PoisonVolley = 51,
	MS_PowerCure = 52,
	MS_PowerShield = 53,
	MS_PrismaticLight = 54,
	MS_ProtFromElements = 55,
	MS_RaiseDead = 56,
	MS_RechargeItem = 57,
	MS_Resurrection = 58,
	MS_Revitalize = 59,
	MS_Shrapmetal = 60,
	MS_Sleep = 61,
	MS_Sparks = 62,
	MS_StarBurst = 63,
	MS_StoneToFlesh = 64,
	MS_SunRay = 65,
	MS_SuperShelter = 66,
	MS_SuppressDisease = 67,
	MS_SuppressPoison = 68,
	MS_Teleport = 69,
	MS_TimeDistortion = 70,
	MS_TownPortal = 71,
	MS_ToxicCloud = 72,
	MS_TurnUndead = 73,
	MS_WalkOnWater = 74,
	MS_WizardEye = 75,
	NO_SPELL = 76
};

class LangConstants {
public:
	static const char *const TERRAIN_TYPES[6];
	static const char *const OUTDOORS_WALL_TYPES[16];
	static const char *const SURFACE_NAMES[16];
	static const byte SYMBOLS[20][64];
	static const byte TEXT_COLORS[40][4];
	static const byte TEXT_COLORS_STARTUP[40][4];
	static const int RACE_HP_BONUSES[5];
	static const int RACE_SP_BONUSES[5][2];
	static const int CLASS_EXP_LEVELS[10];
	static const int CONDITION_COLORS[17];
	static const int FACE_CONDITION_FRAMES[17];
	static const int CHAR_FACES_X[6];
	static const int HP_BARS_X[6];
	static const byte DARKNESS_XLAT[3][256];
	static const int SCREEN_POSITIONING_X[4][48];
	static const int SCREEN_POSITIONING_Y[4][48];
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
	static const int SPELLS_ALLOWED[3][40];
	static const int BASE_HP_BY_CLASS[10];
	static const int AGE_RANGES[10];
	static const int AGE_RANGES_ADJUST[2][10];
	static const int STAT_VALUES[24];
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
	static const int METAL_LAC[22];
	static const int ARMOR_STRENGTHS[14];
	static const int MAKE_ITEM_ARR1[6];
	static const int MAKE_ITEM_ARR2[6][7][2];
	static const int MAKE_ITEM_ARR3[10][7][2];
	static const int MAKE_ITEM_ARR4[2][7][2];
	static const int MAKE_ITEM_ARR5[8][2];
	static const int OUTDOOR_DRAWSTRUCT_INDEXES[44];
	static const int TOWN_MAXES[2][11];
	static const char *const TOWN_ACTION_MUSIC[2][7];
	static const char *const TOWN_ACTION_SHAPES[7];
	static const int TOWN_ACTION_FILES[2][7];
	static const int TAVERN_EXIT_LIST[2][6][5][2];
	static const int MISC_SPELL_INDEX[74];
	static const int SPELL_COSTS[77];
	static const int DARK_SPELL_RANGES[12][2];
	static const int SWORDS_SPELL_RANGES[12][2];
	static const int CLOUDS_GUILD_SPELLS[5][20];
	static const int DARK_SPELL_OFFSETS[3][39];
	static const int SPELL_GEM_COST[77];
	static const int BLACKSMITH_MAP_IDS[2][4];
	static const int WEAPON_BASE_COSTS[35];
	static const int ARMOR_BASE_COSTS[14];
	static const int ACCESSORY_BASE_COSTS[11];
	static const int MISC_MATERIAL_COSTS[22];
	static const int MISC_BASE_COSTS[76];
	static const int METAL_BASE_MULTIPLIERS[22];
	static const int ITEM_SKILL_DIVISORS[4];
	static const int RESTRICTION_OFFSETS[4];
	static const int ITEM_RESTRICTIONS[86];
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
	static const int TOWN_MAP_NUMBERS[3][5];
	static const char *const EVENT_SAMPLES[6];
	static const char *const MUSIC_FILES1[5];
	static const char *const MUSIC_FILES2[6][7];

	virtual ~LangConstants() {}
	virtual const char  *CLOUDS_CREDITS() = 0;
	virtual const char  *DARK_SIDE_CREDITS() = 0;
	virtual const char  *SWORDS_CREDITS1() = 0;
	virtual const char  *SWORDS_CREDITS2() = 0;
	virtual const char  *OPTIONS_MENU() = 0;
	virtual const char **GAME_NAMES() = 0;
	virtual const char  *THE_PARTY_NEEDS_REST() = 0;
	virtual const char  *WHO_WILL() = 0;
	virtual const char  *HOW_MUCH() = 0;
	virtual const char  *WHATS_THE_PASSWORD() = 0;
	virtual const char  *PASSWORD_INCORRECT() = 0;
	virtual const char  *IN_NO_CONDITION() = 0;
	virtual const char  *NOTHING_HERE() = 0;
	virtual const char **WHO_ACTIONS() = 0;
	virtual const char **WHO_WILL_ACTIONS() = 0;
	virtual const char **DIRECTION_TEXT_UPPER() = 0;
	virtual const char **DIRECTION_TEXT() = 0;
	virtual const char **RACE_NAMES() = 0;
	virtual const char **ALIGNMENT_NAMES() = 0;
	virtual const char **SEX_NAMES() = 0;
	virtual const char **SKILL_NAMES() = 0;
	virtual const char **CLASS_NAMES() = 0;
	virtual const char **CONDITION_NAMES_M() = 0;
	virtual const char **CONDITION_NAMES_F() = 0;
	virtual const char *GOOD() = 0;
	virtual const char *BLESSED() = 0;
	virtual const char *POWER_SHIELD() = 0;
	virtual const char *HOLY_BONUS() = 0;
	virtual const char *HEROISM() = 0;
	virtual const char *IN_PARTY() = 0;
	virtual const char *PARTY_DETAILS() = 0;
	virtual const char *PARTY_DIALOG_TEXT() = 0;
	virtual const char *NO_ONE_TO_ADVENTURE_WITH() = 0;
	virtual const char *YOUR_ROSTER_IS_FULL() = 0;
	virtual const char *PLEASE_WAIT() = 0;
	virtual const char *OOPS() = 0;
	virtual const char *BANK_TEXT() = 0;
	virtual const char *BLACKSMITH_TEXT() = 0;
	virtual const char *GUILD_NOT_MEMBER_TEXT() = 0;
	virtual const char *GUILD_TEXT() = 0;
	virtual const char *TAVERN_TEXT() = 0;
	virtual const char *FOOD_AND_DRINK() = 0;
	virtual const char *GOOD_STUFF() = 0;
	virtual const char *HAVE_A_DRINK() = 0;
	virtual const char *YOURE_DRUNK() = 0;
	virtual const char  *TEMPLE_TEXT() = 0;
	virtual const char  *EXPERIENCE_FOR_LEVEL() = 0;
	virtual const char  *TRAINING_LEARNED_ALL() = 0;
	virtual const char  *ELIGIBLE_FOR_LEVEL() = 0;
	virtual const char  *TRAINING_TEXT() = 0;
	virtual const char  *GOLD_GEMS() = 0;
	virtual const char  *GOLD_GEMS_2() = 0;
	virtual const char **DEPOSIT_WITHDRAWL() = 0;
	virtual const char  *NOT_ENOUGH_X_IN_THE_Y() = 0;
	virtual const char  *NO_X_IN_THE_Y() = 0;
	virtual const char **STAT_NAMES() = 0;
	virtual const char **CONSUMABLE_NAMES() = 0;
	virtual const char **CONSUMABLE_GOLD_FORMS() = 0;
	virtual const char **CONSUMABLE_GEM_FORMS() = 0;
	virtual const char **WHERE_NAMES() = 0;
	virtual const char  *AMOUNT() = 0;
	virtual const char  *FOOD_PACKS_FULL() = 0;
	virtual const char  *BUY_SPELLS() = 0;
	virtual const char  *GUILD_OPTIONS() = 0;
	virtual const char  *NOT_A_SPELL_CASTER() = 0;
	virtual const char  *SPELLS_LEARNED_ALL() = 0;
	virtual const char  *SPELLS_FOR() = 0;
	virtual const char  *SPELL_LINES_0_TO_9() = 0;
	virtual const char  *SPELLS_DIALOG_SPELLS() = 0;
	virtual const char  *SPELL_PTS() = 0;
	virtual const char  *GOLD() = 0;
	virtual const char  *SPELL_INFO() = 0;
	virtual const char  *SPELL_PURCHASE() = 0;
	virtual const char  *MAP_TEXT() = 0;
	virtual const char  *LIGHT_COUNT_TEXT() = 0;
	virtual const char  *FIRE_RESISTENCE_TEXT() = 0;
	virtual const char  *ELECRICITY_RESISTENCE_TEXT() = 0;
	virtual const char  *COLD_RESISTENCE_TEXT() = 0;
	virtual const char  *POISON_RESISTENCE_TEXT() = 0;
	virtual const char  *CLAIRVOYANCE_TEXT() = 0;
	virtual const char  *LEVITATE_TEXT() = 0;
	virtual const char  *WALK_ON_WATER_TEXT() = 0;
	virtual const char  *GAME_INFORMATION() = 0;
	virtual const char  *WORLD_GAME_TEXT() = 0;
	virtual const char  *DARKSIDE_GAME_TEXT() = 0;
	virtual const char  *CLOUDS_GAME_TEXT() = 0;
	virtual const char  *SWORDS_GAME_TEXT() = 0;
	virtual const char **WEEK_DAY_STRINGS() = 0;
	virtual const char  *CHARACTER_DETAILS() = 0;
	virtual const char **DAYS() = 0;
	virtual const char  *PARTY_GOLD() = 0;
	virtual const char  *PLUS_14() = 0;
	virtual const char  *CHARACTER_TEMPLATE() = 0;
	virtual const char  *EXCHANGING_IN_COMBAT() = 0;
	virtual const char  *CURRENT_MAXIMUM_RATING_TEXT() = 0;
	virtual const char  *CURRENT_MAXIMUM_TEXT() = 0;
	virtual const char **RATING_TEXT() = 0;
	virtual const char **BORN() = 0;
	virtual const char  *AGE_TEXT() = 0;
	virtual const char  *LEVEL_TEXT() = 0;
	virtual const char  *RESISTENCES_TEXT() = 0;
	virtual const char  *NONE() = 0;
	virtual const char  *EXPERIENCE_TEXT() = 0;
	virtual const char  *ELIGIBLE() = 0;
	virtual const char  *IN_PARTY_IN_BANK() = 0;
	virtual const char **FOOD_ON_HAND() = 0;
	virtual const char  *FOOD_TEXT() = 0;
	virtual const char  *EXCHANGE_WITH_WHOM() = 0;
	virtual const char  *QUICK_REF_LINE() = 0;
	virtual const char  *QUICK_REFERENCE() = 0;
	virtual const char  *ITEMS_DIALOG_TEXT1() = 0;
	virtual const char  *ITEMS_DIALOG_TEXT2() = 0;
	virtual const char  *ITEMS_DIALOG_LINE1() = 0;
	virtual const char  *ITEMS_DIALOG_LINE2() = 0;
	virtual const char  *BTN_BUY() = 0;
	virtual const char  *BTN_SELL() = 0;
	virtual const char  *BTN_IDENTIFY() = 0;
	virtual const char  *BTN_FIX() = 0;
	virtual const char  *BTN_USE() = 0;
	virtual const char  *BTN_EQUIP() = 0;
	virtual const char  *BTN_REMOVE() = 0;
	virtual const char  *BTN_DISCARD() = 0;
	virtual const char  *BTN_QUEST() = 0;
	virtual const char  *BTN_ENCHANT() = 0;
	virtual const char  *BTN_RECHARGE() = 0;
	virtual const char  *BTN_GOLD() = 0;
	virtual const char  *ITEM_BROKEN() = 0;
	virtual const char  *ITEM_CURSED() = 0;
	virtual const char  *ITEM_OF() = 0;
	virtual const char **BONUS_NAMES() = 0;
	virtual const char **WEAPON_NAMES() = 0;
	virtual const char **ARMOR_NAMES() = 0;
	virtual const char **ACCESSORY_NAMES() = 0;
	virtual const char **MISC_NAMES() = 0;
	virtual const char **SPECIAL_NAMES() = 0;
	virtual const char **ELEMENTAL_NAMES() = 0;
	virtual const char **ATTRIBUTE_NAMES() = 0;
	virtual const char **EFFECTIVENESS_NAMES() = 0;
	virtual const char **QUEST_ITEM_NAMES() = 0;
	virtual const char **QUEST_ITEM_NAMES_SWORDS() = 0;
	virtual const char  *NOT_PROFICIENT() = 0;
	virtual const char  *NO_ITEMS_AVAILABLE() = 0;
	virtual const char **CATEGORY_NAMES() = 0;
	virtual const char  *X_FOR_THE_Y() = 0;
	virtual const char  *X_FOR_Y() = 0;
	virtual const char  *X_FOR_Y_GOLD() = 0;
	virtual const char  *FMT_CHARGES() = 0;
	virtual const char  *AVAILABLE_GOLD_COST() = 0;
	virtual const char  *CHARGES() = 0;
	virtual const char  *COST() = 0;
	virtual const char **ITEM_ACTIONS() = 0;
	virtual const char  *WHICH_ITEM() = 0;
	virtual const char  *WHATS_YOUR_HURRY() = 0;
	virtual const char  *USE_ITEM_IN_COMBAT() = 0;
	virtual const char  *NO_SPECIAL_ABILITIES() = 0;
	virtual const char  *CANT_CAST_WHILE_ENGAGED() = 0;
	virtual const char  *EQUIPPED_ALL_YOU_CAN() = 0;
	virtual const char  *REMOVE_X_TO_EQUIP_Y() = 0;
	virtual const char  *RING() = 0;
	virtual const char  *MEDAL() = 0;
	virtual const char  *CANNOT_REMOVE_CURSED_ITEM() = 0;
	virtual const char  *CANNOT_DISCARD_CURSED_ITEM() = 0;
	virtual const char  *PERMANENTLY_DISCARD() = 0;
	virtual const char  *BACKPACK_IS_FULL() = 0;
	virtual const char **CATEGORY_BACKPACK_IS_FULL() = 0;
	virtual const char  *BUY_X_FOR_Y_GOLD() = 0;
	virtual const char  *SELL_X_FOR_Y_GOLD() = 0;
	virtual const char **GOLDS() = 0;
	virtual const char  *NO_NEED_OF_THIS() = 0;
	virtual const char  *NOT_RECHARGABLE() = 0;
	virtual const char  *NOT_ENCHANTABLE() = 0;
	virtual const char  *SPELL_FAILED() = 0;
	virtual const char  *ITEM_NOT_BROKEN() = 0;
	virtual const char **FIX_IDENTIFY() = 0;
	virtual const char  *FIX_IDENTIFY_GOLD() = 0;
	virtual const char  *IDENTIFY_ITEM_MSG() = 0;
	virtual const char  *ITEM_DETAILS() = 0;
	virtual const char  *ALL() = 0;
	virtual const char  *FIELD_NONE() = 0;
	virtual const char  *DAMAGE_X_TO_Y() = 0;
	virtual const char  *ELEMENTAL_XY_DAMAGE() = 0;
	virtual const char  *ATTR_XY_BONUS() = 0;
	virtual const char  *EFFECTIVE_AGAINST() = 0;
	virtual const char  *QUESTS_DIALOG_TEXT() = 0;
	virtual const char  *CLOUDS_OF_XEEN_LINE() = 0;
	virtual const char  *DARKSIDE_OF_XEEN_LINE() = 0;
	virtual const char  *SWORDS_OF_XEEN_LINE() = 0;
	virtual const char  *NO_QUEST_ITEMS() = 0;
	virtual const char  *NO_CURRENT_QUESTS() = 0;
	virtual const char  *NO_AUTO_NOTES() = 0;
	virtual const char  *QUEST_ITEMS_DATA() = 0;
	virtual const char  *CURRENT_QUESTS_DATA() = 0;
	virtual const char  *AUTO_NOTES_DATA() = 0;
	virtual const char  *REST_COMPLETE() = 0;
	virtual const char  *PARTY_IS_STARVING() = 0;
	virtual const char  *HIT_SPELL_POINTS_RESTORED() = 0;
	virtual const char  *TOO_DANGEROUS_TO_REST() = 0;
	virtual const char  *SOME_CHARS_MAY_DIE() = 0;
	virtual const char  *DISMISS_WHOM() = 0;
	virtual const char  *CANT_DISMISS_LAST_CHAR() = 0;
	virtual const char  *DELETE_CHAR_WITH_ELDER_WEAPON() = 0;
	virtual const char **REMOVE_DELETE() = 0;
	virtual const char  *REMOVE_OR_DELETE_WHICH() = 0;
	virtual const char  *YOUR_PARTY_IS_FULL() = 0;
	virtual const char  *HAS_SLAYER_SWORD() = 0;
	virtual const char  *SURE_TO_DELETE_CHAR() = 0;
	virtual const char  *CREATE_CHAR_DETAILS() = 0;
	virtual const char  *NEW_CHAR_STATS() = 0;
	virtual const char  *NAME_FOR_NEW_CHARACTER() = 0;
	virtual const char  *SELECT_CLASS_BEFORE_SAVING() = 0;
	virtual const char  *EXCHANGE_ATTR_WITH() = 0;
	virtual const int *NEW_CHAR_SKILLS_OFFSET() = 0;
	virtual const char  *COMBAT_DETAILS() = 0;
	virtual const char  *NOT_ENOUGH_TO_CAST() = 0;
	virtual const char **SPELL_CAST_COMPONENTS() = 0;
	virtual const char  *CAST_SPELL_DETAILS() = 0;
	virtual const char  *PARTY_FOUND() = 0;
	virtual const char  *BACKPACKS_FULL_PRESS_KEY() = 0;
	virtual const char  *HIT_A_KEY() = 0;
	virtual const char  *GIVE_TREASURE_FORMATTING() = 0;
	virtual const char **FOUND() = 0;
	virtual const char  *X_FOUND_Y() = 0;
	virtual const char  *ON_WHO() = 0;
	virtual const char  *WHICH_ELEMENT1() = 0;
	virtual const char  *WHICH_ELEMENT2() = 0;
	virtual const char  *DETECT_MONSTERS() = 0;
	virtual const char  *LLOYDS_BEACON() = 0;
	virtual const char  *HOW_MANY_SQUARES() = 0;
	virtual const char  *TOWN_PORTAL() = 0;
	virtual const char  *TOWN_PORTAL_SWORDS() = 0;
	virtual const char  *MONSTER_DETAILS() = 0;
	virtual const char **MONSTER_SPECIAL_ATTACKS() = 0;
	virtual const char  *IDENTIFY_MONSTERS() = 0;
	virtual const char  *MOONS_NOT_ALIGNED() = 0;
	virtual const char  *AWARDS_FOR() = 0;
	virtual const char  *AWARDS_TEXT() = 0;
	virtual const char  *NO_AWARDS() = 0;
	virtual const char  *WARZONE_BATTLE_MASTER() = 0;
	virtual const char  *WARZONE_MAXED() = 0;
	virtual const char  *WARZONE_LEVEL() = 0;
	virtual const char  *WARZONE_HOW_MANY() = 0;
	virtual const char  *PICKS_THE_LOCK() = 0;
	virtual const char **PICK_FORM() = 0;
	virtual const char  *UNABLE_TO_PICK_LOCK() = 0;
	virtual const char **UNABLE_TO_PICK_FORM() = 0;
	virtual const char  *CONTROL_PANEL_TEXT() = 0;
	virtual const char  *CONTROL_PANEL_BUTTONS() = 0;
	virtual const char  *ON() = 0;
	virtual const char  *OFF() = 0;
	virtual const char  *CONFIRM_QUIT() = 0;
	virtual const char  *MR_WIZARD() = 0;
	virtual const char  *NO_LOADING_IN_COMBAT() = 0;
	virtual const char  *NO_SAVING_IN_COMBAT() = 0;
	virtual const char  *QUICK_FIGHT_TEXT() = 0;
	virtual const char **QUICK_FIGHT_OPTIONS() = 0;
	virtual const char **WORLD_END_TEXT() = 0;
	virtual const char  *WORLD_CONGRATULATIONS() = 0;
	virtual const char  *WORLD_CONGRATULATIONS2() = 0;
	virtual const char  *CLOUDS_CONGRATULATIONS1() = 0;
	virtual const char  *CLOUDS_CONGRATULATIONS2() = 0;
	virtual const char **GOOBER() = 0;
	virtual const char  *DIFFICULTY_TEXT() = 0;
	virtual const char  *SAVE_OFF_LIMITS() = 0;
	virtual const char  *CLOUDS_INTRO1() = 0;
	virtual const char  *DARKSIDE_ENDING1() = 0;
	virtual const char  *DARKSIDE_ENDING2() = 0;
	virtual const char  *PHAROAH_ENDING_TEXT1() = 0;
	virtual const char  *PHAROAH_ENDING_TEXT2() = 0;
	virtual const char **CLOUDS_MAE_NAMES() = 0;
	virtual const char **CLOUDS_MIRROR_LOCATIONS() = 0;
	virtual const char **CLOUDS_MAP_NAMES() = 0;
	virtual const char **CLOUDS_MONSTERS() = 0;
	virtual const char **CLOUDS_SPELLS() = 0;

	void writeConstants(Common::String num, CCArchive &cc);

	class KeyConstants {
	public:
		class DialogsCharInfo {
		public:
			virtual int KEY_ITEM() = 0;
			virtual int KEY_QUICK() = 0;
			virtual int KEY_EXCHANGE() = 0;
			virtual ~DialogsCharInfo() {}
		};
		virtual DialogsCharInfo *dialogsCharInfo() = 0;

		class DialogsControlPanel {
		public:
			virtual int KEY_FXON() = 0;
			virtual int KEY_MUSICON() = 0;
			virtual int KEY_LOAD() = 0;
			virtual int KEY_SAVE() = 0;
			virtual int KEY_QUIT() = 0;
			virtual int KEY_MRWIZARD() = 0;
			virtual ~DialogsControlPanel() {}
		};
		virtual DialogsControlPanel *dialogsControlPanel() = 0;

		class DialogsCreateChar {
		public:
			virtual int KEY_ROLL() = 0;
			virtual int KEY_CREATE() = 0;
			virtual int KEY_MGT() = 0;
			virtual int KEY_INT() = 0;
			virtual int KEY_PER() = 0;
			virtual int KEY_END() = 0;
			virtual int KEY_SPD() = 0;
			virtual int KEY_ACY() = 0;
			virtual int KEY_LCK() = 0;
			virtual ~DialogsCreateChar() {}
		};
		virtual DialogsCreateChar *dialogsCreateChar() = 0;

		class DialogsDifficulty {
		public:
			virtual int KEY_ADVENTURER() = 0;
			virtual int KEY_WARRIOR() = 0;
			virtual ~DialogsDifficulty() {}
		};
		virtual DialogsDifficulty *dialogsDifficulty() = 0;

		class DialogsItems {
		public:
			virtual int KEY_WEAPONS() = 0;
			virtual int KEY_ARMOR() = 0;
			virtual int KEY_ACCESSORY() = 0;
			virtual int KEY_MISC() = 0;
			virtual int KEY_ENCHANT() = 0;
			virtual int KEY_USE() = 0;
			virtual int KEY_BUY() = 0;
			virtual int KEY_SELL() = 0;
			virtual int KEY_IDENTIFY() = 0;
			virtual int KEY_FIX() = 0;
			virtual int KEY_EQUIP() = 0;
			virtual int KEY_REM() = 0;
			virtual int KEY_DISC() = 0;
			virtual int KEY_QUEST() = 0;
			virtual int KEY_RECHRG() = 0;
			virtual int KEY_GOLD() = 0;
			virtual ~DialogsItems() {}
		};
		virtual DialogsItems *dialogsItems() = 0;

		class DialogsParty {
		public:
			virtual int KEY_DELETE() = 0;
			virtual int KEY_REMOVE() = 0;
			virtual int KEY_CREATE() = 0;
			virtual int KEY_EXIT() = 0;
			virtual ~DialogsParty() {}
		};
		virtual DialogsParty *dialogsParty() = 0;

		class DialogsQuests {
		public:
			virtual int KEY_QUEST_ITEMS() = 0;
			virtual int KEY_CURRENT_QUESTS() = 0;
			virtual int KEY_AUTO_NOTES() = 0;
			virtual ~DialogsQuests() {}
		};
		virtual DialogsQuests *dialogsQuests() = 0;

		class DialogsQuickFight {
		public:
			virtual int KEY_NEXT() = 0;
			virtual ~DialogsQuickFight() {}
		};
		virtual DialogsQuickFight *dialogsQuickFight() = 0;

		class DialogsSpells {
		public:
			virtual int KEY_CAST() = 0;
			virtual int KEY_NEW() = 0;
			virtual int KEY_FIRE() = 0;
			virtual int KEY_ELEC() = 0;
			virtual int KEY_COLD() = 0;
			virtual int KEY_ACID() = 0;
			virtual int KEY_SET() = 0;
			virtual int KEY_RETURN() = 0;
			virtual ~DialogsSpells() {}
		};
		virtual DialogsSpells *dialogsSpells() = 0;

		class Locations {
		public:
			virtual int KEY_DEP() = 0;
			virtual int KEY_WITH() = 0;
			virtual int KEY_GOLD() = 0;
			virtual int KEY_GEMS() = 0;
			virtual int KEY_BROWSE() = 0;
			virtual int KEY_BUY_SPELLS() = 0;
			virtual int KEY_SPELL_INFO() = 0;
			virtual int KEY_SIGN_IN() = 0;
			virtual int KEY_DRINK() = 0;
			virtual int KEY_FOOD() = 0;
			virtual int KEY_TIP() = 0;
			virtual int KEY_RUMORS() = 0;
			virtual int KEY_HEAL() = 0;
			virtual int KEY_DONATION() = 0;
			virtual int KEY_UNCURSE() = 0;
			virtual int KEY_TRAIN() = 0;
			virtual ~Locations() {}
		};
		virtual Locations *locations() = 0;

		class CloudsOfXeenMenu {
		public:
			virtual int KEY_START_NEW_GAME() = 0;
			virtual int KEY_LOAD_GAME() = 0;
			virtual int KEY_SHOW_CREDITS() = 0;
			virtual int KEY_VIEW_ENDGAME() = 0;
			virtual ~CloudsOfXeenMenu() {}
		};
		virtual CloudsOfXeenMenu *cloudsOfXeenMenu() = 0;
		virtual ~KeyConstants() {}
	};
	virtual KeyConstants *keyConstants() = 0;
};

extern void writeConstants(CCArchive &cc);

#endif
