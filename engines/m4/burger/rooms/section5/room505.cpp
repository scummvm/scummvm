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

#include "m4/burger/rooms/section5/room505.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room505::SAID[][4] = {
	{ "WILBUR'S ROOM", nullptr,   "500w001", nullptr   },
	{ "BATHROOM",      nullptr,   "500w001", nullptr   },
	{ "AUNT POLLY'S ROOM", nullptr, "500w001", nullptr },
	{ "STAIRS",        "505w001", "500w001", nullptr   },
	{ "STAIRWAY BORK", "505w003", "500w002", "500w002" },
	{ "RAILING",       "505w004", nullptr,   "505w006" },
	{ "WINDOW",        "500w003", nullptr,   "500w004" },
	{ "CEILING FAN",   "505w008", nullptr,   nullptr   },
	{ "CHANDELIER",    "505w009", nullptr,   nullptr   },
	{ "VASE",          "505w010", "505w011", "505w012" },
	{ "PICTURE",       "505w013", "500w005", "505w014" },
	{ "PICTURE ",      "505w015", "500w005", "505w014" },
	{ "PICTURE  ",     "505w016", "500w005", "505w014" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const Section5Room::BorkPoint Room505::BORK[] = {
	{ -1, -1 }, { 257, 255 }, { 266, 257 }, { 271, 250 }, { 292, 246 },
	{ 317, 251 }, { 323, 256 }, { 338, 237 }, { 370, 227 }, { 348, 187 },
	{ 332, 165 }, { 308, 161 }, { 282, 161 }, { 303, 175 }, { 325, 179 },
	{ 374, 174 }, { 350, 172 }, { 327, 168 }, { 302, 167 }, { 282, 161 },
	{ 303, 175 }, { 325, 179 }, { 374, 174 }, { 354, 175 }, { 330, 183 },
	{ 317, 204 }, { 305, 217 }, { 284, 210 }, { 270, 227 }, { 255, 236 },
	{ -1, -1 }, { -1, -1 }
};

const seriesPlayBreak Room505::PLAY1[] = {
	{ 0,  3, "505_003a", 1, 100, -1, 0, 0, nullptr, 0 },
	{ 4, -1, "505_003b", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room505::PLAY2[] = {
	{ 0,  2, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{ 3, -1, "505_003a", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

Room505::Room505() : Section5Room() {
	Common::strcpy_s(_bork, "BORK");
	Common::strcpy_s(_lookAt, "LOOK AT");
	_borkHotspot.vocab = _bork;
	_borkHotspot.verb = _lookAt;
	_borkHotspot.prep = _bork;
	_borkHotspot.feet_x = 339;
	_borkHotspot.feet_y = 305;
	_borkHotspot.facing = 11;
	_borkHotspot.cursor_number = kArrowCursor;
	_borkThreshold = 1600;
}

void Room505::init() {
	Section5Room::init();
	pal_cycle_init(112, 127, 6);
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 502:
		ws_demand_location(333, 271, 3);
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 506:
		ws_demand_location(52, 349, 3);
		ws_walk(328, 323, nullptr, -1, -1);
		player_set_commands_allowed(true);
		break;

	case 507:
		ws_demand_location(84, 308, 3);
		ws_walk(328, 323, nullptr, -1, -1);
		player_set_commands_allowed(true);
		break;

	case 509:
		ws_demand_location(497, 311, 9);
		ws_walk(328, 323, nullptr, -1, -1);
		player_set_commands_allowed(true);
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(328, 323);
		break;
	}

	if (_G(flags)[kStairsBorkState] == 5003) {
		kernel_trigger_dispatch_now(7);
	} else {
		series_load("505bk01");
		series_load("505bk01s");
		_borkTable = BORK;
		_borkState = 7;
		_G(flags)[V186] = 1;
		kernel_trigger_dispatch_now(4);
		kernel_trigger_dispatch_now(2);
	}

	_initFlag = true;
	Section5Room::init();
}

void Room505::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, _destTrigger);
		break;

	case 2:
		if (_G(flags)[kStairsBorkState] == 5000) {
			player_update_info();

			if (_G(player_info).y > 280 && _G(player_info).y < 300) {
				ws_walk(_G(player_info).x, _G(player_info).y, nullptr, -1, -1);
				player_set_commands_allowed(false);
				kernel_trigger_dispatch_now(6);
			} else {
				kernel_timing_trigger(15, 2);
			}
		}
		break;

	case 3:
		player_update_info();

		if (_G(player_info).y > 300) {
			_borkState = _val3;
			kernel_trigger_dispatch_now(4);
		} else {
			kernel_timing_trigger(15, 3);
		}
		break;

	case 4:
		switch (_borkState) {
		case 4:
			_G(flags)[V186] = 0;
			_borkStairs.terminate();
			_val3 = 5;
			kernel_timing_trigger(imath_ranged_rand(240, 360), 3);
			break;

		case 5:
			_G(flags)[V186] = 1;
			_G(flags)[kStairsBorkState] = 5000;
			_borkState = 6;
			_borkStairs.play("505bk01", 0xbff, 0, 4, 6, 0, 100, 0, 0, 0, 2);
			break;

		case 6:
			_val4 = 0;
			kernel_trigger_dispatch_now(5);
			kernel_trigger_dispatch_now(2);
			_borkState = 7;
			_borkStairs.play("505bk01", 0xbff, 0, 4, 6, 0, 100, 0, 0, 3, 9);
			break;

		case 7:
			terminateMachineAndNull(_fan);
			_borkState = 8;
			_borkStairs[0] = series_play("505bk01", 0xbff, 0, 4, 6, 0, 100, 0, 0, 10, 24);
			_borkStairs[1] = series_play("505bk01s", 0xc00, 0, 7, 6, 0, 100, 0, 0, 10, 24);

			_val4 = 1;
			kernel_trigger_dispatch_now(5);
			break;

		case 8:
			_borkState = 9;
			_borkStairs.play("505bk01", 0xbff, 0, 4, 6, 0, 100, 0, 0, 25, 24);
			_val4 = 2;
			kernel_trigger_dispatch_now(5);
			break;

		case 9:
			_G(flags)[kStairsBorkState] = 5001;
			_borkState = 4;
			_borkStairs.play("505bk01", 0xbff, 0, 4, 6, 0, 100, 0, 0, 27, 31);
			_val4 = 3;
			kernel_trigger_dispatch_now(5);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case 5:
		switch (_val4) {
		case 0:
			digi_play((imath_ranged_rand(1, 2) == 1) ? "505b001a" : "505b001b", 2);
			break;

		case 1:
			digi_play("505b002", 2);
			break;

		case 2:
			digi_play("505b003", 2);
			break;

		case 3:
			digi_play((imath_ranged_rand(1, 2) == 1) ? "505b004a" : "505b004b", 2);
			break;
		}
		break;

	case 6:
		_G(flags)[V206] = 5005;
		kernel_trigger_dispatch_now(5015);
		break;

	case 7:
		_fan = series_play("505fan", 0xbff, 0, -1, 6, -1);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(false);
			ws_demand_location(333, 271, 3);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY1, "505wi01", 0xc00, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 2:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_destTrigger = 5006;
			kernel_timing_trigger(30, 1);
			series_play_with_breaks(PLAY2, "505wi02", 0xc00, -1, 3);
			break;

		case 3:
			_G(wilbur_should) = 2;
			wilbur_speech("500w054", kCHANGE_WILBUR_ANIMATION);
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

void Room505::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said_any("WILBUR'S ROOM", "BATHROOM", "AUNT POLLY'S ROOM") &&
			!player_said_any("ENTER", "LOOK AT", "GEAR"))
		player_set_facing_hotspot();
}

void Room505::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool railingFlag = player_said("RAILING") && _G(flags)[kStairsBorkState] == 5003;

	if (player_said("LOOK AT", "STAIRS") && _G(flags)[kStairsBorkState] == 5003) {
		wilbur_speech("505w002");
	} else if (railingFlag && player_said("LOOK AT")) {
		wilbur_speech("505w005");
	} else if (railingFlag && player_said("GEAR")) {
		wilbur_speech("505w007");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("WILBUR'S ROOM") && player_said_any("GEAR", "ENTER", "LOOK AT")) {
		_destTrigger = 5010;
		kernel_trigger_dispatch_now(1);
	} else if (player_said("LOOK AT", "BATHROOM") || player_said("GEAR", "BATHROOM")) {
		_destTrigger = 5011;
		kernel_trigger_dispatch_now(1);
	} else if (player_said("LOOK AT", "AUNT POLLY'S ROOM") || player_said("GEAR", "AUNT POLLY'S ROOM")) {
		_destTrigger = 5013;
		kernel_trigger_dispatch_now(1);
	} else if (player_said("GEAR", "STAIRS")) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("SOAPY WATER", "RAILING")) {
		_G(flags)[V196] = 1;
		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
