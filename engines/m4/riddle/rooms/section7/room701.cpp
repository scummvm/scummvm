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
	_field8C_unusedFl = false;
	_field50_counter = 0;
	_field9E_triggerNum = -1;
	_field130 = false;

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
		_field88 = false;

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
			_field88 = true;
			_itemDigiName = getItemsPlacedDigi();
			++_field50_counter;
		}

		ws_demand_location(_G(my_walker), 50, 264);
		ws_demand_facing(_G(my_walker), 3);
		_alreadyBeenHereFl = false;

		int32 trig = 80;
		if (!player_been_here(701)) {
			trig = 40;
			_alreadyBeenHereFl = true;
		}

		ws_walk(_G(my_walker), 352, 251, nullptr, trig, 3, true);
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
	bool lookFl = player_said_any("look", "look at");
	bool talkFl = player_said_any("talk", "talk to");
	bool takeFl = player_said("take");
	bool gearFl = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv701a")) {
		if (_G(kernel).trigger == 1) {
			int32 who = conv_whos_talking();
			if (who == 1)
				_field98_should = 1103;
			else if (who <= 0)
				_field6C_should = 2102;

			conv_resume(conv_get_handle());
		} else {
			conv701a();
		}
	} // if (player_said("conv701a"))

	else if (talkFl && player_said("agent")) {
		player_set_commands_allowed(false);
		_field88 = true;
		_field72_triggerNum = -1;
		_field94_mode = 1000;
		_field98_should = 1100;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 102, nullptr);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (lookFl && player_said("COAT RACK")) {
		digi_play("com130", 1, 255, -1, -1);
	} else if (lookFl && player_said("Skin") && !_G(flags[V226])) {
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

	else if (lookFl && player_said("Skin") && _G(flags[V226])) {
		player_set_commands_allowed(false);
		digi_play("701R13", 1, 255, -1, -1);
		player_set_commands_allowed(true);

	} // ecx && player_said("Skin") && _G(flags[V226])

	else if (lookFl && player_said("Window"))
		digi_play("701R17", 1, 255, -1, -1);
	else if (lookFl && player_said("Poster"))
		digi_play("com002", 1, 255, -1, 997);
	else if (lookFl && player_said("Magazines"))
		digi_play("com003", 1, 255, -1, 997);
	else if (lookFl && player_said("Prayer Wheel"))
		digi_play("701R19", 1, 255, -1, -1);
	else if (lookFl && player_said("Agent"))
		digi_play("701R20", 1, 255, -1, -1);
	else if (lookFl && player_said_any("Sofa", "Chair"))
		digi_play("com023", 1, 255, -1, 997);
	else if (lookFl && player_said("Telephone"))
		digi_play("com024", 1, 255, -1, 997);
	else if (lookFl && player_said("Postcard Rack"))
		digi_play("701R14", 1, 255, -1, -1);
	else if (lookFl && player_said("Brochures"))
		digi_play("701R15", 1, 255, -1, -1);
	else if (lookFl && player_said("BUSH"))
		digi_play("701R41", 1, 255, -1, -1);
	else if (lookFl && player_said(" "))
		digi_play("com001", 1, 255, -1, -1);
	else if (takeFl && player_said("Brochures")) {
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

	else if (takeFl && player_said("Postcard Rack"))
		digi_play("com004", 1, 255, -1, 997);
	else if (takeFl && player_said("Magazines"))
		digi_play("com005", 1, 255, -1, 997);
	else if (takeFl && player_said("Yeti Skin"))
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

	else if (gearFl && player_said_any("Sofa", "Chair"))
		digi_play("com025", 1, 255, -1, 997);
	else if (gearFl && player_said("Telephone"))
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
	switch (_G(kernel).trigger) {
	case 19:
		digi_play("701_s01", 3, 127, 19, -1);
		break;

	case 40:
		_field88 = true;
		player_set_commands_allowed(false);
		player_update_info(_G(my_walker), &_G(player_info));
		_safariShadow3Mach = series_place_sprite("SAFARI SHADOW 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
		setGlobals1(_ripTrekHandTalkPos3Series, 1, 4, 4, 4, 0, 5, 16, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1040);
		digi_play("701R01", 1, 255, -1, -1);

		break;

	case 41:
		sendWSMessage_150000(_G(my_walker), -1);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X01", 1, 255, 71, -1);

		break;

	case 42:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("701R02", 1, 255, 43, -1);

		break;

	case 43:
		sendWSMessage_120000(_G(my_walker), -1);
		updateCounter();
		sendWSMessage_150000(_G(my_walker), -1);

		if (_field50_counter == 1) {
			_field8C_unusedFl = true;
			_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 11, -1, _701rp01Series, 11, 11, 0);
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
			sendWSMessage_1a0000(_agentPoshExpressMach, 11);
			digi_play("701X02", 1, 255, 2200, -1);

		} else if (_field50_counter > 0) {
			_field8C_unusedFl = true;
			_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 11, -1, _701rp01Series, 11, 11, 0);
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
			sendWSMessage_1a0000(_agentPoshExpressMach, 11);
			digi_play("701X03", 1, 255, 2200, -1);

		} else {
			kernel_timing_trigger(1, _alreadyBeenHereFl ? 44 : 95, nullptr);
		}

		break;

	case 44:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 12, 12, 45, _agentTalkLoopTjSeries, 12, 12, 0);

		break;

	case 45:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("701R03", 1, 255, 47, -1);

		break;

	case 46:
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("701R03", 1, 255, 47, -1);

		break;

	case 47:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X04", 1, 255, 48, -1);

		break;

	case 48:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 12, 12, -1, _agentTalkLoopTjSeries, 12, 12, 0);
		sendWSMessage_150000(_G(my_walker), 948);

		break;

	case 49:
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 11, -1, _701rp01Series, 11, 11, 0);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X05", 1, 255, 50, -1);

		break;

	case 50:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 12, 12, -1, _agentTalkLoopTjSeries, 12, 12, 0);
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 11, -1, _701rp01Series, 11, 19, 4);
		sendWSMessage_1a0000(_agentPoshExpressMach02, 11);
		digi_play("701R05", 1, 255, 51, -1);

		break;

	case 51:
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 11, -1, _701rp01Series, 11, 11, 0);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 13, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X06", 1, 255, 52, -1);

		break;

	case 52:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 12, 12, -1, _agentTalkLoopTjSeries, 12, 12, 0);
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 11, -1, _701rp01Series, 11, 19, 4);
		sendWSMessage_1a0000(_agentPoshExpressMach02, 11);
		digi_play("701R06", 1, 255, 53, -1);

		break;

	case 53:
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 11, -1, _701rp01Series, 11, 11, 0);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 13, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X07", 1, 255, 54, -1);

		break;

	case 54:
		terminateMachine(_agentPoshExpressMach02);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentShowMapSeries, 1, 47, 55, _agentShowMapSeries, 47, 47, 0);

		break;

	case 55:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentShowMapSeries, 47, 50, -1, _agentShowMapSeries, 47, 50, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X07A", 1, 255, 56, -1);

		break;

	case 56:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentShowMapSeries, 47, 1, 73, _agentShowMapSeries, 1, 1, 0);

		break;

	case 57:
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 11, -1, _701rp01Series, 11, 11, 4);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X08", 1, 255, 58);

		break;

	case 58:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 12, 12, -1, _agentTalkLoopTjSeries, 12, 12, 0);
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 19, -1, _701rp01Series, 10, 19, 4);
		sendWSMessage_1a0000(_agentPoshExpressMach02, 9);
		digi_play("701R08", 1, 255, 59);

		break;

	case 59:
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 1, 60, _701rp01Series, 1, 1, 0);

		break;

	case 60:
		terminateMachine(_agentPoshExpressMach02);
		terminateMachine(_safariShadow3Mach);
		ws_unhide_walker(_G(my_walker));
		player_set_commands_allowed(true);

		break;

	case 71:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, 42, _agentTalkLoopTjSeries, 13, 13, 0);
		break;

	case 72:

		sendWSMessage_150000(_G(my_walker), 54);
		break;

	case 73:
		_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 11, 74, _701rp01Series, 11, 11, 0);

		break;

	case 74:
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 19, -1, _701rp01Series, 10, 19, 4);
		sendWSMessage_1a0000(_agentPoshExpressMach02, 9);
		digi_play("701R07", 1, 255, 57, -1);

		break;

	case 75:
	case 76:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 43);

		break;

	case 80:
		player_set_commands_allowed(false);
		player_update_info(_G(my_walker), &_G(player_info));
		_safariShadow3Mach = series_place_sprite("SAFARI SHADOW 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
		setGlobals1(_ripTrekHandTalkPos3Series, 1, 4, 4, 4, 0, 5, 16, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1080);
		digi_play("701R37", 1, 255, -1, -1);

		break;

	case 81:
		sendWSMessage_150000(_G(my_walker), -1);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X11", 1, 255, 96, -1);

		break;

	case 82:
		if (_G(flags[V286]) != 1 || _G(flags[V362]) || _G(flags[V372]) || _G(flags[V225])) {
			kernel_timing_trigger(1, 75, nullptr);
		} else {
			_G(flags[V225]) = 1;
			kernel_timing_trigger(1, 83, nullptr);
		}

		break;

	case 83:
		digi_play("701r42", 1, 255, 1083, -1);

		break;

	case 84:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 12, 12, 85, _agentTalkLoopTjSeries, 12, 12, 0);

		break;

	case 85:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 13, -1, _agentTalkLoopTjSeries, 11, 13, 1);
		sendWSMessage_1a0000(_agentPoshExpressMach, 11);
		digi_play("701X07", 1, 255, 86, -1);

		break;

	case 86:
		sendWSMessage_10000(1, _agentPoshExpressMach02, _agentTalkLoopTjSeries, 12, 12, -1, _agentTalkLoopTjSeries, 12, 12, 0);
		_ripSketchingInNotebookPos3Series = series_load("RIP SKETCHING IN NOTEBOOK POS 3", -1, nullptr);
		sendWSMessage_150000(_G(my_walker), -1);
		kernel_timing_trigger(1, 88, nullptr);

		break;

	case 88:
		setGlobals1(_ripSketchingInNotebookPos3Series, 1, 32, 32, 34, 1, 32, 32, 32, 32, 0, 32, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 89);

		break;

	case 89:
		sendWSMessage_190000(_G(my_walker), 9);
		digi_play("950_S34", 2, 255, 97, 950);

		break;

	case 90:
		sendWSMessage_150000(_G(my_walker), 91);
		break;

	case 91:
		series_unload(_ripSketchingInNotebookPos3Series);
		ws_hide_walker(_G(my_walker));
		_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 11, 92, _701rp01Series, 11, 11, 0);

		break;

	case 92:
		terminateMachine(_agentPoshExpressMach02);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentGetTelegramSeries, 86, 1, 93, _agentGetTelegramSeries, 1, 1, 0);

		break;

	case 93:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);
		_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 1, 94, _701rp01Series, 1, 1, 0);

		break;

	case 94:
		terminateMachine(_agentPoshExpressMach02);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 11, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);
		ws_unhide_walker(_G(my_walker));
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		kernel_timing_trigger(1, 76, nullptr);

		break;

	case 95:
		if (_G(flags[V286]) == 1 && !_G(flags[V362]))
			_G(flags[V372]) = 1;

		terminateMachine(_safariShadow3Mach);
		player_set_commands_allowed(true);

		break;

	case 96:
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, 82, _agentTalkLoopTjSeries, 13, 13, 0);

		break;

	case 97:
		sendWSMessage_120000(_G(my_walker), 1089);

		break;

	case 100:
		kernel_timing_trigger(1, 102, nullptr);

		break;

	case 101:
		_field94_mode = 1000;
		_field98_should = 1105;

		break;

	case 102:
		if (_field9E_triggerNum != -1) {
			kernel_timing_trigger(1, _field9E_triggerNum, nullptr);
			_field9E_triggerNum = -1;
		} else
			kernel_timing_trigger(1, 103, nullptr);

		break;

	case 103:
		switch (_field94_mode) {
		case 1000:
			switch (_field98_should) {
			case 1100:
				player_set_commands_allowed(false);
				_field68_mode = 2000;
				_field6C_should = 2100;
				kernel_timing_trigger(1, 110, nullptr);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadow3Mach = series_place_sprite("SAFARI SHADOW 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
				ws_hide_walker(_G(my_walker));
				_ripTalksAgentMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, false, triggerMachineByHashCallback, "rip talks agent");
				_field98_should = 1103;
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 5, 5, 102, _ripTrekTalkerPos3Series, 5, 5, 1);
				conv_load("conv701a", 10, 10, 101);
				_lastInventoryCheck = inventoryCheck();
				_fieldBC_unusedFl = true;

				conv_export_value_curr((_G(flags[V088]) < 3) ? 0 : 1, 0);
				conv_export_pointer_curr(&_lastInventoryCheck, 1);

				_hasCrystalSkull = inv_player_has("CRYSTAL SKULL") ? 1 : 0;
				_hasStickAnsShellMap = inv_player_has("STICK AND SHELL MAP") ? 1 : 0;
				_hasWheeledToy = inv_player_has("WHEELED TOY") ? 1 : 0;
				_hasRebusAmulet = inv_player_has("REBUS AMULET") ? 1 : 0;
				_hasShrunkenHead = inv_player_has("SHRUNKEN HEAD") ? 1 : 0;
				_hasSilverButterfly = inv_player_has("SILVER BUTTERFLY") ? 1 : 0;
				_hasPostageStamp = inv_player_has("POSTAGE STAMP") ? 1 : 0;
				_hasGermanBankNote = inv_player_has("GERMAN BANKNOTE") ? 1 : 0;
				_hasWhaleBoneHorn = inv_player_has("WHALE BONE HORN") ? 1 : 0;
				_hasChisel = inv_player_has("CHISEL") ? 1 : 0;
				_hasIncenseBurner = inv_player_has("INCENSE BURNER") ? 1 : 0;
				_hasRomanovEmerald = inv_player_has("ROMANOV EMERALD") ? 1 : 0;

				conv_export_pointer_curr(&_hasCrystalSkull, 3);
				conv_export_pointer_curr(&_hasStickAnsShellMap, 4);
				conv_export_pointer_curr(&_hasWheeledToy, 5);
				conv_export_pointer_curr(&_hasRebusAmulet, 6);
				conv_export_pointer_curr(&_hasShrunkenHead, 7);
				conv_export_pointer_curr(&_hasSilverButterfly, 8);
				conv_export_pointer_curr(&_hasPostageStamp, 9);
				conv_export_pointer_curr(&_hasGermanBankNote, 10);
				conv_export_pointer_curr(&_hasWhaleBoneHorn, 11);
				conv_export_pointer_curr(&_hasChisel, 12);
				conv_export_pointer_curr(&_hasIncenseBurner, 13);
				conv_export_pointer_curr(&_hasRomanovEmerald, 14);

				_travelDest = 0;
				conv_export_pointer_curr(&_travelDest, 15);
				conv_export_pointer_curr(&_fieldC8, 17);
				conv_play(conv_get_handle());

				break;

			case 1102: {
				int32 rnd = imath_ranged_rand(1, 5);
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, rnd, rnd, 102, _ripTrekTalkerPos3Series, rnd, rnd, 0);
				}

				break;

			case 1103:
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 5, 5, 102, _ripTrekTalkerPos3Series, 5, 5, 0);
				break;

			case 1104:
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 5, 5, -1, _ripTrekTalkerPos3Series, 5, 5, 0);
				break;

			case 1105:
				terminateMachine(_ripTalksAgentMach);
				terminateMachine(_safariShadow3Mach);
				ws_unhide_walker(_G(my_walker));
				_field6C_should = 2103;
				kernel_timing_trigger(1, 2700, nullptr);

				break;

			default:
				break;
			}

			break;

		case 3000:
			switch (_field98_should) {
			case 3100:
				player_set_commands_allowed(false);
				_field98_should = 3101;
				break;

			case 3101:
				terminateMachine(_agentPoshExpressMach02);
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentGetTelegramSeries, 1, 60, 102, _agentGetTelegramSeries, 60, 60, 0);
				_field98_should = 3102;

				break;

			case 3102:
				digi_play("950_S06", 2, 255, 102, 950);
				inv_give_to_player("MESSAGE LOG");
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentGetTelegramSeries, 69, 69, -1, _agentGetTelegramSeries, 69, 69, 0);
				kernel_timing_trigger(1, 2204, nullptr);
				_field98_should = 3103;

				break;

			case 3103:
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentGetTelegramSeries, 71, 71, -1, _agentGetTelegramSeries, 71, 71, 0);
				_field98_should = 3103;

				break;

			case 3105:
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentGetTelegramSeries, 71, 86, 102, _agentGetTelegramSeries, 86, 86, 0);
				_field98_should = 3107;

				break;

			case 3106:
				digi_play("950_S06", 2, 255, -1, 950);
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentGetTelegramSeries, 70, 71, -1, _agentGetTelegramSeries, 71, 71, 0);
				_field98_should = 3103;

				break;

			case 3107:
				if (_field130) {
					kernel_timing_trigger(1, 2501, nullptr);
					_field130 = false;
				} else {
					_field98_should = 3108;
					_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
					sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 1, 102, _701rp01Series, 1, 1, 0);
					sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 1, 1, -1, _agentTalkLoopTjSeries, 1, 1, 0);
				}

				break;

			case 3108:
				terminateMachine(_agentPoshExpressMach02);
				kernel_timing_trigger(1, _alreadyBeenHereFl ? 45 : 95, nullptr);
				ws_unhide_walker(_G(my_walker));

				break;

			default:
				break;
			}
			break;

		case 3500:
			switch (_field98_should) {
			case 3501:
				_agentSignsForMoneySeries = series_load("AGENT  SIGNS FOR MONEY", -1, nullptr);
				_field98_should = 3502;
				_field6C_should = 2101;

				terminateMachine(_ripTalksAgentMach);
				player_set_commands_allowed(false);
				_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
				sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 11, -1, _701rp01Series, 11, 11, 0);
				digi_play(conv_sound_to_play(), 1, 255, 103, -1);

				break;

			case 3502:
				_field6C_should = 2103;
				kernel_timing_trigger(1, 103, nullptr);
				_field98_should = 3504;

				break;

			case 3503:
				terminateMachine(_agentPoshExpressMach02);
				_field98_should = 3504;
				_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
				sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 11, 102, _701rp01Series, 11, 11, 0);

				break;

			case 3504:
				terminateMachine(_agentPoshExpressMach);
				terminateMachine(_agentPoshExpressMach02);
				_field98_should = 3505;
				_ripTalksAgentMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, false, triggerMachineByHashCallback, "rip talks agent");
				sendWSMessage_10000(1, _ripTalksAgentMach, _agentSignsForMoneySeries, 1, 70, 102, _agentSignsForMoneySeries, 70, 70, 0);

				break;

			case 3505:
				digi_play("950_S06", 2, 255, -1, 950);
				_field98_should = 3506;
				sendWSMessage_10000(1, _ripTalksAgentMach, _agentSignsForMoneySeries, 70, 94, 102, _agentSignsForMoneySeries, 94, 94, 0);

				break;

			case 3506:
				terminateMachine(_ripTalksAgentMach);
				_field98_should = 3507;
				_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
				sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 1, 102, _701rp01Series, 1, 1, 0);
				_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
				_field68_mode = 2000;
				_field6C_should = 2102;
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, 110, _agentTalkLoopTjSeries, 13, 13, 0);

				break;

			case 3507:
				_field94_mode = 1000;
				inv_give_to_player("US DOLLARS");
				terminateMachine(_agentPoshExpressMach02);
				_field98_should = 1103;
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTalksAgentMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, false, triggerMachineByHashCallback, "rip talks agent");
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 5, 5, 102, _ripTrekTalkerPos3Series, 5, 5, 0);
				conv_resume(conv_get_handle());
				series_unload(_agentSignsForMoneySeries);

				break;

			default:
				break;
			}

			break;
		case 3600:
			switch (_field98_should) {
			case 3601:
				_agentExchangeMoneySeries = series_load("AGENT  EXCHANGE MONEY", -1, nullptr);
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 1, 5, -1, _ripTrekHandTalkPos3Series, 1, 5, 1);
				_field98_should = 3602;
				digi_play_loop(_convDigiName_2.c_str(), 1, 255, 103, -1);

				break;

			case 3602:
				_field98_should = 3604;
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 5, 5, 103, _ripTrekTalkerPos3Series, 5, 5, 1);

				break;

			case 3603:
				terminateMachine(_agentPoshExpressMach);
				terminateMachine(_agentPoshExpressMach02);
				_field98_should = 3605;
				_ripTalksAgentMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, false, triggerMachineByHashCallback, "rip talks agent");
				sendWSMessage_10000(1, _ripTalksAgentMach, _agentExchangeMoneySeries, 1, 52, 102, _agentExchangeMoneySeries, 52, 52, 0);

				break;

			case 3604:
				terminateMachine(_ripTalksAgentMach);
				_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
				_field98_should = 3603;
				sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 11, 102, _701rp01Series, 11, 11, 0);

				break;

			case 3605:
				digi_play("950_S06", 2, 255, -1, 959);
				_field98_should = 33605;
				sendWSMessage_10000(1, _ripTalksAgentMach, _agentExchangeMoneySeries, 52, 19, 102, _agentExchangeMoneySeries, 19, 19, 0);

				break;

			case 3606:
				_field98_should = 3607;
				terminateMachine(_ripTalksAgentMach);
				_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
				sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 1, 102, _701rp01Series, 1, 1, 0);
				_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, 110, _agentTalkLoopTjSeries, 13, 13, 0);

				break;

			case 3607:
				inv_give_to_player("SIKKIMESE RUPEE");
				terminateMachine(_agentPoshExpressMach02);

				_field94_mode = 1000;
				_field98_should = 1103;

				player_update_info(_G(my_walker), &_G(player_info));
				_ripTalksAgentMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, false, triggerMachineByHashCallback, "rip talks agent");
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 5, 5, 102, _ripTrekTalkerPos3Series, 5, 5, 0);
				conv_resume(conv_get_handle());
				series_unload(_agentExchangeMoneySeries);

				break;

			case 33605:
				digi_play("950_S06", 2, 255, -1, 950);
				_field98_should = 3606;
				sendWSMessage_10000(1, _ripTalksAgentMach, _agentExchangeMoneySeries, 19, 1, 102, _agentExchangeMoneySeries, 1, 1, 0);

				break;

			default:
				break;
			}

			break;
		case 3700:
			switch (_field98_should) {
			case 3701:
				player_set_commands_allowed(false);
				terminateMachine(_ripTalksAgentMach);
				_field98_should = 3702;
				_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
				sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 11, 102, _701rp01Series, 11, 11, 0);

				break;

			case 3702:
				terminateMachine(_agentPoshExpressMach);
				terminateMachine(_agentPoshExpressMach02);
				_field98_should = 3709;
				_ripTalksAgentMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, false, triggerMachineByHashCallback, "rip talks agent");
				sendWSMessage_10000(1, _ripTalksAgentMach, _agentGiveParcelSeries, 1, 33, 102, _agentGiveParcelSeries, 33, 33, 0);

				break;

			case 3704:
				switch (conv_current_entry()) {
				case 0:
					inv_move_object("CRYSTAL SKULL", 305);
					break;

				case 1:
					inv_move_object("STICK AND SHELL MAP", 305);
					break;

				case 2:
					inv_move_object("WHEELED TOY", 305);
					break;

				case 3:
					inv_move_object("REBUS AMULET", 305);
					break;

				case 4:
					inv_move_object("SHRUNKEN HEAD", 305);
					break;

				case 5:
					inv_move_object("SILVER BUTTERFLY", 305);
					break;

				case 6:
					inv_move_object("POSTAGE STAMP", 305);
					break;

				case 7:
					inv_move_object("GERMAN BANKNOTE", 305);
					break;

				case 8:
					inv_move_object("WHALE BONE HORN", 305);
					break;

				case 9:
					inv_move_object("CHISEL", 305);
					break;

				case 10:
					inv_move_object("INCENSE BURNER", 305);
					break;

				case 11:
					setFlag45();
					inv_move_object("ROMANOV EMERALD", 999);
					break;

				default:
					break;
				}

				_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
				_field6C_should = 2102;
				_field68_mode = 2000;
				_field98_should = 3705;

				kernel_timing_trigger(1, 102, nullptr);
				sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, 110, _agentTalkLoopTjSeries, 13, 13, 0);

				break;

			case 3705:
				_field98_should = 3706;
				terminateMachine(_ripTalksAgentMach);
				_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
				sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 1, 102, _701rp01Series, 1, 1, 0);

				break;

			case 3706:
				terminateMachine(_agentPoshExpressMach02);
				_field94_mode = 1000;
				_field98_should = 98;
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTalksAgentMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, false, triggerMachineByHashCallback, "rip talks agent");
				sendWSMessage_10000(1, _ripTalksAgentMach, _ripTrekTalkerPos3Series, 5, 5, 102, _ripTrekTalkerPos3Series, 5, 5, 0);
				conv_resume(conv_get_handle());

				break;

			case 3709:
				_field98_should = 3704;
				sendWSMessage_10000(1, _ripTalksAgentMach, _agentGiveParcelSeries, 34, 51, 102, _agentGiveParcelSeries, 51, 51, 0);
				digi_play("950_S09", 2, 255, -1, 950);

				break;
			default:
				break;
			}

			break;

		default:
			break;

		}

		break;

	case 110:
		if (_field72_triggerNum != -1) {
			kernel_timing_trigger(1, _field72_triggerNum, nullptr);
			_field72_triggerNum = -1;
		} else
			kernel_timing_trigger(1, 111, nullptr);

		break;

	case 111:
		if (_field68_mode != 2000)
			return;

		switch (_field6C_should) {
		case 2100:
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 14, 14, 110, _agentTalkLoopTjSeries, 14, 14, 0);
			_field6C_should = 2102;

			break;

		case 2101: {
			int32 rnd = imath_ranged_rand(14, 17);
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, rnd, rnd, 110, _agentTalkLoopTjSeries, rnd, rnd, 0);
			}

			break;

		case 2102:
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 14, 14, 110, _agentTalkLoopTjSeries, 14, 14, 0);
			break;

		case 2103:
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 14, 14, -1, _agentTalkLoopTjSeries, 14, 14, 0);
			break;

		case 2104:
			sendWSMessage_10000(1, _agentPoshExpressMach, _agentExchangeMoneySeries, 1, 66, 110, _agentExchangeMoneySeries, 66, 66, 0);
			break;

		default:
			break;

		}

		break;

	case 948:
		_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
		ws_hide_walker(_G(my_walker));
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 1, 10, -1, _701rp01Series, 11, 19, 4);
		sendWSMessage_1a0000(_agentPoshExpressMach02, 11);
		digi_play("701R04", 1, 255, 49, -1);

		break;

	case 1040:
		sendWSMessage_120000(_G(my_walker), 41);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 1, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);

		break;

	case 1080:
		sendWSMessage_120000(_G(my_walker), 81);
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 1, 11, -1, _agentTalkLoopTjSeries, 11, 11, 0);

		break;

	case 1083:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("701R38", 1, 255, 84, -1);

		break;

	case 1089:
		sendWSMessage_190000(_G(my_walker), 5);
		sendWSMessage_130000(_G(my_walker), 90);

		break;

	case 2200:
		_field98_should = 3100;
		_field94_mode = 3000;
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 13, 13, 102, _agentTalkLoopTjSeries, 13, 13, 0);

		break;

	case 2201:
		_field98_should = 3103;
		kernel_timing_trigger(60, 2300, nullptr);

		break;

	case 2300:
		_field58_digiName = nullptr;
		_field5C_digiName = nullptr;
		_field60_digiName = nullptr;
		_field64_digiName = nullptr;

		if (_field50_counter <= 0) {
			kernel_timing_trigger(1, 44, nullptr);
			return;
		}

		if (_itemDigiName) {
			_field58_digiName = _itemDigiName;
			_itemDigiName = nullptr;
		} else if (_G(flags[V364]) == 1) {
			_field58_digiName = "201R26";
		} else if (_G(flags[V365]) == 1) {
			_field58_digiName = "201R61";
		} else if (_G(flags[V366]) == 1) {
			_field58_digiName = "401R31";
		} else if (_G(flags[V373]) == 1) {
			_field58_digiName = "401R36";
		} else if (_G(flags[V370]) == 1) {
			_field58_digiName = "501R02";
		} else if (_G(flags[V371]) == 1) {
			_field58_digiName = "501R03";
		} else if (_G(flags[V372]) == 1) {
			_field58_digiName = "701R39";
			_field5C_digiName = "701R39A";
		} else if (_G(flags[V367]) == 1) {
			_field58_digiName = "401R37";
			_G(flags[V367]) = 0;
			_field130 = true;
		} else if (_G(flags[V368]) == 1) {
			_field58_digiName = "401R38";
			_G(flags[V368]) = 0;
			_field130 = true;
		} else if (_G(flags[V369]) == 1) {
			_field58_digiName = "401R39";
			_G(flags[V369]) = 0;
			_field130 = true;
		}

		kernel_timing_trigger(1, 2301, nullptr);
		break;

	case 2301:
		_field98_should = 3106;
		kernel_timing_trigger(1, 2302, nullptr);

		break;

	case 2302:
		if (_field58_digiName)
			digi_play(_field58_digiName, 1, 255, 2303, -1);
		else
			kernel_timing_trigger(1, 2303, nullptr);

		break;

	case 2303:
		if (_field5C_digiName)
			digi_play(_field5C_digiName, 1, 255, 2304, -1);
		else
			kernel_timing_trigger(1, 2304, nullptr);

		break;

	case 2304:
		if (_field60_digiName)
			digi_play(_field60_digiName, 1, 255, 2305, -1);
		else
			kernel_timing_trigger(1, 2305, nullptr);

		break;

	case 2305:
		if (_field64_digiName)
			digi_play(_field64_digiName, 1, 255, 2306, -1);
		else
			kernel_timing_trigger(1, 2306, nullptr);

		break;

	case 2306:
		if (_field130) {
			_field94_mode = 3000;
			_field98_should = 3105;
			kernel_timing_trigger(1, 102, nullptr);
		} else {
			--_field50_counter;
			if (_field50_counter <= 0) {
				_field98_should = 3106;
				kernel_timing_trigger(1, 2300, nullptr);
			} else {
				kernel_timing_trigger(1, 2307, nullptr);
			}
		}

		break;

	case 2307:
		_field98_should = 3105;
		kernel_timing_trigger(10, 102, nullptr);

		break;

	case 2501:
		terminateMachine(_agentPoshExpressMach);
		_agentPoshExpressMach03 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach03, _agentGiveParcelSeries, 51, 33, 2509, _agentGiveParcelSeries, 33, 33, 0);

		break;

	case 2504:
		terminateMachine(_agentPoshExpressMach03);
		_agentPoshExpressMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach02, _701rp01Series, 11, 1, 2506, _701rp01Series, 1, 1, 0);
		if (!inv_player_has("ROMANOV EMERALD")) {
			inv_give_to_player("ROMANOV EMERALD");
		}

		_agentPoshExpressMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1792, false, triggerMachineByHashCallback, "agent posh express");
		sendWSMessage_10000(1, _agentPoshExpressMach, _agentTalkLoopTjSeries, 1, 1, -1, _agentTalkLoopTjSeries, 1, 1, 0);

		break;

	case 2506:
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_agentPoshExpressMach02);
		kernel_timing_trigger(1, _alreadyBeenHereFl ? 45 : 95, nullptr);

		break;

	case 2509:
		sendWSMessage_10000(1, _agentPoshExpressMach03, _agentGiveParcelSeries, 32, 1, 2504, _agentGiveParcelSeries, 1, 1, 0);
		digi_play("950_S09", 2, 255, -1, 950);

		break;

	case 2600:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(2601);

		break;

	case 2601:
		switch (_travelDest) {
		case 1:
			_G(flags[kTravelDest]) = 1;
			break;

		case 2:
			_G(flags[kTravelDest]) = 0;
			break;

		case 3:
			_G(flags[kTravelDest]) = 4;
			break;

		case 4:
			_G(flags[kTravelDest]) = 2;
			break;

		default:
			break;
		}

		_G(game).new_room = 495;
		_G(game).new_section = 4;

		break;

	case 2700:
		kernel_timing_trigger(1, 2701, nullptr);
		break;

	case 2701:
		kernel_timing_trigger(1, 2702, nullptr);
		break;

	case 2702:
		if (_field90)
			kernel_timing_trigger(1, 2600, nullptr);
		else
			player_set_commands_allowed(true);

		break;

	default:
		break;
	}
}

