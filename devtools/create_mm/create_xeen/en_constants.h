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

#pragma once

#include "constants.h"
#include "common/keyboard.h"

class EN : public LangConstants {
public:
	virtual ~EN() {}
	const char *CLOUDS_CREDITS() {
		return "\v012\t000\x3"
			   "c\f35Designed and Directed By:\n"
			   "\f17Jon Van Caneghem\x3l\n"
			   "\n"
			   "\t025\f35Programming:\n"
			   "\t035\f17Mark Caldwell\n"
			   "\t035Dave Hathaway\n"
			   "\n"
			   "\t025\f35Sound System & FX:\n"
			   "\t035\f17Todd Hendrix\n"
			   "\n"
			   "\t025\f35Music & Speech:\n"
			   "\t035\f17Tim Tully\n"
			   "\n"
			   "\t025\f35Writing:\n"
			   "\t035\f17Paul Rattner\n"
			   "\t035Debbie Murphy\n"
			   "\t035Jon Van Caneghem\v012\n"
			   "\n"
			   "\n"
			   "\t180\f35Graphics:\n"
			   "\t190\f17Louis Johnson\n"
			   "\t190Jonathan P. Gwyn\n"
			   "\t190Bonita Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n"
			   "\n"
			   "\t180\f35Testing:\n"
			   "\t190\f17Benjamin Bent\n"
			   "\t190Mario Escamilla\n"
			   "\t190Richard Espy\n"
			   "\t190Scott McDaniel\n"
			   "\t190Clayton Retzer\n"
			   "\t190Michael Suarez\x3"
			   "c";
	}

	const char *DARK_SIDE_CREDITS() {
		return "\v012\t000\x3"
			   "c\f35Designed and Directed By:\n"
			   "\f17Jon Van Caneghem\x3l\n"
			   "\n"
			   "\t025\f35Programming:\n"
			   "\t035\f17Mark Caldwell\n"
			   "\t035Dave Hathaway\n"
			   "\n"
			   "\t025\f35Sound System & FX:\n"
			   "\t035\f17Mike Heilemann\n"
			   "\n"
			   "\t025\f35Music & Speech:\n"
			   "\t035\f17Tim Tully\n"
			   "\n"
			   "\t025\f35Writing:\n"
			   "\t035\f17Paul Rattner\n"
			   "\t035Debbie Van Caneghem\n"
			   "\t035Jon Van Caneghem\v012\n"
			   "\n"
			   "\n"
			   "\t180\f35Graphics:\n"
			   "\t190\f17Jonathan P. Gwyn\n"
			   "\t190Bonita Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n"
			   "\n"
			   "\t180\f35Testing:\n"
			   "\t190\f17Benjamin Bent\n"
			   "\t190Christian Dailey\n"
			   "\t190Mario Escamilla\n"
			   "\t190Marco Hunter\n"
			   "\t190Robert J. Lupo\n"
			   "\t190Clayton Retzer\n"
			   "\t190David Vela\x3"
			   "c";
	}

	const char *SWORDS_CREDITS1() {
		return "\v012\x3"
			   "c\f35Published By New World Computing, Inc.\f17\n"
			   "Developed By CATware, Inc.\x3l\n"
			   "\f01Design and Direction\t180Series Created by\n"
			   "\t020Bill Fawcett\t190John Van Caneghem\n"
			   "\n"
			   "\t010Story Contributions\t180Producer & Manual\n"
			   "\t020Ellen Guon\t190Dean Rettig\n"
			   "\n"
			   "\t010Programming & Ideas\t180Original Programming\n"
			   "\t020David Potter\t190Mark Caldwell\n"
			   "\t020Rod Retterath\t190Dave Hathaway\n"
			   "\n"
			   "\t010Manual Illustrations\t180Graphic Artists\n"
			   "\t020Todd Cameron Hamilton\t190Jonathan P. Gwyn\n"
			   "\t020James Clouse\t190Bonnie Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n";
	}

	const char *SWORDS_CREDITS2() {
		return "\f05\v012\t000\x3l\n"
			   "\t100Sound Programming\n"
			   "\t110Todd Hendrix\n"
			   "\n"
			   "\t100Music\n"
			   "\t110Tim Tully\n"
			   "\t110Quality Assurance Manager\n"
			   "\t110Peter Ryu\n"
			   "\t100Testers\n"
			   "\t110Walter Johnson\n"
			   "\t110Bryan Farina\n"
			   "\t110David Baton\n"
			   "\t110Jack Nalls\n";
	}

	const char *OPTIONS_MENU() {
		return "\r\x1\x3"
			   "c\fdMight and Magic Options\n"
			   "%s of Xeen\x2\n"
			   "\v%.3dCopyright (c) %d NWC, Inc.\n"
			   "All Rights Reserved\x1";
	}

	const char **GAME_NAMES() {
		static const char *_gameNames[] = { "Clouds", "Darkside", "World" };
		return _gameNames;
	}

	const char *THE_PARTY_NEEDS_REST() {
		return "\v012The Party needs rest!";
	}

	const char *WHO_WILL() {
		return "\x3"
			   "c\v000\t000%s\n\n"
			   "Who will\n%s?\n\v055F1 - F%d";
	}

	const char *HOW_MUCH() {
		return "\x3"
			   "cHow Much\n\n";
	}

	const char *WHATS_THE_PASSWORD() {
		return "\x3"
			   "cWhat's the Password?\n"
			   "\n"
			   "Please turn to page %u, go to\n"
			   "line %u, and type in word %u.\v067\t000Spaces are not counted as words or lines.  "
			   "Hyphenated words are treated as one word.  Any line that has any text is considered a line."
			   "\x3"
			   "c\v040\t000\n";
	}

	const char *PASSWORD_INCORRECT() {
		return "\x3"
			   "c\v040\n"
			   "\f32Incorrect!\fd";
	}

	const char *IN_NO_CONDITION() {
		return "\v007%s is not in any condition to perform actions!";
	}

	const char *NOTHING_HERE() {
		return "\x3"
			   "c\v010Nothing here.";
	}

	const char **WHO_ACTIONS() {
		static const char *_whoActions[] = {
			"search", "open", "drink", "mine", "touch", "read", "learn", "take",
				"bang", "steal", "bribe", "pay", "sit", "try", "turn", "bathe",
				"destroy", "pull", "descend", "toss a coin", "pray", "join", "act",
				"play", "push", "rub", "pick", "eat", "sign", "close", "look", "try"
		};
		return _whoActions;
	}

	const char **WHO_WILL_ACTIONS() {
		static const char *_whoWillActions[] = {
			"Open Grate", "Open Door", "Open Scroll", "Select Char"
		};
		return _whoWillActions;
	}

	const char **DIRECTION_TEXT_UPPER() {
		static const char *_directionTextUpper[] = { "NORTH", "EAST", "SOUTH", "WEST" };
		return _directionTextUpper;
	}

	const char **DIRECTION_TEXT() {
		static const char *_directionText[] = { "North", "East", "South", "West" };
		return _directionText;
	}

	const char **RACE_NAMES() {
		static const char *_raceNames[] = { "Human", "Elf", "Dwarf", "Gnome", "H-Orc" };
		return _raceNames;
	}

	const char **ALIGNMENT_NAMES() {
		static const char *_alignmentNames[] = { "Good", "Neutral", "Evil" };
		return _alignmentNames;
	}

	const char **SEX_NAMES() {
		static const char *_sexNames[] = { "Male", "Female" };
		return _sexNames;
	}

	const char **SKILL_NAMES() {
		static const char *_skillNames[] = {
			"Thievery\t100", "Arms Master", "Astrologer", "Body Builder", "Cartographer",
				"Crusader", "Direction Sense", "Linguist", "Merchant", "Mountaineer",
				"Navigator", "Path Finder", "Prayer Master", "Prestidigitator",
				"Swimmer", "Tracker", "Spot Secret Door", "Danger Sense"
		};
		return _skillNames;
	}

	const char **CLASS_NAMES() {
		static const char *_classNames[] = {
			"Knight", "Paladin", "Archer", "Cleric", "Sorcerer", "Robber",
				"Ninja", "Barbarian", "Druid", "Ranger", nullptr
		};
		return _classNames;
	}

	const char **CONDITION_NAMES_M() {
		static const char *_conditionNamesM[] = {
			"Cursed", "Heart Broken", "Weak", "Poisoned", "Diseased",
				"Insane", "In Love", "Drunk", "Asleep", "Depressed", "Confused",
				"Paralyzed", "Unconscious", "Dead", "Stone", "Eradicated", "Good"
		};
		return _conditionNamesM;
	}

