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

#ifndef M4_ADV_R_KERNEL_H
#define M4_ADV_R_KERNEL_H

#include "m4/adv_r/adv.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

#define CACHE_NOT_OVERRIDE_BY_FLAG_PARSE 2
#define KERNEL_RESTORING_GAME -2
#define KERNEL_SCRATCH_SIZE 256	// Size of game scratch area

struct Kernel {
	uint32 scratch[KERNEL_SCRATCH_SIZE];  // Scratch variables for room
	bool hag_mode = true;

	uint32 	clock = 0;						// Current game timing clock
	int32 trigger = 0;						// Game trigger code, if any
	int32 letter_box_x = 0;
	int32 letter_box_y = 0;
	int32 restore_slot = -1;
	int16 first_non_walker_cel_hash = 0;
	int16 last_save = 0;					// Most recent save slot #

	char save_file_name[8] = { 0 };
	bool restore_game = false;				// TRUE if we wanna restore
	bool teleported_in = false;				// Flag if player teleported to room

	int32 fade_up_time = 0;
	int16 first_fade = 0;
	bool fading_to_grey = false;
	bool suppress_fadeup = false;
	bool force_restart = false;

	bool pause = false;

	KernelTriggerType trigger_mode = KT_DAEMON;	// trigger was/is invoked in this mode
	bool call_daemon_every_loop = false;
	bool continue_handling_trigger = true;	// set to True in apps code when trigger is to 
											// be handled by the next layer (scene/section/global daemon code)
	int suppress_cache = CACHE_NOT_OVERRIDE_BY_FLAG_PARSE;
	bool start_up_with_dbg_ws = false;
	bool use_debug_monitor = false;
	bool use_log_file = false;
	bool track_open_close = false;
	bool going = false;
	bool camera_pan_instant = false;
	bool unused = false;

	size_t mem_avail() const { return 7999999; }
};

} // namespace M4

#endif