void Room701::conv701a() {
	_conv701aNode = conv_current_node();
	int32 entry = conv_current_entry();
	const char *digiName = conv_sound_to_play();

	if (digiName == nullptr) {
		conv_resume(conv_get_handle());
		return;
	}

	int32 who = conv_whos_talking();
	if (who <= 0) {
		if (_conv701aNode == 3 && entry == 0) {
			_convDigiName_1 = Common::String(digiName);
			_field94_mode = 3500;
			_field98_should = 3501;
			return;
		}

		_field6C_should = 2101;

	} else if (who == 1) {
		if (_conv701aNode == 1 && entry == 1) {
			conv_set_box_xy(490, -4);
			set_dlg_rect();
			int32 x1, y1, x2, y2;
			conv_get_dlg_coords(&x1, &y1, &x2, &y2);
			conv_get_dlg_coords(&x1, &y1, &x2, &y2); // useless?
			conv_set_dlg_coords(x1, y1 - 10, x2, y2 - 10);
			set_dlg_rect();
		} else {
			conv_set_box_xy(10, 10);
		}

		switch (_conv701aNode) {
		case 1:
			if (entry == 3) {
				_convDigiName_2 = Common::String(digiName);
				_field94_mode = 3600;
				_field98_should = 3601;

				return;
			}

			_field98_should = 1102;

			break;

		case 6:
			if (entry == 0) {
				_field98_should = 1102;
				digi_play(digiName, 1, 255, 1, -1);
				_field90 = true;

				return;
			}

			_field98_should = 1102;

			break;

		case 11:
			if (entry != 12) {
				_field94_mode = 3700;
				_field98_should = 3701;

				return;
			}

			break;

		default:
			_field98_should = 1102;
			break;
		}
	}

	digi_play(digiName, 1, 255, 1, -1);
}