	const char **CONDITION_NAMES_F() {
		static const char *_conditionNamesF[] = {
			"Cursed", "Heart Broken", "Weak", "Poisoned", "Diseased",
				"Insane", "In Love", "Drunk", "Asleep", "Depressed", "Confused",
				"Paralyzed", "Unconscious", "Dead", "Stone", "Eradicated", "Good"
		};
		return _conditionNamesF;
	}

	const char *GOOD() {
		return "Good";
	}

	const char *BLESSED() {
		return "\n\t020Blessed\t095%+d";
	}

	const char *POWER_SHIELD() {
		return "\n\t020Power Shield\t095%+d";
	}

	const char *HOLY_BONUS() {
		return "\n\t020Holy Bonus\t095%+d";
	}

	const char *HEROISM() {
		return "\n\t020Heroism\t095%+d";
	}

	const char *IN_PARTY() {
		return "\014"
			   "15In Party\014"
			   "d";
	}

	const char *PARTY_DETAILS() {
		return "\015\003l\002\014"
			   "00"
			   "\013"
			   "001"
			   "\011"
			   "035%s"
			   "\013"
			   "009"
			   "\011"
			   "035%s"
			   "\013"
			   "017"
			   "\011"
			   "035%s"
			   "\013"
			   "025"
			   "\011"
			   "035%s"
			   "\013"
			   "001"
			   "\011"
			   "136%s"
			   "\013"
			   "009"
			   "\011"
			   "136%s"
			   "\013"
			   "017"
			   "\011"
			   "136%s"
			   "\013"
			   "025"
			   "\011"
			   "136%s"
			   "\013"
			   "044"
			   "\011"
			   "035%s"
			   "\013"
			   "052"
			   "\011"
			   "035%s"
			   "\013"
			   "060"
			   "\011"
			   "035%s"
			   "\013"
			   "068"
			   "\011"
			   "035%s"
			   "\013"
			   "044"
			   "\011"
			   "136%s"
			   "\013"
			   "052"
			   "\011"
			   "136%s"
			   "\013"
			   "060"
			   "\011"
			   "136%s"
			   "\013"
			   "068"
			   "\011"
			   "136%s";
	}

	const char *PARTY_DIALOG_TEXT() {
		return "%s\x2\x3"
			   "c\v106\t013Up\t048Down\t083\f37D\fdel\t118\f37R\fdem"
			   "\t153\f37C\fdreate\t188E\f37x\fdit\x1";
	}

	const char *NO_ONE_TO_ADVENTURE_WITH() {
		return "You have no one to adventure with";
	}

	const char *YOUR_ROSTER_IS_FULL() {
		return "Your Roster is full!";
	}

	const char *PLEASE_WAIT() {
		return "\014"
			   "d\003"
			   "c\011"
			   "000"
			   "\013"
			   "002Please Wait...";
	}

	const char *OOPS() {
		return "\003"
			   "c\011"
			   "000\013"
			   "002Oops...";
	}

	const char *BANK_TEXT() {
		return "\r\x2\x3"
			   "c\v122\t013"
			   "\f37D\fdep\t040\f37W\fdith\t067ESC"
			   "\x1\t000\v000Bank of Xeen\v015\n"
			   "Bank\x3l\n"
			   "Gold\x3r\t000%s\x3l\n"
			   "Gems\x3r\t000%s\x3"
			   "c\n"
			   "\n"
			   "Party\x3l\n"
			   "Gold\x3r\t000%s\x3l\n"
			   "Gems\x3r\t000%s";
	}

	const char *BLACKSMITH_TEXT() {
		return "\x1\r\x3"
			   "c\v000\t000"
			   "store options for\t039\v027%s\x3l\v046\n"
			   "\t011\f37b\fdrowse\n"
			   "\t000\v090gold\x3r\t000%s"
			   "\x2\x3"
			   "c\v122\t040esc\x1";
	}

	const char *GUILD_NOT_MEMBER_TEXT() {
		return "\n\nYou have to be a member to shop here.";
	}

	const char *GUILD_TEXT() {
		return "\x3"
			   "c\v027\t039%s"
			   "\x3l\v046\n"
			   "\t012\f37B\fduy Spells\n"
			   "\t012\f37S\fdpell Info";
	}

	const char *TAVERN_TEXT() {
		return "\r\x3"
			   "c\v000\t000Tavern Options for\t039"
			   "\v027%s%s\x3l\t000"
			   "\v090Gold\x3r\t000%s\x2\x3"
			   "c\v122"
			   "\t021\f37S\fdign in\t060ESC\x1";
	}

	const char *FOOD_AND_DRINK() {
		return "\x3l\t017\v046\f37D\fdrink\n"
			   "\t017\f37F\fdood\n"
			   "\t017\f37T\fdip\n"
			   "\t017\f37R\fdumors";
	}

	const char *GOOD_STUFF() {
		return "\n"
			   "\n"
			   "Good Stuff\n"
			   "\n"
			   "Hit a key!";
	}

	const char *HAVE_A_DRINK() {
		return "\n\nHave a Drink\n\nHit a key!";
	}

	const char *YOURE_DRUNK() {
		return "\n\nYou're Drunk\n\nHit a key!";
	}

	const char *TEMPLE_TEXT() {
		return "\r\x3"
			   "c\v000\t000Temple Options for"
			   "\t039\v027%s\x3l\t000\v046"
			   "\f37H\fdeal\x3r\t000%u\x3l\n"
			   "\f37D\fdonation\x3r\t000%u\x3l\n"
			   "\f37U\fdnCurse\x3r\t000%s"
			   "\x3l\t000\v090Gold\x3r\t000%s"
			   "\x2\x3"
			   "c\v122\t040ESC\x1";
	}

	const char *EXPERIENCE_FOR_LEVEL() {
		return "%s needs %u experience for level %u.";
	}

	const char *TRAINING_LEARNED_ALL() {
		return "%s has learned all we can teach!";
	}

	const char *ELIGIBLE_FOR_LEVEL() {
		return "%s is eligible for level %u.\x3l\n"
			   "\v081Cost\x3r\t000%u";
	}

	const char *TRAINING_TEXT() {
		return "\r\x3"
			   "cTraining Options\n"
			   "\n"
			   "%s\x3l\v090\t000Gold\x3r\t000%s\x2\x3"
			   "c\v122\t021"
			   "\f37T\fdrain\t060ESC\x1";
	}

	const char *GOLD_GEMS() {
		return "\x3"
			   "c\v000\t000%s\x3l\n"
			   "\n"
			   "Gold\x3r\t000%s\x3l\n"
			   "Gems\x3r\t000%s\x2\x3"
			   "c\v096\t013G\f37o\fdld\t040G\f37e\fdms\t067ESC\x1";
	}

	const char *GOLD_GEMS_2() {
		return "\x3"
			   "c\v000\t000%s\x3l\n"
			   "\n"
			   "\x4"
			   "077Gold\x3r\t000%s\x3l\n"
			   "\x4"
			   "077Gems\x3r\t000%s\x3l\t000\v051\x4"
			   "077\n"
			   "\x4"
			   "077";
	}

	const char **DEPOSIT_WITHDRAWL() {
		static const char *_depositWithdrawl[] = { "Deposit", "Withdrawl" };
		return _depositWithdrawl;
	}

	const char *NOT_ENOUGH_X_IN_THE_Y() {
		return "\x3"
			   "c\v012Not enough %s in the %s!\x3l";
	}

	const char *NO_X_IN_THE_Y() {
		return "\x3"
			   "c\v012No %s in the %s!\x3l";
	}

	const char **STAT_NAMES() {
		static const char *_statNames[] = {
			"Might", "Intellect", "Personality", "Endurance", "Speed",
				"Accuracy", "Luck", "Age", "Level", "Armor Class", "Hit Points",
				"Spell Points", "Resistances", "Skills", "Awards", "Experience"
		};
		return _statNames;
	}

	const char **CONSUMABLE_NAMES() {
		static const char *_consumableNames[] = { "Gold", "Gems", "Food", "Condition" };
		return _consumableNames;
	}

	const char **CONSUMABLE_GOLD_FORMS() {
		static const char *_consumableGoldForms[] = { "" };
		return _consumableGoldForms;
	}

	const char **CONSUMABLE_GEM_FORMS() {
		static const char *_consumableGemForms[] = { "" };
		return _consumableGemForms;
	}

	const char **WHERE_NAMES() {
		static const char *_whereNames[] = { "Party", "Bank" };
		return _whereNames;
	}

	const char *AMOUNT() {
		return "\x3"
			   "c\t000\v051Amount\x3l\n";
	}

