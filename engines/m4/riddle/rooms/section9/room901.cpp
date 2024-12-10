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

#include "common/config-manager.h"
#include "m4/riddle/rooms/section9/room901.h"
#include "m4/riddle/hotkeys.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room901::preload() {
	if (_G(game).room_id == 494)
		digi_preload("wind", 901);

	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	_G(player).walker_in_this_scene = false;
}

void Room901::init() {
	// WORKAROUND: Original created a dummy file called believe.it 
	// on first run with "Never forget the llama and the spleen."
	// For ScummVM, I've instead used a simple config manager flag

	if (!ConfMan.getBool("believe_it")) {
		ConfMan.setBool("believe_it", true);

		_G(game).previous_room = 494;
		digi_preload("wind", 901);
		_G(flags)[V001] = 1;
	}

	player_set_commands_allowed(false);

	if (_G(game).previous_room == 494) {
		digi_play_loop("wind", 1, 255, -1, 901);
		kernel_timing_trigger(1, 10);
		AddSystemHotkey(KEY_ESCAPE, escapePressed);
		_G(kernel).call_daemon_every_loop = true;

	} else {
		mouse_hide();
		kernel_timing_trigger(1, 5);
		interface_hide();
		AddSystemHotkey(KEY_ESCAPE, escapePressed);
	}
}

void Room901::daemon() {
	switch (_G(kernel).trigger) {
	case 5:
		kernel_timing_trigger(300, 6);
		break;

	case 6:
		disable_player_commands_and_fade_init(7);
		break;

	case 7:
	case 56:
		AddSystemHotkey(KEY_ESCAPE, Hotkeys::escape_key_pressed);
		AddSystemHotkey(KEY_F2, Hotkeys::saveGame);
		AddSystemHotkey(KEY_F3, Hotkeys::loadGame);

		_G(game).setRoom(494);
		break;

	case 10:
		kernel_timing_trigger(300, 20);
		break;

	case 20:
		digi_play_loop("wind", 2, 255, -1, 901);
		kernel_timing_trigger(100, 21);
		disable_player_commands_and_fade_init(30);
		break;

	case 21:
		digi_stop(1);
		break;

	case 30:
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(902);
		break;

	case 55:
		_G(game).setRoom(304);
		break;

	default:
		break;
	}

	bool mouseReleased = false;
	if (_G(MouseState).ButtonState) {
		_btnPressed = true;
	} else if (_btnPressed) {
		_btnPressed = false;
		mouseReleased = true;
	}

	if (mouseReleased)
		disable_player_commands_and_fade_init(55);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
