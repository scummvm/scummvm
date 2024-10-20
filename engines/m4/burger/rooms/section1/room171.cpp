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

#include "m4/burger/rooms/section1/room171.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_POLLY_ANIMATION = 5
};

static const char *SAID[][4] = {
	{ "AUNT POLLY", "171w001", "171w002", "171w002" },
	{ "FRONT DOOR", "171w003", "171w002", nullptr   },
	{ "KITCHEN",    nullptr,   "171w002", nullptr   },
	{ "STAIRS",     "171w004", "171w002", nullptr   },
	{ "FIREPLACE",  nullptr,   "171w002", "171w005" },
	{ "KINDLING ",  "171w006", "171w007", "171w007" },
	{ "WINDOW",     "171w008", "171w002", "171w009" },
	{ "TELEPHONE",  "171w010", "171w011", "171w012" },
	{ "ARMCHAIR",   "171w013", "171w014", "171w015" },
	{ "PIANO",      "171w019", "171w020", "171w021" },
	{ "PICTURE",    "171w022", "171w023", "171w024" },
	{ "PICTURE ",   "171w025", "171w023", nullptr   },
	{ "PAINTING",   "171w026", "171w027", nullptr   },
	{ "CLOCK",      "171w028", "171w029", "171w030" },
	{ "PILLS",      "171w031", "171w032", "171w033" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room171::PLAY1[] = {
	{ 0, 2, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 3, 6, "171_014a", 1, 100, -1, 0, 0, 0, 0 },
	{ 7, 12, "171_014b", 1, 100, -1, 0, 0, 0, 0 },
	{ 13, 18, "171_014a", 1, 100, -1, 0, 0, 0, 0 },
	{ 19, 22, "171_014b", 1, 100, -1, 0, 0, 0, 0 },
	{ 23, 27, "171_014a", 1, 100, -1, 0, 0, 0, 0 },
	{ 28, 32, "171_014b", 1, 100, -1, 0, 0, 0, 0 },
	{ 33, 39, "171_014a", 1, 100, -1, 0, 0, 0, 0 },
	{ 40, -1, "171_014b", 1, 100, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY2[] = {
	{ 0, 0, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 1, 6, "171_014a", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 7, 12, "171_014b", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 13, 17, "171_014a", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 18, 22, "171_014b", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 24, 29, "171_014a", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 30, 35, "171_014b", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 36, 40, "171_014a", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 41, -1, "171_014b", 2, 100, -1, 0, 0, &Flags::_flags[V086], 0 },
	{ 1, 6, "171_014a", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	{ 7, 12, "171_014b", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	{ 13, 17, "171_014a", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	{ 18, 22, "171_014b", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	{ 24, 29, "171_014a", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	{ 30, 35, "171_014b", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	{ 36, 40, "171_014a", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	{ 41, -1, "171_014b", 1, 100, -1, 0, 0, &Flags::_flags[V086], 1 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY3[] = {
	{ 0, 4, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 5, 16, 0, 0, 0, 3, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY4[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY5[] = {
	{ 0, -1, "171_001", 2, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY6[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY7[] = {
	{ 0, -1, 0, 2, 0, -1, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY8[] = {
	{ 0, 4, "171_011", 2, 255, -1, 0, 0, 0, 0 },
	{ 5, 19, 0, 0, 0, 3, 0, 0, 0, 0 },
	{ 20, 25, "171_012", 2, 255, -1, 0, 0, 0, 0 },
	{ 26, 33, "171_010", 2, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY9[] = {
	{ 34, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY10[] = {
	{ 7, 8, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 8, 8, 0, 0, 0, -1, 0, 2, 0, 0 },
	{ 7, 8, 0, 0, 0, -1, 2, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY11[] = {
	{ 7, 12, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 13, -1, "171_006", 2, 255, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY12[] = {
	{ 0, 4, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 4, 4, 0, 0, 0, -1, 0, 10, 0, 0 },
	{ 0, 4, 0, 0, 0, -1, 2, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY13[] = {
	{ 0, -1, 0, 0, 0, -1, 1, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY14[] = {
	{ 0, 17, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 18, -1, "171_004", 2, 255, -1, 0, 0, 0, 0 },
	{ 0, 11, 0, 0, 0, -1, 2, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY15[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY16[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room171::PLAY17[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};


void Room171::init() {
	_flag1 = false;
	_val1 = 0;

	if (_G(flags)[V092]) {
		hotspot_set_active("AUNT POLLY", false);
	} else {
		switch (_G(game).previous_room) {
		case 170:
			player_set_commands_allowed(false);

			if (_G(flags)[V083] && _G(flags)[V084] && _G(flags)[V085] && _G(flags)[V086]) {
				loadSeries1();
				_pollyShould = 26;
				kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			} else {
				while (!_val1) {
					switch (imath_ranged_rand(1, 4)) {
					case 1:
						if (!_G(flags)[V083]) {
							_val1 = 10019;
							loadSeries2();
							_pollyShould = 5;
							kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
						}
						break;

					case 2:
						if (!_G(flags)[V084]) {
							_val1 = 10021;
							loadSeries3();
							digi_preload("171_002");
							digi_play_loop("171_002", 2);
						}
						break;

					case 3:
						if (!_G(flags)[V085]) {
							_val1 = 10020;
							loadSeries4();

							if (imath_rand_bool(2)) {
								digi_preload("171_013a");
								digi_play_loop("171_013a", 2, 255);
							} else {
								digi_preload("171_013b");
								digi_play_loop("171_013b", 2, 255);
							}
						}
						break;

					case 4:
						if (!_G(flags)[V086]) {
							_val1 = 10022;
							loadSeries5();
							_pollyShould = 20;
							kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
						}
						break;

					default:
						break;
					}
				}
			}
			break;

		case 175:
			if (!_G(flags)[V092]) {
				if (_G(flags)[V086]) {
					loadSeries1();
					_pollyShould = 26;
					kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
				} else {
					_val1 = 10022;
					loadSeries5();
					kernel_trigger_dispatch_now(6);
				}
			}
			break;

		default:
			loadSeries1();
			_pollyShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;
		}
	}

	_series3 = series_show("171pills", 0x700);

	if (_val1 || _G(flags)[V092]) {
		_chair.show("171chair", "171chars", 0x900);
	}

	hotspot_set_active("PIANO STOOL", false);
	hotspot_set_active("PIANO STOOL ", false);

	if (_val1 == 10020 || _val1 == 10019) {
		hotspot_set_active("PIANO STOOL ", true);
		intr_add_no_walk_rect(378, 300, 518, 354, 377, 355);

		if (_val1 == 10020) {
			series_show("171stool", 0x300);
			series_show("171stols", 0x301);
		}
	} else {
		series_show("171st02", 0x100);
		hotspot_set_active("PIANO STOOL", true);
	}

	_lid = series_show("171plid", 0);

	if (_val1 != 10022)
		series_show("171phone", 0x700);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 170:
		ws_demand_location(84, 275, 3);

		if (_G(flags)[V092] || (_G(flags)[V083] && _G(flags)[V084] &&
			_G(flags)[V085] && _G(flags)[V086])) {
			ws_walk(134, 275, nullptr, -1);
		} else {
			ws_walk(134, 275, nullptr, 6);
		}
		break;

	case 172:
		ws_demand_location(639, 289, 9);
		ws_walk(541, 295, nullptr, -1);
		break;

	case 175:
		ws_demand_location(237, 232, 9);
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(324, 319, 5);
		break;
	}
}

void Room171::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		disable_player_commands_and_fade_init(1019);
		break;

	case 2:
		disable_player_commands_and_fade_init(1022);
		break;

	case 3:
		npc_say();
		break;

	case 4:
		if (_pollyShould == 28 || _pollyShould == 36 || _pollyShould == 34 || _pollyShould == 33 ||
				_pollyShould == 38 || _pollyShould == 30) {
			digi_stop(2);
			freeSeries();
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
		}
		break;

	case kCHANGE_POLLY_ANIMATION:
		// Aunt Polly
		switch (_pollyShould) {
		case 5:
			digi_play_loop("171_003", 2);
			_series.play("171ap01", 0x300, 0, -1, 6, -1);
			break;

		case 6:
			npc_say(conv_sound_to_play(), kRESUME_CONVERSATION, "171ap01n", 0x2ff, false);
			break;

		case 7:
			digi_stop(2);
			freeSeries();
			series_play_with_breaks(PLAY3, "171ap02", 0x300, kRESUME_CONVERSATION, 3);
			break;

		case 8:
			_pollyShould = 9;
			npc_say(conv_sound_to_play(), 5, "171ap02", 0x300, 1, 17, 22);
			break;

		case 9:
			freeSeries();
			_pollyShould = 10;
			series_play_with_breaks(PLAY4, "171ap03", 0x300, 5, 3);
			break;

		case 10:
			_G(flags)[V083] = 1;
			_chair.terminate();

			series_show("171stool", 0x300);
			series_show("171stols", 0x301);
			_pollyShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 11:
			npc_say(kRESUME_CONVERSATION);
			break;

		case 12:
			digi_stop(2);
			npc_say();
			series_play_with_breaks(PLAY5, "171ap04", 0x800, kRESUME_CONVERSATION, 3);
			break;

		case 13:
			_pollyShould = 14;
			npc_say(kCHANGE_POLLY_ANIMATION, "171ap05", 0x800);
			break;

		case 14:
			_G(flags)[V084] = 1;
			_chair.terminate();
			_pollyShould = 26;
			series_play_with_breaks(PLAY6, "171ap06", 0x800, 5, 3);
			break;

		case 15:
			terminateMachineAndNull(_lid);
			_pollyShould = 16;
			series_play_with_breaks(PLAY7, "171ap07", 0, kCHANGE_POLLY_ANIMATION, 2);
			break;

		case 16:
			npc_say(kRESUME_CONVERSATION, "171ap08", 0, false);
			break;

		case 17:
			freeSeries();
			_pollyShould = 18;
			series_play_with_breaks(PLAY8, "171ap09", 0, kCHANGE_POLLY_ANIMATION, 3);
			break;

		case 18:
			_pollyShould = 19;
			series_play_with_breaks(PLAY9, "171ap09", 0x301, kCHANGE_POLLY_ANIMATION, 3);
			series_show("171plid", 0);
			break;

		case 19:
			_G(flags)[V085] = 1;
			_chair.terminate();
			_pollyShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 20:
			_series.show("171ap10", 0x700);
			break;

		case 21:
			freeSeries();
			npc_say(kRESUME_CONVERSATION, "171ap10", 0x700, true, 0, 6);
			break;

		case 22:
			npc_say(5);
			_pollyShould = 23;
			series_play_with_breaks(PLAY10, "171ap10", 0x700, kCHANGE_POLLY_ANIMATION, 3);
			break;

		case 23:
			_pollyShould = 24;
			_series.play("171ap10", 0x700, 0, -1, 6, -1, 100, 0, 0, 0, 6);
			break;

		case 24:
			freeSeries();
			_pollyShould = 25;
			series_play_with_breaks(PLAY11, "171ap10", 0x700, 5, 3);
			break;

		case 25:
			_G(flags)[V086] = 1;
			_chair.terminate();
			series_show("171phone", 0x700);
			_pollyShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 26:
			player_update_info();

			if (_flag1 && (_G(player_info).x != 436 || _G(player_info).y != 288))
				ws_walk(436, 288, 0, kRESUME_CONVERSATION, 2);

			digi_play_loop("171_009", 2, 50);
			_series[0] = series_play("171ap11", 0x8ff, 32, 4, 6, -1);
			_series[1] = series_play("171ap11", 0x900, 0, -1, 6, -1);
			break;

		case 28:
			_pollyShould = 29;
			npc_say(_digi1, kCHANGE_POLLY_ANIMATION, "171ap13", 0x8fe);
			break;

		case 29:
			_pollyShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			player_set_commands_allowed(true);
			break;

		case 30:
			_pollyShould = 32;
			series_play_with_breaks(PLAY15, "171ap16", 0x8ff, kCHANGE_POLLY_ANIMATION, 3);
			break;

		case 31:
			_pollyShould = 32;
			series_play_with_breaks(PLAY16, "171ap17", 0x8ff, 5, 3);
			break;

		case 32:
			_chair.show("171chair", "171chars", 0x900);
			series_play_with_breaks(PLAY17, "171ap18", 0x400, 1, 3);
			break;

		case 33:
			npc_say(kRESUME_CONVERSATION, "171ap13", 0x8fe);
			break;

		case 34:
			_pollyShould = 35;
			npc_say(kCHANGE_POLLY_ANIMATION, "171ap13", 0x8fe);
			break;

		case 35:
			_pollyShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			conv_resume_curr();
			break;

		case 36:
			_pollyShould = 26;
			series_play_with_breaks(PLAY12, "171ap12", 0x8ff, kCHANGE_POLLY_ANIMATION, 3);
			break;

		case 37:
			series_play_with_breaks(PLAY13, "171ap14", 0x8ff, kCHANGE_POLLY_ANIMATION, 3);
			break;

		case 38:
			_pollyShould = 39;
			npc_say(kCHANGE_POLLY_ANIMATION, "171ap13", 0x8fe);
			break;

		case 39:
			freeSeries();
			_chair.show("171chair", "171chars", 0x900);

			if (_series3)
				terminateMachineAndNull(_series3);

			_pollyShould = 40;
			series_play_with_breaks(PLAY14, "171ap15", 0x700, kCHANGE_POLLY_ANIMATION, 2, 8);
			break;

		case 40:
			_chair.terminate();
			_series3 = series_show("171pills", 0x700);
			_pollyShould = 26;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			conv_resume_curr();
			break;

		default:
			term_message("ERROR!!!! polly_should not set!");
			break;
		}
		break;

	case 6:
		_G(flags)[kDisableFootsteps] = 1;
		_G(flags)[V299] = 1;
		_flag1 = true;
		conv_load_and_prepare("conv40", 7);

		switch (_val1) {
		case 10019:
			conv_export_value_curr(1, 0);
			break;
		case 10020:
			conv_export_value_curr(3, 0);
			break;
		case 10021:
			conv_export_value_curr(2, 0);
			break;
		case 10022:
			conv_export_value_curr(4, 0);
			break;
		default:
			conv_export_value_curr(0, 0);
			break;
		}

		conv_export_value_curr(inv_player_has("WHISTLE") ? 1 : 0, 1);
		conv_export_pointer_curr(&_G(flags)[V087], 2);
		conv_export_pointer_curr(&_G(flags)[V088], 3);
		conv_play_curr();
		break;

	case 7:
		_val1 = 0;
		_G(flags)[kDisableFootsteps] = 0;
		_G(flags)[V299] = 0;
		_flag1 = false;
		player_set_commands_allowed(true);
		_pollyShould = 26;

		if (_series) {
			freeSeries();
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			disable_player();
			series_play_with_breaks(PLAY1, "171wi01", 0xc01, 2, 3, 5);
			break;

		case 2:
			disable_player();
			_G(wilbur_should) = kRESUME_CONVERSATION;
			series_play_with_breaks(PLAY2, "171wi02", 0xc01, kCHANGE_WILBUR_ANIMATION, 3, 5, 100, 0, -3);
			break;

		case 3:
			if (_G(flags)[V087]) {
				_digi1 = "171p903";
			} else {
				_digi1 = "1711p902";
				_G(flags)[V087] = 1;
			}

			_pollyShould = 28;
			break;

		case kRESUME_CONVERSATION:
			ws_unhide_walker();

			if (_G(flags)[V092] || _G(flags)[V086])
				player_set_commands_allowed(true);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCALLED_EACH_LOOP:
		if (_doorFlag) {
			frontDoor();
		} else {
			_G(kernel).call_daemon_every_loop = false;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room171::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if ((_val1 == 10019 || _val1 == 10020) &&
		_G(player).walk_x > 378 && _G(player).walk_x <= 518 &&
		_G(player).walk_y >= 300 && _G(player).walk_y <= 354) {
		player_walk_to(_G(player).click_x, 355);
	}

	if (!player_said("GEAR") && player_said("FRONT DOOR")) {
		player_set_facing_at(94, 281);

	} else if (!player_said_any("GEAR", "LOOK AT") && player_said("KITCHEN")) {
		player_set_facing_at(601, 296);

	} else if (player_said("GEAR", "FRONT DOOR")) {
		frontDoor();
		_G(kernel).call_daemon_every_loop = true;
		_G(player).command_ready = false;
	}
}

void Room171::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Do nothing
	} else if (player_said("conv40")) {
		conv40();

	} else if (player_said_any("GEAR", "LOOK AT") && player_said("KITCHEN")) {
		if (_G(flags)[V091]) {
			disable_player_commands_and_fade_init(1019);
		} else {
			player_set_commands_allowed(false);
			_pollyShould = 30;
		}

	} else if (player_said("GEAR", "STAIRS")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("WHISTLE", "AUNT POLLY")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 3;
		wilbur_speech("171w901", kCHANGE_WILBUR_ANIMATION);

	} else if (player_said("TALK", "AUNT POLLY")) {
		kernel_trigger_dispatch_now(6);

	} else if (player_said("AUNT POLLY") && player_said_any("PHONE BILL", "CARROT JUICE")) {
		player_set_commands_allowed(false);
		_digi1 = "171p901";
		_pollyShould = 28;

	} else if (player_said("ROCKER")) {
		if (player_said("LOOK AT")) {
			wilbur_speech(_G(flags)[V092] ? "171w017" : "171w016");

		} else if (player_said("GEAR")) {
			wilbur_speech(_G(flags)[V092] ? "171w015" : "171w018");

		} else {
			return;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room171::loadSeries1() {
	series_load("171ap11");
	series_load("171ap12");
	series_load("171ap13");
	series_load("171ap11s");
	series_load("171ap12s");
	series_load("171ap13s");
	digi_preload("171_009");
}

void Room171::loadSeries2() {
	series_load("171ap01");
	series_load("171ap02");
	series_load("171ap03");
	series_load("171ap01s");
	series_load("171ap02s");
	series_load("171ap03s");
	loadSeries1();
}

void Room171::loadSeries3() {
	series_load("171ap04");
	series_load("171ap05");
	series_load("171ap06");
	series_load("171ap04s");
	series_load("171ap05s");
	series_load("171ap06s");
	loadSeries1();
}

void Room171::loadSeries4() {
	series_load("171ap07");
	series_load("171ap08");
	series_load("171ap09");
	series_load("171ap09s");
	loadSeries1();
}

void Room171::loadSeries5() {
	series_load("171ap10");
	series_load("171ap10s");
	loadSeries1();
}

void Room171::freeSeries() {
	_series.terminate();
}

void Room171::frontDoor() {
	if (player_said_any("GEAR", "LOOK AT") && player_said("FRONT DOOR"))
		_doorFlag = true;

	if (_doorFlag) {
		player_update_info();

		if (_G(player_info).x < 94 && _G(player_info).y < 281 && player_commands_allowed())
			disable_player_commands_and_fade_init(1017);
	}
}

void Room171::conv40() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (!conv_sound_to_play())
		return;

	if (who == 1) {
		wilbur_speech(conv_sound_to_play(), kRESUME_CONVERSATION);

		switch (node) {
		case 6:
			if (entry <= 0 || (entry == 6 || entry == 7))
				_pollyShould = 36;
			break;

		case 7:
			_pollyShould = 36;
			break;

		case 10:
			if (entry == 0)
				_pollyShould = 36;
			break;

		case 15:
			if (entry == 1 || entry == 2 || entry == 5)
				_pollyShould = 36;
			break;

		default:
			break;
		}

	} else {
		switch (node) {
		case 1:
			switch (entry) {
			case 0:
				_pollyShould = 6;
				break;
			case 1:
				_pollyShould = 7;
				break;
			case 2:
				_pollyShould = 8;
				break;
			default:
				break;
			}

			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 2:
			switch (entry) {
			case 0:
				_pollyShould = 11;
				break;
			case 1:
				_pollyShould = 12;
				break;
			case 2:
				_pollyShould = 13;
				break;
			default:
				break;
			}

			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 3:
			switch (entry) {
			case 0:
				_pollyShould = 15;
				break;
			case 1:
				_pollyShould = 17;
				break;
			default:
				break;
			}

			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 4:
			switch (entry) {
			case 0:
				_pollyShould = 21;
				break;
			case 1:
				_pollyShould = 22;
				break;
			default:
				break;
			}

			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 6:
			_pollyShould = (entry == 6) ? 33 : 34;
			break;

		case 8:
			switch (entry) {
			case 0:
				_pollyShould = 33;
				break;
			case 1:
				_pollyShould = 38;
				break;
			default:
				break;
			}

			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		case 9:
			_pollyShould = 34;
			kernel_trigger_dispatch_now(kCHANGE_POLLY_ANIMATION);
			break;

		default:
			_pollyShould = 34;
			break;
		}
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
