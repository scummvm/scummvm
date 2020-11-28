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

#ifndef AGS_ENGINE_AC_TIMER_H
#define AGS_ENGINE_AC_TIMER_H

#include "ags/lib/std/type_traits.h"
#include "ags/lib/std/chrono.h"
#include "ags/lib/std/xtr1common.h"

namespace AGS3 {

// use high resolution clock only if we know it is monotonic/steady.
// refer to https://stackoverflow.com/a/38253266/84262
using AGS_Clock = std::conditional<
	std::chrono::high_resolution_clock::is_steady,
	std::chrono::high_resolution_clock, std::chrono::steady_clock
>::type;

extern void WaitForNextFrame();

// Sets real FPS to the given number of frames per second; pass 1000+ for maxed FPS mode
extern void setTimerFps(int new_fps);
// Tells whether maxed FPS mode is currently set
extern bool isTimerFpsMaxed();
extern bool waitingForNextTick();  // store last tick time.
extern void skipMissedTicks();  // if more than N frames, just skip all, start a fresh.

} // namespace AGS3

#endif
