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

#include "common/debug.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/triggers.h"
#include "m4/riddle/console.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/other.h"
#include "m4/core/errors.h"
#include "m4/console.h"

namespace M4 {
namespace Riddle {

RiddleEngine::RiddleEngine(OSystem *syst, const M4GameDescription *gameDesc) :
		M4Engine(syst, gameDesc) {
	_sections.push_back(&_section1);
	_sections.push_back(&_section2);
	_sections.push_back(&_section3);
	_sections.push_back(&_section4);
	_sections.push_back(&_section5);
	_sections.push_back(&_section6);
	_sections.push_back(&_section7);
	_sections.push_back(&_section8);
	_sections.push_back(&_section9);
}

M4::Vars *RiddleEngine::createVars() {
	return new Riddle::Vars();
}

void RiddleEngine::setupConsole() {
	setDebugger(new Riddle::Console());
}

void RiddleEngine::showEngineInfo() {
	debug("The Riddle of Master Lu\n");
	debug("Game Version %s -- %s\n", "2.05", "Dec 14, 1995");
	debug("%s\n", "M4 Runtime by Nick, Tinman, Mike, Xi, Andras, Paul (the fifth Beatle)");
	debug("M4 Library Version %s -- %s\n", "v1.213", "September 8, 1995");
	debug("%s.\n\n", "Copyright (c) 1995 by Sanctuary Woods Multimedia Corporation");
}

void RiddleEngine::syncFlags(Common::Serializer &s) {
	g_vars->_flags.sync(s);
}

void RiddleEngine::global_daemon() {
	_G(i_just_hyperwalked) = false;

	if (_G(kernel).trigger == kGOTO_MAIN_MENU)
		_G(game).setRoom(494);
}

void RiddleEngine::global_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");
	bool npcFlag = player_said("MEI CHEN") ||
		player_said("FENG LI") ||
		player_said("TWELVETREES") ||
		player_said("WOLF") ||
		player_said("PERSON IN PIT") ||
		player_said("TWELVETREES ") ||
		player_said("OLD WOMAN") ||
		player_said("OLD LADY") ||
		player_said("AGENT") ||
		player_said("BUTLER") ||
		player_said("ACOLYTE") ||
		player_said("OFFICIAL") ||
		player_said("GATEKEEPER") ||
		player_said("PEASANT") ||
		player_said("ACOLYTE") ||
		player_said("MENENDEZ") ||
		player_said("GUARD") ||
		player_said("FARMER");
	bool combineFlag = inv_player_has(_G(player).verb) &&
		inv_player_has(_G(player).noun);
	bool splitFlag = useFlag && inv_player_has(_G(player).noun);

