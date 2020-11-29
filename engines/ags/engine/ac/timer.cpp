/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/engine/ac/timer.h"
#include "ags/shared/core/platform.h"
#include "ags/lib/std/chrono.h"
#include "ags/lib/std/thread.h"
#if AGS_PLATFORM_DEBUG && defined (__GNUC__)
//include <stdio.h>
//include <execinfo.h>
//include <unistd.h>
#endif
#include "ags/engine/platform/base/agsplatformdriver.h"

namespace AGS3 {

namespace {

const auto MAXIMUM_FALL_BEHIND = 3;

auto tick_duration = std::chrono::microseconds(1000000LL / 40);
auto framerate_maxed = false;

uint32 last_tick_time = 0; // AGS_Clock::now();
uint32 next_frame_timestamp = 0; // AGS_Clock::now();

}

std::chrono::microseconds GetFrameDuration() {
	if (framerate_maxed) {
		return std::chrono::microseconds(0);
	}
	return tick_duration;
}

void setTimerFps(int new_fps) {
	tick_duration = std::chrono::microseconds(1000000LL / new_fps);
	framerate_maxed = new_fps >= 1000;

	last_tick_time = AGS_Clock::now();
	next_frame_timestamp = AGS_Clock::now();
}

bool isTimerFpsMaxed() {
	return framerate_maxed;
}

void WaitForNextFrame() {
	auto now = AGS_Clock::now();
	auto frameDuration = GetFrameDuration();

	// early exit if we're trying to maximise framerate
	if (frameDuration <= std::chrono::milliseconds::zero()) {
		next_frame_timestamp = now;
		return;
	}

	// jump ahead if we're lagging
	if (next_frame_timestamp < (now - MAXIMUM_FALL_BEHIND * frameDuration)) {
		next_frame_timestamp = now;
	}

	auto frame_time_remaining = next_frame_timestamp - now;
	if (frame_time_remaining > std::chrono::milliseconds::zero()) {
		std::this_thread::sleep_for(frame_time_remaining);
	}

	next_frame_timestamp += frameDuration;
}

bool waitingForNextTick() {
	auto now = AGS_Clock::now();

	if (framerate_maxed) {
		last_tick_time = now;
		return false;
	}

	auto is_lagging = (now - last_tick_time) > (MAXIMUM_FALL_BEHIND * tick_duration);
	if (is_lagging) {
#if AGS_PLATFORM_DEBUG && defined (__GNUC__)
		auto missed_ticks = ((now - last_tick_time) / tick_duration);
		warning("Lagging! Missed %lld ticks!\n", (long long)missed_ticks);
/*		void *array[10];
		auto size = backtrace(array, 10);
		backtrace_symbols_fd(array, size, STDOUT_FILENO);
*/
		warning("\n");
#endif
		last_tick_time = now;
		return false;
	}

	auto next_tick_time = last_tick_time + tick_duration;
	if (next_tick_time <= now) {
		last_tick_time = next_tick_time;
		return false;
	}

	return true;
}

void skipMissedTicks() {
	last_tick_time = AGS_Clock::now();
	next_frame_timestamp = AGS_Clock::now();
}

} // namespace AGS3
