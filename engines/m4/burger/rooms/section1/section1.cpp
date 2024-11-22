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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/core/imath.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

Section1::Section1() : Rooms::Section() {
	add(101, &_room101);
	add(102, &_room102);
	add(103, &_room103);
	add(104, &_room104);
	add(105, &_room105);
	add(106, &_room106);
	add(120, &_room120);
	add(133, &_room133_136);
	add(134, &_room134_135);
	add(135, &_room134_135);
	add(136, &_room133_136);
	add(137, &_room137);
	add(138, &_room138);
	add(139, &_room139_144);
	add(140, &_room140_141);
	add(141, &_room140_141);
	add(142, &_room142);
	add(143, &_room143);
	add(145, &_room145);
	add(144, &_room139_144);
	add(170, &_room170);
	add(171, &_room171);
	add(172, &_room172);
	add(173, &_room173);
	add(174, &_room174);
	add(175, &_room175);
	add(176, &_room176);
}

void Section1::updateWalker(int x, int y, int dir, int trigger, bool mode) {
	Section1 *s = dynamic_cast<Section1 *>(g_engine->_activeSection);
	assert(s);
	s->updateWalker_(x, y, dir, trigger, mode);
}

void Section1::updateWalker_(int x, int y, int dir, int trigger, bool mode) {
	_trigger = trigger;
	player_set_commands_allowed(false);
	ws_demand_location(_G(my_walker), x, y);
	ws_demand_facing(_G(my_walker), dir);
	ws_hide_walker(_G(my_walker));
	_G(roomVal3) = 0;

	gr_backup_palette();
	pal_fade_set_start(_G(master_palette), 0);
	_series1 = series_load("110", -1, _G(master_palette));
	_play = series_play("110", 0, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	kernel_trigger_dispatch_now(mode ? 1032 : 1027);
	kernel_timing_trigger(1, 1026);
}

void Section1::updateDisablePlayer() {
	player_update_info(_G(my_walker), &_G(player_info));
	player_set_commands_allowed(false);
	g_vars->getInterface()->freshen_sentence();
	walk(-1);

	_G(flags)[V000] = _G(flags)[kRoadOpened] ? 1002 : 1003;
}

void Section1::walk(int facing, int trigger) {
	if (_G(my_walker) && _G(player).walker_in_this_scene) {
		player_update_info(_G(my_walker), &_G(player_info));

		if (facing == -1)
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, trigger, _G(player_info).facing);
		else
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, trigger, facing, _G(completeWalk));
	}
}

void Section1::daemon() {
	switch (_G(kernel).trigger) {
	case GOTO_TOWN_HALL:
	case 1002:
	case 1003:
	case 1004:
	case 1005:
	case 1006:
	case 1007:
		_G(game).new_room = _G(kernel).trigger - 1000 + 100;
		break;
	case 1008:
		_G(game).new_room = _G(flags)[V000] == 1002 ? 134 : 135;
		break;
	case 1009:
		_G(game).new_room = _G(flags)[V000] == 1002 ? 133 : 136;
		break;
	case 1010:
		_G(game).new_room = 137;
		break;
	case 1011:
		_G(game).new_room = 138;
		break;
	case 1012:
		_G(game).new_room = _G(flags)[V000] == 1002 ? 144 : 139;
		break;
	case 1013:
		_G(game).new_room = _G(flags)[V000] == 1002 ? 141 : 140;
		break;
	case 1014:
		_G(game).new_room = 142;
		break;
	case 1015:
		_G(game).new_room = 143;
		break;
	case 1016:
		_G(game).new_room = 145;
		break;
	case 1017:
		_G(game).new_room = 170;
		break;
	case 1018:
		_G(game).new_room = 171;
		break;
	case 1019:
		_G(game).new_room = 172;
		break;
	case 1020:
		_G(game).new_room = 173;
		break;
	case 1021:
		_G(game).new_room = 174;
		break;
	case 1022:
		_G(game).new_room = 175;
		break;
	case 1023:
		_G(game).new_room = 176;
		break;
	case 1024:
		pal_fade_init(_G(master_palette), 0, 255, 100, 0, -1);
		break;
	case 1025:
		kernel_trigger_dispatch_now(_trigger);
		break;
	case 1026:
		pal_fade_init(_G(master_palette), 15, 255, 100, 0, -1);
		break;
	case 1027:
		_series2 = series_load("110bu01", -1, _G(master_palette));
		series_play("110bu01", 0, 0, 1028, 6, 0, 100, 0, 0, 0, 8);
		_state1 = 2;
		break;
	case 1028:
		kernel_trigger_dispatch_now(1029);
		digi_play((_G(game).room_id == 135) ? "100b002" : "100b001", 1, 255, 1030);
		break;
	case 1029:
		if (_state1 == 2) {
			int frame = imath_ranged_rand(9, 12);
			series_play("110bu01", 0, 0, 1029, 5, 0, 100, 0, 0, frame, frame);
		} else {
			pal_fade_set_start(_G(master_palette), 0);
			terminateMachineAndNull(_play);
			kernel_timing_trigger(1, 1031);
		}
		break;
	case 1030:
		_state1 = 1;
		return;
	case 1031:
		series_unload(_series2);
		series_unload(_series1);
		kernel_trigger_dispatch_now(_trigger);
		break;
	case 1032:
		_series2 = series_load("110bu02", -1, _G(master_palette));
		_state1 = 2;
		kernel_trigger_dispatch_now(1033);
		digi_play("100b003", 1, 255, 1034);
		break;
	case 1033:
		if (_state1 == 2) {
			int frame = imath_ranged_rand(0, 4);
			series_play("110bu02", 0, 0, 1033, 5, 0, 100, 0, 0, frame, frame);
		} else {
			series_play("110bu02", 0, 0, 1035, 5, 0, 100, 0, 0, 5, 16);
		}
		break;
	case 1034:
		_state1 = 1;
		break;
	case 1035:
		pal_fade_set_start(_G(master_palette), 0);
		terminateMachineAndNull(_play);
		kernel_timing_trigger(1, 1036);
		break;
	case 1036:
		series_unload(_series2);
		series_unload(_series1);
		kernel_trigger_dispatch_now(_trigger);
		break;

	case kAdvanceTime:
		if (_G(flags)[V012] == 2) {
			term_message("  Harry watches tv at: %d", _G(flags)[V009]);

			if (_G(flags).get_boonsville_time_and_display() == (int32)_G(flags)[V009]) {
				if (_G(game).room_id == 102) {
					term_message("make harry walk in");
					kernel_timing_trigger(1, 1037);
				} else {
					term_message("harry is back in chair");
					_G(flags)[V012] = 0;
				}
			}
		}

		_G(kernel).continue_handling_trigger = true;
		break;
	case kBurlEntersTown:
		_G(flags)[V000] = _G(flags)[kRoadOpened] ? 1002 : 1003;
		break;
	case kBurlGetsFed:
		if (!_G(flags)[kRoadOpened])
			_G(flags)[V063] = 1;
		break;
	case kBurlStopsEating:
		if (!_G(flags)[kRoadOpened])
			_G(flags)[V063] = 0;
		break;
	case kBurlLeavesTown:
		if (!_G(flags)[kRoadOpened])
			_G(flags)[V000] = 1004;
		break;
	case kBandEntersTown:
		_G(flags)[kTourBusAtDiner] = 1;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
