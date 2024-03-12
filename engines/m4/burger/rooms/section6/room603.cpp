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

#include "m4/burger/rooms/section6/room603.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const Section6Room::GerbilPoint Room603::GERBIL[] = {
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 429, 183 },
	{ 429, 185 }, { 429, 185 }, { 436, 200 }, { 436, 200 },
	{ 436, 200 }, { 422, 188 }, { 422, 188 }, { 422, 188 },
	{ 417, 192 }, { 417, 192 }, { 417, 192 }, { 411, 190 },
	{ 411, 190 }, { 411, 190 }, { 411, 190 }, { 411, 190 },
	{ 411, 190 }, { 403, 182 }, { 403, 182 }, { 403, 182 },
	{ 339, 177 }, { 339, 177 }, { 339, 177 }, { 301, 195 },
	{ 301, 195 }, { 301, 195 }, { 295, 199 }, { 439, 189 },
	{ 439, 189 }, { 295, 199 }, { 431, 190 }, { 431, 190 },
	{ 295, 199 }, { 413, 193 }, { 413, 193 }, { 290, 196 },
	{ 403, 202 }, { 403, 202 }, { 293, 195 }, { 392, 203 },
	{ 392, 203 }, { 293, 195 }, { 389, 222 }, { 389, 222 },
	{ 295, 193 }, { 415, 230 }, { 423, 185 }, { 293, 196 },
	{ 428, 235 }, { 415, 185 }, { 293, 196 }, { 430, 233 },
	{ 420, 204 }, { 293, 196 }, { 430, 233 }, { 420, 204 },
	{ 293, 196 }, { 430, 232 }, { 420, 204 }, { 293, 196 },
	{ 438, 233 }, { 411, 199 }, { 293, 196 }, { 441, 235 },
	{ 408, 198 }, { 293, 196 }, { 444, 235 }, { 403, 197 },
	{ 293, 196 }, { 447, 237 }, { 402, 195 }, { 293, 196 },
	{ 450, 238 }, { 399, 194 }, { 293, 196 }, { 452, 237 },
	{ 397, 194 }, { 293, 196 }, { 455, 235 }, { 395, 194 },
	{ 293, 196 }, { 458, 233 }, { 393, 195 }, { 293, 196 },
	{ 461, 233 }, { 392, 195 }, { 293, 196 }, { 461, 233 },
	{ 392, 195 }, { 293, 196 }, { 461, 233 }, { 392, 195 }
};

