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

#include "mediastation/mediastation.h"
#include "mediastation/debugchannels.h"

#include "mediastation/actors/timer.h"

namespace MediaStation {

ScriptValue TimerActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.size() == 0);
		timePlay();
		return returnValue;
	}

	case kTimeStopMethod: {
		assert(args.size() == 0);
		timeStop();
		return returnValue;
	}

	case kIsPlayingMethod: {
		assert(args.size() == 0);
		returnValue.setToBool(_isPlaying);
		return returnValue;
	}

	default:
		return Actor::callMethod(methodId, args);
	}
}

void TimerActor::timePlay() {
	_isPlaying = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;

	// Get the duration of the timer.
	// TODO: Is there a better way to find out what the max time is? Do we have to look
	// through each of the timer event handlers to figure it out?
	_duration = 0;
	const Common::Array<EventHandler *> &timeHandlers = _eventHandlers.getValOrDefault(kTimerEvent);
	for (EventHandler *timeEvent : timeHandlers) {
		// Indeed float, not time.
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.asFloat();
		uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
		if (timeEventInMilliseconds > _duration) {
			_duration = timeEventInMilliseconds;
		}
	}

	debugC(5, kDebugScript, "Timer::timePlay(): Now playing for %d ms", _duration);
}

void TimerActor::timeStop() {
	if (!_isPlaying) {
		return;
	}

	_isPlaying = false;
	_startTime = 0;
	_lastProcessedTime = 0;
}

void TimerActor::process() {
	if (_isPlaying) {
		processTimeEventHandlers();
	}
}

} // End of namespace MediaStation
