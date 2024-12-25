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

#include "m4/riddle/rooms/section2/room205.h"

#include "m4/adv_r/other.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/inventory.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {


static const int16 WALKER_DIRS[] = {200, 201, -1};
static const char *WALKER_NAMES[] = {"kuang walker 10", "kuang walker 6"};
static const int16 SHADOW_DIRS[] = {210, 211, -1};
static const char *SHADOW_NAMES[] = {"kuangs shadow 10", "kuangs shadow 6"};

static const int16 WALKER_DIRS2[] = {240, -1};
static const char *WALKER_NAMES2[] = {"shen guo walker 12"};
static const int16 SHADOW_DIRS2[] = {250, -1};
static const char *SHADOW_NAMES2[] = {"shen guo shadow 12"};

void Room205::preload() {
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room205::init() {
	midi_play("vines", 0, -1, -1, 949);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);
	_ripTrekLowReacherPos5Series = series_load("RIP TREK LOW REACHER POS5", -1, nullptr);
	_ripTrekLowReachPos2Series = series_load("RIP TREK LOW REACH POS2", -1, nullptr);
	_ripGetsShotSeries = series_load("RIP GETS SHOT", -1, nullptr);
	_205GunFireSeries = series_load("205 gun fire", -1, nullptr);

	if (!_G(flags)[V029]) {
		digi_preload("205R01", -1);
		digi_preload("205M02", -1);

		_205rp01Series = series_load("205RP01", -1, nullptr);
		_205mc01Series = series_load("205MC01", -1, nullptr);
		_205mc02Series = series_load("205MC02", -1, nullptr);

		digi_preload("205R11", -1);

		_205rp02Series = series_load("205RP02", -1, nullptr);
	}

	_field198 = 0;
	_field19C = 0;
	_field1A0 = 0;
	_fieldD8 = 0;
	_askUnhideMyWalkerFl = false;
	_fieldE4 = 0;
	series_show("205PRIES", 3840, 16, -1, -1, 0, 100, 0, 0);
	_205LeftEntranceTabletMach = series_show("205 LEFT ENTRANCE TABLET", 257, 16, -1, -1, 0, 100, 0, 0);

	if (!_G(flags)[V028])
		hotspot_set_active(_G(currentSceneDef).hotspots, "MASTER LU'S TABLET", false);

	if (_G(flags)[V024]) {
		_fieldE0 = 1;
		series_show("205GONG", 1025, 16, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "GUN", false);
		digi_preload("205_s34", -1);
		digi_play_loop("205_s34", 3, 25, -1, -1);

		if (!_G(flags)[V028])
			kernel_timing_trigger(7200, 1055, nullptr);
	} else {
		_fieldE0 = 0;
		hotspot_set_active(_G(currentSceneDef).hotspots, "GONG ", false);
		_205FireInBrazierSeries = series_load("205 FIRE IN BRAZIER", -1, nullptr);
		_205FireInBrazierMach = series_play("205 FIRE IN BRAZIER", 1024, 0, -1, 7, -1, 100, 0, 0, 0, -1);
		digi_preload("205_s34", -1);
		digi_play_loop("205_s34", 3, 166, -1, -1);

		if (_G(flags)[V029])
			_205GunInBrazierMach = series_show("205 gun in brazier", 768, 16, -1, -1, 0, 100, 0, 0);
	}

	if (inv_player_has("CHARCOAL")) {
		if (_G(flags)[V029])
			hotspot_set_active(_G(currentSceneDef).hotspots, "CHARCOAL", false);
	} else if (_G(flags)[V029])
		_205CharcoalSpriteMach = series_show("205 CHARCOAL SPRITE", 3840, 16, -1, -1, 0, 100, 0, 0);

	if (_G(flags)[V025]) {
		series_show("205 MALLET LAYED ON GONG", 1024, 16, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "GAP WITH JOURNAL", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MALLET ", false);
	}

	if (_G(flags)[V029]) {
		series_show("205BITS", 3846, 16, -1, -1, 0, 100, 0, 0);
		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 901, nullptr);
	}

	if (_G(flags)[V028]) {
		_205MeiStanderMach = series_show("205 MEI STANDER", 3845, 16, -1, -1, 0, 100, 0, 0);
		series_show("205 TABLETS DOWN", 3840, 16, -1, -1, 0, 100, 0, 0);
		series_show("205GLASS", 3840, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active(_G(currentSceneDef).hotspots, "TABLET ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "RIGHT TABLET", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "SHEN GUO", false);
	} else {
		_205TabletsSeries = series_load("205 TABLETS", -1, nullptr);
		_205TabletsMach = series_show("205 TABLETS", 3584, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active(_G(currentSceneDef).hotspots, "GLASSES", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "FALLEN TABLETS", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
	}

	_G(flags)[V027] = 0;
	_fieldDC = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(false);
		if (_G(flags)[V029]) {
			ws_demand_facing(_G(my_walker), 11);
			ws_demand_location(_G(my_walker), 320, 480);
			ws_walk(_G(my_walker), 317, 356, nullptr, 500, 11, true);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
			ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2);
			_mcEntranceTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 258, 490, 1, triggerMachineByHashCallback3000, "mc_entrance_trek");
			ws_demand_facing(_G(my_walker), 11);
			ws_demand_location(_G(my_walker), 320, 480);
			ws_walk(_G(my_walker), 317, 356, nullptr, 501, 11, true);
			sendWSMessage_10000(_mcEntranceTrekMach, 261, 359, 2, -1, true);

			return;
		}
	}

	if (!_G(flags)[V028])
		kernel_timing_trigger(1, 1000, nullptr);
}

void Room205::pre_parser() {
	if (player_said("GEAR", "RIGHT TABLET") && _G(flags)[V022] && !_G(flags)[V024]) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("EXIT") && _G(flags)[V025]) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room205::parser() {
	bool lookFl = player_said_any("look", "look at");
	bool takeFl = player_said("take");
	bool gearFl = player_said("gear");

	if (player_said("GONG", "BRAZIER") || player_said("GONG", "GUN")) {
		if (!_G(flags)[V024]) {
			switch (_G(kernel).trigger) {
			case -1:
				_fieldDC = 1;
				player_set_commands_allowed(false);
				setGlobals1(_ripTrekMedReachHandPos1Series, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 3);

				break;

			case 3:
				terminateMachine(_205FireInBrazierMach);
				terminateMachine(_205GunPointedMach);
				series_unload(_205FireInBrazierSeries);
				series_show("205GONG", 1025, 16, -1, -1, 0, 100, 0, 0);
				digi_play_loop("205_s34", 3, 25, -1, -1);
				digi_play_loop("205_S23", 1, 255, -1, -1);
				hotspot_set_active(_G(currentSceneDef).hotspots, "GONG ", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "GUN", false);
				inv_move_object("GONG", 999);
				sendWSMessage_120000(_G(my_walker), 5);

				break;

			case 5:
				sendWSMessage_150000(_G(my_walker), 7);

				break;

			case 7:
				player_set_commands_allowed(true);
				_G(flags)[V024] = 1;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(7200, 1055);
				_fieldDC = 0;

				break;

			default:
				break;
			}
		}
	} // if (player_said("GONG", "BRAZIER") || player_said("GONG", "GUN"))
	else if (player_said("MALLET", "GAP WITH JOURNAL") && _G(flags)[V028] && _G(flags)[V024]) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			series_play("205 RIP GETS BOOK WITH MALLET", 256, 0, 1, 7, 0, 100, 0, 0, 0, -1);

			break;

		case 1:
			series_play("205 RIP GETS BOOK WITH MALLET", 256, 2, 3, 7, 0, 100, 0, 0, 0, -1);
			digi_play("205_s32", 1, 255, -1, -1);

			break;

		case 3:
			ws_unhide_walker(_G(my_walker));
			ws_walk(_G(my_walker), 215, 336, nullptr, 5, 11, true);

			break;

		case 5:
			setGlobals1(_ripTrekMedReachHandPos1Series, 1, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 7);

			break;

		case 7:
			digi_play("202_s01", 1, 255, -1, 202);
			sendWSMessage_120000(_G(my_walker), 9);

			break;

		case 9:
			sendWSMessage_150000(_G(my_walker), 11);

			break;

		case 11:
			player_set_commands_allowed(true);
			series_unload(_ripTrekMedReachHandPos1Series);
			series_show("205 MALLET LAYED ON GONG", 1024, 16, -1, -1, 0, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "GAP WITH JOURNAL", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MALLET ", true);
			_G(flags)[V025] = 1;
			inv_give_to_player("JOURNAL");
			inv_move_object("MALLET", 999);

			break;

		default:
			break;

		}
	} // if (player_said("MALLET", "GAP WITH JOURNAL") && _G(flags)[V028] && _G(flags)[V024])
	else if (player_said("CHARCOAL", "JOURNAL") && _G(flags)[V025] && inv_player_has("CHARCOAL")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(_G(my_walker), 328, 253, nullptr, 1, 10, true);
			_205JournalRippedPopupSeries = series_load("205 JOURNAL RIPPED POPUP", -1, nullptr);

			break;
		case 1:
			ws_hide_walker(_G(my_walker));
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_show("SAFARI SHADOW 1", 2304, 16, -1, -1, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y);
			series_play("205 RIP RUBS CHARCOAL", 256, 0, 2, 7, 0, 100, 0, 0, 0, 13);

			break;

		case 2:
			series_play("205 RIP RUBS CHARCOAL", 256, 0, 3, 7, 0, 100, 0, 0, 14, 33);
			digi_play("205_S33", 1, 255, -1, -1);

			break;

		case 3:
			_205JournalCharcoalPopupMach = series_play("205 JOURNAL CHARCOAL POPUP", 256, 16, -1, 1, 0, 100, 0, 0, 0, -1);
			_205MeiSighAndTalkSeries = series_load("205 MEI SIGH AND TALK", -1, nullptr);
			kernel_timing_trigger(180, 5, nullptr);

			break;

		case 5:
			terminateMachine(_205JournalCharcoalPopupMach);
			series_play("205 JOURNAL CHARCOAL POPUP", 256, 2, 7, 1, 0, 100, 0, 0, 0, -1);

			break;

		case 7:
			ws_unhide_walker(_G(my_walker));
			terminateMachine(_safariShadow1Mach);
			terminateMachine(_205MeiStanderMach);
			_205MeiStanderMach = series_play("205 MEI SIGH AND TALK", 3845, 16, 9, 5, 0, 100, 0, 0, 0, 21);
			digi_play("205M06", 1, 255, 15, -1);

			break;

		case 9:
			series_unload(_205JournalRippedPopupSeries);
			kernel_timing_trigger(180, 11);

			break;

		case 11:
			terminateMachine(_205MeiStanderMach);
			series_play("205 MEI SIGH AND TALK", 3845, 0, 13, 5, 0, 100, 0, 0, 22, -1);

			break;

		case 13:
			terminateMachine(_205MeiStanderMach);
			series_play("205 MEI SIGH AND TALK", 3845, 2, 14, 5, 0, 100, 0, 0, 22, 45);

			break;

		case 14:
			series_show("205 MEI SIGH AND TALK", 3845, 16, -1, -1, 22, 100, 0, 0);

			break;

		case 15:
			digi_play("205R58", 1, 255, 20, -1);
			break;

		case 20:
			_G(flags)[V290] = 1;
			disable_player_commands_and_fade_init(21);

			break;
		case 21:
			inv_give_to_player("CHINESE YUAN");
			inv_move_object("CHARCOAL", 999);
			_G(game).new_section = 8;
			_G(game).new_room = 850;

			break;

		default:
			break;
		}

	} // if (player_said("CHARCOAL", "JOURNAL") && _G(flags)[V025] && inv_player_has("CHARCOAL"))

	else if (gearFl && player_said_any("TABLET", "TABLET "))
		digi_play("205R30", 1, 255, -1, -1);
	else if (gearFl && player_said("GONG "))
		digi_play("205R64", 1, 255, -1, -1);
	else if (gearFl && player_said("LEFT TABLET")) {
		if (_G(flags)[V028]) {
			digi_play("205r57", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_fieldE0 = 1;
				kernel_timing_trigger(10, 1, nullptr);

				break;

			case 1:
				if (_fieldDC) {
					kernel_timing_trigger(60, 1, nullptr);
				} else {
					kernel_timing_trigger(10, 2, nullptr);
					_fieldDC = 1;
				}

				break;

			case 2:
				digi_preload("205_S30");
				digi_preload("205R33");
				ws_hide_walker(_G(my_walker));
				terminateMachine(_205LeftEntranceTabletMach);
				series_stream("205LTEST", 7, 0, 7);
				kernel_timing_trigger(240, 5, nullptr);

				break;

			case 5:
				digi_play("205_S30", 1, 255, -1, -1);

				break;

			case 7:
				digi_stop(1);
				digi_unload("205_S30");
				_G(flags)[V023] = 1;
				_205LeftEntranceTabletMach = series_show("205 LEFT ENTRANCE TABLET", 257, 16, -1, -1, 0, 100, 0, 0);
				if (_G(flags)[V022]) {
					digi_preload("205R31");
					digi_play("205R31", 1, 255, 9, -1);
				} else {
					digi_preload("205R32");
					digi_play("205R32", 1, 255, 9, -1);
				}
				ws_unhide_walker(_G(my_walker));

				break;

			case 9:
				digi_unload("205R31");
				digi_unload("205R32");
				digi_play("205R33", 1, 255, 11, -1);

				break;

			case 11:
				digi_unload("205R33");
				_fieldDC = 0;
				_fieldE0 = 0;
				player_set_commands_allowed(true);

				break;

			default:
				break;
			}
		}
	} // if (edi && player_said("LEFT TABLET"))

	else if (gearFl && player_said("RIGHT TABLET")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_fieldE0 = 1;
			kernel_timing_trigger(10, 1, nullptr);

			break;

		case 1:
			if (!_G(flags)[V022] && _fieldDC)
				kernel_timing_trigger(60, 1, nullptr);
			else
				kernel_timing_trigger(10, 3, nullptr);

			break;

		case 3:
			if (!_G(flags)[V022]) {
				_fieldDC = 1;
				digi_preload("205_S30", -1);
				digi_preload("205R34", -1);
				ws_unhide_walker(_G(my_walker));
				series_stream("205RTEST", 7, 0, 7);
				kernel_timing_trigger(240, 5, nullptr);
			} else if (!_G(flags)[V024]) {
				ws_walk(_G(my_walker), 450, 351, nullptr, 15, 1, true);
				intr_cancel_sentence();
			} else {
				_field1A0 = 1;
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "SHEN GUO", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", true);
				player_update_info(_G(my_walker), &_G(player_info));
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, 0, 90, 10, true);
			}
			break;

		case 5:
			digi_play("205_S30", 1, 255, -1, -1);

			break;

		case 7:
			digi_stop(1);
			digi_unload("205_S30");
			if (_G(flags)[V023]) {
				digi_preload("205R32");
				digi_play("205R32", 1, 255, 9, -1);
			} else {
				digi_preload("205R31");
				digi_play("205R31", 1, 255, 9, -1);
			}

			_G(flags)[V022] = 1;
			ws_unhide_walker(_G(my_walker));
			break;

		case 9:
			digi_unload("205R31");
			digi_unload("205R32");
			digi_play("205R34", 1, 255, 11, -1);

			break;

		case 11:
			digi_unload("205R34");
			_fieldDC = 0;
			_fieldE0 = 0;
			player_set_commands_allowed(true);

			break;

		case 15:
			digi_play("205_s29", 1, 255, -1, -1);
			kernel_timing_trigger(1, 20, nullptr);

			break;

		case 20:
			setGlobals1(_ripGetsShotSeries, 14, 32, 32, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 25);

			break;

		case 25:
			kernel_timing_trigger(60, 30, nullptr);
			break;

		case 30:
			disable_player_commands_and_fade_init(35);
			break;

		case 35:
			sendWSMessage_150000(_G(my_walker), 36);
			break;

		case 36:
			_fieldDC = 0;
			_fieldE0 = 0;
			other_save_game_for_resurrection();
			_G(game).new_section = 4;
			_G(game).new_room = 413;

			break;

		case 99:
			_fieldE4 = 1;
			break;

		default:
			break;
		}
	} // if (edi && player_said("RIGHT TABLET"))

	else if (takeFl && player_said("CHARCOAL")) {
		if (!_G(flags)[V028]) {
			digi_play("205r26", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				setGlobals1(_ripTrekLowReacherPos5Series, 1, 25, 25, 25, 0, 25, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 3);

				break;

			case 3:
				kernel_examine_inventory_object("PING CHARCOAL", nullptr, 5, 1, 144, 271, 4, nullptr, -1);

				break;

			case 4:
				sendWSMessage_120000(_G(my_walker), 5);
				inv_give_to_player("CHARCOAL");

				break;

			case 5:
				sendWSMessage_150000(_G(my_walker), 7);

				break;

			case 7:
				series_unload(_ripTrekLowReacherPos5Series);
				player_set_commands_allowed(true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "CHARCOAL", false);

				break;

			default:
				break;
			}
		}
	} // if (esi && player_said("CHARCOAL"))

	else if (takeFl && player_said("GAP WITH JOURNAL") && !_G(flags)[V028])
		digi_play("205r27", 1, 255, -1, -1);
	else if (takeFl && player_said_any("TABLET", "TABLET ", "LEFT TABLET", "RIGHT TABLET"))
		digi_play("205r28", 1, 255, -1, -1);
	else if (takeFl && player_said("BRAZIER"))
		digi_play("205r29", 1, 255, -1, -1);
	else if (takeFl && player_said("GLASSES"))
		digi_play("205r56", 1, 255, -1, -1);
	else if (takeFl && player_said("GUN"))
		digi_play("205r53", 1, 255, -1, -1);
	else if (takeFl && player_said("GONG "))
		digi_play("205r54", 1, 255, -1, -1);
	else if (takeFl && player_said("MALLET "))
		digi_play("205r55", 1, 255, -1, -1);
	else if (lookFl && player_said("JOURNAL") && _G(flags)[V025] && !_showMeiTalkFl) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(_G(my_walker), 328, 253, nullptr, 1, 10, true);
			_205JournalRippedPopupSeries = series_load("205 JOURNAL RIPPED POPUP", -1, nullptr);

			break;

		case 1:
			ws_hide_walker(_G(my_walker));
			player_update_info(_G(my_walker), &_G(player_info));
			series_play("205 RIP SHOWS MEI TORN PAGE", 256, 0, 3, 7, 0, 100, 0, 0, 0, -1);
			_safariShadow1Mach = series_show("SAFARI SHADOW 1", 2304, 16, -1, -1, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y);

			break;

		case 3:
			_205JournalCharcoalPopupMach = series_play("205 JOURNAL RIPPED POPUP", 256, 16, -1, 1, 0, 100, 0, 0, 0, -1);
			digi_play("205r23", 1, 255, 5, -1);

			break;

		case 5:
			terminateMachine(_205JournalCharcoalPopupMach);
			series_play("205 JOURNAL RIPPED POPUP", 256, 2, 7, 1, 0, 100, 0, 0, 0, -1);

			break;

		case 7:
			series_play("205 RIP SHOWS MEI TORN PAGE", 256, 2, 9, 7, 0, 100, 0, 0, 0, -1);

			break;

		case 9:
			ws_unhide_walker(_G(my_walker));
			terminateMachine(_safariShadow1Mach);
			if (_showMeiTalkFl) {
				kernel_timing_trigger(10, 12, nullptr);
			} else {
				terminateMachine(_205MeiStanderMach);
				series_play("205 MEI TALKS", 3845, 0, 11, 7, 0, 100, 0, 0, 0, -1);
				digi_play("205M05", 1, 255, -1, -1);
			}

			break;

		case 11:
			_205MeiStanderMach = series_show("205 MEI TALKS", 3845, 16, -1, -1, 52, 100, 0, 0);
			series_unload(_205JournalRippedPopupSeries);

			_showMeiTalkFl = true;
			player_set_commands_allowed(true);

			break;

		case 12:
			terminateMachine(_205MeiStanderMach);
			_205MeiStanderMach = series_show("205 MEI TALKS", 3845, 16, -1, -1, 52, 100, 0, 0);
			series_unload(_205JournalRippedPopupSeries);

			_showMeiTalkFl = true;
			player_set_commands_allowed(true);

			break;

		default:
			break;

		}
	} // if (ecx && player_said("JOURNAL") && _G(flags)[V025] && !_showMeiTalkFl)

	else if (lookFl && player_said("BRAZIER"))
		digi_play("205r66", 1, 255, -1, -1);
	else if (lookFl && player_said("ARM"))
		digi_play("205r12a", 1, 255, -1, -1);
	else if (lookFl && player_said("GUN")) {
		if (!_G(flags)[V024])
			digi_play("205R15", 1, 255, -1, -1);
		else if (!_G(flags)[V028])
			digi_play("205R16", 1, 255, -1, -1);
		else
			digi_play("205r48", 1, 255, -1, -1);
	} // if (ecx && player_said("GUN"))

	else if (lookFl && player_said("GONG ")) {
		if (_G(flags)[V028])
			digi_play("205R48", 1, 255, -1, -1);
		else if (!_G(flags)[V024])
			digi_play("205R15", 1, 255, -1, -1);
		else
			digi_play("205R16", 1, 255, -1, -1);
	} // if (ecx && player_said("GONG "))

	else if (lookFl && player_said("CHARCOAL") && !inv_player_has("CHARCOAL"))
		digi_play("205R17", 1, 255, -1, -1);
	else if (lookFl && player_said("CHARCOAL") && inv_player_has("CHARCOAL")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			kernel_examine_inventory_object("PING CHARCOAL", _G(master_palette), 5, 1, 270, 150, 1, nullptr, -1);

			break;

		case 1:
			player_set_commands_allowed(true);

			break;

		default:

			break;
		}
	} // if (ecx && player_said("CHARCOAL") && inv_player_has("CHARCOAL"))

	else if (lookFl && player_said("MASTER LU'S TABLET"))
		digi_play("205R02", 1, 255, -1, -1);
	else if (lookFl && player_said_any("TABLET", "TABLET "))
		digi_play("205R18", 1, 255, -1, -1);
	else if (lookFl && player_said_any("RIGHT TABLET", "LEFT TABLET")) {
		if (_G(flags)[V028])
			digi_play("205R18", 1, 255, -1, -1);
		else
			digi_play("205R19", 1, 255, -1, -1);
	} else if (lookFl && player_said("FALLEN TABLETS"))
		digi_play("205R49", 1, 255, -1, -1);
	else if (lookFl && player_said_any("MEI CHEN", "MEI CHEN ")) {
		if (_G(flags)[V028])
			digi_play("205r50", 1, 255, -1, -1);
		else
			digi_play("205r20", 1, 255, -1, -1);
	} else if (lookFl && player_said_any("SHEN GUO", "SHEN GUO "))
		digi_play("205r65", 1, 255, -1, -1);
	else if (lookFl && player_said("KEY"))
		digi_play("205R22", 1, 255, -1, -1);
	else if (lookFl && player_said("GLASSES"))
		digi_play("205r51", 1, 255, -1, -1);
	else if (lookFl && player_said("GAP WITH JOURNAL")) {
		if (_G(flags)[V028])
			digi_play("205R52", 1, 255, -1, -1);
		else
			digi_play("205r27", 1, 255, -1, -1);
	} else if (lookFl && player_said("MALLET "))
		digi_play("205r55", 1, 255, -1, -1);
	else if (lookFl && player_said(" ")) {
		if (_G(flags)[V028])
			digi_play("205R47", 1, 255, -1, -1);
		else
			digi_play("205R14", 1, 255, -1, -1);
	} else if (player_said("journal") && !takeFl && !lookFl) {
		if (_G(flags)[V028])
			digi_play("205R43", 1, 255, -1, -1);
		else
			digi_play("205R42", 1, 255, -1, -1);
	} else if (player_said("EXIT")) {
		if (_G(flags)[V025])
			digi_play("205r59", 1, 255, -1, -1);
		else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(1);

				break;

			case 1:
				_G(game).new_room = 204;

				break;

			default:
				break;
			}
		}
	}


	_G(player).command_ready = 0;
}

