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

#include "m4/riddle/rooms/section2/room204.h"
#include "m4/riddle/rooms/section2/section2.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room204::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room204::init() {
	digi_preload("950_s04", -1);
	digi_play_loop("950_s04", 3, 70, -1, -1);
	if (!_G(flags[V070]))
		_G(flags[V078]) = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_field4 = 0;
		_field180 = 0;
		_field184 = 0;
		_field188 = 0;

		if (!player_been_here(205) && (!_G(flags[V056]) || _G(flags[V049]) == 1)) {
			_field4 = 1;
			initWalkerSeries();
			_G(flags[V049]) = 0;
		}
	}

	_courtyardGongSeries = series_load("COURTYARD GONG", -1, nullptr);
	_malletSpriteSeries = series_load("MALLET SPRITE", -1, nullptr);
	_field68 = 0;
	_field44_triggerNum = -1;
	_field48_triggerNum = -1;
	_fieldC4 = -1;
	_fieldBC_trigger = -1;
	_fieldEC = -1;
	_fieldF0 = -1;
	_field104 = 0;
	_field108 = 0;

	if (inv_object_is_here("SILVER BUTTERFLY")) {
		_silverButterflyCoinMach = series_place_sprite("SILVER BUTTERFLY COIN", 0, 1280, 0, 100, 3840);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "silver butterfly", false);
	}

	if (inv_object_is_here("GONG")) {
		_courtyardGongMach = series_place_sprite("COURTYARD GONG", 0, 0, 0, 100, 2457);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "gong", false);
	}

	if (inv_object_is_here("MALLET")) {
		_malletSpriteMach = series_place_sprite("MALLET SPRITE", 0, 0, 0, 100, 2304);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "mallet", false);
	}

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (player_been_here(205)) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "acolyte", false);
			_G(camera_reacts_to_player) = false;
		} else {
			kernel_timing_trigger(1, 578, nullptr);
			if (_field4 == 1) {
				initWalkerSeries();

				_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, _fieldE0, 323, (_fieldDC == 1) ? 10 : 2, Walker::player_walker_callback, "mc walker room 204");
				addMovingMeiHotspot();
				if (_fieldE0 == 472) {
					deleteMalletHotspot();
					addLookMalletHotspot();
				}

				kernel_timing_trigger(1, 630, nullptr);
			}
		}
	} else if (_G(game).previous_room == 205) {
		player_set_commands_allowed(false);
		_G(camera_reacts_to_player) = false;
		hotspot_set_active(_G(currentSceneDef).hotspots, "acolyte", false);
		ws_demand_location(_G(my_walker), 421, 330);
		ws_demand_facing(_G(my_walker), 3);
		ws_hide_walker(_G(my_walker));
		series_play("204 leap down", 3840, 0, 15, 5, 0, 100, 0, 0, 0, -1);
	} else if (keyCheck() && !player_been_here(205)) {
		_G(camera_reacts_to_player) = false;
		player_set_commands_allowed(false);
		ws_demand_location(_G(my_walker), 600, 334);
		ws_demand_facing(_G(my_walker), 9);
		sendWSMessage_10000(_G(my_walker), 424, 331, 9, 9, true);
		_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 620, 340, 9, Walker::player_walker_callback, "mc");
		sendWSMessage_10000(_mcMach, 450, 340, 9, -1, true);
		kernel_timing_trigger(60, 5, nullptr);
	} else {
		_fieldDC = 0;
		_field40 = 0;
		player_set_commands_allowed(false);
		if (!_G(flags[V070])) {
			digi_preload("204_S02", -1);
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 1864, 334, 9, Walker::player_walker_callback, "mc walker room 204");
			DisposePath(_mcMach->walkPath);
			_ripDropsSeries = series_load("RIP DROPS", -1, nullptr);
			player_set_commands_allowed(false);
			ws_demand_location(_G(my_walker), 1864, 334);
			ws_demand_facing(_G(my_walker), 3);

			_G(flags[V070]) = 1;
			_G(flags[V068]) = 1;
			_G(flags[V078]) = 1;

			int32 status;
			ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
			MoveScreenDelta(game_buff_ptr, -1280, 0);
			kernel_timing_trigger(1, 500, nullptr);
		} else {
			_G(flags[V068]) = 1;
			if (!_field4) {
				ws_demand_location(_G(my_walker), 1864, 334);
				ws_demand_facing(_G(my_walker), 9);
				int32 status;
				ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
				MoveScreenDelta(game_buff_ptr, -1280, 0);
				kernel_timing_trigger(1, 708, nullptr);
			} else if (_G(flags[V049]) == 1) {
				_G(flags[V049]) = 0;
				_G(flags[V078]) = 2;
				ws_demand_location(_G(my_walker), 1864, 334);
				ws_demand_facing(_G(my_walker), 9);
				_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 1864, 334, 4, Walker::player_walker_callback, "mc walker room 204");
				kernel_timing_trigger(1, 536, nullptr);
				int32 status;
				ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
				MoveScreenDelta(game_buff_ptr, -1280, 0);
			} else {
				ws_demand_location(_G(my_walker), 1864, 334);
				ws_demand_facing(_G(my_walker), 9);
				int32 status;
				ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
				MoveScreenDelta(game_buff_ptr, -1280, 0);
				kernel_timing_trigger(1, 708, nullptr);
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					_fieldE4_walkerDestX = 1663;
					break;

				case 2:
					_fieldE4_walkerDestX = 1576;
					break;

				case 3:
					_fieldE4_walkerDestX = 1494;
					break;

				case 4:
					_fieldE4_walkerDestX = 1412;
					break;

				default:
					break;
				}

				_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, _fieldE4_walkerDestX, 323, 2, Walker::player_walker_callback, "mc walker room 204");
				addMovingMeiHotspot();
				kernel_timing_trigger(1, 630, nullptr);
				kernel_timing_trigger(1, 578, nullptr);
			}
		}
	}
}

