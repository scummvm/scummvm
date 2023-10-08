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

#include "m4/burger/rooms/section4/room407.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room407::SAID[][4] = {
	{ "TOUR BUS",     "407w001", "407w002", "407w003" },
	{ "ROXY",         nullptr,   "400w001", "400w001" },
	{ "DRUMZ",        "407w007", "400w001", "400w001" },
	{ "INSTRUMENTS",  "407w009", "407w010", "407w010" },
	{ "FORCE FIELD",  "400w005", nullptr,   "400w001" },
	{ "FORCE FIELD ", "400w005", nullptr,   "400w001" }
};

const WilburMatch Room407::MATCH[] = {
	{ "TALK", "DRUMZ",           11, 0, 0, nullptr, 0 },
	{ "AMPLIFIER", "DRUMZ",       4, 0, 0, &_state1, 4 },
	{ nullptr, "DRUMZ",           4, 0, 0, &_state1, 7 },
	{ "LOOK AT", "ROXY",      10016, 0, 0, &Vars::_wilbur_should, 1 },
	{ "TALK", "ROXY",         10016, 0, 0, &Vars::_wilbur_should, 2 },
	{ "AMPLIFIER", "ROXY",        4, 0, 0, &_state1, 4 },
	{ "DOG COLLAR", "ROXY",       4, 0, 0, &_state1, 5 },
	{ nullptr, "ROXY",            4, 0, 0, &_state1, 6 },
	{ "AMPLIFIER", "INSTRUMENTS", 4, 0, 0, &_state1, 8 },
	{ nullptr, "INSTRUMENTS",     4, 0, 0, &_state1, 9 },
	{ nullptr, "TOUR BUS",        4, 0, 0, &_state1, 3 },
	{ nullptr, nullptr, -1, 0, 0, nullptr, 0 }
};

