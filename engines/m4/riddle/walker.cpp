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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/riddle/walker.h"
#include "m4/adv_r/adv_control.h"
#include "m4/riddle/vars.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Riddle {

// Starting hashes for walker machines/sequences/etc
#define WALKER_HASH 8			  // machine/data starting hash for wilbur
#define WALKER_SERIES_HASH 0
#define NUM_WALKER_SERIES  8
#define SHADOW_SERIES_HASH 8
#define NUM_SHADOW_SERIES  5

const int16 RIPLEY_SERIES_DIRS[] = {
	0, 1, 2, 3, 4, -4
};
const int16 RIPLEY_SHADOWS_DIRS[6] = {
	10, 11, 12, 13, 14, -1
};

static const char *RIPLEY_SERIES[5] = {
	"test1", "test2", "test3", "test4", "test5"
};

static const char *RIPLEY_SHADOWS[5] = {
	"ripsh1", "ripsh2", "ripsh3", "ripsh4", "ripsh5"
};

static const char *SAFARI_SERIES[4] = {
	"rip safari walker position 1",
	"rip safari walker position 2",
	"rip safari walker position 3",
	"rip safari walker position 4"
};
static const int16 SAFARI_SERIES_DIRS[] = {
	0, 1, 2, 3, 4, -4
};

static const char *SAFARI_SHADOWS[8] = {
	"safari shadow 1", "safari shadow 2", "safari shadow 3",
	"safari shadow 4", "safari shadow 5", "trek feng walker pos 3",
	"trek feng walker pos 4", "trek feng walker pos 5"
};
static const int16 SAFARI_SHADOWS_DIRS[6] = {
	10, 11, 12, 13, 14, -1
};

void Walker::player_walker_callback(frac16 myMessage, machine *sender) {
	int32 triggerType = _G(globals)[GLB_TEMP_1] >> 16;
	//int32 subVal = _G(globals)[GLB_TEMP_3] >> 16;

	switch (triggerType) {
	case 0:
	default:
		// Ignore this trigger, it's not important
		return;

	case 1:
		break;

	case 2:
		if (walker_has_walk_finished(sender))
			sendWSMessage(0x30000, 0, nullptr, 0, nullptr, 1);
		else
			_G(player).waiting_for_walk = false;
		break;

	case 3:
		_G(player).waiting_for_walk = false;
		break;
	}

	if (triggerType >= 0)
		kernel_trigger_dispatchx(myMessage);
}

bool Walker::walk_load_walker_and_shadow_series() {
	switch (_G(player).walker_type) {
	case 0:
		return ws_walk_load_walker_series(RIPLEY_SERIES_DIRS, RIPLEY_SERIES, true) &&
			ws_walk_load_shadow_series(RIPLEY_SHADOWS_DIRS, RIPLEY_SHADOWS);
	case 1:
		return ws_walk_load_walker_series(SAFARI_SERIES_DIRS, SAFARI_SERIES, true) &&
			ws_walk_load_shadow_series(SAFARI_SHADOWS_DIRS, SAFARI_SHADOWS);
	default:
		return false;
	}
}

machine *Walker::walk_initialize_walker() {
	machine *m;
	int32 s;

	if (!_G(player).walker_in_this_scene) {
		_G(player).walker_visible = false;
		m = nullptr;

	} else {
		_G(player).walker_visible = true;

		// Wilbur walker
		_G(player).walker_type = WALKER_PLAYER;
		_G(player).shadow_type = SHADOW_PLAYER;

		_G(globals)[GLB_TEMP_1] = _G(player).walker_type << 16;
		_G(globals)[GLB_TEMP_2] = WALKER_SERIES_HASH << 24;  // starting series hash of default walker	        GAMECTRL loads shadows starting @ 0
		_G(globals)[GLB_TEMP_3] = SHADOW_SERIES_HASH << 24;  // starting series hash of default walker shadows. GAMECTRL loads shadows starting @ 10

		// initialize with bogus data (this is for the real walker)
		s = _G(globals)[GLB_MIN_SCALE] + FixedMul((400 << 16) - _G(globals)[GLB_MIN_Y], _G(globals)[GLB_SCALER]);
		_G(globals)[GLB_TEMP_4] = 320 << 16;
		_G(globals)[GLB_TEMP_5] = 400 << 16;
		_G(globals)[GLB_TEMP_6] = s;
		_G(globals)[GLB_TEMP_7] = 3 << 16;	 // facing

		m = TriggerMachineByHash(WALKER_HASH, nullptr, _G(player).walker_type + WALKER_HASH, 0, player_walker_callback, false, "PLAYER WALKER");

		// we need to all init sequences to happen immediately (init coordinates)
		cycleEngines(nullptr, &(_G(currentSceneDef).depth_table[0]),
			nullptr, (uint8 *)&_G(master_palette)[0], _G(inverse_pal)->get_ptr(), true);

		_G(inverse_pal)->release();
	}

	return m;
}

void Walker::reset_walker_sprites() {
	error("TODO: reset_walker_sprites");
}

void Walker::unloadSprites() {
	if (_G(player).walker_in_this_scene) {
		term_message("Unloading Wilbur walker...");
		player_update_info();

		// Send message for the unload
		sendWSMessage(0x60000, 0, _G(my_walker), 0, nullptr, 1);
		_G(player).walker_in_this_scene = false;

		for (int i = 0; i < 7; ++i)
			series_unload(i);

		// TODO: This seems like it would leak memory
		_G(my_walker) = nullptr;
	}
}

bool Walker::ripley_said(const char *const list[][2]) {
	if (!list)
		return false;

	for (int index = 0; list[index][0]; ++index) {
		if (player_said(list[index][0])) {
			digi_play(list[index][1], 1);
			return true;
		}
	}

	return false;
}

void enable_player() {
	player_set_commands_allowed(true);
	ws_unhide_walker(_G(my_walker));
}

void disable_player() {
	player_set_commands_allowed(false);
	ws_hide_walker(_G(my_walker));
}

void player_walk_to(int32 x, int32 y, int32 facing_x, int32 facing_y, int trigger) {
#ifdef TODO
	_G(player_facing_x) = facing_x;
	_G(player_facing_y) = facing_y;
	_G(player_trigger) = trigger;
	player_hotspot_walk_override(x, y, -1, gSET_FACING);
#else
	error("TODO: player_walk_to");
#endif
}

void player_walk_to(int32 x, int32 y, int trigger) {
#ifdef TODO
	player_walk_to(x, y, _G(hotspot_x), _G(hotspot_y), trigger);
#else
	error("TODO: player_walk_to");
#endif
}

} // namespace Riddle
} // namespace M4

