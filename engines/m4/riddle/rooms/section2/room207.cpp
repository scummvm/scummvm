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
		_field96 = 0;
		_field9E = -1;
		_field88 = 2;
		_field89 = 2;
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

		if (!_fieldAC) {
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

		if (_fieldAC == 1) {
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
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
