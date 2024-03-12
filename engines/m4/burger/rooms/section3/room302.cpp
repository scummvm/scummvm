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

#include "m4/burger/rooms/section3/room302.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room302::SAID[][4] = {
	{ "DOOR",           nullptr,   "300w001", nullptr   },
	{ "STORM CELLAR",   nullptr,   "300w002", nullptr   },
	{ "BACK YARD",      "302w002", "302w002", nullptr   },
	{ "BURGER MORSEL ", nullptr,   nullptr,   "302w010" },
	{ "TRUFFLES",       nullptr,   "302w013", "302w014" },
	{ "TROUGH",         "302w015", "300w003", "302w016" },
	{ "SIGN",           "302w017", "300w004", "300w005" },
	{ "CABIN",          "302w018", "300w002", "300w002" },
	{ "WINDOW",         "302w021", "300w002", "302w022" },
	{ "ROCKING CHAIR",  "302w019", "302w020", "302w020" },
	{ "CRASHED ROCKET", "302w023", "300w003", "302w024" },
	{ "DOCK",           "302w025", "300w002", "302w026" },
	{ "FORCE FIELD",    "302w027", "300w002", "300w002" },
	{ "ROCK",           "302w028", "300w002", "300w002" },
	{ "TREES",          "302w029", "300w002", "300w002" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room302::SERIES1[] = {
	{  0, "302p004", 1, 255, -1, 0, nullptr, 0 },
	{ 19, "302_008", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room302::SERIES2[] = {
	{   7, "302_002",  1, 255, -1,    0, nullptr,  0 },
	{   9, "302w001a", 2, 255, -1,    0, &_state1, 0 },
	{   9, "302w001b", 2, 255, -1,    0, &_state1, 1 },
	{   9, "302w001c", 2, 255, -1,    0, &_state1, 2 },
	{  34, "302_005",  2, 125, -1,    0, nullptr,  0 },
	{  48, "302_006",  2, 125, -1, 1024, nullptr,  0 },
	{ 138, "302_007",  1, 255, -1,    0, nullptr,  0 },
	{ 138, "302_003",  1, 255, -1,    0, nullptr,  0 },
	{ 152, "302_007",  1, 255, -1,    0, nullptr,  0 },
	{ 152, "302_003",  1, 255, -1,    0, nullptr,  0 },
	{ 165, nullptr,    2,   0,  4,    0, nullptr,  0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room302::SERIES3[] = {
	{  7, "302_002",  2, 255, -1,    0, nullptr,  0 },
	{  9, "302w001a", 1, 255, -1,    0, &_state1, 0 },
	{  9, "302w001b", 1, 255, -1,    0, &_state1, 1 },
	{  9, "302w001c", 1, 255, -1,    0, &_state1, 2 },
	{ 24, "302p001",  2, 255, -1,    0, nullptr,  0 },
	{ 35, "302_005",  2, 125, -1,    0, nullptr,  0 },
	{ 48, "302_006",  2, 125, -1, 1024, nullptr,  0 },
	{ 52, "302p003",  1, 255, -1,    0, nullptr,  0 },
	{ 71, "302_008",  2, 255, -1,    0, nullptr,  0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room302::PLAY1[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY2[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY3[] = {
	{ 0, -1, "304_001", 2, 255, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY4[] = {
	{ 0,  1, nullptr,   1,   0, -1, 0, 0, nullptr, 0 },
	{ 2, -1, "302w002", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY5[] = {
	{  0, 22, nullptr,   1,   0, -1, 0, 0, nullptr, 0 },
	{ 23, -1, "302_001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY6[] = {
	{ 0,  7, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	{ 8, -1, nullptr, 1, 0,  5,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY7[] = {
	{  0, 29, "302t004", 2, 155, -1, 2048,  0, nullptr, 0 },
	{ 26, 29, "302t005", 2, 155, -1,    0, 12, &Flags::_flags[V110], 0 },
	{ 26, 29, "302t005", 2, 155, -1,    0,  3, &Flags::_flags[V110], 1 },
	{ 30, 61, nullptr,   2,   0, -1,    0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY8[] = {
	{ 62, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY9[] = {
	{  0, 28, nullptr, 1, 0, -1, 0, 0, nullptr, 0 },
	{ 29, -1, nullptr, 1, 0, 6, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY10[] = {
	{ 0, 4, nullptr, 2, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY11[] = {
	{ 5, 13, nullptr, 2, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY12[] = {
	{  6, 16, "302t002", 2, 155, -1, 0, 0, nullptr, 0 },
	{ 11, 17, nullptr,   2,   0, -1, 3, 2, nullptr, 0 },
	{ 12,  6, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room302::PLAY13[] = {
	{ 0, 4, "302t003", 2, 155, -1, 0, 0, nullptr, 0 },
	{ 5, 3, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 4, 5, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 4, 3, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 3, 5, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 5, 3, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 3, 4, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	{ 5, 0, nullptr,   2,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

int32 Room302::_state1;


Room302::Room302() : Section3Room() {
	_state1 = 0;
}

void Room302::init() {
	setupDigi();

	if (!_G(flags)[kTrufflesInMine]) {
		static const char *NAMES[12] = {
			"302t001a", "302t001b", "302t001c", "302t001d", "302t001e",
			"302t001f", "302t001g", "302t001h", "302t002", "302t003",
			"302t004", "302t005"
		};
		for (int i = 0; i < 12; ++i)
			digi_preload(NAMES[i], 302);
	}

	player_set_commands_allowed(false);
	pal_cycle_init(112, 127, 6, -1, -1);

	if (_G(flags)[kTrufflesInMine]) {
		hotspot_set_active("TRUFFLES", false);
	} else if (_G(flags)[kPerkinsLostIsland]) {
		series_load("302tr01");
		series_load("302tr01s");
		series_load("302tr02");
		series_load("302tr02s");
		series_load("302tr03");
		series_load("302tr03s");
		_val1 = 24;
		kernel_trigger_dispatch_now(9);
	} else {
		_val1 = 21;
		kernel_trigger_dispatch_now(9);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 303:
		_G(wilbur_should) = _G(flags)[V125] ? 6 : 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 304:
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 305:
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(341, 287, 9);
		ws_hide_walker();
		_state1 = imath_ranged_rand(0, 2);

		if (_G(flags)[kPerkinsLostIsland]) {
			_G(flags)[V113] = 1;
			_volume = 125;
			_val4 = 29;
		} else {
			_val4 = 30;
		}

		kernel_trigger_dispatch_now(10);
		_G(flags)[V107] = 1;
		break;
	}

	if (_G(flags)[V107]) {
		hotspot_set_active("BURGER MORSEL ", false);
	} else {
		_series1 = series_show("302BURG", 0x200);
		hotspot_set_active("BURGER MORSEL ", true);
	}
}

void Room302::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		wilbur_speech("302w009z");
		break;

	case 2:
		ws_unhide_walker();
		ws_walk(500, 250, nullptr, 3, 8, 1);
		break;

	case 3:
		player_set_commands_allowed(true);
		break;

	case 4:
		_volume -= 20;
		if (_volume > 0) {
			term_message("fading propellor, current volume = %d", _volume);
			digi_change_volume(2, _volume);
			kernel_timing_trigger(10, 4);
		} else {
			digi_stop(2);
		}
		break;

	case 5:
		terminateMachineAndNull(_series1);
		inv_give_to_player("BURGER MORSEL");
		hotspot_set_active("BURGER MORSEL ", false);
		_G(flags)[V107] = 1;
		break;

	case 6:
		inv_put_thing_in("BURGER MORSEL", NOWHERE);
		break;

	case 7:
		if (!digi_play_state(2)) {
			_val7 = imath_ranged_rand(0, 6);
			digi_play(Common::String::format("302t001%c", 'a' + _val7).c_str(), 2, 55, -1, 302);
		}
		break;

	case 8:
		if (_val2 == 19) {
			digi_unload_stream_breaks(SERIES3);
			_series2.terminate();
			pal_fade_set_start(0);
			_series3 = series_stream_with_breaks(SERIES1, "302pe02", 6, 1, 3007);
			pal_fade_init(0, 255, 100, 15, -1);
		}
		break;

	case 9:
		switch (_val1) {
		case 20:
			series_show("302tr02", 0x900, 0, 9, 30, 13);
			series_show("302tr02s", 0x901, 0, -1, 30, 13);
			_val1 = 22;
			break;

		case 21:
			_series2.show("302tr02", 0x900);
			_val1 = 22;
			break;

		case 22:
			_val6 = imath_ranged_rand(0, 1);
			_val1 = _val6 ? 24 : 23;
			kernel_trigger_dispatch_now(9);
			break;

		case 23:
			kernel_trigger_dispatch_now(7);
			_val1 = 20;
			series_play_with_breaks(PLAY10, "302tr02", 0x900, 9, 3);
			break;

		case 24:
			kernel_trigger_dispatch_now(7);
			_val1 = 20;
			series_play_with_breaks(PLAY11, "302tr02", 0x900, 9, 3);
			break;

		case 25:
			_val1 = 20;
			series_play_with_breaks(PLAY12, "302tr03", 0x900, 9, 3);
			break;

		case 26:
			_val1 = 20;
			series_play_with_breaks(PLAY13, "302tr03", 0x900, 9, 3);
			break;

		case 27:
			ws_hide_walker();
			_G(wilbur_should) = 11;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			_val1 = 28;
			series_play_with_breaks(PLAY7, "302tr01", 0x700, 9, 3);
			break;

		case 28:
			hotspot_set_active("truffles", false);
			_G(flags)[kTrufflesInMine] = 1;
			_G(wilbur_should) = 8;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			series_play_with_breaks(PLAY8, "302tr01", 0x900, -1, 3);
			break;

		default:
			break;
		}
		break;

	case 10:
		switch (_val4) {
		case 29:
			digi_preload_stream_breaks(SERIES2);
			_G(wilbur_should) = 9;
			series_stream_with_breaks(SERIES2, "302pr01", 6, 1, kCHANGE_WILBUR_ANIMATION);
			break;

		case 30:
			digi_preload_stream_breaks(SERIES3);
			_val2 = 19;
			series_stream_with_breaks(SERIES3, "302pr02", 6, 1, 8);
			break;

		default:
			break;
		}
		break;

	case 10008:
		_val1 = 26;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			ws_hide_walker();
			player_set_commands_allowed(false);
			series_play_with_breaks(PLAY1, "302wi03", 0x500, 3004, 3);
			pal_fade_init(_G(kernel).first_fade, 255, 0, 120, 3004);
			break;

		case 2:
			ws_demand_location(427, 228, 4);
			ws_hide_walker();
			player_set_commands_allowed(false);
			series_play_with_breaks(PLAY2, "302wi04", 0x500, 2, 3);
			break;

		case 3:
			ws_hide_walker();
			player_set_commands_allowed(false);
			series_play_with_breaks(PLAY3, "302wi01", 0x500, 3003, 3);
			pal_fade_init(_G(kernel).first_fade, 255, 0, 120, 3003);
			break;

		case 4:
			player_first_walk(233, 222, 7, 180, 270, 7, true);
			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;

		case 5:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10002;
			player_first_walk(483, 210, 5, 490, 245, 7, true);
			break;

		case 6:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10002;
			player_first_walk(184, 189, 8, 78, 225, 8, true);
			break;

		case 7:
			_val1 = 27;
			break;

		case 8:
			ws_unhide_walker();
			_G(wilbur_should) = _G(flags)[V110] ? 10001 : 18;
			ws_turn_to_face(3, kCHANGE_WILBUR_ANIMATION);
			break;

		case 9:
			digi_unload_stream_breaks(SERIES2);
			_G(wilbur_should) = 12;
			hotspot_set_active("BURGER MORSEL ", true);
			series_play_with_breaks(PLAY4, "302wi10", 0x200, kCHANGE_WILBUR_ANIMATION, 2);
			break;

		case 10:
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			series_play_with_breaks(PLAY6, "302wi06", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 11:
			_G(wilbur_should) = 17;
			series_play_with_breaks(PLAY9, "302wi07", 0x200, kCHANGE_WILBUR_ANIMATION, 3, 4);
			break;

		case 12:
			_series1 = series_show("302BURG", 0x200);
			_G(flags)[V107] = 0;
			_G(wilbur_should) = 15;
			series_play_with_breaks(PLAY5, "302wi09", 0x200, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 13:
			_G(wilbur_should) = 14;
			kernel_timing_trigger(30, kCHANGE_WILBUR_ANIMATION);
			break;

		case 14:
			player_set_commands_allowed(true);
			wilbur_speech("302w009z");
			break;

		case 15:
			ws_unhide_walker();
			_G(wilbur_should) = 10001;

			if (_G(flags)[V108]) {
				_val5 = imath_ranged_rand(0, 9);
				wilbur_speech(Common::String::format("302w004%c", 'a' + _val5).c_str(), kCHANGE_WILBUR_ANIMATION);
			} else {
				wilbur_speech("302w003", kCHANGE_WILBUR_ANIMATION);
			}
			break;

		case 16:
			_G(wilbur_should) = 7;
			player_set_commands_allowed(false);
			wilbur_speech(_G(flags)[V110] ? "300w010" : "300w007", kCHANGE_WILBUR_ANIMATION);
			break;

		case 17:
			ws_unhide_walker();
			if (!_G(flags)[V110])
				wilbur_speech("300w008");
			break;

		case 18:
			_G(flags)[V110] = 1;
			_G(wilbur_should) = 10001;
			wilbur_speech("300w009");
			break;

		case 10013:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			_G(wilbur_should) = 10002;

			if (_G(flags)[kTrufflesInMine]) {
				_G(flags)[V002] = 1;
				wilbur_speech("300w059");
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

void Room302::pre_parser() {
	if (player_said_any("BACKYARD", "BACKYARD ") && !player_said_any("LOOK AT", "GEAR", "ENTER"))
		player_set_facing_hotspot();
}

void Room302::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("LOOK AT", "BURGER MORSEL ")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 13;
		wilbur_speech("302w009y", kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("LOOK AT", "DOOR")) {
		wilbur_speech(player_been_here(304) ? "302w006" : "302w005");
	} else if (player_said("DISTILLED CARROT JUICE", "CRASHED ROCKET")) {
		wilbur_speech("300w032");
	} else if (player_said("CARROT JUICE", "CRASHED ROCKET")) {
		wilbur_speech("300w045");
	} else if (player_said("MATCHES", "CRASHED ROCKET")) {
		wilbur_speech("300w018");
	} else if (player_said("TAKE", "BURGER MORSEL ")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 10;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("ENTER", "DOOR") || player_said("GEAR", "DOOR")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("MATCHES", "PROBE")) {
		wilbur_speech("300w023");
	} else if (player_said("BOTTLE", "TROUGH")) {
		wilbur_speech("300w073");
	} else if (player_said("LOOK AT", "STORM CELLAR")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 1;
		wilbur_speech("302w007", kCHANGE_WILBUR_ANIMATION);
	} else if (player_said_any("GEAR", "ENTER") && player_said("STORM CELLAR")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 1;

		if (_G(flags)[V133]) {
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			term_message("Wilbur: Here we go...!");
			wilbur_speech("302w008", kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("CARROT JUICE", "TROUGH")) {
		wilbur_speech("300w044");
	} else if (player_said("BACKYARD") && player_said_any("LOOK AT", "GEAR", "ENTER")) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 6, 3002);
		_G(flags)[V125] = 0;
	} else if (player_said("BACKYARD ") && player_said_any("LOOK AT", "GEAR", "ENTER")) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 6, 3002);
		_G(flags)[V125] = 1;
	} else if (player_said("TRUFFLES")) {
		if (player_said("BURGER MORSEL")) {
			player_set_commands_allowed(false);
			_G(wilbur_should) = 16;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else if (player_said("LOOK AT")) {
			wilbur_speech(_G(flags)[V109] ? "302w012" : "302w011");
		} else if (player_said("MATCHES")) {
			wilbur_speech("300w016");
		} else if (player_said("DISTILLED CARROT JUICE")) {
			wilbur_speech("300w031");
		} else if (player_said("CARROT JUICE")) {
			wilbur_speech("300w044");
		} else if (player_said("BOTTLE")) {
			wilbur_speech("300w072");
		} else {
			_val1 = 25;
			return;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
