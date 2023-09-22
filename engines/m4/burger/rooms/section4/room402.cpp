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
	term_message(_G(visited_room) ? " been here before" : " NOT been here before");
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
	_series1Set = _series2Set = false;

	pal_cycle_init(114, 127, 6);
	digi_preload("400_001");

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
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

		if (_G(game).previous_room != RESTORING_GAME) {
			_G(wilbur_should) = 5;
			kernel_timing_trigger(120, gCHANGE_WILBUR_ANIMATION);
		}
	} else {
		_G(flags)[V298] = 1;
		static const char *NAMES[11] = {
			"402p903a", "402p903b", "402p903c", "402p903d", "402p903e", "402p903f",
			"402s010f", "402s010g", "402s010h", "402s010a", "402s010d"
		};
		for (int i = 0; i < 11; ++i)
			digi_preload(NAMES[i]);

		loadPe1();
		_val5 = 15;
		_val3 = 15;
		kernel_trigger_dispatch_now(3);

		if (_G(visited_room)) {
			if (_G(game).previous_room == RESTORING_GAME) {
				_val2 = 15;
			} else {
				_G(wilbur_should) = 5;
				kernel_timing_trigger(120, gCHANGE_WILBUR_ANIMATION);
				_val2 = 15;
			}
		} else {
			_val2 = 15;

			if (_G(game).previous_room != RESTORING_GAME) {
				digi_preload_stream_breaks(SERIES1);
				digi_preload_stream_breaks(SERIES2);
				series_load("402dr06");
				series_load("402dr06s");
				digi_preload("402p901");
				series_load("402pe4n");
				series_load("402pe4ns");

				_val2 = 16;
				_G(wilbur_should) = 2;
				kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
			}
		}

		_val6 = 15;
		kernel_trigger_dispatch_now(2);
	}

	digi_play_loop("400_001", 3, 175);
}

void Room402::daemon() {
	// TODO
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
		Section4::poof(4006);
	} else if (player_said("poof ") || player_said("gear", "disc ")) {
		Section4::poof(4005);
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
			_flag1 = true;
			_val1 = 2;
			freeSeries1();
			_val2 = 45;
			break;

		case 2:
			_val2 = 15;
			kernel_trigger_dispatch_now(2);
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
			_val3 = 15;
			kernel_trigger_dispatch_now(1);
			break;

		default:
			break;
		}
	} else if (player_said("TALK TO", "stolie")) {
		player_set_commands_allowed(false);
		conv_load_and_prepare("conv84", 1);
	} else if (player_said("TALK TO", "elmo")) {
		if (_G(flags)[V158]) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_G(kernel).trigger_mode = KT_PARSE;
				wilbur_speech("402w501");
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
				_val3 = 15;
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
				wilbur_speech("402w501");
				break;

			case 1:
				_digiName = "402p501";
				_flag1 = true;
				_val1 = 2;
				_val3 = 45;
				break;

			case 2:
				_G(flags)[V158] = 1;
				_val3 = 15;
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
			wilbur_speech(_G(flags)[V157] ? "402w902" : "402w502");
			break;

		case 1:
			freeSeries2();
			freeDr2();
			_digiName = "402p902";
			_flag1 = true;
			_val1 = 2;
			_val3 = 45;
			break;

		case 2:
			_val3 = 41;
			_digiName = "402s007";
			_flag1 = true;
			_val1 = 3;
			_val2 = 40;
			break;

		case 3:
			_digiName = "402s008";
			_flag1 = true;
			_val1 = 4;
			freeSeries1();
			freeDr2();
			_val2 = 46;
			kernel_trigger_dispatch_now(2);
			break;

		case 4:
			_val2 = 47;
			kernel_trigger_dispatch_now(2);
			_G(kernel).trigger_mode = KT_PARSE;
			wilbur_speech("402w903");
			break;

		case 5:
			_digiName = "402s009";
			_flag1 = true;
			_val1 = 6;
			_val2 = 45;
			break;

		case 6:
			_val2 = 47;
			kernel_trigger_dispatch_now(2);
			_G(kernel).trigger_mode = KT_PARSE;
			wilbur_speech("402w904");
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
	// TODO
}

void Room402::freeSeries1() {
	if (_series1Set) {
		_series1.terminate();
		_series1Set = false;
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
		_pe03 = series_load("403pe03");
		_pe03s = series_load("403pe03s");
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
		_pe04 = series_load("404pe04");
		_pe04s = series_load("404pe04s");
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
