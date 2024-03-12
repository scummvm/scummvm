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

enum {
	kWILBUR_SPEECH = 4,
	kCHANGE_DRUMZ_ANIMATION = 7,
	kCHANGE_ROXY_ANIMATION = 8,
	kCHANGE_VIPE_ANIMATION = 9
};

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
	{ "AMPLIFIER", "DRUMZ",       4, 0, 0, &_speechNum, 4 },
	{ nullptr, "DRUMZ",           4, 0, 0, &_speechNum, 7 },
	{ "LOOK AT", "ROXY",      10016, 0, 0, &Vars::_wilbur_should, 1 },
	{ "TALK", "ROXY",         10016, 0, 0, &Vars::_wilbur_should, 2 },
	{ "AMPLIFIER", "ROXY",        4, 0, 0, &_speechNum, 4 },
	{ "DOG COLLAR", "ROXY",       4, 0, 0, &_speechNum, 5 },
	{ nullptr, "ROXY",            4, 0, 0, &_speechNum, 6 },
	{ "AMPLIFIER", "INSTRUMENTS", 4, 0, 0, &_speechNum, 8 },
	{ nullptr, "INSTRUMENTS",     4, 0, 0, &_speechNum, 9 },
	{ nullptr, "TOUR BUS",        4, 0, 0, &_speechNum, 3 },
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

int32 Room407::_speechNum;


Room407::Room407() : Section4Room() {
	_speechNum = 0;
}

