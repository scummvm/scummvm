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

#include "m4/riddle/rooms/section4/room456.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room456::preload() {
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
	_G(player).walker_in_this_scene = false;
}

void Room456::init() {
	_G(player).disable_hyperwalk = true;
	_trigger = 577;
	_G(player).walker_in_this_scene = false;

	_seriesLighterChain = series_load("456 Lighter chain");
	_seriesGrateOpens = series_load("45 GRATE OPENS");
	_seriesCigarBoxTop = series_load("45 CIGAR BOX TOP");

	_lighter = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
		triggerMachineByHashCallback456, "Lighter");
	_vent = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
			triggerMachineByHashCallback456, "Vent");
	_lid = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
		triggerMachineByHashCallback456, "Cigar box lid");

	if (_G(flags)[V335]) {
		sendWSMessage_10000(1, _vent, _seriesGrateOpens, 15, 15, -1,
			_seriesGrateOpens, 15, 15, 0);
		hotspot_set_active("VENT", false);
	} else {
		sendWSMessage_10000(1, _vent, _seriesGrateOpens, 1, 1, -1,
			_seriesGrateOpens, 1, 1, 0);
		hotspot_set_active("VENT ", false);
		hotspot_set_active("HOLE", false);
		hotspot_set_active("HOLE ", false);
	}

	if (_G(flags)[V336]) {
		sendWSMessage_10000(1, _lid, _seriesCigarBoxTop, 14, 14, -1,
			_seriesCigarBoxTop, 14, 14, 0);
		hotspot_set_active("LID", false);
	} else {
		sendWSMessage_10000(1, _lid, _seriesCigarBoxTop, 1, 1, -1,
			_seriesCigarBoxTop, 1, 1, 0);
		hotspot_set_active("LID ", false);
		hotspot_set_active("CIGAR ", false);
	}

	sendWSMessage_10000(1, _lighter, _seriesLighterChain, 1, 1, -1,
		_seriesLighterChain, 1, 1, 0);

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(true);
		digi_preload("456_s03a");
		digi_preload("456_s03");
		digi_preload("456_s01");

		if (_G(flags)[V322]) {
			digi_play("456_s03a", 3);
			kernel_timing_trigger(_trigger, 700);
		}
	} else if (_G(flags)[V322]) {
		digi_play("456_s03a", 3);
		kernel_timing_trigger(_trigger, 700);
	} else {
		digi_preload("456_s03a");
		digi_preload("456_s03");
		digi_preload("456_s01");
	}
}

void Room456::daemon() {
	if (_G(kernel).trigger == 700 && _G(flags)[V322]) {
		digi_play_loop("456_s03a", 3);
	}
}

void Room456::pre_parser() {
	_G(player).resetWalk();
}

