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

#include "m4/riddle/rooms/section2/room207.h"
#include "m4/riddle/rooms/section2/section2.h"

#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/other.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room207::preload() {
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room207::init() {
	if (_G(flags[V061]) == 0)
		kernel_load_variant("207lock1");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_node1Entry2Fl = false;
		_dollarFl = false;
		_digiName = nullptr;
		_dispatchTriggerNum = -1;
		_field88 = 2;
		_nextTriggerMode = KT_DAEMON;
		_hiddenWalkerFl = false;
		_fieldA4 = false;
		_commandNotAllowedFl = false;
		_fieldA8 = 0;
		_fieldAA = false;
		_fieldAE_rnd = 1;
		_fieldB2 = false;
	}

	_fieldB6_counter = 0;
	digi_preload("950_s02", -1);
	digi_preload("950_s03", -1);
	digi_preload("950_s04", -1);
	_peskyBegLoopSeries = series_load("pesky beg loop", -1, nullptr);

	if (inv_player_has("LEAD PIPE")) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "PIPES", false);
	} else {
		_leadPipeSeries = series_load("lead pipe", -1, nullptr);
		_leadPipeMach = series_show_sprite("lead pipe", 0, 3840);
	}

	_oneFrameMoneySpriteSeries = series_load("one frame money sprite", -1, nullptr);
	if (_G(flags[V058]) == 0) {
		_metalRimSeries = series_load("metal rim", -1, nullptr);
		_sevenSpokesSeries = series_load("SEVEN spokes", -1, nullptr);
		_woodenRimSeries = series_load("wooden rim", -1, nullptr);

		hotspot_set_active(_G(currentSceneDef).hotspots, "metal rim", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "SEVEN spokes", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "wooden rim", false);

		_wheelSeries = series_load("wheel", -1, nullptr);
		_wheelMach = series_show_sprite("wheel", 0, 3840);

	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "WHEEL", false);
		if (inv_player_has("METAL RIM")) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "METAL RIM", false);
		} else {
			_metalRimSeries = series_load("metal rim", -1, nullptr);
			_metalRimMach = series_show_sprite("metal rim", 0, 3840);
		}

		if (inv_object_is_here("SEVEN SPOKES")) {
			_sevenSpokesSeries = series_load("SEVEN spokes", -1, nullptr);
			_sevenSpokesMach = series_show_sprite("SEVEN spokes", 0, 3840);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "SEVEN SPOKES", false);
		}

		if (inv_player_has("WOODEN RIM")) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "WOODEN RIM", false);
		} else {
			_woodenRimSeries = series_load("wooden rim", -1, nullptr);
			series_show_sprite("wooden rim", 0, 3840);
		}
	}

	if (inv_object_is_here("REBUS AMULET")) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "REBUS AMULET", false);
	} else {
		_rebusAmuletSeries = series_load("rebus amulet", -1, nullptr);
		_rebusAmuletMach = series_show_sprite("rebus amulet", 0, 3840);

		if (_G(flags[V063]) == 0)
			hotspot_set_active(_G(currentSceneDef).hotspots, "REBUS AMULET", false);
	}

	if (_G(flags[V061]) == 0) {
		_peasantSquatTo9Series = series_load("peasant squat to 9", -1, nullptr);
		_shadowPeskyFrom9ToSquatSeries = series_load("shadow pesky from 9 to squat", -1, nullptr);
		_peskyRockLoopSeries = series_load("pesky rock loop", -1, nullptr);
		_ppShowsAmuletTakesMoneySeries = series_load("pp shows amulet takes money", -1, nullptr);
		_withoutAmuletSeries = series_load("without amulet", -1, nullptr);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "PEASANT", false);
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		ws_demand_location(_G(my_walker), 375, 347);
		ws_demand_facing(_G(my_walker), 10);
		if (_G(flags[V061]) == 0) {
			player_set_commands_allowed(false);
			_peasant3ToSquatSeries = series_load("peasant 3 to squat", -1, nullptr);
			ws_walk_load_walker_series(S2_DIRS1, S2_NAMES1, false);
			ws_walk_load_walker_series(S2_DIRS2, S2_NAMES2, false);
			_ppWalkerMach = triggerMachineByHash_3000(8, 6, *S2_DIRS1, *S2_DIRS2, 660, 367, 9, triggerMachineByHashCallback3000, "pp walker");
			sendWSMessage_10000(_ppWalkerMach, 562, 359, 9, 10, 1);
		}

	} else { // KERNEL_RESTORING_GAME
		series_play_xy("shadow pesky rock loop", -1, 0, 514, 367, 97, 3849, 300, -1);
		_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 614, 367, 97, 3840, false, triggerMachineByHashCallback, "pp squat");

		if (inv_player_has("LEAD PIPE")) {
			_peskyPointsRipForegroundSeries = series_load("pesky points at rip foreground", -1, nullptr);
			sendWSMessage_10000(1, _ppSquatMach, 1, _peskyPointsRipForegroundSeries, 19, 100, _peskyPointsRipForegroundSeries, 19, 19, 0);
		} else if (inv_player_has("METAL RIM")) {
			_peskyPointsRipBackgroundSeries = series_load("pesky points at rip background", -1, nullptr);
			sendWSMessage_10000(1, _ppSquatMach, 1, _peskyPointsRipBackgroundSeries, 19, 100, _peskyPointsRipBackgroundSeries, 19, 19, 0);
		} else
			sendWSMessage_10000(1, _ppSquatMach, 1, _peskyRockLoopSeries, 20, 101, _peskyRockLoopSeries, 20, 20, 0);
	}

	digi_play("950_s02", 3, 30, -1, -1);
	kernel_timing_trigger(1500, 1, nullptr);
}

void Room207::pre_parser() {
	bool ecx = player_said("take");
	bool esi = player_said_any("look", "look at");

	if (esi && player_said(" ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (_fieldAA) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		if (!_ripForegroundFl) {
			if (player_said(" ") && player_said_any("exit", "walk to")) {
				other_save_game_for_resurrection();
				player_set_commands_allowed(false);
				_G(kernel).trigger_mode = KT_PARSE;
				disable_player_commands_and_fade_init(203);
				_G(flags[V067]) = 1;
			} else if (player_said("take", "METAL RIM") && inv_player_has("METAL RIM")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said("take", "PIPES") && inv_object_is_here("LEAD PIPE")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said("take", "SEVEN SPOKES") && inv_object_is_here("SEVEN SPOKES")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said("take", "REBUS AMULET") && inv_object_is_here("REBUS AMULET")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said(" ", "METAL RIM")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "METAL RIM");
				_fieldB2 = false;
			}
		}

		if (_ripForegroundFl) {
			if (player_said(" ") && player_said_any("exit", "walk to")) {
				other_save_game_for_resurrection();
				player_set_commands_allowed(false);
				_G(kernel).trigger_mode = KT_PARSE;
				disable_player_commands_and_fade_init(203);
				_G(flags[V067]) = 1;
			} else if (player_said("take", "PIPES") && inv_player_has("LEAD PIPE")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said("take", "METAL RIM") && inv_object_is_here("METAL RIM")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said(" ", "WHEEL")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "METAL RIM");
			} else if (player_said("take", "SEVEN SPOKES") && inv_object_is_here("SEVEN SPOKES")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said("take", "REBUS AMULET") && inv_object_is_here("REBUS AMULET")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "SPLEEN");
			} else if (player_said("LEAD PIPE", "PIPES") || player_said("LEAD PIPE", " ")) {
				Common::strcpy_s(_G(player).verb, "take");
				Common::strcpy_s(_G(player).noun, "pipes");
				_fieldB2 = false;
			}

		}
	} // if (_fieldAA)

	if (ecx && player_said("PIPES") && inv_object_is_here("LEAD PIPE") && _G(flags[V061]) == 0) {
		_G(player).need_to_walk = false;
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 1, nullptr);
	}

	if (ecx && player_said("REBUS AMULET") && inv_object_is_here("REBUS AMULET") && _G(flags[V061]) != 0) {
		_G(player).need_to_walk = false;
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 1, nullptr);
	}

	if (player_said("CHINESE YUAN", "PEASANT") || player_said("US DOLLARS", "PEASANT")) {
		if (!inv_object_is_here("REBUS AMULET")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
			Common::strcpy_s(_G(player).verb, "handout");
		} else if (_G(flags[V063]) == 0) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
			Common::strcpy_s(_G(player).verb, "handout");
		} else {
			if (player_said("US DOLLARS"))
				_dollarFl = true;

			Common::strcpy_s(_G(player).verb, "give");
			Common::strcpy_s(_G(player).noun, "money");
			_G(player).need_to_walk = false;
		}
	}
}

