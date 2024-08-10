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

#include "m4/riddle/rooms/section4/room402.h"
#include "m4/riddle/rooms/section4/section4.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room402::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room402::init() {
	player_set_commands_allowed(false);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = 0;
		_val5 = -1;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
		_val9 = -1;
		_val10 = 0;
		_val11 = 0;
		_val12 = 0;
		_val13 = 0;
	}

	hotspot_set_active("WOLF", false);
	hotspot_set_active("TOPIARY", false);
	hotspot_set_active("TOPIARY ", false);

	_wolfie = series_place_sprite("WOLFIE WHLBRW OUTSIDE CASTLE", 0, 0, 0, 100, 0x300);
	_shadow3 = series_load("SAFARI SHADOW 3");

	digi_preload("950_s19");
	digi_preload("950_s20");
	digi_preload("950_s21");
	digi_preload("950_s23");

	if (!_G(flags)[GLB_TEMP_12] && !_G(flags)[V117]) {
		if (!_G(flags)[V110])
			_G(flags)[V131] = 402;
		else if (inv_player_has("TURTLE"))
			_G(flags)[V131] = 408;
		else
			_G(flags)[V131] = _G(flags)[V124] ? 402 : 403;
	}

	if (inv_player_has("PLANK")) {
		_G(flags)[V139] = 1;
	} else if (inv_player_has("STEP LADDER")) {
		_G(flags)[V139] = 2;
	} else if (inv_player_has("EDGER")) {
		_G(flags)[V139] = inv_player_has("TURTLE") ? 3 : 4;
	} else {
		ws_walk_load_walker_series(S4_NORMAL_DIRS, S4_NORMAL_NAMES);
		ws_walk_load_shadow_series(S4_SHADOW_DIRS, S4_SHADOW_NAMES);

		if (!player_been_here(402)) {
			digi_preload("950_s22");
			hotspot_set_active("WOLF", true);
			hotspot_set_active("TOPIARY", true);

			_G(flags)[V131] = 402;
			_G(flags)[V318] = 0;

			_ripPaysWolfie = series_load("RIP PAYS WOLFIE");
			_ripTalkWolf = series_load("RIP TALKS TO WOLF");
			_ripTalker = series_load("RIP TREK TALKER POS3");
			_wolfClipping = series_load("WOLF CLIPPING LOOP");
			_wolfClippersDown = series_load("WOLF CLPRS DOWN TURNS POS9");
			_wolfShakesHead = series_load("WOLF TALKS SHAKES HEAD");
			_wolfWantsMoney = series_load("WOLF WANTS MONEY");
			_ripHeadTurn = series_load("RIP TREK HEAD TURN POS3");

			_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
				triggerMachineByHashCallbackNegative, "WOLFIE");
			sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
				_wolfClipping, 10, 10, 0);
			_val12 = 2001;
			_val13 = 2300;
			ws_demand_facing(11);

			// The original uses a getenv("KITTY") call
#ifdef KITTY_SCREAMING
			ws_demand_location(425, 285);
			player_set_commands_allowed(true);
#else
			digi_preload("950_s22");
			_ripDownStairs = series_load("RIP DOWN STAIRS");
			ws_hide_walker();
			_ripLeaving = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
				triggerMachineByHashCallbackNegative, "rip leaving castle");
			sendWSMessage_10000(1, _ripLeaving, _ripDownStairs, 1, 27, 55,
				_ripDownStairs, 27, 27, 0);