void Room456::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (lookFlag && player_said_any("RED BUTTON", "BLACK BUTTON", "FAN SWITCH")) {
		digi_play("456r09", 1);
	} else if (lookFlag && player_said_any("RED BUTTON ", "BLACK BUTTON ")) {
		digi_play("456r03", 1);
	} else if (lookFlag && player_said("ASHTRAY")) {
		digi_play("456r11", 1);
	} else if (lookFlag && player_said("SWITCH")) {
		digi_play("456r26", 1);
	} else if (lookFlag && player_said("HOLE ")) {
		digi_play("456r27", 1);
	} else if (lookFlag && player_said("HOLE")) {
		digi_play("456r28", 1);
	} else if (lookFlag && player_said("ACE OF SPADES")) {
		digi_play("456r26", 1);
	} else if (lookFlag && player_said("LIGHTER")) {
		digi_play("456r06", 1);
	} else if (lookFlag && player_said("CIGAR ")) {
		digi_play("com098", 1, 255, -1, 997);
	} else if (lookFlag && player_said_any("VENT", "VENT ")) {
		digi_play("456r10", 1);
	} else if (lookFlag && player_said_any("CIGAR BOX", "LID", "LID ")) {
		digi_play("456r04", 1);
	} else if (lookFlag && player_said("PICTURE")) {
		digi_play("456r16", 1);
	} else if (lookFlag && player_said("CHAIN")) {
		digi_play("456r07", 1);
	} else if (lookFlag && player_said(" ")) {
		digi_play("456r17", 1);
	} else if (useFlag && player_said("LID")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			sendWSMessage_10000(1, _lid, _seriesCigarBoxTop, 1, 14, 2,
				_seriesCigarBoxTop, 14, 14, 0);
			hotspot_set_active("LID", false);
			_G(flags)[V336] = 1;
			hotspot_set_active("LID ", true);
			digi_play("456_s05", 2);
			break;
		case 2:
			player_set_commands_allowed(false);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("LID ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			sendWSMessage_10000(1, _lid, _seriesCigarBoxTop, 14, 1, 2,
				_seriesCigarBoxTop, 1, 1, 0);
			hotspot_set_active("LID", true);
			_G(flags)[V336] = 0;
			hotspot_set_active("LID ", false);
			digi_play("456_s05", 2);
			break;
		case 2:
			player_set_commands_allowed(false);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("VENT")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			sendWSMessage_10000(1, _vent, _seriesGrateOpens, 1, 15, 2,
				_seriesGrateOpens, 15, 15, 0);
			hotspot_set_active("VENT", false);
			_G(flags)[V335] = 1;
			hotspot_set_active("VENT ", true);
			hotspot_set_active("HOLE", true);
			hotspot_set_active("HOLE ", true);

			digi_play("950_s36", 2, 255, -1, 950);
			break;
		case 2:
			player_set_commands_allowed(false);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("VENT ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			sendWSMessage_10000(1, _vent, _seriesGrateOpens, 15, 1, 2,
				_seriesGrateOpens, 1, 1, 0);
			hotspot_set_active("VENT", true);
			_G(flags)[V335] = 0;
			hotspot_set_active("VENT ", false);
			hotspot_set_active("HOLE", false);
			hotspot_set_active("HOLE ", false);

			digi_play("950_s36", 2, 255, -1, 950);
			break;
		case 2:
			player_set_commands_allowed(false);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("RED BUTTON")) {
		if (_G(flags)[V322]) {
			digi_play("456_s01", 2);
		} else {
			_G(flags)[V322] = 1;
			_G(kernel).trigger_mode = KT_DAEMON;
			digi_play("456_s03", 3, 255, 700);
			_G(kernel).trigger_mode = KT_PARSE;
		}
	} else if (useFlag && player_said("BLACK BUTTON")) {
		if (_G(flags)[V322]) {
			digi_stop(3);
			digi_play("456_s04", 3);
			_G(flags)[V322] = 0;
		} else {
			digi_play("456_s01", 3);
		}
	} else if (useFlag && player_said("CHAIN")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			sendWSMessage_10000(1, _lighter, _seriesLighterChain, 1, 4, 2,
				_seriesLighterChain, 4, 4, 0);
			digi_play("456_s02", 2);
			break;
		case 2:
			sendWSMessage_10000(1, _lighter, _seriesLighterChain, 4, 1, 3,
				_seriesLighterChain, 1, 1, 0);
			break;
		case 3:
			digi_play("456r08", 1, 255, 4);
			break;
		case 4:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said("CIGAR ")) {
		if (_G(flags)[V315]) {
			digi_play("456r05", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				kernel_examine_inventory_object("PING CIGAR", _G(master_palette),
					5, 1, 210, 210, 2, 0, -1);
				break;
			case 2:
				inv_give_to_player("CIGAR");
				_G(flags)[V315] = 1;
				player_set_commands_allowed(true);
				break;
			}
		}
	} else if (takeFlag && player_said("ASHTRAY")) {
		digi_play("456r12", 1);
	} else if (takeFlag && player_said("LIGHTER")) {
		digi_play("456r14", 1);
	} else if (takeFlag && player_said("CIGAR BOX")) {
		digi_play("r56r15", 1);
	} else if (useFlag && player_said("RED BUTTON ")) {
		digi_play("456_s01", 2);
	} else if (useFlag && player_said("BLACK BUTTON ")) {
		switch (_G(kernel).trigger) {
		case -1:
			digi_play("456_s01", 2);
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			_G(game).setRoom(406);

			if (_G(flags)[V322]) {
				adv_kill_digi_between_rooms(false);
				digi_play_loop("456_s03a", 3);
			}
			break;
		default:
			break;
		}
	} else if (player_said("BILLIARD BALL", "HOLE")) {
		digi_play("456r13", 1);
	} else if (player_said("BILLIARD BALL", "HOLE ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_G(flags)[V321] = 1;
			_G(flags)[V312] = 1;
			inv_move_object("BILLIARD BALL", 456);
			digi_play("456_s06", 1, 255, 1);
			break;
		case 1:
			digi_play("456_s07", 1, 255, 2);
			break;
		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room456::triggerMachineByHashCallback456(frac16 myMessage, machine *) {
	kernel_trigger_dispatchx(myMessage);
}


} // namespace Rooms
} // namespace Riddle
} // namespace M4
