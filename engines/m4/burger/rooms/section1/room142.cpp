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

#include "m4/burger/rooms/section1/room142.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "VERA'S DINER",   "142w001", "142w002", "142w002" },
	{ "CAR",            "142w003", nullptr,   "142w004" },
	{ "FANBELT",        "142w005", nullptr,   "142w002" },
	{ "ICE BOX",        "142w006", "142w002", nullptr   },
	{ "ICE",            nullptr,   nullptr,   "142w002" },
	{ "GARBAGE CANS",   "142w010", "142w011", "142w012" },
	{ "BACK DOOR",      "142w013", "142w002", nullptr   },
	{ "PLANTS",         "142w014", "142w015", "142w002" },
	{ "SIGN",           "142w016", "142w017", "142w002" },
	{ "SIGN ",          "142w018", "142w002", "142w002" },
	{ "FRONT DOOR",     nullptr,   "142w002", nullptr   },
	{ "MAIN STREET",    nullptr,   "142w002", nullptr   },
	{ "HANLON'S POINT", nullptr,   "142w002", nullptr   },
	{ "AUNT POLLY'S HOUSE", nullptr, "142w002", nullptr },
	{ "PARKING LOT",    nullptr,   "142w002", nullptr   },
	{ "HIGHWAY 2",      "142w020", "142w002", "142w021" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const WilburMatch Room142::MATCH[] = {
	{ "GEAR", "PARKING LOT", 9, &Flags::_flags[V000], 1003, &_val1, 18 },
	{ "LOOK AT", "PARKING LOT", 9, &Flags::_flags[V000], 1003, &_val1, 18 },
	{ "GEAR", "PARKING LOT", 5, &Flags::_flags[kTourBusAtDiner], 0, &_val2, 13 },
	{ "LOOK AT", "PARKING LOT", 5, &Flags::_flags[kTourBusAtDiner], 0, &_val2, 13 },
	{ "GEAR", "ICE BOX", kCHANGE_WILBUR_ANIMATION, &Flags::_flags[kIceBoxOpened], 0, &Vars::_wilbur_should, 1 },
	{ "TAKE", "FANBELT", kCHANGE_WILBUR_ANIMATION, nullptr, 0, &Vars::_wilbur_should, 9 },
	{ "GEAR", "BACK DOOR", 6, nullptr, 0, nullptr, 0 },
	WILBUR_MATCH_END
};

const seriesPlayBreak Room142::PLAY1[] = {
	{  8, 12, "100_010", 2, 255, -1, 0, 0, &_val3,  1 },
	{  8, 12, "100_011", 2, 255, -1, 0, 0, &_val3,  2 },
	{  8, 12, "100_012", 2, 255, -1, 0, 0, &_val3,  3 },
	{ 13, -1, nullptr,   0,   0, 10, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY2[] = {
	{  0, -1, "100_015", 2, 255, -1, 1, 3, nullptr, 0 },
	{  0, -1, "100_017", 2, 255, -1, 1, 3, nullptr, 0 },
	{  1,  1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY3[] = {
	{ 0,  26, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 27, -1, nullptr, 0, 0, 14, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY4[] = {
	{  0, 2, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{  3, 6, "142b901", 2, 255, -1, 4, -1, nullptr, 0 },
	{  7, -1, nullptr,  0,   0, -1, 0,  0, nullptr, 0 },
	{ -1, -1, nullptr,  0,   0, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY5[] = {
	{ 1,  1, "100_021", 2, 255, -1, 0, 12, nullptr, 0 },
	{ 0, -1, "100_015", 2, 255, -1, 1,  4, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY6[] = {
	{  0,  0, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  1, 10, "100_010", 2, 255, -1, 0, 0, &_val3,  1 },
	{  1, 10, "100_011", 2, 255, -1, 0, 0, &_val3,  2 },
	{  1, 10, "100_012", 2, 255, -1, 0, 0, &_val3,  3 },
	{ 11, 14, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY7[] = {
	{ 15, -1, "100_010", 2, 255, -1, 0, 0, &_val3,  1 },
	{ 15, -1, "100_011", 2, 255, -1, 0, 0, &_val3,  2 },
	{ 15, -1, "100_012", 2, 255, -1, 0, 0, &_val3,  3 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY8[] = {
	{ 0,  5, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 6, 25, nullptr, 0, 0, 13, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY9[] = {
	{ 0, 3, "142_003", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY10[] = {
	{ 3, 0, "142_002", 1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY11[] = {
	{  4, 13, "142_001", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 14, -1, "142_002", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY12[] = {
	{ 0, -1, "142_005", 1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY13[] = {
	{  0,  1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{  2,  7, "100_010", 2, 255, -1, 0, 0, &_val3,  1 },
	{  2,  7, "100_011", 2, 255, -1, 0, 0, &_val3,  2 },
	{  2,  7, "100_012", 2, 255, -1, 0, 0, &_val3,  3 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room142::PLAY14[] = {
	{ 0, 19, "142_007", 2, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};


int32 Room142::_val1;
int32 Room142::_val2;
int32 Room142::_val3;

Room142::Room142() : Room() {
	_val1 = 0;
	_val2 = 0;
	_val3 = 0;
}

void Room142::init() {
	digi_preload("142_004");
	digi_play_loop("142_004", 3, 255, -1);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[kIceBoxOpened]) {
			ws_hide_walker();
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 101:
		ws_demand_location(120, 400, 2);
		ws_walk(120, 344, 0, -1, -1);
		break;

	case 139:
		ws_demand_location(-40, 375, 2);
		ws_walk(25, 344, 0, -1, -1);
		break;

	case 143:
		ws_demand_location(350, 270, 8);
		ws_walk(297, 275, 0, -1, -1);
		break;

	case 145:
		ws_demand_location(293, 275, 7);
		ws_walk(282, 280, 0, -1, -1);
		break;

	case 170:
		ws_demand_location(680, 325, 9);
		ws_walk(613, 331, 0, -1, -1);
		break;

	default:
		ws_demand_location(270, 320);
		ws_demand_facing(5);
		break;
	}

	series_play("142sm01", 0xf00, 4, -1, 6, -1);
	_series1 = series_show("142door", 0xe00);

	if (inv_player_has("PANTYHOSE")) {
		hotspot_set_active("FANBELT", false);
		hotspot_set_active("ENGINE", true);
	} else {
		hotspot_set_active("FANBELT", true);
		hotspot_set_active("ENGINE", false);
	}

	hotspot_set_active("TRUCK", false);

	if (_G(flags)[V000] == 1003) {
		_noWalk = intr_add_no_walk_rect(230, 250, 294, 277, 229, 278,
			_G(screenCodeBuff)->get_buffer());
		_series2 = series_show("142dt01", 0xd00, 0, -1, -1, 22);
		_series3 = series_show("142dt01s", 0xd01, 0, -1, -1, 22);
		hotspot_set_active("TRUCK", true);
	}

	hotspot_set_active("TOUR BUS", false);

	if (_G(flags)[kTourBusAtDiner]) {
		_series4 = series_show("142ba01", 0xf00, 0, -1, -1, 21);
		hotspot_set_active("TOUR BUS", true);
	}

	if (!_G(flags)[kIceBoxOpened]) {
		_series5 = series_show("142icedr", 0xe00);
		hotspot_set_active("ICE", false);
	}
}

void Room142::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		terminateMachineAndNull(_series8);
		terminateMachineAndNull(_series5);
		terminateMachineAndNull(_series1);

		if (_G(player_info.y) < 285) {
			Section1::updateWalker(getRoadEdgeX(_G(player_info).y), 285, 9, 2);
		} else {
			Section1::updateWalker(getRoadEdgeX(_G(player_info).y), _G(player_info).y, 9, 2);
		}
		break;

	case 2:
		_series8 = series_play("142sm01", 0xf00, 4);
		_series5 = series_play("142icedr", 0xe00);
		_series1 = series_play("142door", 0xe00);
		_G(walker).reset_walker_sprites();
		play015();
		break;

	case 3:
		play015();
		break;

	case 4:
		digi_unload("142_006");
		digi_play_loop("142_004", 3);
		hotspot_set_active("TOUR BUS", true);
		_series4 = series_play("142ba01", 0xf00, 16, -1, 10, 0, 100, 0, 0, 20, 21);
		break;

	case 5:
		switch (_val2) {
		case 11:
			wilbur_speech("142w007");
			break;
		case 12:
			wilbur_speech(inv_player_has("BLOCK OF ICE") ?
				"142w007" : "142w008");
			break;
		case 13:
			wilbur_speech("142w019");
			break;
		default:
			break;
		}
		break;

	case 6:
		player_set_commands_allowed(false);
		digi_play("142v901", 1, 255, 7);
		break;

	case 7:
		digi_play("142e901", 1, 255, 8);
		break;

	case 8:
		player_set_commands_allowed(true);
		break;

	case 9:
		switch (_val1) {
		case 14:
			_val3 = imath_ranged_rand(1, 3);
			_val1 = 15;
			_volume = 155;
			series_play_with_breaks(PLAY1, "142dt01", 0xd00, 9, 3, 6, 100);
			break;

		case 15:
			digi_stop(1);
			digi_unload("100_015");
			faceTruck();
			hotspot_set_active("TRUCK", true);
			_val1 = 16;
			series_play_with_breaks(PLAY2, "142dt02", 0xd00, 9, 3, 8, 100);
			break;

		case 16:
			_val1 = 17;
			series_play_with_breaks(PLAY3, "142bu01", 0xd00, 9, 3, 6, 100);
			break;

		case 17:
			_noWalk = intr_add_no_walk_rect(230, 250, 294, 277, 229, 278);
			player_set_commands_allowed(true);
			_series2 = series_show("142dt01", 0xd00, 0, -1, -1, 22);
			_series3 = series_show("142dt01s", 0xd01, 0, -1, -1, 22);
			_series1 = series_show("142door", 0xe00);
			break;

		case 18:
			player_set_commands_allowed(false);
			ws_turn_to_face(3);
			series_play_with_breaks(PLAY4, "142bu02", 0xd00, 8, 2, 6, 100);
			break;

		case 19:
			_val1 = 20;
			series_play_with_breaks(PLAY5, "142dt02", 0xd00, 9, 3, 8, 100);
			break;

		case 20:
			digi_preload("100_015");
			digi_play_loop("100_015", 1);
			_val3 = imath_ranged_rand(1, 3);
			_val1 = 21;
			series_play_with_breaks(PLAY6, "142dt03", 0xd00, 9, 3);
			break;

		case 21:
			_val3 = imath_ranged_rand(1, 3);
			series_play_with_breaks(PLAY7, "142dt03", 0xf00, 8, 3);
			_volume = 255;
			kernel_trigger_dispatch_now(11);
			break;

		default:
			break;
		}
		break;

	case 10:
		_volume -= 10;
		if (_volume > 0) {
			term_message("fading truck theme, current volume = %d", _volume);
			digi_change_volume(3, _volume);
			kernel_timing_trigger(6, 10);
		} else {
			digi_stop(3);
			digi_unload("100_013");
			digi_play_loop("142_004", 3);
		}
		break;

	case 11:
		_volume -= 20;
		if (_volume > 0) {
			term_message("fading truck noise, current volume = %d", _volume);
			digi_change_volume(1, _volume);
			kernel_timing_trigger(6, 11);
		} else {
			digi_stop(1);
			digi_stop(2);
			digi_unload("100_015");
			digi_unload("100_021");
		}
		break;

	case 13:
		_series1 = series_show("142door", 0xe00);
		break;

	case 14:
		terminateMachineAndNull(_series1);
		break;


	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			terminateMachineAndNull(_series5);
			disable_player();
			_G(wilbur_should) = 2;
			series_play_with_breaks(PLAY9, "142wi04", 0xdff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 2:
			player_set_commands_allowed(true);
			_G(flags)[kIceBoxOpened] = 1;
			hotspot_set_active("ICE", true);
			_series6 = series_show("142wi04", 0xdff, 0, -1, -1, 3);
			_series7 = series_show("142wi04s", 0xdfe, 0, -1, -1, 3);
			break;

		case 3:
			if (inv_player_has("BLOCK OF ICE")) {
				wilbur_speech("142w009");
			} else {
				player_set_commands_allowed(false);
				terminateMachineAndNull(_series6);
				terminateMachineAndNull(_series7);
				_G(wilbur_should) = 4;

				series_play_with_breaks(PLAY11, "142wi04", 0xdff, kCHANGE_WILBUR_ANIMATION, 3);
			}
			break;

		case 4:
			_G(flags)[kIceBoxOpened] = 0;
			hotspot_set_active("ICE", false);
			_series5 = series_show("142icedr", 0xf00);
			enable_player();
			inv_give_to_player("BLOCK OF ICE");
			break;

		case 5:
			player_set_commands_allowed(false);
			terminateMachineAndNull(_series6);
			terminateMachineAndNull(_series7);
			_G(wilbur_should) = 6;

			series_play_with_breaks(PLAY10, "142wi02", 0xdff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 6:
			_G(flags)[kIceBoxOpened] = 0;
			hotspot_set_active("ICE", false);
			_series5 = series_show("142icedr", 0xe00);
			enable_player();
			break;

		case 7:
			player_set_commands_allowed(false);
			terminateMachineAndNull(_series6);
			terminateMachineAndNull(_series7);
			_G(wilbur_should) = 8;

			series_play_with_breaks(PLAY10, "142wi04", 0xdff, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 8:
			_G(flags)[kIceBoxOpened] = 0;
			hotspot_set_active("ICE", false);
			_series5 = series_show("142icedr", 0xe00);
			enable_player();
			triggerParser();
			break;

		case 9:
			disable_player();
			_G(wilbur_should) = 10;
			series_play_with_breaks(PLAY12, "142wi05", 0x400, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 10:
			enable_player();
			hotspot_set_active("FANBELT", false);
			inv_give_to_player("PANTYHOSE");
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kBurlEntersTown:
		if (!_G(flags)[kRoadOpened]) {
			if (_series2) {
				terminateMachineAndNull(_series2);
				terminateMachineAndNull(_series3);
			}
			if (_series4) {
				terminateMachineAndNull(_series4);
				_G(flags)[kTourBusAtDiner] = 0;
			}

			_G(flags)[V000] = 1003;

			if (player_commands_allowed() && _G(player).walker_visible &&
					INTERFACE_VISIBLE && !digi_play_state(1)) {
				digi_preload("100_013");
				digi_play("100_013", 3, 155);
				Section1::updateDisablePlayer();
				preloadAssets2(); 

				if (getRoadEdgeX(_G(player_info).y) > _G(player_info).x ||
						(_G(player_info).x < 298 && _G(player_info).y < 285)) {
					// Get off the road
					kernel_timing_trigger(240, 1);
				} else {
					// Not on road, so show truck arriving
					kernel_timing_trigger(240, 3);
				}
			} else {
				kernel_timing_trigger(15, 10028);
			}
		} else {
			_G(kernel).continue_handling_trigger = true;
		}
		break;

	case kBurlLeavesTown:
		if (_G(flags)[V000] == 1002) {
			_G(kernel).continue_handling_trigger = true;

		} else {
			player_update_info();

			if (player_commands_allowed() && _G(player).walker_visible &&
					INTERFACE_VISIBLE && !digi_play_state(1)) {
				player_set_commands_allowed(false);

				if (_G(player_info).y > 285) {
					faceTruck();
				} else {
					player_walk_to(267, 290, 262, 278);
				}

				preloadAssets();
				_G(flags)[V000] = 1004;
				intr_remove_no_walk_rect(_noWalk);
				hotspot_set_active("TRUCK", false);

				terminateMachineAndNull(_series1);
				terminateMachineAndNull(_series2);

				_val1 = 19;
				series_play_with_breaks(PLAY8, "142bu03", 0xd00, 9, 3);
			} else {
				kernel_timing_trigger(15, kBurlLeavesTown);
			}
		}
		break;

	case kBandEntersTown:
		if (_series2) {
			terminateMachineAndNull(_series2);
			_G(flags)[V000] = _G(flags)[kRoadOpened] ? 1002 : 1004;
		}

		if (!_G(flags)[kTourBusAtDiner]) {
			_G(flags)[kTourBusAtDiner] = 1;
			digi_preload("142_006");
			digi_play("142_006", 3, 255, 4);
			series_play_with_breaks(PLAY14, "142ba01", 0xf00, -1, 2);
		}
		break;

	case kCALLED_EACH_LOOP:
		if (_actionType == 0) {
			_G(kernel).call_daemon_every_loop = false;
		} else {
			checkAction();
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room142::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[kIceBoxOpened]) {
		_G(player).ready_to_walk = false;
		_G(player).need_to_walk = false;

		if (player_said("GEAR", "ICE BOX")) {
			_G(wilbur_should) = 5;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else if (player_said("ICE")) {
			if (player_said("LOOK AT")) {
				if (_G(flags)[V040] && !inv_player_has("BLOCK OF ICE")) {
					wilbur_speech("142w008");
				} else {
					wilbur_speech("142w007");
				}
			} else if (player_said("TAKE")) {
				if (!_G(flags)[V040]) {
					wilbur_speech("142w017");
				} else if (inv_player_has("BLOCK_OF_ICE")) {
					wilbur_speech("142w009");
				} else {
					_G(wilbur_should) = 3;
					kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				}
			} else if (player_said("GEAR")) {
				wilbur_speech("142w002");
			} else {
				term_message("ERROR - don't know what to do with ice!!!");
			}
		} else {
			_G(wilbur_should) = 7;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}

		_G(player).command_ready = false;

	} else {
		if (_G(flags)[V000] == 1003 &&
			_G(player).walk_x >= 230 && _G(player).walk_x <= 294 &&
			_G(player).walk_y >= 250 && _G(player).walk_y <= 277) {
			player_hotspot_walk_override(_G(player).click_x, 278);
		} else if (player_said_any("GEAR", "LOOK AT")) {
			if (player_said("MAIN STREET")) {
				player_hotspot_walk_override(_G(player).walk_x, 400);
				checkAction();
				_G(kernel).call_daemon_every_loop = true;
				_G(player).command_ready = false;
			} else if (player_said("PARKING LOT") && _G(flags)[kTourBusAtDiner]) {
				checkAction();
				_G(kernel).call_daemon_every_loop = true;
				_G(player).command_ready = false;
			}

		} else if (player_said("MAIN STREET")) {
			player_set_facing_at(120, 400);
		} else if (player_said("HANLON'S POINT")) {
			player_set_facing_at(-40, 375);
		} else if (player_said("AUNT POLLY'S HOUSE")) {
			player_set_facing_at(680, 325);
		} else if (player_said("FRONT DOOR")) {
			player_set_facing_at(350, 270);
		} else if (player_said("HIGHWAY 2")) {
			player_set_facing_at(192, 252);
		} else if (player_said("PARKING LOT")) {
			player_set_facing_at(303, 247);
		} else if (player_said("TOUR BUS")) {
			checkAction();
			_G(kernel).call_daemon_every_loop = true;
			_G(player).command_ready = false;
		}
	}
}

void Room142::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(walker).wilbur_said(SAID)) {
		if (player_said_any("GEAR", "LOOK AT") && player_said("HANLON'S POINT")) {
			disable_player_commands_and_fade_init(1012);
		} else if (player_said_any("GEAR", "LOOK AT") && player_said("FRONT DOOR")) {
			disable_player_commands_and_fade_init(1015);
		} else if (player_said_any("GEAR", "LOOK AT") && player_said("AUNT POLLY'S HOUSE")) {
			disable_player_commands_and_fade_init(1017);
		} else if (!_G(walker).wilbur_match(MATCH)) {
			return;
		}
	}

	_G(player).command_ready = false;
}

void Room142::checkAction() {
	_actionType = 0;

	if (player_said_any("GEAR", "LOOK AT", "GO TO")) {
		if (player_said("MAIN STREET")) {
			_actionType = 1;
		} else if (_G(flags)[kTourBusAtDiner] && (player_said("PARKING LOT") ||
				player_said("GO TO", "TOUR BUS"))) {
			_actionType = 2;
		}
	}

	if (_actionType) {
		player_update_info();

		if (_actionType == 1 && _G(player_info).y > 375 && player_commands_allowed()) {
			player_update_info();
			player_hotspot_walk_override(_G(player_info).x, 400);
			disable_player_commands_and_fade_init(1001);

		} else if (_actionType == 2 && _G(player_info).y < 280 &&
				player_commands_allowed()) {
			disable_player_commands_and_fade_init(1016);
		}
	}
}

void Room142::faceTruck() {
	ws_turn_to_face(calc_facing(262, 277));
}

void Room142::preloadAssets() {
	series_load("142dt02");
	series_load("142dt02s");
	series_load("142dt03");
	series_load("142dt03s");

	digi_preload_play_breaks(PLAY5);
	digi_preload_play_breaks(PLAY6);
	digi_preload_play_breaks(PLAY7);
}

void Room142::preloadAssets2() {
	series_load("142dt01");
	series_load("142dt01s");
	series_load("142dt02");
	series_load("142dt02s");
	digi_preload_play_breaks(PLAY1, true);
	digi_preload_play_breaks(PLAY1, true);
}

void Room142::triggerParser() {
	_G(player).waiting_for_walk = true;
	_G(player).ready_to_walk = true;
	_G(player).need_to_walk = true;
	_G(player).command_ready = true;
	_G(kernel).trigger = -1;
	_G(kernel).trigger_mode = KT_PREPARSE;
	_G(player).walker_trigger = -1;

	pre_parser();
	g_engine->global_pre_parser();
}

void Room142::play015() {
	digi_preload("100_015");
	digi_play_loop("100_015", 1);

	_val3 = imath_ranged_rand(1, 3);
	_val1 = 14;
	series_play_with_breaks(PLAY13, "142dt01", 0x100, 9, 3);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