void Room207::parser() {
	bool esi = player_said_any("look", "look at");
	bool talkFl = player_said_any("talk", "talk to");
	bool ecx = player_said("take");
	bool edi = player_said_any("push", "pull", "gear", "open", "close");

	if (esi && player_said_any("anvil", "forge"))
		_G(flags[V036] = 1);

	if (_G(kernel).trigger == 747) {
		player_set_commands_allowed(false);

		if (!_node1Entry2Fl) {
			_field92 = 0;
			_fieldC2 = 3;
		} else {
			_field92 = 6;
			_commandNotAllowedFl = true;
		}
	} // if (_G(kernel).trigger == 747)

	else if (player_said("conv203d"))
		convHandler();
	else if (_G(kernel).trigger == 203)
		_G(game).new_room = 203;
	else if (talkFl && player_said("peasant")) {
		player_set_commands_allowed(false);
		player_update_info(_G(my_walker), &_G(player_info));
		_safariShadow3Mach = series_place_sprite("safari shadow 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
		ws_hide_walker(_G(my_walker));
		_ripHeadDownTalkOffTd33Series = series_load("rip head down talk off td33", -1, nullptr);
		_ripTrekArmsXPos3Series = series_load("rip trek arms x pos3", -1, nullptr);
		_ripInConvMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 1024, 0, triggerMachineByHashCallback, "rip in conv");
		_G(kernel).trigger_mode = KT_DAEMON;
		_fieldC6 = false;
		_fieldC2 = 1;
		sendWSMessage_10000(1, _ripInConvMach, _ripTrekArmsXPos3Series, 1, 15, 23, _ripTrekArmsXPos3Series, 15, 15, 0);
		_field92 = 13;
	} // if (talkFl && player_said("peasant"))

	else if (player_said("handout")) {
		switch (_G(kernel).trigger) {
		case 1:
			player_set_commands_allowed(false);
			ws_walk(_G(my_walker), 498, 380, nullptr, 2, 1, true);

			break;

		case 2:
			_field92 = 16;
			_ripTrekLowReacherPos1Series = series_load("rip trek low reacher pos1", -1, nullptr);

			break;

		case 3:
			player_update_info(_G(my_walker), &_G(player_info));
			ws_hide_walker(_G(my_walker));
			_safariShadow3Mach = series_place_sprite("safari shadow 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			series_ranged_play_xy("rip trek low reacher pos1", 1, 0, 0, 9, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 4, false);

			break;

		case 4:
			series_ranged_play_xy("rip trek low reacher pos1", 1, 2, 0, 9, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 6, 6, false);
			_field92 = 0;

			break;

		case 5:
			ws_unhide_walker(_G(my_walker));
			terminateMachine(_safariShadow3Mach);
			ws_unhide_walker(_G(my_walker));
			series_unload(_ripTrekLowReacherPos1Series);

			break;

		default:
			break;
		}
	} // if (player_said("handout"))

	else if (player_said("give", "money")) {
		if (_G(flags[V061]) != 0) {
			switch (_G(kernel).trigger) {
			case 1:
				player_set_commands_allowed(false);
				ws_walk(_G(my_walker), 456, 366, nullptr, 2, 2, true);

				break;

			case 2:
				ws_hide_walker(_G(my_walker));
				_ripTrekLowReacherPos1Series = series_load("rip trek low reach pos2", -1, nullptr);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadow3Mach = series_place_sprite("safari shadow 2", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
				series_ranged_play_xy("rip trek low reach pos2", 1, 0, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 3, false);

				break;

			case 3:
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTrekLowReachMach = series_ranged_play_xy("rip trek low reach pos2", 0, 2, 15, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 300, -1, false);
				inv_give_to_player("REBUS AMULET");
				hotspot_set_active("REBUS AMULET", false);
				if (_G(flags[V063]) == 0) {
					kernel_examine_inventory_object("ping REBUS AMULET", _G(master_palette), 5, 1, 387, 250, 4, "207R04", -1);
				} else {
					kernel_examine_inventory_object("ping REBUS AMULET", _G(master_palette), 5, 1, 387, 250, 4, nullptr, -1);
				}

				terminateMachine(_rebusAmuletMach);

				break;

			case 4:
				terminateMachine(_ripTrekLowReachMach);
				player_update_info(_G(my_walker), &_G(player_info));
				series_ranged_play_xy("rip trek low reach pos2", 1, 2, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5,5,false);
				digi_play("207_s08", 2, 255, -1, -1);

				break;

			case 5:
				ws_unhide_walker(_G(my_walker));
				terminateMachine(_safariShadow3Mach);
				ws_walk(_G(my_walker), 418, 345, nullptr, 6,2,true);

				break;

			case 6:
				ws_hide_walker(_G(my_walker));
				_ripTrekLowReacherPos1Series = series_load("rip trek low reach pos2", -1, nullptr);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadow3Mach = series_place_sprite("safari shadow 2", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
				series_ranged_play_xy("rip trek low reach pos2", 1, 0, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 7, false);
				digi_play("207r35", 1, 255, -1, -1);

				break;

			case 7:
				_oneFrameMoneySpriteMach = series_show_sprite("one frame money sprite", 0, 3840);
				player_update_info(_G(my_walker), &_G(player_info));
				series_ranged_play_xy("rip trek low reach pos2", 1, 2, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 8, false);

				break;

			case 8:
				terminateMachine(_safariShadow3Mach);
				ws_unhide_walker(_G(my_walker));
				series_unload(_ripTrekLowReacherPos1Series);
				player_set_commands_allowed(true);

				break;

			default:
				break;
			}
		} else if (!_dollarFl) {
			switch (_G(kernel).trigger) {
			case 1:
				player_set_commands_allowed(false);
				ws_walk(_G(my_walker), 435, 352, nullptr, 2, 3, true);

				break;

			case 2:
				ws_hide_walker(_G(my_walker));
				_ripTrekLowReacherPos1Series = series_load("rip trek med reach pos3", -1, nullptr);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadow3Mach = series_place_sprite("safari shadow 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
				series_ranged_play_xy("rip trek med reach pos3", 1, 0, 0, 9, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 3, false);

				break;

			case 3:
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTrekLowReachMach = series_ranged_play_xy("rip trek med reach pos3", -1, 2, 10, 10, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 300, -1, false);
				_field92 = 7;

				break;

			case 4:
				terminateMachine(_ripTrekLowReachMach);
				player_update_info(_G(my_walker), &_G(player_info));
				series_ranged_play_xy("rip trek med reach pos3", 1, 2, 0, 9, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 5, false);

				break;

			case 5:
				ws_unhide_walker(_G(my_walker));
				terminateMachine(_safariShadow3Mach);
				series_unload(_ripTrekLowReacherPos1Series);
				player_set_commands_allowed(true);

				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case 1:
				player_set_commands_allowed(false);
				ws_walk(_G(my_walker), 435, 352, nullptr, 2, 3, true);

				break;

			case 2:
				ws_hide_walker(_G(my_walker));
				_ripTrekLowReacherPos1Series = series_load("rip trek med reach pos3", -1, nullptr);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadow3Mach = series_place_sprite("safari shadow 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
				series_ranged_play_xy("rip trek med reach pos3", 1, 0, 0, 9, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 3, false);

				break;

			case 3:
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTrekLowReachMach = series_ranged_play_xy("rip trek med reach pos3", -1, 2, 10, 10, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 300, -1, false);
				_field92 = 9;

				break;

			case 4:
				digi_play("203r40a", 1, 255, 5, -1);

				break;

			case 5:
				terminateMachine(_ripTrekLowReachMach);
				player_update_info(_G(my_walker), &_G(player_info));
				series_ranged_play_xy("rip trek med reach pos3", 1, 2, 0, 9, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 6, false);

				break;

			case 6:
				ws_unhide_walker(_G(my_walker));
				terminateMachine(_safariShadow3Mach);
				series_unload(_ripTrekLowReacherPos1Series);
				player_set_commands_allowed(true);
				_dollarFl = false;

				break;

			default:
				break;
			}
		}
	} // if (player_said("give", "money"))

	else if (ecx && player_said("junkpile"))
		digi_play("207r35a", 1, 255, -1, -1);
	else if (ecx && player_said("REBUS AMULET") && _G(flags[V061]) != 0 && _G(kernel).trigger > 0 && _G(kernel).trigger < 9) {
		switch (_G(kernel).trigger) {
		case 1:
			ws_walk(_G(my_walker), 456, 366, nullptr, 2, 2, true);
			break;

		case 2:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_ripTrekLowReacherPos1Series = series_load("rip trek low reach pos2", -1, nullptr);
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow3Mach = series_place_sprite("safari shadow 2", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			series_ranged_play_xy("rip trek low reach pos2", 1, 0, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 3, false);

			break;

		case 3:
			player_update_info(_G(my_walker), &_G(player_info));
			_ripTrekLowReachMach = series_ranged_play_xy("rip trek low reach pos2", -1, 2, 15, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 300, -1, false);
			inv_give_to_player("REBUS AMULET");
			hotspot_set_active(_G(currentSceneDef).hotspots, "REBUS AMULET", false);

			if (_G(flags[V063]) == 0) {
				kernel_examine_inventory_object("ping REBUS AMULET", _G(master_palette), 5, 1, 387, 250, 4, "207R04", -1);
			} else {
				kernel_examine_inventory_object("ping REBUS AMULET", _G(master_palette), 5, 1, 387, 250, 4, nullptr, -1);
			}
			terminateMachine(_rebusAmuletMach);

			break;

		case 4:
			terminateMachine(_ripTrekLowReachMach);
			player_update_info(_G(my_walker), &_G(player_info));
			series_ranged_play_xy("rip trek low reach pos2", 1, 2, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 5, false);

			break;

		case 5:
			ws_unhide_walker(_G(my_walker));
			terminateMachine(_safariShadow3Mach);
			ws_walk(_G(my_walker), 418, 345, nullptr, 6, 2, true);

			break;

		case 6:
			ws_hide_walker(_G(my_walker));
			_ripTrekLowReacherPos1Series = series_load("rip trek low reach pos2", -1, nullptr);
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow3Mach = series_place_sprite("safari shadow 2", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			series_ranged_play_xy("rip trek low reach pos2", 1, 0, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 7, false);
			digi_play("207r35", 1, 255, -1, -1);

			break;

		case 7:
			_oneFrameMoneySpriteMach = series_show_sprite("one frame money sprite", 0, 3840);
			player_update_info(_G(my_walker), &_G(player_info));
			series_ranged_play_xy("rip trek low reach pos2", 1, 2, 0, 15, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 512, 5, 8, false);

			break;

		case 8:
			terminateMachine(_safariShadow3Mach);
			ws_unhide_walker(_G(my_walker));
			series_unload(_ripTrekLowReacherPos1Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} else if (ecx && player_said("REBUS AMULET") && _G(flags[V061]) == 0 && inv_object_is_here("REBUS AMULET"))
		digi_play("207r35a", 1, 255, -1, -1);
	else if (_G(kernel).trigger == 666) {
		digi_unload("ripwhis");
	} // if (ecx && player_said("REBUS AMULET"))
	else if ((ecx || edi) && player_said("wheel")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_preload("207_s04", -1);
			digi_preload("207_s03", -1);
			digi_preload("ripwhis", -1);
			ws_hide_walker(_G(my_walker));
			terminateMachine(_wheelMach);
			hotspot_set_active(_G(currentSceneDef).hotspots, "WHEEL", false);
			_G(flags[V058]) = 1;
			hotspot_set_active(_G(currentSceneDef).hotspots, "METAL RIM", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "SEVEN SPOKES", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "WOODEN RIM", true);
			_dropsWheelMach = series_stream("drops wheel", 5, 1, 3);
			series_stream_break_on_frame(_dropsWheelMach, 22, 4);

			break;

		case 1:
			series_stream_break_on_frame(_dropsWheelMach, 84, 3);
			digi_play("207_s04", 1, 255, 2, -1);

			break;

		case 2:
			digi_play("ripwhis", 1, 255, 666, -1);
			break;

		case 3:
			ws_unhide_walker(_G(my_walker));
			player_update_info(_G(my_walker), &_G(player_info));
			ws_demand_location(_G(my_walker), 5 + _G(player_info).x, 20 + _G(player_info).y);
			_metalRimMach = series_show_sprite("metal rim", 0, 3840);
			_sevenSpokesMach = series_show_sprite("SEVEN spokes", 0, 3840);
			series_show_sprite("wooden rim", 0, 3840);
			digi_unload("207_s04");
			digi_unload("207_s03");
			player_set_commands_allowed(true);

			break;

		case 4:
			series_stream_break_on_frame(_dropsWheelMach, 46, 1);
			digi_play("207_s03", 2, 255, -1, -1);

			break;

		default:
			break;
		}
	} // if ((ecx || edi) && player_said("wheel"))

	else if (ecx && "PIPES") {
		_G(flags[V036]) = 1;
		if (_G(flags[V061]) != 0 && inv_object_is_here("LEAD PIPE")) {
			digi_play("207r36", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 1:
				if (inv_object_is_here("LEAD PIPE") || _fieldAA) {
					ws_walk(_G(my_walker), 279, 346, nullptr, 2, 7, true);
					_ripForegroundFl = true;
				}

				break;

			case 2:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				_ripTrekLowReacherPos1Series = series_load("rip trek low reacher pos5", -1, nullptr);
				player_update_info(_G(my_walker), &_G(player_info));
				series_play("rip trek low reacher pos5", 512, 128, 3, 3, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 15);
				_safariShadow3Mach = series_place_sprite("safari shadow 5", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);

				break;

			case 3:
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTrekLowReacherPos5Mach = series_play("rip trek low reacher pos5", 512, 128, -1, 300, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 15, 15);
				if (!_fieldAA) {
					inv_move_object("LEAD PIPE", 207);
					hotspot_set_active_xy(_G(currentSceneDef).hotspots, "PIPES", 277, 359, true);
					_leadPipeMach2 = series_show_sprite("lead pipe", 0, 3840);
					kernel_timing_trigger(1, 4, nullptr);
					digi_play("207_s02", 2, 255, -1, -1);
				} else {
					inv_give_to_player("LEAD PIPE");
					kernel_examine_inventory_object("ping LEAD PIPE", _G(master_palette), 5, 1, 179, 250, 4, nullptr, -1);
					terminateMachine(_leadPipeMach2);
				}

				break;

			case 4:
				terminateMachine(_ripTrekLowReacherPos5Mach);
				player_update_info(_G(my_walker), &_G(player_info));
				series_play("rip trek low reacher pos5", 512, 130, 5, 3, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 15);
				digi_play("207_s01", 2, 255, -1, -1);

				break;

			case 5:
				ws_unhide_walker(_G(my_walker));
				terminateMachine(_safariShadow3Mach);
				series_unload(_ripTrekLowReacherPos1Series);
				if (!_fieldAA) {
					hotspot_set_active(_G(currentSceneDef).hotspots, "PEASANT", false);
					_field92 = 3;
					_digiName = "207p01";
					_digiTriggerNum = 6;
				} else {
					_field92 = 3;
					_digiName = "207p01a";
					_digiTriggerNum = 7;
				}
				_nextTriggerMode = KT_PARSE;

				break;

			case 6:
				_field92 = 2;
				player_set_commands_allowed(true);
				_fieldB6_counter = 0;
				_fieldB2 = true;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(60, 40, nullptr);

				break;

			case 7:
				_fieldAA = false;
				_field92 = 0;
				_commandNotAllowedFl = true;
				_G(flags[V043]) = 1;

				break;

			default:
				break;
			}
		}
	} // if (ecx && "PIPES")

	else if (ecx && player_said("METAL RIM")) {
		_G(flags[V036]) = 1;

		if (_G(flags[V061]) == 0) {
			digi_play("207r29", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				if (inv_object_is_here("METAL RIM") || _fieldAA) {
					player_set_commands_allowed(false);
					player_update_info(_G(my_walker), &_G(player_info));
					_safariShadow3Mach = series_place_sprite("safari shadow 1", 0, _G(player_info).x, _G(player_info).y, -_G(player_info).scale, 3840);
					ws_hide_walker(_G(my_walker));
					_ripTrekLowReacherPos1Series = series_load("rip low reach pos1", -1, nullptr);
					player_update_info(_G(my_walker), &_G(player_info));
					series_play("rip low reach pos1", 512, 128, 1, 5, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 14);
					_ripForegroundFl = false;
				}

				break;

			case 1:
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTrekLowReacherPos5Mach = series_play("rip low reach pos1", 512, 128, -1, 300, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 14, 14);

				if (!_fieldAA) {
					inv_give_to_player("METAL RIM");
					kernel_examine_inventory_object("PING METAL RIM", _G(master_palette), 5, 1, 245, 244, 2, nullptr, -1);
					terminateMachine(_metalRimMach);
				} else {
					inv_move_object("METAL RIM", 207);
					hotspot_set_active(_G(currentSceneDef).hotspots, "METAL RIM", true);
					_metalRimMach = series_show_sprite("metal rim", 0, 3840);
					digi_play("207_s01", 2, 255, -1, -1);
					kernel_timing_trigger(1, 2, nullptr);
				}

				break;

			case 2:
				terminateMachine(_ripTrekLowReacherPos5Mach);
				player_update_info(_G(my_walker), &_G(player_info));
				series_play("rip low reach pos1", 512, 130, 3, 5, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 14);
				digi_play("207_s06", 2, 255, -1, -1);

				break;

			case 3:
				ws_unhide_walker(_G(my_walker));
				terminateMachine(_safariShadow3Mach);
				series_unload(_ripTrekLowReacherPos1Series);

				if (_fieldAA) {
					_field92 = 3;
					_digiName = "207p01a";
					_digiTriggerNum = 5;
					_nextTriggerMode = KT_PARSE;
				} else if (_G(flags[V061]) != 0) {
					player_set_commands_allowed(true);
				} else {
					hotspot_set_active(_G(currentSceneDef).hotspots, "PEASANT", false);
					_field92 = 3;
					_digiName = "207p01";
					_digiTriggerNum = 4;
					_nextTriggerMode = KT_PARSE;
				}
				break;

			case 4:
				hotspot_set_active(_G(currentSceneDef).hotspots, "METAL RIM", false);
				_field92 = 2;
				player_set_commands_allowed(true);
				_fieldB6_counter = 0;
				_fieldB2 = true;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(60, 40, nullptr);

				break;

			case 5:
				_fieldAA = false;
				_field92 = 0;
				_commandNotAllowedFl = true;
				_G(flags[V043]) = 1;

				break;

			default:
				break;
			}
		}

	} // if (ecx && player_said("METAL RIM"))

	else if (ecx && player_said("SEVEN SPOKES")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (inv_object_is_here("SEVEN SPOKES")) {
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadow3Mach = series_place_sprite("safari shadow 1", 0, _G(player_info).x, _G(player_info).y, -_G(player_info).scale, 3840);
				ws_hide_walker(_G(my_walker));
				_ripTrekLowReacherPos1Series = series_load("rip low reach pos1", -1, nullptr);
				player_update_info(_G(my_walker), &_G(player_info));
				series_play("rip low reach pos1", 512, 128, 1, 5, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 14);
			}

			break;

		case 1:
			player_update_info(_G(my_walker), &_G(player_info));
			_ripTrekLowReacherPos5Mach = series_play("rip low reach pos1", 512, 130, -1, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 14, 14);
			inv_give_to_player("SEVEN SPOKES");
			kernel_examine_inventory_object("ping SEVEN SPOKES", _G(master_palette), 5, 1, 241, 217, 2, nullptr, -1);
			terminateMachine(_sevenSpokesMach);

			break;

		case 2:
			terminateMachine(_ripTrekLowReacherPos5Mach);
			player_update_info(_G(my_walker), &_G(player_info));
			series_play("rip low reach pos1", 512, 130, 3, 5, 0, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 14);
			digi_play("207_s05", 2, 255, -1, -1);

			break;

		case 3:
			ws_unhide_walker(_G(my_walker));
			terminateMachine(_safariShadow3Mach);
			series_unload(_ripTrekLowReacherPos1Series);
			hotspot_set_active(_G(currentSceneDef).hotspots, "SEVEN SPOKES", false);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}

	} // if (ecx && player_said("SEVEN SPOKES"))

	else if (!_fieldAA) {
		if (esi && player_said("REBUS AMULET") && !inv_object_is_here("REBUS AMULET")) {
			if (_G(flags[V061] == 0)) {
				_G(flags[V061]) = 1;
				digi_play("207r04", 1, 255, -1, -1);
			} else
				digi_play("207r04a", 1, 255, -1, -1);
		} else if (esi && player_said("junkpile")) {
			if (_G(flags[V061]) == 0 && _G(flags[V063]) == 0) {
				switch (_G(kernel).trigger) {
				case -1:
					player_set_commands_allowed(false);
					player_update_info(_G(my_walker), &_G(player_info));
					_safariShadow3Mach = series_place_sprite("safari shadow 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3849);
					ws_hide_walker(_G(my_walker));
					_ripTrekLowReacherPos1Series = series_load("rip trek arms x pos3", -1, nullptr);
					_ripHeadDownTalkOffTd33Series2 = series_load("rip head down talk off td33", -1, nullptr);
					_digi207r04PlayedFl = false;
					player_update_info(_G(my_walker), &_G(player_info));
					_ripTrekLowReachMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 1024, false, triggerMachineByHashCallback, "rip exchange goods");
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripTrekLowReacherPos1Series, 1, 15, 1, _ripTrekLowReacherPos1Series, 1, 1, 0);

					break;

				case 1:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 1, 3, 2, _ripHeadDownTalkOffTd33Series2, 3, 3, 0);
					break;

				case 2:
					if (_G(flags[V036]) == 0)
						digi_play("207r03b", 1, 255, 8, -1);
					else
						digi_play("207r03", 1, 255, 3, -1);

					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 3, 3, -1, _ripHeadDownTalkOffTd33Series2, 3, 3, 0);

					break;

				case 3:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 3, 3, -1, _ripHeadDownTalkOffTd33Series2, 3, 5, 1);
					digi_play("207r03a", 1, 255, 4, -1);

					break;

				case 4:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 3, 3, -1, _ripHeadDownTalkOffTd33Series2, 3, 3, 1);
					_field92 = 6;

					break;

				case 5:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 3, 1, 6, _ripTrekLowReacherPos1Series, 15, 15, 0);
					break;

				case 6:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripTrekLowReacherPos1Series, 15, 1, 7, _ripTrekLowReacherPos1Series, 1, 1, 0);
					break;

				case 7:
					terminateMachine(_ripTrekLowReachMach);
					terminateMachine(_safariShadow3Mach);
					series_unload(_ripHeadDownTalkOffTd33Series2);
					series_unload(_ripTrekLowReacherPos1Series);
					ws_unhide_walker(_G(my_walker));
					player_set_commands_allowed(true);
					_G(flags[V043]) = 1;
					_G(flags[V063]) = 1;

					if (inv_object_is_here("REBUS AMULET"))
						hotspot_set_active(_G(currentSceneDef).hotspots, "REBUS AMULET", true);

					break;

				case 8:
					digi_play("207r34", 1, 255, 3, -1);
					break;

				default:
					break;
				}
			} else if (_G(flags[V063]) == 0) {
				switch (_G(kernel).trigger) {
				case -1:
					player_set_commands_allowed(false);
					_digi207r04PlayedFl = false;
					player_update_info(_G(my_walker), &_G(player_info));
					_safariShadow3Mach = series_place_sprite("safari shadow 3", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
					ws_hide_walker(_G(my_walker));
					_ripTrekLowReacherPos1Series = series_load("rip trek arms x pos3", -1, nullptr);
					_ripHeadDownTalkOffTd33Series2 = series_load("rip head down talk off td33", -1, nullptr);
					player_update_info(_G(my_walker), &_G(player_info));
					_ripTrekLowReachMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 1024, false, triggerMachineByHashCallback, "rip exchange goods");
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripTrekLowReacherPos1Series, 1, 15, 1, _ripTrekLowReacherPos1Series, 1, 1, 0);

					break;

				case 1:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 1, 3, 2, _ripHeadDownTalkOffTd33Series2, 3, 3, 0);
					break;

				case 2:
					digi_play("207r03b", 1, 255, 3, -1);
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 3, 3, -1, _ripHeadDownTalkOffTd33Series2, 3, 3, 0);

					break;

				case 3:
					if (inv_object_is_here("REBUS AMULET")) {
						digi_play("207r34", 1, 255, 4, -1);
					} else {
						kernel_timing_trigger(1, 4, nullptr);
					}

					break;

				case 4:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripHeadDownTalkOffTd33Series2, 3, 1, 5, _ripHeadDownTalkOffTd33Series2, 15, 15, 0);
					break;

				case 5:
					sendWSMessage_10000(1, _ripTrekLowReachMach, _ripTrekLowReacherPos1Series, 15, 1, 6, _ripTrekLowReacherPos1Series, 1, 1, 0);
					break;

				case 6:
					terminateMachine(_ripTrekLowReachMach);
					terminateMachine(_safariShadow3Mach);
					series_unload(_ripHeadDownTalkOffTd33Series2);
					series_unload(_ripTrekLowReacherPos1Series);
					ws_unhide_walker(_G(my_walker));
					player_set_commands_allowed(true);
					_G(flags[V063]) = 1;
					if (inv_object_is_here("REBUS AMULET")) {
						hotspot_set_active(_G(currentSceneDef).hotspots, "REBUS AMULET", true);
					}

					break;

				default:
					break;
				}
			} else {
				digi_play("207r03b", 1, 255, -1, -1);
			}
		}
	} // if (_fieldAA == 0)

	else if (player_said("exit")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);

			break;

		case 1:
			_G(game).new_room = 203;

			break;

		default:
			break;
		}
	} // if (player_said("exit")) {

	else if (esi && player_said("peasant")) {
		if (_G(flags[V062]) == 0) {
			digi_play("207r01", 1, 255, -1, -1);
			_G(flags[V062]) = 1;
		} else {
			digi_play("207r02", 1, 255, -1, -1);
		}
	} // if (esi && player_said("peasant"))

	else if (esi && player_said("SEVEN SPOKES") && inv_object_is_here("SEVEN SPOKES"))
		digi_play("207r08", 1, 255, -1, -1);
	else if (esi && player_said("METAL RIM") && inv_object_is_here("METAL RIM"))
		digi_play("207r21", 1, 255, -1, -1);
	else if (edi && (player_said("window") || player_said("door")))
		digi_play("207r33", 1, 255, -1, -1);
	else if (ecx && player_said("wooden rim"))
		digi_play("207r25", 1, 255, -1, -1);
	else if (ecx && player_said("wheel"))
		digi_play("207r26", 1, 255, -1, -1);
	else if (ecx && player_said("woodpile"))
		digi_play("207r40", 1, 255, -1, -1);
	else if (ecx && player_said("vase"))
		digi_play("207r27", 1, 255, -1, -1);
	else if (ecx && player_said("anvil"))
		digi_play("207r28", 1, 255, -1, -1);
	else if (ecx && player_said("cart"))
		digi_play("207r29", 1, 255, -1, -1);
	else if (ecx && player_said("sign"))
		digi_play("207r30", 1, 255, -1, -1);
	else if (ecx && player_said("jars"))
		digi_play("207r31", 1, 255, -1, -1);
	else if (ecx && player_said("vise"))
		digi_play("207r32", 1, 255, -1, -1);
	else if (player_said("SOLDIER'S HELMET", "PEASANT"))
		digi_play("203r64", 1, 255, -1, -1);
	else if (esi && player_said("cart pieces"))
		; // Nothing
	else if (inv_player_has(_G(player).verb) && player_said("peasant"))
		digi_play("203r50", 1, 255, -1, -1);
	else if (esi && player_said(" ")) {
		if (_G(flags[V054]) == 0)
			digi_play("207r23a", 1, 255, -1, -1);
		else
			digi_play("207r23", 1, 255, -1, -1);
	} else if (!player_said("xxx"))
		return;


	_G(player).command_ready = 0;

}

