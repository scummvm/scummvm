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

#include "mediastation/assets/timer.h"

namespace MediaStation {

Operand Timer::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.size() == 0);
		timePlay();
		return Operand();
	}

	case kTimeStopMethod: {
		assert(args.size() == 0);
		timeStop();
		return Operand();
	}

	default: {
		error("Got unimplemented method ID %d", methodId);
	}
	}
}

void Timer::timePlay() {
	if (_isActive) {
		warning("Timer::timePlay(): Attempted to play a timer that is already playing");
		//return;
	}

	// SET TIMER VARIABLES.
	_isActive = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	g_engine->addPlayingAsset(this);

	// GET THE DURATION OF THE TIMER.
	// TODO: Is there a better way to find out what the max time is? Do we have to look
	// through each of the timer event handlers to figure it out?
	_duration = 0;
	for (EventHandler *timeEvent : _header->_timeHandlers) {
		// TODO: Centralize this converstion to milliseconds, as the same logic
		// is being used in several places.
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.u.f;
		uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
		if (timeEventInMilliseconds > _duration) {
			_duration = timeEventInMilliseconds;
		}
	}

	debugC(5, kDebugScript, "Timer::timePlay(): Now playing for %d ms", _duration);
}

void Timer::timeStop() {
	if (!_isActive) {
		warning("Timer::stop(): Attempted to stop a timer that is not playing");
		return;
	}

	_isActive = false;
	_startTime = 0;
	_lastProcessedTime = 0;
}

void Timer::process() {
	if (!_isActive) {
		error("Timer::processTimeEventHandlers(): Attempted to process time event handlers while not playing");
		return;
	}

	uint currentTime = g_system->getMillis();
	//uint movieTime = currentTime - _startTime;
	debugC(7, kDebugScript, "** Timer %d: ON TIME Event Handlers **", _header->_id);
	for (EventHandler *timeEvent : _header->_timeHandlers) {
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.u.f;
		uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
		bool timeEventAlreadyProcessed = timeEventInMilliseconds < _lastProcessedTime;
		bool timeEventNeedsToBeProcessed = timeEventInMilliseconds <= currentTime - _startTime;
		if (!timeEventAlreadyProcessed && timeEventNeedsToBeProcessed) {
			// TODO: What happens when we try re-run the timer when itʻs already
			// running? Seems like this would cause re-entrancy issues.
			timeEvent->execute(_header->_id);
		}
	}
	debugC(7, kDebugScript, "** Timer %d: End ON TIME Event Handlers **", _header->_id);
	_lastProcessedTime = currentTime - _startTime;
}

} // End of namespace MediaStation