#endif

		} else if (_G(flags)[V131] != 402) {
			hotspot_set_active("TOPIARY ", true);

			switch (_G(game).previous_room) {
			case KERNEL_RESTORING_GAME:
				digi_preload("950_s22");

				if (_G(flags)[V117]) {
					_branch = series_place_sprite("sprite of the pulled out branch",
						0, 0, -53, 100, 0x300);
				}

				player_set_commands_allowed(true);
				break;

			case 404:
				digi_preload("950_s22");
				_ripDownStairs = series_load("RIP DOWN STAIRS");
				ws_hide_walker();
				_ripLeaving = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
					triggerMachineByHashCallbackNegative, "rip leaving castle");
				sendWSMessage_10000(1, _ripLeaving, _ripDownStairs, 1, 27, 55,
					_ripDownStairs, 27, 27, 0);
				break;

			case 408:
				ws_demand_location(517, 239, 3);
				ws_walk(510, 260, 0, 50, 8);
				break;

			default:
				digi_preload("950_s22");
				ws_demand_location(660, 290, 3);
				ws_walk(612, 287, nullptr, 50, 9);
				midi_fade_volume(0, 120);
				break;
			}
		} else {
			if (!_G(flags)[V117]) {
				hotspot_set_active("WOLF", true);
				hotspot_set_active("TOPIARY", true);

				_ripPaysWolfie = series_load("RIP PAYS WOLFIE");
				_ripTalkWolf = series_load("RIP TALKS TO WOLF");
				_ripTalker = series_load("RIP TREK TALKER POS3");
				_wolfClipping = series_load("WOLF CLIPPING LOOP");
				_wolfClippersDown = series_load("WOLF CLPRS DOWN TURNS POS9");
				_wolfShakesHead = series_load("WOLF TALKS SHAKES HEAD");
				_wolfWantsMoney = series_load("WOLF WANTS MONEY");

				_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
					triggerMachineByHashCallbackNegative, "WOLFIE");

				if (_val6) {
					sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 16, -1,
						_wolfWantsMoney, 16, 16, 0);
				} else {
					_val12 = 2001;
					_val13 = 2300;
					sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
						_wolfClipping, 10, 10, 0);
				}
			}

			switch (_G(game).previous_room) {
			case KERNEL_RESTORING_GAME:
				digi_preload("950_s22");

				if (_G(flags)[V117])
					_branch = series_place_sprite("sprite of the pulled out branch", 0, 0, -53, 100, 0x300);
				player_set_commands_allowed(true);
				break;

			case 404:
				digi_preload("950_s22");
				_ripDownStairs = series_load("RIP DOWN STAIRS");
				ws_hide_walker();

				_ripLeaving = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
					triggerMachineByHashCallbackNegative, "rip leaving castle");
				sendWSMessage_10000(1, _ripLeaving, _ripDownStairs, 1, 27, 55,
					_ripDownStairs, 27, 27, 0);
				break;

			case 408:
				if (_G(flags)[V125] == 3) {
					_G(flags)[GLB_TEMP_12] = 1;
					_G(flags)[V125] = 4;
					_G(flags)[V131] = 999;

					_wolfClipping = series_load("WOLF CLIPPING LOOP");
					_wolfClippersDown = series_load("WOLF CLPRS DOWN TURNS POS9");
					_wolfTurnsClipping = series_load("WOLF TURNS CLPNG TO POS1");
					_turtlePopup = series_load("402 TURTLE POPUP");
					_ripLeanWall = series_load("Rip leans against far wall");

					_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
						triggerMachineByHashCallbackNegative, "WOLFIE");
					_val12 = 2001;
					_val13 = 2300;

					sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 100,
						_wolfClipping, 10, 10, 0);
					ws_demand_location(517, 239, 3);
					ws_walk(503, 248, nullptr, 350, 7);
				} else if (_G(flags)[V117] != 0 && inv_player_has("TURTLE")) {
					_G(flags)[V117] = 0;
					hotspot_set_active("TOPIARY ", true);
					_G(flags)[V131] = 408;
					ws_demand_location(517, 239, 3);

					_wolfWalker = triggerMachineByHash_3000(8, 8, S4_NORMAL_DIRS, S4_SHADOW_DIRS,
						475, 300, 11, triggerMachineByHashCallback3000, "wolf_walker");
					sendWSMessage_10000(_wolfWalker, 549, 239, 9, 42, 0);
					kernel_timing_trigger(90, 40);
				} else {
					ws_demand_location(517, 239, 3);
					ws_walk(449, 317, nullptr, 30, 3);
				}
				break;

			default:
				digi_preload("950_s22");
				ws_demand_location(660, 290, 3);
				ws_walk(449, 317, nullptr, 30, 3);
				midi_fade_volume(0, 120);

				if (inv_player_has("TURTLE"))
					inv_move_object("STEP LADDER", NOWHERE);
				break;
			}
		}
	}

	if (_G(flags)[V139]) {
		_wolfClipping = series_load("WOLF CLIPPING LOOP");
		_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
			triggerMachineByHashCallbackNegative, "WOLFIE");
		_val12 = 2001;
		_val13 = 2300;

		sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
			_wolfClipping, 10, 10, 0);
		ws_demand_location(517, 239, 3);
		ws_walk(510, 260, nullptr, 370, 8);
	}

	digi_play_loop("950_s22", 3, 120);
}

void Room402::daemon() {
}

void Room402::pre_parser() {
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool lookFlag = player_said_any("look", "look at");

	if (lookFlag && player_said(" ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("journal") && !takeFlag && !lookFlag && _G(kernel).trigger == -1) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("DANZIG") && !player_said("ENTER", "DANZIG")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (!_G(flags)[V112] && !player_said("WALK TO") &&
		(!talkFlag || !player_said("WOLF")) &&
		!player_said("POMERANIAN MARKS", "WOLF"))
		_G(flags)[V112] = 1;

	_val6 = 0;

	if (player_said("POMERANIAN MARKS", "WOLF") && inv_player_has("POMERANIAN MARKS")) {
		player_set_commands_allowed(false);
		intr_cancel_sentence();
		_G(flags)[V114] = 1;
		_val12 = 2000;
		_val13 = 2240;

		_G(kernel).trigger_mode = KT_DAEMON;
		_G(flags)[V111]++;
		kernel_timing_trigger(1, 110);
	} else if (_G(flags)[V116] || !player_said("WOLF") || (
		(!player_said("US DOLLARS") || !inv_player_has("US DOLLARS")) &&
		(!player_said("CHINESE YUAN") || !inv_player_has("CHINESE YUAN")) &&
		(!player_said("PERUVIAN INTI") || !inv_player_has("PERUVIAN INTI")) &&
		(!player_said("SIKKIMESE RUPEE") || !inv_player_has("SIKKIMESE RUPEE"))
	)) {
		if (talkFlag && player_said("WOLF"))
			intr_cancel_sentence();

		player_set_commands_allowed(false);
		_val12 = 2000;
		_val13 = 2250;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 110);
	} else {
		player_set_commands_allowed(false);
		_G(flags)[V116] = 1;
		intr_cancel_sentence();
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 230);
	}

	_G(kernel).trigger_mode = KT_PREPARSE;
}

void Room402::parser() {

}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
