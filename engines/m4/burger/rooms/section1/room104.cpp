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

#include "m4/burger/rooms/section1/room104.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID1[][4] = {
	{ "TOWN HALL",   "104W001", "104W002", "104w002" },
	{ "DOORS",       "104W003", "104W002", nullptr },
	{ "STOLIE",      "104W004", "104W002", "104W002" },
	{ "MAIN STREET", nullptr,   "104W002", "104W002" },
	{ "AUNT POLLY'S HOUSE", nullptr, "104W002", "104W002" },
	{ nullptr, nullptr, nullptr, nullptr }
};

void Room104::init() {
	_G(player).walker_in_this_scene = true;
	_val1 = 0;
	_val2 = 14;
	_val3 = 0;

	digi_preload("104_001");

	switch (_G(game).previous_room) {
	case 20:
		break;

	case 101:
		_G(roomVal1) = 1;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	case 105:
		_G(roomVal1) = 2;
		digi_preload("104_005");
		digi_play("104_005", 2, 255, 2);
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	case 107:
		_G(roomVal1) = 3;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	case 170:
		_G(roomVal1) = 4;
		kernel_trigger_dispatch_now(gTELEPORT);
		break;

	default:
		ws_demand_location(213, 297);
		ws_demand_facing(5);
		break;
	}

	static const char *NAMES[14] = {
		"104W101",  "104W100A", "104W100B", "104W100C", "104W100D",
		"104W100E", "104W100F", "104W100G", "104s100a", "104s100b",
		"104s100c", "104s100d", "104s100e", "104s101"
	};
	for (int i = 0; i < 14; ++i)
		digi_preload(NAMES[i]);

	digi_play_loop("104_001", 3, 200, -1);
	_G(flags)[V028] = 1;
	kernel_trigger_dispatch_now(3);
}

void Room104::daemon() {
}

void Room104::pre_parser() {
	_G(kernel).call_daemon_every_loop = player_said("MAIN STREET") &&
		player_said_any("look at", "enter");
}

void Room104::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (!_G(walker).wilbur_said(SAID1)) {
		if (player_said("MAIN STREET") && player_said_any("look at", "enter")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1001);

		} else if (player_said("ENTER", "DOORS") || player_said("ENTER", "DOOR HANDLE") ||
				player_said("gear", "doors") || player_said("gear", "door handle")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1);

		} else if (player_said("LOOK AT", "AUNT POLLY'S HOUSE")) {
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1017);

		} else if (player_said("ENTER", "AUNT POLLY'S HOUSE")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1017);

		} else if (player_said("talk to", "stolie")) {
			if (_G(flags)[V028]) {
				player_set_commands_allowed(false);

				switch (_G(flags)[V027]) {
				case 0:
					_val2 = 12;
					_G(walker).wilbur_speech_random("104W100A", "104W100B", "104W100C",
						"104W100D", "104W100E", "104W100F", "104W100G");
					break;
				case 12:
					_G(walker).wilbur_speech("104W101");
					_G(flags)[V028] = 0;
					player_set_commands_allowed(true);
					break;
				default:
					_val2 = 10;
					break;
				}
			} else {
				_G(walker).wilbur_speech("104W101");
			}
		} else if (player_said("conv16")) {
			conv();
		} else {
			return;
		}
	}

	_G(player).command_ready = false;
}

void Room104::conv() {
	if (conv_sound_to_play()) {
		switch (conv_whos_talking()) {
		case 0:
			_val2 = 17;
			kernel_trigger_dispatch_now(3);
			break;
		case 1:
			_G(walker).wilbur_speech(conv_sound_to_play(), 10001);
			break;
		default:
			break;
		}
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
