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

enum {
	kCHANGE_VIPE_ANIMATION = 15,
	kCHANGE_MUFFINS_ANIMATION = 16,
	kCHANGE_VERA_ANIMATION = 17
};

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
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(false);
		ws_demand_location(199, 253, 4);
		player_set_commands_allowed(true);
		break;

	case 406:
		player_set_commands_allowed(false);
		ws_demand_location(413, 356, 10);
		poofArrive();
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(241, 266, 4);
		poofArrive();
		break;
	}

	_series1 = series_play("405plate", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	if (_G(flags)[V166]) {
		hotspot_set_active("vipe", false);
		hotspot_set_active("guitar", false);
	} else {
		_vipeShould = 39;
		_vipeMode = 39;
		kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
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
		_veraShould = 4;
		_veraMode = 4;
		kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
	}

	_muffinsState = 3;
	kernel_trigger_dispatch_now(kCHANGE_MUFFINS_ANIMATION);
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
	int frame;

	switch (_G(kernel).trigger) {
	case 1:
	case 4:
		player_set_commands_allowed(true);
		_vipeShould = 39;
		break;

	case 2:
		player_set_commands_allowed(false);
		_vipeShould = 49;
		break;

	case 3:
		player_set_commands_allowed(false);
		_G(wilbur_should) = 30;
		break;

	case 5:
		player_set_commands_allowed(true);
		_veraShould = 4;
		break;

	case 7:
		_volume -= 10;
		if (_volume >= 6) {
			digi_change_volume(3, _volume);
			kernel_timing_trigger(3, 7);
		} else {
			digi_stop(3);
		}
		break;

	case 8:
		ws_unhide_walker();
		_G(walker).wilbur_poof();
		player_set_commands_allowed(true);
		break;

	case 9:
		digi_play("92n0603", 1, 255);
		digi_play("405_009", 2, 255, 12);
		break;

	case 11:
		terminateMachineAndNull(_series1);
		_series1 = nullptr;
		break;

	case 12:
		conv_resume_curr();
		break;

	case 13:
		digi_play("405v513", 1, 255);
		break;

	case kCHANGE_VIPE_ANIMATION:
		switch (_vipeMode) {
		case 39:
			switch (_vipeShould) {
			case 39:
				if (imath_ranged_rand(1, 20) == 1) {
					_vipeMode = 42;
					Series::series_play("405vp03", 0x900, 0, kCHANGE_VIPE_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				} else {
					Series::series_play("405vp01", 0x900, 0, kCHANGE_VIPE_ANIMATION, 60, 0, 100, 0, 0, 0, 0);
				}
				break;

			case 40:
				frame = imath_ranged_rand(0, 5);
				Series::series_play("405vp02", 0x900, 0, kCHANGE_VIPE_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigi2();
				break;

			default:
				_vipeMode = 42;
				Series::series_play("405vp03", 0x900, 0, kCHANGE_VIPE_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 41:
			if (_vipeShould == 39) {
				if (imath_ranged_rand(1, 10) == 1) {
					Series::series_play("405vp07", 0x900, 0, kCHANGE_VIPE_ANIMATION, 10, 0, 100, 0, 0, 5, 6);
				} else {
					Series::series_play("405vp07", 0x900, 2, kCHANGE_VIPE_ANIMATION, 60, 0, 100, 0, 0, 4, 4);
				}
			} else {
				_vipeMode = 42;
				Series::series_play("405vp07", 0x900, 0, kCHANGE_VIPE_ANIMATION, 10, 0, 100, 0, 0, 5, 6);				
			}
			break;

		case 42:
			switch (_vipeShould) {
			case 39:
				switch (imath_ranged_rand(1, 20)) {
				case 1:
					_vipeMode = 39;
					Series::series_play("405vp03", 0x900, 2, kCHANGE_VIPE_ANIMATION, 10, 0, 100, 0, 0, 0, 4);
					break;
				case 2:
					_vipeMode = 41;
					Series::series_play("405vp07", 0x900, 0, kCHANGE_VIPE_ANIMATION, 10, 0, 100, 0, 0, 0, 4);
					break;

				default:
					Series::series_play("405vp03", 0x900, 0, kCHANGE_VIPE_ANIMATION, 60, 0, 100, 0, 0, 4, 4);
					break;
				}
				break;

			case 43:
				Series::series_play("405vp03", 0x900, 0, kCHANGE_VIPE_ANIMATION, 60, 0, 100, 0, 0, 4, 4);
				break;

			case 44:
				frame = imath_ranged_rand(0, 5);
				Series::series_play("405vp05", 0x900, 0, kCHANGE_VIPE_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigi2();
				break;

			case 45:
				frame = imath_ranged_rand(0, 6);
				Series::series_play("405vp06", 0x900, 0, kCHANGE_VIPE_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 49:
				digi_preload("999_003");
				_vpoof = series_load("405vpoof");
				_vipeShould = 50;
				Series::series_play("405vp13", 0x900, 0, kCHANGE_VIPE_ANIMATION, 7, 0, 100, 0, 0, 0, 16);
				break;

			case 50:
				_vipeShould = 51;
				Series::series_play("405vp13", 0x900, 0, -1, 7, 0, 100, 0, 0, 17, 18);

				_G(flags)[V166] = 1;
				digi_play("999_003", 1, 255);
				series_play("405vpoof", 0x500, 0, kCHANGE_VIPE_ANIMATION, 7, 0, 70, 40, 40, 0, -1);
				_muffinsState = 2;
				kernel_trigger_dispatch_now(kCHANGE_MUFFINS_ANIMATION);
				break;

			case 51:
				series_unload(_vpoof);
				player_set_commands_allowed(true);
				break;

			default:
				kernel_trigger_dispatch_now(25);
				break;
			}
			break;

		case 46:
			switch (_vipeShould) {
			case 47:
				Series::series_play("405vp10", 0x900, 0, kCHANGE_VIPE_ANIMATION, 60, 0, 100, 0, 0, 5, 5);
				break;

			case 48:
				frame = imath_ranged_rand(0, 6);
				Series::series_play("405vp11", 0x900, 0, kCHANGE_VIPE_ANIMATION, 5, 0, 100, 0, 0, frame, frame);
				playDigi2();
				break;

			case 49:
				_vipeMode = 42;
				kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
				break;

			}
			break;

		case 47:
			switch (_vipeShould) {
			case 47:
				Series::series_play("405vp10", 0x900, 0, kCHANGE_VIPE_ANIMATION,
					60, 0, 100, 0, 0, 5, 5);
				break;

			case 48:
				frame = imath_ranged_rand(0, 6);
				Series::series_play("405vp11", 0x900, 0, kCHANGE_VIPE_ANIMATION,
					5, 0, 100, 0, 0, frame, frame);
				playDigi2();
				break;

			case 49:
				_vipeMode = 42;
				kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
				break;

			default:
				break;
			}
		default:
			break;
		}
		break;

	case kCHANGE_MUFFINS_ANIMATION:
		switch (_muffinsState) {
		case 1:
			_cat.terminate();
			_muffinsState = 3;
			series_play_with_breaks(PLAY3, "405cat", 0, kCHANGE_MUFFINS_ANIMATION, 3, 9, 100, 0, 0);
			break;

		case 2:
			_cat.terminate();
			_muffinsState = 3;
			series_play_with_breaks(PLAY4, "405cat", 0, kCHANGE_MUFFINS_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 3:
			_cat.show("405cat", 0);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_VERA_ANIMATION:
		switch (_veraMode) {
		case 4:
			switch (_veraShould) {
			case 4:
				switch (imath_ranged_rand(1, 15)) {
				case 1:
					series_play_with_breaks(PLAY1, "405ve02", 0xf00, kCHANGE_VERA_ANIMATION, 2);
					break;
				case 2:
					series_play("405ve03", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
					break;
				default:
					series_play("405ve01", 0xf00, 0, kCHANGE_VERA_ANIMATION, 60, 0);
					break;
				}
				break;

			case 7:
				_veraMode = 6;
				series_play("405ve03", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				break;

			case 9:
			case 10:
			case 11:
				_veraMode = 9;
				series_play("405ve14", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 8);
				break;

			case 17:
				_veraMode = 16;
				series_play("405ve08", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			case 21:
				_veraMode = 22;
				series_play("405ve04", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 5);
				break;

			default:
				break;
			}
			break;

		case 5:
			switch (_veraShould) {
			case 4:
				if (imath_ranged_rand(1, 10) == 1) {
					_veraMode = 16;
					Series::series_play("405ve10", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				} else {
					Series::series_play("405ve10", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 3, 3);
				}
				break;

			case 24:
				break;

			default:
				_veraMode = 16;
				Series::series_play("405ve10", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				break;
			}
			break;

		case 6:
			switch (_veraShould) {
			case 4:
				if (imath_ranged_rand(1, 10) == 1) {
					series_play("405ve03", 0xf00, 2, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				} else {
					series_play("405ve03", 0xf00, 0, kCHANGE_VERA_ANIMATION, 60, 0, 100, 0, 0, 3, 3);
				}
				break;

			case 7:
				frame = imath_ranged_rand(3, 6);
				series_play("405ve03", 0xf00, 2, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigi();
				break;

			case 24:
				break;

			default:
				_veraMode = 4;
				series_play("405ve03", 0xf00, 2, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				break;
			}
			break;

		case 9:
			switch (_veraShould) {
			case 9:
				Series::series_play("405ve15", 0xf00, 0, kCHANGE_VERA_ANIMATION, 60, 0, 100, 0, 0, 4, 4);
				break;

			case 10:
				_veraMode = 10;
				Series::series_play("405ve17", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 2);
				break;

			case 11:
				_veraMode = imath_ranged_rand(1, 2) == 1 ? 12 : 13;
				Series::series_play("405ve18", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;

			case 14:
				_veraShould = 15;
				digi_preload("405_009");
				series_play_with_breaks(PLAY2, "405ve16", 0x100, kCHANGE_VERA_ANIMATION, 3);
				break;

			case 15:
				_veraShould = 9;
				kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
				break;

			case 16:
				_veraMode = 16;
				Series::series_play("405ve15", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			case 24:
				break;

			default:
				_veraMode = 4;
				Series::series_play("405ve08", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 10:
			switch (_veraShould) {
			case 10:
				frame = imath_ranged_rand(3, 5);
				Series::series_play("405ve17", 0xf00, 0, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigi();
				break;

			case 24:
				break;

			default:
				_veraMode = 9;
				Series::series_play("405ve17", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 2);
				break;
			}
			break;

		case 12:
			switch (_veraShould) {
			case 11:
				frame = imath_ranged_rand(1, 4);
				Series::series_play("405ve18", 0xf00, 0, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				break;

			case 24:
				break;

			default:
				_veraMode = 9;
				Series::series_play("405ve18", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;
			}
			break;

		case 13:
			switch (_veraShould) {
			case 11:
				frame = imath_ranged_rand(5, 8);
				Series::series_play("405ve18", 0xf00, 0, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigi();
				break;

			case 24:
				break;

			default:
				_veraMode = 9;
				Series::series_play("405ve18", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 0);
				break;
			}
			break;

		case 16:
			switch (_veraShould) {
			case 11:
				_veraMode = 9;
				Series::series_play("405ve15", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;

			case 16:
				if (imath_ranged_rand(1, 10) == 1) {
					_veraMode = 5;
					Series::series_play("405ve10", 0xf00, 0, kCHANGE_VERA_ANIMATION, 10, 0, 100, 0, 0, 0, 3);
				} else {
					series_play("405ve08", 0xf00, 0, kCHANGE_VERA_ANIMATION, 60, 0, 100, 0, 0, 4, 4);
				}
				break;

			case 17:
				if (imath_ranged_rand(1, 2) == 1) {
					_veraMode = 18;
					kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
				} else {
					_veraMode = 19;
					Series::series_play("405ve11", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 2, 4);
				}
				break;

			case 24:
				break;

			default:
				_veraMode = 4;
				series_play("405ve08", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 18:
			switch (_veraShould) {
			case 17:
				frame = imath_ranged_rand(0, 1);
				Series::series_play("405ve11", 0xf00, 0, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigi();
				break;

			case 24:
				break;

			default:
				_veraMode = 16;
				kernel_trigger_dispatch_now(kCHANGE_VERA_ANIMATION);
				break;
			}
			break;

		case 19:
			switch (_veraShould) {
			case 17:
				frame = imath_ranged_rand(4, 6);
				Series::series_play("405ve11", 0xf00, 0, kCHANGE_VERA_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigi();
				break;

			case 24:
				break;

			default:
				_veraMode = 16;
				Series::series_play("405ve11", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 2, 4);
				break;
			}
			break;

		case 20:
			switch (_veraShould) {
			case 20:
				Series::series_play("405ve06", 0xf00, 0, kCHANGE_VERA_ANIMATION, 60, 0, 100, 0, 0, 0, 0);
				break;

			case 21:
				frame = imath_ranged_rand(0, 4);
				Series::series_play("405ve06", 0xf00, 0, kCHANGE_VERA_ANIMATION, 5, 0, 100, 0, 0, frame, frame);
				playDigi();
				break;

			case 24:
				break;

			default:
				_veraMode = 23;
				Series::series_play("405ve05", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
				break;
			}
			break;

		case 22:
			_veraMode = 20;
			Series::series_play("405ve05", 0xf00, 0, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 4);
			break;

		case 23:
			_veraMode = 4;
			series_play("405ve04", 0xf00, 2, kCHANGE_VERA_ANIMATION, 6, 0, 100, 0, 0, 0, 5);
			break;

		default:
			break;
		}
		break;

	case 19:
		_muffinsState = 1;
		kernel_trigger_dispatch_now(kCHANGE_MUFFINS_ANIMATION);
		break;

	case 20:
		player_set_commands_allowed(true);
		break;

	case 21:
		ws_unhide_walker();
		break;

	case 22:
		pal_fade_init(15, 255, 100, 0, -1);
		break;

	case 23:
		player_update_info();
		ws_demand_location(_G(player_info).x - 5, _G(player_info).y);
		player_set_commands_allowed(false);
		ws_hide_walker();
		terminateMachineAndNull(_records);

		Series::series_play("405wi02", 0, 0, 24, 6, 0, 100);
		break;

	case 24:
		conv_resume_curr();
		ws_unhide_walker();
		inv_give_to_player("records");
		hotspot_set_active("records", false);
		player_set_commands_allowed(true);
		break;

	case 25:
		_vp03.terminate();
		_vp03.play("405vp03", 0x900, 0, -1, 600, -1, 100, 0, 0, 4, 4);
		Series::series_play("405wi01", 0x100, 0, 26, 7, 0, 100, 0, 0, 16, 18);
		break;

	case 26:
		_vp03.terminate();
		_vp03.play("405wi01", 0x100, 0, -1, 600, -1, 100, 0, 0, 18, 18);
		Series::series_play("405vp09", 0x900, 0, 27, 7, 0, 100, 0, 0, 0, 3);
		break;

	case 27:
		_vp03.terminate();
		inv_move_object("dog collar", 405);

		_G(flags)[V171] = 4003;
		Series::series_play("405wi01", 0x100, 0, 21, 7, 0, 100, 0, 0, 19, 24);
		Series::series_play("405vp09", 0x900, 0, 28, 7, 0, 100, 0, 0, 4, 4);
		break;

	case 28:
		_vipeMode = 47;
		Series::series_play("405vp10", 0x900, 0, kCHANGE_VIPE_ANIMATION, 7, 0, 100, 0, 0, 0, 5);
		break;

	case 29:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 30);
		break;

	case 30:
		digi_play("405w503", 1, 255, 31);
		break;

	case 31:
		_G(flags)[V168] = 1;
		hotspot_set_active("order window", false);
		if (!Section4::checkOrderWindow())
			_G(game).room_id = 1;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 25:
			disable_player();
			_G(wilbur_should) = 26;
			Series::series_play("405wi01", 0x100, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 15);
			break;

		case 26:
			_vp03.play("405wi01", 0x100, 0, -1, 600, -1, 100, 0, 0, 15, 15);
			startConv89();
			break;

		case 27:
			disable_player();
			_G(wilbur_should) = 28;
			Series::series_play("405wi01", 0x100, 0, kCHANGE_WILBUR_ANIMATION, 7, 0, 100, 0, 0, 0, 11);
			break;

		case 28:
			_G(wilbur_should) = 29;
			Series::series_play("405wi01", 0x100, 0, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 11, 11);
			startConv90();
			break;
			
		case 29:
			Series::series_play("405wi01", 0x100, 0, kCHANGE_WILBUR_ANIMATION, 60, 0, 100, 0, 0, 11, 11);
			break;

		case 30:
			_G(wilbur_should) = 31;
			Series::series_play("405wi01", 0x100, 2, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 0, 11);
			break;

		case 31:
			enable_player();
			break;

		case 32:
			terminateMachineAndNull(_lid);
			disable_player();
			_G(wilbur_should) = 33;
			series_play_with_breaks(PLAY5, "405wi07", 0x100, kCHANGE_WILBUR_ANIMATION, 3, 7, 100, 0, 0);
			break;

		case 33:
			inv_move_object("records", NOWHERE);
			_G(flags)[V167] = 1;

			_lid = series_play("405lid", 0xf00, 2, -1, 600, -1, 100, 0, 0, 0, 0);
			_box = series_play("405box1", 0xf00, 2, -1, 600, -1, 100, 0, 0, 0, 0);
			hotspot_set_active("box", true);
			enable_player();
			break;

		case 34:
			disable_player();
			_G(wilbur_should) = 35;
			series_play_with_breaks(PLAY6, "405wi04", 0xe00, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 35:
			inv_move_object("quarter", 405);
			_G(wilbur_should) = 36;
			series_play_with_breaks(PLAY7, "405wi05", 0xe00, kCHANGE_WILBUR_ANIMATION, 3, 6, 100, 0, 0);
			break;

		case 36:
			digi_preload("405_001");
			digi_play("405_001", 2, 255);
			ws_unhide_walker();
			player_update_info();

			_G(wilbur_should) = 37;
			ws_walk(_G(player_info).x, _G(player_info).y, nullptr, kCHANGE_WILBUR_ANIMATION, 5);
			break;

		case 37:
			_G(wilbur_should) = 38;
			gr_backup_palette();
			pal_fade_init(_G(kernel).first_fade, 255, 0, 15, kCHANGE_WILBUR_ANIMATION);
			break;

		case 38:
			player_update_info();
			_G(walker).unloadSprites();
			_cat.terminate();
			terminateMachineAndNull(_lid);
			terminateMachineAndNull(_box);

			if (_series1)
				terminateMachineAndNull(_series1);

			_veraShould = 24;

			digi_preload("405_007");
			digi_preload("405w503");
			digi_preload("405v513");

			digi_stop(3);
			digi_unload("405_010");
			digi_play_loop("405_007", 3, 255);

			digi_preload_stream_breaks(SERIES1);
			series_stream_with_breaks(SERIES1, "405disco", 7, 0, 6);
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
		poof(4006);
	} else if (player_said("poof ") || player_said("gear", "disc ")) {
		poof(4005);
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
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		hotspot_set_active("vipe", false);
		hotspot_set_active("guitar", false);
	} else if (player_said("swinging door") && player_said_any("enter", "gear") && !_G(flags)[V168]) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_veraShould = 7;
			_digiName = "405V504";
			_newMode = KT_PARSE;
			_newTrigger = 1;
			break;

		case 1:
			_veraShould = 4;
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
			_veraShould = 21;
			_digiName = "405V502";
			_newMode = KT_PARSE;
			_newTrigger = 1;
			_muffinsState = 1;
			kernel_trigger_dispatch_now(kCHANGE_MUFFINS_ANIMATION);
			break;

		case 1:
			term_message("spat 1");
			_G(kernel).trigger_mode = KT_PARSE;
			series_play("405eu02", 0xf00, 0, 4, 4, 0, 100, 0, 0, 10, 14);
			_veraShould = 9;
			digi_play("405e501", 1, 255, 2);
			break;

		case 2:
			terminateMachineAndNull(_eu02);
			series_play("405eu02", 0xf00, 0, -1, 4, 0, 100, 0, 0, 16, 18);
			_veraShould = 10;
			_digiName = "405V503";
			_newMode = KT_PARSE;
			_newTrigger = 3;
			break;

		case 3:
			player_set_commands_allowed(true);
			_veraShould = 4;
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
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("records", "jukebox")) {
		_G(wilbur_should) = 32;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (!_G(flags)[V168] && (player_said("quarter", "jukebox") || player_said("gear", "jukebox"))) {
		if (!inv_player_has("quarter")) {
			wilbur_speech("405w008");
		}  else if (_G(flags)[V167]) {
			_G(wilbur_should) = 34;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				_veraShould = 17;
				player_set_commands_allowed(false);
				_digiName = "405v510";
				_newMode = KT_PARSE;
				_newTrigger = 1;
				break;

			case 1:
				_veraShould = 4;
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
			_veraShould = 7;
			_digiName = "405v511";
			_newMode = KT_PARSE;
			_newTrigger = 1;
			break;

		case 1:
			player_set_commands_allowed(true);
			_veraShould = 4;
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
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 18) {
		if (who <= 0) {
			if (_vipeMode == 39)
				_vipeShould = 39;
			else if (_vipeMode == 47)
				_vipeShould = 47;
			else
				_vipeShould = 43;

			conv_resume();
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);

			conv_resume();
		}
	} else if (sound) {
		if (who <= 0) {
			if (_vipeMode == 39)
				_vipeShould = 40;
			else if (_vipeMode == 47)
				_vipeShould = 48;
			else
				_vipeShould = 44;

			_digiName = sound;
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);

			digi_play(sound, 1, 255, 18);
		}

		_newMode = KT_PARSE;
		_newTrigger = 18;
	} else {
		conv_resume();
	}
}

void Room405::conv89() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 18) {
		if (who <= 0) {
			if (_vipeMode == 39)
				_vipeShould = 39;
			else if (_vipeMode == 47)
				_vipeShould = 47;
			else
				_vipeShould = 43;

			conv_resume();
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);

			conv_resume();
		}
	} else if (sound) {
		if (who <= 0) {
			if ((node == 1 && entry == 0) || (node == 2 && entry == 0)) {
				_vipeShould = 44;
			} else if ((node == 1 && entry == 1) || (node == 2 && entry == 1)) {
				_vipeShould = 48;
			} else if (_vipeMode == 39) {
				_vipeShould = 40;
			} else if (_vipeMode == 47) {
				_vipeShould = 48;
			} else {
				_vipeShould = 44;
			}

			_digiName = sound;
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);

			digi_play(sound, 1, 255, 18);
		}

		_newMode = KT_PARSE;
		_newTrigger = 18;
	} else {
		conv_resume();
	}
}

void Room405::conv90() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 18) {
		if (who <= 0) {
			_vipeShould = 39;
			conv_resume();
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);

			conv_resume();
		}
	} else if (sound) {
		if (who <= 0) {
			_vipeShould = 44;
			_digiName = sound;
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);

			digi_play(sound, 1, 255, 18);
		}

		_newMode = KT_PARSE;
		_newTrigger = 18;
	} else {
		conv_resume();
	}
}

void Room405::conv91() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 18) {
		if (who <= 0) {
			_vipeShould = 39;
			conv_resume();
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);

			conv_resume();
		}
	} else if (sound) {
		if (who <= 0) {
			_vipeShould = 44;
			_digiName = sound;
		} else if (who == 1) {
			if (_G(player).walker_visible)
				sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);

			digi_play(sound, 1, 255, 18);
		}

		_newMode = KT_PARSE;
		_newTrigger = 18;
	} else {
		conv_resume();
	}
}

void Room405::conv92() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 18) {
		if (who <= 0) {
			if (node == 5 && entry == 0) {
				_veraShould = 9;
			} else if (node == 6 && entry == 1) {
				sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			} else if (node == 5 && entry == 1) {
				digi_preload("92n0603");
				_veraShould = 9;
			} else {
				_veraShould = 4;
			}

			conv_resume();

		} else if (who == 1) {
			sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			conv_resume();
		}
	} else if (sound) {
		if (who <= 0) {
			if ((node == 2 && entry == 1) ||
					(node == 3) ||
					(node == 5 && entry == 1) ||
					(node == 5 && entry == 3) ||
					(node == 6 && entry == 2) ||
					(node == 7 && entry == 0) ||
					(node == 8 && entry == 2) ||
					(node == 9 && entry == 1) ) {
				if (node == 2 && entry == 1)
					_veraShould = 9;

				digi_play(sound, 1, 255, 18);
			} else if (node == 5 && entry == 0) {
				_veraShould = 10;
				_digiName = sound;
			} else if (node == 5 && entry == 2) {
				_veraShould = 14;
			} else if (node == 6 && entry == 1) {
				sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);
				digi_play(sound, 1, 255, 18);
			} else if ((node == 1 && entry == 1) || (node == 2 && entry == 2) ||
					(node == 1 && entry == 4)) {
				_veraShould = 17;
				_digiName = sound;
			} else if (node == 4) {
				_veraShould = 10;
				_digiName = sound;
			} else {
				_veraShould = 7;
				_digiName = sound;
			}
		} else if (who == 1) {
			sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);
			digi_play(sound, 1, 255, 18);
		}

		_newMode = KT_PARSE;
		_newTrigger = 18;
	} else {
		conv_resume();
	}
}

void Room405::talkToVipe() {
	conv_load_and_prepare("conv86", 1, 0);
	conv_export_pointer_curr(&_G(flags)[V165], 1);
	conv_play_curr();
}

void Room405::talkToVera() {
	conv_load_and_prepare("conv92", 5);
	conv_export_value_curr(_G(flags)[V062], 0);
	conv_play_curr();
}

void Room405::startConv89() {
	conv_load_and_prepare("conv89", 2);
	conv_export_value_curr(_G(flags)[V165], 0);
	conv_play_curr();
}

void Room405::startConv90() {
	conv_load_and_prepare("conv90", 3);
	conv_export_value_curr(_G(flags)[V165], 0);
	conv_play_curr();
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