static const char *SAID[][4] = {
	{ "WATER DISH", nullptr,   "603w004", "603w005" },
	{ "WATER",      "603w006", nullptr,   nullptr   },
	{ "CARROT",     "603w008", "603w009", "603w010" },
	{ "TUBE",       "603w011", "603w005", nullptr   },
	{ "TUBE ",      "603w011", "603w005", nullptr   },
	{ "TUBE  ",     "603w012", "603w005", nullptr   },
	{ "TUBE   ",    "603w012", "603w005", nullptr   },
	{ "BARS",       "603w013", "603w005", "603w005" },
	{ "FLOOR",      "603w014", "603w005", "603w005" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room603::PLAY1[] = {
	{ 0,  0, "603_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 1, -1, "603w001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY2[] = {
	{  0, 22, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 23, 24, "600_015", 2, 255, -1,    0, 0, nullptr, 0 },
	{ 25, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY3[] = {
	{ 0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 4,  5, "600w016", 2, 100, -1,    0, 0, nullptr, 0 },
	{ 6, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY4[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY5[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w015", 2, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY6[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w015", 2, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY7[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "603_004", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY8[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "603_006", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY9[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "600_014", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


const seriesPlayBreak Room603::PLAY10[] = {
	{ 0,  6, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	{ 7, -1, nullptr, 1, 0,  0,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY11[] = {
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 0 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 1 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 2 },
	{ 7, -1, "600w011a", 1, 255, -1,    0, 0, &_state1, 0 },
	{ 7, -1, "600w011b", 1, 255, -1,    0, 0, &_state1, 1 },
	{ 7, -1, "600w011c", 1, 255, -1,    0, 0, &_state1, 2 },
	{ 0, -1, "600w011d", 1, 255, -1,    0, 0, &_state1, 3 },
	{ 0, -1, "600w011e", 1, 255, -1,    0, 0, &_state1, 4 },
	{ 0, -1, "600w011f", 1, 255, -1,    0, 0, &_state1, 5 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY12[] = {
	{  0, 13, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 14, -1, "600_012", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY13[] = {
	{ 0,  5, nullptr,   1,   0, -1, 2048,  0, nullptr, 0 },
	{ 6,  6, "603w007", 1, 255, -1,    0, 10, nullptr, 0 },
	{ 5, -1, nullptr,   1, 255, -1,    0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY14[] = {
	{  0, 24, nullptr, 1,   0, -1, 2048, 0, nullptr, 0 },
	{ 25, -1, nullptr, 1, 255,  0,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY15[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

int32 Room603::_state1;

Room603::Room603() : Section6Room() {
	_gerbilTable = GERBIL;
	_state1 = 0;
}

void Room603::init() {
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;
	_G(flags)[kStandingOnKibble] = 0;

	if (_G(flags)[V269] == 1)
		series_show("602spill", 0x900, 0, -1, -1, 0, 100, 80, 0);
	_G(kernel).call_daemon_every_loop = _G(flags)[V269] == 1;

	if (_G(flags)[V270] == 6000) {
		hotspot_set_active("WATER", true);
		_val1 = 22;
		_series1 = series_show("603hole", 0xfff);
	} else {
		hotspot_set_active("WATER", false);
		_val1 = 27;
		kernel_trigger_dispatch_now(0);
	}

	if (_G(flags)[V245] == 10029) {
		Section6::_state1 = 6002;
		kernel_trigger_dispatch_now(6013);
	}

	if (_G(flags)[kHampsterState] == 6000) {
		Section6::_state4 = 1;
		kernel_trigger_dispatch_now(6014);
	}
	if (_G(flags)[kHampsterState] == 6006) {
		_series2 = series_load("603mg01");
		Section6::_gerbilState = 6001;
		kernel_timing_trigger(60, 6011);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 602:
	case 612:
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 604:
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(203, 333);
		break;
	}

	if (_G(flags)[kHampsterState] == 6006)
		Section6::_state4 = 5;

	if (_G(flags)[kHampsterState] == 6007)
		Section6::_state4 = 8;
	else if (_G(flags)[kHampsterState] == 6000)
		Section6::_state4 = 1;

	kernel_trigger_dispatch_now(6014);
}

void Room603::daemon() {
	switch (_G(kernel).trigger) {
	case 0:
		switch (_val1) {
		case 22:
			_series1 = series_show("603hole", 0xfff);
			break;

		case 23:
			hotspot_set_active("WATER", false);
			terminateMachineAndNull(_series1);
			_G(flags)[V270] = 6001;

			if (_G(flags)[V277] == 6001) {
				_G(wilbur_should) = 20;
				_val1 = 24;
				series_play_with_breaks(PLAY14, "603hole", 0xfff, -1, 6);
			} else {
				_G(wilbur_should) = 20;
				_val1 = 27;
				series_play_with_breaks(PLAY15, "603hole", 0xfff, kCHANGE_WILBUR_ANIMATION, 2);
			}

			_roomSeries1.play("603wi20", 0x400, 2, kCHANGE_WILBUR_ANIMATION, 0, 0, 100, 0, 0, 0, 17);
			digi_play("603_005", 2, 255, -1, 603);
			break;

		case 24:
			_val1 = 27;
			digi_play("603_007", 2);
			series_play("603motor", 0xbff, 0, kCHANGE_WILBUR_ANIMATION);
			series_show("603hole", 0xfff, 0, -1, -1, 41);
			_G(flags)[V277] = 6002;
			break;

		case 25:
			inv_put_thing_in("KIBBLE", 602);

			if (_G(flags)[V270] == 6001) {
				digi_play("603_009", 2, 255, -1, 603);
			} else {
				digi_play("603_008", 2, 255, -1, 603);
				series_play("603full", 0xc00, 0, -1);
			}
			break;

		case 27:
			_series1 = series_show("603hole", 0xfff, 0, -1, -1, 41);
			break;

		default:
			break;
		}
		break;

	case 1:
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		kernel_trigger_dispatch_now(2);

		term_message("Kibble is now being removed.");
		inv_move_object("KIBBLE", NOWHERE);
		break;

	case 2:
		_G(kernel).call_daemon_every_loop = true;
		series_show("602spill", 0x900, 0, -1, -1, 0, 100, 80, 0);
		kernel_trigger_dispatch_now(3);
		break;

	case 3:
		term_message("Kibble is now being removed.");
		inv_move_object("KIBBLE", NOWHERE);
		break;

	case 6002:
		_G(game).new_room = 602;
		break;

	case 6004:
		_G(game).new_room = 604;
		break;

	case 6005:
		_G(game).new_room = 605;
		break;

	case 6010:
		_G(game).new_room = 612;
		break;

	case 6011:
		switch (Section6::_gerbilState) {
		case 6001:
			Section6::_gerbilState = 6002;
			_sectionMachine1 = series_play("603mg01", 0xdff, 0, 6011, 8, 0, 100, 0, 0, 0, 84);
			_sectionMachine2 = series_play("603mg01s", 0xe00, 0, -1, 8, 0, 100, 0, 0, 0, 84);
			break;

		case 6002:
			Section6::_state4 = 4;
			kernel_trigger_dispatch_now(6014);
			Section6::_gerbilState = 6003;

			_sectionMachine1 = series_play("603mg01", 0xdff, 0, 6011, 8, 0, 100, 0, 0, 85, -1);
			_sectionMachine2 = series_play("603mg01s", 0xe00, 0, -1, 8, 0, 100, 0, 0, 85, -1);
			break;

		case 6003:
			Section6::_gerbilState = (_G(flags)[V245] == 10029) ? 6005 : 6004;
			series_play("603mg01", 0xdff, 0, -1, -1, 95);
			_sectionMachine2 = series_play("603mg01s", 0xe00, 0, -1, -1, 95);
			kernel_trigger_dispatch_now(6011);
			break;

		case 6004:
			player_set_commands_allowed(false);

			if (_G(flags)[V246] == 0)
				kernel_trigger_dispatch_now(6005);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 2:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(false);
			ws_demand_location(215, 311, 3);
			ws_hide_walker();
			series_play_with_breaks(PLAY1, "603wi02", 0x7ff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 3:
			_G(wilbur_should) = 10001;
			player_set_commands_allowed(false);
			ws_demand_location(400, 280, 7);
			ws_hide_walker();
			series_play_with_breaks(PLAY2, "603wi06", 0xdff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			term_message("Climbing up to 604...");
			ws_demand_facing(10);
			player_set_commands_allowed(false);
			ws_hide_walker();
			Section6::_state2 = 0;

			series_play_with_breaks(PLAY3, "603wi03", 0xdff, 6004, 3);
			break;

		case 5:
			term_message("Climbing down to 602...");
			player_set_commands_allowed(false);
			ws_demand_facing(8);
			ws_hide_walker();
			Section6::_state2 = 1;
			_G(roomVal7) = 1;

			if (_G(flags)[kHampsterState] == 6007) {
				series_play_with_breaks(PLAY4, "603wi01", 0xdff, 6010, 3);
			} else {
				series_play_with_breaks(PLAY4, "603wi01", 0xdff, 6002, 3);
			}
			break;

		case 6:
			term_message("Climbing up to 604...");
			ws_demand_facing(1);
			player_set_commands_allowed(false);
			ws_hide_walker();
			Section6::_state2 = 2;

			series_play_with_breaks(PLAY5, "603wi04", 0xdff, 6004, 3);
			break;

		case 7:
			ws_demand_facing(1);
			player_set_commands_allowed(false);
			ws_hide_walker();
			Section6::_state2 = 2;
			if (_G(flags)[kHampsterState] == 6007) {
				series_play_with_breaks(PLAY6, "603wi05", 0xdff, 6010, 3);
			} else {
				series_play_with_breaks(PLAY6, "603wi05", 0xdff, 6002, 3);
			}
			break;

		case 8:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);

			if (_G(flags)[V270] == 6000) {
				_val1 = 23;
				ws_hide_walker();
				series_play_with_breaks(PLAY7, "603wi20", 0x400, 0, 3);
			} else {
				wilbur_speech("603w017");
				_G(wilbur_should) = 10001;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			}
			break;

		case 9:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 17;
			series_play_with_breaks(PLAY8, "603wi10", 0x900, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 10:
			_G(flags)[V247] = 1;
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY9, "603wi40", 0x700, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 11:
			player_set_commands_allowed(false);
			ws_demand_facing(2);
			ws_hide_walker();
			_G(wilbur_should) = 19;
			_val1 = 25;
			series_play_with_breaks(PLAY10, "603wi08", 0xbff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 12:
			term_message("Skip on kibble!");
			player_set_commands_allowed(false);
			player_update_info();
			_G(wilbur_should) = 13;

			ws_walk(_G(player_info).x + 1, 305, nullptr, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 13:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 18;
			Section6::_savedX = _G(player_info).x - 425;
			_state1 = imath_ranged_rand(0, 5);

			series_play_with_breaks(PLAY11, "602wi13", _G(player_info).depth,
				kCHANGE_WILBUR_ANIMATION, 3, 6, 100, Section6::_savedX + 80, 0);
			break;

		case 14:
			ws_demand_location(502, 301, 9);
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(flags)[V269] = 1;

			series_play_with_breaks(PLAY12, "602wi20", 0x901, 1, 3, 6, 100, 80, 0);
			break;

		case 15:
			if (_G(flags)[kHampsterState] != 6006) {
				_G(wilbur_should) = 10001;
				player_set_commands_allowed(false);
				ws_demand_facing(2);
				ws_hide_walker();

				series_play_with_breaks(PLAY13, "603wi07", 0xbff, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 16:
			_G(wilbur_should) = 10001;

			if (_G(flags)[kHampsterState] == 6007) {
				wilbur_speech(_G(flags)[V279] ? "603w016" : "603w015");
			} else {
				kernel_trigger_dispatch_now(0);
				wilbur_speech("603w021");
			}
			break;

		case 17:
			_G(wilbur_should) = 10002;
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("603w018");
			break;

		case 18:
			_G(wilbur_should) = 10002;
			player_set_commands_allowed(true);
			ws_unhide_walker();
			wilbur_speech("600w012");
			break;

		case 19:
			_G(wilbur_should) = 10002;
			player_set_commands_allowed(true);
			ws_unhide_walker();

			if (_G(flags)[V270] == 6000)
				wilbur_speech("603w020");
			break;

		case 20:
			ws_unhide_walker();
			_G(wilbur_should) = 16;
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCALLED_EACH_LOOP:
		player_update_info();

		if (_G(player_info).x > 420 && _G(player_info).x < 490 &&
				_G(player_info).y > 284 && _G(player_info).y < 305 &&
				_G(player_info).facing > 2 && _G(player_info).facing < 7 &&
				_G(flags)[V269] == 1) {
			if (_G(flags)[kStandingOnKibble]) {
				_G(flags)[kStandingOnKibble] = 1;
			} else {
				_G(flags)[kStandingOnKibble] = 1;
				term_message("Wilbur now slips on kibble!");
				intr_cancel_sentence();
				_G(wilbur_should) = 12;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			}
		} else {
			_G(flags)[kStandingOnKibble] = 0;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room603::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[kHampsterState] == 6006 && (player_said("TUBE ") || player_said("TUBE  "))) {
		term_message("Can't leave through back tube as gerbils are in the way.");
		wilbur_speech("600w003");
		intr_cancel_sentence();

	} else if (player_said("GEAR", "WATER") || player_said("TAKE", "WATER")) {
		_G(wilbur_should) = 15;
		player_hotspot_walk_override(272, 271, 2, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("KIBBLE", "FLOOR")) {
		if (_G(flags)[V269]) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = false;
			wilbur_speech("600w008z");
		} else {
			_sectionSeries1 = series_load("602wi20");
			Section6::_series603 = series_load("602wi20s");
			_G(wilbur_should) = 14;
			player_hotspot_walk_override(502, 301, 9, kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("RAY GUN", "GERBILS")) {
		_G(wilbur_should) = 10;
		player_hotspot_walk_override(226, 301, 2, kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("WATER DISH") || player_said("WATER")) {
		if (_G(flags)[kHampsterState] == 6006) {
			wilbur_speech("600w003");
			intr_cancel_sentence();
		} else if (player_said("KIBBLE")) {
			_G(wilbur_should) = 11;
			player_hotspot_walk_override(261, 274, 2, kCHANGE_WILBUR_ANIMATION);
		} else if (player_said("RAY GUN", "WATER DISH")) {
			if (_G(flags)[V277] == 6001 && _G(flags)[V270] == 6000)
				_series4 = series_load("603motor");

			_G(wilbur_should) = 8;
			player_hotspot_walk_override(180, 329, 2, kCHANGE_WILBUR_ANIMATION);
		} else if (player_said("RAY GUN", "WATER") && _G(flags)[V270] == 6000) {
			_G(wilbur_should) = 9;
			player_hotspot_walk_override(350, 282, 10, kCHANGE_WILBUR_ANIMATION);
		} else {
			return;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room603::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if ((player_said("WATER DISH") || player_said("WATER")) &&
			player_said("LOOK AT")) {
		wilbur_speech(_G(flags)[V270] == 6000 ? "603w002" : "603w003");

	} else if (player_said("GEAR", "TUBE ") || player_said("CLIMB IN", "TUBE ")) {
		_G(flags)[V246] = 1;
		_G(wilbur_should) = 6;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "TUBE  ") || player_said("CLIMB IN", "TUBE  ")) {
		_G(flags)[V246] = 1;
		_G(wilbur_should) = 7;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "TUBE") || player_said("CLIMB IN", "TUBE")) {
		_G(flags)[V246] = 1;
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("GEAR", "TUBE   ") || player_said("CLIMB IN", "TUBE   ")) {
		_G(flags)[V246] = 1;
		_G(wilbur_should) = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "CARROT")) {
		wilbur_speech("603w019");

	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