void Room205::daemon() {
	switch (_G(kernel).trigger) {
	case 500:
		player_set_commands_allowed(true);
		break;

	case 501:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_hide_walker(_G(my_walker));
		_safariShadow1Mach = series_show("SAFARI SHADOW 1", 2304, 16, -1, -1, 0, _G(player_info).scale + 1, _G(player_info).x, _G(player_info).y);
		series_play("205RP01", 1792, 0, 502, 5, 0, 100, 0, 0, 0, 16);

		break;

	case 502:
		digi_play("205R01", 1, 255, 506, -1);
		series_play("205RP01", 1792, 0, 503, 5, 0, 100, 0, 0, 17, 18);
		ws_hide_walker(_mcEntranceTrekMach);
		_candlemanShadow3Mach = series_show("CANDLEMAN SHADOW3", 2304, 16, -1, -1, 0, 84, 261, 359);
		_205MeiStanderMach = series_play("205MC01", 1792, 16, -1, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 503:
		series_play("205RP01", 1792, 2, 504, 5, 0, 100, 0, 0, 11, 17);
		break;

	case 504:
		_205rp1Mach = series_play("205RP01", 1792, 16, 505, 5, 0, 100, 0, 0, 12, -1);
		break;

	case 505:
	case 526:
	case 538:
	case 544:
	case 552:
	case 588:
	case 592:
		_fieldD8 = 1;
		break;

	case 506:
		if (_fieldD8) {
			_fieldD8 = 0;
			terminateMachine(_205MeiStanderMach);
			series_play("205MC02", 1792, 0, 507, 5, 0, 100, 0, 0, 0, 5);
		} else {
			kernel_timing_trigger(10, 529, nullptr);
		}

		break;

	case 507:
		_205MeiStanderMach = series_play("205MC02", 256, 16, 508, 5, 0, 100, 0, 0, 6, -1);
		digi_play("205M02", 1, 255, -1, -1);

		break;

	case 508:
		terminateMachine(_205rp1Mach);
		terminateMachine(_safariShadow1Mach);
		ws_unhide_walker(_G(my_walker));
		ws_walk(_G(my_walker), 277, 304, nullptr, 509, 9, true);
		digi_play("205R11", 1, 255, -1, -1);

		break;

	case 509:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_hide_walker(_G(my_walker));
		_safariShadow1Mach = series_show("SAFARI SHADOW 3", 2304, 16, -1, -1, 0, _G(player_info).scale + 1, _G(player_info).x, _G(player_info).y);
		_205rp1Mach = series_play("205RP02", 2048, 16, 510, 5, 0, 100, 0, 0, 0, 13);

		break;

	case 510:
		terminateMachine(_safariShadow1Mach);
		terminateMachine(_205rp1Mach);
		_205rp1Mach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 2048, false, triggerMachineByHashCallback, "205rp02");
		sendWSMessage_10000(1, _205rp1Mach, _205rp02Series, 14, 17, -1, _205rp02Series, 14, 17, 1);
		sendWSMessage_1a0000(_205rp1Mach, 11);
		digi_play("205R12a", 1, 255, 512, -1);

		break;

	case 512:
		terminateMachine(_205rp1Mach);
		series_play("205RP02", 2048, 2, 513, 5, 0, 100, 0, 0, 0, 8);
		digi_play("205r12b", 1, 255, -1, -1);

		break;

	case 513:
		ws_unhide_walker(_G(my_walker));
		ws_walk(_G(my_walker), 277, 304, nullptr, 514, 9, true);

		break;

	case 514:
		digi_unload("205R01");
		digi_unload("205M02");
		series_unload(_205rp01Series);
		series_unload(_205mc01Series);

		digi_unload("205R11");
		series_unload(_205rp02Series);

		_ripTrekHeadTurnPos5Series = series_load("RIP TREK HEAD TURN POS5", -1, nullptr);
		digi_preload("205R12", -1);
		_ripTalkerPos5Series = series_load("RIP TALKER POS 5", -1, nullptr);
		_205mc03Series = series_load("205MC03", -1, nullptr);
		digi_preload("205R02", -1);
		_205rp03Series = series_load("205RP03", -1, nullptr);
		kernel_timing_trigger(10, 515, nullptr);

		break;

	case 515:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 516, 5, true);

		break;

	case 516:
		setGlobals1(_ripTrekHeadTurnPos5Series, 1, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 517);

		break;

	case 517:
		sendWSMessage_140000(_G(my_walker), 518);
		digi_play("205R12c", 1, 255, -1, -1);

		break;

	case 518:
		setGlobals1(_ripTrekHeadTurnPos5Series, 8, 12, 12, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 519);

		break;

	case 519:
		sendWSMessage_140000(_G(my_walker), 523);

		break;

	case 520:
		setGlobals1(_ripTrekHeadTurnPos5Series, 1, 1, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 521);

		break;

	case 521:
		sendWSMessage_190000(_G(my_walker), 13);

		break;

	case 522:
		sendWSMessage_150000(_G(my_walker), 523);

		break;

	case 523:
		terminateMachine(_205MeiStanderMach);
		terminateMachine(_candlemanShadow3Mach);
		series_play("205MC03", 1792, 0, 524, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 524:
		ws_unhide_walker(_mcEntranceTrekMach);
		ws_demand_facing(_mcEntranceTrekMach, 2);
		sendWSMessage_10000(_mcEntranceTrekMach, 300, 249, 2, 526, 1);
		ws_walk(_G(my_walker), 349, 249, nullptr, 527, 11, true);

		break;

	case 527:
		ws_hide_walker(_G(my_walker));
		_205rp1Mach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "205rp03");
		sendWSMessage_10000(1, _205rp1Mach, _205rp03Series, 1, 11, -1, _205rp03Series, 2, 10, 4);
		sendWSMessage_1a0000(_205rp1Mach, 11);
		digi_play("205R02", 1, 255, 528, -1);

		break;

	case 528:
		sendWSMessage_10000(1, _205rp1Mach, _205rp03Series, 11, 11, 529, _205rp03Series, 11, 11, 0);

		break;

	case 529:
		if (_fieldD8) {
			_fieldD8 = 0;
			sendWSMessage_60000(_mcEntranceTrekMach);
			kernel_timing_trigger(10, 532, nullptr);
			series_unload(_ripTrekHeadTurnPos5Series);
			series_unload(_ripTalkerPos5Series);
			series_unload(_205mc02Series);

			digi_unload("205R12");
			series_unload(_205mc03Series);
			digi_unload("205R02");

			_205all0Series = series_load("205ALL0", -1, nullptr);
			digi_preload("205_s01", -1);
			digi_preload("205R13", -1);
			terminateMachine(_205rp1Mach);
			series_play("205ALL0", 3840, 0, 530, 5, 0, 100, 0, 0, 0, 32);
		} else {
			kernel_timing_trigger(10, 529, nullptr);
		}

		break;

	case 530:
		digi_play("205_s01", 2, 255, -1, -1);
		series_play("205ALL0", 3840, 0, 531, 5, 0, 100, 0, 0, 33, 56);

		break;

	case 531:
		digi_play("205_s01", 2, 255, -1, -1);
		series_play("205ALL0", 3840, 0, 533, 5, 0, 100, 0, 0, 57, 65);

		break;

	case 532:
		series_unload(S8_SHADOW_DIRS2[4]);
		series_unload(S8_SHADOW_DIRS2[3]);
		series_unload(S8_SHADOW_DIRS2[2]);
		series_unload(S8_SHADOW_DIRS2[1]);
		series_unload(S8_SHADOW_DIRS2[0]);

		break;

	case 533:
		series_play("205ALL0", 3840, 0, 534, 5, 0, 100, 0, 0, 66, -1);
		_G(flags)[V289] = 1;
		digi_play("205R13", 1, 255, 538, -1);

		break;

	case 534:
		if (_fieldD8) {
			_fieldD8 = 0;
			_205all0Mach = series_show("205ALL0", 3840, 16, -1, -1, 88, 100, 0, 0);
			kernel_timing_trigger(10, 539, nullptr);
		} else {
			series_play("205ALL0", 3840, 2, 535, 5, 0, 100, 0, 0, 66, -1);
		}

		break;

	case 535:
		if (_fieldD8) {
			_fieldD8 = 0;
			_205all0Mach = series_show("205ALL0", 3840, 16, -1, -1, 88, 100, 0, 0);
			kernel_timing_trigger(10, 539, nullptr);
		} else {
			series_play("205ALL0", 3840, 16, 536, 5, 0, 100, 0, 0, 66, -1);
		}

		break;

	case 536:
		series_unload(_205rp03Series);
		digi_unload("205_s01");
		ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
		ws_walk_load_walker_series(WALKER_DIRS, WALKER_NAMES);
		digi_preload("205K01", -1);
		_205all1Series = series_load("205ALL1", -1, nullptr);
		digi_preload("205_s02", -1);
		series_load("205BITS", -1, nullptr);
		_205rtlk1Series = series_load("205RTLK1", -1, nullptr);
		_205mtlk1Series = series_load("205MTLK1", -1);
		digi_preload("205M01", -1);
		_205ktlk1Series = series_load("205KTLK1", -1, nullptr);
		digi_preload("205K03", -1);
		digi_preload("205r05", -1);
		digi_preload("205k04", -1);
		_205ktlk2Series = series_load("205KTLK2", -1, nullptr);
		digi_preload("205k05", -1);
		ws_walk_load_shadow_series(SHADOW_DIRS2, SHADOW_NAMES2);
		ws_walk_load_walker_series(WALKER_DIRS2, WALKER_NAMES2);
		digi_preload("205S01", -1);
		digi_preload("205k06", -1);

		kernel_timing_trigger(10, 537, nullptr);

		break;

	case 537:
		if (!_fieldD8) {
			kernel_timing_trigger(10, 537, nullptr);
		} else {
			_fieldD8 = 0;
			kernel_timing_trigger(10, 538, nullptr);
		}

		break;

	case 539:
		_kuangWalker205Mach = triggerMachineByHash_3000(8, 17, *WALKER_DIRS, *SHADOW_DIRS, 1000, 0, 10, triggerMachineByHashCallback3000, "kuang walker 205");
		terminateMachine(_205TabletsMach);
		_205TabletsMach = series_show("205 TABLETS", 0, 16, -1, -1, 0, 100, 0, 0);
		ws_demand_location(_kuangWalker205Mach, 435, 254);
		ws_demand_facing(_kuangWalker205Mach, 10);
		digi_play("205k01", 1, 255, 544, -1);
		kernel_timing_trigger(120, 540, nullptr);
		sendWSMessage_10000(_kuangWalker205Mach, 393, 254, 10, -1, true);

		break;

	case 540:
		terminateMachine(_205all0Mach);
		series_play("205ALL1", 3840, 0, 541, 5, 0, 100, 0, 0, 0, 6);

		break;

	case 541:
		digi_play("205_s02", 2, 255, 542, -1);
		_205all0Mach = series_play("205ALL1", 3840, 16, 545, 5, 0, 100, 0, 0, 7, -1);

		break;

	case 542:
		series_show("205BITS", 3846, 16, -1, -1, 0, 100, 0, 0);
		inv_move_object("KEY", 999);
		player_set_commands_allowed(true);
		player_set_commands_allowed(false);

		break;

	case 543:
		digi_play("205K02", 1, 255, 544, -1);

		break;

	case 545:
		if (!_fieldD8) {
			kernel_timing_trigger(10, 545);
		} else {
			terminateMachine(_205TabletsMach);
			_205TabletsMach = series_show("205 TABLETS", 3584, 16, -1, -1, 0, 100, 0, 0);
			_fieldD8 = 0;
			terminateMachine(_205all0Mach);
			_candlemanShadow3Mach = series_show("CANDLEMAN SHADOW5", 3840, 16, -1, -1, 0, 50, 299, 247);
			_205MeiStanderMach = series_show("205MTLK1", 3840, 16, -1, -1, 0, 100, 0, 0);
			_safariShadow1Mach = series_show("SAFARI SHADOW 4", 3840, 16, -1, -1, 0, 51, 356, 253);
			_205rp1Mach = series_play("205RTLK1", 3840, 16, -1, 5, 0, 100, 0, 0, 0, -1);
			digi_play("205R04", 1, 255, 546);
		}

		break;

	case 546:
		terminateMachine(_205MeiStanderMach);
		_205MeiStanderMach = series_play("205MTLK1", 3840, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		digi_play("205M01", 1, 255, 547, -1);

		break;

	case 547:
		ws_hide_walker(_kuangWalker205Mach);
		_kuangsShadow2Mach = series_show("KUANGS SHADOW 2", 3840, 16, -1, -1, 0, 52, 394, 254);
		_kuangTalkerMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3584, false, triggerMachineByHashCallback, "kuang talker");
		sendWSMessage_10000(1, _kuangTalkerMach, _205ktlk1Series, 1, 1, -1, _205ktlk1Series, 1, 5, 1);
		sendWSMessage_1a0000(_kuangTalkerMach, 11);
		digi_play("205K03", 1, 255, 548, -1);

		break;

	case 548:
		sendWSMessage_10000(1, _kuangTalkerMach, _205ktlk1Series, 1, 1, -1, _205ktlk1Series, 1, 1, 0);
		terminateMachine(_205rp1Mach);
		_205rp1Mach = series_play("205RTLK1", 3840, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		kernel_timing_trigger(10, 553, nullptr);
		digi_play("205R05", 1, 255, 549, -1);

		break;

	case 549:
		sendWSMessage_10000(1, _kuangTalkerMach, _205ktlk1Series, 1, 1, -1, _205ktlk1Series, 1, 5, 1);
		digi_play("205K04", 1, 255, 550, -1);

		break;

	case 550:
		sendWSMessage_10000(1, _kuangTalkerMach, _205ktlk1Series, 1, 1, -1, _205ktlk1Series, 1, 1, 0);
		kernel_timing_trigger(60, 551, nullptr);

		break;

	case 551:
		terminateMachine(_kuangTalkerMach);
		_kuangTalkerMach = series_play("205KTLK2", 3584, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		digi_play("205K05", 1, 255, 552, -1);

		break;


	case 553:
		_shenSuitGunWalkerMach = triggerMachineByHash_3000(8, 18, *WALKER_DIRS2, *SHADOW_DIRS2, 320, 600, 11, triggerMachineByHashCallback3000, "shen suit gun walker");
		sendWSMessage_10000(_shenSuitGunWalkerMach, 346, 293, 11, 554, true);

		break;

	case 554:
		if (!_fieldD8) {
			kernel_timing_trigger(10, 554, nullptr);
		} else {
			_fieldD8 = 0;
			kernel_timing_trigger(10, 555, nullptr);
		}

		break;

	case 555:
		digi_play("205S01", 1, 255, 556, -1);
		digi_unload("205R13");
		series_unload(_205all0Series);
		series_unload(_205all1Series);
		digi_unload("205K01");
		series_unload(_205all1Series);
		digi_unload("205_s02");
		digi_unload("205K02");
		digi_unload("205M01");
		digi_unload("205K03");
		digi_unload("205r05");
		digi_unload("205k04");
		series_unload(_205ktlk2Series);
		digi_unload("205k05");

		break;

	case 556:
		digi_unload("205S01");
		digi_play("205K06", 1, 255, -1, -1);
		digi_preload("205_s05", -1);
		digi_preload("205_s06", -1);
		digi_preload("205_s03", -1);
		digi_preload("205_s04", -1);
		digi_preload("205k07", -1);
		_205all4Series = series_load("205ALL4", -1, nullptr);
		_205rtlk2Series = series_load("205RTLK2", -1, nullptr);
		digi_preload("205r06", -1);
		kernel_timing_trigger(10, 557, nullptr);

		break;

	case 557:
		terminateMachine(_kuangTalkerMach);
		series_unload(_205ktlk1Series);
		terminateMachine(_205rp1Mach);
		series_unload(_205rtlk1Series);
		terminateMachine(_205MeiStanderMach);
		series_unload(_205mtlk1Series);
		terminateMachine(_kuangsShadow2Mach);
		terminateMachine(_safariShadow1Mach);
		terminateMachine(_candlemanShadow3Mach);
		_205all0Mach = series_stream("205ALL2", 7, 3328, 563);
		series_stream_break_on_frame(_205all0Mach, 14, 558);

		break;

	case 558:
		inv_move_object("JOURNAL", 999);
		hotspot_set_active("GAP WITH JOURNAL", true);
		digi_play("205_s05", 2, 255, -1, -1);
		player_set_commands_allowed(true);
		player_set_commands_allowed(false);
		series_stream_break_on_frame(_205all0Mach, 21, 559);

		break;

	case 559:
		digi_play("205_s06", 2, 255, -1, -1);
		series_stream_break_on_frame(_205all0Mach, 26, 560);

		break;

	case 560:
		digi_play("205_s06", 2, 255, -1, 1);
		series_stream_break_on_frame(_205all0Mach, 47, 561);

		break;

	case 561:
		digi_play("205_s03", 2, 255, -1, 1);
		series_stream_break_on_frame(_205all0Mach, 64, 562);

		break;

	case 562:
		digi_play("205_s04", 2, 255, -1, 1);
		series_stream_break_on_frame(_205all0Mach, -1, -1);

		break;

	case 563:
		sendWSMessage_60000(_shenSuitGunWalkerMach);
		_shenGuoShadow12Mach = series_show("SHEN GUO SHADOW 12", 3840, 16, -1, -1, 0, 63, 344, 291);
		_kuangsShadow2Mach = series_show("KUANGS SHADOW 6", 3840, 16, -1, -1, 0, 63, 390, 276);
		_candlemanShadow3Mach = series_show("CANDLEMAN SHADOW5", 3840, 16, -1, -1, 0, 50, 293, 248);
		digi_play("205k07", 1, 255, -1, -1);
		_205all0Mach = series_stream("205ALL3", 5, 3840, 564);

		break;

	case 564:
		terminateMachine(_kuangsShadow2Mach);
		ws_unhide_walker(_kuangWalker205Mach);
		ws_demand_location(_kuangWalker205Mach, 389, 275);
		ws_demand_facing(_kuangWalker205Mach, 7);
		sendWSMessage_10000(_kuangWalker205Mach, 400, 600, 0, 565, true);
		terminateMachine(_shenGuoShadow12Mach);
		_205all0Mach = series_play("205ALL4", 3840, 16, 567, 5, 0, 100, 0, 0, 0, -1);
		digi_unload("205k06");
		digi_unload("205_s05");
		digi_unload("205_s06");
		digi_unload("205_s03");
		digi_unload("205_s04");
		digi_unload("205k07");

		break;

	case 565:
		sendWSMessage_60000(_kuangWalker205Mach);
		kernel_timing_trigger(10, 566, nullptr);

		break;

	case 566:
		series_unload(WALKER_DIRS[0]);
		series_unload(WALKER_DIRS[1]);
		series_unload(SHADOW_DIRS[0]);
		series_unload(SHADOW_DIRS[1]);

		break;

	case 567:
		terminateMachine(_205all0Mach);
		series_unload(_205all4Series);
		_205all0Mach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip talker");
		sendWSMessage_10000(1, _205all0Mach, _205rtlk2Series, 1, 14, -1, _205rtlk2Series, 4, 14, 4);
		digi_play("205r06", 1, 255, 568, -1);

		break;

	case 568:
		digi_unload("205r06");
		sendWSMessage_10000(1, _205all0Mach, _205rtlk2Series, 13, 15, -1, _205rtlk2Series, 15, 15, 0);
		series_unload(WALKER_DIRS2[0]);
		series_unload(SHADOW_DIRS2[0]);
		_205ShenGouStaresSeries = series_load("205 SHEN GOU STARES", -1, nullptr);
		_205all5Series = series_load("205ALL5", -1, nullptr);
		_205rtlk3Series = series_load("205RTLK3", -1, nullptr);
		digi_preload("205r07", -1);
		terminateMachine(_205all0Mach);
		series_unload(_205rtlk2Series);
		terminateMachine(_candlemanShadow3Mach);
		_205all0Mach = series_stream("205 RIP SMILES", 5, 0, 572);

		break;

	case 572:
		_205all0Mach = series_play("205 SHEN GOU STARES", 0, 16, 573, 5, 0, 100, 0, 0, 0, 8);

		break;

	case 573:
		kernel_timing_trigger(60, 574, nullptr);

		break;

	case 574:
		terminateMachine(_205all0Mach);
		series_play("205 SHEN GOU STARES", 0, 0, 575, 5, 0, 100, 0, 0, 9, -1);

		break;

	case 575:
		series_play("205ALL5", 3840, 0, 576, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 576:
		_unkInventoryId = 0;

		digi_play("205r07", 1, 255, 578, -1);
		_candlemanShadow3Mach = series_show("MCTSH1", 3840, 16, -1, -1, 2, 51, 290, 251);
		_205all0Mach = series_play("205RTLK3", 3840, 16, 577, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 577:
		if (INVENTORY_COUNT - 1 <= _unkInventoryId) {
			_fieldD8 = 1;
			break;
		}

		if (inv_player_has(Inventory::get_name(_unkInventoryId))) {
			inv_move_object(Inventory::get_name(_unkInventoryId), 999);
			player_set_commands_allowed(true);
			player_set_commands_allowed(false);
			kernel_timing_trigger(30, 277, nullptr);
		} else {
			kernel_timing_trigger(1, 577, nullptr);
		}
		++_unkInventoryId;

		break;

	case 578:
		if (!_fieldD8) {
			kernel_timing_trigger(10, 578, nullptr);
		} else {
			_fieldD8 = 0;
			series_unload(_205ShenGouStaresSeries);
			series_unload(_205all5Series);
			digi_unload("205r07");
			terminateMachine(_205all0Mach);
			series_unload(_205rtlk3Series);
			terminateMachine(_candlemanShadow3Mach);
			_205GunPointedSeries = series_load("205 GUN POINTED", -1, nullptr);
			_205all0Mach = series_stream("205SRISE", 5, 0, 500);
		}

		break;

	case 580:
		midi_play("suspens2", 255, 0, 581, 949);
		_205GunPointedMach = series_play("205 GUN POINTED", 0, 16, 582, 5, 0, 100, 0, 0, 0, 4);

		break;

	case 581:
		if (!_fieldD8) {
			kernel_timing_trigger(10, 581, nullptr);
		} else {
			_fieldD8 = 0;
			terminateMachine(_205GunPointedMach);
			series_play("205 GUN POINTED", 0, 0, 584, 5, 0, 100, 0, 0, 12, -1);
		}

		break;

	case 582:
		_fieldD8 = 0;

		_205rtlk4Series = series_load("205RTLK4", -1, nullptr);
		digi_preload("205r09", -1);
		_205strlk1Series = series_load("205STLK1", -1, nullptr);
		digi_preload("205s02", -1);
		digi_preload("205r08", -1);

		_fieldD8 = 1;

		break;

	case 583:
		terminateMachine(_205GunPointedMach);
		series_play("205 GUN POINTED", 0, 0, 584, 5, 0, 100, 0, 0, 4, -1);

		break;

	case 584:
		digi_play("205r08", 1, 255, 585, -1);
		_candlemanShadow3Mach = series_show("CANDLEMAN SHADOW3", 3840, 16, -1, -1, 0, 61, 312, 283);
		_205all0Mach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip talker");
		sendWSMessage_10000(1, _205all0Mach, _205rtlk4Series, 1, 10, -1, _205rtlk4Series, 2, 10, 4);

		break;

	case 585:
		digi_play("205r09", 1, 255, 586, -1);

		break;

	case 586:
		series_unload(_205GunPointedSeries);
		terminateMachine(_205all0Mach);
		series_unload(_205rtlk4Series);
		digi_play("205s02", 1, 255, 588, -1);
		_shenSuitGunWalkerMach = series_play("205STLK1", 3840, 16, 589, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 589:
		if (!_fieldD8) {
			kernel_timing_trigger(10, 589, nullptr);
		} else {
			_fieldD8 = 0;
			kernel_timing_trigger(10, 590, nullptr);
		}

		break;

	case 590:
		digi_preload("205r10", -1);
		digi_play("205r10", 1, 255, 593, -1);
		_205rtlk5Series = series_load("205RTLK5", -1, nullptr);
		_205all6Series = series_load("205ALL6", -1, nullptr);
		_205RipGetsBitchSlappedSeries = series_load("205 RIP GETS BITCH SLAPPED", -1, nullptr);
		digi_preload("205_s07", -1);
		digi_preload("205_s08", -1);
		digi_preload("205_s22", -1);
		terminateMachine(_candlemanShadow3Mach);
		terminateMachine(_shenSuitGunWalkerMach);
		series_unload(_205strlk1Series);
		_205all0Mach = series_play("205RTLK5", 3840, 16, 592, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 593:
		if (!_fieldD8) {
			kernel_timing_trigger(10, 593, nullptr);
		} else {
			_fieldD8 = 0;
			terminateMachine(_205all0Mach);
			series_unload(_205rtlk5Series);
			series_play("205ALL6", 768, 0, 594, 5, 0, 100, 0, 0, 0, 10);
		}

		break;

	case 594:
		digi_play("205_s07", 2, 255, -1, -1);
		series_play("205ALL6", 768, 0, 595, 5, 0, 100, 0, 0, 11, 19);

		break;

	case 595:
		digi_play("205_s08", 2, 255, -1, -1);
		series_play("205ALL6", 768, 0, 596, 5, 0, 100, 0, 0, 20, 40);

		break;

	case 596:
		_205CharcoalSpriteMach = series_show("205 CHARCOAL SPRITE", 32767, 16, -1, -1, 0, 100, 0, 0);
		_205GunPointedMach = series_show("205 GUN IN BRAZIER", 768, 16, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "CHARCOAL", true);
		series_play("205ALL6", 768, 0, 897, 5, 0, 100, 0, 0, 41, -1);

		break;

	case 597:
		_205MeiStanderMach = series_show("205 MEI SPRITE TO FILL IN", 3845, 16, -1, -1, 0, 100, 0, 0);
		series_play("205 RIP GETS BITCH SLAPPED", 3584, 0, 598, 5, 0, 100, 0, 0, 0, 11);

		break;

	case 598:
		digi_play("205_s22", 2, 255, -1, -1);
		_205all0Mach = series_play("205 RIP GETS BITCH SLAPPED", 3584, 16, 599, 5, 0, 100, 0, 0, 12, -1);

		break;

	case 599:
		series_unload(_205all6Series);
		digi_unload("205r08");
		digi_unload("205r09");
		digi_unload("205r10");
		digi_unload("205s02");
		digi_unload("205_s07");
		digi_unload("205_s08");
		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 901, nullptr);
		_askUnhideMyWalkerFl = true;
		kernel_timing_trigger(10, 1000, nullptr);

		break;

	case 666:
		other_save_game_for_resurrection();
		_G(game).new_section = 4;
		_G(game).new_room = 413;

		break;

	case 669:
		sendWSMessage_150000(_G(my_walker), 670);

		break;

	case 670:
		_fieldDC = 0;
		_fieldE0 = 0;
		other_save_game_for_resurrection();
		_G(game).new_section = 4;
		_G(game).new_room = 413;

		break;

	case 901:
		if (_fieldE0 || !player_commands_allowed()) {
			kernel_timing_trigger(60, 901, nullptr);
		} else if (_G(flags)[V024]) {
			digi_stop(1);

			switch (imath_ranged_rand(1, 2)) {
			case 1:
				digi_play("205_s24", 1, 255, -1, -1);
				break;

			case 2:
				digi_play("205_s25", 1, 255, -1, -1);
				break;

			default:

				break;
			}
		} else {
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			digi_stop(1);
			series_play("205 gun fire", 768, 0, -1, 3, 0, 100, 0, 0, 0,-1);

			switch (_G(flags)[V027]) {
			case 0:
				digi_play("205_s26", 1, 255, -1, -1);
				kernel_timing_trigger(10, 902, nullptr);

				break;

			case 1:
				digi_play("205_s27", 1, 255, -1, -1);
				kernel_timing_trigger(10, 902, nullptr);

				break;

			case 2:
				digi_play("205_s28", 1, 255, -1, -1);
				kernel_timing_trigger(10, 902, nullptr);

				break;

			case 3:
				kernel_timing_trigger(10, 906, nullptr);

				break;

			default:
				break;
			}
		}

		break;

	case 902:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 903, 1, true);

		break;

	case 903:
		setGlobals1(_ripGetsShotSeries, 1, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 904);

		break;

	case 904:
		sendWSMessage_140000(_G(my_walker), 905);

		break;

	case 905:
		_fieldDC = 0;
		++_G(flags)[V027];
		player_set_commands_allowed(true);
		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 901, nullptr);

		break;

	case 906:
		player_update_info(_G(my_walker), &_G(player_info));
		if (_G(player_info).x < 170) {
			ws_walk(_G(my_walker), 190, _G(player_info).y, nullptr, 907, 1, true);
		} else if (_G(player_info).x <= 470) {
			kernel_timing_trigger(10, 907, nullptr);
		} else {
			ws_walk(_G(my_walker), 450, _G(player_info).y, nullptr, 907, 1, true);
		}

		break;

	case 907:
		digi_stop(1);
		midi_stop();
		digi_play("205_s29", 1, 255, -1, -1);
		setGlobals1(_ripGetsShotSeries, 14, 32, 32, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 908);

		break;

	case 908:
		kernel_timing_trigger(60, 909, nullptr);

		break;

	case 909:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(910);

		break;

	case 910:

		sendWSMessage_150000(_G(my_walker), 666);
		break;

	case 1000:
		_G(flags)[V029] = 1;
		midi_play("vines", 191, -1, -1, 949);
		digi_preload("205_s09", -1);
		digi_preload("205_s10", -1);
		digi_preload("205_s11", -1);
		digi_preload("205_s13", -1);
		digi_preload("205_s14", -1);
		digi_preload("205_s15", -1);
		digi_preload("205_s16", -1);
		digi_preload("205_s17", -1);
		digi_preload("205_s18", -1);
		digi_preload("205_s19", -1);
		digi_preload("205_s20", -1);
		digi_preload("205_s21", -1);
		digi_preload("205_s24", -1);
		digi_preload("205_s25", -1);
		digi_preload("205_s26", -1);
		digi_preload("205_s27", -1);
		digi_preload("205_s28", -1);
		digi_preload("205_s29", -1);
		_205Fite1Series = series_load("205FITE1", -1, nullptr);
		_205Fite2Series = series_load("205FITE2", -1, nullptr);

		if (_askUnhideMyWalkerFl) {
			_askUnhideMyWalkerFl = false;
			ws_unhide_walker(_G(my_walker));
			ws_demand_location(_G(my_walker), 340, 284);
			ws_demand_facing(_G(my_walker), 1);
			player_set_commands_allowed(true);
		}

		terminateMachine(_205all0Mach);
		series_unload(_205RipGetsBitchSlappedSeries);
		terminateMachine(_205MeiStanderMach);
		series_play("205FITE1", 3845, 0, 1001, 5, 0, 100, 0, 0, 0, 15);

		break;

	case 1001:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE1", 3845, 0, 1002, 5, 0, 100, 0, 0, 16, 29);

		break;

	case 1002:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE1", 3845, 0, 1003, 5, 0, 100, 0, 0, 30, 41);

		break;

	case 1003:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		_205all0Mach = series_play("205FITE1", 3845, 16, 1004, 5, 0, 100, 0, 0, 42, -1);

		break;

	case 1004:
		terminateMachine(_205all0Mach);
		series_unload(_205Fite1Series);
		series_play("205FITE2", 3845, 1005, 5, 0, 100, 0, 0, 0, 29);

		break;

	case 1005:
		digi_play("205_s14", 2, 255, -1, -1);
		series_play("205FITE2", 3845, 0, 1006, 5, 0, 100, 0, 0, 30, 54);

		break;

	case 1006:
		digi_play("205_s14", 2, 255, -1, -1);
		series_play("205FITE2", 3845, 0, 1007, 5, 0, 100, 0, 0, 55, 66);

		break;

	case 1007:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE2", 3845, 0, 1008, 5, 0, 100, 0, 0, 67, 80);

		break;

	case 1008:
		digi_play("205_s11", 2, 255, -1, -1);
		series_play("205FITE2", 3845, 0, 1009, 5, 0, 100, 0, 0, 81, 91);

		break;

	case 1009:
		digi_play("205_s13", 2, 255, -1, -1);
		series_play("205FITE2", 3845, 0, 1010, 5, 0, 100, 0, 0, 92, 108);

		break;

	case 1010:
		digi_play("205_s11", 2, 255, -1, -1);
		series_play("205FITE2", 3845, 0, 1011, 5, 0, 100, 0, 0, 109, 131);

		break;

	case 1011:
		digi_play("205_s14", 2, 255, -1, -1);
		_205all0Mach = series_play("205FITE2", 3845, 16, 1013, 5, 0, 100, 0, 0, 132, -1);

		break;

	case 1013:
		if (_fieldDC) {
			kernel_timing_trigger(60, 1013, nullptr);
		} else {
			_fieldDC = 1;
			_205Fite3Series = series_load("205FITE3", -1, nullptr);
			terminateMachine(_205all0Mach);
			series_unload(_205Fite2Series);
			_205all0Mach = series_show("205FITE3", 3845, 16, -1, -1, 0, 100, 0, 0);
			_205Fite4Series = series_load("205FITE4", -1, nullptr);
			_fieldDC = 0;

			kernel_timing_trigger(10, 1015, nullptr);
		}

		break;

	case 1014:
		_fieldE4 = 1;

		break;

	case 1015:
		if (!_field1A0) {
			kernel_timing_trigger(10, 1017, nullptr);
		} else if (_fieldDC) {
			kernel_timing_trigger(60, 1015, nullptr);
		} else if (!_fieldE4) {
			kernel_timing_trigger(10, 1015, nullptr);
		} else {
			_fieldDC = 1;
			series_unload(_205Fite4Series);
			_205all7Series = series_load("205ALL7", -1, nullptr);
			_fieldDC = 0;
			ws_walk(_G(my_walker), 482, 351, 0, 1016, 3, true);
		}

		break;

	case 1016:
		ws_hide_walker(_G(my_walker));
		terminateMachine(_205all0Mach);
		series_unload(_205Fite3Series);
		series_play("205ALL7", 256, 0, 1037, 7, 0, 100, 0, 0, 9, -1);

		break;

	case 1017:
		terminateMachine(_205all0Mach);
		series_play("205FITE3", 3845, 0, 1018, 5, 0, 100, 0, 0, 0, 13);

		break;

	case 1018:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE3", 3845, 0, 1019, 5, 0, 100, 0, 0, 14, 26);

		break;

	case 1019:
		digi_play("205_s14", 2, 255, -1);
		series_play("205FITE3", 3845, 0, 1020, 5, 0, 100, 0, 0, 27, 36);

		break;

	case 1020:
		switch (imath_ranged_rand(1, 8)) {
		case 1:
			digi_play("205_s15", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s16", 2, 255, -1, -1);
			break;

		case 3:
			digi_play("205_s17", 2, 255, -1, -1);
			break;

		case 4:
			digi_play("205_s18", 2, 255, -1, -1);
			break;

		case 5:
			digi_play("205_s19", 2, 255, -1, -1);
			break;

		case 6:
			digi_play("205_s20", 2, 255, -1, -1);
			break;

		case 7:
			digi_play("205_s21", 2, 255, -1, -1);
			break;

		case 8:
			digi_play("205_s11", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE3", 3845, 0, 1021, 5, 0, 100, 0, 0, 37, 45);
		break;

	case 1021:
		switch (imath_ranged_rand(1, 8)) {
		case 1:
			digi_play("205_s15", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s16", 2, 255, -1, -1);
			break;

		case 3:
			digi_play("205_s17", 2, 255, -1, -1);
			break;

		case 4:
			digi_play("205_s18", 2, 255, -1, -1);
			break;

		case 5:
			digi_play("205_s19", 2, 255, -1, -1);
			break;

		case 6:
			digi_play("205_s20", 2, 255, -1, -1);
			break;

		case 7:
			digi_play("205_s21", 2, 255, -1, -1);
			break;

		case 8:
			digi_play("205_s11", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		_205all0Mach = series_play("205FITE3", 3845, 16, 1021, 5, 0, 100, 0, 0, 46, -1);
		break;

	case 1022:
		if (!_field198) {
			kernel_timing_trigger(10, 1027, nullptr);
		} else if (_fieldDC) {
			player_set_commands_allowed(false);
			_fieldE0 = 1;
			kernel_timing_trigger(60, 1022);
		} else {
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			series_unload(_205Fite4Series);

			_fieldDC = 1;
			_fieldE0 = 1;
			series_unload(_205Fite3Series);
			terminateMachine(_205all0Mach);
			series_play("205KILLM", 3845, 0, 1023, 7, 0, 100, 0, 0, 0, -1);
		}

		break;

	case 1023:
		midi_stop();
		digi_play("205m02", 1, 255, -1, -1);
		ws_walk(_G(my_walker), 339, 285, nullptr, 1024, 1, true);

		break;

	case 1024:
		series_play("205KILLR", 3845, 16, -1, 7, 0, 100, 0, 0, 0, -1);
		setGlobals1(_ripGetsShotSeries, 1, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1026);

		break;

	case 1026:
		disable_player_commands_and_fade_init(669);
		break;

	case 1027:
		terminateMachine(_205all0Mach);
		series_play("205FITE4", 3845, 0, 1028, 7, 0, 100, 0, 0, 0, 27);

		break;

	case 1028:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE4", 3845, 0, 1029, 5, 0, 100, 0, 0, 28, 35);

		break;

	case 1029:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE4", 3845, 0, 1030, 5, 0, 100, 0, 0, 36, 47);

		break;

	case 1030:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			digi_play("205_s09", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s10", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE4", 3845, 0, 1031, 5, 0, 100, 0, 0, 48, 62);

		break;

	case 1031:
		digi_play("205_s14", 2, 255, -1, -1);
		series_play("205FITE4", 3845, 0, 1032, 5, 0, 100, 0, 0, 63, 70);

		break;

	case 1032:
		digi_play("205_s14", 2, 255, -1, -1);
		series_play("205FITE4", 3845, 0, 1033, 5, 0, 100, 0, 0, 71, 82);

		break;

	case 1033:
		switch (imath_ranged_rand(1, 8)) {
		case 1:
			digi_play("205_s15", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s16", 2, 255, -1, -1);
			break;

		case 3:
			digi_play("205_s17", 2, 255, -1, -1);
			break;

		case 4:
			digi_play("205_s18", 2, 255, -1, -1);
			break;

		case 5:
			digi_play("205_s19", 2, 255, -1, -1);
			break;

		case 6:
			digi_play("205_s20", 2, 255, -1, -1);
			break;

		case 7:
			digi_play("205_s21", 2, 255, -1, -1);
			break;

		case 8:
			digi_play("205_s11", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE4", 3845, 0, 1034, 5, 0, 100, 0, 0, 83, 86);

		break;

	case 1034:
		switch (imath_ranged_rand(1, 8)) {
		case 1:
			digi_play("205_s15", 2, 255, -1, -1);
			break;

		case 2:
			digi_play("205_s16", 2, 255, -1, -1);
			break;

		case 3:
			digi_play("205_s17", 2, 255, -1, -1);
			break;

		case 4:
			digi_play("205_s18", 2, 255, -1, -1);
			break;

		case 5:
			digi_play("205_s19", 2, 255, -1, -1);
			break;

		case 6:
			digi_play("205_s20", 2, 255, -1, -1);
			break;

		case 7:
			digi_play("205_s21", 2, 255, -1, -1);
			break;

		case 8:
			digi_play("205_s11", 2, 255, -1, -1);
			break;

		default:
			break;
		}

		series_play("205FITE4", 3845, 0, 1035, 5, 0, 100, 0, 0, 87, 106);

		break;

	case 1035:
		digi_play("205_s14", 2, 255, -1, -1);
		series_play("205FITE4", 3845, 0, 1036, 5, 0, 100, 0, 0, 107, -1);

		break;

	case 1036:
		_205all0Mach = series_show("205FITE3", 3845, 16, -1, -1, 0, 100, 0, 0);
		kernel_timing_trigger(10, 1015, nullptr);

		break;

	case 1037:
		_fieldDC = 1;
		terminateMachine(_205TabletsMach);
		series_unload(_205TabletsSeries);
		series_unload(_205all7Series);
		digi_preload("205_s31", -1);
		digi_preload("205_s31a", -1);
		_205rp1Mach = series_stream("205 TABLETS FALL", 7, 0, 1040);
		series_stream_break_on_frame(_205rp1Mach, 20, 1038);

		break;

	case 1038:
		digi_play("205_S31", 1, 255, -1, -1);
		series_stream_break_on_frame(_205rp1Mach, 55, 1039);

		break;

	case 1039:
		midi_stop();
		digi_play("205_S31a", 1, 255, -1, -1);

		break;

	case 1040:
		terminateMachine(_205rp1Mach);
		ws_unhide_walker(_G(my_walker));
		ws_demand_facing(_G(my_walker), 1);
		ws_demand_location(_G(my_walker), 494, 351);
		_205MeiStanderMach = series_show("205 MEI STANDER" ,3584, 16, -1, -1, 0, 100, 0, 0);
		_candlemanShadow3Mach = series_show("CANDLEMAN SHADOW5", 3840, 16, -1, -1, 0, 50, 314, 248);
		series_show("205 TABLETS DOWN", 3840, 16, -1, -1, 0, 100, 0, 0);
		series_show("205GLASS", 3840, 16, -1, -1, 0, 100, 0, 0);
		kernel_timing_trigger(180, 1041, nullptr);

		break;

	case 1041:
		ws_walk(_G(my_walker), 324, 252, nullptr, 1042, 11, true);
		_G(flags)[V028] = 1;

		break;

	case 1042:
		ws_hide_walker(_G(my_walker));
		terminateMachine(_205MeiStanderMach);
		_safariShadow1Mach = series_show("SAFARI SHADOW 2", 3840, 16, -1, -1, 0, 51, 324, 252);
		series_play("205ALL8", 256, 0, 1043, 7, 0, 100, 0, 0, 0, -1);
		digi_play("205M03", 1, 255, 1044, -1);

		break;

	case 1043:
		_205all0Mach = series_show("205ALL8", 256, 16, -1, -1, 24, 100, 0, 0);

		break;

	case 1044:
		digi_play("205R44", 1, 255, 1045, -1);

		break;

	case 1045:
		terminateMachine(_205all0Mach);
		terminateMachine(_safariShadow1Mach);
		_205all9Mach = series_play("205ALL9", 256, 16, 1046, 7, 0, 100, 0, 0, 0, 24);

		break;

	case 1046:
		kernel_timing_trigger(90, 1047);

		break;

	case 1047:
		terminateMachine(_205all9Mach);
		series_play("205ALL9", 256, 0, 1048, 7, 0, 10, 0, 0, 25, -1);

		break;

	case 1048:
		_safariShadow1Mach = series_show("SAFARI SHADOW 3", 3840, 16, -1, -1, 0, 53, 304, 257);
		_205MeiStanderMach = series_play("205MTLK2", 1792, 16, -1, 7, 0, 100, 0, 0, 0, -1);
		digi_play("205M04", 1, 255, 1049, -1);

		break;

	case 1049:
		terminateMachine(_205MeiStanderMach);
		_205rp1Mach = series_play("205RTLK6", 256, 16, -1, 7, 0, 100, 0, 0, 0, -1);
		digi_play("205R45", 1, 255, 1050, -1);

		break;

	case 1050:
		terminateMachine(_205rp1Mach);
		_205MeiStanderMach = series_show("205 MEI STANDER", 3845, 16, -1, -1, 0, 100, 0, 0);
		terminateMachine(_safariShadow1Mach);
		ws_unhide_walker(_G(my_walker));
		ws_demand_location(_G(my_walker), 302, 257);
		ws_demand_facing(_G(my_walker), 3);
		ws_walk(_G(my_walker), 382, 265, nullptr, 1051, 2, true);

		break;

	case 1051:
		setGlobals1(_ripTrekLowReachPos2Series, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1052);

		break;

	case 1052:
		digi_play("205R46", 1, 255, 1053, -1);

		break;

	case 1053:
		sendWSMessage_120000(_G(my_walker), 1054);

		break;

	case 1054:
		sendWSMessage_150000(_G(my_walker), -1);
		hotspot_set_active(_G(currentSceneDef).hotspots, "TABLET ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "RIGHT TABLET", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "FALLEN TABLETS", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "GLASSES", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MASTER LU'S TABLET", true);
		player_set_commands_allowed(true);
		_fieldDC = 0;

		break;

	case 1055:
		_field198 = 1;

		break;

	default:

		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