const seriesPlayBreak Room407::PLAY1[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY2[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY3[] = {
	{ 0, 1, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 2, -1, 0, 0, 0, -1, 0, 4, 0, 0 },
	{ 0, 1, 0, 0, 0, -1, 2, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY4[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY5[] = {
	{ 0, -1, "145_004", 2, 255, -1, 1, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY6[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY7[] = {
	{ 0, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY8[] = {
	{ 0, -1, "407r901", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY9[] = {
	{ 0, -1, "407r903", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY10[] = {
	{ 0, -1, 0, 0, 0, -1, 2, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY11[] = {
	{ 0, 2, 0, 0, 0, -1, 0, 0, 0, 0 },
	{ 3, -1, "407r904x", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY12[] = {
	{ 0, 6, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY13[] = {
	{ 7, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY14[] = {
	{ 0, -1, "407r904y", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY15[] = {
	{ 0, 3, "407v901", 1, 255, -1, 4, -1, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY16[] = {
	{ 5, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY17[] = {
	{ 1, -1, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room407::PLAY18[] = {
	{ 0, 1, "999_003", 1, 255, -1, 0, 0, 0, 0 },
	{ 2, 9, 0, 0, 0, 7, 0, 0, 0, 0 },
	{ 10, 11, "999_003", 1, 255, -1, 0, 0, 0, 0 },
	{ 12, -1, 0, 0, 0, 9, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

long Room407::_state1;


Room407::Room407() : Section4Room() {
	_state1 = 0;
}

void Room407::init() {
	digi_preload("400_001");
	digi_play_loop("400_001", 3, 125, -1);
	pal_cycle_init(112, 127, 6);

	if (!_G(flags)[V181] && _G(flags)[V171] == 4003) {
		_G(flags)[V298] = 1;
		player_set_commands_allowed(false);
	}

	_flag1 = false;
	if (_G(flags)[V181]) {
		hotspot_set_active("DRUMZ", false);
		hotspot_set_active("VIPE", false);
		hotspot_set_active("ROXY", false);

	} else {
		loadSeries();
		_walk1 = intr_add_no_walk_rect(140, 295, 240, 325, 139, 326);
		_val2 = 10;
		kernel_trigger_dispatch_now(7);

		_val3 = 22;
		kernel_trigger_dispatch_now(8);

		if (_G(flags)[V171] == 4003)
			_vp02.show("407vp02", 0x300);
	}

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		break;

	case 402:
		ws_demand_location(43, 302);
		setDest();
		break;

	case 405:
		ws_demand_location(578, 296);
		setDest();
		break;

	default:
		ws_demand_location(520, 330, 9);
		break;
	}
}

void Room407::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		poof(2);
		break;

	case 2:
		disable_player_commands_and_fade_init(_val1);
		break;

	case 3:
		_G(walker).wilbur_poof();

		if (!_G(flags)[V181] && _G(flags)[V171] == 4003) {
			ws_unhide_walker();
		} else {
			enable_player();
		}
		break;

	case 4:
		switch (_state1) {
		case 3:
			wilbur_speech("400w001");
			break;
		case 4:
			wilbur_speech("407w004");
			break;
		case 5:
			wilbur_speech("407w005");
			break;
		case 6:
			wilbur_speech("407w006");
			break;
		case 7:
			wilbur_speech("407w008");
			break;
		case 8:
			wilbur_speech("407w011");
			break;
		case 9:
			wilbur_speech("407w012");
			break;
		default:
			break;
		}
		break;

	case 5:
		_val2 = 17;

		if (_val4) {
			kernel_timing_trigger(120, 7);
		} else {
			_digiName = Common::String::format("407r902%c", 'a' + imath_ranged_rand(1, 25));

			_flag1 = true;
			digi_preload(_digiName);
			digi_play(_digiName.c_str(), 2, 180, 6);
		}
		break;

	case 6:
		digi_unload(_digiName);
		kernel_trigger_dispatch_now(7);
		break;

	case 7:
		switch (_val2) {
		case 10:
			if (!digi_play_state(2))
				digi_play_loop("407_001", 2);

			_val2 = getRandomState();
			series_play_with_breaks(PLAY1, "407dz01", 0xa01, 7, 3);
			break;

		case 11:
			_val2 = 14;
			series_play_with_breaks(PLAY2, "407dz02", 0xa01, 7, 3);
			break;

		case 12:
			_val2 = 14;
			series_play_with_breaks(PLAY3, "407dz03", 0xa01, 7, 3);
			break;

		case 13:
			digi_stop(2);
			_val2 = 14;
			series_play_with_breaks(PLAY4, "407dz04", 0xa01, 7, 3);
			break;

		case 14:
			_val2 = getRandomState();
			kernel_trigger_dispatch_now(7);
			kernel_trigger_dispatch_now(10001);
			break;

		case 15:
			digi_stop(2);
			_val2 = getRandomState();
			series_play_with_breaks(PLAY5, "407dz05", 0xa01, 7, 3);
			break;

		case 16:
			digi_stop(2);
			_dzS1 = series_load("407dz06");
			_dzS2 = series_load("407dz06s");
			_dz.show("407dz06", 0xa01, 18);
			kernel_trigger_dispatch_now(5);
			break;

		case 17:
			_flag1 = false;
			freeDz();
			_val2 = getRandomState();
			series_play_with_breaks(PLAY6, "407dz06", 0xa01, 7, 3);
			break;

		case 18:
			_flag1 = false;
			freeDz();
			_val2 = 19;
			series_play_with_breaks(PLAY6, "407dz06", 0xa01, 7, 3);
			break;

		case 19:
			if (!digi_play_state(2))
				digi_play_loop("407_001", 2);

			conv88();
			kernel_trigger_dispatch_now(7);
			break;

		case 20:
			digi_stop(2);
			_dz.play("407dz02", 0xa01, 16, -1, 6, 0, 100, 0, 0, 0, 2);
			break;

		case 21:
			_dz.terminate();
			hotspot_set_active("DRUMZ", false);
			intr_remove_no_walk_rect(_walk1);
			break;

		default:
			break;
		}
		break;

	case 8:
		switch (_val3) {
		case 22:
			_rx = series_show("407rx02", 0x901);
			break;

		case 23:
			terminateMachineAndNull(_rx);

			switch (_val4) {
			case 1:
				_val3 = 26;
				break;
			case 2:
				_val3 = 24;
				break;
			case 3:
				_val3 = 27;
				break;
			default:
				break;
			}
			break;

		case 24:
			_rx = series_show("407rx02", 0x901, 0, -1, -1, 4);
			kernel_trigger_dispatch_now(10);
			break;

		case 25:
			terminateMachineAndNull(_rx);
			_rx = series_show("407rx02", 0x901, 0, -1, -1, 4);
			conv_resume_curr();
			break;

		case 26:
			terminateMachineAndNull(_rx);
			_val3 = 30;
			series_play_with_breaks(PLAY9, "407rx04", 0x901, 8, 2);
			break;

		case 27:
			_val3 = 28;
			series_play_with_breaks(PLAY8, "407rx04", 0x901, 8, 2);
			break;

		case 28:
			_val3 = 29;
			series_play_with_breaks(PLAY10, "407rx02", 0x901, 8, 2);
			break;

		case 29:
			_val3 = 22;
			kernel_trigger_dispatch_now(8);
			kernel_trigger_dispatch_now(1);
			break;

		case 30:
			_val3 = 31;
			series_play_with_breaks(PLAY11, "407rx06", 0x901, 8, 2);
			break;

		case 31:
			_val5 = 36;
			kernel_trigger_dispatch_now(9);
			_val3 = 32;
			series_play_with_breaks(PLAY12, "407rx08", 0x901, 8, 2);
			break;

		case 32:
			_val3 = 33;
			series_play_with_breaks(PLAY13, "407rx08", 0x901, 8, 2);
			break;

		case 33:
			_val3 = 34;
			series_play_with_breaks(PLAY14, "407rx09", 0x901, 8, 2);
			break;

		case 34:
			kernel_trigger_dispatch_now(14);
			terminateMachineAndNull(_rx);
			_rx = series_show("407rx09", 0x901);
			break;

		default:
			break;
		}
		break;

	case 9:
		switch (_val5) {
		case 35:
			_vp02.show("407vp02", 0x300);
			_val4 = 1;
			_val3 = 23;
			kernel_trigger_dispatch_now(8);
			break;

		case 36:
			_vp02.terminate();
			_val5 = 37;
			series_play_with_breaks(PLAY16, "407vp02", 0x300, 9, 3);
			break;

		case 37:
			_val5 = 39;
			series_play_with_breaks(PLAY17, "407vp03", 0x300, 9, 3);
			break;

		case 38:
			_vp02.terminate();
			_val2 = 20;
			_val5 = 35;
			series_play_with_breaks(PLAY15, "407vp02", 0x300, 9, 3);
			player_set_facing_at(412, 336);
			break;

		case 39:
			_vp02.show("407vp05", 0x300);
			break;

		case 40:
			terminateMachineAndNull(_rx);
			_vp02.terminate();
			hotspot_set_active("ROXY", false);
			break;

		default:
			break;
		}
		break;

	case 10:
		_G(flags)[V298] = 1;
		_G(flags)[V299] = 1;
		conv_load_and_prepare("conv87", 12);
		conv_export_pointer_curr(&_G(flags)[V180], 0);
		conv_play_curr();
		break;

	case 11:
		_G(flags)[V298] = 1;
		_G(flags)[V299] = 1;
		player_set_commands_allowed(false);
		conv_load_and_prepare("conv88", 13);
		conv_play_curr();
		break;

	case 12:
		_G(flags)[V298] = 0;
		_G(flags)[V299] = 0;
		_G(flags)[V180] = 1;
		terminateMachineAndNull(_rx);
		_val3 = 28;
		kernel_trigger_dispatch_now(8);
		break;

	case 13:
		_G(flags)[V298] = 0;
		_G(flags)[V299] = 0;
		player_set_commands_allowed(true);
		break;

	case 14:
		_G(flags)[V298] = 0;
		_val2 = 21;
		_val5 = 40;
		series_play_with_breaks(PLAY18, "407poof", 0x2ff, 4008, 2);
		_G(flags)[V181] = 1;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(false);

			if (_flag1) {
				digi_stop(2);
				_val2 = 17;
				kernel_trigger_dispatch_now(7);
			}

			_val4 = 3;
			_val3 = 20;
			kernel_trigger_dispatch_now(8);
			break;

		case 2:
			player_set_commands_allowed(false);

			if (_flag1) {
				digi_stop(2);
				_val2 = 17;
				kernel_trigger_dispatch_now(7);
			}

			_val4 = 2;
			_val3 = 23;
			kernel_trigger_dispatch_now(8);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kSET_COMMANDS_ALLOWED:
		if (!_G(flags)[V181] && _G(flags)[V171] == 4003) {
			_val5 = 38;
			kernel_timing_trigger(60, 9);
		} else {
			player_set_commands_allowed(true);
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room407::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(flags)[V067] && _G(player).walk_x >= 140 && _G(player).walk_x <= 240 &&
		_G(player).walk_y >= 295 && _G(player).walk_y <= 325)
		player_walk_to(139, 326);

	if (player_said("FORCE FIELD")) {
		player_set_facing_hotspot();
	} else if (player_said("GEAR", "DISC")) {
		_val1 = 4007;
		player_hotspot_walk_override(43, 302, -1, 1);
		_G(player).command_ready = false;
	} else if (player_said("GEAR", "DISC ")) {
		_val1 = 4007;
		player_hotspot_walk_override(578, 296, -1, 1);
		_G(player).command_ready = false;
	}
}

void Room407::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("conv87")) {
		conv87();
	} else if (player_said("conv88")) {
		conv88();
	} else if (!_G(walker).wilbur_match(MATCH)) {
		return;
	}

	_G(player).command_ready = false;
}

void Room407::loadSeries() {
	static const char *NAMES[16] = {
		"407dz01", "407dz01s", "407dz02", "407dz02s", "407dz05",
		"407dz05s", "407dz06", "407dz06s", "407rx04",
		"407vp03", "407vp03s", "407vp05", "407vp05s", "407rx06",
		"407rx08", "407rx09"
	};
	for (int i = 0; i < (_G(flags)[V171] == 4003 ? 16 : 9); ++i)
		series_load(NAMES[i]);

	digi_preload("407_001");
	digi_preload("145_004");
}

void Room407::conv87() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();

	if (sound) {
		if (who == 1)
			wilbur_speech(sound, 10001);
		else
			playConvSound();
	}
}

void Room407::conv88() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who == 1) {
			wilbur_speech(sound, 10001);
		} else if (_flag1) {
			_val2 = 18;
		} else if (node == 1) {
			switch (entry) {
			case 0:
			case 2:
			case 4:
				_val2 = 13;
				break;
			case 1:
				_val2 = 12;
				break;
			case 3:
			case 5:
				_val2 = 11;
				break;
			case 6:
				_val2 = 10;
				conv_resume_curr();
				break;
			default:
				break;
			}
		}
	}
}

int Room407::getRandomState() const {
	switch (imath_ranged_rand(1, 10)) {
	case 1:
		return 15;
	case 2:
		return 16;
	default:
		return 10;
	}
}

void Room407::freeDz() {
	_dz.terminate();
	series_unload(_dzS1);
	series_unload(_dzS2);
}

void Room407::playConvSound() {
	terminateMachineAndNull(_rx);

	_val3 = 25;
	digi_play(conv_sound_to_play(), 1, 255, 8);
	_rx = series_play("407rx04", 0x901, 4, -1, 6, -1);
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
