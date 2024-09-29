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

#include "m4/riddle/rooms/section6/room610.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/other.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room610::init() {
	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 1;
		_val2 = 1;
		_val3 = 0;
		_val4 = 1;
		_val5 = 1;
		_val6 = 0;
	}

	_val7 = 0;
	_val8 = 0;
	_G(kernel).call_daemon_every_loop = true;

	digi_preload("610s01");
	digi_preload("610_s02");
	digi_preload("610_s02a");
	digi_preload("610_s03");
	digi_preload("610_s03a");
	digi_preload("610_s03b");
	digi_preload("610k07");
	digi_preload("610k08");
	digi_preload("610k09");
	digi_preload("610k10");
	digi_preload("610k11");
	digi_preload("610k12");
	digi_preload("610k13");

	_ripPeerAroundHut = series_load("RIP PEERS AROUND THE HUT");
	_rp04 = series_load("610RP04");
	_k00 = series_load("610K00");
	_k01 = series_load("610K01");
	_sgSlapsTt1 = series_load("SG SLAPS TT");
	_sgSlapsTt2 = series_load("SG SLAPS TT AGAIN");
	_sgPullsGun = series_load("SG PULLS A GUN");

	if (_G(flags)[GLB_TEMP_6]) {
		hotspot_set_active("window", false);
		series_show("610 shed window open", 0xf00, 16);
	} else {
		hotspot_set_active("window ", false);
	}

	_k = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xa00, 0,
		triggerMachineByHashCallback610, "k");
	_sgTt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xb00, 0,
		triggerMachineByHashCallback610, "sg and tt");

	switch (_G(kernel).trigger) {
	case KERNEL_RESTORING_GAME:
		midi_play("tensions", 255, 1, -1, 949);

		if (_flag1)
			ws_demand_location(615, 364, 10);

		setup();
		kernel_timing_trigger(300, 135);
		digi_preload("950_s28a");
		break;

	case 604:
		digi_preload("950_s28a");
		setup();
		kernel_timing_trigger(300, 135);
		ws_demand_location(472, 262, 3);
		break;
 
	default:
		digi_preload("950_s28a");
		digi_stop(3);
		digi_unload("950_s28");

		if (player_been_here(610)) {
			setup();
			player_set_commands_allowed(false);
			ws_demand_location(665, 364, 9);
			ws_walk(615, 364, nullptr, 1, 10);
		} else {
			sendWSMessage_10000(1, _k, _k00, 1, 50, -1, _k00, 50, 60, 4);
			sendWSMessage_10000(1, _sgTt, _sgSlapsTt1, 1, 1, -1,
				_sgSlapsTt1, 1, 1, 4);

			player_set_commands_allowed(false);
			ws_demand_location(665, 364, 9);
			ws_walk(240, 272, nullptr, 10, 10);
			kernel_timing_trigger(1, 100);
		}
		break;
	}

	_flag2 = true;
	_flag1 = false;
	digi_play_loop("950_s28a", 3, 110);
}

void Room610::daemon() {
	// TODO
}

void Room610::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");

	if (lookFlag && player_said_any("HUT", "PIT", "SAMANTHA", "KUANG", "SHEN GUO"))
		_G(player).resetWalk();
}

void Room610::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (talkFlag && player_said_any("KUANG", "SHEN GUO")) {
		talkKuangShenGuo();
	} else if (talkFlag && player_said("SAMANTHA")) {
		talkSamantha();
	} else if (useFlag && player_said("WHALE BONE HORN") && useHorn()) {
		// No implementation
	} else if (lookFlag && player_said("SAMANTHA")) {
		digi_play("610r01", 1);
	} else if (lookFlag && player_said("KUANG")) {
		digi_play("610r02", 1);
	} else if (lookFlag && player_said("SHEN GUO")) {
		digi_play("610r03", 1);
	} else if (lookFlag && player_said("HUT")) {
		digi_play("610r04", 1);
	} else if (lookFlag && player_said("PIT")) {
		digi_play("610r05", 1);
	} else if (lookFlag && player_said("SHED")) {
		digi_play("610r06", 1);
	} else if (lookFlag && player_said("WINDOW")) {
		digi_play("610r07", 1);
	} else if (lookFlag && player_said("WINDOW ")) {
		digi_play("604r47", 1);
	} else if (lookFlag && player_said("POWER LINE")) {
		digi_play("610r08", 1);
	} else if ((useFlag && player_said_any("WINDOW", "WINDOW ")) ||
			player_said("enter")) {
		useWindow();
	} else if (_G(kernel).trigger == 555) {
		midi_stop();

		static const char *DIGI[] = {
			"610_s02", "610_s02a", "610_s03", "610_s03a",
			"610_s03b", "610k07", "610k08", "610k09",
			"610k10", "610k11", "610k12", "610k13", nullptr
		};
		for (const char **name = DIGI; *name; ++name)
			digi_unload(*name);

		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28a", 3, 110);
		_G(game).setRoom(605);
	} else if (player_said("exit")) {
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(-1);
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 555);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room610::triggerMachineByHashCallback610(frac16 myMessage, machine *sender) {
	int trigger = myMessage >> 16;

	if (trigger >= 0)
		kernel_trigger_dispatchx(trigger);
}

