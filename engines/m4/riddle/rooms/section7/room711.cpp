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

#include "m4/riddle/rooms/section7/room711.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room711::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room711::init() {
	digi_preload("711R03", -1);
	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MASTER LU'S BOOK", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "SIKKIMESE LU'S BOOK", false);
	}

	if (_G(game).previous_room == 710) {
		_711Rp01Series = series_load("711RP01", -1, nullptr);
		ws_demand_location(_G(my_walker), 245, 354);
		ws_demand_facing(_G(my_walker), 9);
		ws_hide_walker(_G(my_walker));
		kernel_timing_trigger(10, 100, nullptr);
	}
}

void Room711::pre_parser() {
	// No Implementation
}

void Room711::parser() {
	const bool lookFl = player_said_any("look", "look at");
	const bool talkFl = player_said_any("talk", "talk to", "take");
	const bool gearFl = player_said_any("push", "pull", "gear", "open", "close");

	if (lookFl && player_said("Window")) {
		digi_play("711R04", 1, 255, -1, -1);
	} else if (lookFl && player_said("Ladder")) {
		digi_play("711R05", 1, 255, -1, -1);
	} else if (lookFl && player_said("Sikkimese Book")) {
		digi_play("711R02", 1, 255, -1, -1);
	} else if (lookFl && player_said("Gold Icon")) {
		digi_play("711R18", 1, 255, -1, -1);
	} else if ((lookFl || player_said("JOURNAL")) && player_said("Master Lu's Book")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(flags[V286]))
				digi_play("203r54", 1, 255, -1, -1);
			else {
				player_set_commands_allowed(false);
				digi_play("711R03", 1, 255, 2, -1);
				digi_preload("950_s34", -1);
			}

			break;

		case 2:
			_ripSketchingSeries = series_load("RIP SKETCHING IN NOTEBOOK POS 2", -1, nullptr);
			setGlobals1(_ripSketchingSeries, 1, 17, 17, 17, 0, 18, 39, 39, 39, 39, 38, 1, 1, 1, 1, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 4);

			break;

		case 4:
			sendWSMessage_120000(_G(my_walker), 5);
			digi_play("950_S34", 2, 200, 5, 950);

			break;

		case 5:
			sendWSMessage_130000(_G(my_walker), 6);
			break;

		case 6:
			sendWSMessage_150000(_G(my_walker), 8);
			digi_unload("950_s34");

			break;

		case 8:
			_G(flags[V286]) = 1;
			player_set_commands_allowed(true);

			break;

		default:
			break;

		}
	} // (ecx || player_said("JOURNAL")) && player_said("Master Lu's Book")

	else if (lookFl && player_said(" ")) {
		digi_play("711R01", 1, 255, -1, -1);
	} else if (talkFl && player_said("Sikkimese Book")) {
		digi_play("711R07", 1, 255, -1, -1);
	} else if (talkFl && player_said("Master Lu's Book")) {
		digi_play("711R06", 1, 255, -1, -1);
	} else if (talkFl && player_said("Ladder")) {
		digi_play("711R08", 1, 255, -1, -1);
	} else if (talkFl && player_said("Gold Icon")) {
		digi_play("711R19", 1, 255, -1, -1);
	} else if (gearFl && player_said("Ladder")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);

			break;

		case 2:
			player_set_commands_allowed(true);
			_G(game).new_room = _G(flags[V286]) ? 706 : 710;

			break;

		default:
			break;
		}
	} else
		return;

	_G(player).command_ready = false;
}

void Room711::daemon() {
	switch (_G(kernel).trigger) {
	case 100:
		series_play("711RP01", 0, 0, 101, 0, 0, 100, 0, 0, 0, -1);
		break;

	case 101:
		series_unload(_711Rp01Series);
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));

		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
