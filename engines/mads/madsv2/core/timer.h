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

#ifndef MADS_CORE_TIMER_H
#define MADS_CORE_TIMER_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {

extern long *timer_address;         /* "Default" timer count address */
extern word timer_rate;                 /* Current timer ticks/sec       */
extern int  timer_service_active;       /* Flag if timer service active  */

extern word timer_sound_on;             /* Flag if sound service active */
extern word timer_noise_on;             /* Flag if sound in noise mode  */

extern word timer_low_priority;         /* Flag if low priority routine active */
extern word timer_low_semaphore;        /* Low priority activity semaphore     */
extern word timer_low_stacking;         /* Low priority stacking count         */
extern word timer_low_deferred;         /* Low priority deferred DOS flag      */
extern void *timer_low_routine;     /* Low priority function pointer       */

extern long timer_600_low;             /* 600/s timer count */
extern long timer_60_low;              /* 60/s timer_count  */
extern long timer_dos_low;


/* extern byte *interrupt_stack_pointer; */
/* extern word interrupt_stack_size;         */

/**
 * Reads system clock, returns number of ticks since startup (1 tick = 54.9ms)
 */
long timer_read();
inline long timer_read_dos() {
	return timer_read();
}

long timer_read_600();
long timer_read_60();
void timer_set_rate(word count_down);

/**
 * Installs the timer interrupt handler
 */
void timer_install();

/**
 * Removes timer interrupt handler
 */
void timer_remove();

/**
 * Activates or deactivates timer sound support.
 */
void timer_set_sound_flag(int sound_flag);

/**
 * Designates a low priority routine.  Pass NULL (0) to deactivate.
 */
void timer_activate_low_priority(void (*(routine))());

byte *timer_get_interrupt_stack();

int timer_set_copy_protect(int protect);
int timer_get_copy_protect();

} // namespace MADSV2
} // namespace MADS

#endif
