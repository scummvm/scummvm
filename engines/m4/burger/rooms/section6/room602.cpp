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

#include "m4/burger/rooms/section6/room602.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room602::init() {
	player_set_commands_allowed(false);
	_G(flags)[V256] = 0;
	_series1 = 0;

	if (_G(game).room_id == 612) {
		digi_preload("612_001a");

		switch (_G(flags)[V277]) {
		case 6001:
			_val1 = 55;
			break;

		case 6002:
			_val1 = _G(flags)[V278] ? 56 : 55;
			break;

		case 6003:
			if (_G(flags)[V278]) {
				digi_preload("602_005");
				_val1 = 58;
			} else {
				_val1 = 57;
			}
			break;

		default:
			break;
		}

		if (_G(flags)[V278]) {
			term_message("Adding the pushed walk code.");
			_walk1 = intr_add_no_walk_rect(322, 304, 472, 329, 312, 320);
			hotspot_set_active("PAW", false);
			hotspot_set_active("MOTOR ", true);
			hotspot_set_active("MOTOR", false);
			hotspot_set_active("FLOOR  ", false);

		} else {
			term_message("Adding the non-pushed walk code.");
			term_message("x1 = %d, y1 = %d, x2 = %d, y2 = %d", 208, 304, 369, 329);
			_walk1 = intr_add_no_walk_rect(208, 304, 369, 329, 197, 322);
			hotspot_set_active("PAW", true);
			hotspot_set_active("MOTOR", true);
			hotspot_set_active("MOTOR ", false);
			hotspot_set_active("FLOOR  ", true);
		}

		kernel_trigger_dispatch_now(12);

		if (_G(game).previous_room != 609)
			kernel_trigger_dispatch_now(1);

		_val2 = 48;
	} else {
		_val2 = 0;
	}

	_G(flags)[V264] = 0;
	if (_G(flags)[V255] == 1)
		series_show("602spill", 0xf00);

	if (_G(flags)[V255] && _G(flags)[V278])
		_G(kernel).call_daemon_every_loop = true;

	if (_G(flags)[V277] == 6003 && _G(flags)[V278] == 1) {
		_series2 = series_play("612wheel", 0x5ff);

	} else if (_G(game).room_id == 502) {
		_series2 = series_show("602wheel", 0x6ff);

	} else {
		_series2 = series_show("612wheel", 0x6ff);
	}

	_series3 = series_show("602door", 0xf00, 1, -1, -1, 0, 100,
		_G(flags)[V257] / 21, _G(flags)[V257]);

	if (_G(flags)[V256] == 1) {
		_val3 = 63;
		kernel_trigger_dispatch_now(2);
	}

	if (_G(flags)[V256] == 3) {
		_val3 = 64;
		kernel_trigger_dispatch_now(2);
	}

	if (_G(flags)[V256]) {
		hotspot_set_active("DOOR", false);
		hotspot_set_active("EXIT", true);

	} else {
		hotspot_set_active("DOOR", true);
		hotspot_set_active("EXIT", false);
	}

	if (_G(flags)[V256] == 10028) {
		Section6::_state1 = 6002;
		kernel_trigger_dispatch_now(6013);
	}

	// TODO: Set table used for custom hotspot handling
	error("TODO: Ptr1");

	if (_G(flags)[V243] == 6006) {
		_series4 = series_load("602mg01");
		_series5 = series_load("602mg01s");
		_series6 = series_load("602mg02");
		_series7 = series_load("602mg02s");
		Section6::_state3 = 6001;
		kernel_timing_trigger(60, 6011);
	}

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(true);
		if (_G(flags)[V263]) {
			_G(wilbur_should) = 11;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 603:
		ws_hide_walker();
		if (_G(roomVal7) == 1) {
			_G(wilbur_should) = 2;
			kernel_timing_trigger(30, gCHANGE_WILBUR_ANIMATION);
		} else {
			_G(wilbur_should) = 3;
			kernel_timing_trigger(60, gCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 604:
		ws_hide_walker();
		_G(wilbur_should) = 3;
		kernel_timing_trigger(60, gCHANGE_WILBUR_ANIMATION);
		break;

	case 609:
		ws_demand_location(548, 355, 9);
		_G(wilbur_should) = 10002;

		hotspot_set_active("PAW", true);
		hotspot_set_active("MOTOR", true);
		hotspot_set_active("MOTOR ", false);
		hotspot_set_active("FLOOR  ", true);

		_series8 = series_play("612mot02", 0x700, 0, -1, 0, -1);
		_val1 = 53;
		kernel_trigger_dispatch_now(1);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(548, 355, 9);

		if (_G(game).room_id == 612) {
			_G(flags)[V277] = 6001;
			_G(flags)[V243] = 6007;
			_G(flags)[V244] = 6004;
			_G(flags)[V245] = 10031;
			_G(flags)[ROOM101_FLAG8] = 1;
			_val1 = 53;
			kernel_trigger_dispatch_now(1);
		}
		break;
	}

	if (_G(flags)[V243] == 6006)
		Section6::_state4 = 5;
	if (_G(flags)[V243] == 6007)
		Section6::_state4 = 9;
	else if (_G(flags)[V243] == 6000)
		Section6::_state4 = 2;

	kernel_trigger_dispatch_now(6014);
}

void Room602::daemon() {
}

void Room602::pre_parser() {

}

void Room602::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
