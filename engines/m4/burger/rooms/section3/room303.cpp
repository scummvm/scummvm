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

#include "m4/burger/rooms/section3/room303.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room303::SAID[][4] = {
	{ "JUG ",        "303w022", nullptr,   "303w022z" },
	{ "JUG  ",       "303w022", nullptr,   "303w022z" },
	{ "FUEL TANK",   "303w003", "303w004", nullptr    },
	{ "BURNER",      nullptr,   "303w010", nullptr    },
	{ "BOILER",      "303w014", "303w015", nullptr    },
	{ "STOOL",       "303w019", "300w004", "303w020"  },
	{ "DISTILLED CARROT JUICE ",  "303w021", nullptr, nullptr },
	{ "DISTILLED CARROT JUICE  ", "303w021", nullptr, nullptr },
	{ "CONDENSER",   "303w023", "303w024", "303w025"  },
	{ "KEG",         nullptr,   "303w024", "303w025"  },
	{ "CABIN",       "303w028", "300w002", "300w002"  },
	{ "WINDOW",      "303w029", "300w002", "303w030"  },
	{ "WOOD",        "303w031", "303w032", "303w033"  },
	{ "ROCK",        "303w034", "300w002", "300w002"  },
	{ "TREES",       "303w035", "300w002", "300w002"  },
	{ "FORCE FIELD", "303w036", "300w002", "300w002"  },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room303::SERIES1[] = {
	{ 0, "303_001", 2, 255, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room303::PLAY1[] = {
	{ 0, 7, 0, 1, 0, -1, 2048, 0, 0, 0 },
	{ 8, -1, 0, 1, 0, 9, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY2[] = {
	{  0,  5, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{  6,  7, "300w048", 1, 255,   -1,    0, 0, nullptr, 0 },
	{ 10, -1, "303_002", 2, 255,   -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY3[] = {
	{  9, 14, nullptr,   1,   0,   -1, 2050, 0, nullptr, 0 },
	{  0,  8, nullptr,   1,   0,    9, 2050, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY4[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2050, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY5[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY6[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY7[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY8[] = {
	{  0, 31, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{ 32, -1, "303_003", 1, 255,   -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY9[] = {
	{  0, 28, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{ 29, 36, "300_003", 2, 255,   -1,    0, 0, nullptr, 0 },
	{ 37, -1, nullptr,   0,   0,    5,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY10[] = {
	{  0,  6, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{  7,  6, "303_007", 1, 100,   -1,    0, 0, nullptr, 0 },
	{  6,  6, nullptr,   0,   0,   -1,    0, 2, nullptr, 0 },
	{  6,  7, nullptr,   0,   0,   -1,    1, 1, nullptr, 0 },
	{  8, -1, nullptr,   0,   0,   -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

Room303::Room303() : Section3Room() {
	Common::fill(_triggers, _triggers + 5, -1);
}

const char *Room303::getDigi() {
	if (_G(flags)[V118] == 3002) {
		_digiVolume = 125;
		return "303_005";
	} else if (_G(flags)[V117]) {
		return "303_006";
	} else {
		return "300_005";
	}
}

void Room303::init() {
	setupDigi();
	pal_cycle_init(112, 127, 0, -1, -1);

	_val1 = _val2 = 0;
	_series1 = nullptr;
	series_load("303ft");
	series_load("303burnr");
	series_load("303stil");
	series_load("303stilb");
	Common::fill(_triggers, _triggers + 5, -1);

	if (_G(flags)[V117]) {
		_series2 = series_load("303ft_on");
		_series3 = series_play("303ft_on", 0xd00, 0, -1, 0, -1);
		_val3 = 2;
	} else {
		_series2 = series_load("303ft");
		_series3 = series_show("303ft", 0xd00);
		_val3 = 4;
	}

	if (_G(flags)[V118] == 3002) {
		_series4 = series_load("303burn");
		_series5 = series_play("303burn", 0xd14, 0, -1, 0, -1);
		_val4 = 7;
	} else {
		_series4 = series_load("303burnr");
		_series5 = series_show("303burnr", 0xd14);
		_val4 = 9;
	}

	_series6 = series_load("303stil");
	_series7 = series_show("303stil", 0xd10);
	series_show("303stilb", 0xd15);

	_val5 = 0;
	kernel_trigger_dispatch_now(11);

	switch (_G(flags)[V121]) {
	case 3001:
		_val6 = _G(flags)[V122] ? 32 : 30;
		kernel_trigger_dispatch_now(10);
		break;

	case 3002:
		_val6 = _G(flags)[V122] ? 33 : 31;
		kernel_trigger_dispatch_now(10);
		break;

	default:
		break;
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		if (_G(flags)[V117])
			_timer = timer_read_60();
		break;

	case 302:
		if (_G(flags)[V125]) {
			ws_demand_location(51, 276, 3);
			kernel_trigger_dispatch_now(3);
		} else {
			ws_demand_location(245, 373, 1);
			kernel_trigger_dispatch_now(2);
		}
		break;

	default:
		player_set_commands_allowed(true);
		if (_G(flags)[V117])
			_timer = timer_read_60();

		ws_demand_location(290, 325, 1);
		break;
	}
}

void Room303::daemon() {
	doDaemon(_G(kernel).trigger);
}

void Room303::doDaemon(int trigger) {
	switch (trigger) {
	case 1:
		for (_ctr = 0; _ctr < 5; ++_ctr) {
			if (_triggers[_ctr] != -1) {
				kernel_trigger_dispatch_now(_triggers[_ctr]);
				_triggers[_ctr] = -1;
			}
		}
		break;

	case 2:
		if (_G(flags)[V115] > 1) {
			player_set_commands_allowed(true);
			ws_walk(290, 325, 0, 4, 1);
		} else {
			_G(wilbur_should) = 18;
			player_set_commands_allowed(false);
			ws_walk(290, 325, 0, kCHANGE_WILBUR_ANIMATION, 1);
		}
		break;

	case 3:
		if (_G(flags)[V115] > 1) {
			player_set_commands_allowed(true);
			ws_walk(261, 276, 0, 4, 3);
		} else {
			_G(wilbur_should) = 18;
			player_set_commands_allowed(false);
			ws_walk(261, 276, 0, kCHANGE_WILBUR_ANIMATION, 3);
		}
		break;

	case 4:
		player_set_commands_allowed(true);
		break;

	case 5:
		setupDigi();
		break;

	case 6:
		switch (_val7) {
		case 25:
			if (_G(flags)[V119] && _G(flags)[V118] == 3002) {
				terminateMachineAndNull(_series7);
				series_unload(_series6);

				_G(flags)[V120] = 1;
				_val7 = 26;
				digi_preload_stream_breaks(SERIES1);
				series_stream_with_breaks(SERIES1, "303dist", 6, 0xc10, 6);
				player_set_commands_allowed(false);
				intr_cancel_sentence();
				ws_walk(330, 350, nullptr, -1, 2);
			}
			break;

		case 26:
			digi_unload_stream_breaks(SERIES1);
			player_set_commands_allowed(true);

			_G(flags)[V119] = 0;
			_series6 = series_load("303stil");
			_series7 = series_show("303stil", 0xd10);

			if (_G(flags)[V121] == 3002) {
				_val8 = 27;
				_G(flags)[V122] = 1;
				terminateMachineAndNull(_series8);
				_val6 = 33;
				kernel_trigger_dispatch_now(10);
			} else {
				_val8 = 28;
			}

			kernel_trigger_dispatch_now(9);
			break;

		default:
			break;
		}
		break;

	case 7:
		switch (_val3) {
		case 1:
			freeSeries1();
			series_play_with_breaks(PLAY6, "303wi07", 0xd00, 7, 3);

			_val3 = 2;
			_G(flags)[V117] = 1;
			setupDigi();
			ws_hide_walker();

			_G(wilbur_should) = 19;
			_timer = timer_read_60();
			break;

		case 2:
			_series2 = series_load("303ft_on");
			_series3 = series_play("303ft_on", 0xd00, 0, -1, 0, -1);

			if (_G(wilbur_should) == 19)
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 3:
			freeSeries1();
			series_play_with_breaks(PLAY7, "303wi08", 0xd00, 7, 3);
			_val3 = 4;
			ws_hide_walker();
			_G(wilbur_should) = 20;
			break;

		case 4:
			_series2 = series_load("303ft");
			_series3 = series_show("303ft", 0xd00);
			_G(flags)[V117] = 0;
			setupDigi();

			if (_G(flags)[V118] == 3002) {
				_val4 = 8;
				doDaemon(8);
			}
			if (_G(wilbur_should) == 20)
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 8:
		switch (_val4) {
		case 5:
			freeSeries2();
			player_update_info();

			if (timer_read_60() - _timer > 900) {
				_G(flags)[V118] = 3003;
				series_play_with_breaks(PLAY8, "303wi10", 0xd01, 3007, 1);
			} else {
				_G(flags)[V118] = 3002;
				series_play_with_breaks(PLAY9, "303wi02", 0xd01, 8, 3);
			}

			_val4 = 7;
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			break;

		case 6:
			freeSeries2();
			series_play_with_breaks(PLAY9, "303wi09", 0xd01, 8, 3);
			_val4 = 9;
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			break;

		case 7:
			setupDigi();
			_series4 = series_load("303burn");
			_series5 = series_play("303burn", 0xd14, 0, -1, 0, -1);

			if (_G(wilbur_should) == 10001) {
				ws_unhide_walker();
				_G(wilbur_should) = 10002;
				player_set_commands_allowed(true);
			}

			_val7 = 25;
			kernel_timing_trigger(180, 6);
			break;

		case 8:
			freeSeries2();
			_series4 = series_load("303burnr");
			_series5 = series_show("303burnr", 0xd14);
			_G(flags)[V118] = 3001;
			setupDigi();
			break;

		case 9:
			_series4 = series_load("303burnr");
			_series5 = series_show("303burnr", 0xd14);
			_G(flags)[V118] = 3001;

			if (_G(wilbur_should) == 10001) {
				ws_unhide_walker();
				_G(wilbur_should) = 10002;
				player_set_commands_allowed(true);
			}
			break;

		default:
			break;
		}
		break;

	case 9:
		switch (_val8) {
		case 27:
			if (_val5 < 15) {
				++_val5;
				_val8 = 27;
				terminateMachineAndNull(_series9);
				_series9 = series_play("303cj01", 0xd15, 0, 9, 8);
			}
			break;
		case 28:
			if (_val5 < 15) {
				++_val5;
				_val8 = 28;
				terminateMachineAndNull(_series9);
				_series9 = series_play("303cj02", 0xd15, 0, 9, 6);
			}
			break;

		default:
			break;
		}
		break;

	case 10:
		switch (_val6) {
		case 30:
			inv_move_object("JUG", 303);
			inv_move_object("DISTILLED CARROT JUICE", 303);

			_G(flags)[V121] = 3001;
			_series8 = series_show("303juga", 0xbff);
			hotspot_set_active("JUG  ", true);
			break;

		case 31:
			inv_move_object("JUG", 303);
			inv_move_object("DISTILLED CARROT JUICE", 303);

			_G(flags)[V121] = 3002;
			_series8 = series_show("303JUG", 0xd14);
			hotspot_set_active("JUG ", true);
			break;

		case 32:
			inv_move_object("JUG", 303);
			inv_move_object("DISTILLED CARROT JUICE", 303);
			_G(flags)[V122] = 1;
			_G(flags)[V121] = 3001;

			_series8 = series_show("303juga", 0xbff);
			hotspot_set_active("DISTILLED CARROT JUICE  ", true);
			break;

		case 33:
			inv_move_object("JUG", 303);
			inv_move_object("DISTILLED CARROT JUICE", 303);
			_G(flags)[V122] = 1;
			_G(flags)[V121] = 3002;

			_series8 = series_show("303JUG", 0xd14);
			hotspot_set_active("DISTILLED CARROT JUICE ", true);
			hotspot_set_active("JUG ", true);
			break;

		case 34:
			_G(flags)[V121] = 3003;
			inv_give_to_player(_G(flags)[V122] ? "DISTILLED CARROT JUICE" : "JUG");
			break;

		default:
			break;
		}
		break;

	case 11:
		terminateMachineAndNull(_series8);
		hotspot_set_active("DISTILLED CARROT JUICE ", false);
		hotspot_set_active("DISTILLED CARROT JUICE  ", false);
		hotspot_set_active("JUG ", false);
		hotspot_set_active("JUG  ", false);
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 11:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			_triggers[0] = kCHANGE_WILBUR_ANIMATION;

			if (_val5 > 0 && _val5 < 15) {
				_val8 = 27;
				_G(flags)[V122] = 1;
			} else {
				_val8 = 29;
			}

			_val6 = _G(flags)[V122] ? 33 : 31;
			_triggers[1] = 10;
			series_play_with_breaks(PLAY1, "303wi4b", 0x100, 1, 3);
			break;

		case 12:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 10001;
			_triggers[0] = kCHANGE_WILBUR_ANIMATION;
			kernel_trigger_dispatch_now(11);

			_val6 = 34;
			_triggers[1] = 10;
			_val8 = (_val5 > 0 && _val5 < 15) ? 28 : 29;
			series_play_with_breaks(PLAY3, "303wi4b", 0x100, 1, 3);
			break;

		case 13:
			ws_hide_walker();
			player_set_commands_allowed(false);
			kernel_trigger_dispatch_now(11);
			_G(wilbur_should) = 10001;
			_triggers[0] = kCHANGE_WILBUR_ANIMATION;
			_triggers[1] = 10;
			_val6 = 34;
			series_play_with_breaks(PLAY4, "303wi4a", 0x100, 1, 3);
			break;

		case 14:
			player_set_commands_allowed(false);
			ws_hide_walker();

			_G(wilbur_should) = 10001;
			_triggers[0] = kCHANGE_WILBUR_ANIMATION;
			_val6 = _G(flags)[V122] ? 32 : 30;
			_triggers[1] = 10;
			series_play_with_breaks(PLAY5, "303wi4a", 0x100, 1, 3);
			break;

		case 15:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 23;
			series_play_with_breaks(PLAY10, "303wi15", 0x100, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 16:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_G(wilbur_should) = 17;
			series_play_with_breaks(PLAY2, "303wi03", 0x100, kCHANGE_WILBUR_ANIMATION, 3);
			break;

		case 17:
			_G(flags)[V119] = 1;
			_G(flags)[V126] = 1;
			inv_put_thing_in("CARROT JUICE", NOWHERE);
			inv_give_to_player("BOTTLE");
			_val7 = 25;
			kernel_trigger_dispatch_now(6);
			enable_player();
			break;

		case 18:
			_G(wilbur_should) = 10001;
			wilbur_speech(_G(flags)[V115] ? "303w002" : "303w001");
			_G(flags)[V115]++;
			break;

		case 19:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;

			if (_G(flags)[V116]) {
				wilbur_speech("303w006");
			} else {
				wilbur_speech("303w005");
				_G(flags)[V116] = 1;
			}
			break;

		case 20:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("303w007");
			break;

		case 21:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech(_G(flags)[V118] == 3001 ? "303w008" : "303w009");
			break;

		case 22:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech(_G(flags)[V118] == 3001 ? "303w011" : "303w012");
			break;

		case 23:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("303w016");
			break;

		case 24:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			wilbur_speech("303w017");
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

void Room303::pre_parser() {
	if (player_said("GEAR", "BOILER") && !_G(flags)[V119]) {
		_G(wilbur_should) = 15;
		player_hotspot_walk_override(409, 312, 3, kCHANGE_WILBUR_ANIMATION);
		_G(player).command_ready = false;
	} else {
		if (player_said("FRONT YARD") && !player_said_any("LOOK AT", "WALK TO", "GEAR", "ENTER"))
			player_set_facing_hotspot();

		if (player_said("FRONT YARD ") && !player_said_any("LOOK AT", "WALK TO", "GEAR", "ENTER"))
			player_set_facing_hotspot();
	}
}

void Room303::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("FRONT YARD") && player_said_any("LOOK AT", "WALK TO", "GEAR", "ENTER")) {
		frontYard();
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 6, 3001);
		_G(flags)[V125] = 0;
	} else if (player_said("FRONT YARD ") && player_said_any("LOOK AT", "WALK TO", "GEAR", "ENTER")) {
		frontYard();
		player_set_commands_allowed(false);
		pal_fade_init(_G(kernel).first_fade, 255, 0, 6, 3001);
		_G(flags)[V125] = 1;
	} else if (player_said("DISTILLED CARROT JUICE", "BURNER")) {
		wilbur_speech("300w033");
	} else if (player_said("DISTILLED CARROT JUICE", "BOILER")) {
		wilbur_speech("300w034");
	} else if (player_said("JUG", "BOILER")) {
		wilbur_speech("300w027");
	} else if (player_said("JUG", "STOOL") || player_said("DISTILLED CARROT JUICE", "STOOL")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 11;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("JUG", "JUG ", "JUG  ") && player_said("CARROT JUICE")) {
		wilbur_speech("300w046");
	} else if (player_said("JUG", "STUMP") || player_said("DISTILLED CARROT JUICE", "STUMP")) {
		player_set_commands_allowed(false);
		_G(wilbur_should) = 14;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("BOTTLE", "BOILER")) {
		wilbur_speech("300w074");
	} else if (player_said("BOTTLE", "STOOL")) {
		wilbur_speech("300w075");
	} else if (player_said("CARROT JUICE", "BURNER")) {
		wilbur_speech("300w047");
	} else if (player_said("CARROT JUICE", "STOOL")) {
		wilbur_speech("300w049");
	} else if (player_said("MATCHES", "STUMP") || player_said("MATCHES", "WOOD")) {
		wilbur_speech("300w019");
	} else if (player_said("GEAR", "FUEL TANK")) {
		player_set_commands_allowed(false);
		_val3 = _G(flags)[V117] ? 3 : 1;
		kernel_trigger_dispatch_now(7);
	} else if (player_said("LOOK AT", "BURNER")) {
		_G(wilbur_should) = 21;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("GEAR", "BURNER")) {
		_G(wilbur_should) = 22;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("MATCHES", "BURNER")) {
		if (_G(flags)[V118] == 3001) {
			player_set_commands_allowed(false);
			_val4 = _G(flags)[V117] ? 5 : 6;
			kernel_trigger_dispatch_now(8);
		}
	} else if (player_said("GEAR", "BOILER")) {
		if (_G(flags)[V119])
			_G(wilbur_should) = 24;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);		
	} else if (player_said("BOILER", "CARROT JUICE")) {
		_G(wilbur_should) = 16;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "JUG ") || player_said("TAKE", "DISTILLED CARROT JUICE ")) {
		_G(wilbur_should) = 12;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("TAKE", "JUG  ") || player_said("TAKE", "DISTILLED CARROT JUICE  ")) {
		_G(wilbur_should) = 13;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else if (player_said("LOOK AT", "KEG")) {
		wilbur_speech(_G(flags)[V120] ? "303w027" : "303w026");
	} else if (player_said("BURNER")) {
		wilbur_speech("303w013");
	} else if (player_said("BOILER")) {
		wilbur_speech("303w018");
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room303::frontYard() {
	term_message("------- %ld %ld %ld", _G(flags)[V119], _G(flags)[V118], _G(flags)[V121]);

	if (_G(flags)[V119] && _G(flags)[V118] == 3002 && _G(flags)[V121] == 3002) {
		term_message("jug filled with distilled juice!!");
		_G(flags)[V122] = 1;
		_G(flags)[V120] = 1;
		_G(flags)[V119] = 0;
	}
}

void Room303::freeSeries1() {
	if (_series3)
		terminateMachineAndNull(_series3);
	if (_series2)
		series_unload(_series2);
}

void Room303::freeSeries2() {
	if (_series5)
		terminateMachineAndNull(_series5);
	if (_series4)
		series_unload(_series4);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
