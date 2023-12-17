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

#include "m4/burger/rooms/section1/room104.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_STOLIE_ANIMATION = 3
};

static const char *SAID1[][4] = {
	{ "TOWN HALL",   "104W001", "104W002", "104w002" },
	{ "DOORS",       "104W003", "104W002", nullptr },
	{ "STOLIE",      "104W004", "104W002", "104W002" },
	{ "MAIN STREET", nullptr,   "104W002", "104W002" },
	{ "AUNT POLLY'S HOUSE", nullptr, "104W002", "104W002" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 1,  3, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 4, 23, "104S101", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 18, 28, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 29, 47, "104_003", 2, 150, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room104::init() {
	_stolieSleepCtr = 0;
	_stolieShould = 14;
	_val3 = 0;

	digi_preload("104_001");

	switch (_G(game).previous_room) {
	case 20:
		break;

	case 101:
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 105:
		_G(wilbur_should) = 2;
		digi_preload("104_005");
		digi_play("104_005", 2, 255, 2);
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 107:
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 170:
		_G(wilbur_should) = 4;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(213, 297, 5);
		break;
	}

	static const char *NAMES[14] = {
		"104W101",  "104W100A", "104W100B", "104W100C", "104W100D",
		"104W100E", "104W100F", "104W100G", "104s100a", "104s100b",
		"104s100c", "104s100d", "104s100e", "104s101"
	};
	for (int i = 0; i < 14; ++i)
		digi_preload(NAMES[i]);

	digi_play_loop("104_001", 3, 200, -1);
	_G(flags)[V028] = 1;
	kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
}

void Room104::daemon() {
	int frame, frameRate;

	if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
		player_update_info();

		if (_G(player_info).y > 374) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1001);
			_G(kernel).call_daemon_every_loop = false;
		}
	}

	switch (_G(kernel).trigger) {
	case 1:
		digi_play("104_006", 2, 255, 1005);
		break;

	case 2:
		digi_unload("104_005");
		break;

	case kCHANGE_STOLIE_ANIMATION:
		switch (_stolieShould) {
		case 6:
			_flag1 = false;
			_stolieShould = 9;
			Series::series_play("104dr04", 0x200, 2, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 0, 10);
			break;

		case 8:
			_flag1 = false;
			_stolieShould = 13;
			Series::series_play("104dr04", 0x200, 2, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 0, 10);
			break;

		case 9:
			_flag1 = false;
			_stolieShould = 14;
			Series::series_play("104dr02", 0x200, 0, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
			break;

		case 10:
			digi_stop(2);
			freeSeries();
			_flag1 = true;
			_series1.play("104DR04", 0x200, 16, 4, 6, 0, 100, 0, 0, 0, 10);
			break;

		case 12:
			freeSeries();
			_stolieShould = 10;
			series_play_with_breaks(PLAY1, "104DR03", 0x200, kCHANGE_STOLIE_ANIMATION, 1);
			break;

		case 13:
			_flag1 = false;
			_stolieShould = 9;
			series_play_with_breaks(PLAY2, "104dr01", 0x200, kCHANGE_STOLIE_ANIMATION, 1);
			break;

		case 14:
			if (!digi_play_state(2)) {
				if (imath_ranged_rand(1, 4) == 1) {
					_stolieShould = 15;
					Series::series_play("104dr02", 0x200, 0, kCHANGE_STOLIE_ANIMATION, 8, 0, 100, 0, 0, 13, 14);
					return;
				}
				_stolieSleepCtr = 0;

				switch (getRandom()) {
				case 1:
					digi_play("104s100a", 2, 125, -1);
					break;
				case 2:
					digi_play("104s100b", 2, 125, -1);
					_stolieSleepCtr = 100;
					break;
				case 3:
					digi_play("104s100c", 2, 125, -1);
					break;
				case 4:
					digi_play("104s100d", 2, 125, -1);
					break;
				case 5:
					digi_play("104s100e", 2, 125, -1);
					break;
				default:
					break;
				}
			}

			_flag1 = false;
			++_stolieSleepCtr;

			frame = 0;
			if (_stolieSleepCtr < 3)
				frame = 22;
			else if (_stolieSleepCtr < 14)
				frame = 23;
			else {
				switch (imath_ranged_rand(1, 3)) {
				case 1:
					frame = 22;
					break;
				case 2:
					frame = 24;
					break;
				case 3:
					frame = 25;
					break;
				default:
					break;
				}
			}

			frameRate = _stolieSleepCtr < 14 ? 15 : imath_ranged_rand(6, 15);
			Series::series_play("104dr02", 0x200, 0, kCHANGE_STOLIE_ANIMATION,
				frameRate, 0, 100, 0, 0, frame, frame);
			break;

		case 15:
			if (!digi_play_state(2)) {
				switch (imath_ranged_rand(1, 6)) {
				case 1:
					_stolieShould = 14;
					Series::series_play("104dr02", 0x200, 2, kCHANGE_STOLIE_ANIMATION, 8, 0, 100, 0, 0, 13, 14);
					return;
				case 2:
					_stolieShould = 16;
					Series::series_play("104dr02", 0x200, 0, kCHANGE_STOLIE_ANIMATION, 8, 0, 100, 0, 0, 20, 20);
					return;
				default:
					break;
				}

				_stolieSleepCtr = 0;

				switch (getRandom()) {
				case 1:
					digi_play("104s100a", 2, 125, -1);
					break;
				case 2:
					digi_play("104s100b", 2, 125, -1);
					_stolieSleepCtr = 100;
					break;
				case 3:
					digi_play("104s100c", 2, 125, -1);
					break;
				case 4:
					digi_play("104s100d", 2, 125, -1);
					break;
				case 5:
					digi_play("104s100e", 2, 125, -1);
					break;
				default:
					break;
				}
			}

			_flag1 = false;
			++_stolieSleepCtr;

			frame = 0;
			if (_stolieSleepCtr < 3)
				frame = 11;
			else if (_stolieSleepCtr < 14)
				frame = 10;
			else {
				switch (imath_ranged_rand(1, 3)) {
				case 1:
					frame = 8;
					break;
				case 2:
					frame = 9;
					break;
				case 3:
					frame = 12;
					break;
				default:
					break;
				}
			}
			frameRate = _stolieSleepCtr < 14 ? 15 : imath_ranged_rand(6, 15);
			Series::series_play("104dr02", 0x200, 0, kCHANGE_STOLIE_ANIMATION, frameRate, 0, 100, 0, 0, frame, frame);
			break;

		case 16:
			if (!digi_play_state(2)) {
				if (imath_ranged_rand(1, 4) == 1) {
					_stolieShould = 15;
					Series::series_play("104dr02", 0x200, 0, kCHANGE_STOLIE_ANIMATION, 8, 0, 100, 0, 0, 20, 20);
					break;
				} else {
					switch (getRandom()) {
					case 1:
						digi_play("104s100a", 2, 125, -1);
						break;
					case 2:
						digi_play("104s100b", 2, 125, -1);
						_stolieSleepCtr = 100;
						break;
					case 3:
						digi_play("104s100c", 2, 125, -1);
						break;
					case 4:
						digi_play("104s100d", 2, 125, -1);
						break;
					case 5:
						digi_play("104s100e", 2, 125, -1);
						break;
					default:
						break;
					}
				}
			}

			_flag1 = false;
			++_stolieSleepCtr;

			frame = 0;
			if (_stolieSleepCtr < 3)
				frame = 16;
			else if (_stolieSleepCtr < 14)
				frame = 18;
			else {
				switch (getRandom()) {
				case 1:
					frame = 15;
					break;
				case 2:
					frame = 17;
					break;
				case 3:
					frame = 19;
					break;
				default:
					break;
				}
			}

			frameRate = (_stolieSleepCtr < 14) ? 15 : imath_ranged_rand(6, 15);
			Series::series_play("104dr02", 0x200, 0, kCHANGE_STOLIE_ANIMATION, frameRate, 0, 100, 0, 0, frame, frame);
			break;

		case 17:
			freeSeries();
			_flag1 = true;
			_stolieShould = 18;
			_series1.play("104DR04", 512, 4, -1, 6, -1, 100, 0, 0, 11, 15);
			digi_play(conv_sound_to_play(), 1, 255, kCHANGE_STOLIE_ANIMATION);
			break;

		case 18:
			freeSeries();
			_flag1 = true;
			_series1.show("104DR04", 512, 0, -1, -1, 11);
			conv_resume_curr();
			break;

		default:
			break;
		}
		break;

	case 4:
		conv_load_and_prepare("conv16", 5, false);
		conv_export_pointer_curr(&_G(flags)[WAKE_UP_STOLIE_COUNT], 0);
		conv_play_curr();
		break;

	case 5:
		term_message("WAKE UP STOLIE COUNT = %d", _G(flags)[WAKE_UP_STOLIE_COUNT]);
		if (!_G(flags)[WAKE_UP_STOLIE_COUNT]) {
			term_message("AT END OF CONV? COUNT = %d", _G(flags)[WAKE_UP_STOLIE_COUNT]);
			_G(flags)[V028] = 0;
			_G(flags)[WAKE_UP_STOLIE_COUNT]++;
		}

		freeSeries();

		switch (_G(flags)[WAKE_UP_STOLIE_COUNT]) {
		case 1:
		case 4:
		case 7:
		case 11:
			_stolieShould = 6;
			break;
		case 2:
		case 3:
		case 5:
		case 6:
		case 8:
		case 9:
		case 10:
			_stolieShould = 8;
			break;
		default:
			break;
		}

		kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
		player_set_commands_allowed(true);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			ws_demand_location(315, 373);
			ws_demand_facing(10);
			ws_walk(271, 348, 0, -1, 10);
			break;
		case 2:
			ws_demand_location(208, 243);
			ws_demand_facing(7);
			ws_walk(180, 287, 0, -1, 7);
			break;
		case 3:
			ws_demand_location(417, 361);
			ws_demand_facing(3);
			break;
		case 4:
			ws_demand_location(0, 326);
			ws_demand_facing(3);
			ws_walk(36, 338, nullptr, -1, 3);
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

void Room104::pre_parser() {
	_G(kernel).call_daemon_every_loop = player_said("MAIN STREET") &&
		player_said_any("look at", "enter");
}

void Room104::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(walker).wilbur_said(SAID1)) {
		if (player_said("MAIN STREET") && player_said_any("look at", "enter")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1001);

		} else if (player_said("ENTER", "DOORS") || player_said("ENTER", "DOOR HANDLE") ||
				player_said("gear", "doors") || player_said("gear", "door handle")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1);

		} else if (player_said("LOOK AT", "AUNT POLLY'S HOUSE")) {
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1017);

		} else if (player_said("ENTER", "AUNT POLLY'S HOUSE")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1017);

		} else if (player_said("talk to", "stolie")) {
			if (_G(flags)[V028]) {
				player_set_commands_allowed(false);

				switch (_G(flags)[WAKE_UP_STOLIE_COUNT]) {
				case 0:
					_stolieShould = 12;
					_G(walker).wilbur_speech_random("104W100A", "104W100B", "104W100C",
						"104W100D", "104W100E", "104W100F", "104W100G");
					break;
				case 12:
					_G(walker).wilbur_speech("104W101");
					_G(flags)[V028] = 0;
					player_set_commands_allowed(true);
					break;
				default:
					_stolieShould = 10;
					break;
				}
			} else {
				_G(walker).wilbur_speech("104W101");
			}
		} else if (player_said("conv16")) {
			conv();
		} else {
			return;
		}
	}

	_G(player).command_ready = false;
}

void Room104::conv() {
	if (conv_sound_to_play()) {
		switch (conv_whos_talking()) {
		case 0:
			_stolieShould = 17;
			kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			break;
		case 1:
			_G(walker).wilbur_speech(conv_sound_to_play(), 10001);
			break;
		default:
			break;
		}
	}
}

void Room104::freeSeries() {
	if (_flag1) {
		_series1.terminate();
		_flag1 = false;
	}
}

int Room104::getRandom() const {
	if (imath_ranged_rand(1, 4) == 1) {
		return imath_ranged_rand(3, 4);
	} else {
		switch (imath_ranged_rand(1, 5)) {
		case 1:
		case 2:
			return 1;
		case 3:
			return 2;
		default:
			return 5;
		}
	}
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
