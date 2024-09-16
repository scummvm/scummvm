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

#include "m4/riddle/rooms/section5/room510.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room510::init() {
	digi_preload("509_s01");
	_val1 = 0;

	static const char *HOTSPOTS[] = {
		"ROPE ", "GREEN VINE ", "BROWN VINE ", "VINES ",
		"ROPE", "GREEN VINE", "BROWN VINE", "VINES"
	};
	for (int i = 0; i < 8; ++i)
		hotspot_set_active(HOTSPOTS[i], false);

	switch (_G(flags)[V169]) {
	case 0:
		_statue = series_play("510 STATUE", 0xa00, 16, -1, 5);
		hotspot_set_active("ALTAR POST ", false);
		kernel_load_variant("510lock0");
		break;

	case 1:
		_statue = series_play("510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);
		hotspot_set_active("ALTAR POST", false);
		kernel_load_variant("510lock1");
		break;

	case 2:
		_statue = series_play(" 510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);

		if (inv_object_is_here("ROPE")) {
			hotspot_set_active("ROPE ", true);
			_ropeStatue = series_play("510 STATUE AND ROPE", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		if (inv_object_is_here("BROWN VINE")) {
			hotspot_set_active("BROWN VINE ", true);
			_ropeStatue = series_play("510 STATUE AND ANY VINE", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		if (inv_object_is_here("GREEN VINE")) {
			hotspot_set_active("GREEN VINE ", true);
			_ropeStatue = series_play("510 STATUE AND ANY VINE", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		if (inv_object_is_here("VINES")) {
			hotspot_set_active("VINES  ", true);
			_ropeStatue = series_play("510 STATUE AND COMBO VINES", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		hotspot_set_active("ALTAR POST", false);
		kernel_load_variant("510lock1");
		break;

	default:
		break;
	}

	if (inv_object_is_here("WOODEN LADDER"))
		_ladder = series_play("510 LADDER", 0xf00, 16, -1, 5);
	else
		hotspot_set_active("WOODEN LADDER", false);

	if (_G(flags)[V169] != 2) {
		if (inv_object_is_here("ROPE")) {
			hotspot_set_active("ROPE", true);
			_ropeStatue = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}

		if (inv_object_is_here("GREEN VINE")) {
			hotspot_set_active("GREEN VINE", true);
			_ropeStatue = series_play("510 SHORTER VINE UNWINDS", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}

		if (inv_object_is_here("BROWN VINE")) {
			hotspot_set_active("BROWN VINE", true);
			_ropeStatue = series_play("510 LONGER VINE UNWINDS", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}

		if (inv_object_is_here("VINES")) {
			hotspot_set_active("VINES", true);
			_ropeStatue = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}
	}

	hotspot_set_active("SKULL", false);

	if (inv_object_is_here("CRYSTAL SKULL"))
		_skull = series_play("SKULL SPARKLES", 0x100, 16, -1, 15);

	if (_G(flags)[V170] == 1 && inv_object_is_here("CRYSTAL SKULL")) {
		hotspot_set_active("SKULL", true);
		hotspot_set_active("NICHE", false);
		_G(flags)[V292] = 0;
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		if (_G(flags)[V170]) {
			ws_hide_walker();
			_steps = series_play("RIP DANGLES FROM ROPE", 0x100, 16, -1, 5);
		}

		kernel_timing_trigger(60, 999);

	} else {
		player_set_commands_allowed(false);
		_stepsSeries = series_load("510 UP STEPS");
		_steps = series_play("510 UP STEPS", 0xf00, 0, 110, 5);
		ws_demand_location(409, 86, 11);
		ws_hide_walker();
	}

	digi_play_loop("509_s01", 3, 127);
}

void Room510::daemon() {
}

void Room510::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (!useFlag && !lookFlag && !takeFlag)
		return;
	if (!_G(flags)[V170])
		return;

	if (takeFlag && player_said("SKULL"))
		return;
	if (useFlag && player_said("ROPE"))
		return;

	if (lookFlag && player_said("NICHE")) {
		digi_play("com127", 1);
		intr_cancel_sentence();
		return;
	}

	if (lookFlag && player_said("ROPE"))
		return;

	if (lookFlag && player_said("WOODEN LADDER")) {
		digi_play("com107", 1);
	} else {
		if (lookFlag && player_said("TOWER"))
			return;
		if (lookFlag && player_said("SKULL"))
			return;
		if (!lookFlag && !takeFlag && !useFlag)
			return;

		if (player_said_any("ALTAR POST", "ALTAR POST ", "TROUGH", "STAIRS")) {
			switch (imath_ranged_rand(1, 5)) {
			case 1:
				digi_play("301r23", 1, 255, -1, 301);
				break;
			case 2:
				digi_play("301r25", 1, 255, -1, 301);
				break;
			case 3:
				digi_play("301r26", 1, 255, -1, 301);
				break;
			case 4:
				digi_play("301r35", 1, 255, -1, 301);
				break;
			case 5:
				digi_play("301r36", 1, 255, -1, 301);
				break;
			default:
				break;
			}
		}
	}
}

void Room510::parser() {

}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
