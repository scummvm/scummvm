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

#include "m4/burger/rooms/section4/section4.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Section4::PARSER[] = {
	"DEED",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	nullptr,
	PARSER_ITEM_END,
	"JAWZ O' LIFE",
	"TAKE",
	nullptr,
	"BARRED WINDOW",
	"404w102",
	"BARRED WINDOW ",
	"404w102",
	"WILBUR",
	"400w001",
	nullptr,
	nullptr,
	PARSER_ITEM_END,
	"FISH",
	"LOOK AT",
	"400w100",
	"TAKE",
	nullptr,
	"MUFFIN",
	"405w101",
	"WILBUR",
	"400w101",
	nullptr,
	"400w102",
	PARSER_ITEM_END,
	"DOG COLLAR",
	"LOOK AT",
	"400w103",
	"TAKE",
	nullptr,
	"PEGLEG  ",
	"406w101",
	"WILBUR",
	"400w104",
	nullptr,
	"400W105",
	PARSER_ITEM_END,
	"HOOK",
	"LOOK AT",
	"400w106",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	"400w107",
	PARSER_ITEM_END,
	"RECORDS",
	"LOOK AT",
	"400w108",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	"400w109",
	PARSER_ITEM_END,
	"QUARTER",
	"LOOK AT",
	"400w110",
	"TAKE",
	nullptr,
	"WILBUR",
	"400w001",
	nullptr,
	"400w111",
	PARSER_ITEM_END,
	"DISC",
	"LOOK AT",
	"400w002",
	"TAKE",
	"400w003",
	nullptr,
	"400w004",
	PARSER_ITEM_END,
	"DISC ",
	"LOOK AT",
	"400w002",
	"TAKE",
	"400w003",
	nullptr,
	"400w004",
	PARSER_ITEM_END,
	nullptr
};

const Section4::TeleportEntry Section4::TELEPORTS[] = {
	{ 402, 407, 406 },
	{ 405, 407, 406 },
	{ 406, 402, 405 },
	{ 407, 402, 405 },
	{   0,   0,   0 }
};


Section4::Section4() : Rooms::Section() {
	add(401, &_room401);
	add(402, &_room402);
	add(404, &_room404);
	add(405, &_room405);
	add(406, &_room406);
	add(407, &_room407);
}

void Section4::init() {
	if (_G(executing) != WHOLE_GAME) {
		inv_give_to_player("JAWZ O' LIFE");
		inv_give_to_player("DEED");
	}
}

void Section4::daemon() {
	switch (_G(kernel).trigger) {
	case 4001:
		_G(game).new_room = 401;
		break;

	case 4002:
		_G(game).new_room = 402;
		break;

	case 4003:
		_G(game).new_room = 404;
		break;

	case 4004:
		_G(game).new_room = 405;
		break;

	case 4005:
		_G(game).new_room = 406;
		break;

	case 4006:
		_G(game).new_room = 407;
		break;

	case 4007:
		term_message("Teleporting...");
		term_message(teleport() ? "...scuccessful" : "ERROR while teleporting!");
		break;

	case 4008:
		player_set_commands_allowed(!checkOrderWindow());
		break;

	case kCHANGE_WILBUR_ANIMATION:
		if (_G(wilbur_should) == 10015) {
			kernel_trigger_dispatch_now(10027);
		} else {
			_G(kernel).continue_handling_trigger = true;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Section4::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_parser(PARSER))
		_G(player).command_ready = false;
}

void Section4::poof(int trigger) {
	player_set_commands_allowed(false);
	digi_preload("999_003");
	player_update_info();

	if (_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
		ws_hide_walker();
		series_play("999poof", _G(player_info).depth, 0, trigger, 6, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
	} else {
		kernel_timing_trigger(120, trigger);
	}

	digi_play("999_003", 2);

	if (!_G(flags)[V154] || imath_rand_bool(8)) {
		digi_preload("402w005z");
		digi_play("402w005z", 1);
		_G(flags)[V154] = 1;
	}
}

bool Section4::checkOrderWindow() {
	if (_G(flags)[V168] && _G(flags)[V181] && _G(flags)[V176] && _G(flags)[V159]) {
		player_set_commands_allowed(false);
		_G(flags)[kFifthTestPassed] = 1;
		disable_player_commands_and_fade_init(k10027);
		return true;
	} else {
		return false;
	}
}

bool Section4::teleport() {
	const TeleportEntry *te;
	for (te = TELEPORTS; te->_room && te->_room != _G(game).room_id; ++te) {
	}

	if (te->_room) {
		if (player_said("DISC")) {
			_G(game).new_room = te->_newRoom1;
			term_message("...%d...", te->_newRoom1);
			return true;
		} else if (player_said("DISC ")) {
			_G(game).new_room = te->_newRoom2;
			term_message("...%d...", te->_newRoom2);
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
