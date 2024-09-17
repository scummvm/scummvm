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

#include "m4/riddle/rooms/section6/room603.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS1[] = { 200, -1 };
static const char *NORMAL_NAMES1[] = { "shen guo walker 3" };
static const int16 SHADOW_DIRS1[] = { 210, -1 };
static const char *SHADOW_NAMES1[] = { "shen guo shadow 3" };
static const int16 NORMAL_DIRS2[] = { 220, -1 };
static const char *NORMAL_NAMES2[] = { "kuang walker 2" };
static const int16 SHADOW_DIRS2[] = { 230, -1 };
static const char *SHADOW_NAMES2[] = { "kuangs shadow 2" };

void Room603::preload() {
	if (_G(flags)[V191]) {
		_G(art_base_override) = "EXPLOSION BACKGROUND";
		_G(use_alternate_attribute_file) = false;
		_G(player).walker_in_this_scene = false;
	}

	Room::preload();
}

void Room603::init() {
	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = 0;
		_val3 = 0;
		_val4 = 0;
		_val5 = 0;
	}

	_val6 = 0;

	if (_G(game).previous_room != 604 && _G(game).previous_room != KERNEL_RESTORING_GAME &&
			inv_player_has("PULL CORD") && !inv_object_is_here("POLE") &&
			_G(flags)[V203] == 4) {
		_G(flags)[V203] = 5;
		_G(flags)[GLB_TEMP_5] = 0;
	}

	if (_G(flags)[V191]) {
		interface_hide();
		player_set_commands_allowed(false);
		digi_preload("explode");
		digi_preload("950_s28a");
		digi_play_loop("950_s28a", 3, 90);
		series_play("653 RIP HOPS ROCK", 0xf00, 16, -1, 5);
		series_load("653 FIRST FRAME SHED");
		_shed = series_show("653 FIRST FRAME SHED", 0xf00, 16);

		ws_walk_load_walker_series(NORMAL_DIRS1, NORMAL_NAMES1);
		ws_walk_load_shadow_series(SHADOW_DIRS1, SHADOW_NAMES1);

		_shenWalker = triggerMachineByHash_3000(8, 12, NORMAL_DIRS1, SHADOW_DIRS1, 525, 238, 3,
			triggerMachineByHashCallback3000, "shen suit gun walker");
		sendWSMessage_10000(_shenWalker, 535, 235, 3, 10, 1);

		ws_walk_load_walker_series(NORMAL_DIRS2, NORMAL_NAMES2);
		ws_walk_load_shadow_series(SHADOW_DIRS2, SHADOW_NAMES2);

		_kuangWalker = triggerMachineByHash_3000(8, 13, NORMAL_DIRS2, SHADOW_DIRS2, 554, 245, 2,
			triggerMachineByHashCallback3000, "kuang suit walker");
		sendWSMessage_10000(_kuangWalker, 563, 244, 2, -1, 1);
		return;
	}

	if (_G(flags)[V203] != 9) {
		digi_preload("603_S02");
		digi_preload("603_S02a");
		digi_preload("603_S02b");
		digi_preload("603_S02c");

		if (_G(flags)[V203] == 0) {
			digi_preload("603_S01");
			digi_preload("603_S01A");
			digi_preload("603_S01B");
			_rp01 = series_load("603RP01");
			_rp02 = series_load("603RP02");
		}

		_tt03 = series_load("603TT03");
		_ttD01 = series_load("603TTD01");
		_rp04 = series_load("603RP04");
		_ripHandsBehindBack = series_load("RIP TREK HANDS BEHIND BACK POS2");
		_ripTalk = series_load("RIP TREK TALK");
		_ripHandsHips = series_load("RIP TREK HANDS HIPS POS2");
		_ripHandChin = series_load("RIP TREK HAND ON CHIN POS2");
		_ripTwoHandTalk = series_load("RIP TREK TWO HAND TALK POS2");
	}

	if (_G(flags)[V203] == 9) {
		_note = series_show("603NOTE", 0x700, 16);
		hotspot_set_active("NOTE", false);
		hotspot_set_active("twelvetrees", false);
		hotspot_set_active("twelvetrees ", false);
		hotspot_set_active("person in pit", false);
	} else {
		hotspot_set_active("NOTE ", false);

		if (!inv_object_is_here("TWELVETREES' NOTE")) {
			hotspot_set_active("NOTE", false);
		} else if (_G(flags)[V203] == 4) {
			_note = series_show("603NOTE", 0x800, 16);
		} else {
			hotspot_set_active("NOTE", false);
		}
	}

	if (inv_object_is_here("POLE")) {
		_pole = series_show("603POLE", 0x400, 16);
	} else {
		hotspot_set_active("POLE", false);
	}

	if (_G(flags)[V203] < 1) {
		_shirt = series_show("603SHIRT", 0x400, 16);
		hotspot_set_active("SLEEVE", false);
	} else {
		hotspot_set_active("SHIRT", false);

		if (inv_object_is_here("SLEEVE"))
			_sleeve = series_show("603SLEEV", 0x400, 16);
		else
			hotspot_set_active("SLEEVE", false);
	}

	if (_G(flags)[V203] == 9) {
		hotspot_set_active("POWER LINE", false);
		hotspot_set_active("shed", false);
		hotspot_set_active("wire", false);
		hotspot_set_active("  ", false);
		series_show("603ROCKS", 0xf00, 16);
	} else {
		series_load("603 POWER LINE");
		series_show("603 POWER LINE", 0, 16);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload(_G(globals)[GLB_TEMP_5] ? "genrun" : "950_s28a");
		break;

	case 604:
		player_set_commands_allowed(false);
		digi_preload("950_s28a");
		ws_demand_location(542, 230, 2);
		ws_walk(534, 240, nullptr, 5, 7);
		break;

	case 605:
		if (_G(globals)[GLB_TEMP_5]) {
			digi_preload("genrun");
		} else {
			digi_preload("950_s28a");
			digi_stop(3);
			digi_unload("950_s28");
		}

		player_set_commands_allowed(false);
		ws_demand_location(670, 232, 2);
		ws_walk(497, 245, nullptr, 1, 8);
		break;

	default:
		player_set_commands_allowed(false);
		digi_preload("950_s28a");
		ws_demand_location(67, 391, 2);
		ws_walk(135, 356, nullptr, 1, 2);
		break;
	}

	switch (_G(flags)[V203]) {
	case 0:
		digi_preload("17_04p01");
		digi_preload("17_04n06");
		_G(player).disable_hyperwalk = true;
		_G(kernel).call_daemon_every_loop = true;

		_door = series_show("603DOOR", 0xf00, 16, -1, -1, 23, 100, 0, 0);
		_ttDigShirtOff = series_load("TT DIG LOOP NO SHIRT");
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
			triggerMachineByHashCallbackNegative, "tt");
		sendWSMessage_10000(1, _tt, _ttDigShirtOff, 2, 2, 200, _ttDigShirtOff, 2, 2, 0);

		_val1 = 200;
		_val7 = 1;
		_val8 = 0;
		_val9 = 0;
		_val10 = 0;

		hotspot_set_active("twelvetrees ", false);
		hotspot_set_active("twelvetrees", false);
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		_door = series_show("603DOOR", 0xf00, 16);
		hotspot_set_active("wire", false);
		_ttDigShirtOn = series_load("TT DIG LOOP SHIRT ON");
		_val7 = 6;
		_val8 = 17;
		_val9 = 0;
		_val10 = 0;
		hotspot_set_active("person in pit", false);

		if (_val5) {
			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
				triggerMachineByHashCallbackNegative, "tt");
			sendWSMessage_10000(1, _tt, _ttD01, 1, 1, 400, _ttD01, 1, 1, 0);
			_val1 = 400;
			hotspot_set_active("twelvetrees ", false);
			_ttShadow = series_show("tt walker shadow 4", 0xf00, 0, -1, -1, 0, 53, 291, 293);

		} else {
			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
				triggerMachineByHashCallbackNegative, "tt");
			sendWSMessage_10000(1, _tt, _ttDigShirtOn, 1, 1, 500,
				_ttDigShirtOn, 1, 1, 0);
			_val1 = 500;
			hotspot_set_active("twelvetrees", false);
		}
		break;

	default:
		if (_G(game).previous_room == 604)
			_door = series_show("603DOOR", 0xf00, 16, -1, -1, 14, 100, 0, 0);
		else
			_door = series_show("603DOOR", 0xf00, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active("wire", false);
		hotspot_set_active("twelvetrees", false);
		hotspot_set_active("twelvetrees ", false);
		hotspot_set_active("person in pit", false);
		break;
	}

	if (_G(flags)[GLB_TEMP_5])
		digi_play_loop("genrun", 3, 140, -1, 604);
	else
		digi_play_loop("950_s28a", 3, 90, -1, -1);
}

void Room603::daemon() {
	// TODO
}

void Room603::pre_parser() {
	// TODO
}

void Room603::parser() {
	// TODO
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
