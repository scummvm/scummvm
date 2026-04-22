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

#include "common/textconsole.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

long _timer_clock = 0;          /* Backing store for *timer_address writes */

long *timer_address = dos_timer_address;
word timer_rate = 20;
int  timer_service_active = false;
word timer_sound_on;
word timer_noise_on;
byte timer_copy_protect_in = 0;
byte timer_copy_protect_out = 0;

constexpr int interrupt_stack_size = 12800;
byte _interrupt_stack[interrupt_stack_size];

word timer_low_priority;
word timer_low_semaphore;
word timer_low_stacking;
word timer_low_deferred;
void *timer_low_routine;


void timer_install() {
	// No implementation in ScummVM
}

void timer_remove() {
	// No implementation in ScummVM
}

long timer_read() {
	// kernel.clock is compared against timing constants where ONE_SECOND = 60,
	// so this counter must run at 60 Hz.  The original DOS code achieved this
	// by pointing timer_address at the interrupt handler's timer_60_low counter;
	// here we derive the same rate directly from wall-clock milliseconds.
	unsigned long ms = g_engine->getMillis();
	return (long)(ms * 60 / 1000);
}

long timer_read_600() {
	unsigned long ms = g_engine->getMillis();
	return ms * 600 / 1000;
}

long timer_read_60() {
	unsigned long ms = g_engine->getMillis();
	return ms * 60 / 1000;
}

void timer_set_sound_flag(int sound_flag) {
	timer_sound_on = sound_flag;
	timer_noise_on = sound_flag;
}

int timer_set_copy_protect(int protect) {
	timer_copy_protect_in = protect;
	timer_copy_protect_out = timer_copy_protect_in;
	return protect;
}

int timer_get_copy_protect() {
	return timer_copy_protect_out;
}

void timer_activate_low_priority(void (*(routine))()) {
	warning("TODO: timer_activate_low_priority");
}

byte *timer_get_interrupt_stack() {
	return _interrupt_stack;
}

} // namespace MADSV2
} // namespace MADS
