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

#ifndef BLADERUNNER_FRAMELIMITER_H
#define BLADERUNNER_FRAMELIMITER_H

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

enum FramelimiterFpsRate {
	kFramelimiterDisabled       = 0,
	kFramelimiter15fps          = 1,
	kFramelimiter25fps          = 2,
	kFramelimiter30fps          = 3,
	kFramelimiter60fps          = 4,
	kFramelimiter120fps         = 5
};

class BladeRunnerEngine;

class Framelimiter {
	friend class Debugger;

public:
	static const FramelimiterFpsRate kDefaultFpsRate = kFramelimiter60fps;
	static const bool kDefaultUseDelayMillis = false;

private:
	BladeRunnerEngine *_vm;

	bool   _forceFirstPass;
	uint32 _speedLimitMs;

	// A pass is when a tick or while loop that contains a potential screen update is repeated
	// it's essentially when the check is made for a screen update
	// Not every pass will necessarily result in a screen update (because that's the purpose of the frame limiter)
	// So the "_startFrameTime" is not always equal to "_timeOfCurrentPass"
	uint32 _timeOfLastPass;
	uint32 _timeOfCurrentPass;

	uint32 _startFrameTime;      // is updated and valid, only if the current pass will result in a screen update (see method: shouldExecuteScreenUpdate())
	uint32 _lastFrameDurationMs; // can be used for average FPS calculation and display purposes when frame limiter is enabled

	bool   _enabled;
	bool   _useDelayMs;          // true: will use calls to delayMillis(), false: will use non-blocking software timer instead

public:
	Framelimiter(BladeRunnerEngine *vm, FramelimiterFpsRate framerateMode, bool useDelayMs);
	~Framelimiter();

//	void startFrame();
//	void delayBeforeSwap();

//	void pause(bool pause);

	void init(bool forceFirstPass = true);
	uint32 getLastFrameDuration() const;
	uint32 getTimeOfCurrentPass() const;
	uint32 getTimeOfLastPass() const;

	bool shouldExecuteScreenUpdate();
	void postScreenUpdate();

private:
	void reset();

};

} // End of namespace BladeRunner

#endif
