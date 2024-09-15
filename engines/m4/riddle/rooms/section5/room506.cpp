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

#include "m4/riddle/rooms/section5/room506.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_vmng_screen.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

#define PALETTE_BLOCK 39

Room506::Room506() : Room() {
}

void Room506::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
}

void Room506::init() {
	_flag2 = _flag3 = _flag4 = false;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_flag1 = true;

	for (int i = 0; i < 39; ++i)
		_palette[i] = _G(master_palette)[i + 7];

	_G(kernel).call_daemon_every_loop = true;
	hotspot_set_active("OBSERVATORY DOOR", !_G(flags)[V156]);
	digi_preload("506_s01");
	digi_play_loop("506_s01", 3, 100);

	if (_G(flags)[V157] == 1)
		_dome = series_play("506dome2", 0x100, 16, -1, 5);

	if (inv_object_in_scene("CRYSTAL SKULL", 510)) {
		_skullSparkle = series_play("506 skull sparkle", 0x100, 16, -1, 5, 0, 100, 0, 0, 0, 0);
		kernel_timing_trigger(imath_ranged_rand(600, 3600), 505);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 504:
		_flag1 = false;
		MoveScreenDelta(-640, 0);
		player_set_commands_allowed(false);
		series_play("506 RIP UP STEPS", 0x700, 0, 503, 5);
		ws_demand_location(1054, 346, 12);
		ws_hide_walker();
		break;

	case 507:
		digi_unload("507_s02");
		ws_demand_location(346, 214, 4);
		ws_walk(420, 234, nullptr, 502, 4);
		player_set_commands_allowed(false);
		break;

	case 508:
		player_set_commands_allowed(false);

		if (_G(flags)[V158]) {
			ws_demand_location(100, 100);
			ws_hide_walker();
			digi_preload("506_s02");
			digi_unload("508_s01");
			_domeRotation = series_stream("506 DOME ROTATION", 20, 0x100, -1);
			series_stream_break_on_frame(_domeRotation, 45, 500);
			digi_play("506_S02", 1);
		} else {
			ws_demand_location(346, 214, 4);
			ws_walk(420, 234, nullptr, 502, 4);
		}
		break;

	default:
		MoveScreenDelta(-432, 0);
		ws_demand_location(754, 242, 8);
		ws_walk(697, 247, nullptr, 502, 8);
		break;
	}

	if (_flag1)
		restorePalette();
	else
		setupPalette();
}

void Room506::daemon() {
	switch (_G(kernel).trigger) {
	case 500:
		disable_player_commands_and_fade_init(501);
		break;

	case 501:
		adv_kill_digi_between_rooms(false);
		digi_play_loop("506_s01", 3, 100);
		_G(game).setRoom(508);
		break;

	case 502:
		player_set_commands_allowed(true);
		break;

	case 503:
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 505:
		terminateMachineAndNull(_skullSparkle);
		_skullSparkle = series_play("506 skull sparkle", 0x100, 16, 506, 5);
		break;

	case 506:
		terminateMachineAndNull(_skullSparkle);
		_skullSparkle = series_play("506 skull sparkle", 0x100, 16, -1, 5);
		kernel_timing_trigger(imath_ranged_rand(600, 3600), 505);
		break;

	default:
		break;
	}
}

void Room506::pre_parser() {
	switch (_G(kernel).trigger) {
	case 1000:
		_G(i_just_hyperwalked) = true;

		if (_flag4) {
			ws_walk(_destX, _destY, nullptr, 1002, _destFacing);
			Common::strcpy_s(_G(player).verb, "IN TOWER");
		} else {
			ws_walk(_destX, _destY, nullptr, -1, _destFacing);
		}

		restorePalette();
		break;

	case 1001:
		_G(i_just_hyperwalked) = true;

		if (_flag4) {
			ws_walk(_destX, _destY, nullptr, 1002, _destFacing);
			Common::strcpy_s(_G(player).verb, "IN TOWER");
		} else {
			ws_walk(_destX, _destY, nullptr, -1, _destFacing);
		}
		break;

	case 1002:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(1004);
		break;

	case 1004:
		_G(game).setRoom(509);
		break;

	default:
		break;
	}

	if (_G(player).need_to_walk || _G(player).ready_to_walk || _G(player).waiting_for_walk) {
		_flag4 = !scumm_strnicmp(_G(player).verb, "IN TOWER", 8);

		if (_G(player).walk_x > 900 && _flag2 && !player_said("   ") && !player_said("    ")) {
			saveWalk();
			ws_walk(717, 144, nullptr, 1000, 1);
		}

		if (_G(player).walk_x > 900 && _flag3 && !player_said("   ") && !player_said("    ")) {
			saveWalk();
			ws_walk(995, 308, nullptr, 1001, 1);
		}
	}
}

