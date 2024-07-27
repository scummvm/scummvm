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

#include "m4/riddle/rooms/section3/room303.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room303::preload() {
	LoadWSAssets("OTHER SCRIPT");

	if (_G(flags)[V000]) {
		_G(art_base_override) = player_been_here(201) ?
			"EXHIBIT HALL-TREK" : "EH TREK NO SNAKE";
		_G(use_alternate_attribute_file) = true;
		_G(player).walker_type = 1;
		_G(player).shadow_type = 1;
		_G(player).walker_in_this_scene = true;

	} else {
		_G(player).walker_type = 0;
		_G(player).shadow_type = 0;
		_G(player).walker_in_this_scene = true;

		if (_G(game).room_id == 352) {
			_G(player).walker_in_this_scene = false;
			_G(player).disable_hyperwalk = true;
		}
	}
}

void Room303::init() {
	_val1 = _val2 = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val3 = 0;
		_val4 = -1;
		_triggerMode1 = _triggerMode2 = KT_DAEMON;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
	}

	if (player_been_here(301)) {
		hotspot_set_active("MEI CHEN", false);
		hotspot_set_active("FENG LI", false);
		hotspot_set_active("COVER", false);
	}

	if (_G(game).previous_room != 304) {
		_door = series_show_sprite("DOOR", 0, 0xf05);
	}

	switch (_G(game).previous_room) {
	case 301:
		break;
	default:
		break;
	}
}

void Room303::daemon() {
}

void Room303::loadHands() {
	_hands1 = series_load("MC NY hands behind back pos4");
	_hands2 = series_load("MC NY hand on hip pos4");
	_hands3 = series_load("MC NY hand out talk pos4");
	_hands4 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 357, 255, 86, 0xf00, 0,
		triggerMachineByHashCallbackNegative, "mc");

	_G(kernel).trigger_mode = KT_DAEMON;
	sendWSMessage_10000(1, _hands4, _hands1, 1, 1, 200,
		_hands1, 1, 1, 0);
	_val10 = _val11 = 0;
}

void Room303::loadClasped() {
	_clasped1 = series_load("hands clasped pos5");
	_clasped2 = series_load("shrug head shake pos5");
	_clasped3 = series_load("hands clasped bow pos5");
	_clasped4 = series_load("hands clasped flame loop pos5");
}

void Room303::setFengActive(bool flag) {
	for (HotSpotRec *hotspot = _G(currentSceneDef).hotspots;
		hotspot; hotspot = hotspot->next) {
		if (!strcmp(hotspot->vocab, "FENG LI")) {
			if (flag) {
				hotspot->active = hotspot->lr_x < 600;
			} else {
				hotspot->active = hotspot->lr_x > 600;
			}
		}
	}
}

void Room303::setShadow4(bool active) {
	if (active) {
		_shadow4 = series_place_sprite("candleman shadow4", 0, 360, 252, -86, 0xe06);
	} else {
		terminateMachineAndNull(_shadow4);
	}
}

void Room303::setShadow5(bool active) {
	if (active) {
		_shadow5 = series_place_sprite("candleman shadow5", 0, 480, 256, -84, 0xe06);
	} else {
		terminateMachineAndNull(_shadow5);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
