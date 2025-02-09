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

#include "m4/riddle/rooms/section8/room852.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room852::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room852::init() {
	interface_hide();
	player_set_commands_allowed(false);
	digi_preload("852_s01");
	digi_preload("852r03");
	kernel_timing_trigger(30, 1);
}

void Room852::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		_all = series_stream("852all", 5, 0x100, -1);
		series_stream_break_on_frame(_all, 56, 6);
		break;

	case 6:
		series_stream_break_on_frame(_all, 95, 7);
		digi_play("852r03", 2);
		break;

	case 7:
		series_stream_break_on_frame(_all, 255, 9);
		digi_play("852_s01", 2, 255, 8);
		break;

	case 8:
		digi_play("950_s29", 2);
		break;

	case 9:
		disable_player_commands_and_fade_init(11);
		break;

	case 11:
		_G(game).setRoom(803);
		break;

	default:
		break;
	}
}

void Room852::shutdown() {
	interface_show();
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
