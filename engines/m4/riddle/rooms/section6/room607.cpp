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

#include "m4/riddle/rooms/section6/room607.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room607::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
}

void Room607::init() {
	_ripKicksRock = series_load("RIPLEY KICKS ROCK");
	_ripMakesMud = series_load("RIPLEY MAKES MUD");
	_ripLowReach = series_load("RIP LOW REACH POS1");
	digi_preload("950_s37a");
	digi_preload("607_s02");
	digi_preload("607_s01");

	if (inv_object_is_here("RONGORONGO TABLET"))
		_tablet = series_show("607tab", 0x800, 16);

	if (_G(flags)[V193]) {
		_mud = series_show("CLAY MUD", 0x700, 16);
		_rock = series_show("ROCK BOTTOM", 0x700, 16);
		hotspot_set_active("GREY ROCK", false);

		if (!_G(flags)[V194] || !inv_object_is_here("RONGORONGO TABLET"))
			hotspot_set_active("RONGORONGO TABLET", true);

	} else {
		_rock = series_show("ROCK TOP", 0x700, 16);

		if (_G(flags)[V198] > 0)
			_mud = series_show("CLAY MUD", 0x700, 16);

		hotspot_set_active("GREY ROCK ", false);
		hotspot_set_active("RONGORONGO TABLET", false);
		hotspot_set_active("FISSURE", false);
	}

	switch (_G(kernel).trigger) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s28C");
		break;

	case 633:
		ws_demand_location(316, 358, 1);
		break;

	case 638:
		ws_demand_location(-30, 334, 3);
		ws_walk(32, 334, nullptr, 1, 3);
		player_set_commands_allowed(false);
		break;

	case 640:
		ws_demand_location(670, 288, 9);
		ws_walk(604, 288, nullptr, 1, 9);
		player_set_commands_allowed(false);
		break;

	case 645:
		ws_demand_location(124, 308, 5);
		break;

	default:
		digi_preload("950_s28C");
		ws_demand_location(316, 358, 1);
		break;
	}

	digi_play_loop("950_s28C", 3, 90);
}

void Room607::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;
	default:
		break;
	}
}

void Room607::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (useFlag && player_said("GREY ROCK") && !_G(flags)[V193])
		_G(player).resetWalk();
	if (player_said("HORN/PULL CORD/WATER", "CLAY") && !_G(flags)[V193])
		_G(player).resetWalk();
	if ((lookFlag || useFlag || takeFlag) && player_said(" "))
		_G(player).resetWalk();
}