	if (npcFlag && inv_player_has(_G(player).verb)) {
		digi_play("com017", 1);
	} else if (npcFlag && player_said("TURTLE")) {
		digi_play("305r19b", 1);

	} else if (combineFlag && player_said("WHALE BONE HORN", "PULL CORD")) {
		combineItems("HORN/PULL CORD");
	} else if (combineFlag && player_said("CIGAR WITHOUT BAND", "CIGAR BAND")) {
		combineItems("CIGAR");
	} else if (combineFlag && player_said("WOODEN LADDER", "ROPE")) {
		combineItems("LADDER/ROPE");
	} else if (combineFlag && player_said("GREEN VINE", "WOODEN LADDER")) {
		combineItems("LADDER/GREEN VINE");
	} else if (combineFlag && player_said("GREEN VINE", "WOODEN LADDER")) {
		combineItems("LADDER/GREEN VINE");
	} else if (combineFlag && player_said("BROWN VINE", "WOODEN LADDER")) {
		combineItems("LADDER/BROWN VINE");
	} else if (combineFlag && player_said("VINES", "WOODEN LADDER")) {
		combineItems("LADDER/VINES");
	} else if (combineFlag && player_said("BROWN VINE", "GREEN VINE")) {
		combineItems("VINES");

	} else if (takeFlag && player_said_any("statue", "statue ")) {
		digi_play("608r22", 1);
	} else if (splitFlag && player_said("HORN/PULL CORD")) {
		splitItems("WHALE BONE HORN", "PULL CORD");
	} else if (splitFlag && player_said("LADDER/ROPE")) {
		splitItems("WOODEN LADDER", "ROPE");
	} else if (splitFlag && player_said("CIGAR")) {
		splitItems("CIGAR BAND", "CIGAR WITHOUT BAND");
	} else if (splitFlag && player_said("LADDER/GREEN VINE")) {
		splitItems("WOODEN LADDER", "GREEN VINE");
	} else if (splitFlag && player_said("LADDER/BROWN VINE")) {
		splitItems("WOODEN LADDER", "BROWN VINE");
	} else if (splitFlag && player_said("ENVELOPE")) {
		splitItems("VON SELTSAM'S NOTE", "POSTAGE STAMP");
		kernel_examine_inventory_object("PING VON SELTSAM'S NOTE",
			5, 1, 270, 150, 10000, "406R18C");
	} else if (splitFlag && player_said("LADDER/VINES")) {
		splitItems("WOODEN LADDER", "VINES");
	} else if (splitFlag && player_said("VINES")) {
		splitItems("GREEN VINE", "BROWN VINE");
	} else if (splitFlag && player_said("POST MARKED ENVELOPE")) {
		inv_move_object("POST MARKED ENVELOPE", NOWHERE);
		inv_give_to_player("VON SELTSAM'S LETTER");
		Common::strcpy_s(_G(player).noun, "VON SELTSAM'S LETTER");

	} else if (player_said_any("LIGHTER", "LIT LIGHTER") && player_said("TURTLE")) {
		digi_play("com090a", 1);
	} else if (useFlag && HAS("LIGHTER")) {
		inv_move_object("LIGHTER", NOWHERE);
		inv_give_to_player("LIT LIGHTER");
		digi_play("604_s01", 2);
	} else if (useFlag && HAS("LIT LIGHTER")) {
		inv_move_object("LIT LIGHTER", NOWHERE);
		inv_give_to_player("LIGHTER");
	} else if (useFlag && player_said_any("URN", "URN ") &&
			_G(game).room_id != 802 && _G(game).room_id != 403) {
		digi_play("COM072", 1, 255, -1, 997);
	} else if (useFlag && player_said("UNLIT URN")) {
		digi_play("COM073", 1, 255, -1, 997);

	} else if (!player_said("journal", " ") &&
			!player_said("journal", "tower") &&
			!player_said("journal", "tower ") &&
			!player_said("journal", "dome") &&
			!player_said("journal", "observatory dome") &&
			!player_said("journal", "terrace") &&
			!takeFlag && !lookFlag && inv_player_has(_G(player).noun) &&
			_G(game).room_id >= 504 && _G(game).room_id <= 510 &&
			_G(game).room_id != 507) {
		if (_G(flags)[kMocaMocheCartoon]) {
			digi_play("COM029", 1, 255, -1, 997);
		} else {
			if (_G(kernel).trigger == 6) {
				_G(flags)[V089] = 1;
				_G(flags)[kMocaMocheCartoon] = 1;
			}
			
			sendWSMessage_multi("COM028");
		}
	} else if ((player_said("journal", " ") || player_said("journal", "temple")) &&
			!takeFlag && !lookFlag && !inv_player_has(_G(player).noun) &&
			_G(game).room_id >= 702 && _G(game).room_id <= 799) {
		if (_G(flags)[kTempleCartoon]) {
			digi_play("com033", 1);
		} else {
			if (_G(kernel).trigger == 6) {
				_G(flags)[kTempleCartoon] = 1;
				_G(flags)[V089] = 1;
			}

			sendWSMessage_multi("com032");
		}
	} else if (player_said("journal") && player_said_any(" ", "lava") &&
			!takeFlag && !lookFlag && !inv_player_has(_G(player).noun) &&
			_G(game).room_id >= 601 && _G(game).room_id <= 699) {
		if (_G(flags)[kEasterIslandCartoon]) {
			digi_play("203r54", 1);
		} else {
			if (_G(kernel).trigger == 6) {
				_G(flags)[kEasterIslandCartoon] = 1;
				_G(flags)[V089] = 1;
			}

			sendWSMessage_multi("605r13");
		}
	} else if ((player_said("journal", "romanov emerald") ||
			player_said("journal", "emerald/cork")) &&
			!takeFlag && !lookFlag) {
		if (_G(flags)[kEmeraldCartoon]) {
			digi_play("407r33", 1);
		} else {
			if (_G(kernel).trigger == 6) {
				_G(flags)[kEmeraldCartoon] = 1;
				_G(flags)[V089] = 1;
			}

			sendWSMessage_multi(nullptr);
		}
	} else if (player_said("journal", "rongorongo tablet") &&
			!takeFlag && !lookFlag) {
		if (_G(flags)[V288]) {
			digi_play("com086", 1, 255, -1, 997);
		} else {
			if (_G(kernel).trigger == 6) {
				_G(flags)[V288] = 1;
			}

			sendWSMessage_multi(nullptr);
		}

	} else if (lookFlag && player_said("JOURNAL")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1111);
			break;
		case 1111:
			other_save_game_for_resurrection();
			_G(game).setRoom(996);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("MEI CHEN")) {
		digi_play("com043", 1);
	} else if (lookFlag && player_said("feng li")) {
		digi_play("com139", 1, 255, -1, 997);
	} else if ((player_said("SHRUNKEN HEAD") ||
			player_said("INCENSE BURNER") ||
			player_said("CRYSTAL SKULL") ||
			player_said("WHALE BONE HORN") ||
			player_said("WHEELED TOY") ||
			player_said("SILVER BUTTERFLY") ||
			player_said("REBUS AMULET") ||
			player_said("CHISEL") ||
			player_said("GERMAN BANKNOTE") ||
			player_said("POSTAGE STAMP") ||
			player_said("STICK AND SHELL MAP") ||
			player_said("ROMANOV EMERALD")) &&
			player_said("agent")) {
		digi_play("com021", 1, 255, -1, 997);
	} else if (!lookFlag && !takeFlag && !useFlag && !talkFlag &&
			player_said("AGENT")) {
		if (player_said_any("US DOLLARS", "CHINESE YUAN", "POMERANIAN MARKS",
			"PERUVIAN INTI", "GERMAN BANKNOTE", "SIKKIMESE RUPEE")) {
			digi_play("COM012", 1, 255, -1, 997);
		} else {
			digi_play("COM017", 1, 255, -1, 997);
		}
	} else if ((lookFlag || useFlag) && player_said("MESSAGE LOG")) {
		switch (_G(kernel).trigger) {
		case -1:
			showMessageLog(7777);
			break;
		case 1:
			player_update_info();
			_ripAction = series_load("RIP TREK READS TELEGRAM POS5");
			setGlobals1(_ripAction, 1, 20, 20, 31, 2);
			sendWSMessage_110000(2);
			break;
		case 2:
			if (_G(player).walker_in_this_scene && _G(flags)[V292])
				sendWSMessage_190000(12);

			if (_G(flags)[V349] < 14) {
				static const char *DIGI[14] = {
					"201r26", "301r18", "301r19", "301r20", "301r21",
					"201r61c","401R31", "401R37", "401r38", "401r39",
					"501R02", "501R03", "701R39", "401R36"
				};
				digi_play(DIGI[_G(flags)[V349]], 1, 255, 3);
			}
			break;
		case 3:
			if (_G(player).walker_in_this_scene && _G(flags)[V292]) {
				sendWSMessage_140000(5);
				sendWSMessage_190000(5);
			} else {
				_G(flags)[V052] = 0;
				player_set_commands_allowed(true);
			}
			break;
		case 5:
			series_unload(_ripAction);
			_G(flags)[V052] = 0;
			player_set_commands_allowed(true);
			break;
		case 7777:
			if (_messageLog._result != 16) {
				_G(flags)[V052] = 1;

				if (_G(player).walker_in_this_scene && _G(flags)[292]) {
					player_update_info();

					switch (_G(player_info).facing) {
					case 1:
					case 2:
					case 3:
					case 4:
						ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 5, 1);
						break;
					case 5:
					case 7:
						kernel_timing_trigger(1, 1);
						player_set_commands_allowed(false);
						break;
					case 8:
					case 9:
					case 10:
					case 11:
						ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 7, 1);
						break;
					default:
						player_set_commands_allowed(false);
						break;
					}
				} else {
					kernel_timing_trigger(1, 2);
				}
			}
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("WHALE BONE HORN")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(player).walker_in_this_scene && _G(flags)[V292]) {
				player_update_info();

				switch (_G(player_info).facing) {
				case 1:
					ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 2, 1);
					break;
				case 2:
				case 3:
				case 9:
				case 10:
					kernel_timing_trigger(1, 1);
					break;
				case 4:
				case 5:
					ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 3, 1);
					break;
				case 7:
				case 8:
					ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 9, 1);
					break;
				case 11:
					ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 10, 1);
					break;
				default:
					break;
				}

				player_set_commands_allowed(false);

			} else {
				digi_play("950HORN", 1);
			}
			break;
		case 1:
			player_update_info();
			_savedFacing = _G(player_info).facing;

			if (_savedFacing == 2 || _savedFacing == 10) {
				_ripAction = series_load("RIP HORN BLOW POS2");
				setGlobals1(_ripAction, 1, 23, 24, 24, 0, 25, 33, 34, 34, 0, 35, 48, 48, 48);
				sendWSMessage_110000(2);
			} else {
				_ripAction = series_load("RIP HORN BLOW POS3");
				setGlobals1(_ripAction, 1, 23, 24, 24, 0, 25, 29, 30, 30, 0, 31, 42, 42, 42);
				sendWSMessage_110000(2);
			}
			break;
		case 2:
			sendWSMessage_120000(3);
			sendWSMessage_190000((_savedFacing == 3 || _savedFacing == 9) ? 40 : 20);
			digi_play("950HORN", 1);
			break;
		case 3:
			sendWSMessage_130000(4);
			sendWSMessage_190000(5);
			break;
		case 4:
			sendWSMessage_150000(5);
			break;
		case 5:
			series_unload(_ripAction);

			if (_G(game).room_id != 604 && _G(game).room_id != 610)
				player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (_G(kernel).trigger == kINVENTORY_CLOSEUP_END) {
		if (player_said_any("MENENDEZ'S LETTER", "VON SELTSAM'S LETTER",
				"ENVELOPE", "VON SELTSAM'S NOTE"))
			digi_stop(1);
		player_set_commands_allowed(true);
	} else if (_G(kernel).trigger == 10000) {
		player_set_commands_allowed(false);
		kernel_timing_trigger(1, 10001);
	} else if (_G(kernel).trigger == 10001) {
		player_set_commands_allowed(true);
		digi_stop(1);
		kernel_examine_inventory_object("PING POSTAGE STAMP", 5, 1,
			270, 150, 990, "406R19");
	} else if (useFlag && HAS("TWELVETREES' NOTE")) {
		inv_move_object("TWELVETREES' NOTE", NOWHERE);
		inv_give_to_player("TWELVETREES' MAP");
		Common::strcpy_s(_G(player).noun, "TWELVETREES' MAP");
	} else if (lookFlag && player_said("TOWER")) {
		digi_play("COM135", 1, 255, -1, 997);
	} else if (lookFlag && inv_player_has(_G(player).noun)) {
		lookAtInventoryItem();
	} else if (player_said("LIT LIGHTER", "CIGAR WITHOUT BAND") ||
			player_said("LIGHTER", "CIGAR WITHOUT BAND") ||
			player_said("LIGHTER", "CIGAR") ||
			player_said("LIT LIGHTER", "CIGAR") ||
			(useFlag && player_said("CIGAR WITHOUT BAND"))) {
		digi_play("com100", 1);
	} else if (inv_player_has(_G(player).noun) && inv_player_has(_G(player).verb)) {
		if (player_said("journal")) {
			digi_play("com085", 1, 255, -1, 997);
		} else {
			digi_play("207R12", 1);
		}
	} else if (player_said("journal") && !takeFlag && !lookFlag &&
			!inv_player_has(_G(player).noun)) {
		switch (_G(game).room_id) {
		case 202:
			digi_play("202r16", 1);
			break;
		case 203:
			digi_play("203r33", 1);
			break;
		case 301:
			digi_play("310r12", 1);
			break;
		case 303:
			digi_play("303R43", 1);
			break;
		case 304:
			digi_play("304r78", 1);
			break;
		case 305:
			digi_play("305r38", 1);
			break;
		default:
			digi_play("com014", 1);
			break;
		}
	} else if (lookFlag && _G(player).click_y <= 374 &&
			inv_player_has(_G(player).noun)) {
		switch (_G(game).room_id) {
		case 301:
			digi_play(_G(player).been_here_before ? "301r13" : "301r02", 1);
			break;
		case 303:
			digi_play("303r23", 1);
			break;
		case 304:
			digi_play(_G(flags)[V001] ? "304r02" : "304r55", 1);
			break;
		case 305:
			digi_play("305r30", 1);
			break;
		case 605:
			digi_play("605r02", 1);
			break;
		default:
			break;
		}
	} else if (takeFlag && !inv_player_has(_G(player).noun)) {
		static const char *DIGI[5] = {
			"207r09", "207r10", "207r11", "207r38", "207r39"
		};
		digi_play(DIGI[imath_ranged_rand(0, 4)], 1);
	} else if (!player_said("walk to")) {
		switch (_G(game).room_id) {
		case 407: {
			static const char *DIGI[6] = {
				"com006", "com011", "com018", "com019", "com096", "com129"
			};
			digi_play(DIGI[imath_ranged_rand(0, 5)], 1);
			break;
		}
		case 604:
		case 610:
			if (_G(flags)[V203] == 8 &&
				(inv_player_has(_G(player).verb) || inv_player_has(_G(player).noun)) &&
				(!useFlag || !inv_player_has(_G(player).noun))
			) {
				digi_play(_G(game).room_id == 610 ? "610r16" : "604r33", 1);
			} else {
				static const char *DIGI[5] = {
					"301r23", "301r26", "301r25", "301r35", "301r36"
				};
				digi_play(DIGI[imath_ranged_rand(0, 4)], 1);
			}
			break;
		default: {
			static const char *DIGI[5] = {
				"301r23", "301r26", "301r25", "301r35", "301r36"
			};
			digi_play(DIGI[imath_ranged_rand(0, 4)], 1);
			break;
		}
		}
	}
}