void Room407::init() {
	digi_preload("400_001");
	digi_play_loop("400_001", 3, 125, -1);
	pal_cycle_init(112, 127, 6);

	if (!_G(flags)[V181] && _G(flags)[V171] == 4003) {
		_G(flags)[kDisableFootsteps] = 1;
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
		_drumzShould = 10;
		kernel_trigger_dispatch_now(kCHANGE_DRUMZ_ANIMATION);

		_roxyShould = 22;
		kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);

		if (_G(flags)[V171] == 4003)
			_vp02.show("407vp02", 0x300);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 402:
		ws_demand_location(43, 302);
		poofArrive();
		break;

	case 405:
		ws_demand_location(578, 296);
		poofArrive();
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

	case kWILBUR_SPEECH:
		switch (_speechNum) {
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
		_drumzShould = 17;

		if (_roxyState) {
			kernel_timing_trigger(120, 7);
		} else {
			int rnd = imath_ranged_rand(1, 25);
			if (rnd >= 16)
				_digiName = "407r902a";
			else
				_digiName = Common::String::format("407r902%c", 'a' + rnd);

			_flag1 = true;
			digi_preload(_digiName);
			digi_play(_digiName.c_str(), 2, 180, 6);
		}
		break;

	case 6:
		digi_unload(_digiName);
		kernel_trigger_dispatch_now(7);
		break;

	case kCHANGE_DRUMZ_ANIMATION:
		switch (_drumzShould) {
		case 10:
			if (!digi_play_state(2))
				digi_play_loop("407_001", 2);

			_drumzShould = getDrumzShould();
			series_play_with_breaks(PLAY1, "407dz01", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 11:
			_drumzShould = 14;
			series_play_with_breaks(PLAY2, "407dz02", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 12:
			_drumzShould = 14;
			series_play_with_breaks(PLAY3, "407dz03", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 13:
			digi_stop(2);
			_drumzShould = 14;
			series_play_with_breaks(PLAY4, "407dz04", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 14:
			_drumzShould = getDrumzShould();
			kernel_trigger_dispatch_now(kCHANGE_DRUMZ_ANIMATION);
			kernel_trigger_dispatch_now(10001);
			break;

		case 15:
			digi_stop(2);
			_drumzShould = getDrumzShould();
			series_play_with_breaks(PLAY5, "407dz05", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 16:
			digi_stop(2);
			_dzS1 = series_load("407dz06");
			_dzS2 = series_load("407dz06s");
			_dz.play("407dz06", 0xa01, 18, -1, 6, 0, 100, 0, 0, 2, 7);
			kernel_trigger_dispatch_now(5);
			break;

		case 17:
			_flag1 = false;
			freeDz();
			_drumzShould = getDrumzShould();
			series_play_with_breaks(PLAY6, "407dz06", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 18:
			_flag1 = false;
			freeDz();
			_drumzShould = 19;
			series_play_with_breaks(PLAY6, "407dz06", 0xa01, kCHANGE_DRUMZ_ANIMATION, 3);
			break;

		case 19:
			if (!digi_play_state(2))
				digi_play_loop("407_001", 2);

			conv88();
			kernel_trigger_dispatch_now(kCHANGE_DRUMZ_ANIMATION);
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

	case kCHANGE_ROXY_ANIMATION:
		switch (_roxyShould) {
		case 22:
			_roxyState = 0;
			_rx = series_show("407rx02", 0x901);
			break;

		case 23:
			terminateMachineAndNull(_rx);

			switch (_roxyState) {
			case 1:
				_roxyShould = 26;
				break;
			case 2:
				_roxyShould = 24;
				break;
			case 3:
				_roxyShould = 27;
				break;
			default:
				break;
			}

			series_play_with_breaks(PLAY7, "407rx02", 0x901, kCHANGE_ROXY_ANIMATION, 2);
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
			_roxyShould = 30;
			series_play_with_breaks(PLAY9, "407rx04", 0x901, kCHANGE_ROXY_ANIMATION, 2);
			break;

		case 27:
			_roxyShould = 28;
			series_play_with_breaks(PLAY8, "407rx04", 0x901, kCHANGE_ROXY_ANIMATION, 2);
			break;

		case 28:
			_roxyShould = 29;
			series_play_with_breaks(PLAY10, "407rx02", 0x901, kCHANGE_ROXY_ANIMATION, 2);
			break;

		case 29:
			_roxyShould = 22;
			kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			player_set_commands_allowed(true);
			break;

		case 30:
			_roxyShould = 31;
			series_play_with_breaks(PLAY11, "407rx06", 0x901, kCHANGE_ROXY_ANIMATION, 2);
			break;

		case 31:
			_vipeShould = 36;
			kernel_trigger_dispatch_now(kCHANGE_VIPE_ANIMATION);
			_roxyShould = 32;
			series_play_with_breaks(PLAY12, "407rx08", 0x901, kCHANGE_ROXY_ANIMATION, 2);
			break;

		case 32:
			_roxyShould = 33;
			series_play_with_breaks(PLAY13, "407rx08", 0x901, kCHANGE_ROXY_ANIMATION, 2);
			break;

		case 33:
			_roxyShould = 34;
			series_play_with_breaks(PLAY14, "407rx09", 0x901, kCHANGE_ROXY_ANIMATION, 2);
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

	case kCHANGE_VIPE_ANIMATION:
		switch (_vipeShould) {
		case 35:
			_vp02.show("407vp02", 0x300);
			_roxyState = 1;
			_roxyShould = 23;
			kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			break;

		case 36:
			_vp02.terminate();
			_vipeShould = 37;
			series_play_with_breaks(PLAY16, "407vp02", 0x300, kCHANGE_VIPE_ANIMATION, 3);
			break;

		case 37:
			_vipeShould = 39;
			series_play_with_breaks(PLAY17, "407vp03", 0x300, kCHANGE_VIPE_ANIMATION, 3);
			break;

		case 38:
			_vp02.terminate();
			_drumzShould = 20;
			_vipeShould = 35;
			series_play_with_breaks(PLAY15, "407vp02", 0x300, kCHANGE_VIPE_ANIMATION, 3);
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
		_G(flags)[kDisableFootsteps] = 1;
		_G(flags)[V299] = 1;
		conv_load_and_prepare("conv87", 12);
		conv_export_pointer_curr(&_G(flags)[V180], 0);
		conv_play_curr();
		break;

	case 11:
		_G(flags)[kDisableFootsteps] = 1;
		_G(flags)[V299] = 1;
		player_set_commands_allowed(false);
		conv_load_and_prepare("conv88", 13);
		conv_play_curr();
		break;

	case 12:
		_G(flags)[kDisableFootsteps] = 0;
		_G(flags)[V299] = 0;
		_G(flags)[V180] = 1;
		terminateMachineAndNull(_rx);
		_roxyShould = 28;
		kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
		break;

	case 13:
		_G(flags)[kDisableFootsteps] = 0;
		_G(flags)[V299] = 0;
		player_set_commands_allowed(true);
		break;

	case 14:
		_G(flags)[kDisableFootsteps] = 0;
		_drumzShould = 21;
		_vipeShould = 40;
		series_play_with_breaks(PLAY18, "407poof", 0x2ff, 4008, 2);
		_G(flags)[V181] = 1;
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			player_set_commands_allowed(false);

			if (_flag1) {
				digi_stop(2);
				_drumzShould = 17;
				kernel_trigger_dispatch_now(kCHANGE_DRUMZ_ANIMATION);
			}

			_roxyState = 3;
			_roxyShould = 20;
			kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			break;

		case 2:
			player_set_commands_allowed(false);

			if (_flag1) {
				digi_stop(2);
				_drumzShould = 17;
				kernel_trigger_dispatch_now(kCHANGE_DRUMZ_ANIMATION);
			}

			_roxyState = 2;
			_roxyShould = 23;
			kernel_trigger_dispatch_now(kCHANGE_ROXY_ANIMATION);
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kSET_COMMANDS_ALLOWED:
		if (!_G(flags)[V181] && _G(flags)[V171] == 4003) {
			_vipeShould = 38;
			kernel_timing_trigger(60, kCHANGE_VIPE_ANIMATION);
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

	if (!_G(flags)[kDrumzFled] && _G(player).walk_x >= 140 && _G(player).walk_x <= 240 &&
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
			_drumzShould = 18;
		} else if (node == 1) {
			switch (entry) {
			case 0:
			case 2:
			case 4:
				_drumzShould = 13;
				break;
			case 1:
				_drumzShould = 12;
				break;
			case 3:
			case 5:
				_drumzShould = 11;
				break;
			case 6:
				_drumzShould = 10;
				conv_resume_curr();
				break;
			default:
				break;
			}
		}
	}
}

int Room407::getDrumzShould() const {
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

	_roxyShould = 25;
	digi_play(conv_sound_to_play(), 1, 255, kCHANGE_ROXY_ANIMATION);
	_rx = series_play("407rx04", 0x901, 4, -1, 6, -1);
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
