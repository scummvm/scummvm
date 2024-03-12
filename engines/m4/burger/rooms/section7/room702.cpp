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

#include "m4/burger/rooms/section7/room702.h"
#include "m4/burger/rooms/section7/section7.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_FLUMIX_ANIMATION = 5
};

static const char *SAID[][4] = {
	{ "DOOR",          nullptr,    "702W002", nullptr   },
	{ "FLUMIX",        "702w001z", "702W002", "702W002" },
	{ "ABDUCT-O-TRON", "702W006",  nullptr,   "702W007" },
	{ "ABDUCT-O CONTROLS",    "702W008", nullptr,   "702W009" },
	{ "NEUTRINO PULSE GUN",   "702W010", nullptr,   "702W011" },
	{ "NEUTRINO GUN CONTROL", "702W012", nullptr,   "702W009" },
	{ "VENT",          "702W013",  nullptr,   "702W013" },
	{ "BRIG",          "702W014",  "702W002", "702W015" },
	{ "MIRROR",        "999W010", nullptr,    "999W010" },
	{ nullptr, nullptr, nullptr, nullptr }
};


void Room702::init() {
	pal_fade_set_start(0);
	_flag1 = false;
	_flag2 = false;
	_flumixMode = 1;

	_series1 = series_load("702FLT01");
	_series2 = series_load("702FLX02");
	_series3 = series_load("702C1FL1");
	_series4 = series_load("702C1FL2");
	_series5 = series_load("702C1FL3");
	_series1 = series_load("702C1FX1");

	digi_preload("700_001");
	digi_preload("702f003b");
	digi_preload("702_007");
	digi_preload("702_006");

	kernel_trigger_dispatch_now(1);
}

void Room702::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		ws_demand_location(302, 284, 6);
		_val2 = 12;
		_G(flumix_should) = 8;
		_G(wilbur_should) = 10;
		digi_play_loop("700_001", 3, 50, -1, 700);
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		kernel_trigger_dispatch_now(kCHANGE_FLUMIX_ANIMATION);
		kernel_trigger_dispatch_now(6);
		_G(flags)[kDisableFootsteps] = 1;
		pal_fade_init(0, 255, 100, 30, -1);
		break;

	case 2:
		conv_load_and_prepare("conv82", 3);
		conv_play_curr();
		break;

	case 3:
		_G(flumix_should) = 4;
		pal_fade_init(_G(kernel).first_fade, 255, 0, 90, -1);
		kernel_timing_trigger(30, kCHANGE_FLUMIX_ANIMATION);
		break;

	case 4:
		_G(game).new_room = 706;
		break;

	case kCHANGE_FLUMIX_ANIMATION:
		switch (_flumixMode) {
		case 1:
			switch (_G(flumix_should)) {
			case 3:
				_flumixMode = 2;
				break;

			case 4:
				if (_flag1)
					terminateMachineAndNull(_flumix);

				_flumix = series_play("702FLX02", 0, 0, 4);
				break;

			case 5:
				_G(flumix_should) = getFlumixShould();
				_flag1 = true;
				_flumix = series_play("702C1FL2", 0, 2, kCHANGE_FLUMIX_ANIMATION);

				if (imath_ranged_rand(0, 1) == 1) {
					digi_play("702F003B", 2, 100);
				} else {
					digi_play("702_007", 2, 100);
				}
				break;

			case 6:
				_G(flumix_should) = getFlumixShould();
				_flag1 = true;
				_flumix = series_play("702C1FL3", 0, 1, kCHANGE_FLUMIX_ANIMATION);

				if (imath_ranged_rand(0, 1) == 1) {
					digi_play("702F003B", 2, 100);
				} else {
					digi_play("702_007", 2, 100);
				}
				break;

			case 7:
				_G(flumix_should) = getFlumixShould();
				_flag1 = true;
				_flumix = series_play("702C1FL2", 0, 0, kCHANGE_FLUMIX_ANIMATION);

				if (imath_ranged_rand(0, 1) == 1) {
					digi_play("702F003B", 2, 100);
				} else {
					digi_play("702_007", 2, 100);
				}
				break;

			case 8:
				_G(flumix_should) = getFlumixShould();
				_flag1 = true;
				_flumix = series_play("702C1FL1", 0, 0, kCHANGE_FLUMIX_ANIMATION);
				digi_play("702F003B", 2, 120);
				break;

			case 9:
				_G(flumix_should) = getFlumixShould();
				_flag1 = true;
				_flumix = series_play("702FLT01", 0, 0, kCHANGE_FLUMIX_ANIMATION, 60);
				break;

			default:
				break;
			}
			break;

		case 2:
			switch (_G(flumix_should)) {
			case 3:
				if (_flag1)
					terminateMachineAndNull(_flumix);

				_flag1 = true;
				_G(flumix_should) = 21;
				_flumix = series_play("702FLT01", 0, 4, -1, 6, -1);
				digi_play(conv_sound_to_play(), 1, 255, 5);
				break;

			case 21:
				terminateMachineAndNull(_flumix);
				_flag1 = false;
				_G(flumix_should) = getFlumixShould();
				kernel_trigger_dispatch_now(5);
				conv_resume_curr();
				break;


			default:
				_flumixMode = 1;
				kernel_trigger_dispatch_now(5);
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 6:
		switch (_val2) {
		case 12:
			_val2 = 13;
			_series11 = series_play("702C1FX1", 0xf00, 0, 6, 6);
			digi_play("702_006", 1, 200);
			break;

		case 13:
			series_unload(_series6);
			digi_unload("702_006");
			break;

		default:
			break;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 10:
			ws_walk(236, 338, 0, 11, 4);
			break;

		case 11:
			_G(flags)[kDisableFootsteps] = 0;
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

void Room702::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("TALK", "FLUMIX")) {
		kernel_trigger_dispatch_now(2);

	} else if (player_said("conv82")) {
		conv82();

	} else if (player_said("DOOR", "LOOK AT")) {
		if (_flag2) {
			wilbur_speech("702W004");
		} else {
			_flag2 = true;
			wilbur_speech("702W003");
		}
	} else if (player_said("DOOR", "GEAR")) {
		ws_walk(174, 316, 0, -1, 10);
		wilbur_speech("702W005");

	} else if (!_G(walker).wilbur_said(SAID)) {
		return;
	}

	_G(player).command_ready = false;
}

void Room702::conv82() {
	int who = conv_whos_talking();

	if (conv_sound_to_play()) {
		if (who <= 0) {
			_G(flumix_should) = 3;
			kernel_trigger_dispatch_now(kCHANGE_FLUMIX_ANIMATION);

		} else if (who == 1) {
			wilbur_speech(conv_sound_to_play(), 10001);
		}
	}
}

int Room702::getFlumixShould() {
	switch (imath_ranged_rand(1, 12)) {
	case 1:
	case 2:
	case 3:
		return 7;
	case 4:
	case 5:
	case 6:
		return 5;
	case 7:
	case 8:
		return 6;
	case 9:
	case 10:
		return 8;
	default:
		return 9;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