	const char *FOOD_PACKS_FULL() {
		return "\v007Your food packs are already full!";
	}

	const char *BUY_SPELLS() {
		return "\x3"
			   "c\v027\t039%s\x3l\v046\n"
			   "\t012\f37B\fduy Spells\n"
			   "\t012\f37S\fdpell Info";
	}

	const char *GUILD_OPTIONS() {
		return "\r\f00\x3"
			   "c\v000\t000Guild Options for%s"
			   "\x3l\t000\v090Gold"
			   "\x3r\t000%s\x2\x3"
			   "c\v122\t040ESC\x1";
	}

	const char *NOT_A_SPELL_CASTER() {
		return "Not a spell caster...";
	}

	const char *SPELLS_LEARNED_ALL() {
		return "You have learned all we\n"
			   "\t010can teach you.";
	}

	const char *SPELLS_FOR() {
		return "\r\fd%s\x2\x3"
			   "c\t000\v002Spells for %s";
	}

	const char *SPELL_LINES_0_TO_9() {
		return "\x2\x3l\v015\t0011\n2\n3\n4\n5\n6\n7\n8\n9\n0";
	}

	const char *SPELLS_DIALOG_SPELLS() {
		return "\x3l\v015"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l"
			   "\t004\v110%s - %u\x1";
	}

	const char *SPELL_PTS() {
		return "Spell Pts";
	}

	const char *GOLD() {
		return "Gold";
	}

	const char *SPELL_INFO() {
		return "\x3"
			   "c\f09%s\fd\x3l\n"
			   "\n"
			   "%s\x3"
			   "c\t000\v100Press a Key!";
	}

	const char *SPELL_PURCHASE() {
		return "\x3l\v000\t000\fd%s  Do you wish to purchase "
			   "\f09%s\fd for %u?";
	}

	const char *MAP_TEXT() {
		return "\x3"
			   "c\v000\t000%s\x3l\v139"
			   "\t000X = %d\x3r\t000Y = %d\x3"
			   "c\t000%s";
	}

	const char *LIGHT_COUNT_TEXT() {
		return "\x3l\n\n\t024Light\x3r\t124%d";
	}

	const char *FIRE_RESISTENCE_TEXT() {
		return "%c%sFire%s%u";
	}

	const char *ELECRICITY_RESISTENCE_TEXT() {
		return "%c%sElectricity%s%u";
	}

	const char *COLD_RESISTENCE_TEXT() {
		return "%c%sCold%s%u";
	}

	const char *POISON_RESISTENCE_TEXT() {
		return "%c%sPoison/Acid%s%u";
	}

	const char *CLAIRVOYANCE_TEXT() {
		return "%c%sClairvoyance%s";
	}

	const char *LEVITATE_TEXT() {
		return "%c%sLevitate%s";
	}

	const char *WALK_ON_WATER_TEXT() {
		return "%c%sWalk on Water";
	}

	const char *GAME_INFORMATION() {
		return "\r\x3"
			   "c\t000\v001\f37%s of Xeen\fd\n"
			   "Game Information\n"
			   "\n"
			   "Today is \f37%ssday\fd\n"
			   "\n"
			   "\t032Time\t072Day\t112Year\n"
			   "\t032\f37%d:%02d%c\t072%u\t112%u\fd%s";
	}

	const char *WORLD_GAME_TEXT() {
		return "World";
	}

	const char *DARKSIDE_GAME_TEXT() {
		return "Darkside";
	}

	const char *CLOUDS_GAME_TEXT() {
		return "Clouds";
	}

	const char *SWORDS_GAME_TEXT() {
		return "Swords";
	}

	const char **WEEK_DAY_STRINGS() {
		static const char *_weekDayStrings[] = {
			"Ten", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"
		};
		return _weekDayStrings;
	}

	const char *CHARACTER_DETAILS() {
		return "\x3l\v041\t196%s\t000\v002%s : %s %s %s"
			   "\x3r\t053\v028\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f%02u%d\fd\t196\f15%u\fd\x3r"
			   "\t053\v051\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f%02u%u\fd\t196\f15%u\fd"
			   "\x3r\t053\v074\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f15%u\fd\t196\f15%u\fd"
			   "\x3r\t053\v097\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f15%u\fd\t196\f15%u day%s\fd"
			   "\x3r\t053\v120\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f15%u\fd\t196\f%02u%s\fd"
			   "\t230%s%s%s%s\fd";
	}

	const char **DAYS() {
		static const char *_days[] = { "", "s", "" };
		return _days;
	}

	const char *PARTY_GOLD() {
		return "Party Gold";
	}

	const char *PLUS_14() {
		return "\f14+";
	}

	const char *CHARACTER_TEMPLATE() {
		return "\x1\f00\r\x3l\t029\v018Mgt\t080Acy\t131H.P.\t196Experience"
			   "\t029\v041Int\t080Lck\t131S.P.\t029\v064Per\t080Age"
			   "\t131Resis\t196Party Gems\t029\v087End\t080Lvl\t131Skills"
			   "\t196Party Food\t029\v110Spd\t080AC\t131Awrds\t196Condition\x3"
			   "c"
			   "\t290\v025\f37I\fdtem\t290\v057\f37Q"
			   "\fduick\t290\v089\f37E\fdxch\t290\v121Exit\x3l%s";
	}

	const char *EXCHANGING_IN_COMBAT() {
		return "\x3"
			   "c\v007\t000Exchanging in combat is not allowed!";
	}

