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

#include "m4/riddle/rooms/section6/room608.h"
#include "m4/riddle/rooms/section6/section6.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS1[] = { 220, 221, -1 };
static const char *NORMAL_NAMES1[] = { "oldwk1", "oldwk3" };
static const int16 SHADOW_DIRS1[] = { 220, 231, -1 };
static const char *SHADOW_NAMES1[] = { "tt walker shadow 1", "oldwk3s" };


void Room608::init() {
	if (_G(flags)[V186] != 2) {
		_rp09 = series_load("608rp09");

		if (!inv_object_is_here("POLE") || _G(flags)[V186])
			kernel_load_variant("608lock1");
		else
			kernel_load_variant("608lock2");

		_old01 = series_load("608old01");
		_old02 = series_load("608old02");
		_old05 = series_load("608old05");
		_old08 = series_load("608old08");
		_old05a = series_load("608old05a");
		_old05b = series_load("608old05b");
		_old05c = series_load("608old05c");
		_old05d = series_load("608old05d");
		_old05f = series_load("608old05f");

		ws_walk_load_walker_series(NORMAL_DIRS1, NORMAL_NAMES1);
		ws_walk_load_shadow_series(SHADOW_DIRS1, SHADOW_NAMES1);
		_ol = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
			triggerMachineByHashCallbackNegative, "ol");
		sendWSMessage_10000(1, _ol, _old01, 1, 1, 200, _old01, 1, 1, 0);
		_val1 = 0;
		_val2 = 0;
		_val3 = 0;
	}

	if (inv_object_is_here("DRIFTWOOD STUMP")) {
		_stump = series_show("608ST_DN", 0xf00, 16);
		hotspot_set_active("DRIFTWOOD STUMP ", false);
	} else {
		hotspot_set_active("DRIFTWOOD STUMP", false);
	}

	if (inv_object_is_here("LIGHTER") && _G(flags)[V203] >= 7) {
		_lighter = series_show("608LITER", 0xf00, 16);
	} else {
		hotspot_set_active("LIGHTER", false);
	}

	if (!inv_object_is_here("DRIFTWOOD STUMP")) {
		hotspot_set_active("DRIFTWOOD STUMP ", false);
		hotspot_set_active("DRIFTWOOD STUMP  ", false);
	} else if (_G(flags)[V186]) {
		hotspot_set_active("DRIFTWOOD STUMP ", false);
	} else {
		_stump = series_show("608ST_UP", 0x400, 16);
		hotspot_set_active("DRIFTWOOD STUMP  ", false);
	}

	if (!inv_object_is_here("POLE")) {
		hotspot_set_active("POLE", false);
		hotspot_set_active("POLE ", false);
	} else if (_G(flags)[V186]) {
		hotspot_set_active("POLE", false);
	} else {
		_pole = series_show("608POLE", 0x400, 16);
		hotspot_set_active("POLE ", false);
	}

	if (inv_object_is_here("DRIFTWOOD PUFFIN")) {
		_puffin = series_show("608PUFIN", 0x400, 16);
	} else {
		hotspot_set_active("DRIFTWOOD PUFFIN", false);
	}

	switch (_G(flags)[V186]) {
	case 0:
		hotspot_set_active("hole in hillside", false);
		_end1 = series_show("608START", 0xf00, 16);
		hotspot_set_active("STATUE ", false);
		digi_preload("608_s01");
		digi_preload("608_s01a");
		break;
	case 1:
		hotspot_set_active("hole in hillside", false);
		_end1 = series_show("608END", 0xf00, 16);
		hotspot_set_active("STATUE", false);
		hotspot_set_active("stone", false);
		digi_preload("950_s37");
		break;
	case 2:
		hotspot_set_active("hole in hillside", true);
		_end1 = series_show("608END", 0xf00, 16);
		_end2 = series_show("608END2", 0xe00, 16);
		hotspot_set_active("STATUE", false);
		hotspot_set_active("stone", false);
		hotspot_set_active("old woman", false);
	default:
		break;
	}

	if (_G(flags)[V203] == 2) {
		_tt01 = series_load("608TT01");
		_rp01 = series_load("608RP01");

		player_set_commands_allowed(false);
		_G(player).disable_hyperwalk = true;
		ws_demand_location(-30, 345, 1);
		ws_walk(43, 345, nullptr, 18, 3);
	
		ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
		ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);

		_tt = triggerMachineByHash_3000(8, 9, SECTION6_NORMAL_DIRS, SECTION6_SHADOW_DIRS,
			-30, 324, 3, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 105, 324, 5, 20, 1);
	} else if (_G(flags)[V203] == 6) {
		_tt02 = series_load("606TT02");
		_tt03 = series_load("608TT03");
		_tt05 = series_load("608TT05");
		_ripHandChin = series_load("RIP TREK HAND CHIN POS3");
		_ripTalker = series_load("RIP TREK TALKER POS3");
		_all5a = series_load("608all5a");
		_loop0 = series_load("608loop0");
		_loop1 = series_load("608loop1");

		player_set_commands_allowed(false);
		_G(player).disable_hyperwalk = true;
		ws_demand_location(-30, 345, 1);
		kernel_timing_trigger(1, 698);

		ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
		ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);

		_tt = triggerMachineByHash_3000(8, 9, SECTION6_NORMAL_DIRS, SECTION6_SHADOW_DIRS,
			-30, 324, 3, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 103, 318, 3, 700, 1);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s28b");
		break;

	case 603:
		digi_preload("950_s28b");
		digi_stop(3);
		digi_unload("950_s28a");
		break;

	case 605:
		digi_preload("950_s28b");
		digi_stop(3);
		digi_unload("950_s28");

		if (_G(flags)[V203] != 2) {
			player_set_commands_allowed(false);
			ws_demand_location(-30, 345, 3);
			ws_walk(43, 345, nullptr, 1, 3);
		}
		break;

	default:
		digi_preload("950_s28b");
		break;
	}

	series_play("608surf", 0xf00, 0, -1, 15, -1, 100, 0, 0, 0, -1);
	digi_play_loop("950_s28b", 3, 90);
}

void Room608::daemon() {
}

void Room608::pre_parser() {

}

void Room608::parser() {

}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