void Room610::setup() {
	sendWSMessage_10000(1, _sgTt, _sgSlapsTt2, 1, 1, 200,
		_sgSlapsTt2, 1, 1, 1);
	sendWSMessage_10000(1, _k, _k00, 50, 50, 400, _k00, 50, 60, 4);
	sendWSMessage_190000(_k, 11);
	sendWSMessage_1a0000(_k, 11);
	_val1 = 1;
	_val2 = 0;
	_val3 = 0;
	_val4 = 0;
	_val5 = 1;
	_val6 = 0;
}

void Room610::talkKuangShenGuo() {
	switch (_G(kernel).trigger) {
	case -1:
		_flag1 = true;
		digi_stop(2);
		player_set_commands_allowed(false);
		_val5 = 2;
		_val2 = 4;
		digi_play("610r12", 1, 255, 1);
		break;

	case 1:
		ws_hide_walker();
		_spleen = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback610, "spleen");
		sendWSMessage_10000(1, _spleen, _rp04, 1, 21, 2, _rp04, 21, 11, 4);
		sendWSMessage_1a0000(_spleen, 50);
		break;

	case 2:
		digi_play("610r13", 1, 255, 3);
		break;

	case 3:
		disable_player_commands_and_fade_init(4);
		interface_hide();
		break;

	case 4:
		kernel_timing_trigger(30, 5);
		break;

	case 5:
		digi_play("950_s14", 1, 255, 6);
		break;

	case 6:
		other_save_game_for_resurrection();
		_G(game).setRoom(413);
		break;

	default:
		break;
	}
}

void Room610::talkSamantha() {
	switch (_G(kernel).trigger) {
	case -1:
		_flag1 = true;
		digi_stop(2);
		player_set_commands_allowed(false);
		_val5 = 2;
		_val2 = 4;
		digi_play("610r14", 1, 255, 1);
		break;

	case 1:
		ws_hide_walker();
		_spleen = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback610, "spleen");
		sendWSMessage_10000(1, _spleen, _rp04, 1, 21, 2, _rp04, 21, 11, 4);
		sendWSMessage_1a0000(_spleen, 50);
		break;

	case 2:
		digi_play("610r15", 1, 255, 3);
		break;

	case 3:
		disable_player_commands_and_fade_init(4);
		interface_hide();
		break;

	case 4:
		kernel_timing_trigger(30, 5);
		break;

	case 5:
		digi_play("950_s14", 1, 255, 6);
		break;

	case 6:
		other_save_game_for_resurrection();
		_G(game).setRoom(413);
		break;

	default:
		break;
	}
}

bool Room610::useHorn() {
	_flag1 = true;

	if (_G(kernel).trigger == 5) {
		_val5 = 2;
		_val2 = 4;
	}

	// Original never exits parser even when this code is called
	return false;
}

void Room610::useWindow() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);

		if (_G(flags)[GLB_TEMP_6]) {
			kernel_timing_trigger(1, 1);
		} else {
			ws_hide_walker();
			_spleen = series_play("610rp02", 0, 0, 3, 5, 0, 100, 0, 0, 0, 32);
		}
		break;

	case 1:
		disable_player_commands_and_fade_init(2);
		break;

	case 2:
		_G(game).setRoom(604);
		break;

	case 3:
		_spleen = series_play("610rp02", 0, 0, 4, 5, 0, 100, 0, 0, 33, 64);
		digi_play("610_s01", 2);
		hotspot_set_active("window", false);
		hotspot_set_active("window ", true);
		_G(flags)[GLB_TEMP_6] = 1;
		break;

	case 4:
		series_show("610 shed window open", 0xf00, 16);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
