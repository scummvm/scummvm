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

#include "m4/burger/rooms/section5/room506.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room506::SERIES1[] = {
	{   0, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  13, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  20, "506_006",  1, 255, -1, 0, nullptr,  0 },
	{  45, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  67, "506_003",  2, 100, -1, 0, nullptr,  0 },
	{  77, "506_003",  2, 255, -1, 0, nullptr,  0 },
	{  91, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 100, "506b005a", 1, 255, -1, 0, &_state1, 1 },
	{ 100, "506b005b", 1, 255, -1, 0, &_state1, 2 },
	{ 100, "506b005c", 1, 255, -1, 0, &_state1, 3 },
	{ 113, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 135, "506_007",  2, 255, -1, 0, nullptr,  0 },
	{ 155, "506_001",  2, 255, -1, 0, nullptr,  0 },
	{ 165, nullptr,    0,   0, 11, 0, nullptr,  0 },
	STREAM_BREAK_END
};

const char *Room506::SAID[][4] = {
	{ "HALLWAY",        nullptr,   "500w001", nullptr   },
	{ "WINDOW",         "500w003", "500w001", nullptr   },
	{ "ROOF",           "500w003", "500w001", nullptr   },
	{ "BORK",           "506w004", "500w002", "500w002" },
	{ "TELEVISION",     "506w005", nullptr,   "506w007" },
	{ "FIRE",           "506w009", "506w010", "506w011" },
	{ "VIDEO GAME",     "506w012", "506w013", "506w015" },
	{ "BED",            "506w017", nullptr,   "506w018" },
	{ "MATTRESS",       "506w017", nullptr,   "506w018" },
	{ "READING LAMP",   "506w019", "506w020", "506w021" },
	{ "NIGHT TABLE",    "506w022", "500w005", "500w005" },
	{ "PILLOW",         "506w023", "506w024", "506w025" },
	{ "LAMP",           "506w026", "506w020", "506w021" },
	{ "BOOK",           "506w027", "506w028", "506w028" },
	{ "WARDROBE",       "506w029", nullptr,   nullptr   },
	{ "CLOTHES",        "506w030", "506w031", "506w032" },
	{ "DRESSER",        "506w033", nullptr,   nullptr   },
	{ "MODEL ROCKET ",  "506w034", "500w005", "500w005" },
	{ "MODEL ROCKET",   "506w035", "506w036", "500w005" },
	{ "MODEL ROCKET  ", "506w037", "500w005", "500w005" },
	{ "POSTER ",        "506w038", "500w005", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room506::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY2[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY3[] = {
	{  0, 25, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 26, 40, nullptr,   0,   0,  5, 0,  0, nullptr, 0 },
	{ 41, 46, "500w067", 1, 255, -1, 4, -1, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY4[] = {
	{  9, 22, "506_003",  2,  50, -1, 0, 0, nullptr,  0 },
	{ 23, 28, "506b003a", 2, 255, -1, 4, 2, &_state1, 1 },
	{ 23, 28, "506b003b", 2, 255, -1, 4, 2, &_state1, 2 },
	{ 29, 37, nullptr,    0,   0, -1, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY5[] = {
	{ 38, 38, "506_003",  2, 120, -1, 0, 12, nullptr,  0 },
	{ 39, 73, "506b004a", 2, 255, -1, 0,  0, &_state1, 1 },
	{ 39, 73, "506b004b", 2, 255, -1, 0,  0, &_state1, 2 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room506::PLAY6[] = {
	{  0, 12, "506b002a", 2, 255, -1, 0, 0, &_state1, 1 },
	{  0, 12, "506b002b", 2, 255, -1, 0, 0, &_state1, 2 },
	{ 13, 13, nullptr,    0,   0,  9, 0, 0, &_state2, 3 },
	{ 13, 30, nullptr,    0,   0,  6, 0, 0, nullptr,  0 },
	{ 31, -1, nullptr,    0,   0,  2, 0, 0, nullptr,  0 },
	PLAY_BREAK_END
};


int32 Room506::_state1;
int32 Room506::_state2;

Room506::Room506() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
}

void Room506::init() {
	Section5Room::init();
	pal_cycle_init(112, 127, 6);

	for (_ctr = 0; _ctr < 5; ++_ctr)
		_triggers[_ctr] = -1;

	if (_G(flags)[kTVOnFire]) {
		kernel_trigger_dispatch_now(12);
	} else {
		hotspot_set_active("FIRE", false);
	}

	if (_G(flags)[kTVOnFire] == 0) {
		_val1 = 19;
		kernel_trigger_dispatch_now(10);
	}

	if (_G(flags)[kTVOnFire]) {
		series_show("506windo", 0xf00);
	} else {
		hotspot_set_active("ROOF", false);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 505:
		if (_G(flags)[V218] != 5003)
			_G(flags)[V218] = 5000;

		player_set_commands_allowed(true);
		ws_demand_location(543, 252, 9);

		if (player_been_here(506)) {
			ws_walk(434, 254, nullptr, -1, 9);
		} else {
			ws_walk(434, 254, nullptr, 6, 9);
		}
		break;

	case 508:
		if (_G(flags)[V218] != 5003)
			_G(flags)[V218] = 5000;

		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 510:
		ws_demand_location(_G(flags)[V187], _G(flags)[V188], _G(flags)[V189]);
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(434, 254, 9);
		break;
	}

	_state2 = 0;

	if (_G(flags)[V218] == 5003) {
		hotspot_set_active("BORK", false);
	} else {
		loadSeries();
		_val2 = 12;
		kernel_trigger_dispatch_now(7);
		_walk1 = intr_add_no_walk_rect(308, 278, 430, 314, 303, 328);
	}

	_initFlag = true;
	Section5Room::init();
}

void Room506::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		ws_walk(434, 254, nullptr, 2, 9);
		break;

	case 2:
		player_set_commands_allowed(true);
		break;

	case 3:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, _destTrigger);
		break;

	case 4:
		for (_ctr = 0; _ctr < 5; ++_ctr) {
			if (_triggers[_ctr] != -1) {
				kernel_trigger_dispatch_now(_triggers[_ctr]);
				_triggers[_ctr] = -1;
			}
		}
		break;

	case 5:
		inv_put_thing_in("KINDLING", NOWHERE);
		break;

	case 6:
		ws_unhide_walker();

		switch (_val4) {
		case 9:
			wilbur_speech("506w001");
			break;

		case 10:
			wilbur_speech("506w002");
			break;

		case 11:
			wilbur_speech("506w003", 5001);
			break;

		default:
			break;
		}
		break;

	case 7:
		switch (_val2) {
		case 12:
			kernel_trigger_dispatch_now(8);
			_G(flags)[V218] = 5000;
			_val2 = imath_ranged_rand(0, 2) == 0 ? 14 : 13;
			_loopCount = imath_ranged_rand(2, 4);

			series_play("506bk01", 0x3ff, 0, 7, 6, _loopCount, 100, 6, 10, 0, 8);
			series_play("506bk01s", 0x400, 0, -1, 6, _loopCount, 100, 6, 10, 0, 8);
			break;

		case 13:
			_G(flags)[V218] = 5001;
			_val1 = 20;
			kernel_trigger_dispatch_now(10);

			_state1 = imath_ranged_rand(1, 2);
			_val2 = 12;
			series_play_with_breaks(PLAY4, "506bk01", 0x3ff, 7, 3, 6, 100, 6, 10);
			break;

		case 14:
			_G(flags)[V218] = 5002;
			_val1 = 22;
			kernel_trigger_dispatch_now(10);

			_state1 = imath_ranged_rand(1, 2);
			_val2 = 12;
			series_play_with_breaks(PLAY5, "506bk01", 0x3ff, 7, 3, 6, 100, 6, 10);
			break;

		case 15:
			_state1 = imath_ranged_rand(1, 2);
			_val4 = 10;
			_val2 = 16;
			series_play_with_breaks(PLAY6, "506bk02", 0x3ff, 7, 3, 6, 100, 6, 10);
			break;

		case 16:
			_val2 = 12;
			kernel_trigger_dispatch_now(7);
			player_set_commands_allowed(true);
			break;

		case 17:
			_G(wilbur_should) = 5;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 18:
			intr_remove_no_walk_rect(_walk1);
			hotspot_set_active("BORK", false);
			break;

		default:
			break;
		}
		break;

	case 8:
		digi_play(Common::String::format("506b001%c", 'a' + imath_ranged_rand(0, 5)).c_str(), 2);
		break;

	case 9:
		_G(flags)[V206] = 5004;
		kernel_trigger_dispatch_now(5015);
		break;

	case 10:
		switch (_val1) {
		case 19:
			term_message("The game is Defender!");
			_series1 = series_play("506game", 0x2fe, 2, -1, 10, -1, 100, 0, 0, 5, 21);
			break;

		case 20:
			term_message("There is an explosion on the screen!");
			terminateMachineAndNull(_series1);

			_val1 = 19;
			_series1 = series_play("506game", 0x2fe, 1, 10, 10, 3, 100, 0, 0, 2, 3);
			break;

		case 21:
			term_message("There is a really big explosion on the screen!");
			_series1 = series_play("506game", 0x2fe, 1, 10, 10, 3, 100, 0, 0, 2, 4);
			break;

		case 22:
			terminateMachineAndNull(_series1);
			_val1 = 21;
			_series1 = series_play("506game", 0x2fe, 1, 10, 10, 1, 100, 0, 0, 0, 1);
			break;

		default:
			break;
		}
		break;

	case 11:
		digi_play_loop("500_002", 3, 125);
		break;

	case 12:
		_G(flags)[kTVOnFire] = 1;
		terminateMachineAndNull(_series1);
		hotspot_set_active("JOYSTICK", false);
		hotspot_set_active("FIRE", true);
		_series2 = series_play("506fire", 0x2fe, 4, -1, 6, -1);
		break;

	case 13:
		series_show("506windo", 0xf00);
		_G(flags)[kTVOnFire] = 1;
		hotspot_set_active("ROOF", true);
		break;

	case 5002:
		_G(wilbur_should) = 10001;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(true);
			kernel_trigger_dispatch_now(6);
			break;

		case 2:
			player_set_commands_allowed(false);
			ws_hide_walker();

			_destTrigger = 5012;
			kernel_timing_trigger(180, 3);
			series_play_with_breaks(PLAY1, "506wi01", 0x800, -1, 3);
			break;

		case 3:
			ws_demand_location(157, 313, 3);
			player_set_commands_allowed(false);
			ws_hide_walker();

			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY2, "506wi02", 0x800, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 4:
			player_set_commands_allowed(false);
			_val2 = 17;
			break;

		case 5:
			terminateMachineAndNull(_series1);
			ws_hide_walker();

			_G(flags)[V218] = 5003;
			_G(flags)[V220] = 1;
			_state1 = imath_ranged_rand(1, 3);
			_G(wilbur_should) = 6;
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "506wi03", 6, 0x100, kCHANGE_WILBUR_ANIMATION);
			break;

		case 6:
			digi_unload_stream_breaks(SERIES1);
			_val2 = 18;
			kernel_trigger_dispatch_now(7);
			kernel_trigger_dispatch_now(12);
			kernel_trigger_dispatch_now(13);
			_val4 = 11;
			kernel_trigger_dispatch_now(6);
			break;

		case 7:
			player_set_commands_allowed(false);
			ws_hide_walker();

			_destTrigger = 5006;
			_triggers[0] = 3;
			_triggers[1] = kCHANGE_WILBUR_ANIMATION;
			_G(wilbur_should) = 8;
			series_play_with_breaks(PLAY3, "506wi05", 0x2fe, 4, 3);
			break;

		case 8:
			series_show("506wi05", 0x2fe, 0, -1, -1, 46);
			series_show("506wi05s", 0x2ff, 0, -1, -1, 46);
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

void Room506::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("AMPLIFIER", "TELEVISION") && _G(flags)[V218] != 5003) {
		_G(wilbur_should) = 4;
		player_hotspot_walk_override(456, 245, 8, kCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	} else if (player_said("FIRE", "KINDLING")) {
		_G(wilbur_should) = 7;
		player_hotspot_walk_override(409, 311, 4, kCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	} else if (player_said("HALLWAY") && !player_said_any("LEAVE", "LOOK AT", "GEAR")) {
		player_set_facing_hotspot();
	}
}

void Room506::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("GEAR") && player_said_any("WINDOW", "ROOF") && !_G(flags)[kTVOnFire]) {
		wilbur_speech("500w004");
	} else if (_G(flags)[kTVOnFire] && player_said("TELEVISION") && player_said("LOOK AT")) {
		wilbur_speech("506w006");
	} else if (_G(flags)[kTVOnFire] && player_said("TELEVISION") && player_said("GEAR")) {
		wilbur_speech("506w008");
	} else if (_G(flags)[kTVOnFire] && player_said("VIDEO GAME") &&
			player_said_any("TAKE", "GEAR")) {
		// No implementation
	} else if (_G(walker).wilbur_said(SAID)) {
		// No implementation
	} else if (player_said("BORK") && player_said("LOOK AT") &&
			!player_said_any("GIZMO", "ROLLING PIN", "DIRTY SOCK", "SOAPY WATER", "RUBBER GLOVES") &&
			!player_said("LAXATIVE")) {
		player_set_commands_allowed(false);
		_val2 = 15;
		++_state2;
	} else if (player_said_any("ROOF", "WINDOW") && player_said_any("ENTER", "GEAR") &&
			_G(flags)[kTVOnFire]) {
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("HALLWAY") && player_said_any("LEAVE", "LOOK AT", "GEAR")) {
		_destTrigger = 5009;
		kernel_trigger_dispatch_now(3);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room506::loadSeries() {
	series_load("506bk01");
	series_load("506bk01s");
	series_load("506bk02");
	series_load("506bk02s");
	series_load("506bk03");
	series_load("506bk03s");
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