	const char *CURRENT_MAXIMUM_RATING_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Current / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u\n"
			   "\x3"
			   "cRating: %s";
	}

	const char *CURRENT_MAXIMUM_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Current / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u";
	}

	const char **RATING_TEXT() {
		static const char *_ratingText[] = {
			"Nonexistant", "Very Poor", "Poor", "Very Low", "Low", "Average", "Good",
				"Very Good", "High", "Very High", "Great", "Super", "Amazing", "Incredible",
				"Gigantic", "Fantastic", "Astoundig", "Astonishing", "Monumental", "Tremendous",
				"Collosal", "Awesome", "Awe Inspiring", "Ultimate"
		};
		return _ratingText;
	}

	const char **BORN() {
		static const char *_born[] = { "Born", "" };
		return _born;
	}

	const char *AGE_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Current / Natural\n"
			   "\x3r\t057%u\x3l\t061/ %u\n"
			   "\x3"
			   "c%s: %u / %u\x1";
	}

	const char *LEVEL_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Current / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u\n"
			   "\x3"
			   "c%u Attack%s/Round\x1";
	}

	const char *RESISTENCES_TEXT() {
		return "\x2\x3"
			   "c%s\x3l\n"
			   "\t020Fire\t100%u\n"
			   "\t020Cold\t100%u\n"
			   "\t020Electricity\t100%u\n"
			   "\t020Poison\t100%u\n"
			   "\t020Energy\t100%u\n"
			   "\t020Magic\t100%u";
	}

	const char *NONE() {
		return "\n\t020None";
	}

	const char *EXPERIENCE_TEXT() {
		return "\x2\x3"
			   "c%s\x3l\n"
			   "\t010Current:\t070%u\n"
			   "\t010Next Level:\t070%s\x1";
	}

	const char *ELIGIBLE() {
		return "\f12Eligible\fd";
	}

	const char *IN_PARTY_IN_BANK() {
		return "\x2\x3"
			   "cParty %s\n"
			   "%u on hand\n"
			   "%u in bank\x1\x3l";
	}

	const char **FOOD_ON_HAND() {
		static const char *_onHand[] = { "on hand", "", "" };
		return _onHand;
	}

	const char *FOOD_TEXT() {
		return "\x2\x3"
			   "cParty %s\n"
			   "%u %s\n"
			   "Enough for %u day%s\x3l";
	}

	const char *EXCHANGE_WITH_WHOM() {
		return "\t010\v005Exchange with whom?";
	}

	const char *QUICK_REF_LINE() {
		return "\v%3d\t007%u)\t027%s\t110%c%c%c\x3r\t160\f%02u%u\fd"
			   "\x3l\t170\f%02u%d\fd\t208\f%02u%u\fd\t247\f"
			   "%02u%u\fd\t270\f%02u%c%c%c%c\fd";
	}

	const char *QUICK_REFERENCE() {
		return "\r\x3"
			   "cQuick Reference Chart\v012\x3l"
			   "\t007#\t027Name\t110Cls\t140Lvl\t176H.P."
			   "\t212S.P.\t241A.C.\t270Cond"
			   "%s%s%s%s%s%s%s%s"
			   "\v110\t064\x3"
			   "cGold\t144Gems\t224Food\v119"
			   "\t064\f15%u\t144%u\t224%u day%s\fd";
	}

	const char *ITEMS_DIALOG_TEXT1() {
		return "\r\x2\x3"
			   "c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
			   "\f37c\fdces\t119\f37M\fdisc\t153%s\t187%s\t221%s"
			   "\t255%s\t289Exit";
	}

	const char *ITEMS_DIALOG_TEXT2() {
		return "\r\x2\x3"
			   "c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
			   "\f37c\fdces\t119\f37M\fdisc\t153\f37%s\t289Exit";
	}

	const char *ITEMS_DIALOG_LINE1() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\n";
	}

	const char *ITEMS_DIALOG_LINE2() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%u\n";
	}

	const char *BTN_BUY() {
		return "\f37B\fduy";
	}

	const char *BTN_SELL() {
		return "\f37S\fdell";
	}

	const char *BTN_IDENTIFY() {
		return "\f37I\fddentify";
	}

	const char *BTN_FIX() {
		return "\f37F\fdix";
	}

	const char *BTN_USE() {
		return "\f37U\fdse";
	}

	const char *BTN_EQUIP() {
		return "\f37E\fdquip";
	}

	const char *BTN_REMOVE() {
		return "\f37R\fdem";
	}

	const char *BTN_DISCARD() {
		return "\f37D\fdisc";
	}

	const char *BTN_QUEST() {
		return "\f37Q\fduest";
	}

	const char *BTN_ENCHANT() {
		return "E\fdnchant";
	}

	const char *BTN_RECHARGE() {
		return "R\fdechrg";
	}

	const char *BTN_GOLD() {
		return "G\fdold";
	}

	const char *ITEM_BROKEN() {
		return "\f32broken ";
	}

	const char *ITEM_CURSED() {
		return "\f09cursed ";
	}

	const char *ITEM_OF() {
		return "of ";
	}

	const char **BONUS_NAMES() {
		static const char *_bonusNames[] = {
			"", "Dragon Slayer", "Undead Eater", "Golem Smasher",
				"Bug Zapper", "Monster Masher", "Beast Bopper"
		};
		return _bonusNames;
	}

	const char **WEAPON_NAMES() {
		static const char *_weaponNames[] = {
			nullptr, "long sword ", "short sword ", "broad sword ", "scimitar ",
				"cutlass ", "sabre ", "club ", "hand axe ", "katana ", "nunchakas ",
				"wakazashi ", "dagger ", "mace ", "flail ", "cudgel ", "maul ", "spear ",
				"bardiche ", "glaive ", "halberd ", "pike ", "flamberge ", "trident ",
				"staff ", "hammer ", "naginata ", "battle axe ", "grand axe ", "great axe ",
				"short bow ", "long bow ", "crossbow ", "sling ", "Xeen Slayer Sword ",
				"Elder LongSword ", "Elder Dagger ", "Elder Mace ", "Elder Spear ",
				"Elder Staff ", "Elder LongBow "
		};
		return _weaponNames;
	}

	const char **ARMOR_NAMES() {
		static const char *_armorNames[] = {
			nullptr, "robes ", "scale armor ", "ring mail ", "chain mail ",
				"splint mail ", "plate mail ", "plate armor ", "shield ",
				"helm ", "boots ", "cloak ", "cape ", "gauntlets "
		};
		return _armorNames;
	}

	const char **ACCESSORY_NAMES() {
		static const char *_accessoryNames[] = {
			nullptr, "ring ", "belt ", "brooch ", "medal ", "charm ", "cameo ",
				"scarab ", "pendant ", "necklace ", "amulet "
		};
		return _accessoryNames;
	}

	const char **MISC_NAMES() {
		static const char *_miscNames[] = {
			nullptr, "rod ", "jewel ", "gem ", "box ", "orb ", "horn ", "coin ",
				"wand ", "whistle ", "potion ", "scroll ", "bogus", "bogus", "bogus",
				"bogus", "bogus", "bogus", "bogus", "bogus", "bogus", "bogus"
		};
		return _miscNames;
	}

	const char **SPECIAL_NAMES() {
		static const char *_specialNames[] = {
			nullptr, "light", "awakening", "magic arrows", "first aid", "fists", "energy blasts", "sleeping",
				"revitalization", "curing", "sparking", "shrapmetal", "insect repellent", "toxic clouds", "elemental protection",
				"pain", "jumping", "beast control", "clairvoyance", "undead turning", "levitation", "wizard eyes", "blessing",
				"monster identification", "lightning", "holy bonuses", "power curing", "nature's cures", "beacons",
				"shielding", "heroism", "hypnotism", "water walking", "frost biting", "monster finding", "fireballs",
				"cold rays", "antidotes", "acid spraying", "time distortion", "dragon sleep", "vaccination", "teleportation",
				"death", "free movement", "golem stopping", "poison volleys", "deadly swarms", "shelter", "daily protection",
				"daily sorcerery", "feasting", "fiery flails", "recharging", "freezing", "town portals", "stone to flesh",
				"raising the dead", "etherealization", "dancing swords", "moon rays", "mass distortion", "prismatic light",
				"enchant item", "incinerating", "holy words", "resurrection", "storms", "megavoltage", "infernos",
				"sun rays", "implosions", "star bursts", "the GODS!"
		};
		return _specialNames;
	}

	const char **ELEMENTAL_NAMES() {
		static const char *_elementalNames[] = {
			"Fire", "Elec", "Cold", "Acid/Poison", "Energy", "Magic"
		};
		return _elementalNames;
	}

	const char **ATTRIBUTE_NAMES() {
		static const char *_attributeNames[] = {
			"might", "Intellect", "Personality", "Speed", "accuracy", "Luck",
				"Hit Points", "Spell Points", "Armor Class", "Thievery"
		};
		return _attributeNames;
	}

	const char **EFFECTIVENESS_NAMES() {
		static const char *_effectivenessNames[] = {
			nullptr, "Dragons", "Undead", "Golems", "Bugs", "Monsters", "Beasts"
		};
		return _effectivenessNames;
	}

	const char **QUEST_ITEM_NAMES() {
		static const char *_questItemNames[] = {
			"Deed to New Castle",
				"Crystal Key to Witch Tower",
				"Skeleton Key to Darzog's Tower",
				"Enchanted Key to Tower of High Magic",
				"Jeweled Amulet of the Northern Sphinx",
				"Stone of a Thousand Terrors",
				"Golem Stone of Admittance",
				"Yak Stone of Opening",
				"Xeen's Scepter of Temporal Distortion",
				"Alacorn of Falista",
				"Elixir of Restoration",
				"Wand of Faery Magic",
				"Princess Roxanne's Tiara",
				"Holy Book of Elvenkind",
				"Scarab of Imaging",
				"Crystals of Piezoelectricity",
				"Scroll of Insight",
				"Phirna Root",
				"Orothin's Bone Whistle",
				"Barok's Magic Pendant",
				"Ligono's Missing Skull",
				"Last Flower of Summer",
				"Last Raindrop of Spring",
				"Last Snowflake of Winter",
				"Last Leaf of Autumn",
				"Ever Hot Lava Rock",
				"King's Mega Credit",
				"Excavation Permit",
				"Cupie Doll",
				"Might Doll",
				"Speed Doll",
				"Endurance Doll",
				"Accuracy Doll",
				"Luck Doll",
				"Widget",
				"Pass to Castleview",
				"Pass to Sandcaster",
				"Pass to Lakeside",
				"Pass to Necropolis",
				"Pass to Olympus",
				"Key to Great Western Tower",
				"Key to Great Southern Tower",
				"Key to Great Eastern Tower",
				"Key to Great Northern Tower",
				"Key to Ellinger's Tower",
				"Key to Dragon Tower",
				"Key to Darkstone Tower",
				"Key to Temple of Bark",
				"Key to Dungeon of Lost Souls",
				"Key to Ancient Pyramid",
				"Key to Dungeon of Death",
				"Amulet of the Southern Sphinx",
				"Dragon Pharoah's Orb",
				"Cube of Power",
				"Chime of Opening",
				"Gold ID Card",
				"Silver ID Card",
				"Vulture Repellant",
				"Bridle",
				"Enchanted Bridle",
				"Treasure Map (Goto E1 x1, y11)",
				"",
				"Fake Map",
				"Onyx Necklace",
				"Dragon Egg",
				"Tribble",
				"Golden Pegasus Statuette",
				"Golden Dragon Statuette",
				"Golden Griffin Statuette",
				"Chalice of Protection",
				"Jewel of Ages",
				"Songbird of Serenity",
				"Sandro's Heart",
				"Ector's Ring",
				"Vespar's Emerald Handle",
				"Queen Kalindra's Crown",
				"Caleb's Magnifying Glass",
				"Soul Box",
				"Soul Box with Corak inside",
				"Ruby Rock",
				"Emerald Rock",
				"Sapphire Rock",
				"Diamond Rock",
				"Monga Melon",
				"Energy Disk"
		};
		return _questItemNames;
	}

	const char **QUEST_ITEM_NAMES_SWORDS() {
		static const char *_questItemNamesSwords[] = {
			"Pass to Hart", "Pass to Impery", "Pass to town3", "Pass to town4", "Pass to town5",
				"Key to Hart Sewers", "Key to Rettig's Pyramid", "Key to the Old Temple",
				"Key to Canegtut's Pyramid", "Key to Ascihep's Pyramid", "Key to Dragon Tower",
				"Key to Darkstone Tower", "Key to Temple of Bark", "Key to Dungeon of Lost Souls",
				"Key to Ancient Pyramid", "Key to Dungeon of Death", "Red Magic Hammer",
				"Green Magic Hammer", "Golden Magic Wand", "Silver Magic Hammer", "Magic Coin",
				"Ruby", "Diamond Mineral", "Emerald", "Sapphire", "Treasure Map (Goto E1 x1, y11)",
				"NOTUSED", "Melon", "Princess Crown", "Emerald Wand", "Druid Carving", "High Sign",
				"Holy Wheel", "Double Cross", "Sky Hook", "Sacred Cow", "Staff of the Mountain",
				"Hard Rock", "Soft Rock", "Rock Candy", "Ivy Plant", "Spirit Gem", "Temple of Sun holy lamp oil",
				"Noams Hammer", "Positive Orb", "Negative Orb", "FireBane Staff", "Diamond Edged Pick",
				"Monga Melon", "Energy Disk", "Old XEEN Quest Item"
		};
		return _questItemNamesSwords;
	}

	const char *NOT_PROFICIENT() {
		return "\t000\v007\x3"
			   "c%ss are not proficient with a %s!";
	}

	const char *NO_ITEMS_AVAILABLE() {
		return "\x3"
			   "c\n"
			   "\t000No items available.";
	}

	const char **CATEGORY_NAMES() {
		static const char *_categoryNames[] = { "Weapons", "Armor", "Accessories", "Miscellaneous" };
		return _categoryNames;
	}

	const char *X_FOR_THE_Y() {
		return "\x1\fd\r%s\v000\t000%s for %s the %s%s\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *X_FOR_Y() {
		return "\x1\fd\r\x3l\v000\t000%s for %s\x3r\t000%s\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *X_FOR_Y_GOLD() {
		return "\x1\fd\r\x3l\v000\t000%s for %s\t150Gold - %u%s\x3l\v011"
			   "\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *FMT_CHARGES() {
		return "\x3rr\t000Charges\x3l";
	}

	const char *AVAILABLE_GOLD_COST() {
		return "\x1\fd\r\x3l\v000\t000Available %s\t150Gold - %u\x3r\t000Cost"
			   "\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *CHARGES() {
		return "Charges";
	}

	const char *COST() {
		return "Cost";
	}

	const char **ITEM_ACTIONS() {
		static const char *_itemActions[] = {
			"Equip", "Remove", "Use", "Discard", "Enchant", "Recharge", "Gold"
		};
		return _itemActions;
	}

	const char *WHICH_ITEM() {
		return "\t010\v005%s which item?";
	}

	const char *WHATS_YOUR_HURRY() {
		return "\v007What's your hurry?\n"
			   "Wait till you get out of here!";
	}

	const char *USE_ITEM_IN_COMBAT() {
		return "\v007To use an item in Combat, invoke the Use command on your turn!";
	}

	const char *NO_SPECIAL_ABILITIES() {
		return "\v005\x3"
			   "c%s\fdhas no special abilities!";
	}

	const char *CANT_CAST_WHILE_ENGAGED() {
		return "\x3"
			   "c\v007Can't cast %s while engaged!";
	}

	const char *EQUIPPED_ALL_YOU_CAN() {
		return "\x3"
			   "c\v007You have equipped all the %ss you can!";
	}

	const char *REMOVE_X_TO_EQUIP_Y() {
		return "\x3"
			   "c\v007You must remove %sto equip %s\b!";
	}

	const char *RING() {
		return "ring";
	}

	const char *MEDAL() {
		return "medal";
	}

	const char *CANNOT_REMOVE_CURSED_ITEM() {
		return "\x3"
			   "You cannot remove a cursed item!";
	}

	const char *CANNOT_DISCARD_CURSED_ITEM() {
		return "\3x"
			   "cYou cannot discard a cursed item!";
	}

	const char *PERMANENTLY_DISCARD() {
		return "\v000\t000\x3lPermanently discard %s\fd?";
	}

	const char *BACKPACK_IS_FULL() {
		return "\v005\x3"
			   "c\fd%s's backpack is full.";
	}

	const char **CATEGORY_BACKPACK_IS_FULL() {
		static const char *_categoryBackpackIsFull[] = {
			"\v010\t000\x3"
			"c%s's weapons backpack is full.",
				"\v010\t000\x3"
				"c%s's armor backpack is full.",
				"\v010\t000\x3"
				"c%s's accessories backpack is full.",
				"\v010\t000\x3"
				"c%s's miscellaneous backpack is full."
		};
		return _categoryBackpackIsFull;
	}

	const char *BUY_X_FOR_Y_GOLD() {
		return "\x3l\v000\t000\fdBuy %s\fd for %u %s?";
	}

	const char *SELL_X_FOR_Y_GOLD() {
		return "\x3l\v000\t000\fdSell %s\fd for %u %s?";
	}

	const char **GOLDS() {
		static const char *_sellXForYGoldEndings[] = { "gold", "" };
		return _sellXForYGoldEndings;
	}

	const char *NO_NEED_OF_THIS() {
		return "\v005\x3"
			   "c\fdWe have no need of this %s\f!";
	}

	const char *NOT_RECHARGABLE() {
		return "\v012\x3"
			   "c\fdNot Rechargeable.  %s";
	}

	const char *NOT_ENCHANTABLE() {
		return "\v012\t000\x3"
			   "cNot Enchantable.  %s";
	}

	const char *SPELL_FAILED() {
		return "Spell Failed!";
	}

	const char *ITEM_NOT_BROKEN() {
		return "\fdThat item is not broken!";
	}

	const char **FIX_IDENTIFY() {
		static const char *_fixIdentify[] = { "Fix", "Identify" };
		return _fixIdentify;
	}

	const char *FIX_IDENTIFY_GOLD() {
		return "\x3l\v000\t000%s %s\fd for %u %s?";
	}

	const char *IDENTIFY_ITEM_MSG() {
		return "\fd\v000\t000\x3"
			   "cIdentify Item\x3l\n"
			   "\n"
			   "\v012%s\fd\n"
			   "\n"
			   "%s";
	}

	const char *ITEM_DETAILS() {
		return "Proficient Classes\t132:\t140%s\n"
			   "to Hit Modifier\t132:\t140%s\n"
			   "Physical Damage\t132:\t140%s\n"
			   "Elemental Damage\t132:\t140%s\n"
			   "Elemental Resistance\t132:\t140%s\n"
			   "Armor Class Bonus\t132:\t140%s\n"
			   "Attribute Bonus\t132:\t140%s\n"
			   "Special Power\t132:\t140%s";
	}

	const char *ALL() {
		return "All";
	}

	const char *FIELD_NONE() {
		return "None";
	}

	const char *DAMAGE_X_TO_Y() {
		return "%d to %d";
	}

	const char *ELEMENTAL_XY_DAMAGE() {
		return "%+d %s Damage";
	}

	const char *ATTR_XY_BONUS() {
		return "%+d %s";
	}

	const char *EFFECTIVE_AGAINST() {
		return "x3 vs %s";
	}

	const char *QUESTS_DIALOG_TEXT() {
		return "\r\x2\x3"
			   "c\v021\t017\f37I\fdtems\t085\f37Q\fduests\t153"
			   "\f37A\fduto Notes	221\f37U\fdp\t255\f37D\fdown"
			   "\t289Exit";
	}

	const char *CLOUDS_OF_XEEN_LINE() {
		return "\b \b*-- \f04Clouds of Xeen\fd --";
	}

	const char *DARKSIDE_OF_XEEN_LINE() {
		return "\b \b*-- \f04Darkside of Xeen\fd --";
	}

	const char *SWORDS_OF_XEEN_LINE() {
		return "\b \b*-- \f04Swords of Xeen\fd --";
	}

	const char *NO_QUEST_ITEMS() {
		return "\r\x3"
			   "c\v000	000Quest Items\x3l\x2\n"
			   "\n"
			   "\x3"
			   "cNo Quest Items";
	}

	const char *NO_CURRENT_QUESTS() {
		return "\x3"
			   "c\v000\t000\n"
			   "\n"
			   "No Current Quests";
	}

	const char *NO_AUTO_NOTES() {
		return "\x3"
			   "cNo Auto Notes";
	}

	const char *QUEST_ITEMS_DATA() {
		return "\r\x1\fd\x3"
			   "c\v000\t000Quest Items\x3l\x2\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s";
	}

	const char *CURRENT_QUESTS_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000Current Quests\x3l\x2\n"
			   "%s\n"
			   "\n"
			   "%s\n"
			   "\n"
			   "%s";
	}

	const char *AUTO_NOTES_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000Auto Notes\x3l\x2\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l\n"
			   "%s\x3l";
	}

	const char *REST_COMPLETE() {
		return "\v000\t0008 hours pass.  Rest complete.\n"
			   "%s\n"
			   "%d food consumed.";
	}

	const char *PARTY_IS_STARVING() {
		return "\f07The Party is Starving!\fd";
	}

	const char *HIT_SPELL_POINTS_RESTORED() {
		return "Hit Pts and Spell Pts restored.";
	}

	const char *TOO_DANGEROUS_TO_REST() {
		return "Too dangerous to rest here!";
	}

	const char *SOME_CHARS_MAY_DIE() {
		return "Some Chars may die. Rest anyway?";
	}

	const char *DISMISS_WHOM() {
		return "\t010\v005Dismiss whom?";
	}

	const char *CANT_DISMISS_LAST_CHAR() {
		return "You cannot dismiss your last character!";
	}

	const char *DELETE_CHAR_WITH_ELDER_WEAPON() {
		return "\v000\t000This character has an Elder Weapon and cannot be deleted!";
	}

	const char **REMOVE_DELETE() {
		static const char *_removeDelete[] = { "Remove", "Delete" };
		return _removeDelete;
	}

	const char *REMOVE_OR_DELETE_WHICH() {
		return "\x3l\t010\v005%s which character?";
	}

	const char *YOUR_PARTY_IS_FULL() {
		return "\v007Your party is full!";
	}

	const char *HAS_SLAYER_SWORD() {
		return "\v000\t000This character has the Xeen Slayer Sword and cannot be deleted!";
	}

	const char *SURE_TO_DELETE_CHAR() {
		return "Are you sure you want to delete %s the %s?";
	}

	const char *CREATE_CHAR_DETAILS() {
		return "\f04\x3"
			   "c\x2\t144\v119\f37R\f04oll\t144\v149\f37C\f04reate"
			   "\t144\v179\f37ESC\f04\x3l\x1\t195\v021\f37M\f04gt"
			   "\t195\v045\f37I\f04nt\t195\v069\f37P\f04er\t195\v093\f37E\f04nd"
			   "\t195\v116\f37S\f04pd\t195\v140\f37A\f04cy\t195\v164\f37L\f04ck%s";
	}

	const char *NEW_CHAR_STATS() {
		return "\f04\x3l\t022\v148Race\t055: %s\n"
			   "\t022Sex\t055: %s\n"
			   "\t022Class\t055:\n"
			   "\x3r\t215\v031%d\t215\v055%d\t215\v079%d\t215\v103%d\t215\v127%d"
			   "\t215\v151%d\t215\v175%d\x3l\t242\v020\f%.2dKnight\t242\v031\f%.2d"
			   "Paladin\t242\v042\f%.2dArcher\t242\v053\f%.2dCleric\t242\v064\f%.2d"
			   "Sorcerer\t242\v075\f%.2dRobber\t242\v086\f%.2dNinja\t242\v097\f%.2d"
			   "Barbarian\t242\v108\f%.2dDruid\t242\v119\f%.2dRanger\f04\x3"
			   "c"
			   "\t265\v142Skills\x3l\t223\v155%s\t223\v170%s%s";
	}

	const char *NAME_FOR_NEW_CHARACTER() {
		return "\x3"
			   "cEnter a Name for this Character\n\n";
	}

	const char *SELECT_CLASS_BEFORE_SAVING() {
		return "\v006\x3"
			   "cSelect a Class before saving.\x3l";
	}

	const char *EXCHANGE_ATTR_WITH() {
		return "Exchange %s with...";
	}

	const int *NEW_CHAR_SKILLS_OFFSET() {
		static const int _newCharSkillsOffset[] = { 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 };
		return _newCharSkillsOffset;
	}

	const char *COMBAT_DETAILS() {
		return "\r\f00\x3"
			   "c\v000\t000\x2"
			   "Combat%s%s%s\x1";
	}

	const char *NOT_ENOUGH_TO_CAST() {
		return "\x3"
			   "c\v010Not enough %s to Cast %s";
	}

	const char **SPELL_CAST_COMPONENTS() {
		static const char *_spellCastComponents[] = { "Spell Points", "Gems" };
		return _spellCastComponents;
	}

	const char *CAST_SPELL_DETAILS() {
		return "\r\x2\x3"
			   "c\v122\t013\f37C\fdast\t040\f37N\fdew"
			   "\t067ESC\x1\t000\v000\x3"
			   "cCast Spell\n"
			   "\n"
			   "%s\x3l\n"
			   "\n"
			   "Spell Ready:\x3"
			   "c\n"
			   "\n"
			   "\f09%s\fd\x2\x3l\n"
			   "\v082Cost\x3r\t000%u/%u\x3l\n"
			   "Cur SP\x3r\t000%u\x1";
	}

	const char *PARTY_FOUND() {
		return "\x3"
			   "cThe Party Found:\n"
			   "\n"
			   "\x3r\t000%u Gold\n"
			   "%u Gems";
	}

	const char *BACKPACKS_FULL_PRESS_KEY() {
		return "\v007\f12Warning!  BackPacks Full!\fd\n"
			   "Press a Key";
	}

	const char *HIT_A_KEY() {
		return "\x3l\v120\t000\x4"
			   "077\x3"
			   "c\f37Hit a key\fd";
	}

	const char *GIVE_TREASURE_FORMATTING() {
		return "\x3l\v060\t000\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077";
	}

	const char **FOUND() {
		static const char *_found[] = { "found", "" };
		return _found;
	}

	const char *X_FOUND_Y() {
		return "\v060\t000\x3"
			   "c%s %s: %s";
	}

	const char *ON_WHO() {
		return "\x3"
			   "c\v009On Who?";
	}

	const char *WHICH_ELEMENT1() {
		return "\r\x3"
			   "c\x1Which Element?\x2\v034\t014\f15F\fdire\t044"
			   "\f15E\fdlec\t074\f15C\fdold\t104\f15A\fdcid\x1";
	}

	const char *WHICH_ELEMENT2() {
		return "\r\x3"
			   "cWhich Element?\x2\v034\t014\f15F\fdire\t044"
			   "\f15E\fdlec\t074\f15C\fdold\t104\f15A\fdcid\x1";
	}

	const char *DETECT_MONSTERS() {
		return "\x3"
			   "cDetect Monsters";
	}

	const char *LLOYDS_BEACON() {
		return "\r\x3"
			   "c\v000\t000\x1Lloyd's Beacon\n"
			   "\n"
			   "Last Location\n"
			   "\n"
			   "%s\x3l\n"
			   "x = %d\x3r\t000y = %d\x3"
			   "c\x2\v122\t021\f15S\fdet\t060\f15R\fdeturn\x1";
	}

	const char *HOW_MANY_SQUARES() {
		return "\x3"
			   "cTeleport\nHow many squares %s (1-9)\n";
	}

	const char *TOWN_PORTAL() {
		return "\x3"
			   "cTown Portal\x3l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\n"
			   "\t0104. %s\n"
			   "\t0105. %s\x3"
			   "c\n"
			   "\n"
			   "To which Town (1-5)\n"
			   "\n";
	}

	const char *TOWN_PORTAL_SWORDS() {
		return "\x3"
			   "cTown Portal\x3l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\x3"
			   "c\n"
			   "\n"
			   "To which Town (1-3)\n"
			   "\n";
	}

	const char *MONSTER_DETAILS() {
		return "\x3l\n"
			   "%s\x3"
			   "c\t100%s\t140%u\t180%u\x3r\t000%s";
	}

	const char **MONSTER_SPECIAL_ATTACKS() {
		static const char *_monsterSpecialAttacks[] = {
			"None", "Magic", "Fire", "Elec", "Cold", "Poison", "Energy", "Disease",
				"Insane", "Asleep", "CurseItm", "InLove", "DrnSPts", "Curse", "Paralys",
				"Uncons", "Confuse", "BrkWpn", "Weak", "Erad", "Age+5", "Dead", "Stone"
		};
		return _monsterSpecialAttacks;
	}

	const char *IDENTIFY_MONSTERS() {
		return "Name\x3"
			   "c\t100HP\t140AC\t177#Atks\x3r\t000Special%s%s%s";
	}

	const char *MOONS_NOT_ALIGNED() {
		return "\x3"
			   "c\v012\t000The moons are not aligned. Passage to the %s is unavailable";
	}

	const char *AWARDS_FOR() {
		return "\r\x1\fd\x3"
			   "c\v000\t000Awards for %s the %s\x3l\x2\n"
			   "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\x1";
	}

	const char *AWARDS_TEXT() {
		return "\r\x2\x3"
			   "c\v021\t221\f37U\fdp\t255\f37D\fdown\t289Exit";
	}

	const char *NO_AWARDS() {
		return "\x3"
			   "cNo Awards";
	}

	const char *WARZONE_BATTLE_MASTER() {
		return "The Warzone\n\t125Battle Master";
	}

	const char *WARZONE_MAXED() {
		return "What!  You again?  Go pick on someone your own size!";
	}

	const char *WARZONE_LEVEL() {
		return "What level of monsters? (1-10)\n";
	}

	const char *WARZONE_HOW_MANY() {
		return "How many monsters? (1-20)\n";
	}

	const char *PICKS_THE_LOCK() {
		return "\x3"
			   "c\v010%s %s the lock!\nPress any key.";
	}

	const char **PICK_FORM() {
		static const char *_pickForm[] = { "picks", "" };
		return _pickForm;
	}

	const char *UNABLE_TO_PICK_LOCK() {
		return "\x3"
			   "c\v010%s was %s the lock!\nPress any key.";
	}

	const char **UNABLE_TO_PICK_FORM() {
		static const char *_unableToPickForm[] = { "unable to pick", "" };
		return _unableToPickForm;
	}

	const char *CONTROL_PANEL_TEXT() {
		return "\x1\f00\x3"
			   "c\v000\t000Control Panel\x3r"
			   "\v022\t045\f06L\fdoad:\t124\f06E\fdfx:"
			   "\v041\t045\f06S\fdave:\t124\f06M\fdusic:"
			   "\v060\t045\f06Q\fduit:"
			   "\v080\t084Mr \f06W\fdizard:%s\t000\x1";
	}

	const char *CONTROL_PANEL_BUTTONS() {
		return "\x3"
			   "c\f11"
			   "\v022\t062load\t141%s"
			   "\v041\t062save\t141%s"
			   "\v060\t062exit"
			   "\v079\t102Help\fd";
	}

	const char *ON() {
		return "\f15on\f11";
	}

	const char *OFF() {
		return "\f32off\f11";
	}

	const char *CONFIRM_QUIT() {
		return "Are you sure you want to quit?";
	}

	const char *MR_WIZARD() {
		return "Are you sure you want Mr.Wizard's Help ?";
	}

	const char *NO_LOADING_IN_COMBAT() {
		return "No Loading Allowed in Combat!";
	}

	const char *NO_SAVING_IN_COMBAT() {
		return "No Saving Allowed in Combat!";
	}

	const char *QUICK_FIGHT_TEXT() {
		return "\r\fd\x3"
			   "c\v000\t000QuickFight Options\n\n"
			   "%s\x3l\n\n"
			   "Current\x3r\n"
			   "\t000%s\x2\x3"
			   "c\v122\t021\f37N\f04ext\t060Exit\x1";
	}

	const char **QUICK_FIGHT_OPTIONS() {
		static const char *_quickFightOptions[] = { "Attack", "Cast", "Block", "Run" };
		return _quickFightOptions;
	}

	const char **WORLD_END_TEXT() {
		static const char *_worldEndText[] = {
			"\n\n\n\n\n\n\n"
			"Congratulations Adventurers!\n\n"
			"Let the unification ceremony begin!",
				("And so the call went out to the people throughout the lands of Xeen"
				" that the prophecy was nearing completion."),
				"They came in great numbers to witness the momentous occasion.",
				"\v026The Dragon Pharoah presided over the ceremony.",
				"\v026Queen Kalindra presented the Cube of Power.",
				"\v026Prince Roland presented the Xeen Sceptre.",
				"\v026Together, they placed the Cube of Power...",
				"\v026and the Sceptre, onto the Altar of Joining.",
				"With the prophecy complete, the two sides of Xeen were united as one",
		};
		return _worldEndText;
	}

	const char *WORLD_CONGRATULATIONS() {
		return "\x3"
			   "cCongratulations\n\n"
			   "Your Final Score is:\n\n"
			   "%010lu\n"
			   "\x3l\n"
			   "Please send this score to the Ancient's Headquarters where "
			   "you'll be added to the Hall of Legends!\n\n"
			   "Ancient's Headquarters\n"
			   "New World Computing, Inc.\n"
			   "P.O. Box 4302\n"
			   "Hollywood, CA 90078";
	}

	const char *WORLD_CONGRATULATIONS2() {
		return "\n\n\n\n\n\n"
			   "But wait... there's more!\n"
			   "\n\n"
			   "Include the message\n"
			   "\"%s\"\n"
			   "with your final score and receive a special bonus.";
	}

	const char *CLOUDS_CONGRATULATIONS1() {
		return "\f23\x3l"
			   "\v000\t000Please send this score to the Ancient's Headquarters "
			   "where you'll be added to the Hall of Legends!\f33\x3"
			   "c"
			   "\v070\t000Press a Key";
	}

	const char *CLOUDS_CONGRATULATIONS2() {
		return "\f23\x3l"
			   "\v000\t000Ancient's Headquarters\n"
			   "New World Computing, Inc.\n"
			   "P.O. Box 4302\n"
			   "Hollywood, CA 90078-4302\f33\x3"
			   "c"
			   "\v070\t000Press a Key";
	}

	const char **GOOBER() {
		static const char *_goober[] = {
			"", "I am a Goober!", "I am a Super Goober!"
		};
		return _goober;
	}

	const char *DIFFICULTY_TEXT() {
		return "\v000\t000\x3"
			   "cSelect Game Preference";
	}

	const char *SAVE_OFF_LIMITS() {
		return "\x3"
			   "c\v002\t000The Gods of Game Restoration deem this area off limits!\n"
			   "Sorry, no saving in this maze.";
	}

	const char *CLOUDS_INTRO1() {
		return "\f00\v082\t040\x3"
			   "cKing Burlock\v190\t040Peasants\v082\t247"
			   "Lord Xeen\v190\t258Xeen's Pet\v179\t150Crodo";
	}

	const char *DARKSIDE_ENDING1() {
		return "\n\x3"
			   "cCongratulations\n"
			   "\n"
			   "Your Final Score is:\n"
			   "\n"
			   "%010lu\n"
			   "\x3"
			   "l\n"
			   "Please send this score to the Ancient's Headquarters "
			   "where you'll be added to the Hall of Legends!\n"
			   "\n"
			   "Ancient's Headquarters\n"
			   "New World Computing, Inc.\n"
			   "P.O. Box 4302\n"
			   "Hollywood, CA 90078";
	}

	const char *DARKSIDE_ENDING2() {
		return "\n"
			   "Adventurers,\n"
			   "\n"
			   "I will save your game in Castleview.\n"
			   "\n"
			   "The World of Xeen still needs you!\n"
			   "\n"
			   "Load your game afterwards and come visit me in the "
			   "Great Pyramid for further instructions";
	}

	const char *PHAROAH_ENDING_TEXT1() {
		return "\fd\v001\t001%s\x3"
			   "c\t000\v180Press a Key!\x3"
			   "l";
	}

	const char *PHAROAH_ENDING_TEXT2() {
		return "\f04\v000\t000%s\x3"
			   "c\t000\v180Press a Key!\x3"
			   "l\fd";
	}

	const char **CLOUDS_MAE_NAMES() {
		static const char *_maeNames[131] = { "" };
		return _maeNames;
	}

	const char **CLOUDS_MIRROR_LOCATIONS() {
		static const char *_mirrLocs[59] = { "" };
		return _mirrLocs;
	}

	const char **CLOUDS_MAP_NAMES() {
		static const char *_mapNames[] = {
			"",
		};
		return _mapNames;
	}

	const char **CLOUDS_MONSTERS() {
		static const char *_monsters[91] = { "" };
		return _monsters;
	}

	const char **CLOUDS_SPELLS() {
		static const char *_spells[77] = { "" };
		return _spells;
	}

	class EN_KeyConstants : public KeyConstants {
	public:
		class EN_DialogsCharInfo : public DialogsCharInfo {
		public:
			int KEY_ITEM()     { return Common::KEYCODE_i; }
			int KEY_QUICK()    { return Common::KEYCODE_q; }
			int KEY_EXCHANGE() { return Common::KEYCODE_e; }
		};
		EN_DialogsCharInfo *dialogsCharInfo() {
			if (!_dci)_dci = new EN_DialogsCharInfo();
			return _dci;
		}

		class EN_DialogsControlPanel : public DialogsControlPanel {
		public:
			int KEY_FXON()     { return Common::KEYCODE_e; }
			int KEY_MUSICON()  { return Common::KEYCODE_m; }
			int KEY_LOAD()     { return Common::KEYCODE_l; }
			int KEY_SAVE()     { return Common::KEYCODE_s; }
			int KEY_QUIT()     { return Common::KEYCODE_q; }
			int KEY_MRWIZARD() { return Common::KEYCODE_w; }
		};
		EN_DialogsControlPanel *dialogsControlPanel() {
			if (!_dcp) _dcp = new EN_DialogsControlPanel();
			return _dcp;
		}

		class EN_DialogsCreateChar : public DialogsCreateChar {
		public:
			int KEY_ROLL()   { return Common::KEYCODE_r; }
			int KEY_CREATE() { return Common::KEYCODE_c; }
			int KEY_MGT()    { return Common::KEYCODE_m; }
			int KEY_INT()    { return Common::KEYCODE_i; }
			int KEY_PER()    { return Common::KEYCODE_p; }
			int KEY_END()    { return Common::KEYCODE_e; }
			int KEY_SPD()    { return Common::KEYCODE_s; }
			int KEY_ACY()    { return Common::KEYCODE_a; }
			int KEY_LCK()    { return Common::KEYCODE_l; }
		};
		EN_DialogsCreateChar *dialogsCreateChar() {
			if (!_dcc) _dcc = new EN_DialogsCreateChar();
			return _dcc;
		}

		class EN_DialogsDifficulty : public DialogsDifficulty {
		public:
			int KEY_ADVENTURER() { return Common::KEYCODE_a; }
			int KEY_WARRIOR()    { return Common::KEYCODE_w; }
		};
		EN_DialogsDifficulty *dialogsDifficulty() {
			if (!_dd) _dd = new EN_DialogsDifficulty();
			return _dd;
		}

		class EN_DialogsItems : public DialogsItems {
		public:
			int KEY_WEAPONS()   { return Common::KEYCODE_w; }
			int KEY_ARMOR()     { return Common::KEYCODE_a; }
			int KEY_ACCESSORY() { return Common::KEYCODE_c; }
			int KEY_MISC()      { return Common::KEYCODE_m; }
			int KEY_ENCHANT()   { return Common::KEYCODE_e; }
			int KEY_USE()       { return Common::KEYCODE_u; }
			int KEY_BUY()       { return Common::KEYCODE_b; }
			int KEY_SELL()      { return Common::KEYCODE_s; }
			int KEY_IDENTIFY()  { return Common::KEYCODE_i; }
			int KEY_FIX()       { return Common::KEYCODE_f; }
			int KEY_EQUIP()     { return Common::KEYCODE_e; }
			int KEY_REM()       { return Common::KEYCODE_r; }
			int KEY_DISC()      { return Common::KEYCODE_d; }
			int KEY_QUEST()     { return Common::KEYCODE_q; }
			int KEY_RECHRG()    { return Common::KEYCODE_r; }
			int KEY_GOLD()      { return Common::KEYCODE_g; }
		};
		EN_DialogsItems *dialogsItems() {
			if (!_di) _di = new EN_DialogsItems();
			return _di;
		}

		class EN_DialogsParty : public DialogsParty {
		public:
			int KEY_DELETE() { return Common::KEYCODE_d; }
			int KEY_REMOVE() { return Common::KEYCODE_r; }
			int KEY_CREATE() { return Common::KEYCODE_c; }
			int KEY_EXIT()   { return Common::KEYCODE_x; }
		};
		EN_DialogsParty *dialogsParty() {
			if (!_dp) _dp = new EN_DialogsParty();
			return _dp;
		}

		class EN_DialogsQuests : public DialogsQuests {
		public:
			int KEY_QUEST_ITEMS()    { return Common::KEYCODE_i; }
			int KEY_CURRENT_QUESTS() { return Common::KEYCODE_q; }
			int KEY_AUTO_NOTES()     { return Common::KEYCODE_a; }
		};
		EN_DialogsQuests *dialogsQuests() {
			if (!_dq) _dq = new EN_DialogsQuests();
			return _dq;
		}

		class EN_DialogsQuickFight : public DialogsQuickFight {
		public:
			int KEY_NEXT() { return Common::KEYCODE_t; }
		};
		EN_DialogsQuickFight *dialogsQuickFight() {
			if (!_dqf) _dqf = new EN_DialogsQuickFight();
			return _dqf;
		}

		class EN_DialogsSpells : public DialogsSpells {
		public:
			int KEY_CAST()   { return Common::KEYCODE_c; }
			int KEY_NEW()    { return Common::KEYCODE_n; }
			int KEY_FIRE()   { return Common::KEYCODE_f; }
			int KEY_ELEC()   { return Common::KEYCODE_e; }
			int KEY_COLD()   { return Common::KEYCODE_c; }
			int KEY_ACID()   { return Common::KEYCODE_a; }
			int KEY_SET()    { return Common::KEYCODE_t; }
			int KEY_RETURN() { return Common::KEYCODE_r; }
		};
		EN_DialogsSpells *dialogsSpells() {
			if (!_ds) _ds = new EN_DialogsSpells();
			return _ds;
		}

		class EN_Locations : public Locations {
		public:
			int KEY_DEP()        { return Common::KEYCODE_d; }
			int KEY_WITH()       { return Common::KEYCODE_w; }
			int KEY_GOLD()       { return Common::KEYCODE_o; }
			int KEY_GEMS()       { return Common::KEYCODE_e; }
			int KEY_BROWSE()     { return Common::KEYCODE_b; }
			int KEY_BUY_SPELLS() { return Common::KEYCODE_b; }
			int KEY_SPELL_INFO() { return Common::KEYCODE_s; }
			int KEY_SIGN_IN()    { return Common::KEYCODE_s; }
			int KEY_DRINK()      { return Common::KEYCODE_d; }
			int KEY_FOOD()       { return Common::KEYCODE_f; }
			int KEY_TIP()        { return Common::KEYCODE_t; }
			int KEY_RUMORS()     { return Common::KEYCODE_r; }
			int KEY_HEAL()       { return Common::KEYCODE_h; }
			int KEY_DONATION()   { return Common::KEYCODE_d; }
			int KEY_UNCURSE()    { return Common::KEYCODE_u; }
			int KEY_TRAIN()      { return Common::KEYCODE_t; }
		};
		EN_Locations *locations() {
			if (!_l) _l = new EN_Locations();
			return _l;
		}

		class EN_CloudsOfXeenMenu : public CloudsOfXeenMenu {
		public:
			int KEY_START_NEW_GAME() { return Common::KEYCODE_s; }
			int KEY_LOAD_GAME()      { return Common::KEYCODE_l; }
			int KEY_SHOW_CREDITS()   { return Common::KEYCODE_c; }
			int KEY_VIEW_ENDGAME()   { return Common::KEYCODE_e; }
		};
		EN_CloudsOfXeenMenu *cloudsOfXeenMenu() {
			if (!_soxm)
				_soxm = new EN_CloudsOfXeenMenu();
			return _soxm;
		}

	private:
		EN_DialogsCharInfo     *_dci = NULL;
		EN_DialogsControlPanel *_dcp = NULL;
		EN_DialogsCreateChar   *_dcc = NULL;
		EN_DialogsDifficulty   *_dd = NULL;
		EN_DialogsItems        *_di = NULL;
		EN_DialogsParty        *_dp = NULL;
		EN_DialogsQuests       *_dq = NULL;
		EN_DialogsQuickFight   *_dqf = NULL;
		EN_DialogsSpells       *_ds = NULL;
		EN_Locations           *_l = NULL;
		EN_CloudsOfXeenMenu    *_soxm = NULL;
	};

	EN_KeyConstants *keyConstants() {
		if (!_kc)_kc = new EN_KeyConstants();
		return _kc;
	}

private:
	EN_KeyConstants *_kc = NULL;
};