void RiddleEngine::combineItems(const char *newItem) {
	inv_move_object(_G(player).verb, NOWHERE);
	inv_move_object(_G(player).noun, NOWHERE);
	inv_give_to_player(newItem);
}

void RiddleEngine::splitItems(const char *item1, const char *item2) {
	inv_move_object(_G(player).noun, NOWHERE);
	inv_give_to_player(item1);
	inv_give_to_player(item2);
}

void RiddleEngine::showMessageLog(int trigger) {
	// TODO
	warning("TODO: showMessageLog");
}

void RiddleEngine::lookAtInventoryItem() {
	if (_G(kernel).trigger != -1)
		return;

	const int f201 = _G(flags)[V201];
	const int f207 = _G(flags)[V207];
	const int f208 = _G(flags)[V208];
	const int f209 = _G(flags)[V209];

	Common::String str = "PING ";
	str += _G(player).noun;
	const char *digi = nullptr;

	if (player_said("HORN/PULL CORD"))
		digi = "COM117";
	else if (player_said("HORN/PULL CORD/WATER"))
		digi = "COM116";
	else if (player_said("MENENDEZ'S LETTER"))
		digi = "407r41";
	else if (player_said("VON SELTSAM'S LETTER"))
		digi = "303r10";
	else if (player_said("VON SELTSAM'S NOTE"))
		digi = "406R18";
	else if (player_said("POSTAGE STAMP"))
		digi = "406R19";
	else if (player_said("TOMB MESSAGE")) {
		if (_G(flags)[V031]) {
			digi = "406R18";
		} else {
			digi = "406R18C";
			_G(flags)[V031] = 1;
		}
	} else if (player_said("TWELVETREES' MAP")) {
		if (!_G(flags)[V037]) {
			_G(flags)[V037] = 1;
			digi = player_been_here(623) ? "603r30" : "603r31";
		}

		str = "PING OBJ1";

		if (!f201 && !f207 && !f209 && !f208)
			str += "36";
		else if (f201 && !f207 && !f209 && !f208)
			str += "22";
		else if (!f201 && f207 && !f209 && !f208)
			str += "23";
		else if (!f201 && !f207 && f209 && !f208)
			str += "24";
		else if (!f201 && !f207 && !f209 && f208)
			str += "25";
		else if (f201 && f207 && !f209 && !f208)
			str += "26";
		else if (f201 && !f207 && !f209 && f208)
			str += "27";
		else if (f201 && !f207 && f209 && !f208)
			str += "28";
		else if (!f201 && f207 && f209 && !f208)
			str += "29";
		else if (!f201 && f207 && !f209 && f208)
			str += "30";
		else if (!f201 && !f207 && f209 && f208)
			str += "31";
		else if (f201 && f207 && !f209 && f208)
			str += "32";
		else if (f201 && !f207 && f209 && f208)
			str += "33";
		else if (f201 && f207 && f209 && !f208)
			str += "34";
		else if (!f201 && f207 && f209 && f208)
			str += "35";
		else if (f201 && f207 && f209 && f208)
			str = "PING TWELVETREES' MAP";
	}

	kernel_examine_inventory_object(str.c_str(), 5, 1, 270, 150, kINVENTORY_CLOSEUP_END, digi);
}

} // namespace Riddle
} // namespace M4
