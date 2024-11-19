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

#include "m4/riddle/rooms/section7/room701.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room701::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room701::init() {
	if (_G(flags[V286]))
		_G(flags[V224]) = 1;

	_itemDigiName = nullptr;
	_field8C = 0;
	_field50 = 0;
	_field9E = -1;
	_field130 = 0;

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		_agentTalkLoopTjSeries = series_load("AGENT TALK LOOP TJ", -1, nullptr);
		_agentGetTelegramSeries = series_load("AGENT  GET TELEGRAM", -1, nullptr);
		_agentShowMapSeries = series_load("AGENT SHOW MAP", -1, nullptr);
		_agentGiveParcelSeries = series_load("AGENT  GIVE PARCEL", -1, nullptr);
		_701rp01Series = series_load("701RP01", -1, nullptr);
		_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
		_ripTrekTalkerPos3Series = series_load("RIP TREK TALKER POS3", -1, nullptr);
		_ripTrekMedReachPos3Series = series_load("RIP TREK MED REACH POS3", -1, nullptr);
		_701rp99Series = series_load("701RP99", -1, nullptr);
		_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, -1, _agentTalkLoopTjSeries, 13, 13, 0);
	} else {
		_field88 = 0;

		player_set_commands_allowed(false);
		_agentTalkLoopTjSeries = series_load("AGENT TALK LOOP TJ", -1, nullptr);
		_agentGetTelegramSeries = series_load("AGENT  GET TELEGRAM", -1, nullptr);
		_agentShowMapSeries = series_load("AGENT SHOW MAP", -1, nullptr);
		_agentGiveParcelSeries = series_load("AGENT  GIVE PARCEL", -1, nullptr);
		_701rp01Series = series_load("701RP01", -1, nullptr);
		_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
		_ripTrekTalkerPos3Series = series_load("RIP TREK TALKER POS3", -1, nullptr);
		_ripTrekMedReachPos3Series = series_load("RIP TREK MED REACH POS3", -1, nullptr);
		_701rp99Series = series_load("701RP99", -1, nullptr);
		_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, -1, _agentTalkLoopTjSeries, 13, 13, 0);

		++_G(flags[V006]);
		if (setItemsPlacedFlags()) {
			_field88 = 1;
			_itemDigiName = getItemsPlacedDigi();
			++_field50;
		}

		ws_demand_location(_G(my_walker), 50, 264);
		ws_demand_facing(_G(my_walker), 3);
		_field134 = 0;
		ws_walk(_G(my_walker), 352, 251, nullptr, player_been_here(701) ? 80 : 40, 3, true);
	}

	digi_preload("701_s01", -1);
	digi_play_loop("701_s01", 2, 45, -1, -1);
}

void Room701::pre_parser() {
	if (player_said("rm702") && !_field88) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room701::parser() {
	warning("STUB - 701 Parser");
}

void Room701::daemon() {
	warning("STUB - 701 Daemon");
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
