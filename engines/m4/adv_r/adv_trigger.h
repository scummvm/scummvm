
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

namespace M4 {

extern int32 kernel_trigger_create(int32 trigger_num);
extern int32 kernel_trigger_create_mode(int32 trigger_num, int32 desired_mode);
extern bool kernel_trigger_dispatch(int32 trigger_num);
extern bool kernel_trigger_dispatch_now(int32 trigger_num);

extern void kernel_timing_trigger(int32 ticks, int16 trigger, char *name = NULL);

extern void cisco_dispatch_triggers();
extern void cisco_clear_triggers();
extern void kernel_service_timing_trigger_q();

} // End of namespace M4

#endif
