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

#include "m4/burger/rooms/section2/room207.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	kCHANGE_ASTRAL_ANIMATION = 1
};

const seriesPlayBreak Room207::PLAY1[] = {
	{ 0, 22, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY2[] = {
	{ 22, 17, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 18, 25, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 31, 33, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 34, 31, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY3[] = {
	{  0, 13, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 25, 29, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY4[] = {
	{ 29, 25, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 13,  0, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY5[] = {
	{ 35, 37, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 37, 35, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY6[] = {
	{ 0, 17, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY7[] = {
	{ 29, 22, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY8[] = {
	{ 5, 23, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY9[] = {
	{ 23, 5, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY10[] = {
	{ 31, 24, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY11[] = {
	{ 24, 31, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room207::PLAY12[] = {
	{ 18, 22, 0, 0, 0, -1, 0, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesStreamBreak Room207::SERIES1[] = {
	{ 0, 0, 0, 0, -1, 0, 0, 0 },
	{ 18, "207w001", 1, 255, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room207::SERIES2[] = {
	{ 0, 0, 0, 0, -1, 0, 0, 0 },
	STREAM_BREAK_END
};


void Room207::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room207::init() {
	player_set_commands_allowed(false);
	digi_preload("207_001");
	digi_play_loop("207_001", 3, 60);

	_G(kernel).trigger_mode = KT_DAEMON;
	series_load("207WI01");
	series_load("207AS01");
	series_load("207AS01S");
	series_load("207AS03");
	series_load("207AS03S");
	series_load("207AS04");
	series_load("207AS04S");

	_conv1 = _G(flags).getConvName();
	if (!_conv1)
		_conv1 = "conv51";

	interface_hide();

	if (!strcmp(_conv1, "conv53")) {
		_G(kernel).suppress_fadeup = true;
		kernel_trigger_dispatch_now(3);
	} else {
		kernel_trigger_dispatch_now(2);
	}
}

void Room207::daemon() {
	switch (_G(kernel).trigger) {
	case kCHANGE_ASTRAL_ANIMATION:
		switch (_astralMode) {
		case 15:
			switch (_astralShould) {
			case 1:
				_astralShould = 7;
				_series1.play("207as03", 0x200, 4, -1, 6, -1, 100, 0, 0, 31, 34);
				digi_play(conv_sound_to_play(), 1, 255, kCHANGE_ASTRAL_ANIMATION);
				break;

			case 7:
				_series1.terminate();
				_astralShould = 10;
				conv_resume_curr();
				kernel_trigger_dispatch_now(kCHANGE_ASTRAL_ANIMATION);
				break;

			case 8:
				_astralMode = 16;
				_astralShould = 8;
				series_play_with_breaks(PLAY4, "207as03", 0x200, 5, kCHANGE_ASTRAL_ANIMATION);
				break;

			case 10:
				_astralShould = getAstralShould();
				Series::series_show("207as03", 0x200, 0, kCHANGE_ASTRAL_ANIMATION, 30, 30);
				break;

			case 11:
				_astralShould = 10;
				series_play_with_breaks(PLAY5, "207as03", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			case 12:
				_astralMode = 17;
				_astralShould = 12;
				series_play_with_breaks(PLAY10, "207as04", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			case 13:
				_astralMode = 0;
				series_play_with_breaks(PLAY7, "207as03", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			default:
				break;
			}
			break;

		case 16:
			switch (_astralShould) {
			case 1:
				_astralShould = 7;
				_series1.play("207as01", 0x200, 4, -1, 6, -1, 100, 0, 0, 26, 30);
				digi_play(conv_sound_to_play(), 1, 255, kCHANGE_ASTRAL_ANIMATION);
				break;

			case 7:
				_series1.terminate();
				_astralShould = 8;
				conv_resume_curr();
				kernel_trigger_dispatch_now(kCHANGE_ASTRAL_ANIMATION);
				break;

			case 8:
				_astralShould = getAstralShould();
				_series1.show("207as01", 0x200, 0, kCHANGE_ASTRAL_ANIMATION, 30, 23);
				break;

			case 9:
				_astralShould = 8;
				series_play_with_breaks(PLAY2, "207as01", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			case 10:
				_astralMode = 15;
				_astralShould = 10;
				series_play_with_breaks(PLAY3, "207as03", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			case 12:
				_astralMode = 17;
				_astralShould = 12;
				series_play_with_breaks(PLAY8, "207as04", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			case 13:
				_astralMode = 0;
				series_play_with_breaks(PLAY6, "207as03", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			default:
				break;
			}
			break;

		case 17:
			switch (_astralShould) {
			case 1:
				_astralShould = 7;
				_series1.play("207as04", 0x200, 4, -1, 6, -1, 100, 0, 0, 0, 4);
				digi_play(conv_sound_to_play(), 1, 255, kCHANGE_ASTRAL_ANIMATION);
				break;

			case 7:
				_series1.terminate();
				_astralShould = 12;
				conv_resume_curr();
				kernel_trigger_dispatch_now(kCHANGE_ASTRAL_ANIMATION);
				break;

			case 8:
				_astralMode = 16;
				_astralShould = 8;
				series_play_with_breaks(PLAY9, "207as04", 0x200, 5, 1);
				break;

			case 10:
				_astralMode = 15;
				_astralShould = 10;
				series_play_with_breaks(PLAY11, "207as04", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			case 12:
				_astralShould = getAstralShould();
				Series::series_show("207as04", 0x200, 0, kCHANGE_ASTRAL_ANIMATION, 30, 0);
				break;

			case 13:
				_astralMode = 15;
				series_play_with_breaks(PLAY11, "207as03", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			default:
				break;
			}
			break;

		default:
			switch (_astralShould) {
			case 5:
				_astralMode = 16;
				_astralShould = 8;
				series_play_with_breaks(PLAY1, "207as01", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
				break;

			case 13:
				digi_play(conv_sound_to_play(), 1, 255, 6);
				_series1.play("207as03", 0x200, 1, -1, 6, -1, 100, 0, 0, 18, 22);
				break;

			default:
				break;
			}
			break;
		}
		break;

	case 2:
		conv_load_and_prepare(_conv1, 7);
		conv_play_curr();

		_G(wilbur_should) = 4;
		_astralMode = 0;
		_astralShould = 5;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		kernel_trigger_dispatch_now(kCHANGE_ASTRAL_ANIMATION);
		break;

	case 3:
		gr_backup_palette();
		digi_preload_stream_breaks(SERIES1);
		series_stream_with_breaks(SERIES1, "207a", 6, 0x100, 4);
		pal_fade_init(_G(kernel).first_fade, 255, 100, 0, -1);
		break;

	case 4:
		pal_fade_init(_G(kernel).first_fade, 255, 0, 0, -1);
		series_stream_with_breaks(SERIES2, "207b", 6, 0xf00, -1);
		gr_restore_palette();
		release_trigger_on_digi_state(2, 1);
		break;

	case 5:
		series_play_with_breaks(PLAY12, "207as01", 0x200, kCHANGE_ASTRAL_ANIMATION, 1);
		break;

	case 6:
		_series1.terminate();
		Series::series_show("207as03", 0x200, 0, -1, -1, 22);
		conv_resume_curr();
		break;

	case 7:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(8);
		break;

	case 8:
		static_cast<Burger::BurgerEngine *>(g_engine)->handleTestDone();
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 1:
			if (!strcmp(conv_sound_to_play(), "53p0101")) {
				_G(wilbur_should) = 3;
				conv_resume_curr();
				kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			} else {
				_G(wilbur_should) = 2;
				digi_play(conv_sound_to_play(), 1, 255, kCHANGE_WILBUR_ANIMATION);
				_general1 = series_play("207wi01", 0x400, 4, -1, 6, -1);
			}
			break;

		case 2:
			terminateMachineAndNull(_general1);
			_G(wilbur_should) = 3;
			conv_resume_curr();
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
			break;

		case 3:
			_G(wilbur_should) = getWilburShould();
			series_show("207wi01", 0x400, 0, kCHANGE_WILBUR_ANIMATION, 30, 0);
			break;

		case 4:
			_G(wilbur_should) = getWilburShould();
			series_play("207wi01", 0x400, 1, kCHANGE_WILBUR_ANIMATION, 6, 0, 100, 0, 0, 0, 2);
			break;

		default:
			break;
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room207::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said(_conv1)) {
		handleConv();
		_G(player).command_ready = false;
	}
}

void Room207::handleConv() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who == 1) {
			_G(wilbur_should) = 1;

			if (player_said("conv56") && node == 4)
				_astralShould = 8;

			if (node == 0 && entry == 1)
				pal_fade_init(_G(kernel).first_fade, 255, 100, 30, -1);
		} else if (player_said("conv51") && node == 8) {
			_astralShould = 13;
		} else {
			_astralShould = 1;
		}
	}
}

int Room207::getAstralShould() const {
	int rand = imath_ranged_rand(1, 24);

	switch (_astralMode) {
	case 15:
		switch (rand) {
		case 1:
			return 8;
		case 2:
			return 11;
		case 3:
			return 12;
		default:
			return 10;
		}
		break;

	case 16:
		switch (rand) {
		case 1:
			return 10;
		case 2:
		case 3:
			return 9;
		case 4:
			return 12;

		default:
			return 8;
		}
		break;

	case 17:
		switch (rand) {
		case 1:
			return 8;
		case 2:
			return 10;
		default:
			return 12;
		}
		break;

	default:
		return _astralShould;
	}
}

int Room207::getWilburShould() const {
	int rand = imath_ranged_rand(1, 8);
	return (rand == 1) ? 4 : 3;
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