void Room701::updateCounter() {
	if (_G(flags[V364]) == 1) {
		++_field50_counter;
		switch (_G(flags[V005])) {
		case 1:
			_G(flags[V351]) = 1;
			break;

		case 2:
			_G(flags[V352]) = 1;
			break;

		case 3:
			_G(flags[V353]) = 1;
			break;

		case 4:
			_G(flags[V354]) = 1;
			break;

		default:
			break;
		}
	}

	if (_G(flags[V365]) == 1) {
		_G(flags[V355]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V366]) == 1) {
		_G(flags[V356]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V367]) == 1) {
		_G(flags[V357]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V368]) == 1) {
		_G(flags[V358]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V369]) == 1) {
		_G(flags[V359]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V370]) == 1) {
		_G(flags[V360]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V371]) == 1) {
		_G(flags[V361]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V372]) == 1) {
		_G(flags[V362]) = 1;
		++_field50_counter;
	}

	if (_G(flags[V373]) == 1) {
		_G(flags[V363]) = 1;
		++_field50_counter;
	}
}

int32 Room701::inventoryCheck() {
	for (int i = 0; i < 12; ++i)
		_inventoryCheckArray[i] = 0;

	_inventoryCheckCounter = 0;

	if (inv_player_has("CRYSTAL SKULL"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 1;

	if (inv_player_has("STICK AND SHELL MAP"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 2;

	if (inv_player_has("WHEELED TOY"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 3;

	if (inv_player_has("REBUS AMULET"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 4;

	if (inv_player_has("SHRUNKEN HEAD"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 5;

	if (inv_player_has("SILVER BUTTERFLY"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 6;

	if (inv_player_has("POSTAGE STAMP"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 7;

	if (inv_player_has("GERMAN BANKNOTE"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 8;

	if (inv_player_has("WHALE BONE HORN"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 9;

	if (inv_player_has("CHISEL"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 10;

	if (inv_player_has("INCENSE BURNER"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 11;

	if (inv_player_has("ROMANOV EMERALD"))
		_inventoryCheckArray[_inventoryCheckCounter++] = 12;

	return _inventoryCheckCounter > 0 ? 1 : 0;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
