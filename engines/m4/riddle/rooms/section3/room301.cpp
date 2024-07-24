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

#include "m4/riddle/rooms/section3/room301.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room301::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	_G(player).walker_in_this_scene = true;
	LoadWSAssets("OTHER SCRIPT");
}

void Room301::init() {
	_G(flags)[V000] = 1;
	digi_preload("950_s01");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = 0;
		_val3 = 0;
		_val4 = 0;
		_val5 = -1;
		_val6 = 2;
		_val7 = 2;
		_val8 = 0;
		_val9 = 0;
		_val10 = 0;
		_val11 = 0;
		_val12 = 0;
	}

	_val13 = 0;
	_val14 = 0;

	_ripTrekHandTalk3 = series_load("rip trek hand talk pos3");
	_ripTrekTalker3 = series_load("RIP TREK TALKER POS3");
	_ripTrekTravel = series_load("rip trek travel disp");
	_agentStander = series_load("agent stander / talk disp");
	_agentCheckingList = series_load("agent checking list on desk");
	_agentSlidesPaper = series_load("AGENT SLIDES PAPER TO RIP");
	_agentTakesMoney = series_load("agent takes money from rip");
	_agentTakesTelegram = series_load("agent takes telegram from slot");
	_agentTalk = series_load("agent animated talk disp");
	_agentSalutes = series_load("agent salutes rip");
	_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
		triggerMachineByHashCallbackNegative, "guy behind desk");

	sendWSMessage(1, _machine1, _agentStander, 1, 1, 10, _agentStander, 1, 1, 0);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		if (inv_object_in_scene("ROMANOV EMERALD", 305)) {
			setFlag45();
			inv_move_object("ROMANOV EMERALD", NOWHERE);
		}

		_val12 = checkFlags(false);
		player_set_commands_allowed(false);
		ws_demand_location(55, 270, 3);

		if (_val12 || (!player_been_here(401) && _G(flags)[V092] && !_G(flags)[V093])) {
			ws_walk(352, 269, nullptr, 60, 3);
		} else if (!player_been_here(301)) {
			digi_preload("301r37");
			digi_play("301r37", 1, 255, 51);
			ws_walk(165, 270, nullptr, -1, 3);
		} else {
			ws_walk(165, 270, nullptr, 51, 3);
		}
	}

	digi_play_loop("950_s01", 3, 140);
}

void Room301::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
