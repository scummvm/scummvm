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

#include "m4/riddle/rooms/section9/room905.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_control.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_buffer.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/m4.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room905::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room905::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

void Room905::init() {
	_roomStates_field4 = 0xff;

	_905_BuffTemp = new GrBuff(640, _G(kernel).letter_box_y);
	Buffer *myBuff = _905_BuffTemp->get_buffer();
	byte *bufferHandle = myBuff->data;

	for (int i = 0; i < 640 * _G(kernel).letter_box_y; i++) {
		bufferHandle[i] = 0;
	}

	gui_GrBuff_register(0, 0, _905_BuffTemp, SF_SURFACE, nullptr);
	gui_buffer_activate((Buffer *)_905_BuffTemp);
	
	vmng_screen_show(_905_BuffTemp);
	
	_905_BuffTemp2 = new GrBuff(640, 77);
	myBuff = _905_BuffTemp2->get_buffer();
	bufferHandle = myBuff->data;
	
	for (int i = 0; i < 640 * 77; i++) {
		bufferHandle[i] = 0;
	}	

	gui_GrBuff_register(0, 403, _905_BuffTemp2, SF_SURFACE, nullptr);
	gui_buffer_activate((Buffer *)_905_BuffTemp2);
	vmng_screen_show(_905_BuffTemp2);

	MoveScreenDelta(_G(game_buff_ptr), 0, -(_G(kernel).letter_box_y + 768));

	series_load("905 hold frame", -1, nullptr);
	g_engine->adv_camera_pan_step(3);
	player_set_commands_allowed(false);
	interface_hide();
	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
	_roomStates_tt = series_stream("RIP EATS SAND WITH HIS ANUS", 5, 2560, 666);
	series_stream_break_on_frame(_roomStates_tt, 50, 3);
}

void Room905::daemon() {
	switch (_G(kernel).trigger) {
	case 3:
		g_engine->camera_shift_xy(0, 0);
		kernel_timing_trigger(120, 4, nullptr);
		digi_play("INTMOAN", 1, 255, -1, 901);
		kernel_timing_trigger(60, 100, nullptr);
		break;

	case 4:
		if (g_engine->game_camera_panning())
			kernel_timing_trigger(120, 4, nullptr);
		else
			kernel_timing_trigger(150, 20, nullptr);
		break;

	case 20:
		disable_player_commands_and_fade_init(30);
		break;

	case 30:
		_roomStates_field4 -= 3;
		if (_roomStates_field4 <= 40) {
			adv_kill_digi_between_rooms(false);
			_G(game).setRoom(906);
		} else {
			digi_change_panning(1, _roomStates_field4);
			kernel_timing_trigger(2, 30, nullptr);
		}
		break;

	case 55:
		_G(game).setRoom(304);
		break;

	case 56:
		_G(game).setRoom(494);
		break;

	case 666:
		series_plain_play("905 hold frame", -1, 0, 100, 256, 3000);
		break;

	case 1000:
		digi_stop(2);
		break;

	default:
		break;
	}
}

void Room905::shutdown() {
	if (_905_BuffTemp) {
		gui_buffer_deregister((Buffer *)_905_BuffTemp);
		delete _905_BuffTemp;
		_905_BuffTemp = nullptr;
	}

	if (_905_BuffTemp2) {
		gui_buffer_deregister((Buffer *)_905_BuffTemp2);
		delete _905_BuffTemp2;
		_905_BuffTemp2 = nullptr;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
