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

#include "common/scummsys.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/files.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

Resources *g_resources;

Resources::Resources() {
	g_resources = this;
	g_vm->_files->setGameCc(1);

	_globalSprites.load("global.icn");
	if (g_vm->getGameID() == GType_Swords)
		_logoSprites.load("logo.int");

	File f((g_vm->getGameID() == GType_Clouds) ? "mae.cld" : "mae.xen");
	while (f.pos() < f.size())
		_maeNames.push_back(f.readString());
	f.close();

	if (g_vm->getGameID() == GType_Clouds) {
		f.open("mapnames.cld");
		while (f.pos() < f.size())
			_cloudsMapNames.push_back(f.readString());
		f.close();
	}

	// Set up items array to map to the names of items in each category
	ITEM_NAMES[CATEGORY_WEAPON] = &WEAPON_NAMES[0];
	ITEM_NAMES[CATEGORY_ARMOR] = &ARMOR_NAMES[0];
	ITEM_NAMES[CATEGORY_ACCESSORY] = &ACCESSORY_NAMES[0];
	ITEM_NAMES[CATEGORY_MISC] = &MISC_NAMES[0];

	// Load the data from the resource
	loadData();
}

void Resources::loadData() {
	int langId;
	switch (g_vm->getLanguage()) {
	case Common::DE_DEU:
		langId = 4;
		break;
	case Common::RU_RUS:
		langId = 26;
		break;
	default:
		// English
		langId = 7;
		break;
	}

	ResFile file("CONSTANTS_", _buffer, langId);
	file.syncString(CLOUDS_CREDITS);
	file.syncString(DARK_SIDE_CREDITS);
	file.syncString(SWORDS_CREDITS1);
	file.syncString(SWORDS_CREDITS2);
	file.syncString(OPTIONS_MENU);
	file.syncStrings(GAME_NAMES, 3);
	file.syncString(THE_PARTY_NEEDS_REST);
	file.syncString(WHO_WILL);
	file.syncString(HOW_MUCH);
	file.syncString(WHATS_THE_PASSWORD);
	file.syncString(PASSWORD_INCORRECT);
	file.syncString(IN_NO_CONDITION);
	file.syncString(NOTHING_HERE);
	file.syncStrings(TERRAIN_TYPES, 6);
	file.syncStrings(OUTDOORS_WALL_TYPES, 16);
	file.syncStrings(SURFACE_NAMES, 16);
	file.syncStrings(WHO_ACTIONS, 32);
	file.syncStrings(WHO_WILL_ACTIONS, 4);
	file.syncBytes2D((byte *)SYMBOLS, 20, 64);
	file.syncBytes2D((byte *)TEXT_COLORS, 40, 4);
	file.syncBytes2D((byte *)TEXT_COLORS_STARTUP, 40, 4);
	file.syncStrings(DIRECTION_TEXT_UPPER, 4);
	file.syncStrings(DIRECTION_TEXT, 4);
	file.syncStrings(RACE_NAMES, 5);
	file.syncNumbers(RACE_HP_BONUSES, 5);
	file.syncNumbers2D((int *)RACE_SP_BONUSES, 5, 2);
	file.syncStrings(CLASS_NAMES, 11);
	file.syncNumbers(CLASS_EXP_LEVELS, 10);
	file.syncStrings(ALIGNMENT_NAMES, 3);
	file.syncStrings(SEX_NAMES, 2);
	file.syncStrings(SKILL_NAMES, 18);
	file.syncStrings(CONDITION_NAMES_M, 17);
	file.syncStrings(CONDITION_NAMES_F, 17);
	file.syncNumbers(CONDITION_COLORS, 17);
	file.syncString(GOOD);
	file.syncString(BLESSED);
	file.syncString(POWER_SHIELD);
	file.syncString(HOLY_BONUS);
	file.syncString(HEROISM);
	file.syncString(IN_PARTY);
	file.syncString(PARTY_DETAILS);
	file.syncString(PARTY_DIALOG_TEXT);
	file.syncNumbers(FACE_CONDITION_FRAMES, 17);
	file.syncNumbers(CHAR_FACES_X, 6);
	file.syncNumbers(HP_BARS_X, 6);
	file.syncString(NO_ONE_TO_ADVENTURE_WITH);
	file.syncBytes2D((byte *)DARKNESS_XLAT, 3, 256);
	file.syncString(YOUR_ROSTER_IS_FULL);
	file.syncString(PLEASE_WAIT);
	file.syncString(OOPS);
	file.syncNumbers2D((int *)SCREEN_POSITIONING_X, 4, 48);
	file.syncNumbers2D((int *)SCREEN_POSITIONING_Y, 4, 48);
	file.syncNumbers(MONSTER_GRID_BITMASK, 12);
	file.syncNumbers2D((int *)INDOOR_OBJECT_X, 2, 12);
	file.syncNumbers2D((int *)MAP_OBJECT_Y, 2, 12);
	file.syncNumbers(INDOOR_MONSTERS_Y, 4);
	file.syncNumbers2D((int *)OUTDOOR_OBJECT_X, 2, 12);
	file.syncNumbers(OUTDOOR_MONSTER_INDEXES, 26);
	file.syncNumbers(OUTDOOR_MONSTERS_Y, 26);
	file.syncNumbers2D((int *)DIRECTION_ANIM_POSITIONS, 4, 4);
	file.syncBytes2D((byte *)WALL_SHIFTS, 4, 48);
	file.syncNumbers(DRAW_NUMBERS, 25);
	file.syncNumbers2D((int *)DRAW_FRAMES, 25, 2);
	file.syncNumbers(COMBAT_FLOAT_X, 8);
	file.syncNumbers(COMBAT_FLOAT_Y, 8);
	file.syncNumbers2D((int *)MONSTER_EFFECT_FLAGS, 15, 8);
	file.syncNumbers2D((int *)SPELLS_ALLOWED, 3, 40);
	file.syncNumbers(BASE_HP_BY_CLASS, 10);
	file.syncNumbers(AGE_RANGES, 10);
	file.syncNumbers2D((int *)AGE_RANGES_ADJUST, 2, 10);
	file.syncNumbers(STAT_VALUES, 24);
	file.syncNumbers(STAT_BONUSES, 24);
	file.syncNumbers(ELEMENTAL_CATEGORIES, 6);
	file.syncNumbers(ATTRIBUTE_CATEGORIES, 10);
	file.syncNumbers(ATTRIBUTE_BONUSES, 72);
	file.syncNumbers(ELEMENTAL_RESISTANCES, 37);
	file.syncNumbers(ELEMENTAL_DAMAGE, 37);
	file.syncNumbers(WEAPON_DAMAGE_BASE, 35);
	file.syncNumbers(WEAPON_DAMAGE_MULTIPLIER, 35);
	file.syncNumbers(METAL_DAMAGE, 22);
	file.syncNumbers(METAL_DAMAGE_PERCENT, 22);
	file.syncNumbers(METAL_LAC, 22);
	file.syncNumbers(ARMOR_STRENGTHS, 14);
	file.syncNumbers(MAKE_ITEM_ARR1, 6);
	file.syncNumbers3D((int *)MAKE_ITEM_ARR2, 6, 7, 2);
	file.syncNumbers3D((int *)MAKE_ITEM_ARR3, 10, 7, 2);
	file.syncNumbers3D((int *)MAKE_ITEM_ARR4, 2, 7, 2);
	file.syncNumbers2D((int *)MAKE_ITEM_ARR5, 8, 2);
	file.syncNumbers(OUTDOOR_DRAWSTRUCT_INDEXES, 44);
	file.syncNumbers2D((int *)TOWN_MAXES, 2, 11);
	file.syncStrings2D(&TOWN_ACTION_MUSIC[0][0], 2, 7);
	file.syncStrings(TOWN_ACTION_SHAPES, 7);
	file.syncNumbers2D((int *)TOWN_ACTION_FILES, 2, 7);
	file.syncString(BANK_TEXT);
	file.syncString(BLACKSMITH_TEXT);
	file.syncString(GUILD_NOT_MEMBER_TEXT);
	file.syncString(GUILD_TEXT);
	file.syncString(TAVERN_TEXT);
	file.syncString(GOOD_STUFF);
	file.syncString(HAVE_A_DRINK);
	file.syncString(YOURE_DRUNK);
	file.syncNumbers4D((int *)TAVERN_EXIT_LIST, 2, 6, 5, 2);
	file.syncString(FOOD_AND_DRINK);
	file.syncString(TEMPLE_TEXT);
	file.syncString(EXPERIENCE_FOR_LEVEL);
	file.syncString(TRAINING_LEARNED_ALL);
	file.syncString(ELIGIBLE_FOR_LEVEL);
	file.syncString(TRAINING_TEXT);
	file.syncString(GOLD_GEMS);
	file.syncString(GOLD_GEMS_2);
	file.syncStrings(DEPOSIT_WITHDRAWL, 2);
	file.syncString(NOT_ENOUGH_X_IN_THE_Y);
	file.syncString(NO_X_IN_THE_Y);
	file.syncStrings(STAT_NAMES, 16);
	file.syncStrings(CONSUMABLE_NAMES, 4);
	file.syncStrings(CONSUMABLE_GOLD_FORMS, 1);
	file.syncStrings(CONSUMABLE_GEM_FORMS, 1);
	file.syncStrings(WHERE_NAMES, 2);
	file.syncString(AMOUNT);
	file.syncString(FOOD_PACKS_FULL);
	file.syncString(BUY_SPELLS);
	file.syncString(GUILD_OPTIONS);
	file.syncNumbers((int *)MISC_SPELL_INDEX, 74);
	file.syncNumbers((int *)SPELL_COSTS, 77);
	file.syncNumbers2D((int *)CLOUDS_GUILD_SPELLS, 5, 20);
	file.syncNumbers2D((int *)DARK_SPELL_OFFSETS, 3, 39);
	file.syncNumbers2D((int *)DARK_SPELL_RANGES, 12, 2);
	file.syncNumbers2D((int *)SWORDS_SPELL_RANGES, 12, 2);
	file.syncNumbers((int *)SPELL_GEM_COST, 77);
	file.syncString(NOT_A_SPELL_CASTER);
	file.syncString(SPELLS_LEARNED_ALL);
	file.syncString(SPELLS_FOR);
	file.syncString(SPELL_LINES_0_TO_9);
	file.syncString(SPELLS_DIALOG_SPELLS);
	file.syncString(SPELL_PTS);
	file.syncString(GOLD);
	file.syncString(SPELL_INFO);
	file.syncString(SPELL_PURCHASE);
	file.syncString(MAP_TEXT);
	file.syncString(LIGHT_COUNT_TEXT);
	file.syncString(FIRE_RESISTANCE_TEXT);
	file.syncString(ELECRICITY_RESISTANCE_TEXT);
	file.syncString(COLD_RESISTANCE_TEXT);
	file.syncString(POISON_RESISTANCE_TEXT);
	file.syncString(CLAIRVOYANCE_TEXT);
	file.syncString(LEVITATE_TEXT);
	file.syncString(WALK_ON_WATER_TEXT);
	file.syncString(GAME_INFORMATION);
	file.syncString(WORLD_GAME_TEXT);
	file.syncString(DARKSIDE_GAME_TEXT);
	file.syncString(CLOUDS_GAME_TEXT);
	file.syncString(SWORDS_GAME_TEXT);
	file.syncStrings(WEEK_DAY_STRINGS, 10);
	file.syncString(CHARACTER_DETAILS);
	file.syncStrings(DAYS, 3);
	file.syncString(PARTY_GOLD);
	file.syncString(PLUS_14);
	file.syncString(CHARACTER_TEMPLATE);
	file.syncString(EXCHANGING_IN_COMBAT);
	file.syncString(CURRENT_MAXIMUM_RATING_TEXT);
	file.syncString(CURRENT_MAXIMUM_TEXT);
	file.syncStrings(RATING_TEXT, 24);
	file.syncStrings(BORN, 2);
	file.syncString(AGE_TEXT);
	file.syncString(LEVEL_TEXT);
	file.syncString(RESISTANCES_TEXT);
	file.syncString(NONE);
	file.syncString(EXPERIENCE_TEXT);
	file.syncString(ELIGIBLE);
	file.syncString(IN_PARTY_IN_BANK);
	file.syncStrings(FOOD_ON_HAND, 3);
	file.syncString(FOOD_TEXT);
	file.syncString(EXCHANGE_WITH_WHOM);
	file.syncString(QUICK_REF_LINE);
	file.syncString(QUICK_REFERENCE);
	file.syncNumbers2D((int *)BLACKSMITH_MAP_IDS, 2, 4);
	file.syncString(ITEMS_DIALOG_TEXT1);
	file.syncString(ITEMS_DIALOG_TEXT2);
	file.syncString(ITEMS_DIALOG_LINE1);
	file.syncString(ITEMS_DIALOG_LINE2);
	file.syncString(BTN_BUY);
	file.syncString(BTN_SELL);
	file.syncString(BTN_IDENTIFY);
	file.syncString(BTN_FIX);
	file.syncString(BTN_USE);
	file.syncString(BTN_EQUIP);
	file.syncString(BTN_REMOVE);
	file.syncString(BTN_DISCARD);
	file.syncString(BTN_QUEST);
	file.syncString(BTN_ENCHANT);
	file.syncString(BTN_RECHARGE);
	file.syncString(BTN_GOLD);
	file.syncString(ITEM_BROKEN);
	file.syncString(ITEM_CURSED);
	file.syncString(ITEM_OF);
	file.syncStrings(BONUS_NAMES, 7);
	file.syncStrings(WEAPON_NAMES, 41);
	file.syncStrings(ARMOR_NAMES, 14);
	file.syncStrings(ACCESSORY_NAMES, 11);
	file.syncStrings(MISC_NAMES, 22);
	file.syncStrings(SPECIAL_NAMES, 74);
	file.syncStrings(ELEMENTAL_NAMES, 6);
	file.syncStrings(ATTRIBUTE_NAMES, 10);
	file.syncStrings(EFFECTIVENESS_NAMES, 7);
	file.syncStrings(QUEST_ITEM_NAMES, 85);
	file.syncStrings(QUEST_ITEM_NAMES_SWORDS, 51);
	file.syncNumbers((int *)WEAPON_BASE_COSTS, 35);
	file.syncNumbers((int *)ARMOR_BASE_COSTS, 14);
	file.syncNumbers((int *)ACCESSORY_BASE_COSTS, 11);
	file.syncNumbers((int *)MISC_MATERIAL_COSTS, 22);
	file.syncNumbers((int *)MISC_BASE_COSTS, 76);
	file.syncNumbers((int *)METAL_BASE_MULTIPLIERS, 22);
	file.syncNumbers((int *)ITEM_SKILL_DIVISORS, 4);
	file.syncNumbers((int *)RESTRICTION_OFFSETS, 4);
	file.syncNumbers((int *)ITEM_RESTRICTIONS, 86);
	file.syncString(NOT_PROFICIENT);
	file.syncString(NO_ITEMS_AVAILABLE);
	file.syncStrings(CATEGORY_NAMES, 4);
	file.syncString(X_FOR_THE_Y);
	file.syncString(X_FOR_Y);
	file.syncString(X_FOR_Y_GOLD);
	file.syncString(FMT_CHARGES);
	file.syncString(AVAILABLE_GOLD_COST);
	file.syncString(CHARGES);
	file.syncString(COST);
	file.syncStrings(ITEM_ACTIONS, 7);
	file.syncString(WHICH_ITEM);
	file.syncString(WHATS_YOUR_HURRY);
	file.syncString(USE_ITEM_IN_COMBAT);
	file.syncString(NO_SPECIAL_ABILITIES);
	file.syncString(CANT_CAST_WHILE_ENGAGED);
	file.syncString(EQUIPPED_ALL_YOU_CAN);
	file.syncString(REMOVE_X_TO_EQUIP_Y);
	file.syncString(RING);
	file.syncString(MEDAL);
	file.syncString(CANNOT_REMOVE_CURSED_ITEM);
	file.syncString(CANNOT_DISCARD_CURSED_ITEM);
	file.syncString(PERMANENTLY_DISCARD);
	file.syncString(BACKPACK_IS_FULL);
	file.syncStrings(CATEGORY_BACKPACK_IS_FULL, 4);
	file.syncString(BUY_X_FOR_Y_GOLD);
	file.syncString(SELL_X_FOR_Y_GOLD);
	file.syncStrings(GOLDS, 2);
	file.syncString(NO_NEED_OF_THIS);
	file.syncString(NOT_RECHARGABLE);
	file.syncString(SPELL_FAILED);
	file.syncString(NOT_ENCHANTABLE);
	file.syncString(ITEM_NOT_BROKEN);
	file.syncStrings(FIX_IDENTIFY, 2);
	file.syncString(FIX_IDENTIFY_GOLD);
	file.syncString(IDENTIFY_ITEM_MSG);
	file.syncString(ITEM_DETAILS);
	file.syncString(ALL);
	file.syncString(FIELD_NONE);
	file.syncString(DAMAGE_X_TO_Y);
	file.syncString(ELEMENTAL_XY_DAMAGE);
	file.syncString(ATTR_XY_BONUS);
	file.syncString(EFFECTIVE_AGAINST);
	file.syncString(QUESTS_DIALOG_TEXT);
	file.syncString(CLOUDS_OF_XEEN_LINE);
	file.syncString(DARKSIDE_OF_XEEN_LINE);
	file.syncString(SWORDS_OF_XEEN_LINE);
	file.syncString(NO_QUEST_ITEMS);
	file.syncString(NO_CURRENT_QUESTS);
	file.syncString(NO_AUTO_NOTES);
	file.syncString(QUEST_ITEMS_DATA);
	file.syncString(CURRENT_QUESTS_DATA);
	file.syncString(AUTO_NOTES_DATA);
	file.syncString(REST_COMPLETE);
	file.syncString(PARTY_IS_STARVING);
	file.syncString(HIT_SPELL_POINTS_RESTORED);
	file.syncString(TOO_DANGEROUS_TO_REST);
	file.syncString(SOME_CHARS_MAY_DIE);
	file.syncString(DISMISS_WHOM);
	file.syncString(CANT_DISMISS_LAST_CHAR);
	file.syncString(DELETE_CHAR_WITH_ELDER_WEAPON);
	file.syncStrings(REMOVE_DELETE, 2);
	file.syncString(REMOVE_OR_DELETE_WHICH);
	file.syncString(YOUR_PARTY_IS_FULL);
	file.syncString(HAS_SLAYER_SWORD);
	file.syncString(SURE_TO_DELETE_CHAR);
	file.syncString(CREATE_CHAR_DETAILS);
	file.syncString(NEW_CHAR_STATS);
	file.syncString(NAME_FOR_NEW_CHARACTER);
	file.syncString(SELECT_CLASS_BEFORE_SAVING);
	file.syncString(EXCHANGE_ATTR_WITH);
	file.syncNumbers((int *)NEW_CHAR_SKILLS, 10);
	file.syncNumbers((int *)NEW_CHAR_SKILLS_OFFSET, 10);
	file.syncNumbers((int *)NEW_CHAR_SKILLS_LEN, 10);
	file.syncNumbers((int *)NEW_CHAR_RACE_SKILLS, 10);
	file.syncNumbers((int *)RACE_MAGIC_RESISTANCES, 5);
	file.syncNumbers((int *)RACE_FIRE_RESISTANCES, 5);
	file.syncNumbers((int *)RACE_ELECTRIC_RESISTANCES, 5);
	file.syncNumbers((int *)RACE_COLD_RESISTANCES, 5);
	file.syncNumbers((int *)RACE_ENERGY_RESISTANCES, 5);
	file.syncNumbers((int *)RACE_POISON_RESISTANCES, 5);
	file.syncNumbers2D((int *)NEW_CHARACTER_SPELLS, 10, 4);
	file.syncString(COMBAT_DETAILS);
	file.syncString(NOT_ENOUGH_TO_CAST);
	file.syncStrings(SPELL_CAST_COMPONENTS, 2);
	file.syncString(CAST_SPELL_DETAILS);
	file.syncString(PARTY_FOUND);
	file.syncString(BACKPACKS_FULL_PRESS_KEY);
	file.syncString(HIT_A_KEY);
	file.syncString(GIVE_TREASURE_FORMATTING);
	file.syncStrings(FOUND, 2);
	file.syncString(X_FOUND_Y);
	file.syncString(ON_WHO);
	file.syncString(WHICH_ELEMENT1);
	file.syncString(WHICH_ELEMENT2);
	file.syncString(DETECT_MONSTERS);
	file.syncString(LLOYDS_BEACON);
	file.syncString(HOW_MANY_SQUARES);
	file.syncString(TOWN_PORTAL);
	file.syncString(TOWN_PORTAL_SWORDS);
	file.syncNumbers2D((int *)TOWN_MAP_NUMBERS, 3, 5);
	file.syncString(MONSTER_DETAILS);
	file.syncStrings(MONSTER_SPECIAL_ATTACKS, 23);
	file.syncString(IDENTIFY_MONSTERS);
	file.syncStrings(EVENT_SAMPLES, 6);
	file.syncString(MOONS_NOT_ALIGNED);
	file.syncString(AWARDS_FOR);
	file.syncString(AWARDS_TEXT);
	file.syncString(NO_AWARDS);
	file.syncString(WARZONE_BATTLE_MASTER);
	file.syncString(WARZONE_MAXED);
	file.syncString(WARZONE_LEVEL);
	file.syncString(WARZONE_HOW_MANY);
	file.syncString(PICKS_THE_LOCK);
	file.syncStrings(PICK_FORM, 2);
	file.syncString(UNABLE_TO_PICK_LOCK);
	file.syncStrings(UNABLE_TO_PICK_FORM, 2);
	file.syncString(CONTROL_PANEL_TEXT);
	file.syncString(CONTROL_PANEL_BUTTONS);
	file.syncString(ON);
	file.syncString(OFF);
	file.syncString(CONFIRM_QUIT);
	file.syncString(MR_WIZARD);
	file.syncString(NO_LOADING_IN_COMBAT);
	file.syncString(NO_SAVING_IN_COMBAT);
	file.syncString(QUICK_FIGHT_TEXT);
	file.syncStrings(QUICK_FIGHT_OPTIONS, 4);
	file.syncStrings(WORLD_END_TEXT, 9);
	file.syncString(WORLD_CONGRATULATIONS);
	file.syncString(WORLD_CONGRATULATIONS2);
	file.syncString(CLOUDS_CONGRATULATIONS1);
	file.syncString(CLOUDS_CONGRATULATIONS2);
	file.syncStrings(GOOBER, 3);
	file.syncStrings(MUSIC_FILES1, 5);
	file.syncStrings2D(&MUSIC_FILES2[0][0], 6, 7);
	file.syncString(DIFFICULTY_TEXT);
	file.syncString(SAVE_OFF_LIMITS);
	file.syncString(CLOUDS_INTRO1);
	file.syncString(DARKSIDE_ENDING1);
	file.syncString(DARKSIDE_ENDING2);
	file.syncString(PHAROAH_ENDING_TEXT1);
	file.syncString(PHAROAH_ENDING_TEXT2);
	file.syncStrings(CLOUDS_MAE_NAMES, 131);
	file.syncStrings(CLOUDS_MIRROR_LOCATIONS, 59);
	file.syncStrings(CLOUDS_MAP_NAMES, 86);
	file.syncStrings(CLOUDS_MONSTERS, 91);
	file.syncStrings(CLOUDS_SPELLS, 77);

	ResFile keys("CONSTKEYS_", _buffer, langId);
	keys.syncNumber(KeyConstants.DialogsCharInfo.KEY_ITEM);
	keys.syncNumber(KeyConstants.DialogsCharInfo.KEY_QUICK);
	keys.syncNumber(KeyConstants.DialogsCharInfo.KEY_EXCHANGE);

	keys.syncNumber(KeyConstants.DialogsControlPanel.KEY_FXON);
	keys.syncNumber(KeyConstants.DialogsControlPanel.KEY_MUSICON);
	keys.syncNumber(KeyConstants.DialogsControlPanel.KEY_LOAD);
	keys.syncNumber(KeyConstants.DialogsControlPanel.KEY_SAVE);
	keys.syncNumber(KeyConstants.DialogsControlPanel.KEY_QUIT);
	keys.syncNumber(KeyConstants.DialogsControlPanel.KEY_MRWIZARD);

	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_ROLL);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_CREATE);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_MGT);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_INT);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_PER);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_END);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_SPD);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_ACY);
	keys.syncNumber(KeyConstants.DialogsCreateChar.KEY_LCK);


	keys.syncNumber(KeyConstants.DialogsDifficulty.KEY_ADVENTURER);
	keys.syncNumber(KeyConstants.DialogsDifficulty.KEY_WARRIOR);

	keys.syncNumber(KeyConstants.DialogsItems.KEY_WEAPONS);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_ARMOR);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_ACCESSORY);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_MISC);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_ENCHANT);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_USE);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_BUY);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_SELL);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_IDENTIFY);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_FIX);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_EQUIP);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_REM);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_DISC);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_QUEST);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_RECHRG);
	keys.syncNumber(KeyConstants.DialogsItems.KEY_GOLD);

	keys.syncNumber(KeyConstants.DialogsParty.KEY_DELETE);
	keys.syncNumber(KeyConstants.DialogsParty.KEY_REMOVE);
	keys.syncNumber(KeyConstants.DialogsParty.KEY_CREATE);
	keys.syncNumber(KeyConstants.DialogsParty.KEY_EXIT);

	keys.syncNumber(KeyConstants.DialogsQuests.KEY_QUEST_ITEMS);
	keys.syncNumber(KeyConstants.DialogsQuests.KEY_CURRENT_QUESTS);
	keys.syncNumber(KeyConstants.DialogsQuests.KEY_AUTO_NOTES);

	keys.syncNumber(KeyConstants.DialogsQuickFight.KEY_NEXT);

	keys.syncNumber(KeyConstants.DialogsSpells.KEY_CAST);
	keys.syncNumber(KeyConstants.DialogsSpells.KEY_NEW);
	keys.syncNumber(KeyConstants.DialogsSpells.KEY_FIRE);
	keys.syncNumber(KeyConstants.DialogsSpells.KEY_ELEC);
	keys.syncNumber(KeyConstants.DialogsSpells.KEY_COLD);
	keys.syncNumber(KeyConstants.DialogsSpells.KEY_ACID);
	keys.syncNumber(KeyConstants.DialogsSpells.KEY_SET);
	keys.syncNumber(KeyConstants.DialogsSpells.KEY_RETURN);

	keys.syncNumber(KeyConstants.Locations.KEY_DEP);
	keys.syncNumber(KeyConstants.Locations.KEY_WITH);
	keys.syncNumber(KeyConstants.Locations.KEY_GOLD);
	keys.syncNumber(KeyConstants.Locations.KEY_GEMS);
	keys.syncNumber(KeyConstants.Locations.KEY_BROWSE);
	keys.syncNumber(KeyConstants.Locations.KEY_BUY_SPELLS);
	keys.syncNumber(KeyConstants.Locations.KEY_SPELL_INFO);
	keys.syncNumber(KeyConstants.Locations.KEY_SIGN_IN);
	keys.syncNumber(KeyConstants.Locations.KEY_DRINK);
	keys.syncNumber(KeyConstants.Locations.KEY_FOOD);
	keys.syncNumber(KeyConstants.Locations.KEY_TIP);
	keys.syncNumber(KeyConstants.Locations.KEY_RUMORS);
	keys.syncNumber(KeyConstants.Locations.KEY_HEAL);
	keys.syncNumber(KeyConstants.Locations.KEY_DONATION);
	keys.syncNumber(KeyConstants.Locations.KEY_UNCURSE);
	keys.syncNumber(KeyConstants.Locations.KEY_TRAIN);

	keys.syncNumber(KeyConstants.CloudsOfXeenMenu.KEY_START_NEW_GAME);
	keys.syncNumber(KeyConstants.CloudsOfXeenMenu.KEY_LOAD_GAME);
	keys.syncNumber(KeyConstants.CloudsOfXeenMenu.KEY_SHOW_CREDITS);
	keys.syncNumber(KeyConstants.CloudsOfXeenMenu.KEY_VIEW_ENDGAME);
}

} // End of namespace Xeen
} // End of namespace MM
