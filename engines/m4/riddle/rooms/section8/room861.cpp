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

#include "m4/riddle/rooms/section8/room861.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room861::preload() {
	Room::preload();
	_G(player).walker_in_this_scene = false;
}

void Room861::init() {
	interface_hide();
	player_set_commands_allowed(false);

	digi_preload("950_s45", 950);
	digi_play_loop("950_s45", 3, 150, -1, 950);
	_all1Series = series_load("861all1");
	_all2Series = series_load("861all2");

	digi_preload("861r03");
	digi_preload("811_s02");
	digi_preload("811_s05");
	digi_preload("861_s01");
	digi_preload("861_s02");
	digi_preload("861_s02a");

	_soldrSeries = series_load("861soldr");
	_soldrMach = series_show("861soldr", 0x400);
	_all1Mach = series_stream("861all", 4, 0x100, 5);
	series_stream_break_on_frame(_all1Mach, 9, 2);
}

void Room861::daemon() {
	switch (_G(kernel).trigger) {
	case 2:
		digi_play("811_s05", 1, 255, -1, 811);
		series_stream_break_on_frame(_all1Mach, 90, 4);
		break;

	case 4:
		digi_play("811_s02", 1, 255, -1, 811);
		break;

	case 5:
		_all1Mach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			0x100, false, triggerMachineByHashCallback, "861all1");
		sendWSMessage_10000(1, _all1Mach, _all1Series, 1, 24, 10,
			_all1Series, 24, 24, 0);
		sendWSMessage_190000(_all1Mach, 12);
		terminateMachineAndNull(_soldrMach);
		series_unload(_soldrSeries);
		_fallMach = series_stream("861_fall", 9, 0x300, 80);
		series_stream_break_on_frame(_fallMach, 42, 1002);
		break;

	case 10:
		digi_play("861_s01", 2, 255, -1, 861);
		midi_play("tiawa", 180, 1, -1, 949);
		_all2Mach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0,
			100, 0x200, false, triggerMachineByHashCallback, "861 all2");
		sendWSMessage_10000(1, _all2Mach, _all2Series, 1, 20, 20,
			_all2Series, 20, 20, 0);
		sendWSMessage_10000(1, _all1Mach, _all1Series, 25, 42, 15,
			_all1Series, 43, 47, 4);
		break;

	case 15:
		sendWSMessage_1a0000(_all1Mach, 10);
		break;

	case 20:
		sendWSMessage_10000(1, _all2Mach, _all2Series, 21, 44, 91,
			_all2Series, 45, 58, 0);
		digi_play("861r03", 2);
		break;

	case 91:
		disable_player_commands_and_fade_init(-1);
		break;

	case 92:
		digi_stop(1);
		adv_kill_digi_between_rooms(false);
		digi_unload("861r03");
		digi_unload("811_s02");
		digi_unload("811_s05");
		digi_unload("861_s01");
		digi_unload("861_s02");
		digi_unload("861_s02a");
		digi_play_loop("950_s45", 3, 128, -1, 950);
		_G(game).setRoom(818);
		break;

	case 1001:
		digi_play("861_s02", 1, 255, 92);
		break;

	case 1002:
		digi_play("861_s02a", 1);
		series_stream_break_on_frame(_fallMach, 52, 1001);
		break;

	case 80:
	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