void Room506::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (takeFlag && player_said("CLUMP OF VINES")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			if (_G(flags)[V152] != 6 && _G(flags)[V153] != 6) {
				digi_play("504R47", 1, 255, -1, 504);
			} else {
				player_set_commands_allowed(false);
				_ripLowReach = series_load("RIP TREK LOW REACHER POS5");
				setGlobals1(_ripLowReach, 1, 26, 26, 26, 0, 26, 1, 1, 1);
				sendWSMessage_110000(2);				
			}
			break;

		case 2:
			kernel_examine_inventory_object(_G(flags)[V152] == 6 ? "PING GREEN VINE" :
				"PING BROWN VINE", 5, 1, 396, 180, 3);
			break;

		case 3:
			if (_G(flags)[V152] == 6) {
				inv_give_to_player("GREEN VINE");
				_G(flags)[V152] = 3;
			} else {
				inv_give_to_player("BROWN VINE");
				_G(flags)[V153] = 3;
			}

			sendWSMessage_120000(4);
			break;

		case 4:
			sendWSMessage_150000(5);
			break;

		case 5:
			series_unload(_ripLowReach);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("CLUMP OF VINES")) {
		digi_play("506R19", 1);
	} else if (lookFlag && player_said("NICHE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			digi_play(inv_object_in_scene("CRYSTAL SKULL", 510) ? "506R04" : "com127", 1, 255, 3);
			_ripley = series_play("RIPLEY LOOKS TO NICH", 0x700, 16, -1, 7);
			break;

		case 3:
			terminateMachineAndNull(_ripley);
			_ripley = series_play("RIPLEY LOOKS TO NICH", 0x700, 2, 5, 7);
			break;

		case 5:
			terminateMachineAndNull(_ripley);
			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("SKELETON")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			digi_play("506R03", 1, 255, 3);
			_ripley = series_play("RIPLEY LOOKS TO NICH", 0x700, 16, -1, 7);
			break;

		case 3:
			terminateMachineAndNull(_ripley);
			_ripley = series_play("RIPLEY LOOKS TO NICH", 0x700, 2, 5, 7);
			break;

		case 5:
			terminateMachineAndNull(_ripley);
			ws_unhide_walker();
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("TERRACE")) {
		digi_play("506R05", 1);
	} else if (lookFlag && player_said("OBSERVATORY")) {
		digi_play("506R06", 1);
	} else if (player_said("OBSERVATORY DOOR") && (!lookFlag && !takeFlag && !useFlag)) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);
			break;

		case 2:
			adv_kill_digi_between_rooms(false);
			digi_preload("506_s01");
			digi_play_loop("506_s01", 3, 100);
			_G(game).setRoom(507);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("OBSERVATORY DOME")) {
		digi_play("506R07", 1);
	} else if (lookFlag && player_said("SPIDER STATUE")) {
		digi_play("506R08", 1);
	} else if (lookFlag && player_said("TOWER")) {
		digi_play("506R02", 1);
	} else if (lookFlag && player_said_any(" ", "  ")) {
		digi_play(_G(player).been_here_before ? "506r01" : "506r01a", 1);
	} else if (player_said("STAIRS FROM LANDING")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			ws_hide_walker();
			series_play("506 RIP DOWN STEPS", 0x700, 0, -1, 5);
			kernel_timing_trigger(60, 3);
			break;

		case 3:
			disable_player_commands_and_fade_init(5);
			break;

		case 5:
			_G(game).setRoom(504);
			break;

		default:
			break;
		}
	} else if (player_said("IN TOWER")) {
		player_set_commands_allowed(false);

		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			disable_player_commands_and_fade_init(2);
			break;

		case 2:
			_G(game).setRoom(509);
			break;

		default:
			break;
		}
	} else if (player_said("IN OBSERVATORY")) {
		player_set_commands_allowed(false);

		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			disable_player_commands_and_fade_init(2);
			break;

		case 2:
			_G(game).setRoom(508);
			break;

		default:
			break;
		}
	} else {
		if (player_said("   ")) {
			restorePalette();
			ws_walk(662, 143, nullptr, -1, 8);

			if (_flag4)
				Common::strcpy_s(_G(player).verb, "IN TOWER");
		}

		if (player_said("    ")) {
			setupPalette();
			ws_walk(1039, 328, nullptr, -1, 4);

			if (_flag4)
				Common::strcpy_s(_G(player).verb, "IN TOWER");
		}

		return;
	}

	_G(player).command_ready = false;
}

void Room506::restorePalette() {
	if (!_flag3) {
		_flag3 = true;
		_flag2 = false;
		hotspot_set_active("   ", false);
		hotspot_set_active("    ", true);

		Common::copy(_palette, _palette + PALETTE_BLOCK, &_G(master_palette)[21]);
		gr_pal_set_range(21, PALETTE_BLOCK);
		_flag1 = true;
	}
}

void Room506::setupPalette() {
	if (!_flag2) {
		_flag2 = true;
		_flag3 = false;
		hotspot_set_active("   ", true);
		hotspot_set_active("    ", false);

		for (int i = 0; i < PALETTE_BLOCK; ++i) {
			int palR = MAX(_G(master_palette)[7 + i].r - 100, 0);
			int palG = MAX(_G(master_palette)[7 + i].g - 100, 0);
			int palB = MAX(_G(master_palette)[7 + i].b - 100, 0);

			_G(master_palette)[7 + i].r = palR;
			_G(master_palette)[7 + i].g = palG;
			_G(master_palette)[7 + i].b = palB;
		}

		gr_pal_set_range(21, PALETTE_BLOCK);
		_flag1 = false;
	}
}

void Room506::saveWalk() {
	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = false;
	_G(player).waiting_for_walk = false;
	_destX = _G(player).walk_x;
	_destY = _G(player).walk_y;
	_destFacing = _G(player).walk_facing;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
