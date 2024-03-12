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

#include "m4/riddle/rooms/section2/room201.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room201::init() {
	if (keyCheck()) {
		_flag1 = true;
		player_set_commands_allowed(false);
	} else {
		_flag1 = false;
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_val1 = _val2 = 0;
	_val3 = 0;

	digi_preload("950_s02");
	_nod = series_load("HEAD NOD Y/N");
	_guyWriting = series_load("GUY WRITING");
	_guyParcel = series_load("GUY TAKE PARCEL FROM RIPLEY");

	if (_flag1) {
		_series1 = series_load("GUY GET ATTENTION / WAVE");
		_series2 = series_load("201rp99");
		_series3 = series_load("201rp02");
		_series4 = series_load("201mc01");
	} else {
		_series5 = series_load("GUY TAKE LETTER FROM RIPLEY");
		_series6 = series_load("RIP TREK HAND TALK POS3");
		_series7 = series_load("RIP TREK TELEGRAM DISPLACEMENT");
		_series8 = series_load("GUY TELEGRAM FROM SLOT");
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_val4 = 0;
	_val5 = 0;

	kernel_timing_trigger(1, 507);
	digi_play_loop("950_s02", 3, 50);

	if (_flag1) {
		_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0,
			triggerMachineByHashCallback, "mc");
		sendWSMessage(1, _machine1, _series4, 1, 1, -1, _series4, 1, 1, 0);

		_machine2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0,
			triggerMachineByHashCallback, "rip");
		sendWSMessage(1, _machine2, _series2, 1, 21, -1, _series2, 21, 21, 0);
		kernel_timing_trigger(10, 100);

	} else {
		if (_val4 == 1)
			_machine3 = series_place_sprite("201DOC", 0, 0, 0, 100, 0x410);

		if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
			if (_G(flags)[V053] == 1) {
				_mei1 = series_load("MEI TREK HAND ON HIP POS4");
				_mei2 = series_load("MEI TREK TALKER POS4");
				_mei3 = series_load("MEI CHEN TREK WALK POS4");
				_machine4 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 200, 238, 73, 0x900, 0,
					triggerMachineByHashCallback, "MC");
				sendWSMessage(1, _machine4, _mei1, 22, 22, 2000, _mei1, 22, 22, 0);
				_val5 = 2;
				_val6 = 2;
				_val7 = -1;
				_val8 = -1;
				_val9 = -1;

			} else {
				hotspot_set_active("MEI CHEN", false);
			}
			return;
		}

		player_set_commands_allowed(false);
		ws_demand_location(79, 257, 3);

		if (_G(flags)[V053] == 2) {
			// TODO
		}
	}
}

void Room201::daemon() {
	// TODO
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
