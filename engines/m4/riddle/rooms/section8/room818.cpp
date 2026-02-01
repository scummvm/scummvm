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

#include "m4/riddle/rooms/section8/room818.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room818::preload() {
	_G(player).walker_in_this_scene = false;
	LoadWSAssets("other script", _G(master_palette));

	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room818::init() {
	player_set_commands_allowed(false);
	interface_hide();
	digi_preload("950_s45", 950);
	digi_play_loop("950_s45", 3, 100, -1, 950);

	if (inv_object_in_scene("CRANK", 808)) {
		series_show("808 HANDLE SPRITE", 3840, 16, -1, -1, 0, 100, 0, 0);
	}

	series_play("818merc", 3840, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("818fire", 3840, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("818baron", 256, 0, -1, 10, -1, 100, 0, 0, 0, -1);
	_818All1Series = series_load("818all1", -1, nullptr);
	_xMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, 0, triggerMachineByHashCallback, "x");
	sendWSMessage_10000(1, _xMach, _818All1Series, 1, 148, 5, _818All1Series, 148, 148, 4);
}

void Room818::pre_parser() {
	// Nothing
}

void Room818::parser() {
	// Nothing
}

void Room818::daemon() {
	int32 edx = _dword1A18DC + 1;
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 5:
		_dword1A18DC = 0;
		digi_play("818_s01", 1, 255, 10, 818);
		sendWSMessage_10000(1, _xMach, _818All1Series, 149, 179, 10, _818All1Series, 179, 179, 4);

		break;

	case 10:
		if (_dword1A18DC != 0) {
			_dword1A18DC = 0;
			sendWSMessage_10000(1, _xMach, _818All1Series, 179, 197, 15, _818All1Series, 187, 194, 1);
			digi_play("808r41", 1, 255, 15, -1);
		} else {
			_dword1A18DC = edx;
		}

		break;

	case 15:
		if (_dword1A18DC != 0) {
			_dword1A18DC = 0;
			sendWSMessage_10000(1, _xMach, _818All1Series, 195, 218, 20, _818All1Series, 218, 218, 0);
		} else {
			_dword1A18DC = edx;
		}

		break;

	case 20:
		sendWSMessage_10000(1, _xMach, _818All1Series, 218, 240, 22, _818All1Series, 240, 240, 0);
		digi_play("808m10", 1, 255, 22, -1);

		break;

	case 22:
		if (_dword1A18DC != 0) {
			_dword1A18DC = 0;
			sendWSMessage_10000(1, _xMach, _818All1Series, 242, 269, 23, _818All1Series, 263, 269, 1);
			digi_play("808r42", 1, 255, 23, -1);
		} else {
			_dword1A18DC = edx;
		}

		break;

	case 23:
		if (_dword1A18DC != 0) {
			_dword1A18DC = 0;
			sendWSMessage_10000(1, _xMach, _818All1Series, 270, 300, 25, _818All1Series, 300, 300, 1);
		} else {
			_dword1A18DC = edx;
		}

		break;

	case 25:
		sendWSMessage_10000(1, _xMach, _818All1Series, 301, 328, 30, _818All1Series, 328, 328, 1);
		disable_player_commands_and_fade_init(-1);
		midi_fade_volume(0, 70);
		kernel_timing_trigger(70, 30, nullptr);

		break;

	case 30:
		_G(game).setRoom(917);

		break;

	default:
		break;

	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
