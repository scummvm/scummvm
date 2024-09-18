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

#ifndef AGS_ENGINE_AC_TIMER_H
#define AGS_ENGINE_AC_TIMER_H

#include "common/std/type_traits.h"
#include "common/std/chrono.h"
#include "common/std/xtr1common.h"

#include "ags/lib/std.h"

namespace AGS3 {

// use high resolution clock only if we know it is monotonic/steady.
// refer to https://stackoverflow.com/a/38253266/84262
using AGS_Clock = std::conditional <
                  std::chrono::high_resolution_clock::is_steady,
                  std::chrono::high_resolution_clock, std::chrono::steady_clock
                  >::type;

template<typename TDur>
inline int64_t ToMilliseconds(TDur dur) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
}

// Sleeps for time remaining until the next game frame, updates next frame timestamp
extern void WaitForNextFrame();

// Sets real FPS to the given number of frames per second; pass 1000+ for maxed FPS mode
extern int setTimerFps(int new_fps);
// Tells whether maxed FPS mode is currently set
extern bool isTimerFpsMaxed();
// If more than N frames, just skip all, start a fresh.
extern void skipMissedTicks();

} // namespace AGS3

#endif
