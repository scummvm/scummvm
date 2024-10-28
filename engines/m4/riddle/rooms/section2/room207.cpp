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
		_fieldCA = 0;
		_fieldBE = 0;
		_field96_digiName = nullptr;
		_field9E_triggerNum = -1;
		_field88 = 2;
		_field89_triggerMode = KT_DAEMON;
		_fieldA2 = 0;
		_fieldA4 = 0;
		_fieldA6 = 0;
		_fieldA8 = 0;
		_fieldAA = 0;
		_fieldAE = 1;
		_fieldB2 = 0;
	}

	_fieldB6 = 0;
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

		if (!_fieldAC_ripForegroundFl) {
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
				_fieldB2 = 0;
			}
		}

		if (_fieldAC_ripForegroundFl == 1) {
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
				_fieldB2 = 0;
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
				_fieldBE = 1;

			Common::strcpy_s(_G(player).verb, "give");
			Common::strcpy_s(_G(player).noun, "money");
			_G(player).need_to_walk = false;
		}
	}
}

void Room207::parser() {
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
		if (_fieldC6 == 0 && _fieldC2 == 0 && _field9E_triggerNum != -1) {
			kernel_trigger_dispatchx(_field9E_triggerNum);
			_field9E_triggerNum = -1;
			if (_fieldA2) {
				ws_unhide_walker(_G(my_walker));
				_fieldA2 = 0;
			}

			if (_fieldA4) {
				sendWSMessage_80000(_field84);
				_fieldA4 = 0;
			}
		}

		kernel_timing_trigger(1, 21, nullptr);
		break;

	case 21:
		if (_fieldC6)
			break;

		switch (_fieldC2) {
		case 0:
			sendWSMessage_10000(1, _field84, _field4C, 3, 3, 20, _field4C, 3, 3, 0);
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
			sendWSMessage_10000(1, _field84, _field4C, rnd, rnd, 20, _field4C, rnd, rnd, 0);
			}
			break;

		case 3:
			sendWSMessage_10000(1, _field84, _field4C, 3, 3, 24, _field4C, 3, 3, 0);
			break;

		default:
			break;

		}

		break;

	case 22:
		terminateMachine(_field84);
		terminateMachine(_field68);
		series_unload(_field4C);
		series_unload(_field50);
		ws_unhide_walker(_G(my_walker));
		player_set_commands_allowed(true);

		break;

	case 23:
		sendWSMessage_10000(1, _field84, _field4C, 1, 3, 20, _field4C, 3, 3, 0);
		break;

	case 24:
		sendWSMessage_10000(1, _field84, _field50, 15, 1, 22, _field50, 1, 1, 0);
		break;

	case 40:
		if (_fieldB2 != 0) {
			++_fieldB6;
			if (_fieldB6 < 20) {
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
		if (_field8E != 0 || _field92 != 0 || _field9E_triggerNum == -1) {
			kernel_trigger_dispatchx(kernel_trigger_create(101));
			break;
		}

		kernel_trigger_dispatchx(_field9E_triggerNum);
		_field9E_triggerNum = -1;

		if (_fieldA2 != 0) {
			ws_unhide_walker(_G(my_walker));
			_fieldA2 = 0;
		}
		kernel_trigger_dispatchx(kernel_trigger_create(101));

		break;

	case 101:
		switch (_field8E) {
		case 2:
			if (_field92 == 2) {
				if (_fieldA6) {
					_fieldA6 = 0;
					player_set_commands_allowed(true);
				}

				++_field8A;
				if (imath_ranged_rand(15, 40) < _field8A) {
					_fieldAE = imath_ranged_rand(1, 3);
					_field8A = 0;
				}

				if (_fieldAC_ripForegroundFl == 1) {
					switch (_fieldAE) {
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
					switch (_fieldAE) {
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
				if (_fieldAC_ripForegroundFl == 1)
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 17, 17, 100, _peskyPointsRipForegroundSeries, 17, 17, 1);
				else
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 17, 17, 100, _peskyPointsRipBackgroundSeries, 17, 17, 1);
				_field8E = 3;
			} else { // _field92 != 2 && _field92 != 3
				if (_fieldAC_ripForegroundFl == 1)
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 18, 1, 115, _peskyRockLoopSeries, 1, 1, 0);
				else
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 18, 1, 118, _peskyRockLoopSeries, 1, 1, 0);

				_field8E = 0;
				hotspot_set_active(_G(currentSceneDef).hotspots, "PEASANT", true);
			}
			break;
		case 3:
			if (_field92 == 3) {
				if (_field96_digiName) {
					_G(kernel).trigger_mode = _field89_triggerMode;
					digi_play(_field96_digiName, 1, 255, _field9A_triggerNum, 203);
					_field96_digiName = nullptr;
					_field89_triggerMode = KT_DAEMON;
					_G(kernel).trigger_mode = KT_DAEMON;
					_fieldAA = 1;
				}

				int32 rnd = imath_ranged_rand(17, 21);
				if (_fieldAC_ripForegroundFl == 1) {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, rnd, rnd, 100, _peskyPointsRipForegroundSeries, rnd, rnd, 0);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, rnd, rnd, 100, _peskyPointsRipBackgroundSeries, rnd, rnd, 0);
				}

				sendWSMessage_190000(_ppSquatMach, 13);
				sendWSMessage_1a0000(_ppSquatMach, 13);
			} else {
				if (_fieldAC_ripForegroundFl == 1) {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipForegroundSeries, 17, 17, 100, _peskyPointsRipForegroundSeries, 17, 17, 1);
				} else {
					sendWSMessage_10000(1, _ppSquatMach, _peskyPointsRipBackgroundSeries, 17, 17, 100, _peskyPointsRipBackgroundSeries, 17, 17, 1);
				}
				_field8E = 2;
			}

			break;

		case 6:
			if (_field92 == 6) {
				if (_fieldBA) {
					if (inv_player_has("REBUS AMULET")) {
						sendWSMessage_10000(1, _ppSquatMach, _withoutAmuletSeries, 52, 1, 104, _withoutAmuletSeries, 1, 1, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _ppShowsAmuletTakesMoneySeries, 52, 1, 104, _ppShowsAmuletTakesMoneySeries, 1, 1, 0);
					}
				} else {
					digi_play("207r04", 1, 255, 100, -1);
					_fieldBA = 1;
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
					if (_fieldA6) {
						_fieldA6 = 0;
						player_set_commands_allowed(true);
					}

					if (_field96_digiName) {
						_G(kernel).trigger_mode = _field89_triggerMode;
						digi_play(_field96_digiName, 1, 255, _field9A_triggerNum, 203);
						_field96_digiName = nullptr;
						_field89_triggerMode = KT_DAEMON;
						_G(kernel).trigger_mode = KT_DAEMON;
					}

					if (imath_ranged_rand(1, 2) == 1) {
						sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 1, 10, 100, _peskyRockLoopSeries, 10, 10, 0);
					} else {
						sendWSMessage_10000(1, _ppSquatMach, _peskyRockLoopSeries, 11, 20, 100, _peskyRockLoopSeries, 20, 20, 0);
					}
				} else {
					int32 series;
					if (_fieldAC_ripForegroundFl == 1) {
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
		_fieldAA = 1;
		kernel_timing_trigger(1, 100, nullptr);
		_fieldB6 = 0;
		_fieldB2 = 1;
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
		_fieldA6 = 1;
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
		_fieldA6 = 0;
		_fieldCA = 0;

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
		_fieldA6 = 1;

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

} // namespace Rooms
} // namespace Riddle
} // namespace M4