void Room204::pre_parser() {
	if (player_said("enter", "PAGODA") || player_said("walk to", "PAGODA") || player_said("walk to", "stairs")) {
		if (player_been_here(205)) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_been_here(205) && inv_object_is_here("GONG")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room204::parser() {
}

void Room204::daemon() {
	switch (_G(kernel).trigger) {
	case 5:
		digi_play("204r47", 1, 255, -1, -1);
		break;

	case 9:
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		series_play("204rp99", 3840, 0, 11, 5, 0, 100, 0, 0, 0, -1);
		kernel_timing_trigger(130, 11, nullptr);

		break;

	case 10:
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		series_play("204up", 3840, 16, 11, 5, 0, 100, 0, 0, 0, -1);
		kernel_timing_trigger(90, 11, nullptr);

		break;

	case 11:
		pal_fade_init(_G(master_palette), 21, 255, 0, 15, 12);
		break;

	case 12:
		interface_show();
		_G(game).new_room = 205;

		break;

	case 15:
		ws_unhide_walker(_G(my_walker));
		sendWSMessage_10000(_G(my_walker), 440, 330, 3, 50, true);

		break;

	case 50:
		player_set_commands_allowed(true);
		break;

	case 500:
		player_set_commands_allowed(false);
		kernel_timing_trigger(1, 501, nullptr);

		break;

	case 501:
		_ripDeltaMachineStateMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, _G(player_info).depth, 0, triggerMachineByHashCallback, "Rip Delta Machine State");
		sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripDropsSeries, 1, 2, 502, _ripDropsSeries, 2, 2, 0);

		break;

	case 502:
		digi_play("204_s02", 1, 255, -1, -1);
		sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripDropsSeries, 3, 40, 503, _ripDropsSeries, 40, 40, 0);

		break;

	case 503:
		digi_unload("204_s02");
		terminateMachine(_ripDeltaMachineStateMach);
		_ripDeltaMachineStateMach = nullptr;
		series_unload(_ripDropsSeries);
		ws_demand_location(_G(my_walker), 1750, 328);
		ws_demand_facing(_G(my_walker), 5);
		_ripTrekHeadTurnPos5Series = series_load("RIP TREK HEAD TURN POS5", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos5Series, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 504);

		break;

	case 504:
		sendWSMessage_140000(_G(my_walker), 505);
		break;

	case 505:
		setGlobals1(_ripTrekHeadTurnPos5Series, 8, 12, 12, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 506);

		break;

	case 506:
		sendWSMessage_140000(_G(my_walker), 507);
		break;

	case 507:
		series_unload(_ripTrekHeadTurnPos5Series);
		DisposePath(_mcMach->walkPath);
		_mcMach->walkPath = CreateCustomPath(1770, 337, -1);

		ws_custom_walk(_mcMach, 10, 509, true);

		break;

	case 509:
		ws_walk(_G(my_walker), 1751, 329, nullptr, 512, 4, true);

		break;

	case 512:
		digi_preload("204R03", -1);
		_ripTrekLHandTalkPos4Series = series_load("RIP TREK L HAND TALK POS4", -1, nullptr);
		setGlobals1(_ripTrekLHandTalkPos4Series, 1, 5, 5, 5, 0, 6, 9, 6, 9, 1, 10, 17, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 513);

		break;

	case 513:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_190000(_G(my_walker), 20);
		digi_play("204r01", 1, 255, 515, -1);

		break;

	case 515:
		sendWSMessage_130000(_G(my_walker), 516);

		break;

	case 516:
		sendWSMessage_150000(_G(my_walker), 517);

		break;

	case 517:
		series_unload(_ripTrekLHandTalkPos4Series);
		_fieldD4 = 3;
		_fieldDC = 1;
		_field108 = 1;
		kernel_timing_trigger(1, 574, nullptr);
		kernel_timing_trigger(2, 518, nullptr);

		break;

	case 518:
		_fieldD4 = 3;
		_fieldBC_trigger = kernel_trigger_create(520);

		break;

	case 520:
		_fieldD4 = 6;
		_fieldC0_trigger = kernel_trigger_create(520);

		break;

	case 521:
		digi_play("204m01", 1, 255, 522, -1);
		break;

	case 522:
		_fieldD4 = 3;
		_fieldBC_trigger = kernel_trigger_create(523);

		break;

	case 523:
		_ripTrekLHandTalkPos4Series = series_load("RIP TREK L HAND TALK POS4", -1, nullptr);
		setGlobals1(_ripTrekLHandTalkPos4Series, 1, 5, 5, 5, 0, 6, 9, 6, 9, 1, 10, 17, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 524);

		break;

	case 524:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_190000(_G(my_walker), 20);
		digi_play("204r02", 1, 255, 525, -1);

		break;

	case 525:
		sendWSMessage_130000(_G(my_walker), 526);
		break;

	case 526:
		sendWSMessage_150000(_G(my_walker), 527);
		break;

	case 527:
		series_unload(_ripTrekLHandTalkPos4Series);
		_meiShowsRipHerPassesSeries = series_load("MEI SHOWS RIP HER PASSES", -1, nullptr);
		_fieldD4 = 7;
		_fieldCC_trigger = kernel_trigger_create(528);

		break;

	case 528:
		digi_play("204m02", 1, 255, -1, -1);
		kernel_examine_inventory_object("PING PASS", _G(master_palette), 5, 1, 370, 234, -1, nullptr, -1);
		_fieldD4 = 3;
		_fieldBC_trigger = kernel_trigger_create(529);

		break;

	case 529:
		_ripTrekLHandTalkPos4Series = series_load("RIP TREK L HAND TALK POS4", -1, nullptr);
		setGlobals1(_ripTrekLHandTalkPos4Series, 1, 5, 5, 5, 0, 6, 9, 6, 9, 1, 10, 17, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 530);

		break;

	case 530:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_190000(_G(my_walker), 20);
		digi_play("204r03", 1, 255, 531, -1);

		break;

	case 531:
		digi_unload("204R03");
		sendWSMessage_130000(_G(my_walker), 532);

		break;

	case 532:
		sendWSMessage_150000(_G(my_walker), 533);

		break;

	case 533:
		series_unload(_ripTrekLHandTalkPos4Series);
		inv_give_to_player("PASS");
		_fieldD4 = 3;
		_fieldBC_trigger = kernel_trigger_create(535);

		break;

	case 535:
		_fieldD8 = 1;
		_field2C = 1;
		kernel_timing_trigger(5, 537, nullptr);

		break;

	case 536:
		ws_walk(_G(my_walker), 1612, 328, nullptr, 538, 4, true);
		kernel_timing_trigger(240, 537, nullptr);

		break;

	case 537:
		DisposePath(_mcMach->walkPath);
		_mcMach->walkPath = CreateCustomPath(1642, 333, -1);
		ws_custom_walk(_mcMach, 10, -1, true);

		break;

	case 538:
		ws_get_walker_info(_mcMach, &_fieldFC_infoX, &_field100_infoY, &_G(player_info).scale, &_G(player_info).depth, &_G(player_info).facing);
		kernel_timing_trigger(1, (_fieldFC_infoX == 1642) ? 539 : 538, nullptr);

		break;

	case 539:
		_ripTrekLHandTalkPos4Series = series_load("RIP TREK L HAND TALK POS4", -1, nullptr);
		setGlobals1(_ripTrekLHandTalkPos4Series, 1, 5, 5, 5, 0, 6, 9, 6, 9, 1, 10, 17, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 540);

		break;

	case 540:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_190000(_G(my_walker), 20);
		digi_play("204R34", 1, 255, 541, -1);

		break;

	case 541:
		sendWSMessage_130000(_G(my_walker), 542);

		break;

	case 542:
		sendWSMessage_150000(_G(my_walker), 543);

		break;

	case 543:
		digi_play("204M20", 1, 255, 544, -1);

		break;

	case 544:
		setGlobals1(_ripTrekLHandTalkPos4Series, 1, 5, 5, 5, 0, 6, 9, 6, 9, 1, 10, 17, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 545);

		break;

	case 545:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_190000(_G(my_walker), 20);
		digi_play("204R35", 1, 255, 546, -1);

		break;

	case 546:
		digi_play("204R36", 1, 255, 547, -1);

		break;

	case 547:
		sendWSMessage_130000(_G(my_walker), 548);

		break;

	case 548:
		sendWSMessage_150000(_G(my_walker), 549);

		break;

	case 549:
		digi_play("204M21", 1, 255, 550, -1);

		break;

	case 550:
		kernel_timing_trigger(60, 551, nullptr);

		break;

	case 551:
		_fieldE4_walkerDestX = 1663;
		DisposePath(_mcMach->walkPath);
		_mcMach->walkPath = CreateCustomPath(_fieldE4_walkerDestX, 323, -1);
		ws_custom_walk(_mcMach, 2, 555, true);

		break;

	case 555:
		kernel_timing_trigger(1, 630, nullptr);
		kernel_timing_trigger(1, 578, nullptr);

		break;

	case 556:
		player_set_commands_allowed(false);
		ws_walk(_G(my_walker), 1612, 328, nullptr, 558, 9, true);
		kernel_timing_trigger(140, 557);

		break;

	case 557:
		_fieldE4_walkerDestX = 1663;
		DisposePath(_mcMach->walkPath);
		_mcMach->walkPath = CreateCustomPath(1770, 330, _fieldE4_walkerDestX, 323, -1);
		ws_custom_walk(_mcMach, 2, -1, true);

		break;

	case 558:
		ws_get_walker_info(_mcMach, &_fieldFC_infoX, &_field100_infoY, &_G(player_info).scale, &_G(player_info).depth, &_G(player_info).facing);
		kernel_timing_trigger(1, (_fieldFC_infoX == _fieldE4_walkerDestX) ? 559 : 558, nullptr);

		break;

	case 559:
		kernel_timing_trigger(60, 630, nullptr);
		kernel_timing_trigger(1, 578, nullptr);
		break;

	case 567:
		if (_mcMach) {
			_fieldE4_walkerDestX = 1663;
			DisposePath(_mcMach->walkPath);
			_mcMach->walkPath = CreateCustomPath(1720, 337, -1);
			ws_custom_walk(_mcMach, 9, 583, true);
			series_unload(_meiShowsRipHerPassesSeries);
		} else {
			kernel_timing_trigger(5, 567, nullptr);
		}

		break;

	case 569:
		_field14 = 1;
		_field2C = 0;
		player_update_info(_G(my_walker), &_G(player_info));
		ws_hide_walker(_G(my_walker));
		_ripDeltaMachineStateMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, _G(player_info).depth, _field68, triggerMachineByHashCallback, "Rip Absolute Machine State");
		switch (_field10) {
		case 8:
			sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field78_series, 1, 1, 571, _field78_series, 1, 1, 0);
			_safariShadow3Mach = series_place_sprite("SAFARI SHADOW 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			_field14 = 8;
			break;

		case 11:
			_ripTrekLowReachPos2Series = series_load("RIP TREK LOW REACH POS2", -1, nullptr);
			sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 1, 1, 571, _ripTrekLowReachPos2Series, 1, 1, 0);
			_safariShadow3Mach = series_place_sprite("SAFARI SHADOW 2", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			_field14 = 11;

			break;

		case 16:
			_ripTrekTalkerPos3Series = series_load("RIP TREK TALKER POS3", -1, nullptr);
			_safariShadow3Mach = series_place_sprite("SAFARI SHADOW 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekTalkerPos3Series, 1, 1, 571, _ripTrekTalkerPos3Series, 1, 1, 0);
			_field14 = 16;

			break;

		default:
			break;
		}

		break;

	case 570:
		_field14 = 1;
		_field2C = 0;
		_field24_triggerNum = -1;

		player_update_info(_G(my_walker), &_G(player_info));
		ws_hide_walker(_G(my_walker));
		_ripDeltaMachineStateMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, _G(player_info).depth, 0, triggerMachineByHashCallback, "Rip Delta Machine State");

		if (_field10 == 8) {
			sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field78_series, 1, 1, 571, _field78_series, 1, 1, 0);
			_field14 = 8;
		}

		_safariShadow3Mach = series_place_sprite("SAFARI SHADOW 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info.scale), _G(player_info).depth);

		break;

	case 571:
		if (_field14 == 8 && _field10 == 8 && _field18_triggerNum != -1) {
			kernel_trigger_dispatchx(_field18_triggerNum);
			_field18_triggerNum = -1;
		}

		if (_field14 == 9 && _field10 == 9 && _field24_triggerNum != -1) {
			kernel_trigger_dispatchx(_field24_triggerNum);
				_field24_triggerNum = -1;
		}

		if (_field14 == 12 && _field10 == 12 && _field44_triggerNum != -1) {
			kernel_trigger_dispatchx(_field44_triggerNum);
			_field44_triggerNum = -1;
		}

		if (_field14 == 13 && _field10 == 13 && _field48_triggerNum != -1) {
			kernel_trigger_dispatchx(_field48_triggerNum);
			_field48_triggerNum = -1;
		}

		if (_field14 == 11 && _field10 == 11 && _field18_triggerNum != -1) {
			kernel_trigger_dispatchx(_field18_triggerNum);
			_field18_triggerNum = -1;
		}

		if (_field14 == 14 && _field10 == 14 && _field28_triggerNum != -1) {
			kernel_trigger_dispatchx(_field28_triggerNum);
			_field28_triggerNum = -1;
		}

		if (_field14 == 16 && _field10 == 16 && _field18_triggerNum != -1) {
			kernel_trigger_dispatchx(_field18_triggerNum);
			_field18_triggerNum = -1;
		}

		if (_field14 == 21 && _field10 == 21 && _field28_triggerNum != -1) {
			kernel_trigger_dispatchx(_field28_triggerNum);
			_field28_triggerNum = -1;
		}

		if (!_field2C)
			kernel_timing_trigger(1, 572, nullptr);
		else {
			terminateMachine(_ripDeltaMachineStateMach);
			_ripDeltaMachineStateMach = nullptr;
			ws_unhide_walker(_G(my_walker));
			terminateMachine(_safariShadow3Mach);
			_field68 = 0;

			if (_field14 == 16)
				series_unload(_ripTrekTalkerPos3Series);
		}

		break;

	case 572:
		switch (_field14) {
		case 8:
			switch (_field10) {
			case 8:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field78_series, 1, 1, 571, _field78_series, 1, 1, 0);
				_field14 = 8;

				break;

			case 9:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field78_series, 1, 16, 571, _field78_series, 16, 16, 0);
				_field14 = 9;

				break;

			default:
				break;
			}
			break;

		case 9:
			switch (_field10) {
			case 9:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field78_series, 16, 16, 571, _field78_series, 16, 16, 0);
				break;

			case 10:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field78_series, 17, 34, 571, _field78_series, 34, 34, 0);
				_field14 = 8;
				_field10 = 8;

				break;

			default:
				break;
			}

			break;

		case 11:
			switch (_field10) {
			case 11:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 1, 1, 571, _ripTrekLowReachPos2Series, 1, 1, 0);
				break;

			case 12:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 1, 16, 571, _ripTrekLowReachPos2Series, 16, 16, 0);
				_field14 = 12;

				break;

			case 13:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 1, 14, 571, _ripTrekLowReachPos2Series, 14, 14, 0);
				_field14 = 13;

				break;

			case 14:
			case 15:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field90_series, 1, 5, 571, _field90_series, 5, 5, 0);
				_field14 = 14;

				break;

			default:
				break;
			}

			break;

		case 12:
			switch (_field10) {
			case 11:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 16, 1, 571, _ripTrekLowReachPos2Series, 1, 1, 0);
				_field14 = 11;

				break;

			case 12:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 16, 16, 571, _ripTrekLowReachPos2Series, 16, 16, 0);
				break;

			default:
				break;

			}


			break;

		case 13:
			if (_field10 == 11) {
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 14, 1, 571, _ripTrekLowReachPos2Series, 1, 1, 0);
				_field14 = 11;
			} else if (_field10 == 13) {
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekLowReachPos2Series, 14, 14, 571, _ripTrekLowReachPos2Series, 14, 14, 0);
			}

			break;
		case 14:
			switch (_field10) {
			case 11:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field90_series, 13, 19, 571, _field90_series, 19, 19, 0);
				_field14 = 11;

				break;

			case 14:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field90_series, 5, 5, 571, _field90_series, 5, 5, 0);
				break;

			case 15: {
				int32 rnd = imath_ranged_rand(6, 12);
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _field90_series, rnd, rnd, 571, _field90_series, rnd, rnd, 0);
				}

				break;

			default:
				break;
			}

			break;

		case 16:
			switch (_field10) {
			case 16:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekTalkerPos3Series, 1, 1, 571, _ripTrekTalkerPos3Series, 1, 1, 0);
				break;

			case 18:
				_field10 = 16;
				_field18_triggerNum = kernel_trigger_create(605);
				kernel_timing_trigger(1, 571, nullptr);

				break;

			case 19: {
				int32 rnd = imath_ranged_rand(1, 5);
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekTalkerPos3Series, rnd, rnd, 571, _ripTrekTalkerPos3Series, rnd, rnd, 0);
				}

				break;

			case 20:
			case 21:
				_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekHandTalkPos3Series, 1, 7, 571, _ripTrekHandTalkPos3Series, 7, 7, 0);
				_field14 = 21;

				break;

			case 22:
				_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekHandTalkPos3Series, 1, 7, 571, _ripTrekHandTalkPos3Series, 7, 7, 0);
				_field10 = 23;

				break;

			case 23:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekHandTalkPos3Series, 8, 10, -1, _ripTrekHandTalkPos3Series, 8, 10, 4);
				digi_play(conv_sound_to_play(), 1, 255, 571, -1);
				_field10 = 24;

				break;

			case 24:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekHandTalkPos3Series, 9, 16, 571, _ripTrekHandTalkPos3Series, 16, 16, 0);
				_field10 = 25;

				break;

			case 25:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekTalkerPos3Series, 1, 1, 571, _ripTrekTalkerPos3Series, 1, 1, 0);
				series_unload(_ripTrekHandTalkPos3Series);
				_field10 = 16;
				conv_resume(conv_get_handle());

				break;


			default:
				break;
			}

			break;

		case 21:
			switch (_field10) {
			case 16:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekHandTalkPos3Series, 9, 16, 571, _ripTrekHandTalkPos3Series, 16, 16, 0);
				_field14 = 16;
				_field10 = 17;

				break;

			case 17:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekTalkerPos3Series, 1, 1, 571, _ripTrekTalkerPos3Series, 1, 1, 0);
				series_unload(_ripTrekHandTalkPos3Series);
				_field10 = 16;
				_field14 = 16;

				break;

			case 20: {
				int32 rnd = imath_ranged_rand(8, 10);
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekHandTalkPos3Series, rnd, rnd, 571, _ripTrekHandTalkPos3Series, rnd, rnd, 0);
				}
				break;

			case 21:
				sendWSMessage_10000(1, _ripDeltaMachineStateMach, _ripTrekHandTalkPos3Series, 8, 8, 571, _ripTrekHandTalkPos3Series, 8, 8, 0);
				break;

			default:
				break;
			}

		default:
			break;
		}


		break;

	case 574:
	case 576:
	case 577:
	case 578:
	case 579:
	case 580:
	case 581:
	case 582:
	case 583:
	case 584:
	case 588:
	case 590:
	case 592:
	case 593:
	case 594:
	case 596:
	case 597:
	case 603:
	case 604:
	case 605:
	case 606:
	case 607:
	case 608:
	case 609:
	case 611:
	case 612:
	case 613:
	case 614:
	case 615:
	case 616:
	case 619:
	case 620:
	case 621:
	case 622:
	case 623:
	case 624:
	case 625:
	case 626:
	case 629:
	case 630:
	case 631:
	case 632:
	case 633:
	case 634:
	case 635:
	case 636:
		sendWSMessage_150000(_G(my_walker), 637);

		break;

	case 637:
	case 638:
	case 639:
	case 647:
	case 648:
	case 649:
	case 650:
	case 651:
	case 652:
	case 660:
	case 661:
	case 662:
	case 663:
	case 664:
	case 665:
	case 666:
	case 667:
	case 669:
	case 670:
	case 675:
	case 676:
	case 677:
	case 678:
	case 679:
	case 680:
	case 681:
	case 682:
	case 683:
	case 684:
	case 687:
	case 688:
	case 689:
	case 691:
	case 692:
	case 693:
	case 694:
	case 695:
	case 696:
	case 697:
	case 699:
	case 700:
	case 701:
	case 702:
	case 703:
	case 708:
	case 709:
	case 710:
	case 711:
	case 712:
	case 713:
	case 714:
	case 715:
	case 716:
	case 719:
	case 720:
	case 721:
	case 722:
	case 723:
	case 725:
	case 726:
	case 727:
	case 728:
	case 729:
	case 730:
	case 1995:
		break;

	default:
		break;
	}

}

