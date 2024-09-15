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
	switch (_G(kernel).trigger) {
	case 503:
		player_set_commands_allowed(false);
		ws_walk(423, 356, nullptr, 504, 1);
		break;

	case 504:
		_ripPutsShovel = series_load("RIP PUTS SHOVEL IN CAPSTAN");
		digi_preload("508_S03");
		player_update_info();
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp1");
		sendWSMessage_10000(1, _ripley, _ripPutsShovel, 1, 22, 505,
			_ripPutsShovel, 22, 22, 0);
		inv_move_object("SHOVEL", 508);
		break;

	case 505:
		digi_play("508_S03", 2);
		sendWSMessage_10000(1, _ripley, _ripPutsShovel, 23, 34, 506,
			_ripPutsShovel, 34, 34, 0);
		break;

	case 506:
		ws_unhide_walker();
		_shovel = series_place_sprite("SHOVEL SPRITE", 0, 0, 0, 100, 0x300);
		hotspot_set_active("SHOVEL", true);
		terminateMachineAndNull(_ripley);
		kernel_timing_trigger(1, 507);
		break;

	case 507:
		series_unload(_ripPutsShovel);
		digi_unload("508_S03");
		player_set_commands_allowed(true);
		break;

	case 508:
		player_set_commands_allowed(false);
		_ripPutsShovel = series_load("RIP PUTS SHOVEL IN CAPSTAN");
		digi_preload("508_S03");
		player_update_info();
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp2");
		sendWSMessage_10000(1, _ripley, _ripPutsShovel, 34, 25, 509,
			_ripPutsShovel, 25, 25, 0);
		hotspot_set_active("SHOVEL", false);
		inv_give_to_player("SHOVEL");
		terminateMachineAndNull(_shovel);
		break;

	case 509:
		digi_play("508_S03", 2);
		sendWSMessage_10000(1, _ripley, _ripPutsShovel, 24, 1, 510,
			_ripPutsShovel, 1, 1, 0);
		break;

	case 510:
		ws_unhide_walker();
		terminateMachineAndNull(_ripley);
		series_unload(_ripPutsShovel);
		digi_unload("508_S03");
		player_set_commands_allowed(true);
		break;

	case 511:
		player_set_commands_allowed(false);
		_ripTryTurnDome = series_load("508 RIP TRYS TURNING DOME");
		player_update_info();
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp3");
		terminateMachineAndNull(_shovel);
		sendWSMessage_10000(1, _ripley, _ripTryTurnDome, 1, 29, 512,
			_ripTryTurnDome, 29, 29, 0);
		break;

	case 512:
		sendWSMessage_10000(1, _ripley, _ripTryTurnDome, 29, 1, 513,
			_ripTryTurnDome, 1, 1, 0);
		break;

	case 513:
		_shovel = series_place_sprite("SHOVEL SPRITE", 0, 0, 0, 100, 0x300);
		ws_unhide_walker();
		terminateMachineAndNull(_ripley);
		series_unload(_ripTryTurnDome);

		if (_val1)
			kernel_timing_trigger(1, 514);
		else
			digi_play("508r11", 1, 255, 514);
		break;

	case 514:
		if (_val1) {
			digi_play("508R16", 1);
			_val1 = 0;
		} else {
			player_set_commands_allowed(true);
		}
		break;

	case 515:
		player_set_commands_allowed(false);
		interface_hide();
		digi_preload("508_s02");
		digi_preload("508_s04");
		_ripTryTurnDome = series_load("508 RIP TRYS TURNING DOME");
		player_update_info();
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp4");
		sendWSMessage_10000(1, _ripley, _ripTryTurnDome, 1, 29, 516,
			_ripTryTurnDome, 29, 29, 0);
		break;

	case 516:
		_ripStartTurnDome = series_stream("RIP STARTS TURNING DOME", 7, 0x100, -1);
		series_stream_break_on_frame(_ripStartTurnDome, 19, 518);
		kernel_timing_trigger(90, 517);
		terminateMachineAndNull(_shovel);
		terminateMachineAndNull(_skull);
		terminateMachineAndNull(_ripley);
		break;

	case 517:
		digi_play_loop("508_S02", 1);
		digi_play_loop("508_S04", 2);
		break;

	case 518:
		terminateMachineAndNull(_shovel);
		terminateMachineAndNull(_skull);
		terminateMachineAndNull(_ripley);
		series_stream_check_series(_ripStartTurnDome, 9999);
		kernel_timing_trigger(1, 519);
		break;

	case 519:
		pal_fade_init(21, 255, 0, 30, 520);
		break;

	case 520:
		_G(flags)[V158] = 1;
		digi_play_loop("508_s01", 3, 120);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("508_S02", 1);
		digi_play_loop("508_S04", 2);
		_G(game).setRoom(506);
		break;

	case 525:
		digi_stop(1);
		digi_stop(2);
		digi_unload("508_s02");
		digi_unload("508_s04");

		_domeAfterTurn = series_place_sprite("DOME SPRITE AFTER ITS TURNED", 0, 0, 0, 100, 0xf00);
		_skull = series_place_sprite("SKULL SPRITE AFTER DOME TURN", 0, 0, 0, 100, 0x450);
		_shovel = series_place_sprite("SHOVEL AFTER DOMES TURNED", 0, 0, 0, 100, 0x300);
		_statue = series_place_sprite("STATU SPRITE AFTER DOME TURN", 0, 0, 0, 100, 0x450);

		_light = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
			triggerMachineByHashCallbackNegative, "Receeding Light");
		sendWSMessage_10000(1, _light, _lightAppearing, 12, 12, -1,
			_lightAppearing, 12, 12, 0);

		player_update_info();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp");
		sendWSMessage_10000(1, _ripley, _ripReturnsToStander, 1, 10, -1,
			_ripReturnsToStander, 10, 10, 0);

		_chain = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
			triggerMachineByHashCallbackNegative, "Chain Breaking Machine");
		sendWSMessage_10000(1, _chain, _chainBreaking, 1, 58, 675,
			_chainBreaking, 58, 58, 0);
		digi_play("508_s08", 1, 255, 555);
		break;

	case 530:
		series_unload(_ripReturnsToStander);
		_G(flags)[V158] = 0;
		_G(flags)[V157] = 0;
		break;

	case 539:
		_x = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
			triggerMachineByHashCallbackNegative, "x");
		sendWSMessage_10000(1, _x, _flick, 1, 2, -1, _flick, 1, 2, 0);
		sendWSMessage_190000(_x, 1);
		sendWSMessage_1a0000(_x, 1);
		series_stream_check_series(_spect, 5);
		ws_OverrideCrunchTime(_spect);
		break;

	case 541:
		sendWSMessage_190000(_light, 17);
		sendWSMessage_10000(1, _light, _lightAppearing, 12, 8, 542,
			_lightAppearing, 8, 8, 0);
		break;

	case 542:
		sendWSMessage_10000(1, _light, _lightAppearing, 8, 1, 543,
			_lightAppearing, 1, 1, 0);
		sendWSMessage_10000(1, _x, _flick, 1, 6, 679, _flick, 6, 6, 0);
		break;

	case 543:
		terminateMachineAndNull(_light);
		series_unload(_lightAppearing);
		sendWSMessage_120000(544);
		break;

	case 544:
		sendWSMessage_10000(1, _x, _flick, 1, 6, 679, _flick, 6, 6, 0);
		sendWSMessage_130000(545);
		break;

	case 545:
		sendWSMessage_150000(546);
		break;

	case 546:
		_G(flags)[V161] = 1;
		series_unload(_ripSketching);
		kernel_timing_trigger(5, 552);
		break;

	case 548:
		digi_play("508R12", 1, 255, 539);
		break;

	case 552:
		digi_play("508R14", 1, 255, 553);
		break;

	case 553:
		_G(flags)[V157] = 1;
		_G(flags)[V287] = 1;
		_val2 -= 5;

		if (_val2 > 0) {
			digi_change_panning(2, _val2);
			kernel_timing_trigger(2, 553);
		} else {
			digi_stop(2);
			interface_show();
			player_set_commands_allowed(true);
		}
		break;

	case 555:
		digi_unload("508_s08");
		digi_preload("508_s07");
		_val2 = 255;

		digi_play("508_S07", 2);
		_lightAppearing = series_load("LIGHT APPEARING ON FLOOR");
		_flick = series_load("508flick");
		_spect = series_stream("508spect", 6, 0, 668);
		series_stream_break_on_frame(_spect, 16, 666);
		kernel_timing_trigger(30, 673);
		break;

	case 562:
		player_set_commands_allowed(true);
		break;

	case 666:
		series_stream_check_series(_spect, 3000);
		hotspot_set_active("CRYSTAL SKULL ", true);
		hotspot_set_active("SHOVEL ", true);
		hotspot_set_active("SHOVEL", false);
		kernel_timing_trigger(5, 530);
		break;

	case 668:
		kernel_timing_trigger(1, 671);
		break;

	case 671:
		_ripSketching = series_load("RIP SKETCHING IN NOTEBOOK POS 2");
		setGlobals1(_ripSketching, 1, 24, 24, 24, 0, 24, 39, 39, 39, 0, 39, 1, 1, 1);
		sendWSMessage_110000(681);
		break;

	case 673:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		ws_demand_location(437, 349, 1);
		ws_walk(436, 359, nullptr, 548, 10);
		break;

	case 675:
		hotspot_set_active("CHAIN ", false);
		terminateMachineAndNull(_chain);
		series_unload(_chainBreaking);
		_chainAfterBreak = series_place_sprite("508 CHAIN AFTER BREAK", 0, 0, 0, 100, 0xf00);
		break;

	case 679:
		terminateMachineAndNull(_x);
		series_unload(_flick);
		break;

	case 681:
		sendWSMessage_120000(682);
		break;

	case 682:
		sendWSMessage_110000(684);
		break;

	case 684:
		sendWSMessage_120000(686);
		kernel_timing_trigger(180, 541);
		break;

	case 686:
		sendWSMessage_130000(545);
		break;

	default:
		break;
	}
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
