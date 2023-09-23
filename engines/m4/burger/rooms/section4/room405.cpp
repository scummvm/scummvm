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

#include "m4/burger/rooms/section4/room405.h"
#include "m4/burger/rooms/section4/section4.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room405::SAID[][4] = {
	{ "VIPE",          nullptr,   "400W001", "400W001" },
	{ "GUITAR",        "405w003", nullptr,   nullptr   },
	{ "RECORDS",       "405w004", nullptr,   nullptr   },
	{ "VERA",          "405w006", "400w001", "400w001" },
	{ "JUKEBOX",       "405w007", nullptr,   nullptr   },
	{ "DISC",          "400w002", "400w003", nullptr   },
	{ "DISC ",         "400w002", "400w003", nullptr   },
	{ "SWINGING DOOR", "405w009", "400w001", nullptr   },
	{ "ORDER WINDOW",  "405w009", "400w001", "400w001" },
	{ "MUFFIN",        "405w010", nullptr,   nullptr   },
	{ "FOUNTAIN",      "405W011", nullptr,   nullptr   },
	{ "MOOSEHEAD",     "405W012", nullptr,   nullptr   },
	{ "MENU",          "405W013", nullptr,   nullptr   },
	{ "STOOL",         nullptr,   nullptr,   "405W019" },
	{ "BOOTH",         nullptr,   nullptr,   "405W019" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room405::SERIES1[] = {
	{  8,  nullptr,   0,   0, 22, 0, nullptr, 0 },
	{  15, "405v512", 1, 255, -1, 0, nullptr, 0 },
	{ 115, "405_008", 2, 255, -1, 0, nullptr, 0 },
	{ 123, "405e503", 1, 255, 13, 0, nullptr, 0 },
	{ 210, "999_003", 2, 255, -1, 0, nullptr, 0 },
	{ 221, "999_003", 2, 255, -1, 0, nullptr, 0 },
	{ 235, nullptr,   2, 255,  7, 0, nullptr, 0 },
	{ 250, nullptr,   0,   0, 29, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room405::PLAY1[] = {
	{ 0, 3, nullptr, 0, 0, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY2[] = {
	{  0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{  2,  9, nullptr, 0, 0, 11, 0, 0, nullptr, 0 },
	{ 10, -1, nullptr, 0, 0,  9, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY3[] = {
	{ 0, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY4[] = {
	{ 0, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY5[] = {
	{  0,  5, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  6, 42, "405_006", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 43, 51, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 60, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 61, 69, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 70, 78, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 79, 85, "405_005", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 86, -1, "405_004", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room405::PLAY7[] = {
	{ 0, -1, "405_002", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room405::init() {
	_volume = 255;
	pal_cycle_init(122, 127, 6);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(false);
		ws_demand_location(199, 253, 4);
		player_set_commands_allowed(true);
		break;

	case 406:
		player_set_commands_allowed(false);
		ws_demand_location(413, 356, 10);
		setDest(120);
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(241, 266, 4);
		setDest(120);
		break;
	}

	_series1 = series_play("405plate", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	if (_G(flags)[V166]) {
		hotspot_set_active("vipe", false);
		hotspot_set_active("guitar", false);
	} else {
		_val1 = 39;
		_val2 = 39;
		kernel_trigger_dispatch_now(15);
	}

	if (inv_object_is_here("records")) {
		_records = series_play("405rec", 0, 0, -1, 600, -1, 100, 0, 0, 0, 0);
	} else {
		hotspot_set_active("records", false);
	}

	if (_G(flags)[V167]) {
		_box = series_play("405box1", 0xf00, 2, -1, 600, -1, 100, 0, 0, 0, 0);
	} else {
		hotspot_set_active("box", false);
	}

	if (_G(flags)[V168]) {
		hotspot_set_active("vera", false);
		hotspot_set_active("order window", false);
	} else {
		_val3 = 4;
		_val4 = 4;
		kernel_trigger_dispatch_now(17);
	}

	_val5 = 3;
	kernel_trigger_dispatch_now(16);
	_lid = series_play("405lid", 0xf00, 2, -1, 600, -1, 100, 0, 0, 0, 0);

	if (_G(flags)[V168]) {
		digi_preload("400_001");
		digi_play_loop("400_001", 3, 140);
	} else {
		digi_preload("405_010");
		digi_play_loop("405_010", 3, 180);
	}
}

void Room405::daemon() {
	// TODO: daemon
}

void Room405::pre_parser() {
	if (player_said("quarter", "jukebox") && inv_player_has("quarter") && _G(flags)[V167])
		player_hotspot_walk_override(199, 253, 2, -1);

	if (player_said("disc") && !player_said("poof") && !player_said("gear"))
		player_hotspot_walk_override(264, 284, 10);

	if (player_said("disc ") && !player_said("poof ") && !player_said("gear"))
		player_hotspot_walk_override(360, 333, 10);

}

void Room405::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("conv86")) {
		conv86();
	} else if (player_said("conv89")) {
		conv89();
	} else if (player_said("conv90")) {
		conv90();
	} else if (player_said("conv91")) {
		conv90();
	} else if (player_said("conv92")) {
		conv92();
	} else if (player_said("talk to", "vipe")) {
		talkToVipe();
	} else if (player_said("talk to", "vera")) {
		talkToVera();
	} else if (player_said("gear", "records") && inv_object_is_here("records") && _G(flags)[V166]) {
		wilbur_speech("405w005");
	} else if (player_said("poof") || player_said("gear", "disc")) {
		Section4::poof(4006);
	} else if (player_said("poof ") || player_said("gear", "disc ")) {
		Section4::poof(4005);
	} else if ((player_said("take", "records") && inv_object_is_here("records")) ||
			(player_said("gear", "records") && inv_object_is_here("records"))) {
		if (!_G(flags)[V166]) {
			startConv91();
		} else if (inv_object_is_here("records")) {
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatch_now(23);
		}
	} else if (player_said("dog collar", "vipe")) {
		_G(wilbur_should) = 25;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		hotspot_set_active("vipe", false);
		hotspot_set_active("guitar", false);
	} else if (player_said("swinging door") && player_said_any("enter", "gear") && !_G(flags)[V168]) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_val3 = 7;
			_digiName = "405V504";
			_newMode = KT_PARSE;
			_newTrigger = 1;
			break;

		case 1:
			_val3 = 4;
			_G(kernel).trigger_mode = KT_PARSE;
			digi_play("405e502", 1, 255, 2);
			break;

		case 2:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (player_said("muffin") && !_G(flags)[V168] &&
			(player_said_any("take", "gear") || inv_player_has(_G(player).verb))) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_val3 = 21;
			_digiName = "405V502";
			_newMode = KT_PARSE;
			_newTrigger = 1;
			_val5 = 1;
			kernel_trigger_dispatch_now(16);
			break;

		case 1:
			term_message("spat 1");
			_G(kernel).trigger_mode = KT_PARSE;
			series_play("405eu02", 0xf00, 0, 4, 4, 0, 100, 0, 0, 10, 14);
			digi_play("405e501", 1, 255, 2);
			break;

		case 2:
			terminateMachineAndNull(_eu02);
			series_play("405eu02", 0xf00, 0, -1, 4, 0, 100, 0, 0, 16, 18);
			_val3 = 10;
			_digiName = "405V503";
			_newMode = KT_PARSE;
			_newTrigger = 3;
			break;

		case 3:
			player_set_commands_allowed(true);
			_val3 = 4;
			break;

		case 4:
			term_message("spat 4");
			_eu02 = series_play("405eu02", 0xf00, 0, -1, 600, -1, 100, 0, 0, 15, 15);
			break;

		default:
			break;
		}
	} else if (player_said("vipe") && inv_player_has(_G(player).verb)) {
		_G(wilbur_should) = 27;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said("records", "jukebox")) {
		_G(wilbur_should) = 32;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (!_G(flags)[V168] && (player_said("quarter", "jukebox") || player_said("gear", "jukebox"))) {
		if (!inv_player_has("quarter")) {
			wilbur_speech("405w008");
		}  else if (_G(flags)[V167]) {
			_G(wilbur_should) = 34;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				_val3 = 17;
				player_set_commands_allowed(false);
				_digiName = "405v510";
				_newMode = KT_PARSE;
				_newTrigger = 1;
				break;

			case 1:
				_val3 = 4;
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		}
	} else if (player_said("vera") && inv_player_has(_G(player).verb)) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_val3 = 7;
			_digiName = "405v511";
			_newMode = KT_PARSE;
			_newTrigger = 1;
			break;

		case 1:
			player_set_commands_allowed(true);
			_val3 = 4;
			break;

		default:
			break;
		}
	} else if ((player_said("disc") && inv_player_has(_G(player).verb)) ||
			(player_said("disc ") && inv_player_has(_G(player).verb))) {
		wilbur_speech("400w004");
	} else if (lookFlag && player_said("vipe")) {
		wilbur_speech(_G(flags)[V165] ? "405w002" : "405w001");
	} else if (!_G(walker).wilbur_said(SAID)) {
		return;
	}

	_G(player).command_ready = false;
}

void Room405::conv86() {
	// TODO: conv
}

void Room405::conv89() {
	// TODO: conv
}

void Room405::conv90() {
	// TODO: conv
}

void Room405::conv91() {
	// TODO: conv
}

void Room405::conv92() {
	// TODO: conv
}

void Room405::talkToVipe() {
	// TODO: talk
}

void Room405::talkToVera() {
	// TODO: talk
}

void Room405::startConv91() {
	conv_load_and_prepare("conv91", 4);
	conv_export_value_curr(_G(flags)[V165], 0);
	conv_play_curr();
}

void Room405::playDigi() {
	if (_digiName) {
		_G(kernel).trigger_mode = _newMode;
		digi_play(_digiName, 1, 255, _newTrigger);
		_digiName = nullptr;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
