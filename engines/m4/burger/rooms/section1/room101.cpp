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

#include "m4/burger/rooms/section1/room101.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/core/imath.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesStreamBreak STREAM_BREAKS1[] = {
	{  0, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{  5, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{ 14, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{ 16, nullptr, 1, 255, 11, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAM_BREAKS2[] = {
	{  0, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{  6, nullptr, 2, 255,  5, 0, nullptr, 0 },
	{ 17, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{ 24, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{ 26, nullptr, 1, 255, 14, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAM_BREAKS3[] = {
	{  0, 0, 2, 255,  4, 0, nullptr, 0 },
	{  5, 0, 2, 255,  4, 0, nullptr, 0 },
	{ 14, 0, 2, 255,  4, 0, nullptr, 0 },
	{ 16, 0, 1, 255, 17, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAM_BREAKS4[] = {
	{   0, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{   7, nullptr, 2, 255,  4, 0, nullptr, 0 },
	{  13, nullptr, 1, 255, 20, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAM_BREAKS5[] = {
	{   0, "100_010", 1, 255, -1, 0, nullptr, 0 },
	{  19, "100_011", 1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAM_BREAKS6[] = {
	{  9, nullptr, 2, 255, 4, 0, nullptr, 0 },
	{ 19, nullptr, 2, 255, 4, 0, nullptr, 0 },
	{ 28, nullptr, 2, 255, 4, 0, nullptr, 0 },
	{ 32, nullptr, 1, 255, 8, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesPlayBreak PLAY_BREAKS1[] = {
	{   0, 10, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  11, -1, "101_004", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY_BREAKS2[] = {
	{   0,  2, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{   3, -1, "102_038", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY_BREAKS3[] = {
	{   0,  8, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{   9, -1, "101_002", 1, 100, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY_BREAKS4[] = {
	{   0,  6, nullptr,   1, 255, -1,    0, 0, nullptr, 0 },
	{   7,  9, "101w005", 1, 255, -1,    0, 0, nullptr, 0 },
	{  10, -1, nullptr,   1, 255, kCHANGE_WILBUR_ANIMATION, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const char *SAID1[][4] = {
	{ "BARBERSHOP", nullptr, "101W003", "101W003" },
	{ "DOOR", "101W006", nullptr, nullptr },
	{ "FIRE ESCAPE", "101W004", nullptr, nullptr },
	{ "TOWN HALL", nullptr, "101W003", nullptr },
	{ "ALLEY", nullptr, "101W003", nullptr },
	{ "VERA'S DINER", nullptr, "101W003", nullptr },
	{ "OLD BRIDGE", nullptr, "101W003", nullptr },
	{ "HARDWARE STORE", "101W006b", "101W003", "101W003" },
	{ "BANK", "101W007", "101W003", "101W003" },
	{ "THEATRE", "101W008", "101W003", "101W003" },
	{ "POLICE STATION", "101W009", "101W003", "101W003" },
	{ "PET AND FEED STORE", "101W010", "101W003", "101W011" },
	{ "FIRE STATION", "101W012", "101W003", "101W003" },
	{ "FIRE HYDRANT", "101W013", "101W003", nullptr },
	{ "PICKUP TRUCK", "101W015", "101W003", "101W016" },
	{ nullptr, nullptr, nullptr, nullptr }
};

void Room101::init() {
	_val1 = 255;

	digi_stop(1);
	digi_preload("101_001");
	_G(kernel).call_daemon_every_loop = true;

	if (_G(game).previous_room != 102)
		door();

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		// Do nothing
		break;

	case 102:
		// Exiting barber shop
		player_set_commands_allowed(false);
		if (_G(flags)[V014]) {
			_G(flags)[V014] = 0;
			ws_demand_facing(_G(my_walker), 10);
			ws_demand_location(_G(my_walker), 338, 265);
			_G(wilbur_should) = 16;

		} else {
			ws_demand_facing(_G(my_walker), 4);
			ws_demand_location(_G(my_walker), 264, 259);
			_G(wilbur_should) = 10;
		}

		ws_hide_walker(_G(my_walker));
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 103:
		// Returning from rooftop
		player_set_commands_allowed(false);
		ws_demand_facing(_G(my_walker), 1);
		ws_demand_location(_G(my_walker), 197, 276);
		ws_hide_walker(_G(my_walker));

		_G(wilbur_should) = 6;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 104:
		// From Town Hall
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 106:
		// Leaving alley
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 134:
	case 135:
		// From Old Bridge
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 142:
		// From Vera's Diner
		_G(wilbur_should) = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		// Have been beamed down
		ws_demand_location(_G(my_walker), 320, 271);
		ws_demand_facing(_G(my_walker), 5);

		if (_G(game).previous_section > 1) {
			if (_G(flags)[kFirstTestPassed]) {
				ws_demand_location(_G(my_walker), 280, 309);
				ws_demand_facing(_G(my_walker), 8);
				player_set_commands_allowed(false);
			}

			kernel_timing_trigger(60, 6);
		}
		break;
	}

	digi_play_loop("101_001", 3, 200, -1);
}

void Room101::daemon() {
	if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
		player_update_info(_G(my_walker), &_G(player_info));

		if (_G(player_info).y > 374 && player_said("old bridge")) {
			// Changing to Old Bridge
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, GOTO_OLD_BRIDGE);
			_G(kernel).call_daemon_every_loop = false;

		} else if (_G(player_info.y < 205) && player_said("town hall")) {
			// Changing to Town Hall
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), 16, 255, 0, 30, GOTO_TOWN_HALL);
			_G(kernel).call_daemon_every_loop = false;
		}
	}

	switch (_G(kernel).trigger) {
	case 1:
		if (_val2 == 12) {
			int frame = imath_ranged_rand(8, 10);
			series_play("101ha01", 3840, 0, 1, 6, 0, 100, 0, 0, frame, frame);
		} else {
			digi_preload("101_002");
			series_play("101ha01", 3840, 0, 21, 6, 0, 100, 0, 0, 11, 13);
		}
		break;

	case 2:
		// Finished gathering items from past tests
		digi_stop(2);
		unloadSounds();
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		break;

	case 4:
		digi_play(Common::String::format("101_0%d", imath_ranged_rand(10, 17)).c_str(), 2, 255, -1);
		break;

	case 5:
		digi_play("101_017", 2, 255, -1);
		break;

	case 6:
		if (!_G(flags)[kFirstTestPassed]) {
			// No tests passed yet
			uint idx = _G(flags)[kNEURO_TEST_COUNTER];
			assert(idx < 8);

			static const char *const NAMES[8] = {
				"101w500", "101w500", "101w501", "101w502",
				"101w503", "101w504", "101w505", "101w506"
			};

			_G(walker).wilbur_speech(NAMES[idx], 23);

		} else if (!_G(flags)[kSecondTestPassed]) {
			// Second test failed
			if (_G(flags)[V242] <= 1) {
				// Oh well, at least I passed the first test
				_G(walker).wilbur_speech("101w520", 7);

			} else if (_G(flags)[V249] && !_G(flags)[kKnowHowToNukeGerbils]) {
				// I know how to nuke those mutant gerbils
				_G(walker).wilbur_speech("101w522", 7);
				_G(flags)[kKnowHowToNukeGerbils] = 1;

			} else if (_G(flags)[V248] && !_G(flags)[kGerbilsAreNasty]) {
				// Those mutant gerbils are nasty
				_G(walker).wilbur_speech("101w521", 7);
				_G(flags)[kGerbilsAreNasty] = 1;

			} else {
				// Sigh...
				_G(walker).wilbur_speech("101w524", 7);
			}
		} else if (!_G(flags)[kThirdTestPassed]) {
			// Third test failed
			if (_G(flags)[V100] <= 1) {
				_G(walker).wilbur_speech("101w530", 7);
			} else if (!_G(flags)[V113] && !_G(flags)[kGetRidOfPerkins]) {
				// Need to get rid of Perkins
				_G(walker).wilbur_speech("101w531", 7);
				_G(flags)[kGetRidOfPerkins] = 1;
			} else if (_G(flags)[V113] && !_G(flags)[kNeedToFindBurger]) {
				// Gotten rid of Perkins, need to find burger
				_G(walker).wilbur_speech("101w533", 7);
				_G(flags)[kNeedToFindBurger] = 1;
			} else if (_G(flags)[V113] && _G(flags)[V100] >= 5 &&
					!_G(flags)[kAmplifyMySenses]) {
				// Amplify my senses? Hmmm..
				_G(walker).wilbur_speech("101w534", 7);
				_G(flags)[kAmplifyMySenses] = 1;
			} else {
				_G(walker).wilbur_speech("101w532", 7);
			}
		} else if (!_G(flags)[kFourthTestPassed]) {
			// Fourth test failed
			switch (_G(flags)[V185]) {
			case 0:
			case 1:
				_G(walker).wilbur_speech("101w550", 7);
				break;
			case 2:
				_G(walker).wilbur_speech("101w552", 7);
				break;
			default:
				if (_G(flags)[V194] && !_G(flags)[ROOM101_FLAG19]) {
					_G(walker).wilbur_speech("101w551", 7);
					_G(flags)[ROOM101_FLAG19] = 1;
				} else {
					kernel_timing_trigger(60, 7);
				}
				break;
			}
		} else if (!_G(flags)[kFifthTestPassed]) {
			// Fifth test failed
			switch (_G(flags)[V153]) {
			case 0:
			case 1:
				_G(walker).wilbur_speech("101w570", 7);
				break;
			case 2:
				_G(walker).wilbur_speech("101w571", 7);
				break;
			default:
				kernel_timing_trigger(60, 7);
				break;
			}
		} else {
			kernel_trigger_dispatch_now(7);
		}
		break;

	case 7:
		loadSounds();
		ws_hide_walker(_G(my_walker));

		if (_G(flags)[kSecondTestPassed] || _G(flags)[kFourthTestPassed])
			series_load("101wi13s", -1, nullptr);
		if (_G(flags)[kThirdTestPassed])
			series_load("101wi12s", -1, nullptr);
		if (_G(flags)[kFifthTestPassed])
			series_load("101wi11s", -1, nullptr);

		_machine1 = series_play("101wi14s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
		_machine2 = series_stream_with_breaks(STREAM_BREAKS6, "101wi14", 6, 0x100, 2);
		break;

	case 8:
		kernel_timing_trigger(1, 9);
		break;

	case 9:
		daemon9();

		if (_G(flags)[kSecondTestPassed] || _G(flags)[V250] ||
				_G(flags)[V280] || _G(flags)[V002]) {
			terminateMachineAndNull(_machine1);
			terminateMachineAndNull(_machine2);
			kernel_trigger_dispatch_now(10);
		}
		break;

	case 10:
		_machine1 = series_play("101wi13s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
		_machine2 = series_stream_with_breaks(STREAM_BREAKS1, "101wi13", 6, 0x100, 2);
		break;

	case 11:
		kernel_timing_trigger(1, 12);
		break;

	case 12:
		daemon12();

		if (_G(flags)[kSecondTestPassed]) {
			if (_G(flags)[kThirdTestPassed] || _G(flags)[kPerkinsLostIsland] ||
					_G(flags)[V080] || _G(flags)[V126]) {
				terminateMachineAndNull(_machine1);
				terminateMachineAndNull(_machine2);
				kernel_timing_trigger(1, 13);
			}
		}
		break;

	case 13:
		_machine1 = series_play("101wi12s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
		_machine2 = series_stream_with_breaks(STREAM_BREAKS2, "101wi12", 6, 0x100, 2);
		break;

	case 14:
		kernel_timing_trigger(1, 15);
		break;

	case 15:
		daemon15();

		if (_G(flags)[kThirdTestPassed] && (_G(flags)[kFourthTestPassed] || _G(flags)[V220])) {
			terminateMachineAndNull(_machine1);
			terminateMachineAndNull(_machine2);
			kernel_timing_trigger(1, 16);
		}
		break;

	case 16:
		_machine1 = series_play("101wi13s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
		_machine2 = series_stream_with_breaks(STREAM_BREAKS3, "101wi13", 6, 0x100, 2);
		break;

	case 17:
		kernel_timing_trigger(1, 18);
		break;

	case 18:
		daemon18();

		if (_G(flags)[kFourthTestPassed]) {
			if (_G(flags)[kFifthTestPassed] || _G(flags)[V220]) {
				terminateMachineAndNull(_machine1);
				terminateMachineAndNull(_machine2);
				kernel_timing_trigger(1, 19);
			}
		}
		break;

	case 19:
		_machine1 = series_play("101wi11s", 0x101, 0, -1, 6, 0, 100, 0, 0, 0, -1);
		_machine2 = series_stream_with_breaks(STREAM_BREAKS4, "101wi11", 6, 0x100, 2);
		break;

	case 20:
		daemon20();
		break;

	case 21:
		digi_play("101_002", 2, 255, -1);
		_G(wilbur_should) = 18;
		series_play("101ha01", 3840, 0, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 14, -1);
		break;

	case 23:
		player_set_commands_allowed(true);
		break;

	case 24:
		if (_G(player_info).x < 218 || (_G(player_info).x < 349 && _G(player_info).y > 277)) {
			kernel_trigger_dispatch_now(27);
		} else {
			digi_preload_stream_breaks(STREAM_BREAKS5);
			series_stream_with_breaks(STREAM_BREAKS5, "101dt01", 6, 1024, 25);
		}
		break;

	case 25:
		_val1 -= 10;
		if (_val1 > 0) {
			digi_change_volume(2, _val1);
			kernel_timing_trigger(3, 25);
		} else {
			digi_stop(1);
			digi_unload_stream_breaks(STREAM_BREAKS5);
			digi_stop(2);
			digi_unload("100_013");
			player_set_commands_allowed(true);
			_val1 = 255;
		}
		break;

	case 26:
		door();
		_G(walker).reset_walker_sprites();
		digi_preload_stream_breaks(STREAM_BREAKS5);
		series_stream_with_breaks(STREAM_BREAKS5, "101dt01", 6, 1, 25);
		break;

	case 27:
		terminateMachineAndNull(_doorMachine);
		Section1::updateWalker(226, 281, 8, 26);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 2:
			player_set_commands_allowed(true);
			ws_demand_location(_G(my_walker), 336, 184);
			ws_demand_facing(_G(my_walker), 5);
			ws_walk(_G(my_walker), 335, 195, nullptr, -1, 5);
			break;

		case 3:
			player_set_commands_allowed(true);
			ws_demand_location(_G(my_walker), 482, 208);
			ws_demand_facing(_G(my_walker), 9);
			ws_walk(_G(my_walker), 410, 218, nullptr, -1, 9);
			break;

		case 4:
			player_set_commands_allowed(true);
			ws_demand_location(_G(my_walker), 224, 373);
			ws_demand_facing(_G(my_walker), 2);
			ws_walk(_G(my_walker), 282, 342, nullptr, -1, 2);
			break;

		case 5:
			player_set_commands_allowed(true);
			ws_demand_location(_G(my_walker), 0, 288);
			ws_demand_facing(_G(my_walker), 3);
			ws_walk(_G(my_walker), 30, 288, nullptr, -1, 3);
			break;

		case 6:
			_G(wilbur_should) = 7;
			series_play_with_breaks(PLAY_BREAKS1, "101wi05", 0x100, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, -53);
			break;

		case 7:
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			break;

		case 8:
			// Barbershop door open
			_G(wilbur_should) = 9;
			terminateMachineAndNull(_doorMachine);
			series_play_with_breaks(PLAY_BREAKS2, "101wi01", 3072, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 9:
			// Fade out for switching to Barbershop
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1002);
			break;

		case 10:
			_G(wilbur_should) = 11;
			series_play_with_breaks(PLAY_BREAKS3, "101wi02", 0x100, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, -53);
			break;

		case 11:
			door();
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			break;

		case 14:
			terminateMachineAndNull(_doorMachine);
			_G(wilbur_should) = 15;
			series_play_with_breaks(PLAY_BREAKS2, "101wi01", 3072, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 15:
			_G(wilbur_should) = 16;
			digi_play("101h001", 1, 255, kCHANGE_WILBUR_ANIMATION);
			break;

		case 16:
			_G(wilbur_should) = 17;
			series_play_with_breaks(PLAY_BREAKS2, "101wi03", 3072, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 17:
			_G(wilbur_should) = 19;
			_val2 = 12;

			if (_G(flags)[V005]) {
				Common::String name = Common::String::format("101h003%c",
					'a' + imath_ranged_rand(0, 5));
				digi_play(name.c_str(), 1, 255, kCHANGE_WILBUR_ANIMATION);
			} else {
				digi_play("101h002", 1, 255, kCHANGE_WILBUR_ANIMATION);
				_G(flags)[V005] = 1;
			}

			series_play("101ha01", 3840, 0, 1, 6, 0, 100, 0, 0, 0, 7);
			break;

		case 18:
			door();
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			break;

		case 19:
			_val2 = 13;
			break;

		case 20:
			_G(wilbur_should) = 21;
			series_play_with_breaks(PLAY_BREAKS4, "101wi04", 0x100, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, -53);
			break;

		case 21:
			_G(wilbur_should) = 22;
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1003);
			break;

		case 22:
			series_play("101wi04", 0x100, 0, -1, 10, -1, 100, 0, -53, 16, 16);
			series_play("101wi04s", 0x100, 0, -1, 10, -1, 100, 0, -53, 16, 16);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kBurlEntersTown:
		if (_G(flags)[kRoadOpened]) {
			_G(kernel).continue_handling_trigger = true;

		} else if (player_commands_allowed() && _G(player).walker_visible &&
				INTERFACE_VISIBLE && !digi_play_state(1)) {
			Section1::updateDisablePlayer();
			digi_preload("100_013");
			digi_play("100_013", 2, 255, -1);
			kernel_timing_trigger(240, 24);
		} else {
			kernel_timing_trigger(60, kBurlEntersTown);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room101::pre_parser() {
	bool lookAt = player_said_any("look", "look at");

	if (player_said("vera's diner") && !player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(9);

	if (player_said("alley") && !player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(3);

	_G(kernel).call_daemon_every_loop = player_said("ENTER", "OLD BRIDGE") ||
		player_said("gear", "old bridge") ||
		player_said("old bridge") ||
		player_said("ENTER", "TOWN HALL") ||
		player_said("gear", "town hall") ||
		(lookAt && player_said("town hall"));
}

void Room101::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID1)) {
		// Nothing
	} else if (player_said("ENTER", "DOOR") || player_said("gear", "door"))   {
		if (_G(flags)[V012] == 2) {
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_G(wilbur_should) = 8;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else if (_G(flags)[V013]) {
			ws_demand_location(_G(my_walker), 338, 265);
			ws_demand_facing(_G(my_walker), 10);
			ws_hide_walker(_G(my_walker));
			player_set_commands_allowed(false);
			_G(wilbur_should) = 14;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_G(wilbur_should) = 8;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}

	} else if (player_said("ENTER", "FIRE ESCAPE") || player_said("gear", "fire escape")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 20;
		ws_hide_walker(_G(my_walker));
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("ENTER", "TOWN HALL") || player_said("gear", "town hall") ||
			(lookFlag && player_said("town hall"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1004);

	} else if (player_said("ENTER", "ALLEY") || player_said("gear", "alley") ||
			(lookFlag && player_said("alley"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1006);

	} else if (player_said("ENTER", "OLD BRIDGE") || player_said("gear", "old bridge") ||
			(lookFlag && player_said("old bridge"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1008);

	} else if (player_said("ENTER", "VERA'S DINER") || player_said("gear", "vera's diner") ||
			(lookFlag && player_said("vera's diner"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1014);

	} else if (inv_player_has(_G(player).verb) &&
			player_said_any("alley", "town hall", "vera's diner", "old bridge")) {
		_G(walker).wilbur_speech("101w003");

	} else if (inv_player_has(_G(player).verb) && player_said("fire hydrant")) {
		_G(walker).wilbur_speech("101w014");

	} else if (lookFlag && player_said("barbershop")) {
		_G(walker).wilbur_speech(_G(flags)[V013] ? "101w002" : "101w001");

	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room101::door() {
	_doorMachine = series_play("101door", 3840, 0, -1, 10, -1, 100, 0, -53, 0, 0);
}

void Room101::loadSounds() {
	digi_preload("101_010");
	digi_preload("101_011");
	digi_preload("101_012");
	digi_preload("101_013");
	digi_preload("101_014");
	digi_preload("101_015");
	digi_preload("101_016");
	digi_preload("101_017");
}

void Room101::unloadSounds() {
	digi_unload("101_010");
	digi_unload("101_011");
	digi_unload("101_012");
	digi_unload("101_013");
	digi_unload("101_014");
	digi_unload("101_015");
	digi_unload("101_016");
	digi_unload("101_017");
}

void Room101::daemon9() {
	_G(flags)[V019] = 1;
	_G(flags)[V017] = 1;

	if (_G(flags)[V021] == 10032)
		_G(flags)[V001] = 12;

	if (_G(flags)[V021] == 10033)
		_G(flags)[V013] = 1;

	_G(flags).set_boonsville_time(600);
}

void Room101::daemon12() {
	if (_G(flags)[V250] || _G(flags)[kSecondTestPassed])
		inv_give_to_player("BLOCK OF ICE");

	if (_G(flags)[V280] || _G(flags)[kSecondTestPassed])
		inv_give_to_player("PANTYHOSE");

	if (_G(flags)[V002]) {
		inv_give_to_player("PHONE BILL");
		inv_give_to_player("WHISTLE");
		_G(flags)[V079] = 0;
		_G(flags)[V080] = 1;
	}

	_G(flags).set_boonsville_time(2400);
}

void Room101::daemon15() {
	if (_G(flags)[V002] || !_G(flags)[V126] || _G(flags)[kThirdTestPassed]) {
		inv_give_to_player("PHONE BILL");
		inv_give_to_player("WHISTLE");
		_G(flags)[V079] = 0;
		_G(flags)[V080] = 1;
	} else {
		_G(flags)[V079] = 1;
		_G(flags)[V080] = 0;
	}

	if (_G(flags)[V126] || _G(flags)[kThirdTestPassed]) {
		inv_give_to_player("CARROT JUICE");
		_G(flags)[V088] = 1;
		_G(flags)[V091] = 1;
	} else {
		_G(flags)[V088] = 0;
		_G(flags)[V091] = 0;
	}

	if (_G(flags)[V113] || _G(flags)[kThirdTestPassed]) {
		_G(flags).set_boonsville_time(6001);
		_G(flags)[V000] = 1002;
		_G(flags)[V001] = 0;
		inv_move_object("MONEY", NOWHERE);
		_G(flags)[V039] = 1;
		_G(flags)[kRoadOpened] = 1;
		inv_give_to_player("DEED");
		_G(flags)[kPerkinsLostIsland] = 1;
	} else {
		_G(flags)[V000] = 1000;
		if (_G(flags)[V021] == 10032)
			_G(flags)[V001] = 12;

		_G(flags)[V039] = 0;
		_G(flags)[kRoadOpened] = 0;
		_G(flags)[kTourBusAtDiner] = 0;
	}

	_G(flags)[V063] = 0;
	_G(flags)[V092] = 0;

	if (_G(flags)[kThirdTestPassed])
		_G(flags).set_boonsville_time(6600);
}

void Room101::daemon18() {
	if (_G(flags)[V220] || _G(flags)[kFourthTestPassed]) {
		inv_give_to_player("laxative");
		inv_give_to_player("amplifier");
		_G(flags)[kDrumzFled] = 1;
		_G(flags)[kTourBusAtDiner] = 1;
		_G(flags).set_boonsville_time(19200);
	}
}

void Room101::daemon20() {
	if (_G(flags)[V177] || _G(flags)[kFifthTestPassed]) {
		inv_move_object("KEYS", 138);
		inv_give_to_player("JAWZ O' LIFE");
		_G(flags)[V046] = 0;
	}

	_G(flags)[kPoliceState] = 0;
	_G(flags)[kPoliceCheckCtr] = 0;
	_G(flags).set_boonsville_time(22800);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
