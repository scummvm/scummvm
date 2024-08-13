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

#include "m4/riddle/rooms/section4/room403.h"
#include "m4/riddle/rooms/section4/section4.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room403::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room403::init() {
	digi_preload("403_s02");
	digi_preload("403_s02a");
	digi_preload("403_s02b");
	digi_preload("950_s23");
	digi_preload("403_s01");

	player_set_commands_allowed(false);
	_bell = series_place_sprite("ONE FRAME BELL", 0, 0, 0, 100, 0xf00);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
		_val9 = 0;
		_val10 = 0;
		_val11 = 0;
		_val12 = 0;

		_G(flags)[V313] = player_been_here(403) && (
			(_G(flags)[V110] && inv_player_has("TURTLE")) ||
			inv_player_has("STEP LADDER") ||
			_G(flags)[GLB_TEMP_12] ||
			!inv_object_is_here("STEP LADDER")) ? 0 : 1;
		_plank = inv_object_in_scene("PLANK", 403) ? 2 : 0;

		_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);
	}

	_safariShadow = series_load("SAFARI SHADOW 3");
	hotspot_set_active("WOLF", false);
	hotspot_set_active("STEP LADDER", false);
	hotspot_set_active("STEP LADDER ", false);
	hotspot_set_active("EDGER", false);
	hotspot_set_active("PLANK", false);
	hotspot_set_active("TURTLE TREAT", false);

	if (_G(flags)[V139] == 2) {
		_G(flags)[V139] = 0;

		if (_G(flags)[V133] && _G(flags)[V131] != 403) {
			_edger = series_place_sprite("ONE FRAME EDGER", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("EDGER", true);
			inv_move_object("EDGER", 403);
		}

		MoveScreenDelta(-640, 0);
		ws_demand_location(1172, 322, 3);
		ws_walk(1172, 322, nullptr, 400, 1);

	} else if (_G(flags)[V139] == 4) {
		_G(flags)[V139] = 0;
		_ladder = series_place_sprite("LADDER LEANS AGAINST WALL", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("STEP LADDER ", true);
		MoveScreenDelta(-640, 0);
		ws_demand_location(1083, 322, 3);
		ws_walk(1201, 321, nullptr, 420, 2);

	} else {
		if (inv_player_has("TURTLE"))
			_G(flags)[V313] = 0;

		switch (_G(flags)[V313]) {
		case 1:
			_ladder = series_place_sprite("LADDER LEANS AGAINST WALL", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("STEP LADDER ", true);
			break;
		case 2:
			_ladder = series_place_sprite("1 sprite of ladder", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("STEP LADDER", true);
			break;
		default:
			break;
		}

		if (_G(flags)[V133] && !_G(flags)[GLB_TEMP_12] && _G(flags)[V131] != 403 &&
				!inv_player_has("TURTLE") && !inv_player_has("EDGER")) {
			_edger = series_place_sprite("ONE FRAME EDGER", 0, 0, 0, 0x100, 0xf00);
			hotspot_set_active("EDGER", true);
		}

		if (inv_object_is_here("PLANK") || _plank == 2) {
			_board = series_place_sprite("1 SPRITE OF BOARD", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("PLANK", true);
			_plank = true;
		}

		if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
			if (_G(flags)[V131] == 403) {
				hotspot_set_active("WOLF", true);
				_wolfTurnTalk = series_load("WOLF TURN AND TALK");
				_wolfTurnHand = series_load("WOLF TURN WITH HAND OUT");
				_wolfTalkLeave = series_load("WOLF TALKS AND LEAVES");
				_ripTalkPay = series_load("RIP TALK PAY LOOP");
				_wolfEdger = series_load("WOLF EDGER LOOP");

				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
					triggerMachineByHashCallbackNegative, "WOLFIE");

				if (_val12) {
					sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 45, 45, -1,
						_wolfTurnHand, 45, 45, 0);
				} else {
					_val8 = 2001;
					_val9 = 2300;
					sendWSMessage_10000(1, _wolfie, _wolfEdger, 1, 6, 110,
						_wolfEdger, 6, 6, 0);
				}
			}

			if (_val10) {
				ws_demand_facing(11);
				ws_hide_walker();
				_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
					triggerMachineByHashCallbackNegative, "RIP on ladder/plank");

				switch (_val11) {
				case 1:
					_ripClimbsLadder = series_load("RIPLEY CLIMBS LADDER");
					sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 52, 52, -1,
						_ripClimbsLadder, 52, 52, 0);
					break;
				case 2:
					_ripClimbsLadder = series_load("RIPLEY CLIMBS LADDER");
					sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 74, 74, -1,
						_ripClimbsLadder, 74, 74, 0);
					break;
				case 3:
					_ripLegUp = series_load("RIP GETS A LEG UP");
					_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);
					sendWSMessage_10000(1, _ripOnLadder, _ripLegUp, 44, 44, -1,
						_ripLegUp, 44, 44, 0);
					break;
				case 4:
					_ripLegUp = series_load("RIP GETS A LEG UP");
					_ripTurtle = series_load("RIP TURTLE SERIES");
					_noTreat = series_load("403RP06 NO TREAT");
					sendWSMessage_10000(1, _ripOnLadder, _noTreat, 28, 28, -1,
						_noTreat, 28, 28, 0);
					break;
				case 5:
					_ripLegUp = series_load("RIP GETS A LEG UP");
					_ripTurtle = series_load("RIP TURTLE SERIES");
					_noTreat = series_load("403RP06 NO TREAT");
					hotspot_set_active("GRATE", false);
					hotspot_set_active("TURTLE TREAT", true);

					if (_G(flags)[V125] == 2)
						sendWSMessage_10000(1, _ripOnLadder, _noTreat, 52, 52, -1,
						_noTreat, 52, 52, 0);
					else
						sendWSMessage_10000(1, _ripOnLadder, _noTreat, 28, 28, -1,
							_noTreat, 28, 28, 0);
					break;
				default:
					break;
				}
			} else {
				_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);
			}
		} else if (_G(flags)[V132]) {
			_G(flags)[V132] = 0;
			_G(camera_reacts_to_player) = false;
			MoveScreenDelta(-640, 0);
			ws_demand_location(620, 326, 3);
			ws_walk_load_shadow_series(S4_SHADOW_DIRS, S4_SHADOW_NAMES);
			ws_walk_load_walker_series(S4_NORMAL_DIRS, S4_NORMAL_NAMES);
			kernel_timing_trigger(1, 310);
		} else {
			if (_G(flags)[V131] == 403) {
				hotspot_set_active("WOLF", true);
				_wolfTurnTalk = series_load("WOLF TURN AND TALK");
				_wolfTurnHand = series_load("WOLF TURN WITH HAND OUT");
				_wolfTalkLeave = series_load("WOLF TALKS AND LEAVES");
				_ripTalkPay = series_load("RIP TALK PAY LOOP");
				_wolfEdger = series_load("WOLF EDGER LOOP");

				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
					triggerMachineByHashCallbackNegative, "WOLFIE");
				sendWSMessage_10000(1, _wolfie, _wolfEdger, 1, 6, 110,
					_wolfEdger, 6, 6, 0);
				_val8 = 2001;
				_val9 = 2300;
			}

			if (0) {
				ws_demand_location(4, 296);
				ws_walk(80, 300, nullptr, 300, 3);
			} else {
				MoveScreenDelta(-640, 0);
				ws_demand_location(1110, 322);
			}
		}
	}

	digi_play_loop("403_s01", 3, 180);
}

void Room403::daemon() {

}

void Room403::pre_parser() {

}

void Room403::parser() {

}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
