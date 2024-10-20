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

#include "m4/burger/rooms/section5/room503.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"
#include "m4/adv_r/adv_hotspot.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room503::SAID[][4] = {
	{ "PARLOUR",        nullptr,   "500w001", nullptr   },
	{ "BASEMENT",       nullptr,   "500w001", nullptr   },
	{ "BORK",           "503w004", "500w002", "500w002" },
	{ "ROLLING PIN ",   "503w007", nullptr,   "503w008" },
	{ "MICROWAVE",      "503w009", nullptr,   nullptr   },
	{ "RUBBER GLOVES ", "503w014", nullptr,   nullptr   },
	{ "HUTCH",          "503w016", nullptr,   "500w001" },
	{ "DISHES",         "503w017", "503w018", "503w018" },
	{ "FRIDGE",         "503w019", nullptr,   "500w001" },
	{ "OVEN",           "503w020", nullptr,   nullptr   },
	{ "MOP",            "503w024", "503w025", "503w025" },
	{ "CUPBOARD ",      "503w026", nullptr,   nullptr   },
	{ "CUPBOARD",       "503w028", nullptr,   nullptr   },
	{ "PRUNES",         "503w029", "503w031", "503w031" },
	{ "WINDOW",         "500w003", nullptr,   "500w004" },
	{ "SINK",           "503w034", "503w035", nullptr   },
	{ "OUTLET",         "503w036", "500w001", "503w037" },
	{ "CALENDAR",       "503w038", "500w005", "503w038" },
	{ "TOASTER",        "503w039", "503w040", "503w040" },
	{ "COOKIES",        "503w041", "503w042", "503w043" },
	{ "WIRES",          "503w044", "503w045", "503w045" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room503::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY2[] = {
	{  0,  2, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  3, 18, "503b001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 19, -1, "503_002", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY3[] = {
	{  0,  3, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  4,  9, "503_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 10, 14, "503_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 15, 18, "503_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 19, -1, "503_002", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY4[] = {
	{  0,  4, "503b004a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0,  4, "503b004b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  0,  4, "503b004c", 2, 255, -1, 0, 0, &_state1, 3 },
	{  5,  9, nullptr,    0,   0, 13, 0, 0, nullptr,  0 },
	{ 10, 10, nullptr,    0,   0, 18, 0, 0, &_state2, 3 },
	{ 10, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY5[] = {
	{ 0, -1, "503b005a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 0, -1, "503b005b", 2, 255, -1, 0, 0, &_state1, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY6[] = {
	{  0,  3, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{  4,  7, nullptr,    0,   0, 10, 0, 0, nullptr,  0 },
	{  8, 10, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 11, 13, "503b006a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 13, "503b006b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 14, 20, "503b007a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 14, 20, "503b007b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 14, 20, "503b007c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 14, 20, "503b007d", 2, 255, -1, 0, 0, &_state3, 4 },
	{ 21, 21, "503b008a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 21, 21, "503b008b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 22, 25, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 25, 25, nullptr,    0,   0, 18, 0, 0, &_state5, 1 },
	{ 26, 26, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 27, -1, nullptr,    0,   0, 11, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY7[] = {
	{  4,  7, nullptr,    0,   0, 10, 0, 0, nullptr,  0 },
	{  8, 10, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 11, 13, "503b006a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 13, "503b006b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 14, 20, "503b007a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 14, 20, "503b007b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 14, 20, "503b007c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 14, 20, "503b007d", 2, 255, -1, 0, 0, &_state3, 4 },
	{ 21, 21, "503b008a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 21, 21, "503b008b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 22, 25, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY8[] = {
	{ 0,  3, "503_007",  3, 255, -1, 0, 0, nullptr,  0  },
	{ 4,  7, "503b002a", 2, 255, -1, 4, 3, &_state1, 1 },
	{ 4,  7, "503b002b", 2, 255, -1, 4, 3, &_state1, 2 },
	{ 4,  7, "503b002c", 2, 255, -1, 4, 3, &_state1, 3 },
	{ 8, -1, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY9[] = {
	{ 0, -1, "503b003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY10[] = {
	{  0, 10, "503_010",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 11, 14, "503b007a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 11, 14, "503b007b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 11, 14, "503b007c", 2, 255, -1, 0, 0, &_state1, 3 },
	{ 11, 14, "503b007d", 2, 255, -1, 0, 0, &_state1, 4 },
	{ 15, 21, "503b010a", 2, 255, -1, 0, 0, &_state3, 1 },
	{ 15, 21, "503b010b", 2, 255, -1, 0, 0, &_state3, 2 },
	{ 15, 21, "503b010c", 2, 255, -1, 0, 0, &_state3, 3 },
	{ 22, 23, "500_003",  2, 255, -1, 0, 0, nullptr,  0 },
	{ 24, 29, "503b011a", 2, 255, -1, 0, 0, &_state4, 1 },
	{ 24, 29, "503b011b", 2, 255, -1, 0, 0, &_state4, 2 },
	{ 30, 34, "503b012a", 2, 255, -1, 0, 0, &_state6, 1 },
	{ 30, 34, "503b012b", 2, 255, -1, 0, 0, &_state6, 2 },
	{ 35, 35, nullptr,    0,   0, -1, 0, 0, &_state5, 0 },
	{ 35, 35, nullptr,    0,   0, 18, 0, 0, &_state5, 1 },
	{ 36, 40, nullptr,    0,   0, 11, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY11[] = {
	{  0,  3, nullptr,    0,   0, -1, 0, 0, nullptr, 0 },
	{  4, 11, nullptr,    0,   0, 10, 0, 0, nullptr, 0 },
	{ 12, 15, "503_006",  2, 255, -1, 0, 0, nullptr, 0 },
	{ 16, -1, "503b013a", 2, 255, -1, 0, 0, &_state1, 1 },
	{ 16, -1, "503b013b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 16, -1, "503b013c", 2, 255, -1, 0, 0, &_state1, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY12[] = {
	{ 0, -1, "503b014a", 2, 255, -1, 0, 3, &_state1, 1 },
	{ 0, -1, "503b014b", 2, 255, -1, 0, 3, &_state1, 2 },
	{ 0, -1, "503b014c", 2, 255, -1, 0, 3, &_state1, 3 },
	{ 0, -1, "503b014d", 2, 255, -1, 0, 3, &_state1, 4 },
	{ 0, -1, "503b014e", 2, 255, -1, 0, 3, &_state1, 5 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY13[] = {
	{ 0, -1, "503_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY14[] = {
	{ 0, -1, "503_003", 2, 255, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY15[] = {
	{  0,  7, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  8, 10, "503_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 11, 15, "503_004", 3, 255, -1, 0, 0, nullptr, 0 },
	{ 16, 17, "503b015", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 18, -1, nullptr,   0,   0, 22, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY16[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY17[] = {
	{ 0, 0, nullptr,    0,   0, -1, 0,  6, &_state1, 1 },
	{ 0, 0, nullptr,    0,   0, -1, 0,  6, &_state1, 2 },
	{ 0, 0, nullptr,    0,   0, -1, 0,  6, &_state1, 3 },
	{ 0, 2, "503b017a", 2, 255, -1, 1, -1, &_state1, 4 },
	{ 0, 2, "503b017b", 2, 255, -1, 1, -1, &_state1, 5 },
	{ 0, 2, "503b017c", 2, 255, -1, 1, -1, &_state1, 6 },
	{ 0, 2, "503b017d", 2, 255, -1, 1, -1, &_state1, 7 },
	{ 0, 2, "503b017e", 2, 255, -1, 1, -1, &_state1, 8 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY18[] = {
	{ 0,  4, "503b018a", 2, 255, -1,    0, 0, &_state1, 1 },
	{ 0,  4, "503b018b", 2, 255, -1,    0, 0, &_state1, 2 },
	{ 0,  4, "503b018c", 2, 255, -1,    0, 0, &_state1, 3 },
	{ 0,  4, "503b018d", 2, 255, -1,    0, 0, &_state1, 4 },
	{ 5,  5, "500_001",  3, 255, -1,    0, 0, nullptr, 0 },
	{ 6, -1, nullptr,    2,   0, 12, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY19[] = {
	{  0,  1, "503_006",  2, 255, -1, 0, 0, nullptr,  0 },
	{  2,  8, "503_003",  2, 255, -1, 0, 0, nullptr,  0 },
	{  9, -1, "503b016a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  9, -1, "503b016b", 2, 255, -1, 0, 0, &_state1, 2 },
	{  9, -1, "503b016c", 2, 255, -1, 0, 0, &_state1, 3 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY20[] = {
	{  0,  9, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	{ 10, -1, nullptr,    0,   0,  8, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY21[] = {
	{  0, 23, "503_003", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 24, 35, "503_012", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 36, -1, "503_005", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY22[] = {
	{ 0, 6, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 2, 7, "503_004", 3, 255, -1, 2, 0, nullptr, 0 },
	{ 0, 1, nullptr,   0,   0, 22, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY23[] = {
	{  0, 10, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 11, -1, nullptr, 0, 0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room503::PLAY24[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

int32 Room503::_state1;
int32 Room503::_state2;
int32 Room503::_state3;
int32 Room503::_state4;
int32 Room503::_state5;
int32 Room503::_state6;

Room503::Room503() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
	_state4 = 0;
	_state5 = 0;
	_state6 = 0;
}

void Room503::init() {
	Section5Room::init();
	_flag5 = false;

	for (_val2 = 0; _val2 < 5; ++_val2)
		_array1[_val2] = _array2[_val2] = -1;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 502:
		ws_demand_location(33, 294, 3);

		if (player_been_here(503)) {
			ws_walk(195, 294, nullptr, -1);
		} else {
			_val4 = 9;
			ws_walk(195, 294, nullptr, 13);
		}
		break;

	case 504:
		ws_demand_location(170, 253, 3);
		ws_walk(230, 253, nullptr, -1, 3);
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(263, 333, 3);
		break;
	}

	_state2 = 0;
	_flag1 = false;
	hotspot_set_active("BORK", false);

	if (_G(flags)[kBORK_STATE] == 13) {
		_val6 = 28;
		kernel_trigger_dispatch_now(14);
		kernel_trigger_dispatch_now(15);
	} else if (inv_player_has("ROLLING PIN") && _G(flags)[kBORK_STATE] != 16) {
		_val6 = 27;
		kernel_trigger_dispatch_now(14);
	} else if (_G(flags)[kBORK_STATE] != 16) {
		loadSeries1();
		_flag1 = true;
		_walk1 = intr_add_no_walk_rect(272, 250, 414, 300, 260, 300);
		hotspot_set_active("BORK", false);
		hotspot_set_active_xy("BORK", 340, 250, true);
		_val6 = 14;
		kernel_trigger_dispatch_now(14);
	}

	_state5 = inv_player_has("ROLLING PIN") ? 1 : 0;
	hotspot_set_active("ROLLING PIN", false);
	_flag3 = false;

	if (inv_where_is("RUBBER GLOVES") == 503) {
		hotspot_set_active("RUBBER GLOVES ", true);
		_series1 = series_show("503glove", 0xb00);
	} else {
		hotspot_set_active("RUBBER GLOVES ", false);
	}

	_flag2 = false;
	if (_G(flags)[kBORK_STATE] == 16) {
		_val9 = 34;
	} else if (_G(flags)[kBORK_STATE] != 13) {
		_val9 = 33;
	}

	kernel_trigger_dispatch_now(23);
	if (_G(flags)[V207])
		kernel_trigger_dispatch_now(20);

	_initFlag = true;
	Section5Room::init();
}

void Room503::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		if (_flag5) {
			kernel_trigger_dispatch_now(1);
		} else {
			for (_val2 = 0; _val2 < 5; ++_val2) {
				_array1[_val2] = _array2[_val2];
			}

			kernel_trigger_dispatch_now(2);
		}
		break;

	case 2:
		_flag5 = true;

		for (_val2 = 0; _val2 < 5; ++_val2) {
			if (_array1[_val2] != -1) {
				kernel_trigger_dispatch_now(_array1[_val2]);
				_array1[_val2] = -1;
			}
		}

		_flag5 = false;
		break;

	case 3:
		player_set_commands_allowed(true);
		break;

	case 4:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, _val5);
		break;

	case 5:
		_val7 = 31;
		kernel_trigger_dispatch_now(24);
		_val7 = 32;
		_array1[0] = 24;
		_val6 = 16;
		_array1[1] = 14;
		_G(flags)[V204] = 5000;
		_state1 = imath_ranged_rand(1, 3);
		series_play_with_breaks(PLAY8, "503bk03", 0x900, 2, 3, 8);
		break;

	case 6:
		_val7 = 31;
		kernel_trigger_dispatch_now(24);
		_val9 = 35;
		kernel_trigger_dispatch_now(23);
		_val7 = 32;
		_array1[0] = 24;
		_val6 = 16;
		_array1[1] = 14;

		series_play_with_breaks(PLAY9, "503bk04", 0x900, 2, 3, 8);
		break;

	case 8:
		if (_series4) {
			inv_give_to_player("ROLLING PIN");
			kernel_trigger_dispatch_now(11);
		} else {
			kernel_trigger_dispatch_now(18);
		}
		break;

	case 9:
		inv_give_to_player("RUBBER GLOVES");
		hotspot_set_active("RUBBER GLOVES ", false);
		break;

	case 10:
		_series4 = series_show("503rollp", 0x8ff);
		hotspot_set_active("ROLLING PIN ", true);
		break;

	case 11:
		terminateMachineAndNull(_series4);
		hotspot_set_active("ROLLING PIN ", false);
		break;

	case 13:
		ws_unhide_walker();

		switch (_val4) {
		case 9:
			wilbur_speech("503w001");
			break;
		case 10:
			wilbur_speech("503w002");
			break;
		case 11:
			_val6 = 26;
			_G(wilbur_should) = 4;
			wilbur_speech("500w017", kCHANGE_WILBUR_ANIMATION);
			break;
		case 12:
			player_set_commands_allowed(false);
			wilbur_speech("503w003", 5001);
			break;
		default:
			break;
		}
		break;

	case 14:
		switch (_val6) {
		case 13:
			_G(flags)[kBORK_STATE] = 0;
			_val8 = 1;
			hotspot_set_active("BORK", false);
			hotspot_set_active_xy("BORK", 340, 250, true);
			_val6 = 14;
			series_play_with_breaks(PLAY1, "503bk01", 0x900, 14, 3, 8);
			break;

		case 14:
			_val8 = 2;
			_G(flags)[kBORK_STATE] = 1;
			_val6 = 15;
			series_play_with_breaks(PLAY2, "503bk02", 0x900, 14, 3, 8);
			break;

		case 15:
			if (!digi_play_state(1) && !digi_play_state(2)) {
				if (_G(flags)[V204] == 5001) {
					_G(flags)[V204] = 5000;
					kernel_trigger_dispatch_now(5);
				} else if (_G(flags)[V204] == 5002) {
					_G(flags)[V204] = 5001;
					kernel_trigger_dispatch_now(6);
				} else {
					_val8 = 0;
					_val6 = 18;
					kernel_trigger_dispatch_now(14);
				}
			} else {
				_val6 = 16;
				kernel_trigger_dispatch_now(14);
			}
			break;

		case 16:
			_val6 = 15;
			series_play_with_breaks(PLAY3, "503bk17", 0x900, 14, 3, 8);
			_G(flags)[kBORK_STATE] = 1;
			break;

		case 17:
			++_state2;
			_val7 = 31;
			kernel_trigger_dispatch_now(24);
			_val7 = 32;

			switch (_G(flags)[kBORK_STATE]) {
			case 0:
				_val6 = 14;
				break;
			case 1:
				_val6 = 16;
				break;
			case 4:
			case 6:
			case 10:
				_val6 = 13;
				break;
			default:
				term_message("bork state should not be %d in SNARL_AT_WILBUR!", _G(flags)[kBORK_STATE]);
				break;
			}

			_array2[0] = 14;
			_array2[1] = 24;
			_array2[2] = 3;
			_G(flags)[kBORK_STATE] = 2;
			_state1 = imath_ranged_rand(1, 3);
			_val4 = 10;
			series_play_with_breaks(PLAY4, "503bk05", 0x900, 1, 3, 8);
			break;

		case 18:
			_G(flags)[kBORK_STATE] = 3;
			_val10 = imath_ranged_rand(0, 1);
			_val6 = _val10 ? 21 : 19;
			_state1 = imath_ranged_rand(1, 2);
			series_play_with_breaks(PLAY5, "503bk06", 0x900, 14, 3);
			break;

		case 19:
			hotspot_set_active("BORK", false);
			hotspot_set_active_xy("BORK", 290, 240, true);
			hotspot_set_active_xy("BORK", 290, 260, true);
			_state1 = imath_ranged_rand(1, 2);
			_state3 = imath_ranged_rand(1, 4);
			_state4 = imath_ranged_rand(1, 2);

			if (_G(flags)[V205]) {
				_G(flags)[kBORK_STATE] = 4;
				_val6 = 13;
				series_play_with_breaks(PLAY6, "503bk07", 0x900, 14, 3, 8);
			} else {
				_G(flags)[kBORK_STATE] = 5;
				_G(flags)[V205] = 1;
				_val6 = 20;
				series_play_with_breaks(PLAY7, "503bk07", 0x900, 14, 3, 8);
			}
			break;

		case 20:
			hotspot_set_active("BORK", false);
			hotspot_set_active_xy("BORK", 430, 209, true);
			hotspot_set_active_xy("BORK", 445, 200, true);
			_G(flags)[kBORK_STATE] = 6;
			_G(flags)[V207] = 1;
			_val6 = 13;
			_array1[0] = 14;
			_array1[1] = 20;
			_state1 = imath_ranged_rand(1, 4);
			_state3 = imath_ranged_rand(1, 3);
			_state4 = imath_ranged_rand(1, 2);
			_state6 = imath_ranged_rand(1, 2);
			series_play_with_breaks(PLAY10, "503bk08", 0x8fe, 2, 3, 8, 100, 0, -2);
			break;

		case 21:
			hotspot_set_active("BORK", false);
			hotspot_set_active_xy("BORK", 415, 155, true);
			_G(flags)[kBORK_STATE] = 7;
			_state1 = imath_ranged_rand(1, 3);
			_val6 = 22;
			series_play_with_breaks(PLAY11, "503bk09", 0x900, 14, 3, 6, 100, 0, -2);
			break;

		case 22:
			_G(flags)[kBORK_STATE] = 8;
			_state1 = imath_ranged_rand(1, 5);
			_val6 = 25;
			series_play_with_breaks(PLAY12, "503bk10", 0x900, 14, 2, 6, 100, 0, -2);
			break;

		case 23:
			_G(flags)[kBORK_STATE] = 9;
			_G(flags)[V204] = 5002;
			_val6 = 24;
			series_play_with_breaks(PLAY15, "503bk11", 0x900, 14, 3, 6, 100, 0, -2);
			break;

		case 24:
			kernel_trigger_dispatch_now(11);

			if (_state5) {
				kernel_trigger_dispatch_now(18);
			} else {
				_G(flags)[kBORK_STATE] = 10;
				_val6 = 13;
				series_play_with_breaks(PLAY16, "503bk12", 0x900, 14, 3, 8, 100, 0, -2);
			}
			break;

		case 25:
			hotspot_set_active("BORK", false);
			hotspot_set_active_xy("BORK", 397, 197, true);
			_G(flags)[kBORK_STATE] = 11;
			_val6 = 26;
			series_play_with_breaks(PLAY13, "503bk13", 0x900, 14, 3, 6, 100, 0, -2);
			break;

		case 26:
			_G(flags)[kBORK_STATE] = 12;
			digi_play("503_003", 2);
			_val6 = 27;
			_series2.play("503bk13", 0x900, 1, 14, 6, 6, 100, 0, -2, 15, 17);
			break;

		case 27:
			_G(flags)[kBORK_STATE] = 14;
			_val6 = 23;
			series_play_with_breaks(PLAY14, "503bk13", 0x900, 14, 1, 6, 100, 0, -2);
			hotspot_set_active("BORK", false);
			hotspot_set_active_xy("BORK", 415, 155, true);
			break;

		case 28:
			hotspot_set_active("BORK", false);
			_state1 = imath_ranged_rand(1, 8);
			series_play_with_breaks(PLAY17, "503bk15", 0x900, 14, 7, 6, 100, 0, -2);
			break;

		case 29:
			_G(flags)[kBORK_STATE] = 15;
			_state1 = imath_ranged_rand(1, 3);
			series_play_with_breaks(PLAY19, "503bk14", 0x900, 18, 3, 6, 100, 0, -2);
			break;

		case 30:
			terminateMachineAndNull(_series3);
			_G(flags)[kBORK_STATE] = 16;
			_state1 = imath_ranged_rand(1, 4);
			_val4 = 12;
			_G(wilbur_should) = 1;
			_array1[0] = 10016;
			_val9 = 34;
			_array1[1] = 23;
			_G(flags)[V204] = 5000;
			series_play_with_breaks(PLAY18, "503bk15", 0x900, 2, 7, 6, 100, 0, -2);
			break;

		default:
			break;
		}
		break;

	case 15:
		kernel_timing_trigger(420, 16);
		break;

	case 16:
		if (!_flag4) {
			if (_G(player).walker_visible && player_commands_allowed()) {
				intr_cancel_sentence();
				player_set_commands_allowed(false);
				ws_walk(260, 300, 0, 17, 2, true);
			} else {
				kernel_timing_trigger(15, 16);
			}
		}
		break;

	case 17:
		_val6 = 29;
		break;

	case 18:
		_G(flags)[V206] = 5000;
		kernel_trigger_dispatch_now(5015);
		break;

	case 19:
		_val6 = 30;
		break;

	case 20:
		series_show("503windo", 0xf00, 0, -1, -1, 0, 100, 0, -2);
		break;

	case 21:
		series_play("503sm03", 0xf00);
		break;

	case 22:
		digi_play_loop("503_013", 3, 125);
		break;

	case 23:
		switch (_val9) {
		case 33:
			_series3 = series_show("503micro", 0xa00);
			break;

		case 34:
			series_show("503bk15", 0xa00, 0, -1, -1, 24, 100, 0, -2);
			break;

		case 35:
			terminateMachineAndNull(_series3);
			_val9 = 33;
			_series3 = series_play("503bk15", 0xa00, 1, 23, 6, 0, 100, 0, -2, 0, 2);
			break;
		}
		break;

	case 24:
		switch (_val7) {
		case 31:
			switch (_val8) {
			case 1:
				_series5 = series_show("503prune", 0x900);
				break;
			case 2:
				_series5 = series_show("503smhpr", 0x900);
				break;
			default:
				break;
			}
			break;
		case 32:
			terminateMachineAndNull(_series5);
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
			kernel_trigger_dispatch_now(13);
			break;

		case 2:
			switch (_G(flags)[kBORK_STATE]) {
			case 0:
			case 1:
			case 4:
			case 6:
			case 10:
				_val6 = 17;
				break;
			case 2:
				term_message("Wilbur waiting for bork to snarl at him but bork snarling already");
				break;
			default:
				kernel_timing_trigger(15, kCHANGE_WILBUR_ANIMATION);
				break;
			}
			break;

		case 3:
			_state5 = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY20, "503wi01", 0x800, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_series2.terminate();
			terminateMachineAndNull(_series3);
			_G(wilbur_should) = 10001;
			_array1[0] = 10016;
			_val6 = 28;
			_array1[1] = 14;
			_array1[2] = 15;
			_state1 = imath_ranged_rand(1, 5);
			series_play_with_breaks(PLAY21, "503wi02", 0x800, 2, 3);
			_G(flags)[kBORK_STATE] = 13;
			break;

		case 5:
			player_set_commands_allowed(false);
			_flag4 = true;
			ws_hide_walker();
			_G(wilbur_should) = 6;
			_G(flags)[V204] = 5003;
			series_play_with_breaks(PLAY22, "503wi03", 0x800, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 6:
			ws_unhide_walker();
			ws_walk(260, 300, 0, -1, 2);
			kernel_timing_trigger(imath_ranged_rand(180, 360), 19);
			break;

		case 7:
			terminateMachineAndNull(_series1);
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY23, "503wi06", 0x800, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 8:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY24, "503wi05", 0x800, kCHANGE_WILBUR_ANIMATION, 3, 6);
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

void Room503::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if ((player_said("GEAR", "MICROWAVE") || player_said("TAKE", "MICROWAVE")) &&
			!_flag2 && _G(flags)[kBORK_STATE] != 13 && _G(flags)[kBORK_STATE] != 16) {
		_flag2 = true;
		player_set_commands_allowed(false);
		_G(wilbur_should) = 2;
		player_hotspot_walk_override(260, 300, 2, kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "RUBBER GLOVES ") && !_flag3 &&
			_G(flags)[kBORK_STATE] == 13 && _G(flags)[kBORK_STATE] != 16) {
		_flag3 = true;
		player_set_commands_allowed(false);
		_G(wilbur_should) = 2;
		player_hotspot_walk_override(260, 300, 2, kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("BORK", "ROLLING PIN") &&
			(_G(flags)[kBORK_STATE] == 12 || _G(flags)[kBORK_STATE] == 11)) {
		_flag1 = false;
		intr_remove_no_walk_rect(_walk1);
		_val6 = 26;
		_val4 = 11;
		_G(wilbur_should) = 1;
		player_set_commands_allowed(false);
		player_hotspot_walk_override(346, 283, 2, kCHANGE_WILBUR_ANIMATION);
	} else {
		if (_flag1) {
			player_update_info();
			HotSpotRec *hotspot = hotspot_which(_G(player).click_x, _G(player).click_y);

			if (_G(player).click_x > 272 && _G(player).click_y < 300 && player_said("FLOOR")) {
				ws_walk(_G(player).click_x, 301, nullptr, -1, 1);
			} else if (hotspot->feet_x > 272 && hotspot->feet_y < 300) {
				player_hotspot_walk_override(hotspot->feet_x, 301, 1);
			}
		}

		if (player_said("PARLOUR") && !player_said_any("ENTER", "LOOK AT", "GEAR"))
			player_set_facing_hotspot();

		if (player_said("BASEMENT") && !player_said_any("ENTER", "LOOK AT", "GEAR"))
			player_set_facing_hotspot();
		return;
	}

	_G(player).command_ready = false;
}

void Room503::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	bool borkFlag = player_said("BORK") && _G(flags)[kBORK_STATE] == 12;
	bool microwaveFlag = player_said("MICROWAVE");
	bool ovenFlag = player_said("OVEN") && _G(flags)[kBORK_STATE] == 16;
	bool prunesFlag = player_said("PRUNES") && _G(flags)[kBORK_STATE] == 16;

	if (borkFlag && player_said("LOOK AT")) {
		wilbur_speech("503w005");
	} else if (borkFlag && player_said("GEAR")) {
		wilbur_speech("503w006");
	} else if (player_said("RUBBER DUCKY") && player_said("BORK")) {
		wilbur_speech("500w031");
	} else if (player_said("RUBBER DUCKY") && player_said("SINK")) {
		wilbur_speech("500w034");
	} else if (player_said("SOAPY WATER", "SINK")) {
		wilbur_speech("500w049");
	} else if (microwaveFlag && player_said("LOOK AT") && _G(flags)[kBORK_STATE] == 16) {
		wilbur_speech("503w011");
	} else if (microwaveFlag && player_said("LOOK AT") && _G(flags)[kBORK_STATE] == 13) {
		wilbur_speech("503w010");
	} else if (microwaveFlag && player_said("TAKE") && _G(flags)[kBORK_STATE] == 16) {
		wilbur_speech("503w013");
	} else if (microwaveFlag && player_said("TAKE") && _flag2) {
		wilbur_speech("503w013");
	} else if (microwaveFlag && player_said("GEAR") && _G(flags)[kBORK_STATE] == 16) {
		wilbur_speech("503w013");
	} else if (microwaveFlag && player_said("GEAR") && _flag2 && _G(flags)[kBORK_STATE] != 13) {
		wilbur_speech("503w012");
	} else if (player_said("RUBBER GLOVES ") && player_said("TAKE") &&
			_G(flags)[kBORK_STATE] != 16 && _G(flags)[kBORK_STATE] != 13 && _flag3) {
		wilbur_speech("503w012");
	} else if (player_said("RUBBER GLOVES ") && player_said("GEAR")) {
		wilbur_speech("503w015");
	} else if (ovenFlag && player_said("LOOK AT")) {
		wilbur_speech("503w021");
	} else if (ovenFlag && player_said("GEAR")) {
		wilbur_speech("503w023");
	} else if (player_said("GEAR", "OVEN") && _G(flags)[kBORK_STATE] == 16 &&
			_G(flags)[kBORK_STATE] != 13) {
		wilbur_speech("503w022");
	} else if (player_said("LOOK AT", "CUPBOARD ") && inv_player_has("RUBBER GLOVES")) {
		wilbur_speech("503w027");
	} else if (prunesFlag && player_said("LOOK AT")) {
		wilbur_speech("503w030");
	} else if (prunesFlag && (player_said("TAKE") || player_said("GEAR"))) {
		wilbur_speech("503w032");
	} else if (player_said("GEAR", "WINDOW") && _G(flags)[V207] != 0) {
		wilbur_speech("503w033");
	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("TAKE", "ROLLING PIN ")) {
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "RUBBER GLOVES ") &&
			(_G(flags)[kBORK_STATE] == 16 || _G(flags)[kBORK_STATE] == 13)) {
		_G(wilbur_should) = 7;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("BORK") && player_said("LOOK AT") &&
			(_G(flags)[kBORK_STATE] == 0 || _G(flags)[kBORK_STATE] == 1) &&
			!player_said_any("GIZMO", "ROLLING PIN", "SOCK", "SOAPY WATER", "RUBBER_GLOVES") &&
			!player_said("LAXATIVE")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("GEAR", "MICROWAVE") && _G(flags)[kBORK_STATE] == 13) {
		_G(wilbur_should) = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("GEAR", "OVEN") && (_G(flags)[kBORK_STATE] == 16 || _G(flags)[kBORK_STATE] == 13)) {
		_G(wilbur_should) = 8;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("PARLOUR") && player_said_any("ENTER", "LOOK AT", "GEAR")) {
		term_message("Going into 502!");
		_val5 = 5006;
		kernel_trigger_dispatch_now(4);
	} else if (player_said("BASEMENT") && player_said_any("ENTER", "LOOK AT", "GEAR")) {
		term_message("Going into 504!");
		_val5 = 5008;
		kernel_trigger_dispatch_now(4);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room503::loadSeries1() {
	static const char *NAMES[] = {
		"503bk01", "503bk01s", "503bk02", "503bk02s", "503bk17",
		"503bk17s", "503bk06", "503bk06s", "503bk07", "503bk07s",
		"503bk06", "503bk06s", "503bk09", "503bk09s", "503bk10",
		"503bk11", "503bk11s", "503bk12", "503bk13", "503bk13s"
	};
	for (int i = 0; i < 20; ++i)
		series_load(NAMES[i]);

	if (!_G(flags)[V207]) {
		series_load("503bk08");
		series_load("503bk08s");
		series_load("503windo");
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
