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
#include "m4/riddle/console.h"
#include "m4/riddle/vars.h"
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
	}

	// TODO: More stuff
	else if (talkFlag) {

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

void RiddleEngine::sendWSMessage_multi(const char *name) {
	Rooms::Room *room = static_cast<Rooms::Room *>(_activeRoom);
	room->sendWSMessage_multi(name);
}

} // namespace Riddle
} // namespace M4
