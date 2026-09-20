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

#include "frame_clock.h"

namespace Scooby {

FrameClock::FrameClock() : _limiter(g_system, kFramesPerSecond) {
}

void FrameClock::waitForNextFrame() {
	_limiter.delayBeforeSwap();
	// _nextFrameTimestamp += Std::chrono::milliseconds(1000 / kFramesPerSecond);
	// const std::chrono::nanoseconds spinThreshold(1000000); // 1ms, mirrors Stopwatch.Frequency / 1000
	// while (true) {
	// 	Std::chrono::steady_clock::duration remaining = _nextFrameTimestamp - Std::chrono::steady_clock::now();
	// 	if (remaining.count() <= 0) {
	// 		return;
	// 	}
	//
	// 	if (remaining > spinThreshold) {
	// 		std::this_thread::yield();
	// 	} else {
	// 		// No portable std::this_thread::spin equivalent; a short sleep_for(0)
	// 		// behaves like Thread.Yield()'s busy-wait fallback closely enough
	// 		// for a sub-millisecond remainder.
	// 		// std::this_thread::sleep_for(Std::chrono::microseconds(0));
	// 	}
	// }
}

void FrameClock::startFrame() {
	_limiter.startFrame();
}
} // namespace Scooby