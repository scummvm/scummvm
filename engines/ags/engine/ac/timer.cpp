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

#include "ags/lib/std/thread.h"
#include "ags/engine/ac/timer.h"
#include "ags/shared/core/platform.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

namespace {
const auto MAXIMUM_FALL_BEHIND = 3;
}

std::chrono::microseconds GetFrameDuration() {
	if (_G(framerate_maxed)) {
		return std::chrono::microseconds(0);
	}
	return _G(tick_duration);
}

int setTimerFps(int new_fps) {
	int old_fps = _G(framerate);
	_G(tick_duration) = std::chrono::microseconds(1000000LL / new_fps);
	_G(framerate) = new_fps;
	_G(framerate_maxed) = new_fps >= 1000;

	_G(last_tick_time) = AGS_Clock::now();
	_G(next_frame_timestamp) = AGS_Clock::now();
	return old_fps;
}

bool isTimerFpsMaxed() {
	return _G(framerate_maxed);
}

void WaitForNextFrame() {
	auto now = AGS_Clock::now();
	auto frameDuration = GetFrameDuration();

	// early exit if we're trying to maximise framerate
	if (frameDuration <= std::chrono::milliseconds::zero()) {
		_G(next_frame_timestamp) = now;
		// suspend while the game is being switched out
		while (_G(game_update_suspend) && !_G(want_exit) && !_G(abort_engine)) {
			sys_evt_process_pending();
			_G(platform)->YieldCPU();
		}
		return;
	}

	// jump ahead if we're lagging
	if (_G(next_frame_timestamp) < (now - MAXIMUM_FALL_BEHIND * frameDuration)) {
		_G(next_frame_timestamp) = now;
	}

	if (_G(next_frame_timestamp) > now) {
		auto frame_time_remaining = _G(next_frame_timestamp) - now;
		std::this_thread::sleep_for(frame_time_remaining);
	}

	_G(next_frame_timestamp) += frameDuration;

	// suspend while the game is being switched out
	while (_G(game_update_suspend) && !_G(want_exit) && !_G(abort_engine)) {
		sys_evt_process_pending();
		_G(platform)->YieldCPU();
	}
}

void skipMissedTicks() {
	_G(last_tick_time) = AGS_Clock::now();
	_G(next_frame_timestamp) = AGS_Clock::now();
}

} // namespace AGS3
