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

#include "m4/riddle/rooms/section5/room508.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room508::init() {
	hotspot_set_active("CRYSTAL SKULL ", false);
	hotspot_set_active("SHOVEL", false);
	hotspot_set_active("SHOVEL ", false);

	if (inv_object_is_here("SHOVEL"))
		kernel_load_variant("508lock1");

	if (_G(flags)[V158] == 1) {
		player_set_commands_allowed(false);
		ws_hide_walker();
		_ripReturnsToStander = series_load("RIP RETURNS TO STANDER");
		_chainBreaking = series_load("508 CHAIN BREAKING");
		_lightAppearing = series_load("LIGHT APPEARING ON FLOOR");

		digi_preload("508_s02");
		digi_preload("508_s04");
		digi_preload("508_s08");

		series_stream("RIP STOPS DOME TURNING", 7, 0x100, 525);
		digi_play("508_S02", 1);
		digi_play("508_S04", 2);

	} else if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		ws_demand_location(246, 265, 5);
		ws_walk(256, 283, nullptr, 562, 5);
	}

	if (_G(flags)[V157] == 1) {
		hotspot_set_active("CHAIN ", false);
		_chainAfterBreak = series_place_sprite("508 CHAIN AFTER BREAK", 0, 0, 0, 100, 0xf00);
		_domeAfterTurn = series_place_sprite("DOME SPRITE AFTER ITS TURNED", 0, 0, 0, 100, 0xf00);

		if (inv_object_is_here("CRYSTAL SKULL")) {
			hotspot_set_active("CRYSTAL SKULL ", true);
			_skull = series_place_sprite("SKULL SPRITE AFTER DOME TURN", 0, 0, 0, 100, 0x450);
		}

		if (inv_object_is_here("SHOVEL")) {
			hotspot_set_active("SHOVEL ", true);
			_shovel = series_place_sprite("SHOVEL AFTER DOMES TURNED", 0, 0, 0, 100, 0x300);
		}
	}

	if (!_G(flags)[V157]) {
		if (_G(flags)[V158]) {
			hotspot_set_active("CRYSTAL SKULL ", true);
			_skull = series_place_sprite("SKULL SPRITE BEFORE DOME TURN", 0, 0, 0, 100, 0x450);
		}

		if (inv_object_is_here("SHOVEL")) {
			hotspot_set_active("SHOVEL", true);
			_shovel = series_place_sprite("SHOVEL SPRITE", 0, 0, 0, 100, 0x300);
		}
	}

	digi_preload("508_s01");
	digi_play_loop("508_s01", 3, 120);

	if (_G(flags)[V158] == 1) {
		digi_unload("506_s01");
		digi_unload("506_s02");
	}

	if (_G(game).previous_room == 507)
		digi_unload("507_s02");
}

void Room508::daemon() {

}

void Room508::pre_parser() {
	if (player_said("SHOVEL") && player_said("HOLE IN CAPSTAN") &&
		!_G(flags)[V157] && !inv_object_is_here("SHOVEL"))
		_G(player).resetWalk();
}

