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

// These are the walker types
#define WALKER_WILBUR          0
#define WALKER_FLUMIX          1

// These are the shadow types
#define SHADOW_WILBUR          0
#define SHADOW_FLUMIX          1

static const char *WILBUR_SERIES[8] = {
	"WILBUR01", "WILBUR02", "WILBUR03", "WILBUR04", "WILBUR05",
	"WILBUR07", "WILBUR08", "WILBUR09"
};
static const int16 WILBUR_SERIES_DIRS[] = {
	0, 1, 2, 3, 4, 5, 6, 7, -1
};

static const char *WILBUR_SHADOWS[5] = {
	"WILBUR01_SHADOW", "WILBUR02_SHADOW", "WILBUR03_SHADOW",
	"WILBUR04_SHADOW", "WILBUR05_SHADOW"
};
static const int16 WILBUR_SHADOWS_DIRS[6] = {
	8, 9, 10, 11, 12, -1
};

void Walker::player_walker_callback(frac16 myMessage, machine *sender) {
#ifdef TODO
	int32 triggerType, soundNumber;

	triggerType = _G(globals)[GLB_TEMP_1] >> 16;

	switch (triggerType) {
	case 0:
		// Ignore this trigger, it's not important
		break;

	case 1:
		// Specific action is finished
		// If user trigger is desired, dispatch it
		if (myMessage >> 16 >= 0)
			// Trigger will go to where it was called from
			kernel_trigger_dispatchx(myMessage);
		break;

	case 2:
		// Walker has arrived at a node
		if (walker_has_walk_finished(sender)) {
			// Walks walker to next node if not at end of walk
			sendWSMessage(ENDWALK << 16, 0, sender, 0, nullptr, 1);
		}
		break;

	case 3:
		// Walker has finished his walk and is facing final direction
		_G(player).waiting_for_walk = false;

		// if user trigger is desired, dispatch it
		if (myMessage >> 16 >= 0)
			// trigger will go to where it was called from
			kernel_trigger_dispatchx(myMessage);
		break;

	case 20:
		// Walker wants to make a sound
		soundNumber = myMessage >> 16;
		switch (soundNumber) {
		case 21:
		case 22:
		case 25:
			if (!_G(flags)[V298])
				_G(digi).playRandom();
			break;

		case 23:
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				digi_play("crack1", 1, 50, NO_TRIGGER, GLOBAL_SCENE);
				break;
			case 2:
				digi_play("crack2", 1, 60, NO_TRIGGER, GLOBAL_SCENE);
				break;
			case 3:
				digi_play("crack3", 1, 80, NO_TRIGGER, GLOBAL_SCENE);
				break;
			default:
				break;
			}
			break;

		case 24:
			if (!_G(flags)[V298])
				digi_play("hmmm", 1, 60, NO_TRIGGER, GLOBAL_SCENE);
			break;

		default:
			break;
		}
		break;

	default:
		_G(player).waiting_for_walk = false;
		break;
	}
#endif
}

bool Walker::walk_load_walker_and_shadow_series() {
	return ws_walk_load_walker_series(WILBUR_SERIES_DIRS, WILBUR_SERIES, true) &&
		ws_walk_load_shadow_series(WILBUR_SHADOWS_DIRS, WILBUR_SHADOWS);
}

machine *Walker::walk_initialize_walker() {
#ifdef TODO
	machine *m;
	int32 s;

	if (!_G(player).walker_in_this_scene) {
		_G(player).walker_visible = false;
		m = nullptr;

	} else {
		_G(player).walker_visible = true;

		// Wilbur walker
		_G(player).walker_type = WALKER_WILBUR;
		_G(player).shadow_type = SHADOW_WILBUR;

		_G(globals)[GLB_TEMP_1] = _G(player).walker_type << 16;
		_G(globals)[GLB_TEMP_2] = WALKER_SERIES_HASH << 24;  // starting series hash of default walker	        GAMECTRL loads shadows starting @ 0
		_G(globals)[GLB_TEMP_3] = SHADOW_SERIES_HASH << 24;  // starting series hash of default walker shadows. GAMECTRL loads shadows starting @ 10

		// initialize with bogus data (this is for the real walker)
		s = _G(globals)[GLB_MIN_SCALE] + FixedMul((400 << 16) - _G(globals)[GLB_MIN_Y], _G(globals)[GLB_SCALER]);
		_G(globals)[GLB_TEMP_4] = 320 << 16;
		_G(globals)[GLB_TEMP_5] = 400 << 16;
		_G(globals)[GLB_TEMP_6] = s;
		_G(globals)[GLB_TEMP_7] = 3 << 16;	 // facing

		m = TriggerMachineByHash(WALKER_HASH, nullptr, _G(player).walker_type + WALKER_HASH, 0, player_walker_callback, false, "Wilbur Walker");

		// we need to all init sequences to happen immediately (init coordinates)
		cycleEngines(nullptr, &(_G(currentSceneDef).depth_table[0]),
			nullptr, (uint8 *)&_G(master_palette)[0], _G(inverse_pal)->get_ptr(), true);

		_G(inverse_pal)->release();
	}

	return m;
#else
	return nullptr;
#endif
}

void Walker::reset_walker_sprites() {
#ifdef TODO
	if (_G(roomVal3)) {
		for (int i = 0; WILBUR_SERIES_DIRS[i] != -1; ++i) {
			series_load(WILBUR_SERIES[i], WILBUR_SERIES_DIRS[i]);
		}
	}

	ws_unhide_walker(_G(my_walker));
	gr_restore_palette();
	kernel_timing_trigger(1, 1026);
#endif
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
#endif
}

void player_walk_to(int32 x, int32 y, int trigger) {
#ifdef TODO
	player_walk_to(x, y, _G(hotspot_x), _G(hotspot_y), trigger);
#endif
}

} // namespace Riddle
} // namespace M4

