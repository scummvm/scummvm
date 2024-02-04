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

#include "m4/burger/rooms/section1/room134_135.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_ODIE_ANIMATION = 32
};

static const char *SAID1[][4] = {
	{ "OLD BRIDGE",       "134W001", "134W002", "134W002" },
	{ "NEW BRIDGE",       "134W003", "134W002", "134W002" },
	{ "WATER",            "134W004", "134W002", "134W005" },
	{ "BANK",             "134W004", "134W002", "134W005" },
	{ "WRECKED TRUCK",    nullptr,   "134W002", "134w002" },
	{ "FORK IN THE ROAD", nullptr,   "134W002", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const char *SAID2[][4] = {
	{ "ODIE",             "135W001", "135W002", "135W002" },
	{ "OLD BRIDGE",       "135W005", "135W002", "135W002" },
	{ "NEW BRIDGE",       "135W006", "135W002", "135W002" },
	{ "WATER",            "135W007", "135W002", "135W008" },
	{ "FORK IN THE ROAD", nullptr,   "135W002", nullptr },
	{ "MAIN STREET",      nullptr,   "135W002", nullptr },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesStreamBreak STREAMS1[] = {
	STREAM_BREAK_END
};

static const seriesStreamBreak STREAMS2[] = {
	{  6, nullptr,   2, 255,  5, 0, nullptr, 0 },
	{ 10, "100_022", 2, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 0, 3, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 3, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 3, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 3, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 3, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0,  8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 8,  8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 8,  8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 8,  8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 8,  8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 8,  8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 14, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room134_135::init() {
	_volume = 255;
	_val2 = 0;
	_odieLoaded = false;
	_val4 = 1000;
	_val5 = 1;
	_val6 = 1;

	digi_preload("135_005");
	digi_preload("135_002");
	digi_preload("135_004");

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 101:
		if (_G(flags)[V000] == 1002)
			_G(flags)[V039] = 1;

		_G(wilbur_should) = 37;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 133:
	case 136:
		if (_G(flags)[V000] == 1002)
			_G(flags)[V039] = 1;

		_G(wilbur_should) = 38;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	case 135:
		player_set_commands_allowed(false);
		kernel_trigger_dispatch_now(24);
		_val6 = 22;
		series_play_with_breaks(PLAY1, "134bu01");
		ws_demand_location(340, 250);
		ws_demand_facing(11);
		break;

	default:
		ws_demand_location(320, 271);
		ws_demand_facing(5);
		break;
	}

	if (_G(flags)[V000] == 1002) {
		hotspot_set_active("odie", false);
		hotspot_set_active("baitbox", false);
	} else {
		hotspot_set_active("wrecked truck", false);
	}

	_G(global_sound_room) = 135;
	_series1 = series_play("135cw01", 3840, 0, -1, 10, -1, 100, 0, 0, 0, 3);

	if (!_G(flags)[V039])
		kernel_trigger_dispatch_now(32);

	digi_preload("135_001");
	digi_play_loop("135_001", 3, 90, -1);

	if (_G(flags)[V000] == 1002) {
		_val7 = 27;
		_val8 = 27;
		kernel_trigger_dispatch_now(34);
		kernel_timing_trigger(imath_ranged_rand(200, 500), 21);
	}
}

void Room134_135::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 1:
		// Burl enters town normally
		_val2 = 1;
		_val6 = 21;

		if (_play1)
			terminateMachineAndNull(_play1);
		terminateMachineAndNull(_series1);
		terminateMachineAndNull(_series2);
		terminateMachineAndNull(_series3);
		Section1::updateWalker(340, 250, 7, 4);
		break;

	case 2:
		// Burl is entering town
		kernel_trigger_dispatch_now(_G(flags)[kRoadOpened] ? 26 : 1);
		break;

	case 3:
		_volume -= 10;
		if (_volume > 0) {
			digi_change_volume(2, _volume);
			kernel_timing_trigger(3, 3);
		} else {
			digi_stop(1);
			digi_unload_stream_breaks(STREAMS1);
			digi_stop(2);
			digi_unload("100_013");
			player_set_commands_allowed(true);
			_volume = 255;
		}
		break;

	case 4:
		_val2 = 0;
		_series1 = series_play("135cw01", 3840, 0, -1, 10, -1, 100, 0, 0, 0, 3);
		_val5 = 1;
		_val6 = 1;
		kernel_trigger_dispatch_now(32);

		_val7 = 27;
		_val8 = 27;
		kernel_trigger_dispatch_now(34);
		kernel_timing_trigger(imath_ranged_rand(200, 500), 21);
		_G(walker).reset_walker_sprites();
		series_stream_with_breaks(STREAMS1, "135dt01", 5, 512, 3);
		break;

	case 5:
		digi_change_volume(2, 0);
		break;

	case 6:
		pal_fade_set_start(0);
		_val2 = 1;
		ws_hide_walker();
		_val6 = 21;

		if (_play1)
			terminateMachineAndNull(_play1);

		terminateMachineAndNull(_series1);
		terminateMachineAndNull(_series2);
		terminateMachineAndNull(_series3);
		gr_backup_palette();
		digi_preload("100_022");
		digi_preload_stream_breaks(STREAMS2);
		kernel_timing_trigger(70, 7);
		break;

	case 7:
		series_stream_with_breaks(STREAMS2, "120dt01", 9, 0, 1008);
		kernel_timing_trigger(1, 8);
		break;

	case 8:
		pal_fade_init(15, 255, 100, 0, -1);
		break;

	case 9:
		ws_unhide_walker();
		break;

	case 10:
		ws_unhide_walker();
		conv_resume_curr();
		break;

	case 11:
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 12:
		player_set_commands_allowed(true);
		break;

	case 14:
		_series5 = series_play("135od07", 0xa00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_series6 = series_play("135od07s", 0xa00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		ws_hide_walker();
		series_play("135wi01", 1536, 0, 15, 6, 0, 100, 0, 0, 0, 14);
		series_play("135wi01s", 1536, 0, -1, 6, 0, 100, 0, 0, 0, 14);
		break;

	case 15:
		_play3 = series_play("135wi01", 1536, 0, -1, 600, -1, 100, 0, 0, 14, 14);
		_play4 = series_play("135wi01s", 1536, 0, -1, 600, -1, 100, 0, 0, 14, 14);
		conv_resume();
		break;

	case 16:
		terminateMachineAndNull(_series5);
		terminateMachineAndNull(_series6);
		series_play("135od06", 0xa00, 0, 17, 6, 0, 100, 0, 0, 0, 4);
		series_play("135od06s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 4);
		break;

	case 17:
		terminateMachineAndNull(_play3);
		terminateMachineAndNull(_play4);
		_val6 = 1;
		_val5 = 1;
		_G(flags)[V001]--;
		series_play("135od06", 0xa00, 0, 32, 6, 0, 100, 0, 0, 5, 13);
		series_play("135od06s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 5, 13);
		series_play("135wi01", 1536, 0, 9, 6, 0, 100, 0, 0, 14, 15);
		series_play("135wi01s", 1536, 0, -1, 6, 0, 100, 0, 0, 14, 15);
		conv_resume();
		break;

	case 18:
		series_play("135od17", 0xa00, 0, 19, 6, 0, 100, 0, 0, 0, 1);
		series_play("135od17s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 1);
		break;

	case 19:
		_series5 = series_play("135od17", 1536, 0, -1, 600, -1, 100, 0, 0, 1, 1);
		_series6 = series_play("135od17s", 1536, 0, -1, 600, -1, 100, 0, 0, 1, 1);
		ws_hide_walker();
		series_play("135wi01", 1536, 2, 20, 6, 0, 100, 0, 0, 14, 15);
		series_play("135wi01s", 1536, 2, -1, 6, 0, 100, 0, 0, 14, 15);
		break;

	case 20:
		terminateMachineAndNull(_series5);
		terminateMachineAndNull(_series6);
		_val5 = 1;
		_val6 = 1;
		inv_give_to_player("broken puz dispenser");
		series_play("135od17", 0xa00, 0, 32, 6, 0, 100, 0, 0, 2, 6);
		series_play("135od17s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 2, 6);
		series_play("135wi01", 1536, 2, 10, 6, 0, 100, 0, 0, 0, 14);
		series_play("135wi01s", 1536, 2, -1, 6, 0, 100, 0, 0, 0, 14);
		break;

	case 21:
		if (!_val2) {
			_xp = imath_ranged_rand(0, 140);
			_yp = imath_ranged_rand(0, 30);
			_play1 = series_play("135bf01", 3840, 0, 22, 6, 0, 100, _xp - 60, _yp, 0, 4);
		}
		break;

	case 22:
		kernel_timing_trigger(imath_ranged_rand(300, 700), 21);
		_play1 = series_play("135bf01", 3840, 0, 23, 6, 0, 100, _xp - 60, _yp, 5, 9);

		if (player_commands_allowed())
			digi_play("135_005", 2, 80, -1);
		break;

	case 23:
		_play1 = 0;
		break;

	case 24:
		_play2 = series_play("134bf02", 3840, 0, 25, 6, 0, 100, 0, 0, 0, 8);

		if (!digi_play_state(2))
			digi_play("135_002", 2, 150, -1);
		break;

	case 25:
		if (player_commands_allowed())
			digi_stop(2);

		_play2 = series_play("134bf02", 3840, 0, 24, imath_ranged_rand(200, 500), 0, 100, 0, 0, 0, 0);
		break;

	case 26:
		// Burl crashing truck off of bridge
		digi_preload("135_006");
		_series4 = series_play("135dt02", 3840, 0, 27, 6, 0, 100, 0, 0, 0, 13);
		break;

	case 27:
		digi_play("135_006", 1, 255, -1);
		_series4 = series_play("135dt02", 3840, 0, 28, 6, 0, 100, 0, 0, 14, 25);
		break;

	case 28:
		_series4 = series_play("135dt02", 3840, 0, 6, 6, 0, 100, 0, 0, 26, 36);
		break;

	case 29:
	case 31:
		_val8 = 27;
		_val6 = 1;

		if (_flag1)
			player_set_commands_allowed(true);
		break;

	case 30:
		player_set_commands_allowed(false);
		_val9 = 33;
		break;

	case kCHANGE_ODIE_ANIMATION:
		switch (_val5) {
		case 1:
			switch (_val6) {
			case 1:
				if (imath_ranged_rand(5, 20) < _val4) {
					_val4 = 0;
					_frame = imath_ranged_rand(0, 5);
				}

				series_play("135od01", 0xa00, 0, kCHANGE_ODIE_ANIMATION, 60, 0, 100, 0, 0, _frame, _frame);
				series_play("135od01s", 0xa00, 0, -1, 60, 0, 100, 0, 0, _frame, _frame);
				break;

			case 2:
				frame = imath_ranged_rand(0, 8);
				series_play("135od03", 0xa00, 0, kCHANGE_ODIE_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				series_play("135od03s", 0xa00, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				loadDigi();
				break;

			case 3:
				_val6 = 1;
				series_play_with_breaks(PLAY2, "135od04", 0xa00, kCHANGE_ODIE_ANIMATION, 3, 6, 100, 0, 0);
				break;

			case 4:
				_val6 = 1;
				series_play_with_breaks(PLAY3, "135od05", 0xa00, kCHANGE_ODIE_ANIMATION, 3, 6, 100, 0, 0);
				break;

			case 5:
			case 7:
				_val5 = 5;
				series_play("135od08", 0xa00, 0, kCHANGE_ODIE_ANIMATION, 6, 0, 100, 0, 0, 0, 11);
				series_play("135od08s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 11);
				break;

			case 8:
				_val6 = 11;
				_val5 = 11;
				series_play("135od06", 0xa00, 0, kCHANGE_ODIE_ANIMATION, 6, 0, 100, 0, 0, 0, 11);
				series_play("135od06s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 11);
				break;

			case 9:
				kernel_trigger_dispatch_now(14);
				break;

			case 10:
				_val6 = 1;
				series_play("135od22", 0xa00, 0, 32, 6, 0, 100, 0, 0, 0, 6);
				series_play("135od22s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 6);
				break;

			case 11:
			case 12:
				_val5 = 11;
				series_play("135od04", 0xa00, 0, kCHANGE_ODIE_ANIMATION, 6, 0, 100, 0, 0, 0, 3);
				series_play("135od04s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 3);
				break;

			case 22:
				_val6 = 23;
				series_play("134od23", 0xa00, 0, 32, 6, 0, 100, 0, 0, 0, -1);
				series_play("134od23s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				break;

			case 23:
				_val6 = 24;
				kernel_timing_trigger(300, 32);
				break;

			case 24:
				_val6 = 25;
				series_play("134od24", 0xa00, 0, 32, 6, 0, 100, 0, 0, 0, 36);
				series_play("134od24s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 36);
				break;

			case 25:
				terminateMachineAndNull(_play2);
				_val6 = 26;
				series_play("134od24", 0xa00, 0, 32, 6, 0, 100, 0, 0, 37, -1);
				series_play("134od24s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 37, -1);
				break;

			case 26:
				_G(flags)[V039] = 1;
				_val6 = 25;
				series_play("134od25", 0xa00, 0, 12, 6, 0, 100, 0, 0, 0, -1);
				series_play("134od25s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, -1);
				break;

			default:
				break;
			}
			break;

		case 2:
			if (_val6 == 2) {
				frame = imath_ranged_rand(0, 5);
				series_play("135od05", 0xa00, 0, 32, 4, 0, 100, 0, 0, frame, frame);
				series_play("135od05s", 0xa00, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				loadDigi();
			} else {
				_val5 = 1;
				kernel_trigger_dispatch_now(32);
			}
			break;

		case 5:
			switch (_val6) {
			case 5:
				series_play("135od09", 0xa00, 0, 32, 20, 0, 100, 0, 0, 0, 0);
				series_play("135od09s", 0xa00, 0, -1, 20, 0, 100, 0, 0, 0, 0);
				break;

			case 7:
				frame = imath_ranged_rand(0, 5);
				series_play("135od10", 0xa00, 0, 32, 4, 0, 100, 0, 0, frame, frame);
				series_play("135od10s", 0xa00, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				loadDigi();
				break;

			default:
				_val5 = 6;
				series_play("135od09", 0xa00, 0, 32, 10, 0, 100, 0, 0, 1, 3);
				series_play("135od09s", 0xa00, 0, -1, 10, 0, 100, 0, 0, 1, 3);
				break;
			}
			break;

		case 6:
			_val5 = 1;
			series_play("135od11", 0xa00, 0, 32, 10, 0, 100, 0, 0, 0, 18);
			series_play("135od11s", 0xa00, 0, -1, 10, 0, 100, 0, 0, 0, 18);
			break;

		case 11:
			switch (_val6) {
			case 11:
				series_play("135od12", 0xa00, 0, 32, 20, 0, 100, 0, 0, 0, 0);
				series_play("135od12s", 0xa00, 0, -1, 20, 0, 100, 0, 0, 0, 0);
				break;

			case 12:
				frame = imath_ranged_rand(0, 5);
				series_play("135od12", 0xa00, 0, 32, 4, 0, 100, 0, 0, frame, frame);
				series_play("135od12s", 0xa00, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				loadDigi();
				break;

			case 14:
				_val6 = 15;
				series_play("135od07", 0xa00, 0, 32, 6, 0, 100, 0, 0, 2, 4);
				series_play("135od07s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 2, 4);
				break;

			case 15:
				_val6 = 16;
				_val5 = 16;
				series_play("135od13", 0xa00, 0, 32, 6, 0, 100, 0, 0, 1, 3);
				series_play("135od13s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 1, 3);

				if (_val11 == 0)
					inv_move_object("puz dispenser", 135);
				if (_val11 == 1)
					inv_move_object("broken puz dispenser", 135);

				_val9 = 33;
				break;

			default:
				_val5 = 1;
				series_play("135od04", 0xa00, 2, 32, 6, 0, 100, 0, 0, 0, 3);
				series_play("135od04s", 0xa00, 2, -1, 6, 0, 100, 0, 0, 0, 3);
				break;
			}
			break;

		case 16:
			if (_val6 == 16) {
				if (_val11 == 0) {
					series_play("135od18", 0xa00, 0, 32, 6, 0, 100, 0, 0, 0, 14);
					series_play("135od18s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 14);
				} else if (_val11 == 1) {
					series_play("135od14", 0xa00, 0, 32, 6, 0, 100, 0, 0, 0, 5);
					series_play("135od14s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 5);
				}
			} else {
				_val5 = 17;
				kernel_trigger_dispatch_now(32);
			}
			break;

		case 17:
			switch (_val6) {
			case 13:
				kernel_trigger_dispatch_now(18);
				break;

			case 17:
				series_play("135od20", 0xa00, 0, 32, 10, 0, 100, 0, 0, 0, 0);
				series_play("135od20s", 0xa00, 0, -1, 10, 0, 100, 0, 0, 0, 0);
				break;

			case 18:
				frame = imath_ranged_rand(0, 5);
				series_play("135od20", 0xa00, 0, 32, 4, 0, 100, 0, 0, frame, frame);
				series_play("135od20s", 0xa00, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				loadDigi();
				break;

			default:
				_val5 = 1;
				series_play("135od21", 0xa00, 0, 32, 6, 0, 100, 0, 0, 0, 2);
				series_play("135od21s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 2);
				break;
			}
			break;

		case 19:
			switch (_val6) {
			case 19:
				series_play("135od16", 0xa00, 0, 32, 10, 0, 100, 0, 0, 0, 0);
				series_play("135od16s", 0xa00, 0, -1, 10, 0, 100, 0, 0, 0, 0);
				break;

			case 20:
				frame = imath_ranged_rand(0, 4);
				series_play("135od16", 0xa00, 0, 32, 4, 0, 100, 0, 0, frame, frame);
				series_play("135od16s", 0xa00, 0, -1, 4, 0, 100, 0, 0, frame, frame);
				loadDigi();
				break;

			default:
				kernel_trigger_dispatch_now(18);
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 33:
		switch (_val10) {
		case 30:
			switch (_val9) {
			case 31:
				_val9 = 32;
				series_play("135wi01", 1536, 0, 33, 6, 0, 100, 0, 0, 0, 14);
				series_play("135wi01s", 1536, 0, -1, 6, 0, 100, 0, 0, 0, 14);
				break;
			case 32:
				conv_load_and_prepare("conv01", 29);

				if (player_said("puz dispenser", "odie")) {
					conv_export_value_curr(17, 0);
					_val11 = 0;
				} else if (player_said("broken puz dispenser", "odie")) {
					conv_export_value_curr(16, 0);
					_val11 = 1;
				} else {
					conv_export_value_curr(0, 0);
					_val11 = 2;
				}

				_val10 = 31;
				_val9 = 31;
				kernel_trigger_dispatch_now(33);
				conv_export_value_curr(inv_player_has("block of ice") ? 1 : 0, 1);
				conv_export_pointer_curr(&_G(flags)[V040], 3);
				_flag1 = true;
				conv_play_curr();
				_val8 = 28;
				break;

			case 34:
				_val9 = 35;
				ws_hide_walker();
				series_play("135wi01", 1536, 0, 33, 6, 0, 100, 0, 0, 16, 18);
				series_play("135wi01s", 1536, 0, -1, 6, 0, 100, 0, 0, 16, 18);
				break;

			case 35:
				conv_load_and_prepare("conv02", 30);
				conv_export_value_curr(inv_player_has("block of ice") ? 1 : 0, 0);
				conv_play_curr();
				_val10 = 34;
				_val9 = 34;
				kernel_trigger_dispatch_now(33);
				break;

			default:
				break;
			}
			break;

		case 31:
			switch (_val9) {
			case 31:
				series_play("135wi01", 1536, 0, 33, 15, 0, 100, 0, 0, 14, 14);
				series_play("135wi01s", 1536, 0, -1, 15, 0, 100, 0, 0, 14, 14);
				break;
			case 33:
				series_play("135wi01", 1536, 0, 9, 6, 0, 100, 0, 0, 15, 15);
				series_play("135wi01s", 1536, 0, -1, 6, 0, 100, 0, 0, 15, 15);
				break;
			case 36:
				series_play("135wi01", 1536, 2, 11, 6, 0, 100, 0, 0, 0, 14);
				series_play("135wi01s", 1536, 2, -1, 6, 0, 100, 0, 0, 0, 14);
				break;
			default:
				break;
			}
			break;

		case 34:
			switch (_val9) {
			case 33:
				series_play("135wi01", 1536, 2, 11, 6, 0, 100, 0, 0, 16, 18);
				series_play("135wi01s", 1536, 2, -1, 6, 0, 100, 0, 0, 16, 18);
				break;
			case 34:
				series_play("135wi01", 1536, 0, 33, 15, 0, 100, 0, 0, 18, 18);
				series_play("135wi01s", 1536, 0, -1, 15, 0, 100, 0, 0, 18, 18);
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 34:
		if (_val7 == 27) {
			switch (_val8) {
			case 27:
				if (imath_ranged_rand(1, 15) == 1) {
					_series2 = series_play("135bb01", 0xa00, 0, 34, 6, 0, 100, 0, 0, 0, 11);
					_series3 = series_play("135bb01s", 0xa00, 0, -1, 6, 0, 100, 0, 0, 0, 11);

					if (player_commands_allowed())
						digi_play("135_004", 2, 205, -1);
				} else {
					_series2 = series_play("135bb01", 0xa00, 0, 34, 30, 0, 100, 0, 0, 0, 0);
					_series3 = series_play("135bb01s", 0xa00, 0, -1, 30, 0, 100, 0, 0, 0, 0);
				}
				break;

			case 28:
				_series2 = series_play("135bb01", 0xa00, 0, 34, 30, 0, 100, 0, 0, 0, 0);
				_series3 = series_play("135bb01s", 0xa00, 0, -1, 30, 0, 100, 0, 0, 0, 0);
				break;

			default:
				break;
			}
		}
		break;

	case 37:
		player_set_commands_allowed(true);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 37:
			player_set_commands_allowed(true);
			player_first_walk(639, 373, 9, 579, 359, 9, true);
			break;

		case 38:
			player_set_commands_allowed(true);
			player_first_walk(0, 248, 3, 42, 260, 3, true);
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
			digi_play("100_013", 2, 255, -1);
			kernel_timing_trigger(240, 2);
		} else {
			kernel_timing_trigger(60, kBurlEntersTown);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room134_135::pre_parser() {
	if (player_said("fork in the road") && !player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(9);

	if (player_said("main street") && !player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(3);

	if (player_said("take", "baitbox") || player_said("gear", "baitbox"))
		player_hotspot_walk_override(308, 249, 10, -1);
}

void Room134_135::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said((_G(flags)[V000] == 1002) ? SAID1 : SAID2)) {
		// Already handled
	} else if (player_said("ENTER", "FORK IN THE ROAD") || player_said("gear", "fork in the road") ||
			(lookFlag && player_said("fork in the road"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1009);

	} else if (player_said("ENTER", "MAIN STREET") || player_said("gear", "main street") ||
			(lookFlag && player_said("main street"))) {
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1001);
	} else if (player_said("conv01")) {
		conv01();
	} else if (player_said("conv02")) {
		conv02();
	} else if (player_said("conv03")) {
		conv03();
	} else if (player_said("odie") && inv_player_has(_G(player).verb)) {
		ws_hide_walker();
		player_set_commands_allowed(false);
		loadOdie();
		_val9 = 31;
		_val10 = 30;
		kernel_trigger_dispatch_now(33);
	} else if (inv_player_has(_G(player).verb) && player_said_any("fork in the road", "main street")) {
		wilbur_speech("135w002");
	} else if (lookFlag && player_said("baitbox")) {
		wilbur_speech(_G(flags)[V038] ? "135w004" : "135w003");
	} else if (lookFlag && player_said("wrecked truck")) {
		wilbur_speech(_G(flags)[V052] ? "134w007" : "134w006");
	} else if (player_said("take", "baitbox") || player_said("gear", "baitbox")) {
		player_set_commands_allowed(false);
		loadOdie();
		_val9 = 34;
		_val10 = 30;
		kernel_trigger_dispatch_now(33);
	} else if (player_said("talk to", "odie")) {
		player_set_commands_allowed(false);
		loadOdie();
		_flag1 = true;
		conv_load_and_prepare("conv03", 31);
		conv_export_pointer_curr(&_G(flags)[V038], 1);
		conv_export_value_curr(_G(flags)[V001], 3);
		conv_play_curr();
	} else if (player_said("conv06")) {
		conv03();
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room134_135::conv01() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	switch (_G(kernel).trigger) {
	case 13:
		digi_play(conv_sound_to_play(), 1, 255, 35);
		break;

	case 35:
		if (who <= 0) {
			if (node == 14 || node == 12 || node == 13 || node == 4) {
				_val9 = 36;
				_flag1 = false;
			}

			if ((node == 7 && entry == 0) || (node == 2 && entry == 0)) {
				_val6 = 8;
			} else if (node == 2 && entry == 3) {
				_val6 = 13;
			} else if (!(node == 6 && !entry) && !(node == 2 && entry == 1) &&
					!(node == 2 && entry == 2)) {
				if (node == 6 && entry == 1)
					_val6 = 17;
				else
					_val6 = 1;
			}
		} else if (who == 1) {
			if ((node == 2 && entry == 0) || (node == 4) || (node == 10) ||
				(node == 6 && entry == 0) || (node == 2 && entry == 1) ||
				(node == 7 && entry == 0)) {
				// Do nothing
			} else {
				sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			}
		}

		conv_resume(conv_get_handle());
		break;

	default:
		if (conv_sound_to_play()) {
			if (who <= 0) {
				if ((node == 2 && entry == 2) || (node == 2 && entry == 3)) {
					digi_play(conv_sound_to_play(), 1, 255, 35);
				} else if ((node == 7 && entry == 1) || (node == 2 && entry == 1)) {
					digi_play(conv_sound_to_play(), 1, 255, 35);
				} else if (node == 6 && entry == 1) {
					kernel_timing_trigger(120, 13);
				} else if ((node == 7 && entry == 0) || (node == 2 && entry == 0)) {
					_val6 = 12;
					_name1 = conv_sound_to_play();
				} else if (node == 6 && entry == 0) {
					_val6 = 14;
					digi_play(conv_sound_to_play(), 1, 255, 35);
				} else if (node == 9 && entry == 1) {
					_val6 = 4;
					digi_play(conv_sound_to_play(), 1, 255, 35);
				} else {
					_val6 = (node == 9 && entry == 0) ? 7 : 2;
					_name1 = conv_sound_to_play();
				}
			} else if (who == 1) {
				if ((node == 2 && entry == 0) || (node == 4) || (node == 10) ||
						(node == 6 && entry == 0) || (node == 7 && entry == 0)) {
					_val6 = 11;
					digi_play(conv_sound_to_play(), 1, 255, 35);
				} else if ((node == 7 && entry == 1) || (node == 2 && entry == 1)) {
					_val6 = 14;
				} else {
					sendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);
				}

				digi_play(conv_sound_to_play(), 1, 255, 35);
			}
		} else {
			conv_resume();
		}
	}
}

void Room134_135::conv02() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();

	if (_G(kernel).trigger == 35) {
		if (who == 0) {
			_val6 = 1;
			conv_resume();
		}
	} else if (conv_sound_to_play()) {
		if (who == 0) {
			_name1 = conv_sound_to_play();
			_val6 = 12;
		}
	} else {
		conv_resume();
	}
}

void Room134_135::conv03() {
	_G(kernel).trigger_mode = KT_PARSE;

	if (_G(kernel).trigger == 35) {
		int who = conv_whos_talking();
		if (who <= 0) {
			if (conv_current_node() == 8 && !conv_current_entry()) {
				digi_preload("03p1001");
				_val6 = 9;
			} else {
				_val6 = 1;
				conv_resume();
			}
		} else if (who == 1) {
			sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			conv_resume();
		}
	} else if (conv_sound_to_play()) {
		int who = conv_whos_talking();
		if (who <= 0) {
			if (conv_current_node() == 8 && conv_current_entry()) {
				kernel_timing_trigger(1, 35);
			} else {
				_name1 = conv_sound_to_play();
				_val6 = 2;
			}
		} else if (who == 1) {
			if (conv_current_node() == 1 || conv_current_node() == 2)
				_val6 = 3;

			if (conv_current_node() == 9 && !conv_current_entry()) {
				digi_preload("03p1001");
				_val6 = 9;
				_G(kernel).trigger_mode = KT_DAEMON;
				digi_play(conv_sound_to_play(), 1, 255, 16);
			} else {
				sendWSMessage(0x140000, 0, _G(my_walker), 0, 0, 1);
				digi_play(conv_sound_to_play(), 1, 255, 35);
			}
		}
	} else {
		conv_resume();
	}
}

void Room134_135::loadOdie() {
	static const char *NAMES[30] = {
		"135od05", "135od05s", "135od06", "135od06s", "135od04",
		"135od04s", "135od08", "135od08s", "135od09", "135od09s",
		"135od10", "135od10s", "135od11", "135od11s", "135od12",
		"135od12s", "135od13", "135od13s", "135od14", "135od14s",
		"135od16", "135od16s", "135od17", "135od17s", "135od18",
		"135od18s", "135od20", "135od20s", "135od21", "135od21s"
	};

	if (!_odieLoaded) {
		_odieLoaded = true;

		for (int i = 0; i < 30; ++i)
			series_load(NAMES[i], -1);
	}
}

void Room134_135::loadDigi() {
	if (_name1) {
		_G(kernel).trigger_mode = KT_PARSE;
		digi_play(_name1, 1, 255, 35);
		_name1 = nullptr;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
