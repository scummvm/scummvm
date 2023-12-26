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

#include "m4/burger/rooms/section1/room138.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_DEPUTY_ANIMATION = 1,
	kCHANGE_SHERRIF_ANIMATION = 2,
	kCHANGE_WILBUR_ANIMATION = 3
};

static const char *SAID[][4] = {
	{ "DONUTS",          "138W002", "138W003", "138W003" },
	{ "IGNITION SWITCH", "138W006", "138W007", "138W006" },
	{ "HORN",            "138W008", "138W007", nullptr   },
	{ "RADIO",           "138W009", "138W007", nullptr   },
	{ "POLICE RADIO",    "138W010", "138W007", "138W004" },
	{ "RADAR",           "138W011", "138W007", "138W004" },
	{ "SHERIFF",         "138W012", "138W013", "138W004" },
	{ "DEPUTY",          "138W014", "138W013", "138W004" },
	{ "PATROL CAR",      "138W015", "138W013", "138W016" },
	{ "EXIT",            nullptr,   "138W017", "138W017" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0,  3, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 4,  4, 0, 1, 255,  8, 0, 0, 0, 0 },
	{ 5, -1, 0, 1, 255, 10, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{  0, 11, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, "138_003", 1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255,  5, 0, 0, 0, 0 },
	{ 12, 12, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	{ 11,  0, nullptr,   1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  6, 8, nullptr,   1, 255, 12,    0, 0, 0, 0 },
	{  7, 6, "138_006", 1, 255, -1,    0, 0, 0, 0 },
	{  7, 8, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  8, 6, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  6, 7, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  6, 7, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  7, 6, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  6, 8, nullptr,   1, 255, -1,    0, 0, 0, 0 },
	{  8, 6, nullptr,   1, 255, -1, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 25, 5, 0, 1, 255, -1, 0, 0, 0, 0 },
	{  4, 0, 0, 1, 255, 11, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

void Room138::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room138::init() {
	_assetsFreed = false;
	_series3 = -1;
	_flag1 = false;

	static const char *NAMES[12] = {
		"138_004", "137_012", "137_013", "137_020", "137_021",
		"137_022", "137_023", "137_024", "137_025", "137_026",
		"137_027", "137_028"
	};
	for (int i = 0; i < 12; ++i)
		digi_preload(NAMES[i]);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[kPoliceCheckCtr])
			_G(flags)[kPoliceCheckCtr] = 1;
		break;

	default:
		break;
	}

	if (inv_object_is_here("keys")) {
		digi_preload("138_001");
		digi_play_loop("138_001", 3, 255);

	} else {
		digi_preload("138_002");
		digi_play_loop("138_002", 3, 255);
	}

	if (inv_object_is_here("keys")) {
		hotspot_set_active("ignition switch", false);
		_series1 = series_play("138keys", 0xa00, 0, -1, 7, -1, 100, 0, 0, 0, 0);
	} else {
		hotspot_set_active("keys", false);
	}

	_frame = 22;
	_deputyMode = 18;
	_deputyShould = 18;
	kernel_trigger_dispatch_now(1);
	_val4 = -1;
	_sherrifMode = 26;
	_sherrifShould = 26;
	kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
	_wilburMode = 2;
	_wilburShould = 1;
	kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	_series2 = series_play("138donut", 0x400, 0, -1, 7, -1, 100, 0, 0, 0, 0);

	if (!inv_object_is_here("keys"))
		kernel_timing_trigger(10, 13);

	if (!player_been_here(138)) {
		player_set_commands_allowed(false);
		wilbur_speech("138w001");
	}

	digi_play("137_021", 2);
}

void Room138::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case kCHANGE_DEPUTY_ANIMATION:
		switch (_deputyMode) {
		case 18:
			switch (_deputyShould) {
			case 18:
			case 23:
			case 25:
				if (_deputyShould == 23 && _frame == 42) {
					freeAssets();
					_deputyMode = 23;
					series_play("138cp06", 0x500, 0, kCHANGE_DEPUTY_ANIMATION, 7, 0, 100, 0, 0, 30, 45);

				} else if (_deputyShould == 25 && _frame == 42) {
					_deputyShould = 18;
					series_play("138cp02", 0x500, 0, kCHANGE_DEPUTY_ANIMATION, 7, 0, 100, 0, 0, 31, 61);

				} else if (_frame == 42 && imath_ranged_rand(1, 5) != 1) {
					series_play("138cp01", 0x500, 0, kCHANGE_DEPUTY_ANIMATION, 60, 0, 100, 0, 0, 23, 23);

				} else {
					if (++_frame == 43)
						_frame = 23;

					series_play("138cp01", 0x500, 0, kCHANGE_DEPUTY_ANIMATION, 7, 0, 100, 0, 0, _frame, _frame);
				}
				break;

			case 22:
				freeAssets();
				_deputyMode = 22;
				series_play("138cp04", 0x500, 0, kCHANGE_DEPUTY_ANIMATION, 7, 0, 100, 0, 0, 10, 16);
				break;

			default:
				break;
			}
			break;

		case 22:
			if (imath_ranged_rand(1, 10) == 1) {
				series_play("138cp04", 0x500, 1, kCHANGE_DEPUTY_ANIMATION, 7, 0, 100, 0, 0, 16, 18);
			} else {
				series_play("138cp04", 0x500, 0, kCHANGE_DEPUTY_ANIMATION, 10, 0, 100, 0, 0, 16, 16);
			}
			break;

		case 23:
			_deputyMode = 24;
			series_play("138cp06", 0x500, 0, kCHANGE_DEPUTY_ANIMATION, 6, 0, 100, 0, 0, 46, 49);
			digi_play("138_004", 1, 255);
			break;

		case 24:
			series_play("138cp06", 0x500, 0, -1, 60, -1, 100, 0, 0, 49, 49);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_SHERRIF_ANIMATION:
		switch (_sherrifMode) {
		case 26:
			switch (_sherrifShould) {
			case 26:
			case 33:
			case 36:
				if (_sherrifShould == 33 && _val4 == 22) {
					freeAssets();
					term_message("xxx");
					_sherrifShould = 34;
					series_play("138cp06", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 18);
					digi_play("138_005", 2);

				} else if (_sherrifShould == 36 && _val4 == 22) {
					_sherrifShould = 26;
					series_play("138cp02", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 0, 30);
					randomDigi();

				} else if (_val4 == 22 && !inv_object_is_here("keys") &&
					imath_ranged_rand(1, 2) == 1) {
					term_message(".........................");
					_sherrifShould = 37;
					series_play("138cp02", 0x100, 2, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 25, 30);

				} else if (_val4 == 22 && imath_ranged_rand(1, 6) != 1) {
					series_play("138cp01", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 60, 0, 100, 0, 0, 0, 0);

				} else {
					_flag1 = false;

					switch (++_val4) {
					case 13:
						digi_play("137_022", 2);
						break;

					case 23:
						_val4 = 0;
						digi_play(imath_ranged_rand(1, 2) == 1 ? "137_020" : "137_021", 2);
						break;

					default:
						break;
					}

					series_play("138cp01", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 7, 0, 100, 0, 0, _val4, _val4);
				}
				break;

			case 31:
				freeAssets();
				_sherrifShould = 32;
				series_play("138cp04", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 7, 0, 100, 0, 0, 0, 7);
				break;

			case 32:
				kernel_timing_trigger(120, 7);

				if (!inv_object_is_here("keys"))
					_G(flags)[kPoliceState] = 6;

				_G(flags)[kPoliceCheckCtr] = 200;
				_sherrifMode = 31;
				kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
				break;

			case 34:
				terminateMachineAndNull(_series2);
				_sherrifShould = 35;
				series_play("138cp06", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 19, 29);
				break;

			case 35:
				kernel_timing_trigger(120, 7);

				if (!inv_object_is_here("keys"))
					_G(flags)[kPoliceState] = 6;

				_G(flags)[kPoliceCheckCtr] = 200;
				_sherrifMode = 33;
				kernel_trigger_dispatch_now(kCHANGE_SHERRIF_ANIMATION);
				break;

			case 37:
				_sherrifShould = 38;
				series_play("138cp02", 0x100, 2, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 10, 24);
				randomDigi();
				break;

			case 38:
				_sherrifShould = 26;
				series_play("138cp02", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 8, 0, 100, 0, 0, 26, 30);
				_flag1 = true;
				break;

			default:
				break;
			}
			break;

		case 31:
			if (imath_ranged_rand(1, 10) == 1) {
				series_play("138cp04", 0x100, 1, kCHANGE_SHERRIF_ANIMATION, 7, 0, 100, 0, 0, 7, 9);
			} else {
				series_play("138cp04", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 10, 0, 100, 0, 0, 7, 7);
			}
			break;

		case 33:
			if (imath_ranged_rand(1, 10) == 1) {
				series_play("138cp06", 0x100, 1, kCHANGE_SHERRIF_ANIMATION, 7, 0, 100, 0, 0, 27, 29);
			} else {
				series_play("138cp06", 0x100, 0, kCHANGE_SHERRIF_ANIMATION, 10, 0, 100, 0, 0, 27, 27);
			}
			break;

		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_wilburMode) {
		case 2:
			switch (_wilburShould) {
			case 1:
				switch (imath_ranged_rand(1, 20)) {
				case 1:
					_wilburMode = 3;
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 3);
					break;

				case 2:
					_wilburMode = 6;
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 17, 17);
					break;

				default:
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 30, 0, 100, 0, 0, 0, 0);
					break;
				}
				break;

			case 7:
				series_load("138wi03");
				digi_preload("138_002");
				_wilburShould = 8;
				series_play("138wi05", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 0, 11);
				break;

			case 8:
				_wilburShould = 9;
				series_play_with_breaks(PLAY1, "138wi03", 0xa00, kCHANGE_WILBUR_ANIMATION, 2, 6, 100, 0, 0);
				break;

			case 9:
				player_set_commands_allowed(true);
				_wilburShould = 1;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				_deputyShould = 25;
				_sherrifShould = 36;
				break;

			case 10:
				_wilburShould = 1;
				series_play_with_breaks(PLAY2, "138wi05", 0xa00, kCHANGE_WILBUR_ANIMATION, 2, 6, 100, 0, 0);
				break;

			case 11:
				series_load("138wi06");
				digi_preload("138_002");
				_wilburShould = 12;
				series_play("138wi05", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 11);
				break;

			case 12:
				_wilburShould = 13;
				series_play_with_breaks(PLAY3, "138wi06", 0xa00, kCHANGE_WILBUR_ANIMATION, 2, 6, 100, 0, 0);
				break;

			case 13:
				_wilburShould = 14;
				series_play("138wi05", 0xa00, 2, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 11);
				break;

			case 14:
				_wilburShould = 1;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

				if (inv_object_is_here("keys")) {
					_sherrifShould = 33;
					_deputyShould = 23;
				} else {
					player_set_commands_allowed(true);
				}
				break;

			case 15:
				_wilburMode = 15;
				_G(walker).wilbur_say();
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
				break;

			case 16:
				series_load("138wi03");
				digi_preload("138_001");
				_wilburShould = 17;
				series_play_with_breaks(PLAY4, "138wi03", 0xa00, kCHANGE_WILBUR_ANIMATION, 2, 6, 100, 0, 0);
				break;

			case 17:
				_wilburShould = 9;
				series_play("138wi05", 0xa00, 2, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 0, 11);
				break;

			default:
				break;
			}
			break;

		case 3:
			if (_wilburShould == 1) {
				switch (imath_ranged_rand(1, 20)) {
				case 1:
					_wilburMode = 2;
					series_play("138wi01", 0xa00, 2, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 3);
					break;

				case 2:
					_wilburMode = 4;
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 4, 9);
					break;

				default:
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 30, 0, 100, 0, 0, 3, 3);
					break;
				}
			} else {
				_wilburMode = 4;
				series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 4, 9);
			}
			break;

		case 4:
			if (_wilburShould == 1) {
				switch (imath_ranged_rand(1, 20)) {
				case 1:
					_wilburMode = 3;
					series_play("138wi01", 0xa00, 2, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 4, 9);
					break;

				case 2:
					_wilburMode = 5;
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 10, 12);
					break;

				default:
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 30, 0, 100, 0, 0, 9, 9);
					break;
				}
			} else {
				_wilburMode = 5;
				series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 10, 12);
			}
			break;

		case 5:
			if (_wilburShould == 1) {
				switch (imath_ranged_rand(1, 20)) {
				case 1:
					_wilburMode = 4;
					series_play("138wi01", 0xa00, 2, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 10, 12);
					break;

				case 2:
					_wilburMode = 6;
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 13, 17);
					break;

				default:
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 30, 0, 100, 0, 0, 12, 12);
					break;
				}
			} else {
				_wilburMode = 6;
				series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 13, 17);
			}
			break;

		case 6:
			if (_wilburShould == 1) {
				switch (imath_ranged_rand(1, 20)) {
				case 1:
					_wilburMode = 5;
					series_play("138wi01", 0xa00, 2, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 13, 17);
					break;

				case 2:
					_wilburMode = 2;
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 0);
					break;

				default:
					series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 30, 0, 100, 0, 0, 17, 17);
					break;
				}
			} else {
				_wilburMode = 2;
				series_play("138wi01", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 0);
			}
			break;

		case 15:
			if (_wilburShould == 15) {
				loadSeries();
				frame = imath_ranged_rand(0, 4);
				series_play("138wi02", 0xa00, 0, kCHANGE_WILBUR_ANIMATION, 4, 0, 100, 0, 0, frame, frame);

			} else {
				freeSeries();
				_wilburShould = 1;
				_wilburMode = 2;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			}
			break;

		default:
			break;
		}
		break;

	case 4:
		player_set_commands_allowed(true);
		break;

	case 5:
		_sherrifShould = 31;
		_deputyShould = 22;
		break;

	case 6:
		// Waking them up
		_sherrifShould = 33;
		_deputyShould = 23;
		player_set_commands_allowed(false);
		break;

	case 7:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1010);
		break;

	case 8:
		terminateMachineAndNull(_series1);
		break;

	case 10:
		digi_play_loop("138_002", 3, 255);
		_G(flags)[kPoliceState] = 4;
		inv_give_to_player("keys");
		hotspot_set_active("keys", false);
		hotspot_set_active("ignition switch", true);
		_G(flags)[kPoliceCheckCtr]++;
		kernel_timing_trigger(10, 13);
		break;

	case 11:
		digi_preload("138_001");
		digi_play_loop("138_001", 3, 255);
		_series1 = series_play("138keys", 0xa00, 0, -1, 7, -1, 100, 0, 0, 0, 0);
		_G(flags)[kPoliceState] = 0;
		_G(flags)[kPoliceCheckCtr] = 0;
		inv_move_object("keys", 138);
		hotspot_set_active("keys", true);
		hotspot_set_active("ignition switch", false);
		break;

	case 12:
		digi_play_loop("138_002", 3, 255);
		break;

	case 13:
		if (_G(flags)[kPoliceCheckCtr] && player_commands_allowed()) {
			if (++_G(flags)[kPoliceCheckCtr] >= 200) {
				player_set_commands_allowed(false);
				_G(flags)[kPoliceState] = 4;
				_sherrifShould = 33;
				_deputyShould = 23;
			}
		}

		kernel_timing_trigger(10, 13);
		break;

	case kWILBURS_SPEECH_START:
		_wilburShould = 15;
		player_set_commands_allowed(false);
		break;

	case kWILBURS_SPEECH_FINISHED:
		_wilburShould = 1;
		player_set_commands_allowed(true);
		_G(kernel).continue_handling_trigger = true;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room138::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Nothing needed
	} else if (player_said("keys", "ignition switch")) {
		_wilburShould = 16;
		player_set_commands_allowed(false);

	} else if ((player_said("donuts") || player_said("keys") ||
			player_said("ignition switch") || player_said("horn") ||
			player_said("radio") || player_said("police radio") ||
			player_said("radar") || player_said("deputy") ||
			player_said("patrol car")) && inv_player_has(_G(player).verb)) {
		wilbur_speech("138w004");

	} else if (player_said("gear", "horn")) {
		player_set_commands_allowed(false);
		_G(flags)[kPoliceState] = 1;
		_wilburShould = 10;

	} else if (player_said("gear", "radio")) {
		player_set_commands_allowed(false);

		if (inv_object_is_here("keys"))
			_G(flags)[kPoliceState] = 2;

		_wilburShould = 11;

	} else if (player_said("talk to") &&
			(player_said("sheriff") || player_said("deputy"))) {
		player_set_commands_allowed(false);
		_G(flags)[kPoliceState] = 3;
		wilbur_speech("138w610", 6, 138);
		_wilburShould = 15;

	} else if (player_said("take", "keys") && !inv_player_has("keys")) {
		_wilburShould = 7;
		player_set_commands_allowed(false);

	} else if (player_said("look at", "keys") && inv_object_is_here("keys")) {
		wilbur_speech("138w005");

	} else if (player_said("gear", "keys")) {
		wilbur_speech("138w004");

	} else if (player_said("keys") && inv_player_has(_G(player).verb)) {
		wilbur_speech("138w004");

	} else if (player_said("exit") || player_said("look at", "exit")) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1010);

	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room138::freeAssets() {
	if (!_assetsFreed) {
		_assetsFreed = true;
		digi_stop(2);

		static const char *NAMES[11] = {
			"137_020", "137_021", "137_022", "137_023", "137_024",
			"137_025", "137_026", "137_027", "137_028", "137_012",
			"137_013"
		};
		for (int i = 0; i < 11; ++i)
			digi_unload(NAMES[i]);

		digi_unload(inv_object_is_here("keys") ? "138_002" : "138_001");
	}
}

void Room138::randomDigi() {
	digi_play(Common::String::format("137_0%d", imath_ranged_rand(23, 27)).c_str(), 2);
}

void Room138::loadSeries() {
	if (_series3 == -1)
		_series3 = series_load("138wi02");
}

void Room138::freeSeries() {
	if (_series3 != -1) {
		series_unload(_series3);
		_series3 = -1;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
