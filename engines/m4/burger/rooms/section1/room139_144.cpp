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

#include "m4/burger/rooms/section1/room139_144.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_BURL_ANIMATION = 9
};

static const char *SAID1[][4] = {
	{ "ISLAND",    nullptr,   "999w011", "999w011" },
	{ "ROCK",      "144W004", "999w011", "999w011" },
	{ "SHACK",     "144W005", "999w011", nullptr   },
	{ "SIGN",      "144W006", nullptr,   nullptr   },
	{ "ROWBOAT",   "144W007", nullptr,   nullptr   },
	{ "WATER",     "144W008", "999w011", "144W009" },
	{ "BURL",      "144W010", "999w011", "999w011" },
	{ "MOTORBOAT", "144W012", nullptr,   "144W013" },
	{ "VERA'S DINER", nullptr, "999w011", "999w011" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const char *SAID2[][4] = {
	{ "ISLAND",     "139W001", "139W002", "139W002" },
	{ "ROCK",       nullptr,   "139W002", "139W002" },
	{ "SHACK",      "139W005", "139W002", nullptr   },
	{ "SIGN",       "139W006", nullptr,   nullptr   },
	{ "ROWBOAT",    "139W007", nullptr,   nullptr   },
	{ "WATER",      "139W008", "139W002", "139W009" },
	{ "VERA'S DINER", nullptr, "139W002", "139W002" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{ 0, 32, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 33, 39, "139_002", 2, 255, -1, 0, 0, 0, 0 },
	{ 40, 43, 0, 1, 255, 8, 0, 0, 0, 0 },
	{ 44, 52, "139_003", 2, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	{ 52, 52, 0, 1, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{  0, 35, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 36, 39, "144_002", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 40, 46, nullptr,   1, 255,  8, 0, 0, nullptr, 0 },
	{ 47, 52, "144_003", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	{ 52, 52, nullptr,   1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesStreamBreak SERIES1[] = {
	{ 6, 0, 2, 255, 2, 0, 0, 0 },
	{ 10, "100_022", 2, 255, -1, 0, 0, 0 },
	{ -1, 0, 0, 0, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

static const seriesStreamBreak SERIES2[] = {
	{ 1, "144b001", 1, 255, -1, 0, nullptr, 0 },
	{ 45, nullptr,  1, 255,  5, 0, nullptr, 0 },
	STREAM_BREAK_END
};


void Room139_144::init() {
	_val1 = 0;
	_burlFrame = 15;
	digi_preload("144_001");

	if (_G(game).previous_room == 144)
		preloadDigi();

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 139:
		player_set_commands_allowed(false);
		ws_demand_location(470, 269, 8);
		_burlShould = 1;
		_burlMode = 3;
		kernel_trigger_dispatch_now(9);
		break;

	case 141:
		ws_demand_location(250, 224, 4);
		break;

	case 142:
		ws_demand_location(670, 257, 9);
		ws_walk(596, 245, 0, -1, 8);
		break;

	default:
		ws_demand_location(320, 271, 5);
		break;
	}

	if (_G(game).previous_room != 139 && _G(flags)[V000] == 1002) {
		_burlShould = _burlMode = 3;
		kernel_trigger_dispatch_now(kCHANGE_BURL_ANIMATION);
	}

	if (_G(flags)[kPerkinsLostIsland]) {
		series_play("144mb01", 0x600, 0, -1, 600, -1, 100, 0, 0, 0, 0);
	} else {
		hotspot_set_active("motorboat", false);
	}

	_series1 = series_play((_G(flags)[V000] == 1002) ? "144rboat" : "139rboat",
		0xf00, 0, -1, 600, -1, 100, 0, 0, 0, 0);
	digi_play_loop("144_001", 3);
}

void Room139_144::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 2:
		digi_change_volume(2, 0);
		break;

	case 3:
		// Burl's truck crashing off of bridge
		pal_fade_set_start(0);
		ws_hide_walker();
		terminateMachineAndNull(_series1);
		gr_backup_palette();

		digi_preload("100_022");
		digi_preload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES1, "120dt01", 8, 0, 1012);
		kernel_timing_trigger(1, 4);
		break;

	case 4:
		pal_fade_init(15, 255, 100, 0, -1);
		break;

	case 5:
		digi_play("144b003d", 1);
		break;

	case 6:
		player_set_commands_allowed(true);
		_burlShould = 3;
		break;

	case 7:
		ws_hide_walker();
		terminateMachineAndNull(_series1);

		if (_G(flags)[V000] == 1002) {
			series_play_with_breaks(PLAY2, "144wi01", 0xf00, -1, 3, 6, 100, 0, 0);
		} else {
			series_play_with_breaks(PLAY1, "139wi01", 0xf00, -1, 3, 6, 100, 0, 0);
		}
		break;

	case 8:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1013);
		break;

	case kCHANGE_BURL_ANIMATION:
		switch (_burlMode) {
		case 3:
			switch (_burlShould) {
			case 1:
				_burlShould = 2;
				series_play("144bu01s", 0x301, 0, -1, 8);
				digi_preload("144b003d");
				digi_preload_stream_breaks(SERIES2);
				series_stream_with_breaks(SERIES2, "144bu01", 8, 0x300, kCHANGE_BURL_ANIMATION);
				break;

			case 2:
				digi_unload_stream_breaks(SERIES2);
				player_set_commands_allowed(true);
				randomDigi();
				_burlShould = 3;

				series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 15);
				series_play("144bu03s", 0x301, 0, -1, 7, 0, 100, 0, 0, 0, 15);
				break;

			case 3:
				if (imath_ranged_rand(5, 10) < ++_val1) {
					switch (imath_ranged_rand(1, 4)) {
					case 1:
						_burlFrame = 25;
						break;

					case 2:
						_burlFrame = 36;
						break;

					case 3:
						_burlFrame = 41;
						break;

					case 4:
						_burlFrame = 42;
						break;

					default:
						break;
					}
				}

				if (imath_ranged_rand(1, 10) == 1) {
					switch (imath_ranged_rand(1, 4)) {
					case 1:
						randomDigi();
						series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 16, 24);
						series_play("144bu03s", 0x301, 0, -1, 10, 0, 100, 0, 0, 16, 24);
						break;

					case 2:
						randomDigi();
						series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 26, 35);
						series_play("144bu03s", 0x301, 0, -1, 10, 0, 100, 0, 0, 26, 35);
						break;

					case 3:
						randomDigi();
						series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 37, 40);
						series_play("144bu03s", 0x301, 0, -1, 10, 0, 100, 0, 0, 37, 40);
						break;

					case 4:
						_burlShould = 7;
						series_play("144bu06", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 12);
						series_play("144bu06s", 0x301, 0, -1, 7, 0, 100, 0, 0, 0, 12);
						break;

					default:
						break;
					}
				} else {
					series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, _burlFrame, _burlFrame);
					series_play("144bu03s", 0x301, 0, -1, 7, 0, 100, 0, 0, _burlFrame, _burlFrame);
				}
				break;

			case 4:
			case 5:
				digi_play("144b002", 2);
				_burlMode = 6;
				series_play("144bu06", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 2);
				series_play("144bu06s", 0x301, 0, -1, 7, 0, 100, 0, 0, 0, 2);
				break;

			case 7:
				digi_change_volume(2, 0);
				_burlMode = 7;
				series_play("144bu07", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 2);
				series_play("144bu07s", 0x301, 0, -1, 7, 0, 100, 0, 0, 0, 2);
				break;

			default:
				break;
			}
			break;

		case 4:
			switch (_burlShould) {
			case 4:
				series_play("144bu08", 0x300, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 3, 3);
				series_play("144bu08s", 0x301, 0, -1, 10, 0, 100, 0, 0, 3, 3);
				break;

			case 5:
				_burlMode = 5;
				series_play("144bu08", 0x300, 0, kCHANGE_BURL_ANIMATION, 4, 0, 100, 0, 0, 4, 7);
				series_play("144bu08s", 0x301, 0, -1, 4, 0, 100, 0, 0, 4, 7);
				break;

			default:
				digi_play("144b002", 2);
				_burlMode = 3;
				_burlShould = 2;
				series_play("144bu08", 0x300, 2, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 3);
				series_play("144bu08s", 0x301, 2, -1, 7, 0, 100, 0, 0, 0, 3);
				break;
			}
			break;

		case 5:
			if (_burlShould == 5) {
				frame = imath_ranged_rand(0, 2);
				series_play("144bu09", 0x300, 0, kCHANGE_BURL_ANIMATION, 4, 0, 100, 0, 0, frame, frame);
				series_play("144bu09s", 0x301, 0, -1, 4, 0, 100, 0, 0, frame, frame);

				if (_digi1) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_digi1, 1, 255, 1);
					_digi1 = nullptr;
				}
			} else {
				_burlMode = 4;
				series_play("144bu08", 0x300, 2, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 4, 7);
				series_play("144bu08s", 0x301, 2, -1, 7, 0, 100, 0, 0, 4, 7);				
			}
			break;

		case 6:
			_burlMode = 4;
			series_play("144bu08", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 3);
			series_play("144bu08s", 0x301, 0, -1, 7, 0, 100, 0, 0, 0, 3);
			break;

		case 7:
			if (_burlShould == 7) {
				if (imath_ranged_rand(1, 10) == 1) {
					randomDigi();
					_burlMode = 3;
					_burlShould = 3;
					series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 15);
					series_play("144bu03s", 0x301, 0, -1, 7, 0, 100, 0, 0, 0, 15);
				} else {
					series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 10, 0, 100, 0, 0, 0, 0);
					series_play("144bu03s", 0x301, 0, -1, 10, 0, 100, 0, 0, 0, 0);
				}
			} else {
				_burlMode = 3;
				series_play("144bu03", 0x300, 0, kCHANGE_BURL_ANIMATION, 7, 0, 100, 0, 0, 0, 15);
				series_play("144bu03s", 0x301, 0, -1, 7, 0, 100, 0, 0, 0, 15);
			}
			break;

		default:
			break;
		}
		break;

	case kBurlEntersTown:
		if (!_G(flags)[kRoadOpened]) {
			_G(kernel).continue_handling_trigger = true;
		} else if (player_commands_allowed() && _G(player).walker_visible && INTERFACE_VISIBLE) {
			Section1::updateDisablePlayer();
			kernel_timing_trigger(1, 3);
		} else {
			kernel_timing_trigger(60, kBurlEntersTown);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room139_144::pre_parser() {
	if (player_said("vera's diner") && player_said_any("enter", "look", "look at"))
		player_hotspot_walk_override_just_face(3);
}

void Room139_144::parser() {
	bool lookFlag = player_said_any("look", "look at");
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("talk to")) {
		conv_load_and_prepare("conv31", 6);
		conv_export_value_curr(_G(flags)[V052], 0);
		conv_export_value_curr(_G(flags)[kPerkinsLostIsland], 1);
		conv_play_curr();

	} else if (_G(flags)[V000] == 1002 && _G(walker).wilbur_said(SAID1)) {
		// Do nothing
	} else if (_G(flags)[V000] == 1002 && _G(walker).wilbur_said(SAID2)) {
		// Do nothing
	} else if (player_said("conv31")) {
		conv31();

	} else if (inv_player_has(_G(player).verb) && _G(flags)[V000] == 1002 &&
			player_said_any("island", "rock")) {
		_G(walker).wilbur_speech("999w011");

	} else if (inv_player_has(_G(player).verb) && _G(flags)[V000] == 1002 &&
			player_said("burl")) {
		_G(walker).wilbur_speech("144w011");

	} else if (inv_player_has(_G(player).verb) && _G(flags)[V000] != 1002 &&
			player_said_any("rock", "island")) {
		_G(walker).wilbur_speech("139w002");

	} else if (lookFlag && player_said("rock")) {
		wilbur_speech(inv_player_has("deed") ? "139w004" : "139w003");

	} else if (lookFlag && player_said("island") && _G(flags)[V000] == 1002) {
		wilbur_speech(_G(flags)[kPerkinsLostIsland] ? "144w002" : "144w001");

	} else if (player_said("enter", "vera's diner") || (lookFlag && player_said("vera's diner"))) {
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 30, 1014);

	} else if (player_said("gear", "rowboat") || player_said("go island")) {
		player_set_commands_allowed(false);
		kernel_trigger_dispatch_now(7);

	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room139_144::preloadDigi() {
	digi_preload("144b002");
	digi_preload("144b003a");
	digi_preload("144b003b");
	digi_preload("144b003c");
}

void Room139_144::randomDigi() {
	if (!digi_play_state(2)) {
		digi_play(Common::String::format("144b003%c", 'a' + imath_ranged_rand(0, 2)).c_str(), 2, 255);
	}

	digi_change_volume(2, 255);
}

void Room139_144::conv31() {
	_G(kernel).trigger_mode = KT_PARSE;
	int who = conv_whos_talking();

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			_burlShould = 4;
			conv_resume();
		} else if (who == 1) {
			conv_resume();
			sendWSMessage(0x150000, 0, _G(my_walker), 0, nullptr, 1);
		}
	} else if (conv_sound_to_play()) {
		if (who <= 0) {
			_burlShould = 5;
			_digi1 = conv_sound_to_play();
		} else if (who == 1) {
			wilbur_speech(conv_sound_to_play(), 1);
		}
	} else {
		conv_resume();
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
