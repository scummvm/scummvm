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
			_ripReadTelegram = series_load("RIP TREK READS TELEGRAM POS5");
			setGlobals1(_ripReadTelegram, 1, 20, 20, 31, 2);
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
			if (_G(player).walker_in_this_scene) {
				// TODO
			}
			break;
		default:
			break;
		}
	}


	// TODO: More stuff
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

} // namespace Riddle
} // namespace M4
