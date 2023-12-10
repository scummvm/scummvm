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

#include "m4/burger/rooms/section5/room508.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kWILBUR_SPEECH = 1,
	kCHANGE_BORK_ANIMATION = 2
};

enum {
	kSPEECH_DESTROYING_ROOF = 10,
	kSPEECH_WONT_LET_ME = 11,
	kSPEECH_BORK_DESTROYED = 12
};

const char *Room508::SAID[][4] = {
	{ "WILBUR'S ROOM",      nullptr,   "500w001", nullptr   },
	{ "BORK",               "508w003", "500w002", "500w002" },
	{ "CHRISTMAS LIGHTS  ", "508w004", nullptr,   "508w007" },
	{ "CHRISTMAS LIGHTS   ","508w005", "508w006", "508w007" },
	{ "HOLE",               "508w008", "500w001", "508w009" },
	{ "BATHROOM",           "508w010", nullptr,   "508w011" },
	{ "CHIMNEY",            "508w012", "500w001", "508w014" },
	{ "CHIMNEY POTS",       "508w012", "500w001", "508w014" },
	{ "ROOF",               "508w015", "500w001", "508w016" },
	{ "FORCE FIELD",        "508w017", "500w001", "500w001" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room508::PLAY1[] = {
	{ 0, 4, "508_001", 1, 255, -1, 1, 0, nullptr, 0  },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY2[] = {
	{ 0,  4, "508_001", 1, 255, -1, 1024, 0, nullptr, 0 },
	{ 5, -1, nullptr,   0,   0,  5,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY3[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, 28, "508_002", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, -1, nullptr,   1,   0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY4[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, 27, "508_002", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 28, -1, nullptr,   0,   0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY7[] = {
	{ 0, -1, "508b001a", 2, 255, -1, 1025, 0, &_state1, 1 },
	{ 0, -1, "508b001b", 2, 255, -1, 1025, 0, &_state1, 2 },
	{ 0, -1, "508b001c", 2, 255, -1, 1025, 0, &_state1, 3 },
	{ 0, -1, "508b001d", 2, 255, -1, 1025, 0, &_state1, 4 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY8[] = {
	{  0,  5, "508b003",  2, 255, -1,    0, 0, nullptr,  0 },
	{  6, 16, "500_005",  2, 255, -1,    0, 0, nullptr,  0 },
	{ 17, -1, "508b001a", 2, 255, -1, 1024, 0, &_state1, 1 },
	{ 17, -1, "508b001b", 2, 255, -1, 1024, 0, &_state1, 2 },
	{ 17, -1, "508b001c", 2, 255, -1, 1024, 0, &_state1, 3 },
	{ 17, -1, "508b001d", 2, 255, -1, 1024, 0, &_state1, 4 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY9[] = {
	{  0,  5, nullptr,    0,   0, -1,    0, 0, nullptr,  0 },
	{  6, 12, "500_006",  2, 255, -1,    0, 0, nullptr,  0 },
	{ 13, 18, "508b005a", 2, 255, -1,    4, 6, &_state1, 1 },
	{ 13, 18, "508b005b", 2, 255, -1,    4, 6, &_state1, 2 },
	{ 19, -1, "508b001a", 2, 255, -1, 1024, 0, &_state2, 1 },
	{ 19, -1, "508b001b", 2, 255, -1, 1024, 0, &_state2, 2 },
	{ 19, -1, "508b001c", 2, 255, -1, 1024, 0, &_state2, 3 },
	{ 19, -1, "508b001d", 2, 255, -1, 1024, 0, &_state2, 4 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY10[] = {
	{  0, 20, "508b002a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0, 20, "508b002b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  0, 20, "508b002c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 21, -1, nullptr,    0,   0,  1, 0, 0, &_state3, 1 },
	{ 21, -1, nullptr,    0,   0,  1, 0, 0, &_state3, 2 },
	{ 21, -1, nullptr,    0,   0,  2, 0, 0, &_state3, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY11[] = {
	{  0, 10, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 11, 17, "508b006a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 17, "508b006b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 11, 17, "508b006c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 11, 17, "508b006d", 2, 255, -1, 0, 0, &_state1, 4 },
	{ 18, 18, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 19, 28, "508b007a", 2, 255, -1, 0, 0, &_state2, 1 },
	{ 19, 28, "508b007b", 2, 255, -1, 0, 0, &_state2, 2 },
	{ 19, 28, "508b007c", 2, 255, -1, 0, 0, &_state2, 3 },
	{ 29, 42, "508b008a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 29, 42, "508b008b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 29, 42, "508b008c", 2, 255, -1, 0, 0, &_state4, 3 },
	{ 43, -1, "508b009",  2, 255, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room508::PLAY12[] = {
	{  0, 12, "500_003",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 13, -1, "508b010a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 13, -1, "508b010b", 2, 255, -1, 0, 0, &_state1, 2 },
	PLAY_BREAK_END
};

int32 Room508::_state1;
int32 Room508::_state2;
int32 Room508::_state3;
int32 Room508::_state4;


Room508::Room508() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
}

void Room508::preload() {
	Section5Room::preload();
	_G(player).walker_in_this_scene = false;
}

void Room508::init() {
	Section5Room::init();
	player_set_commands_allowed(true);
	_flag1 = true;

	pal_cycle_init(112, 127, 6);
	_val1 = 3;
	_flag1 = false;
	_state3 = 0;
	_val2 = 0;

	if (_G(flags)[kFireplaceHasFire] == 1) {
		_series1 = series_play("508SMOKE", 0x500, 0, -1, 6, -1);
		hotspot_set_active("BORK", false);

		if (!_G(flags)[V227]) {
			_borkState = 18;
			kernel_trigger_dispatch_now(kCHANGE_BORK_ANIMATION);
		}
	} else {
		loadSeries();
		setup();
		kernel_trigger_dispatch_now(kCHANGE_BORK_ANIMATION);
	}

	if (inv_player_has("CHRISTMAS LIGHTS") || inv_player_has("CHRISTMAS LIGHTS ")) {
		_val4 = 24;
		kernel_trigger_dispatch_now(5);
	} else if (_G(flags)[V229] == 1) {
		_val4 = 21;
		kernel_trigger_dispatch_now(5);
	} else {
		_val4 = 22;
		kernel_trigger_dispatch_now(22);
	}

	if (_G(flags)[V228] == 1) {
		_val5 = 20;
		kernel_trigger_dispatch_now(6);
	}

	switch (_G(game).previous_room) {
	case 506:
		player_set_commands_allowed(true);
		_G(wilbur_should) = 1;

		if (!player_been_here(508) && !_G(flags)[kFireplaceHasFire]) {
			_flag2 = true;
			_speechNum = kSPEECH_DESTROYING_ROOF;
		}

		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		_flag1 = false;
		_val1 = 3;

		if (inv_player_has("CHRISTMAS LIGHTS")) {
			_G(wilbur_should) = 5;
		} else if (inv_player_has("CHRISTMAS LIGHTS ")) {
			_G(wilbur_should) = 6;
		} else {
			_G(wilbur_should) = 4;
		}

		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;
	}

	_initFlag = true;
	Section5Room::init();
}

void Room508::daemon() {
	switch (_G(kernel).trigger) {
	case kWILBUR_SPEECH:
		switch (_speechNum) {
		case kSPEECH_DESTROYING_ROOF:
			wilbur_speech("508w001");
			break;
		case kSPEECH_WONT_LET_ME:
			wilbur_speech("508w001z");
			break;
		case kSPEECH_BORK_DESTROYED:
			wilbur_speech("508w002");
			break;
		default:
			break;
		}
		break;

	case kCHANGE_BORK_ANIMATION:
		switch (_borkState) {
		case 14:
			setup();
			_state1 = imath_ranged_rand(1, 4);
			series_play_with_breaks(PLAY7, "508BK01", 0x200, kCHANGE_BORK_ANIMATION, 3);

			if (_G(flags)[V228] == 0 && _G(flags)[V223] == 0 && _val2 > 3) {
				kernel_trigger_dispatch_now(3);
			} else {
				++_val2;
			}
			break;

		case 15:
			setup();
			_state1 = imath_ranged_rand(1, 4);
			series_play_with_breaks(PLAY8, "508BK02", 0x200, kCHANGE_BORK_ANIMATION, 3);
			break;

		case 16:
			setup();
			_state1 = imath_ranged_rand(1, 2);
			_state2 = imath_ranged_rand(1, 4);
			series_play_with_breaks(PLAY9, "508BK03", 0x200, kCHANGE_BORK_ANIMATION, 3);
			break;

		case 17:
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			++_state3;
			_speechNum = 11;
			setup();

			if (_state3 == 3)
				_borkState = 19;

			_state1 = imath_ranged_rand(1, 3);
			series_play_with_breaks(PLAY10, "508BK04", 0x200, kCHANGE_BORK_ANIMATION, 3, 6);
			break;

		case 18:
			_speechNum = kSPEECH_BORK_DESTROYED;
			_G(flags)[V227] = 1;
			hotspot_set_active("BORK", false);

			_state1 = imath_ranged_rand(1, 4);
			_state2 = imath_ranged_rand(1, 3);
			_state4 = imath_ranged_rand(1, 3);
			series_play_with_breaks(PLAY11, "508BK05", 0x200, 1, 3, 6);
			break;

		case 19:
			_G(flags)[V206] = 5003;
			_G(kernel).trigger_mode = KT_DAEMON;
			pal_fade_init(_G(kernel).first_fade, 255, 0, 0, 5015);
			break;

		default:
			break;
		}
		break;

	case 3:
		_G(flags)[V228] = 1;
		_state1 = imath_ranged_rand(1, 2);
		_val5 = 20;
		series_play_with_breaks(PLAY12, "508BK06", 0x200, 6, 3);
		break;

	case 5:
		switch (_val4) {
		case 21:
			hotspot_set_active("CHRISTMAS LIGHTS   ", false);
			hotspot_set_active("CHRISTMAS LIGHTS  ", true);
			_G(flags)[V229] = 1;
			_series3 = series_show(_G(flags)[V212] == 5000 ? "508LGT03" : "508LGT01", 0x300);
			break;

		case 22:
			hotspot_set_active("CHRISTMAS LIGHTS   ", true);
			hotspot_set_active("CHRISTMAS LIGHTS  ", false);
			_G(flags)[V229] = 0;
			_series3 = series_show(_G(flags)[V212] == 5000 ? "508LGT04" : "508LGT02", 0x400);
			break;

		case 23:
			_G(flags)[V229] = 0;
			terminateMachineAndNull(_series3);
			hotspot_set_active("CHRISTMAS LIGHTS  ", false);
			hotspot_set_active("CHRISTMAS LIGHTS   ", false);
			break;

		default:
			break;
		}
		break;

	case 6:
		switch (_val5) {
		case 20:
			_series2 = series_show("508WINDO", 0x200);
			break;
		default:
			break;
		}
		break;

	case 7:
		_G(kernel).trigger_mode = KT_DAEMON;
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, 5010);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			_val1 = 3;
			player_set_commands_allowed(false);
			_flag1 = false;
			_G(wilbur_should) = 4;
			series_play_with_breaks(PLAY6, "508Wi01", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
			break;

		case 2:
			player_set_commands_allowed(false);
			_flag1 = false;
			terminateMachineAndNull(_series4);
			_val1 = 1;
			_G(wilbur_should) = 4;
			series_play_with_breaks(PLAY1, "508Wi06", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
			break;

		case 3:
			player_set_commands_allowed(false);
			_flag1 = false;
			_val4 = 23;
			terminateMachineAndNull(_series4);
			_val1 = 1;

			if (_G(flags)[V212] == 5000) {
				_G(wilbur_should) = 6;
				series_play_with_breaks(PLAY2, "508Wi06", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
			} else {
				_G(wilbur_should) = 5;
				series_play_with_breaks(PLAY2, "508Wi03", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
			}
			break;

		case 4:
			if (_flag2) {
				_flag2 = false;
				kernel_trigger_dispatch_now(kWILBUR_SPEECH);
			}

			_G(wilbur_should) = 4;
			if (!_flag1) {
				player_set_commands_allowed(true);
				_flag1 = true;
			}

			if (_flag3) {
				_G(wilbur_should) = 9;
				_flag3 = false;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			}

			if (_val1 == 1) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_show("508Wi09", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 0);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_play("508Wi09", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 2);
					break;

				case 2:
					_val1 = 3;
					_val1 = 1;
					_series4 = series_play("508Wi09", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 3);
					break;

				default:
					break;
				}
			} else if (_val1 == 2) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_play("508Wi09", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 2);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_show("508Wi09", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 2);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_play("508Wi09", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 2, 3);
					break;

				default:
					break;
				}
			} else if (_val1 == 3) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_play("508Wi09", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 3);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_play("508Wi09", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 2, 3);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_show("508Wi09", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 3);
					break;

				default:
					break;
				}
			}
			break;

		case 5:
			if (!_flag1) {
				player_set_commands_allowed(true);
				_flag1 = true;
			}

			inv_give_to_player("CHRISTMAS LIGHTS");

			if (_val1 == 1) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_show("508wi10", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 0);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_play("508wi10", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 2);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_play("508wi10", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 3);
					break;

				default:
					break;
				}
			} else if (_val1 == 2) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_play("508wi10", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 2);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_show("508wi10", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 2);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_play("508wi10", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 2, 3);
					break;

				default:
					break;
				}
			} else if (_val1 == 3) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_play("508wi10", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 3);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_play("508wi10", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 2, 3);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_show("508wi10", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 3);
					break;

				default:
					break;
				}
			}
			break;

		case 6:
			if (!_flag1) {
				player_set_commands_allowed(true);
				_flag1 = true;
			}

			inv_give_to_player("CHRISTMAS LIGHTS ");

			if (_val1 == 1) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_show("508wi11", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 0);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_play("508wi11", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 2);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_play("508wi11", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 3);
					break;

				default:
					break;
				}
			} else if (_val1 == 2) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_play("508wi11", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 2);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_show("508wi11", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 2);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_play("508wi11", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 2, 3);
					break;

				default:
					break;
				}
			} else if (_val1 == 3) {
				switch (imath_ranged_rand(0, 2)) {
				case 0:
					_val1 = 1;
					_series4 = series_play("508wi11", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 0, 3);
					break;

				case 1:
					_val1 = 2;
					_series4 = series_play("508wi11", 0x200, 2, kCHANGE_WILBUR_ANIMATION, 20, 0, 100, 0, 0, 2, 3);
					break;

				case 2:
					_val1 = 3;
					_series4 = series_show("508wi11", 0x200, 0, kCHANGE_WILBUR_ANIMATION, 40, 3);
					break;

				default:
					break;
				}
			}
			break;

		case 7:
			_G(wilbur_should) = 4;
			_val4 = 22;
			player_set_commands_allowed(false);
			_flag1 = false;
			terminateMachineAndNull(_series4);
			_val1 = 1;

			if (_G(flags)[V212] == 5000) {
				series_play_with_breaks(PLAY3, "508Wi07", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
				inv_move_object("CHRISTMAS LIGHTS ", 508);
			} else {
				series_play_with_breaks(PLAY3, "508Wi04", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
				inv_move_object("CHRISTMAS LIGHTS", 508);
			}
			break;

		case 8:
			_G(wilbur_should) = 4;
			_val4 = 21;
			player_set_commands_allowed(false);
			_flag1 = false;
			terminateMachineAndNull(_series4);
			_val1 = 1;

			if (_G(flags)[V212] == 5000) {
				series_play_with_breaks(PLAY4, "508Wi08", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
				inv_move_object("CHRISTMAS LIGHTS ", 508);
			} else {
				series_play_with_breaks(PLAY4, "508Wi05", 0x300, kCHANGE_WILBUR_ANIMATION, 2);
				inv_move_object("CHRISTMAS LIGHTS", 508);
			}
			break;

		case 9:
			player_set_commands_allowed(false);
			_flag1 = false;
			_val1 = 1;
			_flag3 = false;

			if (inv_player_has("CHRISTMAS LIGHTS") || player_said("CHRISTMAS LIGHTS ")) {
				_G(wilbur_should) = 8;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				_G(kernel).continue_handling_trigger = true;
			} else {
				terminateMachineAndNull(_series4);
				_G(wilbur_should) = 10002;
				series_play_with_breaks(PLAY5, "508Wi02", 0x300, 7, 2);
			}
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

void Room508::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("LOOK AT") && player_said_any("CHIMNEY", "CHIMNEY POTS") &&
			_G(flags)[kFireplaceHasFire]) {
		wilbur_speech("508w013");
	} else if (player_said("RUBBER DUCKY", "BORK")) {
		wilbur_speech("500w042");
	} else if (player_said("BORK", "KINDLING")) {
		wilbur_speech("500w069");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("TAKE", "CHRISTMAS LIGHTS  ")) {
		player_set_commands_allowed(false);

		if (_G(flags)[V227]) {
			_flag1 = false;
			_G(wilbur_should) = 3;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			_borkState = 17;
		}
	} else if (player_said("HOLE") && player_said_any("CHRISTMAS LIGHTS ", "CHRISTMAS LIGHTS")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 7;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("ROOF") && player_said_any("CHRISTMAS LIGHTS ", "CHRISTMAS LIGHTS")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 8;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("WILBUR'S ROOM") && player_said_any("LEAVE", "LOOK AT", "GEAR")) {
		player_set_commands_allowed(false);
		_flag3 = true;
		_G(wilbur_should) = 9;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room508::loadSeries() {
	static const char *NAMES[8] = {
		"508bk01", "508bk01s", "508bk02", "508bk02s", "508bk03",
		"508bk03s", "508bk04", "508bk04s"
	};
	for (int i = 0; i < 8; ++i)
		series_load(NAMES[i]);

	if (!_G(flags)[V228]) {
		series_load("508bk06");
		series_load("508bk06s");
	}
}

void Room508::setup() {
	switch (imath_ranged_rand(0, 3)) {
	case 0:
	case 1:
		_borkState = 14;
		break;
	case 2:
		_borkState = 15;
		break;
	case 3:
		_borkState = 16;
		break;
	default:
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
