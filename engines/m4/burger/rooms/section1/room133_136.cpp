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

#include "m4/burger/rooms/section1/room133_136.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "SIGN",         "136w001", "136W003", nullptr   },
	{ "SIGN ",        "136w002", "136W003", "136w005" },
	{ "CONSTRUCTION", nullptr,   "136W008", "136W008" },
	{ "OLD BRIDGE",   nullptr,   "136W008", nullptr   },
	{ "TOWN LIMITS",  nullptr,   "136W008", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{  0, 12, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{ 13, 30, "136_002", 2, 255, -1,    0, 0, 0, 0 },
	{ 31, 57, "136_002", 2, 255, -1,    0, 0, 0, 0 },
	{ 58, -1, nullptr,   2, 255, -1, 2048, 0, 0, 0 },
	{ -1, -1, nullptr,   0,   0, -1,    0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesStreamBreak SERIES1[] = {
	{  6, nullptr,   2, 255,  9, 0, 0, 0 },
	{ 10, "100_022", 2, 255, -1, 0, 0, 0 },
	{ -1, nullptr,   0,   0, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES3[] = {
	{  0, "100_010", 1, 255, -1, 0, nullptr, 0 },
	{ 14, "100_011", 1, 255, -1, 0, nullptr, 0 },
	{ 20, nullptr,   1, 255, 14, 0, nullptr, 0 },
	STREAM_BREAK_END
};

void Room133_136::init() {
	_flag = true;
	_volume = 255;
	_walkCodes = nullptr;
	_G(kernel).call_daemon_every_loop = true;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 134:
	case 135:
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 136:
		ws_demand_location(444, 230, 2);
		break;

	case 137:
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(320, 271, 5);
		break;
	}

	setupSign();
	setupSignWalkAreas();
	digi_preload("136_001");
	digi_play_loop("136_001", 3, 180);
}

void Room133_136::daemon() {
	int frame;

	if (player_commands_allowed() && _G(player).walker_visible) {
		player_update_info();

		if (_G(player_info).y < 188 && _flag) {
			_flag = false;
			kernel_timing_trigger(120, 1);
			player_set_commands_allowed(false);
			intr_freshen_sentence();
			series_play("136cw01", 0xf00, 0, 2, 6, 0, 100, 0, 0, 0, 10);
		}
	}

	switch (_G(kernel).trigger) {
	case 1:
		Section1::walk();
		break;

	case 2:
		_G(flags)[kDisableFootsteps] = 1;
		digi_play("136c001", 1, 255, 5, 136);
		_val1 = 1;
		kernel_trigger_dispatch_now(3);
		break;

	case 3:
		if (_val1 == 1) {
			frame = imath_ranged_rand(8, 10);
			series_play("136cw01", 0xf00, 0, 3, 6, 0, 100, 0, 0, frame, frame);
		} else {
			ws_walk(217, 268, 0, 4, 2);
			series_play("136cw01", 0xf00, 2, -1, 6, 0, 100, 0, 0, 0, 10);
		}
		break;

	case 4:
		wilbur_speech("136w009");
		_flag = true;
		player_set_commands_allowed(true);
		break;

	case 5:
		_G(flags)[kDisableFootsteps] = 0;
		_val1 = 2;
		break;

	case 6:
		pal_fade_set_start(0);
		terminateMachineAndNull(_sign1);
		terminateMachineAndNull(_sign2);
		ws_hide_walker();
		gr_backup_palette();
		digi_preload("100_022");
		digi_preload_stream_breaks(SERIES1);
		kernel_timing_trigger(30, 7);
		break;

	case 7:
		series_stream_with_breaks(SERIES1, "120dt01", 9, 0, 1009);
		kernel_timing_trigger(1, 8);
		break;

	case 8:
		pal_fade_init(15, 255, 100, 0, -1);
		break;

	case 9:
		digi_change_volume(2, 0);
		break;

	case 10:
		player_set_commands_allowed(true);
		break;

	case 11:
		terminateMachineAndNull(_sign1);
		terminateMachineAndNull(_sign2);

		if (_G(flags)[kRoadOpened]) {
			Section1::updateWalker(413, 281, 9, 15, true);
		} else {
			Section1::updateWalker(238, 226, 4, 15);
		}
		break;

	case 13:
		if ((_G(player_info).x < 320 && _G(player_info).y > 250) ||
			(_G(player_info).y > 227 && !_G(flags)[kRoadOpened])) {
			kernel_trigger_dispatch_now(11);
		} else if (!_G(flags)[kRoadOpened]) {
			digi_preload_stream_breaks(SERIES2);
			series_stream_with_breaks(SERIES2, "136dt01", 6, 0x400, 14);
		} else {
			kernel_trigger_dispatch_now(11);
		}
		break;

	case 14:
		if (!_G(flags)[kRoadOpened]) {
			_volume -= 10;
			if (_volume > 0) {
				digi_change_volume(2, _volume);
				kernel_timing_trigger(3, 14);
			} else {
				digi_stop(1);
				digi_unload_stream_breaks(SERIES2);
				digi_stop(2);
				digi_unload("100_013");
				player_set_commands_allowed(true);
				_volume = 255;
			}
		}
		break;

	case 15:
		setupSign();
		_G(walker).reset_walker_sprites();
		digi_preload_stream_breaks(SERIES2);

		if (_G(flags)[kRoadOpened]) {
			_G(flags)[V000] = 1002;
			series_stream_with_breaks(SERIES3, "136dt02", 6, 0x200, 6);
		} else {
			series_stream_with_breaks(SERIES2, "136dt01", 6, 0x200, 14);
		}
		break;

	case 16:
		ws_demand_location(398, 297, 8);
		ws_hide_walker();
		terminateMachineAndNull(_sign1);
		terminateMachineAndNull(_sign2);
		player_set_commands_allowed(false);
		series_play_with_breaks(PLAY1, "136wi01", 0x300, 17, 3);
		break;

	case 17:
		_G(flags)[kRoadOpened] = 1;
		ws_unhide_walker();
		player_set_commands_allowed(true);
		setupSign();
		setupSignWalkAreas();
		wilbur_speech("136w004");
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 3:
			player_set_commands_allowed(true);
			player_first_walk(639, 264, 9, 588, 252, 9, true);
			break;

		case 4:
			player_set_commands_allowed(true);
			player_first_walk(0, 355, 3, 96, 340, 3, true);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kBurlEntersTown:
		if (player_commands_allowed() && _G(player).walker_visible &&
				INTERFACE_VISIBLE && !digi_play_state(1)) {
			Section1::updateDisablePlayer();
			digi_preload("100_013");
			digi_play("100_013", 2);
			kernel_timing_trigger(240, 13);
		} else {
			kernel_timing_trigger(60, 10028);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room133_136::pre_parser() {
	if (player_said("gear", "sign") && !_G(flags)[kRoadOpened])
		player_hotspot_walk_override(246, 247, 8);

	if (player_said("old bridge") &&
			!player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(3);

	if (player_said("town limits") &&
			!player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(9);

	if (player_said("enter", "construction"))
		player_hotspot_walk_override(292, 173, 1);
}

void Room133_136::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("gear", "sign") && !_G(flags)[kRoadOpened]) {
		kernel_trigger_dispatch_now(16);

	} else if (_G(walker).wilbur_said(SAID)) {
		// Already handled

	} else if (lookFlag && player_said("construction")) {
		wilbur_speech(_G(flags)[V000] == 1002 ? "136w006" : "136w007");

	} else if (player_said("enter", "old bridge") || player_said("gear", "old bridge") ||
			(lookFlag && player_said("old bridge"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1008);

	} else if (player_said("enter", "town limits") || player_said("gear", "town limits") ||
			(lookFlag && player_said("town limits"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1010);

	} else if (inv_player_has(_G(player).verb) &&
		player_said_any("old bridge", "town limits", "construction")) {
		wilbur_speech("136w008");
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room133_136::setupSign() {
	if (_G(flags)[kRoadOpened]) {
		hotspot_set_active("sign", false);
		hotspot_set_active("sign ", true);
		_sign1 = series_play("136signr", 0x800, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_sign2 = series_play("136sinrs", 0x801, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	} else {
		hotspot_set_active("sign ", false);
		hotspot_set_active("sign", true);

		_sign1 = series_play("136signl", 0xa00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_sign2 = series_play("136sinls", 0xa01, 0, -1, 600, -1, 100, 0, 0, 0, 0);
	}
}

void Room133_136::setupSignWalkAreas() {
	if (_G(flags)[kRoadOpened]) {
		intr_add_no_walk_rect(294, 263, 332, 278, 297, 292);
		intr_add_no_walk_rect(332, 278, 380, 300, 297, 292);

		if (_walkCodes)
			intr_remove_no_walk_rect(_walkCodes);
	} else {
		_walkCodes = intr_add_no_walk_rect(144, 241, 244, 252, 167, 272);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
