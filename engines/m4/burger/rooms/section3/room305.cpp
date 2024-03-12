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

#include "m4/burger/rooms/section3/room305.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room305::SAID[][4] = {
	{ "STAIRS",      "305w002", "300w002", nullptr   },
	{ "DARKNESS",    "305w004", "300w002", "300w002" },
	{ "MINE",        "305w003", "300w002", "300w002" },
	{ "GENERATOR",   nullptr,   "305w008", nullptr   },
	{ "PICK AXE",    "305w015", "305w016", "305w017" },
	{ "BOX",         "305w018", "300w003", "300w005" },
	{ "DEBRIS",      nullptr,   "305w021", "300w005" },
	{ "GROUND",      "305w022", "300w002", "300w002" },
	{ "WALL",        "305w022", "300w002", "300w002" },
	{ "CEILING",     "305w022", "300w002", "300w002" },
	{ "FORCE FIELD", "302w027", "300w002", "300w002" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room305::SERIES1[] = {
	{  6, "305_003", 2, 255, -1,    0, nullptr, 0 },
	{ 11, "305w009", 1, 255, -1,    0, nullptr, 0 },
	{ 12, "305_005", 2, 255, -1,    0, nullptr, 0 },
	{ 20, "305w010", 1, 255, -1,    0, &Flags::_flags[V135], 0 },
	{ 30, "305_006", 2, 255, -1, (uint)-1, nullptr, 0 },
	{ 72, "305_001", 1, 255, -1,    0, nullptr, 0 },
	{ 72, nullptr,   2,   0, -1, 2048, nullptr, 0 },
	{ 79, "305_004", 2, 255, -1,    0, nullptr, 0 },
	{ 94, "305_002", 2, 255, -1,    0, nullptr, 0 },
	{ -1, 0, 0, 0, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room305::PLAY1[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room305::PLAY2[] = {
	{ 0, -1, 0, 1, 0, -1, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};


void Room305::init() {
	setupDigi();
	set_palette_brightness(60);
	pal_cycle_init(96, 111, 6, -1, -1);
	_G(flags)[kMineRoomIndex] = 0;

	if (_G(flags)[V134] && !_G(flags)[V135] && !_G(flags)[kEnteredMine]) {
		hotspot_set_active("DARKNESS", true);
		hotspot_set_active("MINE", false);
	} else {
		hotspot_set_active("DARKNESS", false);
		hotspot_set_active("MINE", true);
	}

	_series1 = series_show(_G(flags)[V134] ? "305genx" : "305gen", 0x200);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 302:
		_G(wilbur_should) = 101;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 319:
		player_set_commands_allowed(true);
		ws_demand_location(-50, 200);
		kernel_trigger_dispatch_now(301);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(320, 290, 7);
		break;
	}
}

void Room305::daemon() {
	Mine::daemon();
	if (!_G(kernel).continue_handling_trigger)
		return;
	_G(kernel).continue_handling_trigger = false;

	switch (_G(kernel).trigger) {
	case 100:
		_series1 = series_show("305genx", 0x200);
		_G(flags)[V134] = 1;
		_G(wilbur_should) = 105;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 3001:
		_G(game).new_room = 302;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 101:
			ws_demand_location(372, 267, 9);
			player_set_commands_allowed(false);
			ws_hide_walker();

			if (_G(flags)[V133]) {
				_G(wilbur_should) = 10001;
			} else {
				_G(flags)[V133] = 1;
				_G(wilbur_should) = 104;
			}

			series_play_with_breaks(PLAY1, "305wi01", 0x700, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 102:
			player_set_commands_allowed(false);
			ws_demand_location(375, 280, 2);
			ws_hide_walker();
			series_play_with_breaks(PLAY2, "305wi02", 0x700, -1, 3);
			pal_fade_init(_G(kernel).first_fade, 255, 0, 60, 3001);
			break;

		case 103:
			hotspot_set_active("DARKNESS", false);
			hotspot_set_active("MINE", true);
			player_set_commands_allowed(false);
			ws_hide_walker();
			terminateMachineAndNull(_series1);
			series_stream_with_breaks(SERIES1, "305wi04", 6, 0x200, 100);
			break;

		case 104:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			wilbur_speech("305w001");
			break;

		case 105:
			ws_unhide_walker();

			if (_G(flags)[V135]) {
				_G(wilbur_should) = 106;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				_G(flags)[V135] = 1;
				_G(wilbur_should) = 106;
				wilbur_speech("305w011", kCHANGE_WILBUR_ANIMATION);
			}
			break;

		case 106:
			_G(wilbur_should) = 10001;
			wilbur_speech(_G(flags)[kEnteredMine] ? "305w013" : "305w012", kCHANGE_WILBUR_ANIMATION);
			break;

		case 107:
			_G(wilbur_should) = 10001;
			wilbur_speech("305w012", kCHANGE_WILBUR_ANIMATION);
			break;

		case 108:
			mine_travel_link(BACK);
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

void Room305::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	// Handle any generic mine stuff
	Mine::parser();
	if (!_G(player).command_ready)
		// Already handled
		return;

	// Check for standard actions
	if (_G(walker).wilbur_said(SAID)) {
		// Handled
	} else if (player_said("climb", "stairs") || player_said("GEAR", "STAIRS")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 102;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("ENTER", "MINE") || player_said("ENTER", "DARKNESS")) {
		player_set_commands_allowed(false);

		if (!_G(flags)[kDrunkCarrotJuice]) {
			player_set_commands_allowed(true);
			wilbur_speech("305w004");
		} else {
			_G(wilbur_should) = 108;

			if (_G(flags)[kEnteredMine]) {
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				wilbur_speech("305w005", kCHANGE_WILBUR_ANIMATION);
			}
		}
	} else if (player_said("DISTILLED CARROT JUICE", "GENERATOR")) {
		wilbur_speech(_G(flags)[V134] ? "300w037" : "300w036");
	} else if (player_said("CARROT JUICE", "GENERATOR")) {
		wilbur_speech(_G(flags)[V134] ? "300w054" : "300w053");
	} else if (player_said_any("CARROT JUICE", "DISTILLED CARROT JUICE") &&
			player_said("BARREL")) {
		wilbur_speech("300w038");
	} else if (player_said("LOOK AT", "GENERATOR")) {
		wilbur_speech(_G(flags)[V134] ? "300w007" : "300w006");
	} else if (player_said("MATCHES", "GENERATOR")) {
		wilbur_speech(_G(flags)[V134] ? "300w022" : "300w021");
	} else if (player_said("LOOK AT", "DEBRIS")) {
		if (!_G(flags)[V136]) {
			_G(flags)[V136] = 1;
			wilbur_speech("305w019");
		} else {
			wilbur_speech(Common::String::format("305w020%c",
				'a' + imath_ranged_rand(0, 6)).c_str());
		}
	} else if (player_said("GEAR", "GENERATOR")) {
		if (_G(flags)[V134]) {
			wilbur_speech("305w014");
		} else {
			_G(wilbur_should) = 103;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
	} else if (!player_said("LOOK AT WALL")) {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
