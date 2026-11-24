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

#ifndef DUNE_ANIMATION_CLOCK_H
#define DUNE_ANIMATION_CLOCK_H

#include "common/scummsys.h"

namespace Dune {

// D9D2 samples CE7A once, stores DC68 and retains BX across every callback.
// A nested poll updates the shared DC68 but restores its caller's saved BX.
class AnimationClock {
	bool _sampled = false;
	uint32 _lastMillis = 0;
	bool _active = false;
	uint32 _millis = 0;
	uint16 _elapsed = 0;

public:
	class Poll {
		AnimationClock &_clock;
		bool _previousActive;
		uint32 _previousMillis;
		uint16 _previousElapsed;

	public:
		Poll(AnimationClock &clock, uint32 millis)
			: Poll(clock, millis, clock._sampled ? (uint16)(millis / 5 - clock._lastMillis / 5) : 0) {}

		// Explicit relative input is used by controlled DD63 battle waits.
		Poll(AnimationClock &clock, uint32 millis, uint16 elapsed)
			: _clock(clock), _previousActive(clock._active), _previousMillis(clock._millis),
			  _previousElapsed(clock._elapsed) {
			clock._sampled = true;
			clock._lastMillis = millis;
			clock._active = true;
			clock._millis = millis;
			clock._elapsed = elapsed;
		}

		~Poll() {
			_clock._active = _previousActive;
			_clock._millis = _previousMillis;
			_clock._elapsed = _previousElapsed;
			// Do not restore _lastMillis: a nested D9D2 really sampled DC68.
		}

		Poll(const Poll &) = delete;
		Poll &operator=(const Poll &) = delete;
	};

	bool active() const { return _active; }
	bool sampled() const { return _sampled; }
	uint32 millis() const { return _millis; }
	uint32 lastMillis() const { return _lastMillis; }
	uint16 elapsed() const { return _elapsed; }
	void rebase(uint32 millis) { _lastMillis = millis; _sampled = true; }
	void invalidate() { _sampled = false; }
};

} // namespace Dune

#endif
