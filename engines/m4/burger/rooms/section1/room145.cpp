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

#include "m4/burger/rooms/section1/room145.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room145::SAID[][4] = {
	{ "TOUR BUS",      "145w001", "145w002", "145w003" },
	{ "INSTRUMENTS",   "145w005", "145w006", "145w006" },
	{ "AMPLIFIER ",    "145w009", nullptr,   "145w012" },
	{ "VIPE",          "145w013", "145w004", "145w004" },
	{ "ROXY",          "145w016", "145w004", "145w004" },
	{ "DRUMZ",         "145w019", "145w004", "145w004" },
	{ "MAP",           "145w022", "145w002", "145w023" },
	{ "VERA'S DINER",  "145w024", "145w004", "145w004" },
	{ "VERA'S DINER ", "145w024", "145w004", nullptr   },
	{ nullptr, nullptr, nullptr, nullptr }
};

const WilburMatch Room145::MATCH[] = {
	{ nullptr, "TOUR BUS", 1, 0, 0, &_state1, 12 },
	{ "TALK", "ROXY", 10016, 0, 0, &Vars::_wilbur_should, 11 },
	{ "AMPLIFIER", "ROXY", 1, 0, 0, &_state1, 15 },
	{ nullptr, "ROXY", 1, 0, 0, &_state1, 17 },
	{ "TALK" , "DRUMZ", 10, 0, 0, 0, 0 }, 
	{ "LAXATIVE" , "DRUMZ", 10016, 0, 0, &Vars::_wilbur_should, 3 },
	{ nullptr, "DRUMZ", 1, 0, 0, &_state1, 16 },
	{ "TALK" , "VIPE", 10016, 0, 0, &Vars::_wilbur_should, 10 },
	{ "AMPLIFIER", "VIPE", 1, 0, 0, &_state1, 15 },
	{ nullptr, "VIPE", 1, 0, 0, &_state1, 16 },
	{ "AMPLIFIER" , "INSTRUMENTS", 1, 0, 0, &_state1, 13 },
	{ nullptr, "INSTRUMENTS", 1, 0, 0, &_state1, 14 },
	{ "TAKE", "AMPLIFIER ", 10016, &Flags::_flags[V067], 0, &Vars::_wilbur_should, 1 },
	{ "TAKE", "AMPLIFIER ", 10016, &Flags::_flags[V067], 1, &Vars::_wilbur_should, 8 },
	{ nullptr, "AMPLIFIER ", 1, 0, 0, &_state1, 14 },
	{ nullptr, "MAP", 1, 0, 0, &_state1, 18 },
	WILBUR_MATCH_END
};

