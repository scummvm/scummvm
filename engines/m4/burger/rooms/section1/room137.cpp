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

#include "m4/burger/rooms/section1/room137.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_SHERRIF_ANIMATION = 1,
	kCHANGE_DEPUTY_ANIMATION = 2,
	kSOMEONE_TOOK_KEYS = 4
};

static const char *SAID[][4] = {
	{ "PATROL CAR",       "137W002", "137W003", "137W004" },
	{ "TRUNK",            nullptr,   "137W008", "137W008" },
	{ "JAWZ O' LIFE",     "137W010", nullptr,   nullptr   },
	{ "SIGN",             "137W012", "137W008", "137W008" },
	{ "CAR WINDOW",       nullptr,   "137W008", "137W008" },
	{ "HIGHWAY 2",        "137W013", "137W008", "137W008" },
	{ "FORK IN THE ROAD", nullptr,   "137W008", "137W008" },
	{ "KEYS",             "137W009", nullptr,   nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesStreamBreak SERIES1[] = {
	{  9, "100_010", 1, 255, -1, 0, 0, 0 },
	{ 20, nullptr,   1, 255, 12, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesPlayBreak PLAY1[] = {
	{  0, 24, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 25, 32, "137_005", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 33, 33, nullptr,   1, 255, 16, 0, 0, nullptr, 0 },
	{ 34, 40, nullptr,   1, 255, 22, 0, 0, nullptr, 0 },
	{ 41, -1, nullptr,   1, 255, 19, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{  0,  7, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  8,  8, nullptr,   1, 255, 17, 0, 0, nullptr, 0 },
	{  9, 13, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 14, 14, nullptr,   1, 255, 21, 0, 0, nullptr, 0 },
	{ 14, 14, "137_006", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 15, -1, nullptr,   1, 255, 18, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  0, 11, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, nullptr,   1, 255, 20, 0, 0, nullptr, 0 },
	{ 13, -1, "137_007", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room137::init() {
	player_set_commands_allowed(true);
	_G(kernel).call_daemon_every_loop = true;
	_volume = 255;
	_flag1 = true;
	_flag2 = false;
	_arrested = false;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[kPoliceCheckCtr])
			_G(flags)[kPoliceCheckCtr] = 1;
		break;

	case 136:
		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		kernel_timing_trigger(1, kCHANGE_WILBUR_ANIMATION);
		break;

	case 138:
		ws_demand_facing(2);

		if (_G(flags)[kPoliceCheckCtr] >= 200) {
			ws_demand_location(264, 347);
			_flag1 = false;
			digi_preload("137_003");
		} else {
			ws_demand_location(290, 334);
			ws_hide_walker();
			player_set_commands_allowed(false);
			_G(wilbur_should) = 1;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}

		if (_G(flags)[kPoliceCheckCtr] < 200 && _G(flags)[kPoliceCheckCtr] && _G(flags)[kPoliceState] == 4)
			_G(flags)[kPoliceState] = 5;
		break;

	default:
		ws_demand_location(183, 216, 8);
		break;
	}

	_door = series_play("137do01", 0x700, 0, -1, 600, -1, 100, 0, 0, 0, 0);
	jawz();

	const char *NAMES[18] = {
		"137_020", "137_021", "137_022", "137_023", "137_024", "137_025",
		"137_026", "137_027", "137_028", "137_013", "137_014", "137_026",
		"137_027", "137_028", "137_013", "137_014", "137_015", "137_016"
	};
	for (int i = 0; i < 18; ++i)
		digi_preload(NAMES[i]);

	if (inv_object_in_scene("keys", 138) && _G(flags)[kPoliceState] != 2) {
		digi_preload("137_001");
		digi_play_loop("137_001", 3);
	} else {
		digi_preload("137_002");
		digi_play_loop("137_002", 3);
	}

	if (_G(flags)[kPoliceCheckCtr] < 200) {
		_deputyShould = 27;
	} else {
		_deputyShould = _G(flags)[kPoliceState] == 2 || _G(flags)[kPoliceState] == 3 || _G(flags)[kPoliceState] == 4 ? 34 : 27;
		digi_play("137_003", 1);
	}

	_deputyMode = 27;
	kernel_trigger_dispatch_now(kCHANGE_DEPUTY_ANIMATION);
}

void Room137::daemon() {
	int frame;

	if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
		player_update_info();

		if (_G(player_info).y < 235 && player_said("FORK IN THE ROAD")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1009);
		}
	}

	if (!digi_play_state(2) && _flag1 && imath_ranged_rand(1, 3000) == 235) {
		// Occasional actions. *VERY* occasional actions
		if (_flag2 && inv_object_in_scene("keys", 138)) {
			digi_play("137_022", 2, 100);
			_flag2 = false;
		} else if (inv_object_in_scene("keys", 138)) {
			digi_play(imath_ranged_rand(1, 7) == 1 ? "137_020" : "137_021", 2);

		} else {
			digi_play(Common::String::format("137_0%d", imath_ranged_rand(23, 28)).c_str(), 2, 150);
		}
	}

	switch (_G(kernel).trigger) {
	case kCHANGE_SHERRIF_ANIMATION:
		switch (_sherrifMode) {
		case 5:
			switch (_sherrifShould) {
			case 10:
				_sherrifShould = 11;
				series_play("137sh01", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 17);
				break;

			case 11:
				_sherrifShould = 12;
				series_play("137sh07", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 8);
				break;

			case 12:
				startPoliceTalk();
				_sherrifShould = 22;
				_sherrifMode = 10;
				kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
				break;

			case 14:
				_sherrifShould = 15;
				series_play("137sh01", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 17);
				break;

			case 15:
				startPoliceTalk();
				_sherrifMode = 14;
				_sherrifShould = 22;
				kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
				break;

			case 17:
				_sherrifShould = 18;
				series_play("137sh01", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 17);
				break;

			case 18:
				_sherrifShould = 19;
				series_play("137sh07", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 8);
				break;

			case 19:
				_sherrifShould = 20;
				series_play("137sh09", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 8);
				break;

			case 20:
				startPoliceTalk();
				_sherrifShould = 22;
				_sherrifMode = 17;
				kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
				break;

			default:
				break;
			}
			break;

		case 10:
			switch (_sherrifShould) {
			case 22:
				series_play("137sh08", 0x800, 0, 1, 10, 0, 100, 0, 0, 0, 0);
				break;

			case 23:
				_sherrifMode = 13;
				series_play("137sh08", 0x800, 0, 1, 6, 0, 100, 0, 0, 0, 0);
				break;

			default:
				break;
			}
			break;

		case 13:
			if (_sherrifShould == 23) {
				frame = imath_ranged_rand(0, 6);
				series_play("137sh08", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0, frame, frame);

				if (_digi1) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_digi1, 1, 255, 14);
					_digi1 = nullptr;
				}
			} else {
				_sherrifMode = 10;
				series_play("137sh08", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 14:
			switch (_sherrifShould) {
			case 22:
				if (imath_ranged_rand(1, 30) == 2) {
					series_play("137sh04", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0, 0, 14);
				} else {
					series_play("137sh01", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 12, 0, 100, 0, 0, 17, 17);
				}
				break;

			case 23:
				_sherrifMode = 16;
				series_play("137sh03", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;

			case 24:
				_sherrifShould = 23;
				series_play("137sh05", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0, 0, 17);

				if (_digi1) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_digi1, 1, 255, 14);
					_digi1 = nullptr;
				}
				break;

			case 25:
				_sherrifShould = 26;
				series_play("137sh06", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0, 0, 23);
				break;

			case 26:
				conv_resume_curr();
				_sherrifShould = 22;
				kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
				break;

			default:
				break;
			}
			break;

		case 16:
			if (_sherrifShould == 23) {
				frame = imath_ranged_rand(0, 8);
				series_play("137sh03", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0);

				if (_digi1) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_digi1, 1, 255, 14);
					_digi1 = nullptr;
				}
			} else {
				_sherrifMode = 14;
				series_play("137sh03", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 17:
			switch (_sherrifShould) {
			case 22:
				series_play("137sh09", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
				break;

			case 23:
				_sherrifMode = 21;
				series_play("137sh10", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;

			default:
				break;
			}
			break;

		case 21:
			if (_sherrifShould == 23) {
				frame = imath_ranged_rand(0, 5);
				series_play("137sh10", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 6, 0, 100, 0, 0, frame, frame);

				if (_digi1) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_digi1, 1, 255, 14);
					_digi1 = nullptr;
				}
			} else {
				_sherrifMode = 17;
				series_play("137sh10", 0x800, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 0);
			}
			break;

		default:
			break;
		}
		break;

	case kCHANGE_DEPUTY_ANIMATION:
		switch (_deputyMode) {
		case 27:
			switch (_deputyShould) {
			case 27:
				movementCheck();
				kernel_timing_trigger(10, kCHANGE_DEPUTY_ANIMATION);
				break;

			case 29:
				_deputyShould = 30;
				series_play("137dp02", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, 8, 0, 100, 0, 0, 0, 7);
				break;

			case 30:
				terminateMachineAndNull(_door);
				_deputyMode = 31;
				series_play("137dp03", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, 8, 0, 100, 0, 0, 0, 14);
				digi_play("137_003", 1, 255);
				break;

			case 34:
				movementCheck();
				_deputyShould = 29;
				series_play("137dp01", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, 60, 0, 100, 0, 0, 6, 6);
				break;

			default:
				break;
			}
			break;

		case 31:
			if (imath_ranged_rand(1, 20) == 1) {
				_deputyMode = 32;
				series_play("137dp04", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
			} else {
				series_play("137dp04", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, 10, 0, 100, 0, 0, 7, 7);
			}
			break;

		case 32:
			if (imath_ranged_rand(1, 20) == 1) {
				_deputyMode = 33;
				series_play("137dp04", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, imath_ranged_rand(60, 90),
					0, 100, 0, 0, 0, 5);
			} else {
				series_play("137dp04", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 33:
			_deputyMode = 31;
			series_play("137dp04", 0x700, 0, kCHANGE_DEPUTY_ANIMATION, 10, 0, 100, 0, 0, 6, 7);
			break;

		default:
			break;
		}
		break;

	case 3:
		ws_walk(276, 292, 0, -1, 4);
		break;

	case kSOMEONE_TOOK_KEYS:
		digi_play("137_014", 2, 200, 5);
		break;

	case 5:
		unloadAssets();

		if (_G(flags)[kPoliceState] != 2 && _G(flags)[kPoliceState] != 3 && _G(flags)[kPoliceState] != 4)
			_deputyShould = 30;

		_sherrifMode = 5;
		_sherrifShould = 10;

		if (_G(player_info).y > 308) {
			_sherrifShould = 14;
			ws_walk(307, 349, 0, -1, 2);
		}

		kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
		break;

	case 6:
		digi_play("137_015", 2, 200, 7);
		break;

	case 7:
		_sherrifMode = 5;
		_sherrifShould = 14;
		unloadAssets();

		if (_G(player_info).y < 308)
			_sherrifShould = 10;
		if (_G(flags)[kPoliceState] != 2 && _G(flags)[kPoliceState] != 3 && _G(flags)[kPoliceState] != 4)
			_deputyShould = 30;

		kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
		break;

	case 8:
		player_set_commands_allowed(true);
		break;

	case 9:
		terminateMachineAndNull(_door);

		if (_G(flags)[V046]) {
			terminateMachineAndNull(_series3);

			if (inv_object_is_here("jawz o' life"))
				terminateMachineAndNull(_series2);
		}

		Section1::updateWalker(227, 309, 11, 13);
		break;

	case 11:
		if (_G(player_info).y >= 300) {
			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "137dt01", 6, 0xf00, -1);
		} else {
			kernel_trigger_dispatch_now(9);
		}
		break;

	case 12:
		_volume -= 10;
		if (_volume > 0) {
			digi_change_volume(2, _volume);
			kernel_timing_trigger(4, 12);
		} else {
			digi_stop(1);
			digi_unload_stream_breaks(SERIES1);
			digi_stop(2);
			digi_unload("100_013");
			player_set_commands_allowed(true);
			_volume = 255;
		}
		break;

	case 13:
		jawz();
		_door = series_play("137do01", 0x700, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_G(walker).reset_walker_sprites();
		digi_preload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES1, "137dt01", 0, 0xf00, -1);
		_G(flags)[V049] = 1;
		break;

	case 14:
		switch (_sherrifShould) {
		case 22:
			if (imath_ranged_rand(1, 30) == 2) {
				series_play("137sh04", 0x800, 0, 1, 6, 0, 100, 0, 0, 0, 14);
			} else {
				series_play("137sh01", 0x800, 0, 1, 12, 0, 100, 0, 0, 17, 17);
			}
			break;

		case 23:
			_sherrifMode = 16;
			series_play("137sh03", 0x800, 0, 1, 6, 0, 100, 0, 0, 0, 0);
			break;

		case 24:
			_sherrifShould = 23;
			series_play("137sh05", 0x800, 0, 1, 6, 0, 100, 0, 0, 0, 17);

			if (_digi1) {
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(_digi1, 1, 255, 14);
			}
			break;

		case 25:
			_sherrifShould = 26;
			series_play("137sh06", 0x800, 0, 1, 6, 0, 100, 0, 0, 0, 23);
			break;

		case 26:
			conv_resume_curr();
			_sherrifShould = 22;
			kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
			break;

		default:
			break;
		}
		break;

	case 15:
		if (_arrested) {
			// Arrested
			inv_move_object("keys", 138);
			inv_move_object("jawz o' life", 137);
			_G(flags)[V046] = 0;
			pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1006);

		} else {
			// Just sent off
			pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1009);
		}
		break;

	case 16:
		terminateMachineAndNull(_series3);
		inv_move_object("keys", NOWHERE);
		break;

	case 17:
		terminateMachineAndNull(_series3);
		break;

	case 18:
		_series3 = series_play("137tr02", 0x700, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		inv_give_to_player("keys");
		break;

	case 19:
		_series3 = series_play("137tr01", 0x700, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		break;

	case 20:
		terminateMachineAndNull(_series2);
		inv_give_to_player("jawz o' life");
		break;

	case 21:
		if (inv_object_is_here("jawz o' life"))
			terminateMachineAndNull(_series2);
		break;

	case 22:
		if (inv_object_is_here("jawz o' life"))
			_series2 = series_play("137jaws", 0x600, 0, -1, 7, -1, 100, 0, 0, 0, 0);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			_G(wilbur_should) = 2;
			series_play("137wi01", 0x100, 2, kCHANGE_WILBUR_ANIMATION,
				6, 0, 100, 0, 0, 0, 9);
			series_play("137wi01s", 0x101, 2, -1, 6, 0, 100, 0, 0, 0, 9);
			break;

		case 2:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;

		case 3:
			ws_demand_location(183, 216, 8);
			_G(wilbur_should) = 4;
			ws_walk(171, 236, 0, kCHANGE_WILBUR_ANIMATION, 5);
			break;

		case 4:
			player_set_commands_allowed(true);

			if (!_G(player).been_here_before)
				wilbur_speech("137w001");
			break;

		case 6:
			ws_hide_walker();
			_G(wilbur_should) = 9;
			player_set_commands_allowed(false);
			series_load("137jaws");
			series_play_with_breaks(PLAY1, "137wi02", 0x100, kCHANGE_WILBUR_ANIMATION,
				3, 6, 100, 0, 0);
			hotspot_set_active("keys", true);
			hotspot_set_active("trunk ", true);

			if (inv_object_is_here("jawz o' life"))
				hotspot_set_active("jawz o' life", true);

			_G(flags)[V046] = 1;
			break;

		case 7:
			ws_hide_walker();
			player_set_commands_allowed(false);
			_G(wilbur_should) = 9;
			series_play_with_breaks(PLAY2, "137wi03", 0x100, kCHANGE_WILBUR_ANIMATION,
				3, 6, 100, 0, 0);
			hotspot_set_active("keys", false);
			hotspot_set_active("trunk ", false);
			hotspot_set_active("jawz o' life", false);
			_G(flags)[V046] = 0;
			break;

		case 8:
			ws_hide_walker();
			player_set_commands_allowed(false);
			_G(wilbur_should) = 9;
			series_play_with_breaks(PLAY3, "137wi04", 0x100, kCHANGE_WILBUR_ANIMATION,
				3, 6, 100, 0, 0);
			hotspot_set_active("jawz o' life", false);
			break;

		case 9:
			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;

		case 35:
			_G(wilbur_should) = 36;
			ws_hide_walker();
			series_play("137wi01", 0x100, 0, kCHANGE_WILBUR_ANIMATION,
				6, 0, 100, 0, 0, 0, 9);
			series_play("137wi01s", 0x100, 0, -1, 6, 0, 100, 0, 0, 0, 9);
			break;

		case 36:
			series_play("137wi01", 0x100, 0, kCHANGE_WILBUR_ANIMATION,
				600, 0, 100, 0, 0, 9, 9);
			series_play("137wi01s", 0x100, 0, -1, 600, 0, 100, 0, 0, 9, 9);
			pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1011);
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
			kernel_timing_trigger(240, 11);

		} else {
			kernel_timing_trigger(60, kBurlEntersTown);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room137::pre_parser() {
	if (player_said("KEYS", "PATROL CAR")) {
		// Convenience added for ScummVM, using keys on the patrol car is
		// the same as using the keys on the trunk specifically
		player_hotspot_walk_override(500, 355, 10);
	}
}

void Room137::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[V046] && (player_said("gear", "trunk") || player_said("gear", "trunk "))) {
		_G(wilbur_should) = 7;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("conv15")) {
		conv15();

	} else if (_G(walker).wilbur_said(SAID)) {
		// Nothing needed
	} else if (player_said("LOOK AT", "TRUNK") || player_said("LOOK AT", "TRUNK ")) {
		if (!_G(flags)[V046]) {
			wilbur_speech("137w005");
		} else {
			wilbur_speech(inv_object_is_here("JAWZ O' LIFE") ? "137w006" : "137w007");
		}
	} else if (player_said("keys", "trunk") || player_said("keys", "patrol car")) {
		_G(wilbur_should) = 6;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("take", "jawz o' life") && inv_object_is_here("jawz o' life")) {
		_G(wilbur_should) = 8;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if ((player_said("take", "keys") || player_said("gear", "keys")) &&
			!inv_player_has("keys")) {
		_G(wilbur_should) = 7;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("gear", "jawz o' life") && inv_object_is_here("jawz o' life")) {
		wilbur_speech("137w011");

	} else if (player_said("ENTER", "FORK IN THE ROAD") ||
			player_said("look at", "FORK IN THE ROAD")) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1009);

	} else if (player_said("LOOK AT", "CAR WINDOW")) {
		_G(wilbur_should) = 35;
		player_set_commands_allowed(false);
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room137::conv15() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 14) {
		if (who <= 0) {
			if (node == 7) {
				_sherrifShould = 25;
			} else {
				_sherrifShould = 22;
				conv_resume_curr();
			}
		} else if (who == 1) {
			conv_resume_curr();
		}
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			if (node == 3 || node == 9 || node == 12 || node == 13 || node == 19 || node == 11)
				_arrested = true;
			_sherrifShould = (node == 20 && entry == 1) || (node == 21 && entry == 1) ? 24 : 23;
			_digi1 = conv_sound_to_play();

		} else if (who == 1) {
			wilbur_speech(conv_sound_to_play(), 14);
		}
	} else {
		conv_resume_curr();
	}
}

void Room137::unloadAssets() {
	const char *NAMES[13] = {
		"137_020", "137_021", "137_022", "137_023", "137_024",
		"137_025", "137_026", "137_027", "138_028", "138_013",
		"137_014", "137_015", "137_016"
	};

	for (int i = 0; i < 13; ++i)
		digi_unload(NAMES[i]);
}

void Room137::jawz() {
	if (_G(flags)[V046]) {
		_series3 = series_play("137tr01", 0x700, 0, -1, 600, -1, 100, 0, 0, 0, 0);

		if (inv_object_is_here("jawz o' life")) {
			_series2 = series_play("137jaws", 0x600, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		} else {
			hotspot_set_active("jawz o' life", false);
		}
	} else {
		_series3 = series_play("137tr02", 0x700, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		hotspot_set_active("keys", false);
		hotspot_set_active("trunk ", false);
		hotspot_set_active("jawz o' life", false);
	}
}

void Room137::movementCheck() {
	if (_G(flags)[kPoliceCheckCtr] && player_commands_allowed()) {
		++_G(flags)[kPoliceCheckCtr];
		player_update_info();

		if (_G(player_info).y < 300 && inv_player_has("keys")) {
			_G(flags)[kPoliceState] = 6;
			_G(flags)[kPoliceCheckCtr] = 201;
		}

		if (_G(player_info).y < 308 && _G(flags)[V046])
			_G(flags)[kPoliceCheckCtr] = 201;

		if (_G(flags)[kPoliceCheckCtr] >= 200) {
			player_set_commands_allowed(false);
			intr_freshen_sentence();
			_G(flags)[kPoliceCheckCtr] = 0;
			_flag1 = false;

			if (_G(flags)[kPoliceState] == 6) {
				kernel_timing_trigger(100, 3);
				digi_play("137_013", 2, 200, kSOMEONE_TOOK_KEYS);

			} else if (_G(player_info).x > 460) {
				ws_walk(307, 349, nullptr, -1, 2);

				if (_G(flags)[V046]) {
					digi_play("137_013", 2, 200, 6);
				} else {
					digi_play("137_013", 2, 200, 7);
				}
			} else if (_G(flags)[kPoliceState] == 1 || _G(flags)[kPoliceState] == 2 ||
					_G(flags)[kPoliceState] == 3 || _G(flags)[kPoliceState] == 4) {
				kernel_timing_trigger(30, 7);
			} else {
				digi_play("137_013", 2, 200, 7);
				player_update_info();

				if (_G(player_info).y < 308)
					kernel_timing_trigger(180, 3);
			}
		}
	}
}

void Room137::startPoliceTalk() {
	conv_load_and_prepare("conv15", 15);

	switch (_G(flags)[kPoliceState]) {
	case 1:
		conv_export_value_curr(0, 0);
		break;
	case 2:
	case 3:
		conv_export_value_curr(1, 0);
		break;
	case 4:
		conv_export_value_curr(4, 0);
		break;
	case 5:
		conv_export_value_curr(2, 0);
		break;
	case 6:
		conv_export_value_curr(3, 0);
		break;
	default:
		break;
	}

	conv_export_value_curr(_G(flags)[V049], 2);
	conv_export_value_curr(_G(flags)[kRoadOpened], 3);
	conv_play_curr();
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
