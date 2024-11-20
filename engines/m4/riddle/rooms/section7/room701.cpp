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

#include "m4/riddle/rooms/section7/room701.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room701::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room701::init() {
	if (_G(flags[V286]))
		_G(flags[V224]) = 1;

	_itemDigiName = nullptr;
	_field8C = 0;
	_field50 = 0;
	_field9E = -1;
	_field130 = 0;

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		_agentTalkLoopTjSeries = series_load("AGENT TALK LOOP TJ", -1, nullptr);
		_agentGetTelegramSeries = series_load("AGENT  GET TELEGRAM", -1, nullptr);
		_agentShowMapSeries = series_load("AGENT SHOW MAP", -1, nullptr);
		_agentGiveParcelSeries = series_load("AGENT  GIVE PARCEL", -1, nullptr);
		_701rp01Series = series_load("701RP01", -1, nullptr);
		_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
		_ripTrekTalkerPos3Series = series_load("RIP TREK TALKER POS3", -1, nullptr);
		_ripTrekMedReachPos3Series = series_load("RIP TREK MED REACH POS3", -1, nullptr);
		_701rp99Series = series_load("701RP99", -1, nullptr);
		_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, -1, _agentTalkLoopTjSeries, 13, 13, 0);
	} else {
		_field88 = 0;

		player_set_commands_allowed(false);
		_agentTalkLoopTjSeries = series_load("AGENT TALK LOOP TJ", -1, nullptr);
		_agentGetTelegramSeries = series_load("AGENT  GET TELEGRAM", -1, nullptr);
		_agentShowMapSeries = series_load("AGENT SHOW MAP", -1, nullptr);
		_agentGiveParcelSeries = series_load("AGENT  GIVE PARCEL", -1, nullptr);
		_701rp01Series = series_load("701RP01", -1, nullptr);
		_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
		_ripTrekTalkerPos3Series = series_load("RIP TREK TALKER POS3", -1, nullptr);
		_ripTrekMedReachPos3Series = series_load("RIP TREK MED REACH POS3", -1, nullptr);
		_701rp99Series = series_load("701RP99", -1, nullptr);
		_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, -1, _agentTalkLoopTjSeries, 13, 13, 0);

		++_G(flags[V006]);
		if (setItemsPlacedFlags()) {
			_field88 = 1;
			_itemDigiName = getItemsPlacedDigi();
			++_field50;
		}

		ws_demand_location(_G(my_walker), 50, 264);
		ws_demand_facing(_G(my_walker), 3);
		_field134 = 0;
		ws_walk(_G(my_walker), 352, 251, nullptr, player_been_here(701) ? 80 : 40, 3, true);
	}

	digi_preload("701_s01", -1);
	digi_play_loop("701_s01", 2, 45, -1, -1);
}

