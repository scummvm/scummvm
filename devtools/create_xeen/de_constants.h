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

class DE : public LangConstants {
public:
	virtual ~DE() {}
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
			   "c\fdMight and Magic\n"
			   "%s of Xeen\x2\n"
			   "\v%.3dCopyright (c) %d NWC, Inc.\n"
			   "Alle Rechte vorbehalten\x1";
	}

	const char **GAME_NAMES() {
		static const char *_gameNames[] = { "Clouds", "Darkside", "World" };
		return _gameNames;
	}

	const char *THE_PARTY_NEEDS_REST() {
		return "\v012Die Party mu@ rasten!";
	}

	const char *WHO_WILL() {
		return "\x3"
			   "c\v000\t000%s\n\n"
			   "Wer will\n%s?\n\v055F1 - F%d";
	}

	const char *HOW_MUCH() {
		return "\x3"
			   "cWieviel\n\n";
	}

	const char *WHATS_THE_PASSWORD() {
		return "\x3"
			   "cWie lautet das Passwort?\n"
			   "\n"
			   "Schlagen Sie Seite %u auf,\n"
			   "Zeile %u, und geben Sie Wort %u ein.\v067\t000Leerzeichen werden nicht "
			   "gez&hlt. Jede Zeile, die Text enth&lt, wird mitgez&hlt."
			   "\x3"
			   "c\v040\t000\n";
	}

	const char *PASSWORD_INCORRECT() {
		return "\x3"
			   "c\v040\n"
			   "\f32Falsch!\fd";
	}

	const char *IN_NO_CONDITION() {
		return "\v007%s kann nichts machen!";
	}

	const char *NOTHING_HERE() {
		return "\x3"
			   "c\v010Hier ist nichts.";
	}

	const char **WHO_ACTIONS() {
		static const char *_whoActions[] = {
			"suchen", "*ffnen", "trinken", "graben", "ber}hren", "lesen", "lernen", "nehmen",
				"schlagen", "stehlen", "bestechen", "zahlen", "sitzen", "versuchen", "drehen", "baden",
				"zerst*ren", "ziehen", "hinabsteigen", "M}nze werfen", "beten", "beitreten", "handeln",
				"spielen", "dr}cken", "reiben", "aufnehmen", "essen", "unterzeichnen", "schlie@en", "schauen", "versuchen"
		};
		return _whoActions;
	}

	const char **WHO_WILL_ACTIONS() {
		static const char *_whoWillActions[] = {
			"Tor *ffnen", "T}r *ffnen", "Rolle *ffnen", "Char w@hlen"
		};
		return _whoWillActions;
	}

	const char **DIRECTION_TEXT_UPPER() {
		static const char *_directionTextUpper[] = { "NORD", "OST", "SUED", "WEST" }; 
		return _directionTextUpper;
	}

	const char **DIRECTION_TEXT() {
		static const char *_directionText[] = { "Nord", "Ost", "S}d", "West" };
		return _directionText;
	}

	const char **RACE_NAMES() {
		static const char *_raceNames[] = { "Mensch", "Elf", "Zwerg", "Gnom", "H-Ork" };
		return _raceNames;
	}

	const char **ALIGNMENT_NAMES() {
		static const char *_alignmentNames[] = { "Gut", "Neutral", "B*se" };
		return _alignmentNames;
	}

	const char **SEX_NAMES() {
		static const char *_sexNames[] = { "Mann", "Frau" };
		return _sexNames;
	}

	const char **SKILL_NAMES() {
		static const char *_skillNames[] = {
			"Diebstahl\t100", "Waffenmeister", "Astrologe", "Bodybuilder", "Kartograf",
				"Kreuzritter", "Orient.sinn", "Linguist", "H&ndler", "Bergsteiger",
				"Navigator", "Pfadfinder", "Gebetsmeister", "Taschendieb",
				"Schwimmer", "Verfolger", "T}renfinden", "Gefahrensinn"
		};
		return _skillNames;
	}

	const char **CLASS_NAMES() {
		static const char *_classNames[] = {
			"Ritter", "Paladin", "Sch}tze", "Kleriker", "Zauberer", "Dieb",
				"Ninja", "Barbar", "Druide", "Waldl.", nullptr
		};
		return _classNames;
	}
	
	const char **CONDITION_NAMES_M() {
		static const char *_conditionNamesM[] = {
			"verfl.", "gebr. Herz", "schwach", "vergiftet", "krank",
				"irre", "verliebt", "betrunken", "schl&ft", "depressiv", "verwirrt",
				"paralysiert", "bewu@tlos", "tot", "versteinert", "verrottet", "O.K."
		};
		return _conditionNamesM;
	}

	const char **CONDITION_NAMES_F() {
		static const char *_conditionNamesF[] = {
			"verfl.", "gebr. Herz", "schwach", "vergiftet", "krank",
				"irre", "verliebt", "betrunken", "schl&ft", "depressiv", "verwirrt",
				"paralysiert", "bewu@tlos", "tot", "versteinert", "verrottet", "O.K."
		};
		return _conditionNamesF;
	}

	const char *GOOD() {
		return "Gut";
	}

	const char *BLESSED() {
		return "\n\t020Gesegnet\t095%+d";
	}

	const char *POWER_SHIELD() {
		return "\n\t020Kraft Schild\t095%+d";
	}

	const char *HOLY_BONUS() {
		return "\n\t020Heil. Bonus\t095%+d";
	}

	const char *HEROISM() {
		return "\n\t020Held\t095%+d";
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
			   "c\v106\t013Hoch\t048Runter\t083\f37L\f*sch\t118\f37W\fdeg"
			   "\t153\f37E\fdrst.\t188E\f37n\fdde\x1";
	}

	const char *NO_ONE_TO_ADVENTURE_WITH() {
		return "Und wer soll ins Abenteuer ziehen?";
	}

	const char *YOUR_ROSTER_IS_FULL() {
		return "Das Register ist voll!";
	}

	const char *PLEASE_WAIT() {
		return "\014"
			   "d\003"
			   "c\011"
			   "000"
			   "\013"
			   "002Einen Moment...";
	}

	const char *OOPS() {
		return "\003"
			   "c\011"
			   "000\013"
			   "002Hoppla...";
	}

	const char *BANK_TEXT() {
		return "\r\x2\x3"
			   "c\v122\t013"
			   "\f37L\fdag.\t040\f37A\fdbh.\t067ESC"
			   "\x1\t000\v000Bank von Xeen\v015\n"
			   "Bank\x3l\n"
			   "Gold\x3r\t000%s\x3l\n"
			   "Edel\x3r\t000%s\x3"
			   "c\n"
			   "\n"
			   "Party\x3l\n"
			   "Gold\x3r\t000%s\x3l\n"
			   "Edel\x3r\t000%s";
	}

	const char *BLACKSMITH_TEXT() {
		return "\x1\r\x3"
			   "c\v000\t000"
			   "Schmied\t039\v027%s\x3l\v046\n"
			   "\t011\f37S\fdt*bern\n"
			   "\t000\v090Gold\x3r\t000%s"
			   "\x2\x3"
			   "c\v122\t040ESC\x1"; 
	}

	const char *GUILD_NOT_MEMBER_TEXT() {
		return "\n\nDu must hier Mitglied sein."; 
	}

	const char *GUILD_TEXT() {
		return "\x3"
			   "c\v027\t039%s"
			   "\x3l\v046\n"
			   "\t012\f37K\fdaufen\n"
			   "\t012\f37I\fdnfo"; 
	}

	const char *TAVERN_TEXT() {
		return "\r\x3"
			   "c\v000\t000Gasthaus\t039"
			   "\v027%s%s\x3l\t000"
			   "\v090Gold\x3r\t000%s\x2\x3"
			   "c\v122"
			   "\t021\f37R\fdegister\t060ESC\x1";
	}

	const char *FOOD_AND_DRINK() {
		return "\x3l\t017\v046\f37T\fdrinken\n"
			   "\t017\f37N\fdahrung\n"
			   "\t017T\f37i\fdp\n"
			   "\t017\f37G\fder}chte"; 
	}

	const char *GOOD_STUFF() {
		return "\n"
			   "\n"
			   "Gutes Zeug\n"
			   "\n"
			   "Taste...";
	}

	const char *HAVE_A_DRINK() {
		return "\n\nNehmt einen Drink\n\nTaste...";
	}

	const char *YOURE_DRUNK() {
		return "\n\nDu bist betrunken\n\nTaste...";
	}

	const char *TEMPLE_TEXT() {
		return "\r\x3"
			   "c\v000\t000Tempel"
			   "\t039\v027%s\x3l\t000\v046"
			   "\f37H\fdeilen\x3r\t000%u\x3l\n"
			   "\f37S\fdpende\x3r\t000%u\x3l\n"
			   "\f37F\fdlu. weg\x3r\t000%s"
			   "\x3l\t000\v090Gold\x3r\t000%s"
			   "\x2\x3"
			   "c\v122\t040ESC\x1";
	}

	const char *EXPERIENCE_FOR_LEVEL() {
		return "%s braucht %u EP f}r Stufe %u.";
	}

	const char *TRAINING_LEARNED_ALL() {
		return "%s kann schon alles!";
	}

	const char *ELIGIBLE_FOR_LEVEL() {
		return "%s kann in Stufe %u aufsteigen.\x3l\n"
			   "\v081Preis\x3r\t000%u";
	}

	const char *TRAINING_TEXT() {
		return "\r\x3"
			   "cTraining\n"
			   "\n"
			   "%s\x3l\v090\t000Gold\x3r\t000%s\x2\x3"
			   "c\v122\t021"
			   "\f37T\fdrain.\t060ESC\x1";
	}

	const char *GOLD_GEMS() {
		return "\x3"
			   "c\v000\t000%s\x3l\n"
			   "\n"
			   "Gold\x3r\t000%s\x3l\n"
			   "Edel\x3r\t000%s\x2\x3"
			   "c\v096\t013\f37G\fdold\t040\f37E\fddel\t067ESC\x1";
	}

	const char *GOLD_GEMS_2() {
		return "\x3"
			   "c\v000\t000%s\x3l\n"
			   "\n"
			   "\x4"
			   "077Gold\x3r\t000%s\x3l\n"
			   "\x4"
			   "077Edel\x3r\t000%s\x3l\t000\v051\x4"
			   "077\n"
			   "\x4"
			   "077";
	}

	const char **DEPOSIT_WITHDRAWL() {
		static const char *_depositWithdrawl[] = { "Lagern", "Abheben" };
		return _depositWithdrawl;
	}

	const char *NOT_ENOUGH_X_IN_THE_Y() {
		return "\x3"
			   "c\v012Zu wenig %s in %s!\x3l";
	}

	const char *NO_X_IN_THE_Y() {
		return "\x3"
			   "c\v012Kein %s in %s!\x3l";
	}

	const char **STAT_NAMES() {
		static const char *_statNames[] = {
			"Kraft", "Intellekt", "Pers*nlichkeit", "Ausdauer", "Tempo",
				"Genauigkeit", "Gl}ck", "Alter", "Stufe", "R}stungsklasse", "Trefferpunkte",
				"Spruchpunkte", "Widerstand", "Talente", "Preise", "Erfahrung"
		};
		return _statNames;
	}
	const char **CONSUMABLE_NAMES() {
		static const char *_consumableNames[] = { "Gold", "Edel", "Nahrung", "Kondition" };
		return _consumableNames;
	}

	const char **CONSUMABLE_GOLD_FORMS() {
		static const char *_consumableGoldForms[] = { "Gold" };
		return _consumableGoldForms;
	}

	const char **CONSUMABLE_GEM_FORMS() {
		static const char *_consumableGemForms[] = { "Edelsteine" };
		return _consumableGemForms;
	}

	const char **WHERE_NAMES() {
		static const char *_whereNames[] = { "Party", "Bank" };
		return _whereNames;
	}

	const char *AMOUNT() {
		return "\x3"
			   "c\t000\v051Betrag\x3l\n";
	}

	const char *FOOD_PACKS_FULL() {
		return "\v007Ihr habt schon genug!";
	}

	const char *BUY_SPELLS() {
		return "\x3"
			   "c\v027\t039%s\x3l\v046\n"
			   "\t012\f37K\fdaufen\n"
			   "\t012\f37I\fdnfo";
	}

	const char *GUILD_OPTIONS() {
		return "\r\f00\x3"
			   "c\v000\t000Guilde%s"
			   "\x3l\t000\v090Gold"
			   "\x3r\t000%s\x2\x3"
			   "c\v122\t040ESC\x1";
	}

	const char *NOT_A_SPELL_CASTER() {
		return "Ist kein Magier..."; 
	}

	const char *SPELLS_LEARNED_ALL() {
		return "Du kannst hier nichts\n"
			   "\t010mehr lernen.";
	}

	const char *SPELLS_FOR() {
		return "\r\fd%s\x2\x3"
			   "c\t000\v002Spr}che f}r %s";
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
		return "S.P.";
	}

	const char *GOLD() {
		return "Gold";
	}

	const char *SPELL_INFO() {
		return "\x3"
			   "c\f09%s\fd\x3l\n"
			   "\n"
			   "%s\x3"
			   "c\t000\v100Taste...";
	}

	const char *SPELL_PURCHASE() {
		return "\x3l\v000\t000\fd%s "
				"\f09Willst du %s f}r %u kaufen?";
	}

	const char *MAP_TEXT() {
		return "\x3"
			   "c\v000\t000%s\x3l\v139"
			   "\t000X = %d\x3r\t000Y = %d\x3"
			   "c\t000%s";
	}

	const char *LIGHT_COUNT_TEXT() {
		return "\x3l\n\n\t024Licht\x3r\t124%d";
	}

	const char *FIRE_RESISTENCE_TEXT() {
		return "%c%sFeuer%s%u";
	}

	const char *ELECRICITY_RESISTENCE_TEXT() {
		return "%c%sElektrizit*t%s%u";
	}

	const char *COLD_RESISTENCE_TEXT() {
		return "%c%sCold%s%u";
	}

	const char *POISON_RESISTENCE_TEXT() {
		return "%c%sGift%s%u";
	}

	const char *CLAIRVOYANCE_TEXT() {
		return "%c%sHellsehen%s";
	}

	const char *LEVITATE_TEXT() {
		return "%c%sSchweben%s";
	}

	const char *WALK_ON_WATER_TEXT() {
		return "%c%sWasser wandeln";
	}

	const char *GAME_INFORMATION() {
		return "\r\x3"
			   "c\t000\v001\f37%s of Xeen\fd\n"
			   "Spiel Information\n"
			   "\n"
			   "Heute ist \f37%sstag\fd\n"
			   "\n"
			   "\t032Zeit\t072Tag\t112Jahr\n"
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
		    "10", "1", "2", "3", "4", "5", "6", "7", "8", "9"
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
			   "\x3l\t131\f15%u\fd\t196\f15%u Tag%s\fd"
			   "\x3r\t053\v120\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3l\t131\f15%u\fd\t196\f%02u%s\fd"
			   "\t230%s%s%s%s\fd";
	}

	const char **DAYS() {
		static const char *_days[] = { "", "e", "" };
		return _days;
	}

	const char *PARTY_GOLD() {
		return "Party Gold";
	}

	const char *PLUS_14() {
		return "\f14+";
	}

	const char *CHARACTER_TEMPLATE() {
		return "\x1\f00\r\x3l\t029\v018Kra\t080Gen\t131T.P.\t196Erfahrung"
			   "\t029\v041Int\t080Glk\t131S.P.\t029\v064Per\t080Alt"
			   "\t131Wider\t196Edelsteine\t029\v087Asd\t080Stf\t131Talent"
			   "\t196Nahrung\t029\v110Tem\t080Rkl\t131Preis\t196Kondition\x3"
			   "c"
			   "\t290\v025\f37G\fdeg\t290\v057\f37K"
			   "\fdurz\t290\v089\f37T\fdau.\t290\v121Ende\x3l%s";
	}

	const char *EXCHANGING_IN_COMBAT() {
		return "\x3"
			   "c\v007\t000Nicht im Kampf!";
	}

	const char *CURRENT_MAXIMUM_RATING_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Aktuell / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u\n"
			   "\x3"
			   "cEinstufung: %s";
	}

	const char *CURRENT_MAXIMUM_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Aktuell / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u";
	}

	const char **RATING_TEXT() {
		static const char *_ratingText[] = {
			"Nichts", "sehr arm", "Arm", "Sehr wenig", "Wenig", "Normal", "Gut",
				"Sehr gut", "Hoch", "Sehr hoch", "Gro@artig", "Super", "Toll", "Unglaublich",
				"Gigantisch", "Fantastisch", "Tierisch", "Wahnsinnig", "Monumental", "Enorm",
				"Kollosal", "H*llisch", "Titanisch", "Perfekt"
		};
		return _ratingText;
	}

	const char **BORN() {
		static const char *_born[] = { "Geboren", "" };
		return _born;
	}

	const char *AGE_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Aktuell / Nat}rlich\n"
			   "\x3r\t057%u\x3l\t061/ %u\n"
			   "\x3"
			   "c%s: %u / %u\x1";
	}

	const char *LEVEL_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "Aktuell / Maximum\n"
			   "\x3r\t054%u\x3l\t058/ %u\n"
			   "\x3"
			   "c%u Angriff%s/Runde\x1";
	}

	const char *RESISTENCES_TEXT() {
		return "\x2\x3"
			   "c%s\x3l\n"
			   "\t020Feuer\t100%u\n"
			   "\t020K&lte\t100%u\n"
			   "\t020Elektrizit&t\t100%u\n"
			   "\t020Gift\t100%u\n"
			   "\t020Energie\t100%u\n"
			   "\t020Magie\t100%u";
	}

	const char *NONE() {
		return "\n\t020Nichts";
	}

	const char *EXPERIENCE_TEXT() {
		return "\x2\x3"
			   "c%s\x3l\n"
			   "\t010Aktuell:\t070%u\n"
			   "\t010Aufstieg:\t070%s\x1";
	}

	const char *ELIGIBLE() {
		return "\f12Geeignet\fd";
	}

	const char *IN_PARTY_IN_BANK() {
		return "\x2\x3"
			   "cParty %s\n"
			   "%u bar\n"
			   "%u auf Bank\x1\x3l";
	}

	const char **FOOD_ON_HAND() {
		static const char *_onHand[] = { "bar", "", "" };
		return _onHand;
	}

	const char *FOOD_TEXT() {
		return "\x2\x3"
			   "cParty %s\n"
			   "%u %s\n"
			   "Genug f}r %u Tag%s\x3l";
	}

	const char *EXCHANGE_WITH_WHOM() {
		return "\t010\v005Mit wem tauschen?";
	}

	const char *QUICK_REF_LINE() {
		return "\v%3d\t007%u)\t027%s\t110%c%c%c\x3r\t160\f%02u%u\fd"
			   "\x3l\t170\f%02u%d\fd\t208\f%02u%u\fd\t247\f"
			   "%02u%u\fd\t270\f%02u%c%c%c%c\fd";
	}

	const char *QUICK_REFERENCE() {
		return "\r\x3"
			   "cKurz}bersicht\v012\x3l"
			   "\t007#\t027Name\t110Kls\t140Stf\t176T.P."
			   "\t212S.P.\t241Rkl.\t270Kond"
			   "%s%s%s%s%s%s%s%s"
			   "\v110\t064\x3"
			   "cGold\t144Edel\t224Nahr\v119"
			   "\t064\f15%u\t144%u\t224%u Tag%s\fd";
	}

	const char *ITEMS_DIALOG_TEXT1() {
		return "\r\x2\x3"
			   "c\v021\t017\f37W\fdaff\t051\f37R\fd}st\t085A"
			   "\f37c\fdces\t119\f37V\fdsch\t153%s\t187%s\t221%s"
			   "\t255%s\t289Ende"; 
	}

	const char *ITEMS_DIALOG_TEXT2() {
		return "\r\x2\x3"
			   "c\v021\t017\f37W\fdaff\t051\f37R\fd}st\t085A"
			   "\f37c\fdces\t119\f37V\fsch\t153\f37%s\t289Ende";  
	}

	const char *ITEMS_DIALOG_LINE1() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\n";
	}

	const char *ITEMS_DIALOG_LINE2() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%u\n";
	}

	const char *BTN_BUY() {
		return "\f37K\fdaufen";
	}

	const char *BTN_SELL() {
		return "V\f37e\fdrk.";
	}

	const char *BTN_IDENTIFY() {
		return "\f37I\fddentif.";
	}

	const char *BTN_FIX() {
		return "Re\f37p\fd.";
	}

	const char *BTN_USE() {
		return "\f37A\fdnl.";
	}

	const char *BTN_EQUIP() {
		return "\f37A\fdnl.";
	}

	const char *BTN_REMOVE() {
		return "Ab\f37l\fd.";
	}

	const char *BTN_DISCARD() {
		return "We\f37g\fd";
	}

	const char *BTN_QUEST() {
		return "A\f37u\fdfg."; 
	}

	const char *BTN_ENCHANT() {
		return "Ver\fdzaub";
	}

	const char *BTN_RECHARGE() {
		return "Er\fdneu";
	}

	const char *BTN_GOLD() {
		return "G\fdold"; 
	}

	const char *ITEM_BROKEN() {
		return "\f32kaputt ";
	}

	const char *ITEM_CURSED() {
		return "\f09verflucht ";
	}

	const char *ITEM_OF() {
		return "von ";
	}

	const char **BONUS_NAMES() {
		static const char *_bonusNames[] = {
			"", "Drachent*ter", "Leichenfetzer", "Golemquetscher",
				"K&ferkiller", "Monsterbrecher", "Bestient*ter"
		};
		return _bonusNames;
	}

	const char **WEAPON_NAMES() {
		static const char *_weaponNames[] = {
			nullptr, "Lang Schwert ", "Kurz Schwert ", "Breit Schwert ", "Skimitar ",
				"Messer ", "S&bel ", "Stock ", "Axt ", "Katana ", "Nunchakas ",
				"Wakazashi ", "Dolch ", "Keule ", "Pr}gel ", "Kn}ppel ", "Tot Schl&ger ", "Speer ",
				"Bartaxt ", "Glefe ", "Hellebarde ", "Pike ", "Flamberge ", "Dreizack ",
				"Stab ", "Hammer ", "Naginata ", "Streitaxt ", "Gro@axt ", "Hammer Axt ",
				"Kurz Bogen ", "Lang Bogen ", "Armbrust ", "Schleuder ", "Xeen T*ter ",
				"Elfen Lang Schwert ", "Elfen Dolch ", "Elfen Keule ", "Elfen Speer ",
				"Elfen Stab ", "Elfen Lang Bogen "
		};
		return _weaponNames;
	}

	const char **ARMOR_NAMES() {
		static const char *_armorNames[] = {
			nullptr, "Robe ", "Schuppen R}stung ", "Ring Panzer ", "Ketten Panzer ",
				"Schienen Panzer ", "Platten Panzer ", "Platten R}stung ", "Schild ",
				"Helm ", "Stiefel ", "Mantel ", "Umhang ", "Handschuhe "
		};
		return _armorNames;
	}

	const char **ACCESSORY_NAMES() {
		static const char *_accessoryNames[] = {
			nullptr, "Ring ", "G\x7rtel ", "Brosche ", "Medaillon ", "Gemme ", "Kamee ",
				"Skarab&us ", "Anh&nger ", "Kette ", "Amulett "
		};
		return _accessoryNames;
	}

	const char **MISC_NAMES() {
		static const char *_miscNames[] = {
			nullptr, "Stange ", "Juwel ", "Edelstein ", "Dose ", "Kugel ", "Horn ", "M}nze ",
				"Zauberstab ", "Fl*te ", "Trank ", "Schrift Rolle ", "bogus", "bogus", "bogus",
				"bogus", "bogus", "bogus", "bogus", "bogus", "bogus", "bogus"
		};
		return _miscNames;
	}

	const char **SPECIAL_NAMES() {
		static const char *_specialNames[] = {
			nullptr, "Licht", "Erwachen", "Magie Pfeil", "Erste Hilfe", "Faust", "Energie Schlag", "Schlaf",
				"Erfrischen", "Heilen", "Funken", "Schrapnell", "Insekten Abwehr", "Giftwolke", "Element Schutz",
				"Schmerz", "Sprung", "Bestien Kontrolle", "Hellsehen", "Untote Abwenden", "Schweben", "Magierblick", "Segen",
				"Monster Erkennen", "Blitz", "Bonus", "Extra Heilung", "Natur Heilung", "Signal",
				"Schild", "Helden", "Hypnose", "Wasser wandeln", "Frost", "Monster erkennen", "Feuerball",
				"K&lte", "Gegengift", "S&ure", "Zeit verzerren", "Drachenschlaf", "Impfen", "Teleport",
				"Tod", "Frei bewegen", "Golem stoppen", "Giftschwall", "Schwarm", "Schutz", "Tagesschutz",
				"Tageszauber", "Fest", "Feuerpr}gel", "Erneuern", "Frieren", "Portal", "Stein zu Fleisch",
				"Tote erwecken", "&therisieren", "Schwert", "Mondstrahl", "Masse verzerren", "Prismalicht",
				"Verzaubern", "Ein&schern", "Heiliges Wort", "Wiederbelebung", "Sturm", "Hochspannung", "Inferno",
				"Sonnenstrahl", "Implosion", "Sternenstaub", "G*tter"
		};
		return _specialNames;
	}
 
	const char **ELEMENTAL_NAMES() {
		static const char *_elementalNames[] = {
			"Feuer", "Elektro", "K&lte", "S&ure/Gift", "Energie", "Magie"
		};
		return _elementalNames;
	}

	const char **ATTRIBUTE_NAMES() {
		static const char *_attributeNames[] = {
			"Kraft", "Intellekt", "Pers*nlichkeit", "Tempo", "Genauigkeit", "Gl}ck",
				"Trefferpunkte", "Spruchpunkte", "R}stungsklasse", "Diebstahl"
		};
		return _attributeNames;
	}

	const char **EFFECTIVENESS_NAMES() {
		static const char *_effectivenessNames[] = {
			nullptr, "Drachen", "Untoten", "Golem", "K&fer", "Monster", "Bestien"
		};
		return _effectivenessNames;
	}


	const char **QUEST_ITEM_NAMES() {
		static const char *_questItemNames[] = {
			"Urkunde f}r Neuburg",
				"Schl}ssel zum Hexenturm",
				"Schl}ssel zu Darzogs Turm",
				"Schl}ssel zum Turm der Hohen Magie",
				"Amulett der n*rdlichen Sphinx",
				"Stein des Terrors",
				"Golemstein des Zugangs",
				"Yakstein des $ffnens",
				"Xeens Zepter der Zeit Verzerrung",
				"Alacorn von Falista",
				"Elixir der Wieder Herstellung",
				"Stab der Feen Magie",
				"Prinzessin Roxannes Tiara",
				"Heiliges Elfenbuch",
				"Skarab&us der Bilder",
				"Kristall der Piezo Elektrizit&t",
				"Schriftrolle der Einsicht",
				"Phirna Wurzel",
				"Orothins Knochen Pfeife",
				"Baroks magischer Anh&nger",
				"Ligonos fehlender Sch&del",
				"Letzte Sommerbl}te",
				"Letzter Fr}hlings Regen Tropfen",
				"Letzte Winter Schnee Flocke",
				"Letzter Herbstzweig",
				"Immerhei@er Lavastein",
				"Megakredit des K*nigs",
				"Erlaubnis",
				"Liebes Puppe",
				"Kraft Puppe",
				"Tempo Puppe",				
				"Ausdauer Puppe",
				"Genauigkeits Puppe",
				"Gl}cks Puppe",
				"kleines Etwas",
				"Pass f}r Schlo@blick",
				"Pass f}r Sandstadt",
				"Pass f}r Uferstadt",
				"Pass f}r Necropolis",
				"Pass f}r Olympus",
				"Schl}ssel zum Westturm",
				"Schl}ssel zum S}dturm",
				"Schl}ssel zum Ostturm",
				"Schl}ssel zum Nordturm",
				"Schl}ssel zu Ellingers Turm",
				"Schl}ssel zum Drachen Turm",
				"Schl}ssel zum Dunkel Turm",
				"Schl}ssel zum Rinden Tempel",
				"Schl}ssel zu den Verlorenen Seelen",
				"Schl}ssel zur alten Pyramide",
				"Schl}ssel zum Todes Dungeon",
				"Amulett der s}dlichen Sphinx",
				"Kugel des Drachen Pharao",
				"W}rfel der Macht",
				"Melodie des $ffnens",
				"Goldene ID Karte",
				"Silberne ID Karte",
				"Geierabwehr",
				"Zaumzeug",
				"verzaubertes Zaumzeug",
				"Schatz Karte (E1 x1, y11)",
				"",
				"gef&lschte Karte",
				"Onyx Kette",
				"Drachen Ei",
				"Tribble",
				"Goldene Pegasus Statuette",
				"Goldene Drachen Statuette",
				"Goldene Greifen Statuette",
				"Kelch des Schutzes",
				"Juwel des Alters",
				"Singvogel der Heiterkeit",
				"Sandros Herz",
				"Ectors Ring",
				"Vespars Smaragd Griff",
				"K*nigin Kalindras Krone",
				"Calebs Vergr. Glas",
				"Seelen Dose",
				"Seelen Dose mit Corak",
				"Rubin Stein",
				"Smaragd Stein",
				"Saphir Stein",
				"Diamant Stein",
				"Monga Melone",
				"Energie Scheibe"
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
		return _questItemNamesSwords;   //TODO Swords of Xeen
	}

	const char *NOT_PROFICIENT() {
		return "\t000\v007\x3"
			   "c%s kann mit %s nicht umgehen!";
	}

	const char *NO_ITEMS_AVAILABLE() {
		return "\x3"
			   "c\n"
			   "\t000Keine Eintr&ge verf}gbar."; 
	}

	const char **CATEGORY_NAMES() {
		static const char *_categoryNames[] = { "Waffen", "R}stungen", "Accessoires", "Verschiedenes" };
		return _categoryNames;
	}

	const char *X_FOR_THE_Y() {
		return "\x1\fd\r%s\v000\t000%s f}r %s den %s%s\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *X_FOR_Y() {
		return "\x1\fd\r\x3l\v000\t000%s f}r %s\x3r\t000%s\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *X_FOR_Y_GOLD() {
		return "\x1\fd\r\x3l\v000\t000%s f}r %s\t150Gold - %u%s\x3l\v011"
			   "\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *FMT_CHARGES() {
		return "\x3rr\t000Anw.\x3l";
	}

	const char *AVAILABLE_GOLD_COST() {
		return "\x1\fd\r\x3l\v000\t000%s\t150Gold - %u\x3r\t000Preis"
			   "\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *CHARGES() {
		return "Anw."; 
	}

	const char *COST() {
		return "Preis";
	}

	const char **ITEM_ACTIONS() {
		static const char *_itemActions[] = {
			"anlegen", "ablegen", "benutzen", "weglegen", "verzaubern", "erneuern", "zu Gold machen"
		};
		return _itemActions;
	}

	const char *WHICH_ITEM() {
		return "\t010\v005Was %s?";
	}

	const char *WHATS_YOUR_HURRY() {
		return "\v007Warum so eilig?\n"
			   "Wartet, bis Ihr hier rauskommt!";
	}

	const char *USE_ITEM_IN_COMBAT() {
		return "\v007Zum benutzen bitte den entsprechenden Befehl aufrufen!";
	}

	const char *NO_SPECIAL_ABILITIES() {
		return "\v005\x3"
			   "c%s\fdhat keine besonderen F&higkeiten!";
	}

	const char *CANT_CAST_WHILE_ENGAGED() {
		return "\x3"
			   "c\v007Kann %s jetzt nicht sprechen!";
	}

	const char *EQUIPPED_ALL_YOU_CAN() {
		return "\x3"
			   "c\v007Noch mehr %ss kannst du nicht anlegen!";
	}

	const char *REMOVE_X_TO_EQUIP_Y() {
		return "\x3"
			   "c\v007Du mu@t %s ablegen, um %s anzulegen!\b!";
	}

	const char *RING() {
		return "Ring";
	}

	const char *MEDAL() {
		return "medal"; 
	}

	const char *CANNOT_REMOVE_CURSED_ITEM() {
		return "\x3"
			   "Nichts verfluchtes ablegen!";
	}

	const char *CANNOT_DISCARD_CURSED_ITEM() {
		return "\3x"
			   "cNichts verfluchtes ablegen!";
	}

	const char *PERMANENTLY_DISCARD() {
		return "\v000\t000\x3l%s f}r immer weglegen\fd?";
	}

	const char *BACKPACK_IS_FULL() {
		return "\v005\x3"
			   "c\fd%ss Rucksack ist voll.";
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
		return "\x3l\v000\t000\fd%s\fd f}r %u %s kaufen?";
	}

	const char *SELL_X_FOR_Y_GOLD() {
		return "\x3l\v000\t000\fd%s\fd f}r %u %s verkaufen?";
	}

	const char **GOLDS() {
		static const char *_sellXForYGoldEndings[] = { "Gold", "" };
		return _sellXForYGoldEndings;
	}

	const char *NO_NEED_OF_THIS() {
		return "\v005\x3"
			   "c\fdIch habe keine Verwendung f}r %s\f!";
	}

	const char *NOT_RECHARGABLE() {
		return "\v012\x3"
			   "c\fdKann nicht erneuert werden.  %s";
	}

	const char *NOT_ENCHANTABLE() {
		return "\v012\t000\x3"
			   "cKann nicht verzaubert werden.  %s";
	}

	const char *SPELL_FAILED() {
		return "Spruch hat versagt!";
	}

	const char *ITEM_NOT_BROKEN() {
		return "\fdDas ist nicht kaputt!";
	}

	const char **FIX_IDENTIFY() {
		static const char *_fixIdentify[] = { "Repariere", "Identifiziere" };
		return _fixIdentify;
	}

	const char *FIX_IDENTIFY_GOLD() {
		return "\x3l\v000\t000%s %s\fd f}r %u %s?";
	}

	const char *IDENTIFY_ITEM_MSG() {
		return "\fd\v000\t000\x3"
			   "cIdentifizieren\x3l\n"
			   "\n"
			   "\v012%s\fd\n"
			   "\n"
			   "%s";
	}

	const char *ITEM_DETAILS() {
		return "M*gliche Klassen\t132:\t140%s\n"
			   "Schlagbonus\t132:\t140%s\n"
			   "Physikal. Schaden\t132:\t140%s\n"
			   "Element. Schaden\t132:\t140%s\n"
			   "Element. Widerstand\t132:\t140%s\n"
			   "RKl Bonus\t132:\t140%s\n"
			   "Attribut Bonus\t132:\t140%s\n"
			   "Magie\t132:\t140%s";
	}

	const char *ALL() {
		return "Alle";
	}

	const char *FIELD_NONE() {
		return "Kein";
	}

	const char *DAMAGE_X_TO_Y() {
		return "%d zu %d";
	}

	const char *ELEMENTAL_XY_DAMAGE() {
		return "%+d %s Schaden";
	}

	const char *ATTR_XY_BONUS() {
		return "%+d %s";
	}

	const char *EFFECTIVE_AGAINST() {
		return "x3 vs %s";
	}

	const char *QUESTS_DIALOG_TEXT() {
		return "\r\x2\x3"
			   "c\v021\t017\f37G\fdegen.\t085\f37A\fdufgaben\t153"
			   "\f37N\fdotizen	221\f37H\fdoch\t255\f37R\fdunter"
			   "\t289Ende";
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
			   "c\v000	000Gegenst&nde\x3l\x2\n"
			   "\n"
			   "\x3"
			   "cKeine Gegenst&nde";
	}

	const char *NO_CURRENT_QUESTS() {
		return "\x3"
			   "c\v000\t000\n"
			   "\n"
			   "Keine Aufgaben";
	}

	const char *NO_AUTO_NOTES() {
		return "\x3"
			   "cKeine Notizen";
	}

	const char *QUEST_ITEMS_DATA() {
		return "\r\x1\fd\x3"
			   "c\v000\t000Gegenst&nde\x3l\x2\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s\n"
			   "\f04 - \fd%s";
	}

	const char *CURRENT_QUESTS_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000Aufgaben\x3l\x2\n"
			   "%s\n"
			   "\n"
			   "%s\n"
			   "\n"
			   "%s";
	}

	const char *AUTO_NOTES_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000Notizen\x3l\x2\n"
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
		return "\v000\t0008 Stunden vergehen. Rast beendet.\n"
			   "%s\n"
			   "%d Tagesrationen verzehrt.";
	}

	const char *PARTY_IS_STARVING() {
		return "\f07Die Party hungert!\fd";
	}

	const char *HIT_SPELL_POINTS_RESTORED() {
		return "T.P. und S.P. erneuert.";
	}

	const char *TOO_DANGEROUS_TO_REST() {
		return "Hier ist es zu gef&hrlich zum Rasten!";
	}

	const char *SOME_CHARS_MAY_DIE() {
		return "Einige Charaktere k*nnten sterben. Trotzdem rasten?";
	}

	const char *DISMISS_WHOM() {
		return "\t010\v005Wen?";
	}

	const char *CANT_DISMISS_LAST_CHAR() {
		return "Einer mu@ in der Party bleiben!";
	}

	const char *DELETE_CHAR_WITH_ELDER_WEAPON() {
		return "\v000\t000This character has an Elder Weapon and cannot be deleted!";
	}

	const char **REMOVE_DELETE() {
		static const char *_removeDelete[] = { "entlassen", "l*schen" };
		return _removeDelete;
	}

	const char *REMOVE_OR_DELETE_WHICH() {
		return "\x3l\t010\v005Wen %s?";
	}

	const char *YOUR_PARTY_IS_FULL() {
		return "\v007Die Party ist voll!";
	}

	const char *HAS_SLAYER_SWORD() {
		return "\v000\t000Dieser Charakter hat den Xeen T*ter und kann nicht gel*scht werden!";
	}

	const char *SURE_TO_DELETE_CHAR() {
		return "Bestimmt %s den %s l*schen?";
	}

	const char *CREATE_CHAR_DETAILS() {
		return "\f04\x3"
			   "c\x2\t144\v119\f37W\f04urf\t144\v149\f37E\f04rst."
			   "\t144\v179\f37ESC\f04\x3l\x1\t195\v021\f37K\f04ra"
			   "\t195\v045\f37I\f04nt\t195\v069\f37P\f04er\t195\v093\f37A\f04us"
			   "\t195\v116\f37T\f04em\t195\v140\f37G\f04en\t195\v164G\f37l\f04k%s";
	}

	const char *NEW_CHAR_STATS() {
		return "\f04\x3l\t022\v148Rass\t055: %s\n"
			   "\t022Ges.\t055: %s\n"
			   "\t022Klass\t055:\n"
			   "\x3r\t215\v031%d\t215\v055%d\t215\v079%d\t215\v103%d\t215\v127%d"
			   "\t215\v151%d\t215\v175%d\x3l\t242\v020\f%.2dRitter\t242\v031\f%.2d"
			   "Paladin\t242\v042\f%.2dSch}tze\t242\v053\f%.2dKleriker\t242\v064\f%.2d"
			   "Zauberer\t242\v075\f%.2dDieb\t242\v086\f%.2dNinja\t242\v097\f%.2d"
			   "Barbar\t242\v108\f%.2dDruide\t242\v119\f%.2dWaldl.\f04\x3"
			   "c"
			   "\t265\v142Talente\x3l\t223\v155%s\t223\v170%s%s"; 
	}

	const char *NAME_FOR_NEW_CHARACTER() {
		return "\x3"
			   "cBitte einen Namen eingeben.\n\n";
	}

	const char *SELECT_CLASS_BEFORE_SAVING() {
		return "\v006\x3"
			   "cErst eine Klasse w&hlen!\x3l";
	}

	const char *EXCHANGE_ATTR_WITH() {
		return "Tausche %s mit...";
	}

	const int *NEW_CHAR_SKILLS_OFFSET() {
		static const int _newCharSkillsOffset[] = { 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 };
		return _newCharSkillsOffset;
	}

	const char *COMBAT_DETAILS() {
		return "\r\f00\x3"
			   "c\v000\t000\x2"
			   "Kampf%s%s%s\x1";
	}

	const char *NOT_ENOUGH_TO_CAST() {
		return "\x3"
			   "c\v010Zu wenig %s, um %s zu sprechen";
	}

	const char **SPELL_CAST_COMPONENTS() {
		static const char *_spellCastComponents[] = { "Spruchpunkte", "Edelsteine" };
		return _spellCastComponents;
	}

	const char *CAST_SPELL_DETAILS() {
		return "\r\x2\x3"
			   "c\v122\t013\f37Z\fdaub\t040\f37N\fdeu"
			   "\t067ESC\x1\t000\v000\x3"
			   "cZaubern\n"
			   "\n"
			   "%s\x3l\n"
			   "\n"
			   "Bereit:\x3"
			   "c\n"
			   "\n"
			   "\f09%s\fd\x2\x3l\n"
			   "\v082Preis\x3r\t000%u/%u\x3l\n"
			   "Akt SP\x3r\t000%u\x1";
	}

	const char *PARTY_FOUND() {
		return "\x3"
			   "cDie Party fand:\n"
			   "\n"
			   "\x3r\t000%u Gold\n"
			   "%u Edel";
	}

	const char *BACKPACKS_FULL_PRESS_KEY() {
		return "\v007\f12Achtung! Rucks&cke sind voll!\fd\n"
			   "Taste...";
	}

	const char *HIT_A_KEY() {
		return "\x3l\v120\t000\x4"
			   "077\x3"
			   "c\f37Taste...\fd";
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
		static const char *_found[] = { "fand", "" };
		return _found;
	}

	const char *X_FOUND_Y() {
		return "\v060\t000\x3"
			   "c%s %s: %s";
	}
	
	const char *ON_WHO() {
		return "\x3"
			   "c\v009Auf wen?";
	}

	const char *WHICH_ELEMENT1() {
		return "\r\x3"
			   "c\x1Welches Element?\x2\v034\t014\f15F\fdeuer\t044"
			   "\f15E\fdlek\t074\f15K\fd&lte\t104\f15S\fd&ure\x1";
	}

	const char *WHICH_ELEMENT2() {
		return "\r\x3"
			   "cWelches Element?\x2\v034\t014\f15F\fdeuer\t044"
			   "\f15E\fdlek\t074\f15K\fd&lte\t104\f15S\fd&ure\x1";
	}

	const char *DETECT_MONSTERS() {
		return "\x3"
			   "cMonster finden";
	}

	const char *LLOYDS_BEACON() {
		return "\r\x3"
			   "c\v000\t000\x1Lloyds Signal\n"
			   "\n"
			   "Letzte Position\n"
			   "\n"
			   "%s\x3l\n"
			   "x = %d\x3r\t000y = %d\x3"
			   "c\x2\v122\t021\f15S\fdetzen\t060\f15Z\fdur}ck\x1";
	}

	const char *HOW_MANY_SQUARES() {
		return "\x3"
			   "cTeleportieren\nWieviele Felder %s (1-9)\n";
	}

	const char *TOWN_PORTAL() {
		return "\x3"
			   "cPortal\x3l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\n"
			   "\t0104. %s\n"
			   "\t0105. %s\x3"
			   "c\n"
			   "\n"
			   "Zu welcher Stadt (1-5)\n"
			   "\n";
	}

	const char *TOWN_PORTAL_SWORDS() {
		return "\x3"
			   "cPortal\x3l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\x3"
			   "c\n"
			   "\n"
			   "Zu welcher Stadt (1-3)\n"
			   "\n";
	}

	const char *MONSTER_DETAILS() {
		return "\x3l\n"
			   "%s\x3"
			   "c\t100%s\t140%u\t180%u\x3r\t000%s";
	}

	const char **MONSTER_SPECIAL_ATTACKS() {
		static const char *_monsterSpecialAttacks[] = {
			"Nichts", "Magie", "Feuer", "Elek", "K&lte", "Gift", "Energie", "Krank",
		        "Verr}ck", "Schl&ft", "FluchGg", "Verlieb", "SP weg", "Fluch", "Paralys",
				"Bewust", "Verwirr", "WafKapu", "Schwach", "Verrot", "Alt+5", "Tot", "Stein"
		};
		return _monsterSpecialAttacks;
	}

	const char *IDENTIFY_MONSTERS() {
		return "Name\x3"
			   "c\t100TP\t140RKl\t177#Angr\x3r\t000Spezial%s%s%s";
	}

	const char *MOONS_NOT_ALIGNED() {
		return "\x3"
			   "c\v012\t000Die Monde stehen nicht richtig. Der Weg zu %s ist versperrt!";
	}

	const char *AWARDS_FOR() {
		return "\r\x1\fd\x3"
			   "c\v000\t000Preise f}r %s den %s\x3l\x2\n"
			   "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\x1";
	}

	const char *AWARDS_TEXT() {
		return "\r\x2\x3"
			   "c\v021\t221\f37H\fdoch\t255\f37R\fdunter\t289Ende";
	}

	const char *NO_AWARDS() {
		return "\x3"
			   "cKeine Preise";
	}

	const char *WARZONE_BATTLE_MASTER() {
		return "Die Arena\n\t125Kampfmeister";
	}

	const char *WARZONE_MAXED() {
		return "Was? Ihr schon wieder? Vergreift euch an kleinere!";
	}

	const char *WARZONE_LEVEL() {
		return "Welche Stufe Monster? (1-10)\n";
	}

	const char *WARZONE_HOW_MANY() {
		return "Wie viele Monster? (1-20)\n";
	}

	const char *PICKS_THE_LOCK() {
		return "\x3"
			   "c\v010%s %s das Schlo@!\nTaste...";
	}

	const char **PICK_FORM() {
		static const char *_pickForm[] = { "knackt", "" };
		return _pickForm;
	}

	const char *UNABLE_TO_PICK_LOCK() {
		return "\x3"
			   "c\v010%s kann das Schlo@ nicht knacken!\nTaste..";
	}

	const char **UNABLE_TO_PICK_FORM() {
		static const char *_unableToPickForm[] = { "kann nicht knacken", "" };
		return _unableToPickForm;
	}

	const char *CONTROL_PANEL_TEXT() {
		return "\x1\f00\x3"
			   "c\v000\t000Steuermen}\x3r"
			   "\v022\t045\f06L\fdaden:\t124\f06G\fder:"
			   "\v041\t045\f06S\fdpeich:\t124\f06M\fdusik:"
			   "\v060\t045\f06E\fdnde:"
			   "\v080\t084Mr \f06W\fdizard:%s\t000\x1";
	}

	const char *CONTROL_PANEL_BUTTONS() {
		return "\x3"
			   "c\f11"
			   "\v022\t062Disk\t141%s"
			   "\v041\t062Disk\t141%s"
			   "\v060\t062DOS"
			   "\v079\t102Hilfe\fd";
	}

	const char *ON() {
		return "\f15an\f11";
	}

	const char *OFF() {
		return "\f32aus\f11";
	}

	const char *CONFIRM_QUIT() {
		return "Sicher?";
	}

	const char *MR_WIZARD() {
		return "Wollen Sie bestimmt Mr. Wizards Hilfe?";
	}

	const char *NO_LOADING_IN_COMBAT() {
		return "Nicht im Kampf!";
	}

	const char *NO_SAVING_IN_COMBAT() {
		return "Nicht im Kampf!";
	}

	const char *QUICK_FIGHT_TEXT() {
		return "\r\fd\x3"
			   "c\v000\t000Schnell- kampf Men}\n\n"
			   "%s\x3l\n\n"
			   "Aktiv\x3r\n"
			   "\t000%s\x2\x3"
			   "c\v122\t021\f37W\f04eiter\t060Ende\x1";
	}

	const char **QUICK_FIGHT_OPTIONS() {
		static const char *_quickFightOptions[] = { "Angriff", "Zauber", "Parieren", "Fliehen" };
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
			   "cSpielmodus w&hlen";
	}

	const char *SAVE_OFF_LIMITS() {
		return "\x3"
			   "c\v002\t000Die G*tter verbieten das Speichern in dieser\n"
			   "abgelegenen Gegend!";
	}
	
	const char *CLOUDS_INTRO1() {
		return "\f00\v082\t040\x3"
			   "cK*nig Burlock\v190\t040Bauern\v082\t247"
			   "Lord Xeen\v190\t258Xeens Tier\v179\t150Crodo";
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
		static const char *_maeNames[] = {
			"",
		};
		return _maeNames;
	}

	const char **CLOUDS_MIRROR_LOCATIONS() {
		static const char *_mirrLocs[] = {
			"",
		};
		return _mirrLocs;
	}

	const char **CLOUDS_MAP_NAMES() {
		static const char *_mapNames[] = {
			"",
		};
		return _mapNames;
	}

	const char **CLOUDS_MONSTERS() {
		static const char *_monsters[] = {
			"",
		};
		return _monsters;
	}

	const char **CLOUDS_SPELLS() {
		static const char *_spells[] = {
			"",
		};
		return _spells;
	}

	class DE_KeyConstants : public KeyConstants {
	public:
		class DE_DialogsCharInfo : public DialogsCharInfo {
		public:
			int KEY_ITEM()     { return Common::KEYCODE_g; }
			int KEY_QUICK()    { return Common::KEYCODE_k; }
			int KEY_EXCHANGE() { return Common::KEYCODE_t; }
		};
		DE_DialogsCharInfo *dialogsCharInfo() {
			if (!_dci)_dci = new DE_DialogsCharInfo();
			return _dci;
		}

		class DE_DialogsControlPanel : public DialogsControlPanel {
		public:
			int KEY_FXON()     { return Common::KEYCODE_g; }
			int KEY_MUSICON()  { return Common::KEYCODE_m; }
			int KEY_LOAD()     { return Common::KEYCODE_l; }
			int KEY_SAVE()     { return Common::KEYCODE_s; }
			int KEY_QUIT()     { return Common::KEYCODE_e; }
			int KEY_MRWIZARD() { return Common::KEYCODE_w; }
		};
		DE_DialogsControlPanel *dialogsControlPanel() {
			if (!_dcp) _dcp = new DE_DialogsControlPanel();
			return _dcp;
		}

		class DE_DialogsCreateChar : public DialogsCreateChar {
		public:
			int KEY_ROLL()   { return Common::KEYCODE_w; }
			int KEY_CREATE() { return Common::KEYCODE_e; }
			int KEY_MGT()    { return Common::KEYCODE_k; } 
			int KEY_INT()    { return Common::KEYCODE_i; } 
			int KEY_PER()    { return Common::KEYCODE_p; } 
			int KEY_END()    { return Common::KEYCODE_a; } 
			int KEY_SPD()    { return Common::KEYCODE_t; } 
			int KEY_ACY()    { return Common::KEYCODE_g; } 
			int KEY_LCK()    { return Common::KEYCODE_l; } 
		}; 
		DE_DialogsCreateChar *dialogsCreateChar() {
			if (!_dcc) _dcc = new DE_DialogsCreateChar();
			return _dcc;
		}

		class DE_DialogsDifficulty : public DialogsDifficulty {
		public:
			int KEY_ADVENTURER() { return Common::KEYCODE_a; }
			int KEY_WARRIOR()    { return Common::KEYCODE_k; }
		};
		DE_DialogsDifficulty *dialogsDifficulty() {
			if (!_dd) _dd = new DE_DialogsDifficulty();
			return _dd;
		}

		class DE_DialogsItems : public DialogsItems {
		public:
			int KEY_WEAPONS()   { return Common::KEYCODE_w; }
			int KEY_ARMOR()     { return Common::KEYCODE_r; }
			int KEY_ACCESSORY() { return Common::KEYCODE_c; }
			int KEY_MISC()      { return Common::KEYCODE_v; }
			int KEY_ENCHANT()   { return Common::KEYCODE_z; } 
			int KEY_USE()       { return Common::KEYCODE_a; } 
			int KEY_BUY()       { return Common::KEYCODE_k; } 
			int KEY_SELL()      { return Common::KEYCODE_e; } 
			int KEY_IDENTIFY()  { return Common::KEYCODE_i; } 
			int KEY_FIX()       { return Common::KEYCODE_p; } 
			int KEY_EQUIP()     { return Common::KEYCODE_a; } 
			int KEY_REM()       { return Common::KEYCODE_l; }
			int KEY_DISC()      { return Common::KEYCODE_g; }
			int KEY_QUEST()     { return Common::KEYCODE_u; }
			int KEY_RECHRG()    { return Common::KEYCODE_n; }
			int KEY_GOLD()      { return Common::KEYCODE_o; }
		};
		DE_DialogsItems *dialogsItems() {
			if (!_di) _di = new DE_DialogsItems();
			return _di;
		}

		class DE_DialogsParty : public DialogsParty {
		public:
			int KEY_DELETE() { return Common::KEYCODE_l; }
			int KEY_REMOVE() { return Common::KEYCODE_w; }
			int KEY_CREATE() { return Common::KEYCODE_e; }
			int KEY_EXIT()   { return Common::KEYCODE_n; }
		};
		DE_DialogsParty *dialogsParty() {
			if (!_dp) _dp = new DE_DialogsParty();
			return _dp;
		}

		class DE_DialogsQuests : public DialogsQuests {
		public:
			int KEY_QUEST_ITEMS()    { return Common::KEYCODE_g; }
			int KEY_CURRENT_QUESTS() { return Common::KEYCODE_a; }
			int KEY_AUTO_NOTES()     { return Common::KEYCODE_n; }
		};
		DE_DialogsQuests *dialogsQuests() {
			if (!_dq) _dq = new DE_DialogsQuests();
			return _dq;
		}

		class DE_DialogsQuickFight : public DialogsQuickFight {
		public:
			int KEY_NEXT() { return Common::KEYCODE_w; }
		};
		DE_DialogsQuickFight *dialogsQuickFight() {
			if (!_dqf) _dqf = new DE_DialogsQuickFight();
			return _dqf;
		}

		class DE_DialogsSpells : public DialogsSpells {
		public:
			int KEY_CAST()   { return Common::KEYCODE_z; }
			int KEY_NEW()    { return Common::KEYCODE_n; }
			int KEY_FIRE()   { return Common::KEYCODE_f; } //TODO
			int KEY_ELEC()   { return Common::KEYCODE_e; } //TODO
			int KEY_COLD()   { return Common::KEYCODE_c; } //TODO
			int KEY_ACID()   { return Common::KEYCODE_a; } //TODO
			int KEY_SET()    { return Common::KEYCODE_t; } //TODO
			int KEY_RETURN() { return Common::KEYCODE_r; } //TODO
		};
		DE_DialogsSpells *dialogsSpells() {
			if (!_ds) _ds = new DE_DialogsSpells();
			return _ds;
		}

		class DE_Locations : public Locations {
		public:
			int KEY_DEP()        { return Common::KEYCODE_l; } 
			int KEY_WITH()       { return Common::KEYCODE_a; } 
			int KEY_GOLD()       { return Common::KEYCODE_g; }
			int KEY_GEMS()       { return Common::KEYCODE_e; }
			int KEY_BROWSE()     { return Common::KEYCODE_s; }
			int KEY_BUY_SPELLS() { return Common::KEYCODE_s; } 
			int KEY_SPELL_INFO() { return Common::KEYCODE_i; } 
			int KEY_SIGN_IN()    { return Common::KEYCODE_r; }
			int KEY_DRINK()      { return Common::KEYCODE_t; } 
			int KEY_FOOD()       { return Common::KEYCODE_n; } 
			int KEY_TIP()        { return Common::KEYCODE_i; } 
			int KEY_RUMORS()     { return Common::KEYCODE_g; } 
			int KEY_HEAL()       { return Common::KEYCODE_h; } 
			int KEY_DONATION()   { return Common::KEYCODE_s; } 
			int KEY_UNCURSE()    { return Common::KEYCODE_f; } 
			int KEY_TRAIN()      { return Common::KEYCODE_t; } 
		};
		DE_Locations *locations() {
			if (!_l) _l = new DE_Locations();
			return _l;
		}

		class DE_CloudsOfXeenMenu : public CloudsOfXeenMenu {
		public:
			int KEY_START_NEW_GAME() { return Common::KEYCODE_s; }
			int KEY_LOAD_GAME()      { return Common::KEYCODE_l; }
			int KEY_SHOW_CREDITS()   { return Common::KEYCODE_c; }
			int KEY_VIEW_ENDGAME()   { return Common::KEYCODE_e; }
		};
		DE_CloudsOfXeenMenu *cloudsOfXeenMenu() {
			if (!_soxm)
				_soxm = new DE_CloudsOfXeenMenu();
			return _soxm;
		}

	private:
		DE_DialogsCharInfo     *_dci = NULL;
		DE_DialogsControlPanel *_dcp = NULL;
		DE_DialogsCreateChar   *_dcc = NULL;
		DE_DialogsDifficulty   *_dd = NULL;
		DE_DialogsItems        *_di = NULL;
		DE_DialogsParty        *_dp = NULL;
		DE_DialogsQuests       *_dq = NULL;
		DE_DialogsQuickFight   *_dqf = NULL;
		DE_DialogsSpells       *_ds = NULL;
		DE_Locations           *_l = NULL;
		DE_CloudsOfXeenMenu    *_soxm = NULL;
	};

	DE_KeyConstants *keyConstants() {
		if (!_kc)_kc = new DE_KeyConstants();
		return _kc;
	}

private:
	DE_KeyConstants *_kc = NULL;
};
