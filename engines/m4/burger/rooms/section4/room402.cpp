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

#include "m4/burger/rooms/section4/room402.h"
#include "m4/burger/rooms/section4/section4.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_STOLIE_ANIMATION = 2,
	kCHANGE_ELMO_ANIMATION = 3
};

const char *Room402::SAID[][4] = {
	{ "bench",       "402w006", nullptr,   "402w007" },
	{ "force field", "400w005", nullptr,   "400w001" },
	{ "sidewalk",    nullptr,   nullptr,   nullptr   },
	{ "steps",       nullptr,   nullptr,   nullptr   },
	{ "road",        nullptr,   nullptr,   nullptr   },
	{ "townhall",    nullptr,   nullptr,   nullptr   },
	{ "sky",         nullptr,   nullptr,   nullptr   },
	{ "bush",        nullptr,   nullptr,   nullptr   },
	{ "steps",       nullptr,   nullptr,   nullptr   },
	{ "pillar",      nullptr,   nullptr,   nullptr   },
	{ "window",      nullptr,   nullptr,   nullptr   },
	{ "bushes",      nullptr,   nullptr,   nullptr   },
	{ "stolie",      nullptr,   "400w001", "400w001" },
	{ "elmo",        "402w003", "400w001", "400w001" },
	{ "disc",        nullptr,   "400w003", nullptr   },
	{ "disc ",       nullptr,   "400w003", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room402::SERIES1[] = {
	{ 0, "402s001", 1, 255, -1, 0, nullptr, 0 },
	{ 21, nullptr,  1, 255,  7, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room402::SERIES2[] = {
	{  0, "402s003", 1, 255, -1, 0, nullptr, 0 },
	{  4, "402_002", 2, 255, -1, 0, nullptr, 0 },
	{ 11, nullptr,   1, 255,  9, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room402::PLAY1[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY2[] = {
	{  0,  5, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{  6,  9, "402s002", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 10, 11, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY3[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY4[] = {
	{ 2, 7, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY5[] = {
	{ 0, 4, "402p901", 1, 255, -1, 4, -1, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY6[] = {
	{ 10, 16, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY7[] = {
	{ 18, 21, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY8[] = {
	{ 2, 10, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY9[] = {
	{ 0, 2, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 3, 5, "402s004", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 6, 13, nullptr,  0,   0, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY10[] = {
	{  0,  1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{  2,  9, nullptr,   0,   0, 38,    0, 0, nullptr, 0 },
	{ 10, 11, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 12, 12, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 12, 12, "402_001", 2, 255, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 13, 13, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 14, 16, nullptr,   2,   0, -1, 2048, 0, nullptr, 0 },
	{  3,  2, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY11[] = {
	{ 19, 17, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 16, 16, nullptr, 0, 0, 33, 0, 0, nullptr, 0 },
	{ 15, 12, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{  7,  2, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 22, 23, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY12[] = {
	{ 0, 9, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY13[] = {
	{ 0,  2, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 3,  6, "402S005", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 7, 13, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY14[] = {
	{  0,  1, "999_003", 1, 255, -1, 0, 0, nullptr, 0 },
	{  2,  2, nullptr,   0,   0,  4, 0, 0, nullptr, 0 },
	{  3,  9, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 10, 11, "999_003", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 12, -1, nullptr,   0,   0,  5, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY15[] = {
	{ 0, 0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room402::PLAY16[] = {
	{  0, 16, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 17, 20, "402_001",  1, 255,   -1, 0, 0, nullptr, 0 },
	{ 20, 20, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 20, 20, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 20, 20, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	{ 21, 23, nullptr,    0,   0, 2048, 0, 0, nullptr, 0 },
	{ 24, 36, "402p903a", 1, 255,   -1, 0, 0, nullptr, 0 },
	{ 64, 66, nullptr,    0,   0,   -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room402::init() {
	term_message(_G(player).been_here_before ? " been here before" : " NOT been here before");
	_val4 = 0;
	_dr01 = _dr01s = -1;
	_dr02 = _dr02s = -1;
	_dr03 = _dr03s = -1;
	_dr08 = _dr08s = -1;
	_pe01 = _pe01s = -1;
	_pe02 = _pe02s = -1;
	_pe03 = _pe03s = -1;
	_pe04 = _pe04s = -1;
	_wi01 = _wi01s = -1;
	_wi02 = _wi02s = -1;
	_stolieSet = _series2Set = false;

	pal_cycle_init(114, 127, 6);
	digi_preload("400_001");

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 406:
		player_set_commands_allowed(false);
		ws_demand_location(597, 170, 8);
		ws_hide_walker();
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(93, 260, 4);
		ws_hide_walker();
		break;
	}

	if (_G(flags)[V159]) {
		hotspot_set_active("elmo", false);
		hotspot_set_active("stolie", false);

		if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
			_G(wilbur_should) = 5;
			kernel_timing_trigger(120, kCHANGE_WILBUR_ANIMATION);
		}
	} else {
		_G(flags)[kDisableFootsteps] = 1;
		static const char *NAMES[11] = {
			"402p903a", "402p903b", "402p903c", "402p903d", "402p903e", "402p903f",
			"402s010f", "402s010g", "402s010h", "402s010a", "402s010d"
		};
		for (int i = 0; i < 11; ++i)
			digi_preload(NAMES[i]);

		loadPe1();
		_elmoMode = 15;
		_elmoShould = 15;
		kernel_trigger_dispatch_now(kCHANGE_ELMO_ANIMATION);

		if (_G(player).been_here_before) {
			if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
				_stolieShould = 15;
			} else {
				_G(wilbur_should) = 5;
				kernel_timing_trigger(120, kCHANGE_WILBUR_ANIMATION);
				_stolieShould = 15;
			}
		} else {
			_stolieShould = 15;

			if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
				digi_preload_stream_breaks(SERIES1);
				digi_preload_stream_breaks(SERIES2);
				series_load("402dr06");
				series_load("402dr06s");
				digi_preload("402p901");
				series_load("402pe4n");
				series_load("402pe4ns");

				_stolieShould = 16;
				_G(wilbur_should) = 2;
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			}
		}

		_stolieMode = 15;
		kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
	}

	digi_play_loop("400_001", 3, 175);
}

void Room402::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 1:
		_stolieShould = 15;
		if (_stolieSet)
			kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);

		_elmoShould = 15;
		if (_series2Set)
			kernel_trigger_dispatch_now(kCHANGE_ELMO_ANIMATION);

		player_set_commands_allowed(true);
		break;

	case kCHANGE_STOLIE_ANIMATION:
		switch (_stolieMode) {
		case 6:
			freeDr8();
			_wi02 = series_load("402wi02");
			_wi02s = series_load("402wi02s");
			Series::series_play("402dr04", 0x300, 0, 11, 6, 0, 100, 0, 0, 0, 25);
			break;

		case 7:
			if (_stolieShould == 7) {
				loadDr8();
				_stolie.play("402dr08", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 60, 0, 100, 0, 0, 75, 75);
			} else {
				_stolieMode = 45;
				loadDr8();
				_stolie.play("402dr08", 0x300, 2, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 72, 75);
			}
			break;

		case 15:
			switch (_stolieShould) {
			case 6:
				_stolieMode = 6;
				loadDr1();
				Series::series_play("402dr01", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 0, 7);
				break;

			case 8:
				digi_unload_stream_breaks(SERIES1);
				_stolieShould = 11;
				series_play_with_breaks(PLAY2, "402dr06", 0x300, kCHANGE_STOLIE_ANIMATION, 3);
				break;

			case 10:
				_stolieShould = 8;
				digi_preload("402_003");
				series_play_with_breaks(PLAY1, "402DR10s", 0x300, -1, 2);
				series_stream_with_breaks(SERIES1, "402dr10", 6, 0x300, kCHANGE_STOLIE_ANIMATION);
				break;

			case 11:
				_stolieShould = 12;
				loadPe4();
				series_play_with_breaks(PLAY3, "402DR11s", 0x301, -1, 2);
				series_stream_with_breaks(SERIES2, "402dr11", 6, 0x300, kCHANGE_STOLIE_ANIMATION);
				break;

			case 12:
				digi_unload_stream_breaks(SERIES2);
				_stolieShould = 13;
				loadDr1();
				Series::series_play("402dr01", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 3, -1);
				break;

			case 13:
				_stolieSet = true;
				loadDr2();
				_stolie.play("402dr02", 0x300, 0, -1, 6, -1, 100, 0, 0, 12, 12);
				kernel_trigger_dispatch_now(27);
				break;

			case 15:
				loadDr8();
				series_play("402dr08", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 60, 0, 100, 0, 0, 0, 0);
				series_play("402dr08s", 0x301, 0, -1, 60, 0, 100, 0, 0, 0, 0);
				break;

			case 16:
				Series::series_play("402dr12", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 60, 0, 100, 0, 0, 0, 0);
				break;

			case 39:
			case 40:
				_stolieMode = 39;
				loadDr1();
				_stolie.play("402dr01", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 6);
				break;

			case 45:
			case 47:
				_stolieMode = 48;
				playRandom1();
				loadDr8();
				Series::series_play("402dr08", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 5, 0, 100, 0, 0, 0, 8);
				break;

			case 46:
				_stolieMode = 45;
				freeDr8();
				series_play_with_breaks(PLAY16, "402dr08", 0x300, kCHANGE_STOLIE_ANIMATION, 3, 5);
				break;

			default:
				break;
			}
			break;

		case 39:
			switch (_stolieShould) {
			case 39:
				freeStolie();
				_stolieSet = true;
				loadDr2();
				_stolie.play("402dr02", 0x300, 0, -1, 6, -1, 100, 0, 0, 12, 12);
				break;

			case 40:
				freeStolie();
				_stolieMode = 40;
				loadDr2();
				series_play("402dr02", 0x300, 4, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 0, 2);
				series_play("402dr02s", 0x301, 4, -1, 6, 0, 100, 0, 0, 0, 2);
				playDigiName();
				break;

			default:
				freeStolie();
				_stolieMode = 15;
				loadDr1();
				series_play("402dr01", 0x300, 2, 2, 6);
				series_play("402dr01s", 0x301, 2, -1, 6);
				break;
			}
			break;

		case 40:
			if (_stolieShould == 40) {
				freeStolie();
				_stolieSet = true;
				loadDr2();
				_stolie.play("402dr02", 0x300, 4, -1, 6, -1, 100, 0, 0, 3, 5);
			} else {
				freeStolie();
				_stolieMode = 39;
				loadDr2();
				Series::series_play("402dr02", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 6, 12);
			}
			break;

		case 45:
			switch (_stolieShould) {
			case 7:
				freeStolie();
				_stolieMode = 7;
				loadDr8();
				_stolie.play("402dr08", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 72, 75);
				break;

			case 45:
			case 46:
				freeStolie();
				_stolieSet = true;
				loadDr8();
				_stolie.play("402dr08", 0x300, 4, -1, 6, -1, 100, 0, 0, 67, 71);
				playDigiName();
				break;

			case 47:
				freeStolie();
				loadDr8();
				Series::series_play("402dr08", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 60, 0, 100, 0, 0, 67, 67);
				break;

			default:
				freeStolie();
				_stolieMode = 49;
				loadDr8();
				Series::series_play("402dr08", 0x300, 2, kCHANGE_STOLIE_ANIMATION, 5, 0, 100, 0, 0, 64, 66);
				break;
			}
			break;

		case 48:
			_stolieMode = 45;
			loadDr8();
			Series::series_play("402dr08", 0x300, 0, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 64, 66);
			break;

		case 49:
			_stolieMode = 15;
			loadDr8();
			Series::series_play("402dr08", 0x300, 2, kCHANGE_STOLIE_ANIMATION, 6, 0, 100, 0, 0, 0, 8);
			break;

		default:
			break;
		}
		break;

	case kCHANGE_ELMO_ANIMATION:
		switch (_elmoMode) {
		case 15:
			switch (_elmoShould) {
			case 15:
				if (imath_ranged_rand(1, 7) == 1) {
					_elmoMode = 17;
					loadPe2();
					Series::series_play("402pe02", 0x100, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
					playRandom2();
				} else {
					loadPe2();
					Series::series_play("402pe02", 0x100, 0, kCHANGE_ELMO_ANIMATION, 60, 0, 100, 0, 0, 7, 7);
				}
				break;

			case 20:
				_elmoShould = 21;
				series_play_with_breaks(PLAY12, "402pe01", 0x100, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 21:
				loadPe1();
				_series2.show("402pe01", 0x100, 0, kCHANGE_ELMO_ANIMATION, 30, 9);
				break;

			case 27:
				_elmoShould = 28;
				_series2Set = true;
				kernel_trigger_dispatch_now(27);

				loadPe4();
				_series2.show("402pe04", 0x100, 0, -1, -1, 16);
				break;

			case 28:
				freeSeries2();
				freePe4();
				_elmoShould = 29;
				series_play_with_breaks(PLAY7, "402pe04", 0x100, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 29:
				_elmoShould = 30;
				loadPe4();
				_series2.play("402pe04", 0x100, 1, -1, 12, -1, 100, 0, 0, 19, 21);
				break;

			case 30:
				_elmoShould = 20;
				freeSeries2();
				freePe4();
				series_play_with_breaks(PLAY11, "402pe04", 0x100, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 34:
				_elmoShould = 43;
				series_play_with_breaks(PLAY15, "402pe05", 0x100, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 23:
			case 42:
			case 45:
			case 47:
				loadPe1();
				_elmoMode = 45;
				_series2.play("402pe01", 0x100, 0, 3, 5, 0, 100, 0, 0, 0, -1);
				break;

			default:
				break;
			}
			break;

		case 17:
			if (_elmoShould == 15) {
				if (imath_ranged_rand(1, 7) == 1) {
					_elmoMode = 15;
					loadPe2();
					Series::series_play("402pe02", 0x100, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 4, 7);
					playRandom2();
				} else {
					loadPe2();
					Series::series_play("402pe02", 0x100, 0, kCHANGE_ELMO_ANIMATION, 60, 0, 100, 0, 0, 3, 3);
				}
			} else {
				_elmoMode = 15;
				loadPe2();
				Series::series_play("402pe02", 0x100, 0, kCHANGE_ELMO_ANIMATION, 6, 0, 100, 0, 0, 4, 7);
			}
			break;

		case 45:
			switch (_elmoShould) {
			case 23:
				_elmoShould = 24;
				freePe4();
				series_play_with_breaks(PLAY4, "402pe04", 0x100, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 24:
				_elmoShould = 25;
				series_play_with_breaks(PLAY5, "402pe4n", 0x100, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 25:
				_elmoShould = 27;
				_elmoMode = 15;
				freePe4();
				series_play_with_breaks(PLAY6, "402pe04", 0x100, kCHANGE_ELMO_ANIMATION, 3);
				break;

			case 41:
				loadPe1();
				freePe3();
				_series2Set = true;
				_series2.play("402pe01", 0x100, 0, -1, 6, -1, 100, 0, 0, 9, 9);
				break;

			case 42:
				Series::series_play("402pe01", 0x100, 0, kCHANGE_ELMO_ANIMATION, 60, 0, 100, 0, 0, 9, 9);
				break;

			case 45:
				freeSeries2();

				switch (imath_ranged_rand(1, 3)) {
				case 1:
					frame = 0;
					break;
				case 2:
					frame = 1;
					break;
				default:
					frame = 16;
					break;
				}

				loadPe3();
				Series::series_play("402pe03", 0x100, 0, kCHANGE_ELMO_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				playDigiName();
				break;

			case 47:
				loadPe3();
				_series2Set = true;
				_series2.play("402pe03", 0x100, 0, -1, 6, -1, 100, 0, 0, 15, 15);
				break;

			default:
				freeSeries2();
				freePe3();
				loadPe1();
				_elmoMode = 15;
				_series2.play("402pe01", 0x100, 2, kCHANGE_ELMO_ANIMATION, 5, 0, 100, 0, 0, 0, -1);
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 4:
		freeStolie();
		hotspot_set_active("stolie", false);
		_G(flags)[V159] = 1;
		break;

	case 5:
		freeSeries2();
		hotspot_set_active("elmo", false);
		break;

	case 7:
		digi_play("402_003", 2, 150, 8);
		break;

	case 8:
		digi_unload("402_003");
		break;

	case 9:
		_elmoShould = 23;
		break;

	case 11:
		_stolie.play("402dr04", 0x300, 0, -1, 6, -1, 100, 0, 0, 25, 25);
		ws_hide_walker();
		series_play("402wi02", 0x100, 0, 12, 5, 0, 100, 0, 0, 0, 4);
		series_play("402wi02s", 0x101, 0, -1, 5, 0, 100, 0, 0, 0, 4);
		break;

	case 12:
		_stolie.terminate();
		inv_give_to_player("fish");
		series_play("402dr04", 0x300, 0, 14, 5, 0, 100, 0, 0, 26, -1);
		series_play("402dr04s", 0x301, 0, -1, 5, 0, 100, 0, 0, 26, -1);
		series_play("402wi02", 0x100, 0, 13, 5, 0, 100, 0, 0, 5, -1);
		series_play("402wi02s", 0x101, 0, -1, 5, 0, 100, 0, 0, 5, -1);
		break;

	case 13:
		series_unload(_wi02);
		series_unload(_wi02s);
		enable_player();
		break;

	case 14:
		series_play_with_breaks(PLAY14, "402poof", 0x100, -1, 2);
		_stolieSet = true;
		_stolie.play("402dr04", 0x300, 0, -1, 6, -1, 100, 0, 0, 28, 28);
		break;

	case 15:
		freeDr2();
		_wi01 = series_load("402wi01");
		_wi01s = series_load("402wi01s");
		ws_hide_walker();

		_elmoShould = 15;
		series_play("402wi01", 0x100, 0, 16, 5, 0, 100, 0, 0, 0, 16);
		series_play("402wi01s", 0x101, 0, -1, 5, 0, 100, 0, 0, 0, 16);
		break;

	case 16:
		freeSeries2();
		_series3.play("402wi01", 0, 0, -1, 6, -1, 100, 0, 0, 16, 16);
		freePe1();
		freePe2();
		freePe3();
		series_play("402pe05", 0x100, 0, 17, 5, 0, 100, 0, 0, 0, 5);
		series_play("402pe05s", 0x101, 0, -1, 5, 0, 100, 0, 0, 0, 5);
		break;

	case 17:
		_series3.terminate();
		series_play("402wi01", 0x100, 0, 19, 5, 0, 100, 0, 0, 17, -1);
		series_play("402wi01s", 0x101, 0, -1, 5, 0, 100, 0, 0, 17, -1);
		digi_play("402p903b", 1, 255, -1);
		series_play("402pe05", 0x100, 0, 18, 5, 0, 100, 0, 0, 6, -1);
		series_play("402pe05s", 0x101, 0, -1, 5, 0, 100, 0, 0, 6, -1);
		inv_move_object("deed", NOWHERE);
		break;

	case 18:
		kernel_timing_trigger(1, 20);
		_series2Set = true;
		_series2.play("402pe05", 0x100, 0, -1, 6, -1, 100, 0, 0, 10, 10);
		break;

	case 19:
		series_unload(_wi01);
		series_unload(_wi01s);
		ws_unhide_walker();
		kernel_timing_trigger(1, 20);
		break;

	case 20:
		if (++_val4 == 2) {
			_val4 = 0;
			_stolieShould = 6;
			ws_walk(316, 354, 0, -1, 2);
		}
		break;

	case 22:
		_elmoShould = 42;
		break;

	case 25:
		freeDr1();
		series_play_with_breaks(PLAY8, "402dr01", 0x300, 26, 3);
		break;

	case 26:
		loadDr1();
		_stolieSet = true;
		_stolie.show("402dr01", 0x300, 0, -1, -1, 10);
		break;

	case 27:
		if (++_val4 >= 2) {
			_val4 = 0;
			freeStolie();
			freeDr2();
			series_play_with_breaks(PLAY9, "402dr02", 0x300, 28, 3);
		}
		break;

	case 28:
		series_play_with_breaks(PLAY10, "402dr03", 0x300, 30, 3);
		break;

	case 29:
		_series2Set = true;
		loadPe4();
		_series2.show("402pe04", 0x100, 0, -1, -1, 21);
		break;

	case 30:
		freeSeries2();
		digi_play("402s010h", 1, 170);
		_stolieSet = true;
		_stolie.show("402dr03", 0x300, 0, -1, -1, 2);
		_dr03 = series_load("402dr03");
		_dr03s = series_load("402dr03s");

		freePe4();
		series_play_with_breaks(PLAY11, "402pe04", 0x100, 32, 3);
		break;

	case 32:
		_elmoShould = 41;
		_elmoMode = 45;
		kernel_trigger_dispatch_now(kCHANGE_ELMO_ANIMATION);
		break;

	case 33:
		freeStolie();
		series_play("402dr03", 0x300, 2, 34, 6, 0, 100, 0, 0, 0, 1);
		series_play("402dr03s", 0x301, 2, -1, 6, 0, 100, 0, 0, 0, -1);
		break;

	case 34:
		series_unload(_dr03);
		series_unload(_dr03s);
		freeDr2();
		series_play_with_breaks(PLAY13, "402dr02", 0x300, 35, 3);
		break;

	case 35:
		loadDr1();
		series_play("402dr01", 0x300, 2, 36, 6, 0, 100, 0, 0, 0, -1);
		series_play("402dr01s", 0x301, 2, -1, 6, 0, 100, 0, 0, 0, -1);
		break;

	case 36:
		series_play("402dr12", 0x300, 0, 37, 6, 0, 100, 0, 0, 0, 0);
		series_play("402dr12s", 0x301, 0, -1, 6, 0, 100, 0, 0, 0, 0);
		break;

	case 37:
		freeDr1();
		_stolieShould = 15;
		_stolieMode = 15;
		kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
		player_set_commands_allowed(true);
		break;

	case 38:
		freeSeries2();
		loadPe4();
		Series::series_play("402pe04", 0x100, 0, 29, 6, 0, 100, 0, 0, 17, 21);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 2:
			_G(wilbur_should) = 3;
			kernel_timing_trigger(180, kCHANGE_WILBUR_ANIMATION);
			break;

		case 3:
			ws_unhide_walker();
			_G(walker).wilbur_poof();
			_stolieShould = 10;
			kernel_timing_trigger(30, 22);
			break;

		case 5:
			ws_unhide_walker();
			_G(walker).wilbur_poof();
			player_set_commands_allowed(true);
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

void Room402::pre_parser() {
	if (player_said("disc") && !player_said("poof") && !player_said("gear"))
		player_hotspot_walk_override(162, 264, 9, -1);

	if (player_said("disc ") && !player_said("poof ") && !player_said("gear"))
		player_hotspot_walk_override(556, 189, 2, -1);
}

void Room402::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (lookFlag && player_said("stolie")) {
		wilbur_speech(_G(flags)[V157] ? "402w002" : "402w001");
	} else if (lookFlag && player_said_any("disc", "disc ")) {
		wilbur_speech(_G(flags)[V154] ? "400w002" : "402w004");
	} else if (player_said_any("disc", "disc ") && inv_player_has(_G(player).verb)) {
		wilbur_speech("400w004");
	} else if (player_said("poof") || player_said("gear", "disc")) {
		poof(4006);
	} else if (player_said("poof ") || player_said("gear", "disc ")) {
		poof(4005);
	} else if (player_said("stolie") && inv_player_has(_G(player).verb) &&
			!player_said("deed")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_G(kernel).trigger_mode = KT_PARSE;
			wilbur_speech("402w901");
			break;

		case 1:
			_digiName = "402s006";
			_newMode = KT_PARSE;
			_digiTrigger = 2;
			freeStolie();
			_stolieShould = 45;
			break;

		case 2:
			_stolieShould = 15;
			kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			kernel_trigger_dispatch_now(1);
			break;

		default:
			break;
		}
	} else if (player_said("elmo") && inv_player_has(_G(player).verb) &&
			!player_said("deed")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);

			switch (imath_ranged_rand(1, 3)) {
			case 1:
				_digiName = "402p904a";
				break;
			case 2:
				_digiName = "402p904b";
				break;
			case 3:
				_digiName = "402p904c";
				break;
			default:
				break;
			}
			break;

		case 1:
			_elmoShould = 15;
			kernel_trigger_dispatch_now(1);
			break;

		default:
			break;
		}
	} else if (player_said("TALK TO", "stolie")) {
		player_set_commands_allowed(false);
		conv_load_and_prepare("conv84", 1);
		conv_export_pointer_curr(&Flags::_flags[V157], 0);
		conv_play_curr();
	} else if (player_said("TALK TO", "elmo")) {
		if (_G(flags)[V158]) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_G(kernel).trigger_mode = KT_PARSE;
				wilbur_speech("402w501", 1);
				break;

			case 1:
				switch (imath_ranged_rand(1, 3)) {
				case 1:
					_digiName = "402p502a";
					break;
				case 2:
					_digiName = "402p502b";
					break;
				case 3:
					_digiName = "402p502c";
					break;
				default:
					break;
				}
				break;

			case 2:
				_elmoShould = 15;
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_G(kernel).trigger_mode = KT_PARSE;
				wilbur_speech("402w501", 1);
				break;

			case 1:
				_digiName = "402p501";
				_newMode = KT_PARSE;
				_digiTrigger = 2;
				_elmoShould = 45;
				break;

			case 2:
				_G(flags)[V158] = 1;
				_elmoShould = 15;
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		}
	} else if (player_said("deed", "stolie") || player_said("deed", "elmo")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_G(kernel).trigger_mode = KT_PARSE;
			wilbur_speech(_G(flags)[V157] ? "402w902" : "402w502", 1);
			break;

		case 1:
			freeSeries2();
			freeDr2();
			_digiName = "402p902";
			_newMode = KT_PARSE;
			_digiTrigger = 2;
			_elmoShould = 45;
			break;

		case 2:
			_elmoShould = 41;
			_digiName = "402s007";
			_newMode = KT_PARSE;
			_digiTrigger = 3;
			_stolieShould = 40;
			break;

		case 3:
			_digiName = "402s008";
			_newMode = KT_PARSE;
			_digiTrigger = 4;
			freeStolie();
			freeDr2();
			_stolieShould = 46;
			kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			break;

		case 4:
			_stolieShould = 47;
			kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			_G(kernel).trigger_mode = KT_PARSE;
			wilbur_speech("402w903", 5);
			break;

		case 5:
			_digiName = "402s009";
			_newMode = KT_PARSE;
			_digiTrigger = 6;
			_stolieShould = 45;
			break;

		case 6:
			_stolieShould = 47;
			kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			_G(kernel).trigger_mode = KT_PARSE;
			wilbur_speech("402w904", 7);
			break;

		case 7:
			ws_walk(337, 375, nullptr, 15, 2);
			break;

		default:
			break;
		}
	} else if (player_said("conv84")) {
		conv84();
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room402::conv84() {
	int who = conv_whos_talking();
	int entry = conv_current_entry();
	int node = conv_current_node();
	const char *sound = conv_sound_to_play();
	_G(kernel).trigger_mode = KT_PARSE;

	switch (_G(kernel).trigger) {
	case 6:
		if (who == 0) {
			_G(kernel).trigger_mode = KT_DAEMON;

			if (node == 3 && entry == 1) {
				_stolieShould = 47;
				kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			} else if (node == 1 && entry == 1) {
				_stolieShould = 39;
				kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			} else if (node == 9 && entry == 0) {
				_stolieShould = 39;
				kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
				term_message("--------------------------------");
			} else if (node == 9 && entry == 2) {
				_stolieShould = 15;
				kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
				term_message("HHHHHHHHHHHHHHHHHH");
			} else if ((node == 1 && entry == 6) ||
					(node == 2 && entry == 0) ||
					(node == 3 && entry == 0) ||
					(node == 4 && entry == 0) ||
					(node == 5 && entry == 0) ||
					(node == 6 && entry == 1) ||
					(node == 6 && entry == 3) ||
					(node == 8 && entry == 1) ||
					(node == 9 && entry == 1)) {
				_elmoShould = 47;
			} else {
				term_message("xxxxxxxxxxxxxxxxxxxxx");
				_stolieShould = 47;
				kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
			}

			conv_resume(conv_get_handle());
		}
		break;

	case 21:
		_elmoShould = 47;

		if (_elmoMode == 45)
			_G(kernel).trigger_mode = KT_DAEMON;
		break;

	default:
		if (sound) {
			if (who <= 0) {
				_digiName = sound;
				_newMode = KT_PARSE;
				_digiTrigger = 6;

				if (node == 1 && entry == 0)
					kernel_timing_trigger(180, 21);

				if ((node == 4 && entry == 0) ||
						(node == 1 && entry == 6) ||
						(node == 9 && entry == 1)) {
					if (!(node == 9 && entry == 1))
						_stolieShould = 7;

					if (_stolieMode == 45)
						_G(kernel).trigger_mode = KT_DAEMON;
				}

				if (node == 3 && entry == 1) {
					_stolieShould = 40;
					_G(kernel).trigger_mode = KT_DAEMON;
					kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
				} else if (node == 6 && entry == 2) {
					_stolieShould = 40;
				} else if ((node == 1 && entry == 1) || (node == 9 && entry == 0)) {
					_stolieShould = 40;

					if (_stolieMode == 45)
						_G(kernel).trigger_mode = KT_DAEMON;
				} else if (node == 9 && entry == 2) {
					_stolieShould = 40;
					_G(kernel).trigger_mode = KT_DAEMON;
					kernel_trigger_dispatch_now(kCHANGE_STOLIE_ANIMATION);
				} else if ((node == 1 && entry == 6) ||
						(node == 2 && entry == 0) ||
						(node == 3 && entry == 0) ||
						(node == 4 && entry == 0) ||
						(node == 5 && entry == 0) ||
						(node == 6 && entry == 1) ||
						(node == 6 && entry == 3) ||
						(node == 8 && entry == 1) ||
						(node == 9 && entry == 1)) {
					_elmoShould = 45;

					if (_elmoMode == 45) {
						_G(kernel).trigger_mode = KT_DAEMON;
						kernel_trigger_dispatch_now(kCHANGE_ELMO_ANIMATION);
					}
				} else {
					_stolieShould = 45;

					if (_stolieMode == 45)
						_G(kernel).trigger_mode = KT_DAEMON;
				}
			} else if (who == 1) {
				if ((node == 1 && entry == 1) || (node == 1 && entry == 3)) {
					_stolieShould = 39;

					if (_stolieMode == 45)
						_G(kernel).trigger_mode = KT_DAEMON;
				}

				_G(kernel).trigger_mode = KT_DAEMON;
				wilbur_speech(sound, kRESUME_CONVERSATION);
			}
		}
		break;
	}
}

void Room402::playDigiName() {
	if (_digiName) {
		_G(kernel).trigger_mode = _newMode;
		digi_play(_digiName, 1, 255, _digiTrigger);
		_digiName = nullptr;
	}
}

void Room402::playRandom1() {
	static const char *NAMES[5] = {
		"402s010f", "402s010g", "402s010h", "402s010a", "402s010d"
	};

	if (!digi_play_state(1))
		digi_play(NAMES[imath_ranged_rand(0, 4)], 1, 140, -1);
}

void Room402::playRandom2() {
	digi_play(Common::String::format("402p903%c", 'a' + imath_ranged_rand(0, 5)).c_str(), 2, 140);
}

void Room402::freeStolie() {
	if (_stolieSet) {
		_stolie.terminate();
		_stolieSet = false;
	}
}

void Room402::freeSeries2() {
	if (_series2Set) {
		_series2.terminate();
		_series2Set = false;
	}
}

void Room402::loadDr1() {
	if (_dr01 == -1) {
		_dr01 = series_load("402dr01");
		_dr01s = series_load("402dr01s");
	}
}

void Room402::freeDr1() {
	if (_dr01 != -1) {
		series_unload(_dr01);
		series_unload(_dr01);
		_dr01 = _dr01s = -1;
	}
}

void Room402::loadDr2() {
	if (_dr02 == -1) {
		_dr02 = series_load("402dr02");
		_dr02s = series_load("402dr02s");
	}
}

void Room402::freeDr2() {
	if (_dr02 != -1) {
		series_unload(_dr02);
		series_unload(_dr02);
		_dr02 = _dr02s = -1;
	}
}

void Room402::loadDr8() {
	if (_dr08 == -1) {
		_dr08 = series_load("402dr08");
		_dr08s = series_load("402dr08s");
	}
}

void Room402::freeDr8() {
	if (_dr08 != -1) {
		series_unload(_dr08);
		series_unload(_dr08);
		_dr08 = _dr08s = -1;
	}
}

void Room402::loadPe1() {
	if (_pe01 == -1) {
		_pe01 = series_load("402pe01");
		_pe01s = series_load("402pe01s");
	}
}

void Room402::freePe1() {
	if (_pe01 != -1) {
		series_unload(_pe01);
		series_unload(_pe01s);
		_pe01 = _pe01s = -1;
	}
}

void Room402::loadPe2() {
	if (_pe02 == -1) {
		_pe02 = series_load("402pe02");
		_pe02s = series_load("402pe02s");
	}
}

void Room402::freePe2() {
	if (_pe02 != -1) {
		series_unload(_pe02);
		series_unload(_pe02s);
		_pe02 = _pe02s = -1;
	}
}

void Room402::loadPe3() {
	if (_pe03 == -1) {
		_pe03 = series_load("402pe03");
		_pe03s = series_load("402pe03s");
	}
}

void Room402::freePe3() {
	if (_pe03 != -1) {
		series_unload(_pe03);
		series_unload(_pe03s);
		_pe03 = _pe03s = -1;
	}
}

void Room402::loadPe4() {
	if (_pe04 == -1) {
		_pe04 = series_load("402pe04");
		_pe04s = series_load("402pe04s");
	}
}

void Room402::freePe4() {
	if (_pe04 != -1) {
		series_unload(_pe04);
		series_unload(_pe04s);
		_pe04 = _pe04s = -1;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
