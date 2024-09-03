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

#include "m4/riddle/rooms/section3/room354.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room354::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	_G(player).walker_in_this_scene = false;
}

void Room354::init() {
	_G(flags)[V001] = 1;
	digi_preload("354f01");
	digi_preload("354f02");
	digi_preload("354_s02a");
	digi_preload("354_s02b");
	digi_preload("354_s01");
	digi_preload("304_s01");

	_val1 = 0;
	_G(player).disable_hyperwalk = true;
	interface_hide();

	AddSystemHotkey(KEY_ESCAPE, escapeKeyPressed);
	_G(kernel).call_daemon_every_loop = true;
	player_set_commands_allowed(false);
	kernel_timing_trigger(60, 10);
}

void Room354::daemon() {
	switch (_G(kernel).trigger) {
	case 10:
		_all = series_stream("354 ALL", 7, 0x100, 43);
		pal_fade_set_start(0);
		pal_fade_init(21, 255, 100, _G(kernel).fade_up_time, -1);
		series_stream_break_on_frame(_all, 5, 373);
		break;

	case 12:
		series_stream_break_on_frame(_all, 109, 23);
		digi_play("354_s02a", 1);
		break;

	case 23:
		series_stream_break_on_frame(_all, 177, 24);
		digi_play("354f02", 1);
		break;

	case 24:
		series_stream_break_on_frame(_all, 289, 25);
		digi_play("354f01", 1);
		break;

	case 25:
		series_stream_break_on_frame(_all, 302, 292);
		digi_play("354_s02b", 1, 255, 765);
		break;

	case 26:
		digi_play("354_s01", 1);
		break;

	case 43:
		disable_player_commands_and_fade_init(1001);
		break;

	case 54:
		_G(game).setRoom(_G(flags)[V001] ? 304 : 494);
		break;

	case 55:
	case 1001:
		_G(game).setRoom(304);
		break;

	case 56:
		_G(game).setRoom(494);
		break;

	case 292:
		digi_stop(2);
		series_stream_break_on_frame(_all, 305, 26);
		break;

	case 373:
		series_stream_break_on_frame(_all, 54, 12);
		digi_play("304_s01", 1);
		break;

	case 765:
		digi_stop(1);
		break;

	default:
		break;
	}

	if (_G(MouseState).ButtonState) {
		_buttonFlag = true;
	} else if (_buttonFlag) {
		_buttonFlag = false;

		disable_player_commands_and_fade_init(55);
	}
}

void Room354::escapeKeyPressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