void Room607::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (lookFlag && player_said("FISSURE") && inv_object_is_here("RONGORONGO TABLET")) {
		hotspot_set_active("RONGORONGO TABLET", true);
		_G(flags)[V194] = 1;
		digi_play("607r05", 1);
	} else if (lookFlag && player_said("FISSURE") && !inv_object_is_here("RONGORONGO TABLET")) {
		hotspot_set_active("RONGORONGO TABLET", true);
		_G(flags)[V194] = 1;
		digi_play("607r05a", 1);
	} else if (lookFlag && player_said("CLAY")) {
		switch (_G(flags)[V198]) {
		case 0:
			digi_play("607r04", 1);
			break;
		case 1:
			digi_play("607r11", 1);
			break;
		case 2:
			digi_play("607r12", 1);
			break;
		case 3:
			digi_play("607r13", 1);
			break;
		default:
			digi_play("607r14", 1);
			break;
		}
	} else if (lookFlag && player_said(" ")) {
		digi_play("607r01", 1);
	} else if (lookFlag && player_said("RED BOULDER")) {
		digi_play("607r02", 1);
	} else if (lookFlag && player_said_any("GREY ROCK", "GREY ROCK ")) {
		digi_play("607r03", 1);
	} else if (lookFlag && player_said("RONGORONGO TABLET") &&
			inv_object_is_here("RONGORONGO TABLET")) {\
		_val1 = 1;
		digi_play("607r09", 1);
	} else if (takeFlag && player_said("CLAY")) {
		digi_play(_G(flags)[V198] ? "607r15" : "607r08", 1);
	} else if (takeFlag && player_said("RED BOULDER")) {
		digi_play("607r06", 1);
	} else if (takeFlag && player_said_any("GREY ROCK", "GREY ROCK ")) {
		digi_play("607r07", 1);
	} else if (takeFlag && player_said("RONGORONGO TABLET") &&
			inv_object_is_here("RONGORONGO TABLET")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachineAndNull(_rock);
			_rock = series_show("ROCK BOTTOM", 0x400, 16);
			ws_walk(360, 315, nullptr, 1, 1);
			break;
		case 1:
			ws_hide_walker();
			player_update_info();
			series_play("RIP LOW REACH POS1", 0x700, 0, 3, 5, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 15);
			break;
		case 3:
			_ripReach = series_show("RIP LOW REACH POS1", 0x700, 16, -1, -1, 15,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			hotspot_set_active("RONGORONGO TABLET", false);
			terminateMachineAndNull(_tablet);
			kernel_examine_inventory_object("ping rongorongo tablet", 5, 1, 212, 150, 5,
				_val1 ? "607r09" : nullptr);
			break;
		case 5:
			terminateMachineAndNull(_ripReach);
			series_play("RIP LOW REACH POS1", 0x700, 2, 7, 5, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 15);
			break;
		case 7:
			ws_unhide_walker();
			ws_walk(383, 319, nullptr, 9, 0);
			break;
		case 9:
			terminateMachineAndNull(_rock);
			_rock = series_show("ROCK BOTTOM", 0x700, 16);
			inv_give_to_player("RONGORONGO TABLET");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("GREY ROCK") && !_G(flags)[V193]) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(431, 311, nullptr, 2, 9);
			break;
		case 2:
			player_set_commands_allowed(false);
			ws_hide_walker();
			terminateMachineAndNull(_rock);

			if (_G(flags)[V198] > 3) {
				series_play("RIPLEY KICKS ROCK", 0x200, 0, 6, 5, 0, 100, 0, 0, 0, 20);
			} else {
				if (!_G(flags)[V198])
					_mud = series_show("607 NOMUD", 0x100, 16);
				series_play("RIPLEY KICKS ROCK", 0x200, 0, 3, 5, 0, 100, 0, 0, 0, 20);
			}
			break;
		case 3:
			series_play("RIPLEY KICKS ROCK", 0x200, 0, 4, 5, 0, 100, 0, 0, 21, 24);
			digi_play("950_s37", 1);
			break;
		case 4:
			series_play("RIPLEY KICKS ROCK", 0x200, 2, 5, 5, 0, 100, 0, 0, 0, 24);
			break;
		case 5:
			digi_unload("950_s37");
			_rock = series_show("ROCK TOP", 0x700, 16);

			if (!_G(flags)[V198])
				terminateMachineAndNull(_mud);

			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;
		case 6:
			series_play("RIPLEY KICKS ROCK", 0x200, 0, 8, 5, 0, 100, 0, 0, 21, 26);
			digi_play("950_s37a", 1);
			break;
		case 7:
			_rock = series_show("ROCK BOTTOM", 0x700, 16);
			digi_stop(1);
			digi_stop(2);
			digi_unload("950_s37a");
			digi_unload("607_s02");
			hotspot_set_active("GREY ROCK", false);
			hotspot_set_active("GREY ROCK ", true);
			hotspot_set_active("FISSURE", true);
			_G(flags)[V193] = 1;
			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;
		case 8:
			series_play("RIPLEY KICKS ROCK", 0x200, 0, 7, 5, 0, 100, 0, 0, 27, 42);
			digi_play("607_s02", 2);
			break;

			break;
		default:
			break;
		}
	} else if (player_said("HORN/PULL CORD/WATER", "CLAY") && !_G(flags)[V193]) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(311, 349, nullptr, 1, 1);
			break;
		case 1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			player_update_info();
			_ripReach = series_show("SAFARI SHADOW 1", 0x700, 16, -1, -1, 0,
				_G(player_info).scale + 1, _G(player_info).x, _G(player_info).y);
			series_play("RIPLEY MAKES MUD", 0x100, 0, 2, 5, 0, 100, 0, 0, 0, 12);
			break;
		case 2:
			series_play("RIPLEY MAKES MUD", 0x100, 0, 7, 5, 0, 100, 0, 0, 13, 42);
			digi_play("607_s01", 2);
			break;
		case 7:
			_G(flags)[V198] = 4;
			_mud = series_show("CLAY MUD", 0x700, 16);

			switch (_G(flags)[V198]) {
			case 1:
				digi_play("607r11", 1);
				break;
			case 2:
				digi_play("607r12", 1);
				break;
			case 3:
				digi_play("607r13", 1);
				break;
			default:
				inv_move_object("HORN/PULL CORD/WATER", 997);
				inv_give_to_player("HORN/PULL CORD");
				digi_play("607r14", 1);
				break;
			}

			digi_stop(2);
			digi_unload("607_s01");
			terminateMachineAndNull(_ripReach);
			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("RED BOULDER")) {
		digi_play("607r10", 1);
	} else if (player_said("left") ) {
		// No implementation
	} else if (player_said("right") && right()) {
		// No implementation
	} else if (player_said("down") && down()) {
		// No implementation
	} else {
		return;
	}

	_G(player).command_ready = false;
}

bool Room607::left() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(1);
		return true;
	case 1:
		digi_stop(1);
		digi_stop(2);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28C", 3, 90);
		_G(game).setRoom(638);
		return true;
	default:
		break;
	}

	return false;
}

bool Room607::right() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(1);
		return true;
	case 1:
		digi_stop(1);
		digi_stop(2);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28C", 3, 90);
		_G(game).setRoom(640);
		return true;
	default:
		break;
	}

	return false;
}

bool Room607::down() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(1);
		return true;
	case 1:
		digi_stop(1);
		digi_stop(2);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28C", 3, 90);
		_G(game).setRoom(633);
		return true;
	default:
		break;
	}

	return false;
}
} // namespace Rooms
} // namespace Riddle
} // namespace M4