void Room207::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("950_s02", 3, 30, -1, -1);
			kernel_timing_trigger(1500, 1, nullptr);
			break;

		case 2:
			digi_play("950_s03", 3, 30, -1, -1);
			kernel_timing_trigger(1020, 1, nullptr);
			break;

		case 3:
			digi_play("950_s04", 3, 30, -1, -1);
			kernel_timing_trigger(540, 1, nullptr);
			break;

		default:
			break;
		}

		break;

	case 10:
		sendWSMessage_60000(_ppWalkerMach);
		series_play_xy("shadow pesky from 9 to squat", 1, 0, 514, 367, 97, 0, 5, -1);
		series_play_xy("peasant 3 to squat", 1, 0, 514, 367, 97, 0, 5, 11);

		break;

	case 11:
		series_play_xy("shadow pesky rock loop", -1, 0, 514, 367, 97, 3849, 300, -1);
		_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 514, 367, 97, 3840, false, triggerMachineByHashCallback, "pp squat");
		sendWSMessage_10000(1, _ppSquatMach, 1, _peskyRockLoopSeries, 20, 101, _peskyRockLoopSeries, 20, 20, 0);
		_field8E = 0;
		_field92 = 0;

		break;

	case 20:
		if (!_fieldC6 && _fieldC2 == 0 && _dispatchTriggerNum != -1) {
			kernel_trigger_dispatchx(_dispatchTriggerNum);
			_dispatchTriggerNum = -1;
			if (_hiddenWalkerFl) {
				ws_unhide_walker(_G(my_walker));
				_hiddenWalkerFl = false;
			}

			if (_fieldA4) {
				sendWSMessage_80000(_ripInConvMach);
				_fieldA4 = false;
			}
		}

		kernel_timing_trigger(1, 21, nullptr);
		break;

	case 21:
		if (_fieldC6)
			break;

		switch (_fieldC2) {
		case 0:
			sendWSMessage_10000(1, _ripInConvMach, _ripHeadDownTalkOffTd33Series, 3, 3, 20, _ripHeadDownTalkOffTd33Series, 3, 3, 0);
			break;

		case 1:
			_G(kernel).trigger_mode = KT_PARSE;
			conv_load("conv203d", 10, 10, 747);
			conv_set_shading(65);
			conv_export_value(conv_get_handle(), _G(flags[V054]), 0);

			if (inv_player_has("REBUS AMULET"))
				conv_export_value(conv_get_handle(), 0, 1);
			else
				conv_export_value(conv_get_handle(), 1, 1);

			conv_export_value(conv_get_handle(), 1, 2);
			conv_export_value(conv_get_handle(), _G(flags[V073]), 3);
			conv_export_value(conv_get_handle(), _G(flags[V043]), 5);

			conv_play(conv_get_handle());
			_fieldC2 = 0;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 20, nullptr);

			break;

		case 2: {
			int32 rnd = imath_ranged_rand(3, 5);
			sendWSMessage_10000(1, _ripInConvMach, _ripHeadDownTalkOffTd33Series, rnd, rnd, 20, _ripHeadDownTalkOffTd33Series, rnd, rnd, 0);
			}
			break;

		case 3:
			sendWSMessage_10000(1, _ripInConvMach, _ripHeadDownTalkOffTd33Series, 3, 3, 24, _ripHeadDownTalkOffTd33Series, 3, 3, 0);
			break;

		default:
			break;

		}

		break;

	case 22:
		terminateMachine(_ripInConvMach);
		terminateMachine(_safariShadow3Mach);
		series_unload(_ripHeadDownTalkOffTd33Series);
		series_unload(_ripTrekArmsXPos3Series);
		ws_unhide_walker(_G(my_walker));
		player_set_commands_allowed(true);

		break;

	case 23:
		sendWSMessage_10000(1, _ripInConvMach, _ripHeadDownTalkOffTd33Series, 1, 3, 20, _ripHeadDownTalkOffTd33Series, 3, 3, 0);
		break;

	case 24:
		sendWSMessage_10000(1, _ripInConvMach, _ripTrekArmsXPos3Series, 15, 1, 22, _ripTrekArmsXPos3Series, 1, 1, 0);
		break;

	case 40:
		if (_fieldB2) {
			++_fieldB6_counter;
			if (_fieldB6_counter < 20) {
				kernel_timing_trigger(60, 40, nullptr);
			} else {
				other_save_game_for_resurrection();
				disable_player_commands_and_fade_init(749);
				_G(flags[V067]) = 1;
			}
		}

		break;

	case 50:
		player_set_commands_allowed(true);
		break;

	case 100:
		if (_field8E != 0 || _field92 != 0 || _dispatchTriggerNum == -1) {
			kernel_trigger_dispatchx(kernel_trigger_create(101));
			break;
		}

		kernel_trigger_dispatchx(_dispatchTriggerNum);
		_dispatchTriggerNum = -1;

		if (_hiddenWalkerFl) {
			ws_unhide_walker(_G(my_walker));
			_hiddenWalkerFl = false;
		}
		kernel_trigger_dispatchx(kernel_trigger_create(101));

		break;

	case 101:
		switch (_field8E) {
		case 2:
			if (_field92 == 2) {
				if (_commandNotAllowedFl) {
					_commandNotAllowedFl =false;
					player_set_commands_allowed(true);
				}

				++_field8A;
				if (imath_ranged_rand(15, 40) < _field8A) {
					_fieldAE_rnd = imath_ranged_rand(1, 3);
					_field8A = 0;
				}

				if (_ripForegroundFl) {
					switch (_fieldAE_rnd) {
					case 1:
						sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 19, 19, 100, _peskyPointsRipForegroundSeries, 19, 19, 0);
						break;

					case 2:
						sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 20, 20, 100, _peskyPointsRipForegroundSeries, 20, 20, 0);
						break;

					case 3:
						sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 21, 21, 100, _peskyPointsRipForegroundSeries, 21, 21, 0);
						break;

					default:
						break;

					}
				} else {
					switch (_fieldAE_rnd) {
					case 1:
						sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 19, 19, 100, _peskyPointsRipBackgroundSeries, 19, 19, 0);
						break;

					case 2:
						sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 20, 20, 100, _peskyPointsRipBackgroundSeries, 20, 20, 0);
						break;

					case 3:
						sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 21, 21, 100, _peskyPointsRipBackgroundSeries, 21, 21, 0);
						break;

					default:
						break;
					}
				}

			} else if (_field92 == 3) {
				if (_ripForegroundFl)
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 17, 17, 100, _peskyPointsRipForegroundSeries, 17, 17, 1);
				else
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 17, 17, 100, _peskyPointsRipBackgroundSeries, 17, 17, 1);
				_field8E = 3;
			} else { // _field92 != 2 && _field92 != 3
				if (_ripForegroundFl)
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 18, 1, 115, _peskyRockLoopSeries, 1, 1, 0);
				else
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 18, 1, 118, _peskyRockLoopSeries, 1, 1, 0);

				_field8E = 0;
				hotspot_set_active(_G(currentSceneDef).hotspots, "PEASANT", true);
			}
			break;
		case 3:
			if (_field92 == 3) {
				if (_digiName) {
					_G(kernel).trigger_mode = _nextTriggerMode;
					digi_play(_digiName, 1, 255, _digiTriggerNum, 203);
					_digiName = nullptr;
					_nextTriggerMode = KT_DAEMON;
					_G(kernel).trigger_mode = KT_DAEMON;
					_fieldAA = true;
				}

				int32 rnd = imath_ranged_rand(17, 21);
				if (_ripForegroundFl) {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, rnd, rnd, 100, _peskyPointsRipForegroundSeries, rnd, rnd, 0);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, rnd, rnd, 100, _peskyPointsRipBackgroundSeries, rnd, rnd, 0);
				}

				sendWSMessage_190000(_ppSquatMach, 13);
				sendWSMessage_1a0000(_ppSquatMach, 13);
			} else {
				if (_ripForegroundFl) {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 17, 17, 100, _peskyPointsRipForegroundSeries, 17, 17, 1);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 17, 17, 100, _peskyPointsRipBackgroundSeries, 17, 17, 1);
				}
				_field8E = 2;
			}

			break;

		case 6:
			if (_field92 == 6) {
				if (_digi207r04PlayedFl) {
					if (inv_player_has("REBUS AMULET")) {
						sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 52, 1, 104, _withoutAmuletSeries, 1, 1, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 52, 1, 104, _ppShowsAmuletTakesMoneySeries, 1, 1, 0);
					}
				} else {
					digi_play("207r04", 1, 255, 100, -1);
					_digi207r04PlayedFl = true;
				}
			} else {
				if (inv_player_has("REBUS AMULET")) {
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 52, 1, 100, _withoutAmuletSeries, 1, 1, 1);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 52, 1, 100, _ppShowsAmuletTakesMoneySeries, 1, 1, 1);
				}

				_field8E = 0;
			}

			break;

		case 7:
			if (_field92 == 7) {
				if (inv_player_has("REBUS AMULET")) {
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 52, 52, -1, _withoutAmuletSeries, 52, 52, 0);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 52, 52, -1, _ppShowsAmuletTakesMoneySeries, 52, 52, 0);
				}

				_field8E = 8;
				inv_give_to_player("REBUS AMULET");
				hotspot_set_active(_G(currentSceneDef).hotspots, "REBUS AMULET", false);
				kernel_examine_inventory_object("ping rebus amulet", _G(master_palette), 5, 1, 329, 250, 103, nullptr, -1);
			}

			break;

		case 8:
			if (_field92 == 8) {
				if (inv_player_has("REBUS AMULET")) {
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 52, 52, 100, _withoutAmuletSeries, 52, 52, 0);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 52, 52, 100, _ppShowsAmuletTakesMoneySeries, 52, 52, 0);
				}
			}

			break;
		case 9:
			if (_field92 == 9) {
				if (inv_player_has("REBUS AMULET")) {
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 16, 16, -1, _withoutAmuletSeries, 16, 16, 0);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 16, 16, -1, _ppShowsAmuletTakesMoneySeries, 16, 16, 0);
				}

				kernel_timing_trigger(60, 100, nullptr);
				_field8E = 10;
			}

			break;

		case 10:
			if (inv_player_has("REBUS AMULET")) {
				sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 16, 1, 100, _withoutAmuletSeries, 1, 1, 0);
				_field8E = 11;
			} else {
				sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 16, 1, 100, _ppShowsAmuletTakesMoneySeries, 1, 1, 0);
				_field8E = 11;
			}

			break;

		case 11:
			terminateMachine(_ppSquatMach);
			_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 514, 367, 97, 3840, 0, triggerMachineByHashCallback, "pp squat");
			sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 1, 100, _peskyRockLoopSeries, 1, 1, 0);
			_G(kernel).trigger_mode = KT_PARSE;
			kernel_timing_trigger(1, 4, nullptr);
			_field8E = 0;
			_field92 = 0;

			break;

		case 12:
			terminateMachine(_ppSquatMach);
			_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 514, 367, 97, 3840, 0, triggerMachineByHashCallback, "pp squat");
			if (inv_object_is_here("REBUS AMULET"))
				_rebusAmuletMach = series_show_sprite("rebus amulet", 0, 3840);

			sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 1, 100, _peskyRockLoopSeries, 1, 1, 0);

			_field8E = 0;
			_field92 = 0;
			break;

		case 14:
			switch (_field92) {
				case 6:
				terminateMachine(_rebusAmuletMach);
				if (inv_player_has("REBUS AMULET")) {
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 16, 52, 106, _withoutAmuletSeries, 52, 52, 0);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 16, 52, 106, _ppShowsAmuletTakesMoneySeries, 52, 52, 0);
				}

				break;

			case 14:
				if (inv_player_has("REBUS AMULET")) {
					int32 rnd = imath_ranged_rand(75, 77);
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, rnd, rnd, 100, _withoutAmuletSeries, rnd, rnd, 0);
				} else {
					int32 rnd = imath_ranged_rand(75, 77);
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, rnd, rnd, 100, _ppShowsAmuletTakesMoneySeries, rnd, rnd, 0);
				}

				break;

			case 15:
				if (inv_player_has("REBUS AMULET")) {
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 75, 75, 100, _withoutAmuletSeries, 75, 75, 0);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 75, 75, 100, _ppShowsAmuletTakesMoneySeries, 75, 75, 0);
				}

				break;

			default:
				if (inv_player_has("REBUS AMULET"))
					sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 14, 1, 105, _withoutAmuletSeries, 1, 1, 0);
				else
					sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 14, 1, 105, _ppShowsAmuletTakesMoneySeries, 1, 1, 0);
				break;
			}

			break; // case 14

		case 16:
			if (_field92 == 16) {
				kernel_timing_trigger(30, 100, nullptr);
			} else {
				digi_play("203p01", 1, 255, -1, -1);
				sendWSMessage_10000(1, _ppSquatMach, _peskyBegLoopSeries, 15, 1, 111, _peskyBegLoopSeries, 1, 1, 0);
			}

			break;

		default:
			if (_field8E <= 0) {
				if (_field92 == 6) {
					terminateMachine(_ppSquatMach);
					_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "pp squat");
					terminateMachine(_rebusAmuletMach);
					if (inv_player_has("REBUS AMULET")) {
						sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 1, 52, 100, _withoutAmuletSeries, 52, 52, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 1, 52, 100, _ppShowsAmuletTakesMoneySeries, 52, 52, 0);
					}
					_field8E = 6;
				} else if (_field92 == 7) {
					terminateMachine(_ppSquatMach);
					_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "pp squat");
					terminateMachine(_rebusAmuletMach);
					if (inv_player_has("REBUS AMULET")) {
						sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 1, 52, 100, _withoutAmuletSeries, 52, 52, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 1, 52, 100, _ppShowsAmuletTakesMoneySeries, 52, 52, 0);
					}
					_field8E = 7;
				} else if (_field92 == 9) {
					terminateMachine(_ppSquatMach);
					_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "pp squat");
					if (inv_player_has("REBUS AMULET")) {
						sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 1, 16, 100, _withoutAmuletSeries, 16, 16, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 1, 16, 100, _ppShowsAmuletTakesMoneySeries, 16, 16, 0);
					}
					_field8E = 9;
				} else if (_field92 == 13) {
					terminateMachine(_ppSquatMach);
					_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "pp squat");
					if (inv_player_has("REBUS AMULET")) {
						sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 1, 14, 100, _withoutAmuletSeries, 14, 14, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 1, 14, 100, _ppShowsAmuletTakesMoneySeries, 14, 14, 0);
					}
					_field92 = 15;
					_field8E = 14;
				} else if (_field92 == 16) {
					terminateMachine(_ppSquatMach);
					_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 514, 367, 97, 3840, 0, triggerMachineByHashCallback, "pp squat");
					sendWSMessage_10000(1, _ppSquatMach, _peskyBegLoopSeries, 1, 15, 110, _peskyBegLoopSeries, 15, 15, 0);
					_field8E = 16;
				} else if (_field92 <= 0) {
					if (_commandNotAllowedFl) {
						_commandNotAllowedFl = false;
						player_set_commands_allowed(true);
					}

					if (_digiName) {
						_G(kernel).trigger_mode = _nextTriggerMode;
						digi_play(_digiName, 1, 255, _digiTriggerNum, 203);
						_digiName = nullptr;
						_nextTriggerMode = KT_DAEMON;
						_G(kernel).trigger_mode = KT_DAEMON;
					}

					if (imath_ranged_rand(1, 2) == 1) {
						sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 10, 100, _peskyRockLoopSeries, 10, 10, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 11, 20, 100, _peskyRockLoopSeries, 20, 20, 0);
					}
				} else {
					int32 series;
					if (_ripForegroundFl) {
						series = _peskyPointsRipForegroundSeries = series_load("pesky points at rip foreground", -1, nullptr);
					} else {
						series = _peskyPointsRipBackgroundSeries = series_load("pesky points at rip background", -1, nullptr);
					}
					sendWSMessage_10000(1, _ppSquatMach, series, 1, 21, 100, series, 21, 21, 1);
					_field8E = 2;
				}
			}
			break;
		}
		break;

	case 102:
		_field92 = 2;
		_field8E = 2;
		player_set_commands_allowed(true);
		_fieldAA = true;
		kernel_timing_trigger(1, 100, nullptr);
		_fieldB6_counter = 0;
		_fieldB2 = true;
		kernel_timing_trigger(60, 40, nullptr);

		break;

	case 103:
		Common::strcpy_s(_G(player).verb, "give");
		Common::strcpy_s(_G(player).noun, "money");
		if (inv_player_has("REBUS AMULET")) {
			sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 53, 74, 100, _withoutAmuletSeries, 74, 74, 0);
		} else {
			sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 53, 74, 100, _ppShowsAmuletTakesMoneySeries, 74, 74, 0);
		}

		_field8E = 12;
		_commandNotAllowedFl = true;
		_G(kernel).trigger_mode = KT_PARSE;

		kernel_timing_trigger(1,4,nullptr);
		break;

	case 104:
		_field8E = 12;
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(30, 5, nullptr);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 100, nullptr);

		break;

	case 105:
		terminateMachine(_ppSquatMach);
		_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 514, 367, 97, 3840, 0, triggerMachineByHashCallback, "pp squat");
		sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 1, 100, _peskyRockLoopSeries, 1, 1, 0);
		_field8E = 0;

		break;

	case 106:
		kernel_timing_trigger(150, 107, nullptr);
		break;

	case 107:
		if (inv_player_has("REBUS AMULET"))
			sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 52, 1, 108, _withoutAmuletSeries, 1, 1, 0);
		else
			sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 52, 1, 108, _ppShowsAmuletTakesMoneySeries, 1, 1, 0);

		_field92 = 0;
		_field8E = 0;

		break;

	case 108:
		terminateMachine(_ppSquatMach);
		_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 514, 367, 97, 3840, false, triggerMachineByHashCallback, "pp squat");
		_rebusAmuletMach = series_show_sprite("rebus amulet", 0, 3840);
		sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 1, 100, _peskyRockLoopSeries, 1, 1, 0);

		_field8E = 0;
		_fieldC2 = 3;
		_commandNotAllowedFl = false;
		_node1Entry2Fl = false;

		break;

	case 110:
		Common::strcpy_s(_G(player).verb, "handout");
		kernel_timing_trigger(1, 100, nullptr);
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 3, nullptr);

		break;

	case 111:
		terminateMachine(_ppSquatMach);
		_ppSquatMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 514, 367, 97, 3840, false, triggerMachineByHashCallback, "pp squat");
		sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 1, 100, _peskyRockLoopSeries, 1, 1, 0);

		_field8E = 0;
		_commandNotAllowedFl = true;

		break;

	case 115:
		kernel_timing_trigger(1, 116, nullptr);
		break;

	case 116:
		series_unload(_peskyPointsRipForegroundSeries);
		sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 1, 100, _peskyRockLoopSeries, 1, 1, 0);

		break;

	case 118:
		kernel_timing_trigger(1, 119, nullptr);
		break;

	case 119:
		series_unload(_peskyPointsRipBackgroundSeries);
		sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 1, 100, _peskyRockLoopSeries, 1, 1, 0);

		break;

	case 749:
		_G(game).new_room = 203;
		break;

	default:
		break;
	}
}

void Room207::convHandler() {
	conv_sound_to_play();

	if (_G(kernel).trigger == 1) {
		int32 who = conv_whos_talking();
		if (who <= 0)
			_field92 = 15;
		else if (who == 1)
			_fieldC2 = 0;
	} else {
		int32 who = conv_whos_talking();
		if (who <= 0)
			_field92 = 14;
		else if (who == 1) {
			_fieldC2 = 2;
			if (conv_current_node() == 1 && conv_current_entry() == 2)
				_node1Entry2Fl = true;
		}

		if (conv_sound_to_play() != nullptr) {
			digi_play(conv_sound_to_play(), 1, 255, 1, 203);
			return;
		}
	}

	conv_resume(conv_get_handle());
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