void Room508::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (lookFlag && player_said(" ")) {
		if (_G(flags)[V162] == 1) {
			digi_play("508R01", 1);
		} else {
			digi_play("508R01A", 1);
			_G(flags)[V162] = 1;
		}
	} else if (lookFlag && player_said("DOMED CEILING")) {
		digi_play("508R02", 1);
	} else if (lookFlag && player_said("SLIT IN DOMED CEILING")) {
		digi_play("508R03", 1);
	} else if (lookFlag && player_said("SYMBOL ON WALL")) {
		digi_play("508R04", 1);
	} else if (lookFlag && player_said("SKELETON")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			ws_walk(317, 360, nullptr, 2, 1);
			break;

		case 2:
			if (inv_player_has("CRYSTAL SKULL")) {
				digi_play("508R05A", 1);
			} else if (inv_object_is_here("CRYSTALL SKULL")) {
				digi_play("508R05B", 1);
			} else {
				digi_play("508R05", 1);
			}

			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said_any("CHAIN", "CHAIN ")) {
		digi_play(_G(flags)[V157] ? "508R15" : "508R06", 1);
	} else if (lookFlag && player_said("FLOOR")) {
		digi_play("508R07", 1);
	} else if (lookFlag && player_said("CAPSTAN")) {
		digi_play("508R08", 1);
	} else if (lookFlag && player_said("HOLE IN CAPSTAN")) {
		digi_play("508R09", 1);
	} else if (lookFlag && player_said("ORNAMENTATION ON WALL")) {
		digi_play("508R10", 1);
	} else if (player_said("Walk Through")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			ws_walk(237, 255, nullptr, -1, 11);
			pal_fade_init(21, 255, 0, 30, 2);
			break;

		case 2:
			_G(game).setRoom(506);
			break;

		default:
			break;
		}
	} else if (player_said("SHOVEL") && player_said("HOLE IN CAPSTAN")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(423, 356, nullptr, 2, 1);
			break;

		case 2:
			if (!_G(flags)[V157] && !inv_object_is_here("SHOVEL")) {
				player_set_commands_allowed(false);
				kernel_load_variant("508lock1");
				_G(kernel).trigger_mode = KT_DAEMON;
				ws_walk(423, 356, nullptr, 2, 1);
			}
			break;

		default:
			break;
		}
	} else if (takeFlag && player_said("SHOVEL") &&
			inv_object_is_here("SHOVEL") && _G(kernel).trigger == -1) {
		if (_G(flags)[V157] == 0 && inv_object_is_here("SHOVEL")) {
			player_set_commands_allowed(false);
			kernel_load_variant("508lock0");
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 508);
		}

		if (_G(flags)[V157] == 1) {
			player_set_commands_allowed(false);
			_val1 = 1;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 511);
		}
	} else if (lookFlag && player_said_any("crystal skull", "crystal skull ") &&
		inv_object_is_here("CRYSTAL SKULL")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			ws_walk(317, 360, nullptr, 2, 1);
			break;

		case 2:
			digi_play("510r11", 1);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said_any("shovel", "shovel ") &&
			inv_object_is_here("SHOVEL")) {
		digi_play("com106", 1);
	} else if (takeFlag && player_said("SHOVEL ") && _G(kernel).trigger == -1) {
		if (_G(flags)[V157] == 1) {
			player_set_commands_allowed(false);
			_val1 = 1;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 511);
		}
	} else if (useFlag && player_said("SHOVEL") && inv_object_is_here("SHOVEL")) {
		if (!_G(flags)[V157] && inv_object_is_here("SHOVEL") &&
				!inv_object_is_here("CRYSTAL SKULL") && _G(kernel).trigger == -1) {
			player_set_commands_allowed(false);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 511);
		}

		if (!_G(flags)[V157] && inv_object_is_here("SHOVEL") &&
				inv_object_is_here("CRYSTAL SKULL") && _G(kernel).trigger == -1) {
			player_set_commands_allowed(false);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 515);
		}
	} else if (player_said("CRYSTAL SKULL", "SKELETON")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			ws_walk(333, 290, nullptr, 2, 3);
			break;

		case 2:
			player_set_commands_allowed(false);
			hotspot_set_active("CRYSTAL SKULL ", true);
			digi_play("508_s05", 1);

			_skull = series_place_sprite("SKULL SPRITE AFTER DOME TURN", 0, 0, 0, 100, 0x450);
			inv_move_object("CRYSTAL SKULL", 508);
			ws_walk(317, 360, nullptr, 3, 1);
			break;

		case 3:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (takeFlag && player_said("CRYSTAL SKULL ")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			if (inv_object_is_here("CRYSTAL SKULL")) {
				ws_walk(333, 290, nullptr, 2, 3);
			}
			break;

		case 2:
			player_set_commands_allowed(false);
			hotspot_set_active("CRYSTAL SKULL ", false);
			kernel_examine_inventory_object("PING CRYSTAL SKULL", 5, 1, 250, 170, 3, "508_s05", 7);
			break;

		case 3:
			inv_give_to_player("CRYSTAL SKULL");
			ws_walk(317, 360, nullptr, 4, 1);
			break;

		case 4:
			player_set_commands_allowed(true);
			break;

		case 7:
			terminateMachineAndNull(_skull);
			break;

		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
