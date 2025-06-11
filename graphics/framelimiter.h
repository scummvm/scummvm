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

#ifndef GFX_FRAMELIMITER_H
#define GFX_FRAMELIMITER_H

#include "common/system.h"

namespace Graphics {

/**
 * A framerate limiter
 *
 * Ensures the framerate does not exceed the specified value
 * by delaying until all of the timeslot allocated to the frame
 * is consumed.
 * Allows to curb CPU usage and have a stable framerate.
 * If deferToVsync is set true (default), framerate will only be limited to the set value when hardware vsync is inactive, otherwise it will track Vsync directly.
 * If deferToVsync is set false, framerate will always be limited to the set value, whether or not hardware vsync is active.
 * In order for hardware vsync status detection to work correctly, initialize() must be called AFTER initGraphics; see below.
 */
class FrameLimiter {
public:
	FrameLimiter(OSystem *system, const uint framerate, const bool deferToVsync = true);
	/**
	 * Always call this after instantiating Screen & calling initGraphics(), to ensure Vsync status is detected correctly!
	 */
	void initialize();
	void initialize(const uint framerate);

	/**
	 * Call immediately before starting game logic cycle, and immediately after previous screen->update(), if any.
	 * Returns last frame duration.
	 */
	uint startFrame();

	/**
	 * Call immediately after game logic cycle is completed, and immediately before screen->update().
	 * If Vsync is inactive, will wait and return at the appropriate moment to call screen->update() in order to maintain the specified FPS.
	 * If Vsync is active, will return immediately as screen->update() should automatically wait for the right moment.
	 * Returns true if next frame is expected to be late.
	 */
	bool delayBeforeSwap();
	void pause(bool pause);
	/**
	 * Return duration of last whole frame, including game logic, frame limiter or Vsync delay (if any), and screen redraw time.
	 * Specifically, this is the time between the last two successive startFrame() calls.
	 */
	uint getLastFrameDuration() const {
		return _frameDuration;
	}
	/**
	 * Return duration of last screen update
	 * If Vsync is inactive, this value will just be the duration of the redraw process itself;
	 * If Vsync is active, this value will encompass both the redraw time AND the delay between presenting the frame to redraw and the screen redraw actually starting.
	 */
	uint getLastDrawDuration() const {
		return _drawDuration;
	}
	/**
	 * Return duration of last game logic loop
	 * Specifically, this is the time from when startFrame() was last called to when delayBeforeSwap() was last called
	 */
	uint getLastLoopDuration() const {
		return _loopDuration;
	}
	/**
	 * If true, framelimiter is active and applying _system->delayMillis(delay) to maintain the specified FPS, if valid.
	 * If false, framelimiter is inactive, either because supplied FPS was invalid or because Vsync is active.
	 * delayBeforeSwap() should still be called even if inactive, in order to ensure timers are updated correctly.
	 */
	bool isEnabled() const {
		return _enabled;
	}

private:
	OSystem *_system;

	bool _enabled;
	bool _deferToVsync;
	uint _frameStart = 0;  // Time at which screen update completed and startFrame() was called; start of next cycle of game logic
	uint _frameLimit = 0;  // Target frame duration to achieve specified FPS
	uint _frameDuration = 0; // Duration of previous frame between successive startFrame() calls; total game logic, delay (if any) and screen update time
	uint _drawStart = 0;  // Time at which delayBeforeSwap() returns
	uint _drawDuration = 0;  // Measured screen update time
	uint _loopDuration = 0;  // Duration of last game logic cycle, from when startFrame() was called to when delayBeforeSwap() was called
	int _delay = 0; // Time to delay before returning from delayBeforeSwap()
	uint _now = 0; // Current time
};

} // End of namespace Graphics

#endif // GFX_FRAMELIMITER_H
