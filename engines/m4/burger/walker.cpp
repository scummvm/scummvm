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

#include "m4/burger/walker.h"
#include "m4/burger/vars.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {

// Starting hashes for walker machines/sequences/etc */
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

const char **Walker::myFootsteps;
int32 Walker::numFootstepSounds;

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
		if (myMessage >> 16 >= 0) {				// If user trigger is desired, dispatch it
			kernel_trigger_dispatch(myMessage);	// Trigger will go to where it was called from
		}
		break;

	case 2:
		// walker has arrived at a node
		if (walker_has_walk_finished(sender)) {
			// Walks walker to next node if not at end of walk
			SendMessage(ENDWALK << 16, 0, sender, 0, nullptr, 1);
		}
		break;

	case 3:
		// Walker has finished his walk and is facing final direction
		_G(player).waiting_for_walk = false;
		if (myMessage >> 16 >= 0) {              // if user trigger is desired, dispatch it
			kernel_trigger_dispatch(myMessage);  // trigger will go to where it was called from
		}
		break;

	case 20:
		// Walker wants to make a sound
		soundNumber = myMessage >> 16;
		switch (soundNumber) {
		case 21:
			if (myFootsteps) {
				digi_play(myFootsteps[imath_ranged_rand(0, numFootstepSounds - 1)], 1, 50, NO_TRIGGER, GLOBAL_SCENE);
			}
			break;
		case 22:
			if (myFootsteps) {
				digi_play(myFootsteps[imath_ranged_rand(0, numFootstepSounds - 1)], 1, 50, NO_TRIGGER, GLOBAL_SCENE);
			}
			break;
		case 23:
			term_message("Wilbur: crack");
			digi_play("crack1", 1, 50, NO_TRIGGER, GLOBAL_SCENE);
			break;
		case 24:
			term_message("Wilbur: hmmm");
			digi_play("hmmm", 1, 60, NO_TRIGGER, GLOBAL_SCENE);
			break;
		case 25:
			if (myFootsteps) {
				digi_play(myFootsteps[imath_ranged_rand(0, numFootstepSounds - 1)], 1, 50, NO_TRIGGER, GLOBAL_SCENE);
			}
			break;
		}
		break;

	default:
		_G(player).waiting_for_walk = false;
		break;
	}
#else
	error("TODO: player_walker_callback");
#endif
}

bool Walker::walk_load_walker_and_shadow_series() {
	return ws_walk_load_walker_series(WILBUR_SERIES_DIRS, WILBUR_SERIES, true) &&
		ws_walk_load_shadow_series(WILBUR_SHADOWS_DIRS, WILBUR_SHADOWS);
}

machine *Walker::walk_initialize_walker() {
	machine *m;
	int32 s;

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
	CycleEngines(nullptr, &(_G(currentSceneDef).depth_table[0]),
		nullptr, (uint8 *)&_G(master_palette)[0], _G(inverse_pal)->get_ptr(), true);

	_G(inverse_pal)->release();

	return m;
}

void Walker::reset_walker_sprites() {
	if (_G(roomVal3)) {
		for (int i = 0; WILBUR_SERIES_DIRS[i] != -1; ++i) {
			series_load(WILBUR_SERIES[i], WILBUR_SERIES_DIRS[i]);
		}
	}

	ws_unhide_walker(_G(my_walker));
	gr_restore_palette();
	kernel_timing_trigger(1, 1026);
}

void Walker::wilbur_speech(const char *name, int trigger, int room, byte flags, int vol, int channel) {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	_name = name;
	_channel = channel;
	_room = room;
	_vol = vol;
	_trigger = kernel_trigger_create(trigger);
	_flag = (flags & 1) != 0;

	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(gWILBURS_SPEECH_START);
	_G(kernel).trigger_mode = oldMode;
}

void Walker::wilbur_say() {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;

	if (_flag && _G(player).walker_in_this_scene && _G(roomVal2))
		SendWSMessage(0x140000, 0, _G(my_walker), 0, 0, 1);

	term_message("wilbur_say:  wilburs_talk_trigger = %d", _trigger);
	digi_stop(_channel);

	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(10013);
	digi_play(_name, _channel, _vol, gWILBURS_SPEECH_FINISHED, _room);

	_G(kernel).trigger_mode = oldMode;
}

void Walker::wilburs_speech_finished() {
	if (_flag && !_G(player).walker_in_this_scene && !_G(roomVal2))
		SendWSMessage(0x150000, 0, _G(my_walker), 0, 0, 1);

	term_message("wilburs_speech_finished: dispatching wilburs_talk_trigger = %d", _trigger);
	kernel_trigger_dispatchx(_trigger);
}

} // namespace Burger
} // namespace M4