const seriesPlayBreak Room145::PLAY1[] = {
	{ 0, 6, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 6, 6, nullptr,   0,   0, -1, 0,  6, nullptr, 0 },
	{ 6, 6, "145w010", 1, 255, -1, 0, -1, nullptr, 0 },
	{ 0, 6, nullptr,   0,   0, -1, 2,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY2[] = {
	{  0, 23, nullptr,   0,   0, -1, 0, 0, nullptr, 0 },
	{ 24, -1, "145w011", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY3[] = {
	{ 15, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY4[] = {
	{ 0, 3, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{ 4, 4, nullptr,   0,   0, -1, 0,  4, nullptr, 0 },
	{ 5, 9, "145w021", 1, 255, -1, 4, -1, nullptr, 0 },
	{ 4, 4, nullptr,   0,   0, -1, 0,  4, nullptr, 0 },
	{ 0, 3, nullptr,   0,   0, -1, 2,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY5[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY6[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY7[] = {
	{ 0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 2, -1, nullptr, 0, 0, -1, 0, 4, nullptr, 0 },
	{ 0,  1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY8[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY9[] = {
	{ 0, -1, "145_004", 2, 125, -1, 1, 0, &_state2, 0 },
	{ 0, -1, nullptr,   0,   0, -1, 0, 0, &_state2, 1 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY10[] = {
	{ 0, -1, nullptr, 0, 0, -1, 3, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY11[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY12[] = {
	{ 0,  1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 2,  4, nullptr, 0, 0, -1, 1, 2, nullptr, 0 },
	{ 5, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY13[] = {
	{  0,  5, nullptr,   0,   0,    -1, 0, 0, nullptr, 0 },
	{  6, 16, nullptr,   0,   0, 10016, 0, 0, nullptr, 0 },
	{ 17, 20, "145_002", 1, 255,    -1, 0, 0, nullptr, 0 },
	{ 21, 25, "145_005", 1, 255,    -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY14[] = {
	{ 26, -1, "145_001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY15[] = {
	{ 0, 3, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY16[] = {
	{ 0, 3, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY17[] = {
	{ 1, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY18[] = {
	{ 0, -1, "145r901", 2, 125, -1, 4, -1, &_state3, 1 },
	{ 0, -1, "145r902", 2, 125, -1, 4, -1, &_state3, 2 },
	{ 0, -1, "145r903", 2, 125, -1, 4, -1, &_state3, 3 },
	{ 0, -1, "145r904", 2, 125, -1, 4, -1, &_state3, 4 },
	{ 0, -1, "145r905", 2, 125, -1, 4, -1, &_state3, 5 },
	{ 0, -1, "145r906", 2, 125, -1, 4, -1, &_state3, 6 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY19[] = {
	{ 1, -1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY20[] = {
	{ 1, -1, nullptr, 0, 0, -1, 0, 2, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY21[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY22[] = {
	{ 0, -1, nullptr, 0, 0, -1, 1, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room145::PLAY23[] = {
	{ 0, -1, nullptr, 0, 0, -1, 2, 0, nullptr, 0 },
	PLAY_BREAK_END
};

long Room145::_state1;
long Room145::_state2;
long Room145::_state3;

Room145::Room145() : Room() {
	_state1 = 0;
	_state2 = 0;
	_state3 = 0;
}

void Room145::init() {
	digi_preload("145_006");
	digi_play_loop("145_006", 3, 255, -1);
	_state2 = 0;

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		break;

	case 142:
		ws_demand_location(680, 340, 9);
		ws_walk(570, 330, 0, -1, -1);
		break;

	default:
		ws_demand_location(570, 330, 9);
		break;
	}

	kernel_timing_trigger(600, 7);

	if (_G(flags)[V067]) {
		series_show("145drum", 0xa01);
		hotspot_set_active("DRUMZ", false);
	} else {
		loadDrum();
		_walk1 = intr_add_no_walk_rect(140, 295, 240, 325, 139, 326);
		_val1 = 19;
		kernel_trigger_dispatch_now(2);
	}

	loadRx();

	_walk2 = intr_add_no_walk_rect(423, 308, 540, 340, 541, 341);
	_val2 = 101;
	_val3 = 33;
	kernel_trigger_dispatch_now(4);

	_walk3 = intr_add_no_walk_rect(341, 326, 480, 365, 481, 366);
	_val4 = 43;
	kernel_trigger_dispatch_now(5);

	if (inv_player_has("AMPLIFIER")) {
		hotspot_set_active("AMPLIFIER ", false);
	} else {
		_amplifier = series_show("145amp", 0xa01);
	}
}

void Room145::daemon() {
	// TODO
}

void Room145::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(flags)[V067] && _G(player).walk_x >= 140 && _G(player).walk_x <= 240 &&
			_G(player).walk_y >= 295 && _G(player).walk_y <= 325)
		player_walk_to(139, 326);

	if (_G(player).walk_x >= 423 && _G(player).walk_x <= 540 &&
		_G(player).walk_y >= 308 && _G(player).walk_y <= 340)
		player_walk_to(541, 341);

	if (_G(player).walk_x >= 341 && _G(player).walk_x <= 480 &&
		_G(player).walk_y >= 326 && _G(player).walk_y <= 365)
		player_walk_to(481, 366);

	if (player_said("GEAR") && player_said("VERA'S DINER "))
		player_set_facing_at(680, 340);
}

void Room145::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("conv21")) {
		conv21();
	} else if (player_said("conv22")) {
		conv22();
	} else if (player_said("conv23")) {
		conv23();
	} else if (player_said("GEAR", "VERA'S DINER ")) {
		disable_player_commands_and_fade_init(1014);
	} else if (!_G(walker).wilbur_match(MATCH)) {
		return;
	}

	_G(player).command_ready = false;
}

void Room145::loadDrum() {
	static const char *NAMES[12] = {
		"145dz01", "145dz02", "145dz03", "145dz04", "145dz05",
		"145dz06", "145dz01s", "145dz02s", "145dz03s", "145dz04s",
		"145dz05s", "145dz06s"
	};
	for (int i = 0; i < 12; ++i)
		series_load(NAMES[i]);

	digi_preload("145_003");
	digi_preload("145_004");
}

void Room145::loadRx() {
	static const char *NAMES[8] = {
		"145rx01", "145rx02", "145rx04", "145rx06",
		 "145rx01s", "145rx02s", "145rx04s", "145rx06s"
	};
	for (int i = 0; i < 8; ++i)
		series_load(NAMES[i]);
}

void Room145::conv21() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who == 1) {
			wilbur_speech(sound, 10001);
		} else {
			switch (node) {
			case 2:
			case 7:
			case 12:
				loadVp4();
				break;

			case 3:
			case 4:
			case 5:
			case 6:
			case 8:
			case 11:
				loadRx4();
				break;

			case 9:
				if (entry <= 0) {
					loadRx4();
				} else if (entry == 2) {
					loadVp4();
				}
				break;

			case 10:
				if (entry <= 0) {
					loadVp4();
				} else if (entry == 1) {
					loadRx4();
				}
				break;

			default:
				break;
			}
		}
	}
}

void Room145::conv22() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();

	if (sound) {
		if (who == 1)
			wilbur_speech(sound, 10001);
		else
			loadRx4();
	}
}

void Room145::conv23() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who == 1) {
			wilbur_speech(sound, 10001);
		} else {
			switch (node) {
			case 2:
				switch (entry) {
				case 0:
					_val1 = 20;
					break;
				case 1:
					_val1 = 21;
					break;
				case 2:
				case 3:
					_val1 = 22;
					break;
				case 4:
					_val1 = 21;
					break;
				case 5:
					_val1 = 20;
					break;
				case 6:
					_val1 = 20;
					break;
				case 7:
					conv_resume_curr();
					break;
				default:
					break;
				}
				break;

			case 5:
				_val1 = 20;
				break;

			default:
				break;
			}
		}
	}
}

void Room145::loadVp4() {
	_vp.terminate();
	_val4 = 39;
	digi_play(conv_sound_to_play(), 1, 255, 5);
	_vp.play("145vp04", 0x300, 4, -1);
}

void Room145::loadRx4() {
	_rx.terminate();
	_val3 = 39;
	digi_play(conv_sound_to_play(), 1, 255, 4);
	_rx.play("145rx04", 0x900, 4, -1);
}

void Room145::loadSeries1() {
	series_load("145wi02");
	series_load("145wi02s");
	series_load("145wi03");
	series_load("145wi03s");
	series_load("145dz08");
	series_load("145dz08s");
}

int Room145::getRandomState() const {
	switch (imath_ranged_rand(1, 10)) {
	case 1:
		return 24;
	case 2:
		return 25;
	default:
		return 19;
	}
}

void Room145::resetRoxy() {
	switch (_val5) {
	case 31:
		_rx.terminate();
		_val3 = 32;
		kernel_trigger_dispatch_now(4);
		break;

	case 33:
		_rx.terminate();
		_val3 = 33;
		kernel_trigger_dispatch_now(4);
		break;

	default:
		term_message("Can't move roxy to home state, just have to wait!");
		break;
	}
}

void Room145::playRandomDigi1() {
	_state2 = 1;
	static const char *NAMES[4] = { "145v907", "145v908", "145v909", "145v910" };

	_digiName1 = NAMES[imath_ranged_rand(0, 3)];
	digi_preload(_digiName1);
	digi_play(_digiName1, 2, 125, 6);
}

void Room145::playRandomDigi2() {
	_state2 = 1;
	static const char *NAMES[13] = {
		"145r907", "145r908", "145r909", "145r910", "145r911",
		"145r912", "145r913", "145r914", "145r915", "145r916",
		"145r917", "145r918", "145r919"
	};

	_digiName1 = NAMES[imath_ranged_rand(0, 12)];
	_val3 = 34;
	digi_preload(_digiName1);
	npc_say(_digiName2, 4, "145rx01", 0x900, 1, 3, 7, 2);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
