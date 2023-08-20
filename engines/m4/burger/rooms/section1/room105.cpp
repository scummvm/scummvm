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

#include "m4/burger/rooms/section1/room105.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "TOWN RECORDS",     nullptr,   nullptr,   nullptr   },
	{ "MAYOR'S OFFICE",   "105W002", nullptr,   nullptr   },
	{ "COMPLAINTS DEPT.", "105W003", nullptr,   nullptr   },
	{ "CHAIR",            nullptr,   "105W004", "105W005" },
	{ "PETUNIA",          "105W006", "105W007", "105W007" },
	{ "PICTURE",          "105W008", "105W004", "105W009" }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, 5, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 6, -1, 0, 1, 255, 28, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 21, 31, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 32, 37, "105_009", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{ 0, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 0, 8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 5, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 8, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 6, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY5[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY6[] = {
	{ 10, 17, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 28, 29, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, 29, nullptr, 1, 255, 20, 0, 0, nullptr, 0 },
	{ 30, 32, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY7[] = {
	{ 32, 30, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 29, 28, nullptr, 1, 255, 45, 0, 0, nullptr, 0 },
	{ 19, 23, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY8[] = {
	{ 18, 23, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 24, 28, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY9[] = {
	{  0,  9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 16, 23, nullptr,   1, 255, 45, 0, 0, nullptr, 0 },
	{ 24, 28, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY10[] = {
	{ 0,  6, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 7, -1, nullptr,   1, 255, 27, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY11[] = {
	{ 0,  12, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY12[] = {
	{ 0, -1, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY13[] = {
	{  0,  3, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  4, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY14[] = {
	{  0,  6, "105_014", 2, 255, -1, 0, 0, nullptr, 0 },
	{  7, 12, nullptr,   1, 255, 22, 0, 0, nullptr, 0 },
	{ 13, 23, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 24, 45, "105_004", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 46, -1, nullptr,   1, 255, 24, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY15[] = {
	{ 0, 3, nullptr, 1, 255, -1, 0,  0, nullptr, 0 },
	{ 3, 3, nullptr, 1, 255, -1, 0, 12, nullptr, 0 },
	{ 4, 4, nullptr, 1, 255, -1, 0, 10, nullptr, 0 },
	{ 3, 0, nullptr, 1, 255, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY16[] = {
	{  0,  2, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  3,  6, nullptr,   1, 255, 23, 0, 0, nullptr, 0 },
	{  7, 11, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, "105_015", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, 26, "105_008", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 27, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY17[] = {
	{  0,  2, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  3,  6, nullptr,   1, 255, 23, 0, 0, nullptr, 0 },
	{  7, 11, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, "105_016", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, 26, "105_008", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 27, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY18[] = {
	{  0,  2, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  3,  6, nullptr,   1, 255, 23, 0, 0, nullptr, 0 },
	{  7, 11, "105_010", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 12, 12, "105_017", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 13, 26, "105_008", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 27, -1, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY19[] = {
	{ 0, -1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY20[] = {
	{ 27, 30, "105_003", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY21[] = {
	{ 0, 1, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ 2, 2, nullptr, 1, 255,  9, 0, 0, nullptr, 0 },
	{ 3, 0, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY22[] = {
	{ 0, 7, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 8, 8, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 9, 9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 8, 8, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 0, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY23[] = {
	{ 8, 14, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY24[] = {
	{ 0, 13, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY25[] = {
	{ 14, 17, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY26[] = {
	{ 0, 2, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY27[] = {
	{  3,  9, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 10, 38, "100_023", 2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY28[] = {
	{  0,  3, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  6,  6, nullptr,   1, 255, 35, 0, 0, nullptr, 0 },
	{  7, 13, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 14, 19, "105_006", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 19,  6, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{  5,  5, nullptr,   2, 255, 36, 0, 0, nullptr, 0 },
	{  4,  1, nullptr,   2, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY29[] = {
	{ 0, 5, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 6, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 5, 5, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 6, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 0, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY30[] = {
	{ 7, 17, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY31[] = {
	{ 0, 6, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 6, 6, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 7, 7, "105_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 3, 0, 0, 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY32[] = {
	{  8, 14, nullptr, 1, 255, -1, 0, 0, nullptr, 0 },
	{ -1, -1, nullptr, 0,   0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room105::init() {
	_G(player).walker_in_this_scene = true;
	_val1 = 1;
	_val2 = 0;
	digi_preload("105_001");

	if (_G(flags)[V112]) {
		series_play("105do01", 0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
		_val3 = 59;
		_val4 = 59;
		kernel_trigger_dispatch_now(1);

	} else {
		hotspot_set_active("Angus", false);
	}

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		break;

	case 104:
		ws_demand_location(310, 353, 11);
		_G(roomVal1) = 62;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	default:
		ws_demand_location(320, 271, 5);
		break;
	}

	digi_play_loop("105_001", 3, 180);
}

void Room105::daemon() {
}

void Room105::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("gear", "town records") && !_G(flags)[V112])
		player_hotspot_walk_override(186, 263, 10);

	if (player_said("gear", "mayor's office"))
		player_hotspot_walk_override(341, 216, 11);

	if (player_said("GEAR", "complaints dept."))
		player_hotspot_walk_override(415, 254, 2);
}

void Room105::parser() {
	bool lookFlag = player_said("look") || player_said("look at");
	_G(kernel).trigger_mode = KT_PARSE;

	if (player_said("conv12")) {
		conv12();
	} else if (player_said("conv10")) {
		conv10();
	} else if (player_said("conv11")) {
		conv11();
	} else if (player_said("conv13")) {
		conv13();
	} else if (player_said("talk to")) {
		talkTo();
	} else if (lookFlag && player_said("town records")) {
		if (_G(flags)[V112]) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_G(kernel).trigger_mode = KT_PARSE;
				wilbur_speech("105w010");
				break;

			case 1:
				_val5 = 2;
				_val6 = 21;
				_digi1 = "105g001";
				_val3 = 60;
				break;

			default:
				break;
			}
		} else {
			wilbur_speech("105w001");
		}
	} else if (!_G(walker).wilbur_said(SAID)) {
		if (player_said("GEAR", "town records")) {
			if (_G(flags)[V112]) {
				wilbur_speech("105w011");
			} else {
				series_play_with_breaks(PLAY22, "105wi01", 0xa00, 18, 3);
			}
		} else if (player_said("GEAR", "mayor's office")) {
			player_set_commands_allowed(false);
			ws_hide_walker();
			series_play_with_breaks(PLAY31, "105wi05", 0x100, 16, 3);

			if (!_G(flags)[V112]) {
				_G(flags)[V298] = 1;
				enteringMayorsOffice();
			}
		} else if (player_said("GEAR", "complaints dept.")) {
			player_set_commands_allowed(false);
			_G(flags)[V298] = 1;
			loadAssets3();
			digi_preload("11n1402");
			ws_hide_walker();
			series_play_with_breaks(PLAY29, "105wi06", 0xa00, 15, 3);
		} else if (player_said("gear", "town records")) {
			wilbur_speech("105w011");
		} else if (player_said("exit", "STAIRS")) {
			player_set_commands_allowed(false);
			pal_fade_init(7);
		} else if (!player_said("take", "baitbox")) {
			return;
		}
	}

	_G(player).command_ready = false;
}

void Room105::conv10() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();
	int node = conv_current_node();

	if (_G(kernel).trigger == 10) {
		if (who <= 0) {
			if (node == 1) {
				digi_stop(2);

				// TODO: Double-check if the belows should be !strcmp
				if (strcmp(conv_sound_to_play(), "10n02011")) {
					digi_unload("105gucr1");
				} else if (strcmp(conv_sound_to_play(), "10n02012")) {
					digi_unload("105gucr2");
				} else {
					digi_unload("105gucr3");
				}

				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(1);
			} else {
				_val3 = _val8 ? 36 : 26;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatch_now(1);
			}
		} else {
			SendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
			conv_resume();
		}	
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			if (node == 23)
				_flag1 = true;
			if (node == 21)
				_flag2 = true;

			if (node == 1) {
				if (strcmp(conv_sound_to_play(), "10n02011")) {
					digi_preload("105gucr1");
					digi_play("105gucr1", 2, 150);
				} else if (strcmp(conv_sound_to_play(), "10n02012")) {
					digi_preload("105gucr2");
					digi_play("105gucr2", 2, 150);
				} else if (strcmp(conv_sound_to_play(), "10n02013")) {
					digi_preload("105gucr3");
					digi_play("105gucr3", 2, 150);
				}

				digi_play(conv_sound_to_play(), 1, 255, 10);

			} else {
				_val3 = _val8 ? 37 : 27;
			}
		} else if (who == 1) {
			SendWSMessage(0x140000, 0, _G(my_walker), 0, nullptr, 1);
			digi_play(conv_sound_to_play(), 1, 255, 10);
		}
	} else {
		conv_resume();
	}
}

void Room105::conv11() {
	error("TODO: Room105::conv11");
}

void Room105::conv12() {
	error("TODO: Room105::conv12");
}

void Room105::conv13() {
	error("TODO: Room105::conv13");
}

void Room105::talkTo() {
	error("TODO: Room105::talkTo");
}

void Room105::enteringMayorsOffice() {
	if (_G(flags)[V031]) {
		_val8 = 1;
		loadAssets2();
		_val3 = 23;
		_val4 = 35;
	} else {
		_G(flags)[V031] = 1;
		_val8 = 0;
		loadAssets1();
		_val3 = 23;
		_val4 = 25;
	}
}

void Room105::loadAssets1() {
	static const char *NAMES[12] = {
		"105mg02", "105mg02s", "105mg03", "105mg03s", "105mg04", "105mg04s",
		"105mg05", "105mg05s", "105mg06", "105mg06s", "105mg07", "105mg07s"
	};

	for (int i = 0; i < 12; ++i)
		_series1[i] = series_load(NAMES[i]);
}

void Room105::loadAssets2() {
	static const char *NAMES[4] = {
		"105mg10", "105mg11", "105mg12", "105mg13"
	};

	for (int i = 0; i < 4; ++i)
		_series2[i] = series_load(NAMES[i]);
}

void Room105::loadAssets3() {
	static const char *NAMES[12] = {
		"105eg02", "105eg02s", "105eg04", "105eg04s", "105eg05", "105eg05s",
		"105eg06", "105eg06s", "105eg07", "105eg07s", "105eg09", "105eg09s"
	};

	for (int i = 0; i < 12; ++i)
		_series1[i] = series_load(NAMES[i]);

	series_load("105eg10");
	series_load("105eg10s");
}

void Room105::unloadAssets1() {
	for (int i = 0; i < 12; ++i)
		series_unload(_series1[i]);
}

void Room105::unloadAssets2() {
	for (int i = 0; i < 4; ++i)
		series_unload(_series2[i]);
}

void Room105::unloadAssets3() {
	for (int i = 0; i < 12; ++i)
		series_unload(_series3[i]);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