void Room204::initWalkerSeries() {
	ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
	ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1, false);
}

void Room204::addMovingMeiHotspot() {
	HotSpotRec *spot = hotspot_new(_fieldE4_walkerDestX - 10, 273, _fieldE4_walkerDestX + 10, 323);
	hotspot_newVerb(spot, "TALK TO");
	hotspot_newVocab(spot, "MEI CHEN");

	spot->cursor_number = 7;
	spot->feet_x = _fieldE4_walkerDestX + 20;
	spot->feet_y = 333;
	spot->facing = 10;

	_G(currentSceneDef).hotspots = hotspot_add(_G(currentSceneDef).hotspots, spot, true);
}

void Room204::deleteMalletHotspot() {
	HotSpotRec *spot = nullptr;
	for (spot = _G(currentSceneDef).hotspots; spot != nullptr; spot = spot->next) {
		if (scumm_stricmp(spot->vocab, "MALLET")) {
			_G(currentSceneDef).hotspots = hotspot_delete_record(_G(currentSceneDef).hotspots, spot);
			break;
		}
	}
}

void Room204::addLookMalletHotspot() {
	HotSpotRec *spot = hotspot_new(460, 287, 470, 299);
	hotspot_newVerb(spot, "LOOK AT");
	hotspot_newVocab(spot, "MALLET");

	spot->cursor_number = 6;
	spot->feet_x = 473;
	spot->feet_y = 331;
	spot->facing = 11;

	_G(currentSceneDef).hotspots = hotspot_add(_G(currentSceneDef).hotspots, spot, true);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
