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

#include "m4/burger/rooms/section5/room510.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

#define BUTTON_COUNT 7

const Room510::ButtonDef Room510::_BUTTONS[7] = {
	{ 250, 159, 406, 189, 0, 1, 2, 3, 1, 4 },
	{ 250, 190, 406, 216, 4, 5, 6, 7, 1, 5 },
	{ 250, 217, 406, 244, 8, 9, 10, 11, 1, 6 },
	{ 250, 245, 406, 265, 12, 13, 14, 15, 1, 7 },
	{ 250, 266, 406, 291, 16, 17, 18, 19, 1, 8 },
	{ 250, 292, 406, 316, 20, 21, 22, 23, 1, 9 },
	{ 250, 317, 406, 343, 24, 25, 26, 27, 1, 10 }
};

Room510::Room510() : Section5Room() {
	Common::copy(_BUTTONS, _BUTTONS + BUTTON_COUNT, _buttons);
}

void Room510::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room510::init() {
	series_load("510butts");
	_G(kernel).suppress_fadeup = true;
	pal_fade_set_start(0);
	pal_fade_init(_G(kernel).first_fade, 255, 100, 60, 2);

	for (int i = 0; i < 7; ++i) {
		_buttons[i]._state = 1;
		setupButton(i);
	}

	_val2 = -1;
	_val3 = -1;
	_val4 = -1;

	if (!player_been_here(510)) {
		wilbur_speech("510w001");
	} else if (!_G(flags)[V237]) {
		wilbur_speech("510w002");
		_G(flags)[V237] = 1;
	}
}

void Room510::daemon() {
	// TODO
}

void Room510::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("LOOK AT")) {
		bool doneFlag = true;
		if (player_said("RED BUTTON")) {
			if (!player_been_here(503)) {
				wilbur_speech("510w003");
			} else if (_G(flags)[gBORK_STATE] != 16) {
				wilbur_speech("510w004");
			} else {
				wilbur_speech("510w005");
			}
		} else if (player_said("ORANGE BUTTON")) {
			if (!player_been_here(507)) {
				wilbur_speech("510w006");
			} else if (_G(flags)[V223] != 2) {
				wilbur_speech("510w007");
			} else {
				wilbur_speech("510w008");
			}
		} else if (player_said("YELLOW BUTTON")) {
			if (!player_been_here(504)) {
				wilbur_speech("510w009");
			} else if (_G(flags)[V210] != 5002) {
				wilbur_speech("510w010");
			} else {
				wilbur_speech("510w011");
			}
		} else if (player_said("GREEN BUTTON")) {
			if (!player_been_here(508)) {
				wilbur_speech("510w012");
			} else if (_G(flags)[V227] == 0) {
				wilbur_speech("510w013");
			} else {
				wilbur_speech("510w014");
			}
		} else if (player_said("BLUE BUTTON")) {
			if (!player_been_here(506)) {
				wilbur_speech("510w015");
			} else if (_G(flags)[V218] != 5003) {
				wilbur_speech("510w016");
			} else {
				wilbur_speech("510w017");
			}
		} else if (player_said("PURPLE BUTTON")) {
			wilbur_speech(_G(flags)[V200] != 5003 ? "510w018" : "510w019");
		} else if (player_said("PINK BUTTON")) {
			wilbur_speech("510w020");
		} else {
			doneFlag = false;
		}

		if (doneFlag) {
			goto done;
		}
	}

	if (player_said("TAKE")) {
		wilbur_speech("500w001");
	} else if (player_said("EXIT")) {
		kernel_trigger_dispatch_now(1);
	} else if (player_said("GEAR")) {
		if (player_said("RED BUTTON")) {
			_G(kernel).call_daemon_every_loop = false;
			setButtonState(0, 3);
			kernel_trigger_dispatch_now(4);
		} else if (player_said("ORANGE BUTTON")) {
			_G(kernel).call_daemon_every_loop = false;
			setButtonState(1, 3);
			kernel_trigger_dispatch_now(5);
		} else if (player_said("YELLOW BUTTON")) {
			_G(kernel).call_daemon_every_loop = false;
			setButtonState(2, 3);
			kernel_trigger_dispatch_now(6);
		} else if (player_said("GREEN BUTTON")) {
			_G(kernel).call_daemon_every_loop = false;
			setButtonState(3, 3);
			kernel_trigger_dispatch_now(7);
		} else if (player_said("BLUE BUTTON")) {
			_G(kernel).call_daemon_every_loop = false;
			setButtonState(4, 3);
			kernel_trigger_dispatch_now(8);
		} else if (player_said("PURPLE BUTTON")) {
			_G(kernel).call_daemon_every_loop = false;
			setButtonState(5, 3);
			kernel_trigger_dispatch_now(9);
		} else if (player_said("PINK BUTTON")) {
			_G(kernel).call_daemon_every_loop = false;
			setButtonState(6, 3);
			kernel_trigger_dispatch_now(10);
		} else {
			return;
		}
	} else {
		return;
	}

done:
	_G(player).command_ready = false;
}

void Room510::setupButton(int index) {
	Button &btn = _buttons[index];
	
	switch (btn._state) {
	case 0:
		btn._series = series_show("510butts", 0, 0, -1, -1, btn._index0);
		break;
	case 1:
		btn._series = series_show("510butts", 0, 0, -1, -1, btn._index1);
		break;
	case 2:
		btn._series = series_show("510butts", 0, 0, -1, -1, btn._index2);
		break;
	case 3:
		btn._series = series_show("510butts", 0, 0, -1, -1, btn._index3);
		break;
	default:
		break;
	}
}

void Room510::setButtonState(int index, int state) {
	if (index >= 0) {
		Button &btn = _buttons[index];

		if (btn._state != 0 && btn._state != state) {
			terminateMachineAndNull(btn._series);
			btn._state = state;
			setupButton(index);
		}
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
