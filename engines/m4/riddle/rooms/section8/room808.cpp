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

#include "m4/riddle/rooms/section8/room808.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room808::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room808::init() {
	player_set_commands_allowed(false);

	ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
	ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2);

	_808Rp02Series = series_load("808rp02", -1, nullptr);
	_808Rp01Series = series_load("808rp01", -1, nullptr);
	_ripTalkerPos5Series = series_load("RIP TALKER POS 5", -1, nullptr);
	_rptmr15Series = series_load("RPTMR15", -1, nullptr);
	_ripMedReach1HandPos2Series = series_load("RIP MED REACH 1HAND POS2", -1, nullptr);
	_807Rp04Series = series_load("807rp04", -1, nullptr);
	_mctd61Series = series_load("mctd61", -1, nullptr);
	_mctd82aSeries = series_load("mctd82a", -1, nullptr);
	_ripPos3LookAroundSeries = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_ripLooksAroundInAweSeries = series_load("RIP LOOKS AROUND IN AWE", -1, nullptr);
	series_load("808mc98", -1, nullptr);
	series_load("808mc99", -1, nullptr);
	series_load("808pos1", -1, nullptr);
	series_load("808pos2", -1, nullptr);

	if (_G(flags[V096]) == 0) {
		// Unload the freshly loaded series? This doesn't make sense?!
		series_unload(series_load("808 RIP TEST BRIDGESHOVEL FAR", -1, nullptr));
		series_unload(series_load("808 RIP TEST BRIDGE", -1, nullptr));
		series_unload(series_load("808test3", -1, nullptr));
	}

	_808ChainMach = series_plain_play("808chain", -1, 0, 100, 0, 0, -1, true);
	series_play("LIT URN ", 767, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	if (inv_object_in_scene("FARMER'S SHOVEL", 808)) {
		if (_G(flags[V095])) {
			switch (_G(flags[V094])) {
			case 1:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 6, 100, 0, 0);
				break;

			case 2:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 4, 100, 0, 0);
				break;

			case 3:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 3, 100, 0, 0);
				break;

			case 4:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 5, 100, 0, 0);
				break;

			default:
				break;
			}
		} else {
			_808PosMach = series_show("808pos2", 1281, 0, -1, -1, _G(flags[V094]), 100, 0, 0);
		}
	} else {
		if (_G(flags[V095])) {
			switch (_G(flags[V094])) {
			case 1:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 6, 100, 0, 0);
				break;

			case 2:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 4, 100, 0, 0);
				break;

			case 3:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 3, 100, 0, 0);
				break;

			case 4:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 5, 100, 0, 0);
				break;

			default:
				break;
			}
		} else {
			_808PosMach = series_show("808pos1", 1281, 0, -1, -1, _G(flags[V094]), 100, 0, 0);
		}
	}

	if (inv_object_in_scene("crank", 808) && _G(flags[V098]) == 0) {
		_808HandleSpriteMach = series_show("808 handle sprite", 4095, 0, -1, -1, 0, 100, 0, 0);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s29", -1);
		if (_G(flags[V097])) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 345, 115, 5, Walker::player_walker_callback, "mc_trek");
		} else {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 112, 238, 5, Walker::player_walker_callback, "mc_trek");
		}

		kernel_timing_trigger(1, 1, nullptr);
		break;

	case 809:
		ws_demand_facing(_G(my_walker), 5);
		ws_demand_location(_G(my_walker), 459, 36);
		ws_walk(_G(my_walker), 382, 116, nullptr, -1, 7, true);
		_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 448, 38, 5, Walker::player_walker_callback, "mc_trek");
		DisposePath(_mcTrekMach->walkPath);
		_mcTrekMach->walkPath = CreateCustomPath(427, 96, 345, 115, -1);
		ws_custom_walk(_mcTrekMach, 5, 1, true);

		break;

	default:
		_808RpupSeries = series_load("808rpup", -1, nullptr);
		ws_demand_facing(_G(my_walker), 2);
		ws_demand_location(_G(my_walker), 18, 216);
		ws_hide_walker(_G(my_walker));

		if (_G(flags[V276]) == 0) {
			_808McupSeries = series_load("808mcup", -1, nullptr);
			series_play("808rpup", 0, 0, 2, 5, 0, 100, 0, 0, 0, -1);
		} else {
			_G(flags[V276]) = 0;
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 345, 116, 5, Walker::player_walker_callback, "mc_trek");
			series_play("808rpup", 0, 0, 4, 5, 0, 100, 0, 0, 0, -1);
		}

		break;

	}

	digi_play_loop("950_s29", 3, 96, -1, -1);
}

void Room808::pre_parser() {
}

void Room808::parser() {
}

void Room808::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