void Room701::pre_parser() {
	if (player_said("rm702") && !_field88) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room701::parser() {
	bool ecx = player_said_any("look", "look at");
	bool talkFl = player_said_any("talk", "talk to");
	bool esi = player_said("take");
	bool edi = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv701a")) {
		if (_G(kernel).trigger == 1) {
			int32 who = conv_whos_talking();
			if (who == 1)
				_field98 = 1103;
			else if (who <= 0)
				_field6C = 2102;

			conv_resume(conv_get_handle());
		} else {
			subparser_9B2FC();
		}
	} // if (player_said("conv701a"))

	else if (talkFl && player_said("agent")) {
		player_set_commands_allowed(false);
		_field88 = 1;
		_field72 = -1;
		_field94 = 1000;
		_field98 = 1100;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 102, nullptr);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (ecx && player_said("COAT RACK")) {
		digi_play("com130", 1, 255, -1, -1);
	} else if (ecx && player_said("Skin") && !_G(flags[V226])) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 4, 1, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), -1);
			digi_play("701R10", 1, 255, 21, -1);

			break;

		case 21:
			sendWSMessage_120000(_G(my_walker), -1);
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 14, -1, _agentTalkLoopTjSeries, 11, 13, 1);

			break;

		case 22:
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);
			sendWSMessage_110000(_G(my_walker), -1);
			digi_play("701R11", 1, 255, 23, -1);

			break;

		case 23:
			sendWSMessage_120000(_G(my_walker), -1);
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 0);
			sendWSMessage_140000(_agentPoshExpressMach, 11);
			digi_play("701X10", 1, 255, 24, -1);

			break;

		case 24:
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);
			digi_play("701R12", 1, 255, 25, -1);

			break;

		case 25:
			sendWSMessage_150000(_G(my_walker), -1);
			_G(flags[V226]) = 1;
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("Skin") && !_G(flags[V226])

	else if (ecx && player_said("Skin") && _G(flags[V226])) {
		player_set_commands_allowed(false);
		digi_play("701R13", 1, 255, -1, -1);
		player_set_commands_allowed(true);

	} // ecx && player_said("Skin") && _G(flags[V226])

	else if (ecx && player_said("Window"))
		digi_play("701R17", 1, 255, -1, -1);
	else if (ecx && player_said("Poster"))
		digi_play("com002", 1, 255, -1, 997);
	else if (ecx && player_said("Magazines"))
		digi_play("com003", 1, 255, -1, 997);
	else if (ecx && player_said("Prayer Wheel"))
		digi_play("701R19", 1, 255, -1, -1);
	else if (ecx && player_said("Agent"))
		digi_play("701R20", 1, 255, -1, -1);
	else if (ecx && player_said_any("Sofa", "Chair"))
		digi_play("com023", 1, 255, -1, 997);
	else if (ecx && player_said("Telephone"))
		digi_play("com024", 1, 255, -1, 997);
	else if (ecx && player_said("Postcard Rack"))
		digi_play("701R14", 1, 255, -1, -1);
	else if (ecx && player_said("Brochures"))
		digi_play("701R15", 1, 255, -1, -1);
	else if (ecx && player_said("BUSH"))
		digi_play("701R41", 1, 255, -1, -1);
	else if (ecx && player_said(" "))
		digi_play("com001", 1, 255, -1, -1);
	else if (esi && player_said("Brochures")) {
		if (inv_player_has("PRAYER WHEEL BROCHURE"))
			digi_play("701R21", 1, 255, -1, -1);
		else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				setGlobals1(_ripTrekMedReachPos3Series, 1, 17, 17, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 62);

				break;

			case 62:
				digi_play("950_S07", 2, 255, -1, 950);
				kernel_examine_inventory_object("PING PRAYER WHEEL BROCHURE", _G(master_palette), 5, 1, 318, 150, 63, nullptr, -1);

				break;

			case 63:
				sendWSMessage_140000(_G(my_walker), 64);
				break;

			case 64:
				inv_give_to_player("PRAYER WHEEL BROCHURE");
				player_set_commands_allowed(true);

				break;

			default:
				break;
			}
		}
	} // esi && player_said("Brochures")

	else if (esi && player_said("Postcard Rack"))
		digi_play("com004", 1, 255, -1, 997);
	else if (esi && player_said("Magazines"))
		digi_play("com005", 1, 255, -1, 997);
	else if (esi && player_said("Yeti Skin"))
		digi_play("701R24", 1, 255, -1, -1);
	else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_701rp99Series, 1, 12, 12, 12, 0, 12, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 301);

			break;

		case 301:
			digi_play("701R25", 1, 255, 302, -1);
			break;

		case 302:
			sendWSMessage_120000(_G(my_walker), 303);
			break;

		case 303:
			sendWSMessage_150000(_G(my_walker), 304);
			break;

		case 304:
			player_set_commands_allowed(true);
			break;

		default:
			break;

		}
	} // player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE")

	else if (edi && player_said_any("Sofa", "Chair"))
		digi_play("com025", 1, 255, -1, 997);
	else if (edi && player_said("Telephone"))
		digi_play("com026", 1, 255, -1, 997);
	else if ((player_said("SIKKIMESE RUPEE", "Agent") && inv_player_has("SIKKIMESE RUPEE"))
		||   player_said("US DOLLARS", "Agent") // 2 checks in the original: one checks if the payer has dollars, the other doesn't.
		||   (player_said("CHINESE YUAN", "Agent") && inv_player_has("CHINESE YUAN"))
		||   (player_said("PERUVIAN INTI", "Agent") && inv_player_has("PERUVIAN INTI"))
		||   (player_said("SIKKIMESE RUPEE", "Agent") && inv_player_has("SIKKIMESE RUPEE"))) {
		digi_play("com012", 1, 255, -1, 997);
	} else if (player_said("journal", "skin")) {
		digi_play(_G(flags[V226]) ? "701R35" : "701R36", 1, 255, -1, -1);
	} else if (player_said("rm702")) {
		if (_field88) {
			switch (_G(kernel).trigger) {
			case -1:
				disable_player_commands_and_fade_init(3);
				break;

			case 3:
				digi_stop(3);
				_G(game).new_room = 702;
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				ws_walk(_G(my_walker), 214, 265, nullptr, 2, 3, true);

				break;

			case 2:
				setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), -1);
				digi_play("22_12p01", 1, 255, 3, -1);

				break;

			case 3:
				sendWSMessage_120000(_G(my_walker), -1);
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
				sendWSMessage_1a0000(_agentPoshExpressMach, 11);
				digi_play("22_12n01", 1, 255, 4, -1);

				break;

			case 4:
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);
				sendWSMessage_150000(_G(my_walker), -1);
				ws_walk(_G(my_walker), 73, 265, nullptr, 5, 3, true);

				break;

			case 5:
				player_set_commands_allowed(true);
				disable_player_commands_and_fade_init(6);

				break;

			case 6:
				digi_stop(3);
				_G(game).new_room = 702;
				break;

			default:
				break;
			}
		}
	} // player_said("rm702")

	_G(player).command_ready = false;
}

void Room701::daemon() {
	warning("STUB - 701 Daemon");
}

void Room701::subparser_9B2FC() {
	warning("STUB - 701 subparser_9B2FC");
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
