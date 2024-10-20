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
#include "m4/adv_r/adv_control.h"
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
#define WALKER_PLAYER          0
#define WALKER_ALT          1

// These are the shadow types
#define SHADOW_PLAYER          0
#define SHADOW_ALT          1

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
			if (!_G(flags)[kDisableFootsteps])
				_G(digi).playFootsteps();
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
			if (!_G(flags)[kDisableFootsteps])
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
}

bool Walker::walk_load_walker_and_shadow_series() {
	return ws_walk_load_walker_series(WILBUR_SERIES_DIRS, WILBUR_SERIES, true) &&
		ws_walk_load_shadow_series(WILBUR_SHADOWS_DIRS, WILBUR_SHADOWS);
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
		_G(globals)[GLB_TEMP_4] = static_cast<uint32>(-320) << 16;
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

void Walker::unloadSprites() {
	if (_G(player).walker_in_this_scene) {
		term_message("Unloading Wilbur walker...");
		player_update_info();

		// Send message for the unload
		sendWSMessage(0x60000, 0, _G(my_walker), 0, nullptr, 1);
		_G(player).walker_in_this_scene = false;

		for (int i = 0; i < 7; ++i)
			series_unload(i);

		_G(my_walker) = nullptr;
	}
}

void Walker::wilbur_speech(const char *name, int trigger, int room, byte flags, int vol, int channel) {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	_name = name;
	_channel = channel;
	_room = room;
	_vol = vol;
	_trigger = kernel_trigger_create(trigger);
	_animateLips = (flags & 1) == 0;

	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(kWILBURS_SPEECH_START);
	_G(kernel).trigger_mode = oldMode;
}

void Walker::wilbur_say() {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;

	if (_animateLips && _G(player).walker_in_this_scene && _G(player).walker_visible)
		sendWSMessage(0x140000, 0, _G(my_walker), 0, 0, 1);

	term_message("wilbur_say:  wilburs_talk_trigger = %d", _trigger);
	digi_stop(_channel);

	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(kWILBUR_SPEECH_STARTED);
	digi_play(_name, _channel, _vol, kWILBURS_SPEECH_FINISHED, _room);

	_G(kernel).trigger_mode = oldMode;
}

bool Walker::wilbur_said(const char *list[][4]) {
	if (!list)
		return false;

	for (int index = 0; list[index][0]; ++index) {
		if (player_said(list[index][0])) {
			if (player_said("look at") && list[index][1]) {
				wilbur_speech(list[index][1]);
				return true;
			} else if (player_said("take") && list[index][2]) {
				wilbur_speech(list[index][2]);
				return true;
			} else if (player_said("gear") && list[index][3]) {
				wilbur_speech(list[index][3]);
				return true;
			} else {
				return false;
			}
		}
	}

	return false;
}

void Walker::wilburs_speech_finished() {
	if (_animateLips && _G(player).walker_in_this_scene && _G(player).walker_visible)
		sendWSMessage(0x150000, 0, _G(my_walker), 0, 0, 1);

	term_message("wilburs_speech_finished: dispatching wilburs_talk_trigger = %d", _trigger);
	kernel_trigger_dispatchx(_trigger);
}

void enable_player() {
	player_set_commands_allowed(true);
	ws_unhide_walker(_G(my_walker));
}

void disable_player() {
	player_set_commands_allowed(false);
	ws_hide_walker(_G(my_walker));
}

void wilbur_abduct(int trigger) {
	player_set_commands_allowed(false);
	digi_stop(1);

	if (_G(executing) == INTERACTIVE_DEMO) {
		_G(game).setRoom(608);
		return;
	}

	digi_preload("999_004");

	if (_G(my_walker) && _G(player).walker_in_this_scene && _G(player).walker_visible) {
		player_update_info();

		switch (_G(player_info).facing) {
		case 1:
		case 2:
			series_play("999ab02", _G(player_info).depth, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			series_play("999ab02s", _G(player_info).depth + 1, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 3:
		case 4:
		case 5:
			series_play("999ab04", _G(player_info).depth, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			series_play("999ab04s", _G(player_info).depth + 1, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 7:
		case 8:
		case 9:
			series_play("999ab08", _G(player_info).depth, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			series_play("999ab08s", _G(player_info).depth + 1, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		case 10:
		case 11:
			series_play("999ab10", _G(player_info).depth, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			series_play("999ab10s", _G(player_info).depth + 1, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;

		default:
			series_play("999ab02", _G(player_info).depth, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			series_play("999ab02s", _G(player_info).depth + 1, 0, -1, 6, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			break;
		}

		ws_hide_walker();
	}

	digi_play("999_004", 2, 255, -1);
	kernel_timing_trigger_daemon(180, trigger);

	if (!_G(flags)[V154] || imath_rand_bool(5)) {
		digi_preload("402w005z");
		digi_play("402w005z", 1, 255, -1);
		_G(flags)[V154] = 1;
	}
}

void Walker::speech_random(int count, int trigger,
		const char *name1, const char *name2, const char *name3,
		const char *name4, const char *name5, const char *name6,
		const char *name7, const char *name8, const char *name9) {
	const char *names[9] = {
		name1, name2, name3, name4, name5, name6, name7, name8, name9
	};

	wilbur_speech(names[imath_ranged_rand(1, count) - 1], trigger);
}

bool Walker::wilbur_parser(const char **list) {
	for (const char **curr = list; *curr; ++curr) {
		if (player_said(*curr)) {
			// Found section for the item
			bool useDefault = true;
			++curr;
			while (*curr) {
				const char *action = *curr++;
				const char *sound = *curr++;

				if (player_said(action)) {
					// Found the action on the item we want
					if (sound) {
						// Has a sound
						wilbur_speech(sound);
						return true;
					} else {
						// No sound for this
						useDefault = false;
					}
				}
			}

			// Check default fallback
			const char *defaultSound = *++curr;
			if (defaultSound && useDefault) {
				wilbur_speech(defaultSound);
				return true;
			}

			// Skip past the item end entry
			++curr;

		} else {
			// Not the item we want, so skip the section
			for (; !*curr || scumm_stricmp(*curr, PARSER_ITEM_END); ++curr) {
			}
		}
	}

	return false;
}

bool Walker::wilbur_match(const WilburMatch *list) {
	for (; list->_word0 || list->_word1; ++list) {
		const WilburMatch &m = *list;

		if (player_said(m._word0, m._word1)) {
			term_message("matched %s and %s", m._word0, m._word1);
			term_message("test variable:%d  value:%d",
				m._testVariable ? *m._testVariable : 0, m._testValue);

			if (!m._testVariable || *m._testVariable == m._testValue) {
				if (m._newVariable)
					*m._newVariable = m._newValue;

				if (m._trigger != -1)
					kernel_trigger_dispatch_now(m._trigger);

				return true;
			}
		}
	}

	return false;
}

void Walker::wilbur_poof() {
	player_update_info();
	_wilburPoof = series_load("999poof");
	series_play("999poof", _G(player_info).depth, 0, kUNPOOF, 6, 0,
		_G(player_info).scale, _G(player_info).x, _G(player_info).y);
	digi_play("999_003", 1, 255);
}

void Walker::wilbur_unpoof() {
	series_unload(_wilburPoof);
	_wilburPoof = -1;
}


void player_walk_to(int32 x, int32 y, int32 facing_x, int32 facing_y, int trigger) {
	_G(player_facing_x) = facing_x;
	_G(player_facing_y) = facing_y;
	_G(player_trigger) = trigger;
	player_hotspot_walk_override(x, y, -1, kSET_FACING);
}

void player_walk_to(int32 x, int32 y, int trigger) {
	player_walk_to(x, y, _G(player).click_x, _G(player).click_y, trigger);
}

void wilbur_speech(const char *name, int trigger, int room, byte flags,
		int vol, int channel) {
	_G(walker).wilbur_speech(name, trigger, room, flags, vol, channel);
}


} // namespace Burger
} // namespace M4
