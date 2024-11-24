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

#include "m4/riddle/rooms/section7/room702.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room702::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room702::init() {
	_field3C = player_been_here(702) ? 1 : 0;
	_G(flags[V212]) = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_field40 = 0;
		_field44 = 0;
		_field48 = 0;
		_field4C = -1;
		_field50 = 0;
		_field54 = 0;
		_field58 = -1;
	}

	if (_G(game).previous_room == 703) {
		_safariShadow1Series = series_load("SAFARI SHADOW 1", -1, nullptr);
		_guardTalksAndBowsSeries = series_load("GUARD TALKS AND BOWS", -1, nullptr);
		_guardStepsAsideTalksSeries = series_load("GUARD STEPS ASIDE TALKS", -1, nullptr);
		_702GuardShadow1Series = series_load("702 GUARD SHADOW1", -1, nullptr);
		_702GuardShadow2Series = series_load("702 GUARD SHADOW2", -1, nullptr);
		_ripShowsRingSeries = series_load("RIP SHOWS RING", -1, nullptr);
		_ripSafariWalkerPos1Series = series_load("RIP SAFARI WALKER POSITION 1", -1, nullptr);
		_ringCloseupSeries = series_load("Ring closeup", -1, nullptr);
		_G(flags[V210]) = 0;
		_field40 = 0;

		if (_G(flags[V224]) == 0) {
			_guardMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 768, false, callback, "guard");
			sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 1, 1, -1, _guardTalksAndBowsSeries, 1, 1, 0);
			_guardShadowMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 487, 326, 100, 1024, false, callback, "guard shadow");
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 1, 1, -1, _702GuardShadow1Series, 1, 1, 0);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "GUARD", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "RING", false);
		}

		player_first_walk(519, 322, 5, 534, 346, 1, true);
	} else {
		digi_preload("950_s39");
		_safariShadow1Series = series_load("SAFARI SHADOW 1", -1, nullptr);
		_guardTalksAndBowsSeries = series_load("GUARD TALKS AND BOWS", -1, nullptr);
		_guardStepsAsideTalksSeries = series_load("GUARD STEPS ASIDE TALKS", -1, nullptr);
		_702GuardShadow1Series = series_load("702 GUARD SHADOW1", -1, nullptr);
		_702GuardShadow2Series = series_load("702 GUARD SHADOW2", -1, nullptr);
		_ripShowsRingSeries = series_load("RIP SHOWS RING", -1, nullptr);
		_ripSafariWalkerPos1Series = series_load("RIP SAFARI WALKER POSITION 1", -1, nullptr);
		_ringCloseupSeries = series_load("Ring closeup", -1, nullptr);
		_G(flags[V210]) = 0;
		_field40 = 0;
		if (_G(flags[V224]) == 0) {
			_guardMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 768, false, callback, "guard");
			sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 1, 1, -1, _guardTalksAndBowsSeries, 1, 1, 0);
			_guardShadowMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 487, 326, 100, 1024, false, callback, "guard shadow");
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 1, 1, -1, _702GuardShadow1Series, 1, 1, 0);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "GUARD", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "RING", false);
		}

		if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
			player_first_walk(660, 347, 11, 524, 342, 11, true);
		}
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room702::pre_parser() {
	if (player_said_any("  ", "   ") && _G(flags[V211]) && _G(flags[V210]) && _G(flags[V212])) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (_G(flags[V224]) && player_said_any("  ", "   ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room702::parser() {
	//TODO
}

void Room702::daemon() {
	//TODO
}

void Room702::callback(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
