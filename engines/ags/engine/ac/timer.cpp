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
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

const auto MAXIMUM_FALL_BEHIND = 3;

std::chrono::microseconds GetFrameDuration() {
	if (_G(framerate_maxed)) {
		return std::chrono::microseconds(0);
	}
	return _G(tick_duration);
}

void setTimerFps(int new_fps) {
	_G(tick_duration) = std::chrono::microseconds(1000000LL / new_fps);
	_G(framerate_maxed) = new_fps >= 1000;

	_G(last_tick_time) = AGS_Clock::now();
	_G(next_frame_timestamp) = AGS_Clock::now();
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

	::AGS::g_vm->_rawScreen->update();
}

void skipMissedTicks() {
	_G(last_tick_time) = AGS_Clock::now();
	_G(next_frame_timestamp) = AGS_Clock::now();
}

} // namespace AGS3
