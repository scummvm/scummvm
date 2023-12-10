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

#include "m4/burger/rooms/section5/room504.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room504::SAID[][4] = {
	{ "STAIRS",         "504w002", "500w001", nullptr   },
	{ "BORK",           "504w007", "500w002", "500w002" },
	{ "BORK GUTS",      "504w009", "504w010", "504w010" },
	{ "DIRTY SOCK ",    "504w011", nullptr,   nullptr   },
	{ "WRINGER",        "504w013", "504w015", "504w016" },
	{ "BREAKER BOX",    "504w017", nullptr,   "504w019" },
	{ "INSIDE BREAKER", "504w020", "504w021", nullptr   },
	{ "OUTSIDE BREAKER","504w024", "504w021", nullptr   },
	{ "WASHING MACHINE","504w026", nullptr,   "504w027" },
	{ "PAINT CAN",      "504w028", "504w029", "504w030" },
	{ "HOLE",           nullptr,   "500w001", "500w001" },
	{ "LAUNDRY HAMPER", "504w030", "504w016", "504w016" },
	{ "LAUNDRY HAMPER ","504w030", "504w016", "504w016" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room504::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY3[] = {
	{ 0,  7, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 8,  8, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, -1, nullptr,   0,   0,  8, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY4[] = {
	{  0,  8, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  9,  9, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 10, -1, nullptr,   0,   0,  8, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY5[] = {
	{ 0,  5, nullptr, 0, 0,    -1, 1, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 0, 10016, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY6[] = {
	{ 0,  4, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 5,  5, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr,   0,   0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY7[] = {
	{ 0,  6, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 7,  7, "504_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, -1, nullptr,   0,   0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY8[] = {
	{ 0,  6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 7, -1, nullptr, 0, 0, 10, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY9[] = {
	{  0, 30, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 31, -1, nullptr, 0, 0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY10[] = {
	{  0, 12, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 13, 13, "504w004",  1, 255, -1, 0, 0, &_state1, 1 },
	{ 13, 13, "504w018a", 1, 255, -1, 0, 0, &_state1, 2 },
	{ 13, 13, "504w018b", 1, 255, -1, 0, 0, &_state1, 3 },
	{ 13, 13, "504w018c", 1, 255, -1, 0, 0, &_state1, 4 },
	{ 13, 13, "504w018d", 1, 255, -1, 0, 0, &_state1, 5 },
	{ 13, 13, "504w018e", 1, 255, -1, 0, 0, &_state1, 6 },
	{ 14, 14, nullptr,    0,   0, -1, 0, 5, nullptr,  0 },
	{ 15, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY11[] = {
	{  0,  3, nullptr,    0,   0,  6, 0,  0, nullptr,  0 },
	{  6, 15, "504b002a", 2, 255, -1, 0,  0, &_state2, 1 },
	{  6, 15, "504b002b", 2, 255, -1, 0,  0, &_state2, 2 },
	{  6, 15, "504b002c", 2, 255, -1, 0,  0, &_state2, 3 },
	{ 16, 17, "504_005",  3, 125, -1, 0,  0, nullptr,  0 },
	{ 18, 27, "504b003",  2, 255, -1, 0,  0, nullptr,  0 },
	{ 28, 30, "504_006",  3, 125, -1, 0,  0, nullptr,  0 },
	{ 31, 41, "504b013a", 2, 255, -1, 1, -1, nullptr,  0 },
	{ 29, 33, "504b013c", 2, 255, -1, 1, -1, nullptr,  0 },
	{ 42, 47, "504_004",  3, 125, -1, 0,  0, nullptr,  0 },
	{ 49, 55, "504b005a", 2, 255, -1, 0,  0, &_state3, 1 },
	{ 49, 55, "504b005b", 2, 255, -1, 0,  0, &_state3, 2 },
	{ 56, 75, "504b006a", 2, 255, -1, 0,  0, &_state4, 1 },
	{ 56, 75, "504b006b", 2, 255, -1, 0,  0, &_state4, 2 },
	{ 76, 81, "500_014",  2, 255, -1, 0,  0, nullptr,  0 },
	{ 82, 82, nullptr,    0,   0,  6, 0,  7, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY12[] = {
	{ 0, -1, "504b001a", 2, 255, -1, 1, 0, &_state2, 1 },
	{ 0, -1, "504b001b", 2, 255, -1, 1, 0, &_state2, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY13[] = {
	{  0,  8, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  9,  9, "504_005",  1, 125, -1, 0, 0, nullptr,  0 },
	{ 10, 19, "504b008a", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 10, 19, "504b008b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 10, 19, "504b008c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 20, 20, "504_007",  3, 255, -1, 0, 0, nullptr,  0 },
	{ 21, 24, "504_006",  1, 125, -1, 0, 0, nullptr,  0 },
	{ 25, 27, "504b012a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 25, 27, "504b012b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 28, 29, "504_002",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 30, -1, "504_004",  1, 125, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY14[] = {
	{ 0, -1, "504b008a",  2, 255, -1, 0, 0, &_state2, 1 },
	{ 0, -1,  "504b008b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 0, -1, "504b008c",  2, 255, -1, 0, 0, &_state2, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY15[] = {
	{ 0, -1, "504_005", 3, 125, -1, 0, 2, nullptr, 0 },
	{ 0, -1, "504_006", 3, 125, -1, 0, 0, nullptr, 0 },
	{ 0, -1, "504_004", 3, 125, -1, 0, 4, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room504::PLAY16[] = {
	{ 0, -1, "500_008", 2, 255, -1, 0, 0, &_state5, 1 },
	{ 0, -1, "500_009", 2, 255, -1, 0, 0, &_state5, 2 },
	{ 0, -1, "500_010", 2, 255, -1, 0, 0, &_state5, 3 },
	{ 0, -1, "500_011", 2, 255, -1, 0, 0, &_state5, 4 },
	{ 0, -1, "500_012", 2, 255, -1, 0, 0, &_state5, 5 },
	{ 0, -1, "500_013", 2, 255, -1, 0, 0, &_state5, 6 },
	PLAY_BREAK_END
};

int32 Room504::_state1;
int32 Room504::_state2;
int32 Room504::_state3;
int32 Room504::_state4;
int32 Room504::_state5;

Room504::Room504() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
	_state5 = 0;
}

void Room504::init() {
	Section5Room::init();
	_initFlag = false;

	if (_G(flags)[V210] == 5002) {
		series_show("504bk10", 0x910);
		series_show("504bk10s", 0x910);
		hotspot_set_active("BORK", false);
		hotspot_set_active("BORK GUTS", true);
	} else {
		_val1 = 25;
		kernel_trigger_dispatch_now(5);
		_G(kernel).call_daemon_every_loop = true;
		hotspot_set_active("BORK GUTS", false);
		hotspot_set_active("BORK", true);
	}

	if (!_G(flags)[V213]) {
		_val2 = 31;
		kernel_trigger_dispatch_now(7);
	} else if (_G(flags)[V210] == 5002) {
		_G(flags)[V213] = 2;
		_val2 = 32;
		kernel_trigger_dispatch_now(7);
	}

	_val3 = (_G(flags)[V211] == 5001) ? 35 : 34;
	kernel_trigger_dispatch_now(8);
	_val4 = (_G(flags)[V212] == 5001) ? 35 : 34;
	kernel_trigger_dispatch_now(9);

	if (inv_object_is_here("DIRTY SOCK") && _G(flags)[V210] != 5002) {
		_sock = series_show("504SOCK", 0xf00);
	} else {
		hotspot_set_active("DIRTY SOCK ", false);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 501:
		player_set_commands_allowed(true);
		ws_demand_location(133, 320);
		break;

	case 503:
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(133, 320);
		break;
	}

	_initFlag = true;
	Section5Room::init();
}

void Room504::daemon() {
	switch (_G(kernel).trigger) {
	case 4:
		ws_unhide_walker();

		switch (_val5) {
		case 13:
			wilbur_speech("504w001");
			break;
		case 15:
			wilbur_speech("504w005");
			break;
		case 16:
			wilbur_speech("504w006", 5001);
			break;
		case 18:
			wilbur_speech("504w023");
			break;
		case 19:
			_val5 = 20;
			wilbur_speech("504w031");
			break;
		case 20:
			wilbur_speech("504w032");
			break;
		default:
			break;
		}
		break;

	case 5:
		switch (_val1) {
		case 21:
			if (_flag1) {
				_val1 = 26;
				kernel_trigger_dispatch_now(5);
				_G(wilbur_should) = 6;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				_val1 = imath_ranged_rand(0, 100) >= 75 ? 25 : 22;
				kernel_trigger_dispatch_now(5);
			}
			break;

		case 22:
			_G(flags)[V210] = 5000;
			_state2 = imath_ranged_rand(1, 2);
			_val1 = 21;
			series_play_with_breaks(PLAY12, "504bk01", 0x850, 5, 2, 12, 100);
			break;

		case 23:
			_state2 = imath_ranged_rand(1, 3);
			_G(wilbur_should) = 12;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			_val1 = 21;
			series_play_with_breaks(PLAY14, "504BK09", 0x850, 5, 2, 12);
			break;

		case 24:
			_G(flags)[V206] = 5002;
			_G(kernel).trigger_mode = KT_DAEMON;
			pal_fade_init(_G(kernel).first_fade, 255, 0, 0, 5015);
			break;

		case 25:
			_G(flags)[V210] = 5001;
			_state2 = imath_ranged_rand(1, 3);
			_state3 = imath_ranged_rand(1, 2);
			_state4 = imath_ranged_rand(1, 2);
			_val1 = 21;

			series_play_with_breaks(PLAY11, "504WBORK", 0x850, 5, 2, 12, 100);
			break;

		case 26:
			_G(flags)[V210] = 5002;
			_G(kernel).call_daemon_every_loop = false;
			_series3 = series_show("504bk01", 0x850);
			break;

		case 27:
			terminateMachineAndNull(_series3);
			_state2 = imath_ranged_rand(1, 3);
			_state3 = imath_ranged_rand(1, 2);
			_val1 = 28;
			series_play_with_breaks(PLAY13, "504BK08", 0x850, 5, 3);
			break;

		case 28:
			_val2 = 32;
			kernel_trigger_dispatch_now(7);

			_series3 = series_show("504bk10", 0x910);
			_series3s = series_show("504bk10s", 0x911);

			hotspot_set_active("BORK GUTS", true);
			hotspot_set_active("BORK", false);

			_val5 = 16;
			kernel_trigger_dispatch_now(4);
			break;

		default:
			break;
		}
		break;

	case 6: {
		static const char *NAMES[5] = {
			"504SPRK1", "504SPRK2", "504SPRK3", "504SPRK4", "504SPRK5" };
		_sparkName = NAMES[imath_ranged_rand(0, 4)];
		_state5 = imath_ranged_rand(1, 6);
		series_play_with_breaks(PLAY16, _sparkName, 0x910, -1, 2, 6, 100);
		break;
	}

	case 7:
		switch (_val2) {
		case 29:
			switch (imath_ranged_rand(0, 6)) {
			case 0:
			case 1:
			case 2:
				_val2 = 32;
				break;
			case 3:
			case 4:
				_val2 = 31;
				break;
			default:
				_val2 = 30;
				break;
			}

			kernel_trigger_dispatch_now(7);
			break;

		case 30:
			if (_G(flags)[V213] != 1) {
				_G(flags)[V213] = 1;
				kernel_trigger_dispatch_now(6);
			}

			_val2 = 29;
			kernel_trigger_dispatch_now(7);
			break;

		case 31:
			_G(flags)[V213] = 0;
			_val2 = 32;
			_val8 = imath_ranged_rand(0, 10);
			series_play_with_breaks(PLAY15, "504WASH", 0x900, 7, 2, 6, 100);
			break;

		case 32:
			_G(flags)[V213] = 2;
			_val2 = 29;
			_series4 = series_show("504wash", 0x900, 0, 7, 300);
			break;

		default:
			break;
		}
		break;

	case 8:
		switch (_val3) {
		case 34:
			terminateMachineAndNull(_series2);
			pal_fade_init(_G(kernel).first_fade, 255, 100, 30, 0);
			break;

		case 35:
			_series2 = series_show("504IBOFF", 0x900);
			pal_fade_init(_G(kernel).first_fade, 255, 1, 10, 0);
			break;

		default:
			break;
		}
		break;

	case 9:
		switch (_val4) {
		case 34:
			terminateMachineAndNull(_series5);
			break;
		case 35:
			_series5 = series_show("504OBOFF", 0x900);
			break;
		default:
			break;
		}
		break;

	case 10:
		switch (_sockState) {
		case 36:
			terminateMachineAndNull(_sock);
			hotspot_set_active("DIRTY SOCK ", false);
			inv_give_to_player("DIRTY SOCK");
			break;

		case 37:
			_sock = series_show("504SOCK", 0xf00);
			hotspot_set_active("DIRTY SOCK ", true);
			inv_move_object("DIRTY SOCK", NOWHERE);
			break;

		default:
			break;
		}
		break;

	case 5002:
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(true);
			kernel_trigger_dispatch_now(4);
			break;

		case 2:
			ws_demand_location(133, 320, 5);
			player_set_commands_allowed(false);
			ws_hide_walker();

			if (player_been_here(504)) {
				_G(wilbur_should) = 10001;
			} else {
				_val5 = 13;
				_G(wilbur_should) = 1;
			}

			series_play_with_breaks(PLAY1, "504wi01", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 3:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(false);
			ws_hide_walker();
			series_play_with_breaks(PLAY2, "504wi02", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			pal_fade_init(_G(kernel).first_fade, 255, 0, 60, 5007);
			break;

		case 5:
			_G(wilbur_should) = 10001;
			_sockState = 36;
			player_set_commands_allowed(false);
			ws_hide_walker();
			series_play_with_breaks(PLAY8, "504wi06", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 6:
			ws_demand_facing(4);
			ws_hide_walker();
			_val1 = 27;
			_G(wilbur_should) = 39;

			series_play_with_breaks(PLAY9, "504Wi07", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			inv_move_object("DIRTY SOCK", NOWHERE);
			break;

		case 7:
			_val5 = 18;
			_G(wilbur_should) = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_val3 = 34;

			series_play_with_breaks(PLAY3, "504wi3b", 0x200, kCHANGE_WILBUR_ANIMATION, 2);
			_G(flags)[V211] = 5000;
			break;

		case 8:
			_G(wilbur_should) = 9;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_val3 = 35;
			series_play_with_breaks(PLAY4, "504Wi3A", 0x200, kCHANGE_WILBUR_ANIMATION, 2);
			_G(flags)[V211] = 5001;
			break;

		case 9:
			digi_play("504w022", 1);
			_G(wilbur_should) = 7;
			series_play_with_breaks(PLAY5, "504wi10", 0x200, -1, 2, 12);
			break;

		case 10:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_val4 = 34;
			series_play_with_breaks(PLAY6, "504Wi4B", 0x200, kCHANGE_WILBUR_ANIMATION, 2);
			break;

		case 11:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_val4 = 35;
			series_play_with_breaks(PLAY7, "504Wi4A", 0x200, kCHANGE_WILBUR_ANIMATION, 2);
			break;

		case 12: {
			_val5 = 15;
			_G(wilbur_should) = 1;
			_state1 = imath_ranged_rand(1, 6);
			int x = (double)(_state1 * _G(player_info).scale * 42) * 0.01 - _G(player_info).x;

			ws_demand_location(x, _G(player_info).y);
			ws_hide_walker();
			series_play_with_breaks(PLAY10, "504Wi08", 0x200, kCHANGE_WILBUR_ANIMATION, 3, 6,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		}

		case 39:
			ws_unhide_walker();
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCALLED_EACH_LOOP:
		player_update_info();

		if (_G(player_info).x > 300 && _G(player_info).y > 307) {
			if (!_G(flags)[V214]) {
				_G(flags)[V214] = 1;

				if (_G(flags)[V210] != 5002) {
					if (_val6 >= 3) {
						_G(kernel).call_daemon_every_loop = false;
						_val1 = 24;
						kernel_trigger_dispatch_now(5);
					} else {
						++_val6;
						_val1 = 23;
						player_set_commands_allowed(false);
						intr_cancel_sentence();
						_G(player).command_ready = false;

						player_update_info();
						ws_demand_location(_G(player_info).x, _G(player_info).y, 3);
					}
				}
			}
		} else {
			_G(flags)[V214] = 0;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room504::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("STAIRS") && !player_said_any("CLIMB", "GEAR"))
		player_set_facing_hotspot();
}

void Room504::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("LOOK AT", "BORK")) {
		switch (_G(flags)[V210]) {
		case 5000:
			wilbur_speech("504w003");
			break;
		case 5001:
			wilbur_speech("504w008");
			break;
		default:
			break;
		}
	} else if (player_said("RUBBER DUCKY", "BORK")) {
		wilbur_speech("500w036");
	} else if (player_said("SOAPY WATER", "BORK")) {
		wilbur_speech("500w050");
	} else if (player_said("GEAR", "DIRTY SOCK ")) {
		wilbur_speech("500w012");
	} else if (player_said("LOOK AT", "WRINGER") && _G(flags)[V210] == 5002) {
		wilbur_speech("504w014");
	} else if (player_said("LOOK AT", "OUTSIDE BREAKER") && _G(flags)[V212] == 5000) {
		wilbur_speech("504w025");
	} else if (player_said("GEAR", "WASHING MACHINE") && _G(flags)[V210] == 5002) {
		wilbur_speech("504w016");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("CLIMB", "STAIRS") || player_said("GEAR", "STAIRS")) {
		player_set_commands_allowed(false);
		_G(kernel).trigger_mode = KT_DAEMON;
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "DIRTY SOCK ")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("DIRTY SOCK", "WRINGER")) {
		player_set_commands_allowed(false);
		_flag1 = true;
	} else if (player_said("GEAR", "INSIDE BREAKER")) {
		if (_G(flags)[V211] == 5000) {
			_G(wilbur_should) = 8;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			_G(wilbur_should) = 7;
			_G(flags)[V211] = 5000;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("GEAR", "OUTSIDE BREAKER")) {
		if (_G(flags)[V212] == 5000) {
			_G(wilbur_should) = 11;
			_G(flags)[V212] = 5001;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			_G(wilbur_should) = 10;
			_G(flags)[V212] = 5000;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("LOOK AT", "HOLE")) {
		_val5 = 19;
		kernel_trigger_dispatch_now(4);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
