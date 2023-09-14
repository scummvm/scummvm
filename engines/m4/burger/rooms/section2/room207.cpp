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
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

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

	_conv1 = getConvName();
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
	// TODO
}

void Room207::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!player_said(_conv1)) {
		handleConv();
		_G(player).command_ready = false;
	}
}

const char *Room207::getConvName() {
	_conv2 = nullptr;
	if (_G(flags)[V287] && !_G(flags)[V242])
		_conv2 = "conv51";

	if (_G(flags)[V242] == 5 && _G(flags)[V241] == 0 && !player_been_here(612)) {
		if (_G(flags)[V248])
			_conv2 = "conv52";
		else
			--_G(flags)[V242];
	}

	if (_G(flags)[V241] && !_G(flags)[V100])
		_conv2 = "conv53";

	if (_G(flags)[V100] == 5 && !_G(flags)[V099] && !player_been_here(310) &&
			!player_been_here(311) && !player_been_here(312) && !player_been_here(313) &&
			!player_been_here(314) && !player_been_here(315) && !player_been_here(316) &&
			!player_been_here(317) && !player_been_here(318) && !player_been_here(319))
		_conv2 = "conv54";

	if (_G(flags)[V099] && !_G(flags)[V185])
		_conv2 = "conv55";

	if (_G(flags)[V185] == 5 && !_G(flags)[V184])
		_conv2 = "conv56";

	if (_G(flags)[V184] && !_G(flags)[V153])
		_conv2 = "conv57";

	if (_G(flags)[V153] == 5)
		_conv2 = "conv58";

	return _conv2;
}

void Room207::handleConv() {
	int who = conv_whos_talking();
	const char *sound = conv_sound_to_play();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (!sound) {
		if (who == 1) {
			_G(wilbur_should) = 1;

			if (player_said("conv56") && node == 4)
				_val1 = 8;

			if (node == 0 && entry == 1)
				pal_fade_init(_G(kernel).first_fade, 255, 100, 30, -1);
		} else if (player_said("conv51") && node == 8) {
			_val1 = 13;
		} else {
			_val1 = 1;
		}
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
