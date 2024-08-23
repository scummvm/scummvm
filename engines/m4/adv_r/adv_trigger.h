
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

#ifndef M4_ADV_R_ADV_TRIGGER_H
#define M4_ADV_R_ADV_TRIGGER_H

#include "m4/m4_types.h"
#include "m4/adv_r/adv.h"

namespace M4 {

constexpr int NO_TRIGGER = -1;
constexpr int TENTH_SECOND = 6;
constexpr int MAX_TIMERS = 32;

struct Triggers {
	int32 _time_q[MAX_TIMERS];
	int32 _time_trigger_q[MAX_TIMERS];
	int32 _time_q_end = 0;

	int32 _sound_trigger_q[MAX_TIMERS];
	int32 _q_start = 0;
	int32 _q_end = 0;
};

/**
 * Create a new trigger
 * Trigger usage:
 * [31-28][27-16][15-0]
 *   |        |      |
 *   |        |      +--- trigger_num (0 - 0xffff)
 *   |        +--- room_id (0 - 4096) (0 - 0xfff)
 *   +--- trigger_mode (0-16)
 */
int32 kernel_trigger_create(int32 trigger_num);
int32 kernel_trigger_create_mode(int32 trigger_num, int32 desired_mode);

/**
 * Dispatches a trigger.
 * @returns		Returns true if the trigger was handled. If the trigger is for
 * a different room that current room_id, returns false. If no trigger_mode was
 * attached to the trigger, returns false
 */
bool kernel_trigger_dispatchx(int32 trigger_num);
bool kernel_trigger_dispatch_now(int32 trigger_num);
void kernel_timing_trigger(int32 ticks, int16 trigger, const char *name = nullptr);
void kernel_timing_trigger_daemon(int32 ticks, int16 trigger);
void kernel_timing_trigger(int32 ticks, int16 trigger,
	KernelTriggerType preMode, KernelTriggerType postMode);

void cisco_dispatch_triggers();
void cisco_clear_triggers();
void kernel_service_timing_trigger_q();

} // End of namespace M4

#endif
