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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DUNE_CLOCK_H
#define DUNE_CLOCK_H

#include "common/scummsys.h"

namespace Dune {

// EF6A decrements the word, then tests its SIGN, not whether it reached zero.
// In particular, 8000h wraps to 7FFFh and does not expire on its first tick.
inline uint32 timerTicksUntilHour(uint16 countdown) {
	return (int16)(uint16)(countdown - 1) < 0 ? 1 : (uint32)countdown + 1;
}

// D9D2 adds a 16-bit elapsed tick count to the callback's remainder BEFORE
// testing its period. Keep native sub-tick credit, but wrap the sum at the
// same 65536 five-millisecond ticks; a wrapped sum need not be due at all.
inline uint32 animationTimerElapsedMs(uint32 elapsedMs, uint32 remainderMs) {
	const uint32 wrapMs = 65536u * 5u;
	return (elapsedMs % wrapMs + remainderMs) % wrapMs;
}

// CAD4: silent films compare their own last-frame CE7A sample, not D9D2's
// accumulated callback remainder. CA59 rebases to now after one due frame.
inline bool silentMovieFrameDue(uint32 previousTick, uint32 nowTick, byte period) {
	return (uint16)(nowTick - previousTick) >= period;
}

// Exclude native GUI pause time without losing a partially displayed frame.
// A new film loaded while paused has no pre-pause interval to preserve.
inline uint32 resumeMovieTick(uint32 previousTick, uint32 pauseTick, uint32 resumeTick) {
	return previousTick + resumeTick - (previousTick > pauseTick ? previousTick : pauseTick);
}

// Versions 6..51 stored milliseconds into an artificial 60-second hour,
// leaving DS:46DB stale. Preserve their remaining time when migrating.
inline uint16 legacyTimerCountdown(uint32 elapsedMs) {
	return 11999 - (elapsedMs % 60000) / 5;
}

} // namespace Dune

#endif
