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

#include "m4/burger/rooms/section1/room172.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_POLLY_ANIMATION = 1
};

static const char *SAID[][4] = {
	{ "AUNT POLLY",    "172w001", "172w002", "172w002" },
	{ "PARLOUR",       "172w002", nullptr,   nullptr   },
	{ "BASEMENT",      nullptr,   "172w002", nullptr   },
	{ "MICROWAVE",     "172w003", nullptr,   nullptr   },
	{ "REFRIGERATOR",  "172w005", "172w002", nullptr   },
	{ "OVEN",          nullptr,   "172w002", nullptr   },
	{ "SINK",          "172w008", "172w009", nullptr   },
	{ "HUTCH",         "172w011", "172w012", "172w002" },
	{ "BROOM",         "172w013", "172w014", "172w015" },
	{ "WINDOW",        "172w016", "172w002", "172w017" },
	{ "CALENDAR",      "172w018", "172w019", "172w018" },
	{ "CUPBOARD ",     "172w021", "172w002", nullptr   },
	{ "CUPBOARD",      "172w022", "172w002", nullptr   },
	{ "POTS AND PANS", "172w023", "172w024", "172w024" },
	{ "COOKIES",       "172w025", "172w026", "172w027" },
	{ "PRESERVES",     "172w028", "172w029", "172w029" },
	{ "CARROT JUICE ", "172w030", nullptr,   "172w031" },
	{ "JUICER",        "172w032", nullptr,   "172w033" },
	{ "VEGGIES",       "172w034", "172w035", "172w035" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{  0,  9, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 10, -1, nullptr, 0, 0,  2, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 8, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  0, 27, "172_001", 2, 255, -1, 0,  0, nullptr, 0 },
	{ 28, 30, "172_005", 3, 125, -1, 0, -1, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 44, 45, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 45, 45, nullptr, 0, 0, -1, 0, 3, nullptr, 0 },
	{ 45, 44, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY5[] = {
	{ 36, 43, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 36, 36, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY6[] = {
	{ 46, 48, "172_004", 3, 125, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY7[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room172::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 171:
		if (!_G(flags)[V092] && !_G(flags)[V091]) {
			player_set_commands_allowed(false);
			series_load("172ap1t");
			series_load("172ap1ts");
			series_load("172ap02t");
			digi_preload("172p906");

			_pollyShould = 4;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			ws_demand_location(300, 325, 3);
			ws_walk(368, 359, nullptr, -1, 2);

		} else {
			ws_demand_location(11, 296, 3);
			ws_walk(156, 297, nullptr, -1, -1);
		}
		break;

	case 173:
		ws_demand_location(171, 254, 3);
		ws_walk(250, 270, nullptr, -1, -1);
		break;

	default:
		ws_demand_location(312, 292, 5);
		break;
	}

	hotspot_set_active("AUNT POLLY", false);

	if (_G(flags)[V092]) {
		_pollyShould = 15;
		kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
		_walk1 = intr_add_no_walk_rect(357, 311, 639, 353, 356, 354);
		hotspot_set_active("AUNT POLLY", true);
	} else {
		_series1 = series_show(_G(flags)[V091] ? "172fud2" : "172fud", 0x700);
		series_show("172fuds", 0x701);
	}

	if (inv_player_has("CARROT JUICE") || inv_player_has("BOTTLE") ||
			inv_player_has("SOAPY WATER")) {
		hotspot_set_active("CARROT JUICE ", false);
	} else {
		_jug.show("172jug", 0x700);
	}
}

void Room172::daemon() {
	switch (_G(kernel).trigger) {
	case kCHANGE_POLLY_ANIMATION:
		switch (_pollyShould) {
		case 4:
			_pollyShould = 5;
			series_play_with_breaks(PLAY2, "172ap01", 0x600, kCHANGE_POLLY_ANIMATION, 3, 8);
			digi_play("172p906", 2, 255, kCHANGE_POLLY_ANIMATION);
			break;

		case 5:
			_G(flags)[V092] = 1;
			hotspot_set_active("AUNT POLLY", true);
			_pollyShould = 7;
			_series5.play("172ap1t", 0x400, 4, -1, 6, -1);
			break;

		case 6:
			freeSeries5();
			_pollyShould = 8;
			npc_say("172p907", kCHANGE_POLLY_ANIMATION, "172ap1t", 0x400, true);
			break;

		case 7:
			digi_unload("172p906");
			freeSeries5();
			_series5.show("172ap1t", 0x400);
			_pollyShould = 6;
			wilbur_speech("172w902", kCHANGE_POLLY_ANIMATION);
			break;

		case 8:
			_G(flags)[V091] = true;
			terminateMachineAndNull(_series1);
			terminateMachineAndNull(_series1s);
			_pollyShould = 9;
			series_play_with_breaks(PLAY3, "172ap02", 0x400, kCHANGE_POLLY_ANIMATION, 3, 8);
			break;

		case 9:
			_series5.play("172ap02", 0x400, 0, -1, 8, -1, 100, 0, 0, 28, 30);
			kernel_trigger_dispatch_now(3);
			playDigi3();
			break;

		case 10:
			freeSeries7();
			_pollyShould = 11;
			series_play_with_breaks(PLAY4, "172ap02", 0x400, kCHANGE_POLLY_ANIMATION, 3, 8);
			break;

		case 11:
			freeSeries7();
			_pollyShould = 14;
			npc_say(kCHANGE_POLLY_ANIMATION, "172ap02", 0x400, true, 31, 35);
			break;

		case 12:
			player_set_commands_allowed(false);
			_G(flags)[kDisableFootsteps] = 1;
			freeSeries7();
			_series5.terminate();
			_pollyShould = 15;
			npc_say(_convName, kCHANGE_POLLY_ANIMATION, "172ap02", 0x400, true, 31, 35);
			break;

		case 13:
			freeSeries7();
			series_play_with_breaks(PLAY5, "172ap02", 0x400, 10001, 3, 8);
			break;

		case 14:
			_series5.play("172ap02", 0x400, 0, -1, 8, -1, 100, 0, 0, 28, 30);
			playDigi3();
			conv_resume_curr();
			break;

		case 15:
			_G(flags)[kDisableFootsteps] = 0;
			player_set_commands_allowed(true);
			_series5.play("172ap02", 0x400, 0, -1, 8, -1, 100, 0, 0, 28, 30);
			playDigi3();
			break;

		case 16:
			freeSeries7();
			_pollyShould = 17;
			series_play_with_breaks(PLAY6, "172ap02", 0x400, kCHANGE_POLLY_ANIMATION, 3, 8);
			break;

		case 17:
			freeSeries5();
			hotspot_set_active("AUNT POLLY", false);
			_series1 = series_show("172fud2", 0x700);
			_series1s = series_show("172fuds", 0x701);
			intr_remove_no_walk_rect(_walk1);
			_pollyShould = 18;
			series_play_with_breaks(PLAY7, "172ap03", 0x600, kCHANGE_POLLY_ANIMATION, 3, 8);
			break;

		case 18:
			player_set_commands_allowed(true);
			break;

		default:
			term_message("ERROR!!!! polly_should not set!");
			break;
		}
		break;

	case 2:
		_jug.terminate();
		break;

	case 3:
		_G(flags)[kDisableFootsteps] = 1;
		_G(flags)[V299] = 1;
		conv_load_and_prepare("conv41", 4);
		conv_export_value_curr(_G(flags)[V088], 0);
		conv_export_value_curr(inv_player_has("WHISTLE") ? 1 : 0, 1);
		conv_export_pointer_curr(&_G(flags)[V093], 2);
		conv_play_curr();
		break;

	case 4:
		freeSeries5();

		if (_G(flags)[V092]) {
			freeSeries7();
			_pollyShould = 15;
		} else {
			_G(flags)[V088] = 1;
			_pollyShould = 16;
		}

		kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
		_G(flags)[kDisableFootsteps] = 0;
		_G(flags)[V299] = 0;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			_convName = _G(flags)[V087] ? "172p903" : "172p902";
			_pollyShould = 12;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 2:
			disable_player();
			_G(wilbur_should) = 3;
			series_play_with_breaks(PLAY1, "172wi01", 0x700, kCHANGE_WILBUR_ANIMATION, 3, 8);
			break;

		case 3:
			enable_player();
			hotspot_set_active("CARROT JUICE ", false);
			inv_give_to_player("CARROT JUICE");
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

void Room172::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[V092] && _G(player).walk_x >= 357 && _G(player).walk_x <= 639 &&
			_G(player).walk_y >= 311 && _G(player).walk_y <= 353)
		player_hotspot_walk_override(356, 332, 3);

	if (!player_said_any("GEAR", "LOOK AT")) {
		if (player_said("PARLOUR"))
			player_set_facing_at(84, 300);

		if (player_said("BASEMENT"))
			player_set_facing_at(171, 254);
	}
}

void Room172::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("conv41")) {
		conv41();
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("PARLOUR")) {
		disable_player_commands_and_fade_init(1018);
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("BASEMENT")) {
		disable_player_commands_and_fade_init(1020);
	} else if (player_said("TAKE", "CARROT JUICE ")) {
		if (_G(flags)[V092]) {
			_convName = "172p950";
			_pollyShould = 12;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
		} else {
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("WHISTLE", "AUNT POLLY")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 1;
		wilbur_speech("172w901", kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TALK", "AUNT POLLY")) {
		kernel_trigger_dispatch_now(3);
	} else if (player_said("AUNT POLLY") && player_said_any("PHONE BILL", "CARROT JUICE")) {
		_convName = imath_rand_bool(2) ? "172p901" : "172904";
		_pollyShould = 12;
		kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
	} else if (player_said("MICROWAVE")) {
		parserAction("172w004");
	} else if (player_said("GEAR", "REFRIGERATOR")) {
		parserAction("172w005");
	} else if (player_said("GEAR", "OVEN")) {
		parserAction("172w007");
	} else if (player_said("SINK")) {
		parserAction("172w010");
	} else if (player_said("GEAR") && player_said_any("CUPBOARD", "CUPBOARD ")) {
		parserAction("172w036");
	} else if (player_said("TAKE", "JUICER")) {
		if (_G(flags)[V092]) {
			_convName = imath_rand_bool(2) ? "172p905a" : "172p905b";
			_pollyShould = 12;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
		} else {
			wilbur_speech("172w033");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room172::parserAction(const char *name) {
	if (_G(flags)[V092]) {
		_convName = "172p950";
		_pollyShould = 12;
		kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
	} else {
		wilbur_speech(name);
	}
}

void Room172::freeSeries7() {
	digi_stop(2);
	terminateMachineAndNull(_series7);
}

void Room172::playDigi3() {
	loadSeries7();
	digi_preload("172_003");
	digi_play_loop("172_003", 3, 125);
}

void Room172::loadSeries7() {
	digi_preload("172p909");
	digi_play_loop("172p909", 2, 255);
	_series7 = series_play("172ap02t", 0x3ff, 4, -1, 4);
}

void Room172::conv41() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who == 1) {
			wilbur_speech(sound, 10001);
		} else {
			freeSeries5();

			switch (node) {
			case 0:
				_pollyShould = 4;
				kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
				break;

			case 2:
				switch (entry) {
				case 0:
					_pollyShould = 10;
					kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
					break;

				case 1:
					freeSeries7();
					_G(flags)[V092] = 0;
					_pollyShould = 14;
					npc_say(sound, kCHANGE_POLLY_ANIMATION, "172ap02", 0x400, true, 31, 35);
					break;

				default:
					break;
				}
				break;

			case 3:
				freeSeries7();
				_pollyShould = 13;
				npc_say(sound, kCHANGE_POLLY_ANIMATION, "172ap02", 0x400, true, 31, 35);
				break;

			default:
				freeSeries7();
				_pollyShould = 14;
				npc_say(sound, kCHANGE_POLLY_ANIMATION, "172ap02", 0x400, true, 31, 35);
				break;
			}
		}
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
