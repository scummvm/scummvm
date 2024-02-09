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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/burger/rooms/section5/room509.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room509::SAID[][4] = {
	{ "HALLWAY",            nullptr, "500w001",   nullptr   },
	{ "CHRISTMAS LIGHTS  ", "509w002", nullptr,   nullptr   },
	{ "PHONE CORD",         "509w005", "509w006", "509w007" },
	{ "HOLE",               "509w008", "500w001", "509w009" },
	{ "ROOF BORK",          nullptr,   nullptr,   nullptr   },
	{ "BED",                "509w010", nullptr,   "509w011" },
	{ "MATTRESS",           "509w010", nullptr,   "509w011" },
	{ "PILLOW",             "509w012", "509w013", "509w013" },
	{ "LAMP",               "509w014", "509w015", "509w014" },
	{ "WINDOW",             "500w003", nullptr,   "500w004" },
	{ "CHAIR",              "509w016", nullptr,   "509w017" },
	{ "TELEPHONE",          "509w018", "509w019", "509w020" },
	{ "BOOK",               "509w021", "509w022", "509w023" },
	{ "PICTURE",            "509w024", "500w005", nullptr   },
	{ "PICTURE ",           "509w025", "500w005", "509w026" },
	{ "PICTURE  ",          "509w027", "500w005", "509w026" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room509::PLAY1[] = {
	{  0, 14, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 18, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY3[] = {
	{  0, 15, nullptr,   0,   0, -1, 0,  0, nullptr,  0 },
	{ 16, -1, nullptr,   0,   0,  3, 0,  0, nullptr,  0 },
	{ 16, -1, "500w086", 1, 255, -1, 4, -1, &_state1, 1 },
	{ 16, -1, nullptr,   0,   0, -1, 4,  2, &_state1, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY4[] = {
	{  0, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  0, 15, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 16, -1, "509_003", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY7[] = {
	{ 0, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 0, -1, "509_002", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room509::PLAY8[] = {
	{ 0,  4, "508b003", 2, 255, -1, 0, 0, nullptr,  0 },
	{ 5, -1, "500_005", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 5, -1, "500_006", 2, 255, -1, 0, 0, &_state2, 2 },
	PLAY_BREAK_END
};

static const RGB8 PALETTE[1] = { { 0x6E, 0x46, 0x1E } };

int32 Room509::_state1;
int32 Room509::_state2;


Room509::Room509() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
}

void Room509::init() {
	Section5Room::init();
	pal_cycle_init(109, 124, 6);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (inv_player_has("CHRISTMAS LIGHTS") || inv_player_has("CHRISTMAS LIGHTS ")) {
			disable_player();
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 505:
		ws_demand_location(4, 286, 3);
		ws_unhide_walker();

		if (player_been_here(509)) {
			ws_walk(272, 325, nullptr, -1);
		} else {
			ws_walk(272, 325, nullptr, 1);
		}
		break;

	default:
		ws_demand_location(272, 325);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;
	}

	_state1 = 0;

	if (_G(flags)[V227]) {
		_val1 = 17;
	} else {
		digi_preload("508b001a");
		digi_preload("508b001b");
		digi_preload("508b001c");
		digi_preload("508b001d");
		_val1 = 15;
	}

	kernel_trigger_dispatch_now(2);

	if (_G(flags)[V229] == 0 && !inv_player_has("CHRISTMAS LIGHTS") &&
			!inv_player_has("CHRISTMAS LIGHTS ")) {
		hotspot_set_active("CHRISTMAS LIGHTS  ", true);
		series_load("509wi05");
		series_load("509wi06");
		series_load("509wi07");
		series_load("509wi08");

		_lights = series_show((_G(flags)[V212] == 5000) ? "509lgt02" : "509lgt01", 0xc00);			
	} else {
		hotspot_set_active("CHRISTMAS LIGHTS  ", false);
	}

	_wire = series_show("509wire", 0x900);

	_initFlag = true;
	Section5Room::init();
}

void Room509::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		wilbur_speech("509w001");
		break;

	case 2:
		switch (_val1) {
		case 15:
			_state2 = imath_ranged_rand(1, 2);
			_val1 = 16;
			series_play_with_breaks(PLAY8, "509bk01", 0xf00, 2, 2, 6);
			break;

		case 16:
			_val1 = 15;
			series_show("509bk01", 0xf00, 0, 2, imath_ranged_rand(120, 240));
			digi_play_loop(Common::String::format("508b001%c",
				'a' + imath_ranged_rand(0, 3)).c_str(), 2, 255);
			break;

		case 17:
			series_show("509bk01", 0xF00);
			break;

		default:
			break;
		}
		break;

	case 3:
		if (!_G(flags)[V197] && !_flag1) {
			_state1 = 1;
			digi_play("509_001", 2);
			_flag1 = true;
		} else if (!_flag1) {
			digi_play("509_003", 2, 255, -1, 509);
			_flag1 = true;
		}

		_G(flags)[V197] = 1;
		DAC_tint_range(PALETTE, _val4, 32, 255, true);
		_val3 = imath_ranged_rand(0, 1);

		if (_val3)
			_val4 += 7;
		else
			_val4 -= 7;
		_val4 = CLIP(_val4, 0, 70);

		kernel_timing_trigger(6, (_val5 == 1) ? 3 : 4);
		break;

	case 4:
		_flag1 = false;
		_val4 -= 7;
		if (_val4 < 0) {
			_val4 = _val5 = 0;
		}

		DAC_tint_range(PALETTE, _val4, 32, 255, true);

		if (_val5 == 2)
			kernel_timing_trigger(6, 4);
		break;



	case 5002:
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 2;

			terminateMachineAndNull(_lights);
			series_play_with_breaks(PLAY1, (_G(flags)[V212] == 5000) ? "509wi03" : "509wi01",
				0xc00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 2:
			hotspot_set_active("CHRISTMAS LIGHTS  ", false);

			if (_G(flags)[V212] == 5000) {
				_general.show("509wi03", 0xc00, 0, -1, -1, 31);
				inv_give_to_player("CHRISTMAS LIGHTS ");
			} else {
				_general.show("509wi01", 0xc00, 0, -1, -1, 31);
				inv_give_to_player("CHRISTMAS LIGHTS");
			}

			player_set_commands_allowed(true);
			break;

		case 3:
			hotspot_set_active("CHRISTMAS LIGHTS  ", true);
			player_set_commands_allowed(false);
			inv_move_object("CHRISTMAS LIGHTS", NOWHERE);
			inv_move_object("CHRISTMAS LIGHTS ", NOWHERE);
			_G(wilbur_should) = 14;

			series_play_with_breaks(PLAY2, (_G(flags)[V212] == 5000) ? "509wi04" : "509wi02",
				0xc00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			_general.terminate();
			_G(wilbur_should) = 5;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 5:
			player_set_commands_allowed(false);
			terminateMachineAndNull(_wire);

			if (_G(flags)[V212] == 5000) {
				if (_G(flags)[V234]) {
					_G(wilbur_should) = 6;
					_val5 = 1;
					series_play_with_breaks(PLAY3, "509wi05", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
				} else {
					_G(wilbur_should) = 7;
					series_play_with_breaks(PLAY4, "509wi05", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
				}
			} else {
				_G(wilbur_should) = 8;
				series_play_with_breaks(PLAY5, "509wi07", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);				
			}
			break;

		case 6:
			_val5 = 2;
			hotspot_set_active("CHRISTMAS LIGHTS  ", true);
			_G(wilbur_should) = 8;
			series_play_with_breaks(PLAY6, "509wi08", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 7:
			_G(wilbur_should) = 5004;
			series_play_with_breaks(PLAY7, "509wi06", 0xc00, 5016, 3);
			break;

		case 8:
			if (_state1) {
				_G(wilbur_should) = 10;
			} else if (_G(flags)[V212] == 5001) {
				_G(wilbur_should) = 11;
			} else {
				_G(wilbur_should) = 10001;
			}

			inv_move_object("CHRISTMAS LIGHTS", NOWHERE);
			inv_move_object("CHRISTMAS LIGHTS ", NOWHERE);
			hotspot_set_active("CHRISTMAS LIGHTS  ", true);
			_wire = series_show("509wire", 0x900);
			_lights = series_show((_G(flags)[V212] == 5000) ? "509lgt02" : "509lgt01", 0xc00);
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 9:
			_G(wilbur_should) = 6;
			break;

		case 10:
			_state1 = 0;
			ws_unhide_walker();
			wilbur_speech("500w087", 5001);
			break;

		case 11:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			wilbur_speech("500w088");
			break;

		case 12:
			player_set_commands_allowed(false);
			_G(wilbur_should) = 13;

			switch (_val2) {
			case 0:
				digi_play("500w079", 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			case 1:
				digi_play("500w080", 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			case 2:
				digi_play("500w089", 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			case 3:
				digi_play("500w090", 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			case 4:
				digi_play("500w091", 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			case 5:
				_G(wilbur_should) = 4;
				digi_play("500w084", 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			case 6:
				_G(wilbur_should) = 4;
				digi_play("500w085", 1, 255, kCHANGE_WILBUR_ANIMATION);
				break;

			default:
				break;
			}

			if (_G(flags)[V212] == 5000) {
				_general.play("509wi03", 0xc00, 4, -1, 0, -1, 100, 0, 0, 16, 18);
			} else {
				_general.play("509wi01", 0xc00, 4, -1, 0, -1, 100, 0, 0, 16, 18);
			}
			break;

		case 13:
			_general.terminate();
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 14:
			_G(kernel).trigger_mode = KT_PARSE;

			if (_G(flags)[V212] == 5000) {
				_lights = series_show("509lgt02", 0xc00);
			} else {
				_lights = series_show("509lgt01", 0xc00);
			}

			ws_unhide_walker();
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10002;
			_G(player).command_ready = true;
			_G(player).need_to_walk = true;
			_G(player).ready_to_walk = true;
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room509::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!inv_player_has("CHRISTMAS LIGHTS") && !inv_player_has("CHRISTMAS LIGHTS ")) {
		if (player_said("HALLWAY") && !player_said_any("LEAVE", "LOOK AT", "GEAR"))
			player_set_facing_hotspot();
	} else {
		_G(wilbur_should) = 12;

		if (player_said("CHRISTMAS LIGHTS ", "PHONE CORD")) {
			_val2 = _G(flags)[V234] ? 6 : 5;
		} else if (player_said("CHRISTMAS LIGHTS", "PHONE CORD")) {
			_G(wilbur_should) = 5;
		} else if (player_said("LOOK AT") && player_said_any("CHRISTMAS LIGHTS", "CHRISTMAS LIGHTS ")) {
			_val2 = player_said("CHRISTMAS LIGHTS") ? 1 : 0;
		} else if (player_said("HOLE") && player_said_any("CHRISTMAS LIGHTS", "CHRISTMAS LIGHTS ")) {
			_val2 = 2;
		} else if (player_said("CHRISTMAS LIGHTS") || player_said("CHRISTMAS LIGHTS ")) {
			_val2 = 4;
			intr_cancel_sentence();
			_G(player).need_to_walk = false;
		} else {
			_G(wilbur_should) = 3;
		}

		_G(player).ready_to_walk = false;
		_general.terminate();
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	}
}

void Room509::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("LOOK AT", "CHRISTMAS LIGHTS  ") && _G(flags)[V212] == 5001) {
		wilbur_speech("509w003");
	} else if (player_said("GEAR", "CHRISTMAS LIGHTS  ")) {
		wilbur_speech("509w004");
	} else if (player_said_any("CHRISTMAS LIGHTS", "CHRISTMAS LIGHTS ") && player_said("HOLE")) {
		_val2 = 2;
		kernel_trigger_dispatch_now(12);
	} else if (player_said("LOOK AT", "HOLE") && _G(flags)[V227] != 0) {
		wilbur_speech("500w003");
	} else if (player_said("TAKE", "TELEPHONE") && _G(flags)[V197]) {
		wilbur_speech("500w005");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("HALLWAY") && player_said_any("LEAVE", "LOOK AT", "GEAR")) {
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 5009);
	} else if (player_said("TAKE", "CHRISTMAS LIGHTS  ")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
