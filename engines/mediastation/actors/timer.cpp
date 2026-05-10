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
	case kTimePlayMethod:
		ARGCOUNTCHECK(0);
		start();
		break;

	case kTimeStopMethod:
		ARGCOUNTCHECK(0);
		stop();
		break;

	case kTimePauseMethod:
		ARGCOUNTCHECK(0);
		pause();
		break;

	case kTimeResumeMethod: {
		ARGCOUNTRANGE(0, 1);
		bool shouldRestart = false;
		if (args.size() == 1) {
			shouldRestart = args[0].asBool();
		}
		resume(shouldRestart);
		break;
	}

	case kIsPlayingMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_startTime > 0);
		break;

	default:
		returnValue = Actor::callMethod(methodId, args);
	}
	return returnValue;
}

void TimerActor::start() {
	stop();
	_startTime = g_engine->getTotalPlayTime();
	_lastProcessedTime = 0;
	setupNextScriptResponseTimer();
}

void TimerActor::stop() {
	g_engine->getTimerService()->stopTimer(_timer);
	_pauseStartTime = 0;
	_startTime = 0;
}

void TimerActor::pause() {
	bool timerIsRunningAndNotPaused = _startTime > 0 && _pauseStartTime == 0;
	if (timerIsRunningAndNotPaused) {
		_pauseStartTime = g_engine->getTotalPlayTime();
		g_engine->getTimerService()->stopTimer(_timer);
	}
}

void TimerActor::resume(bool shouldRestart) {
	// Resume a paused timer by compensating for the pause duration.
	bool isTimerRunningAndPaused = (_startTime > 0 && _pauseStartTime > 0);
	if (isTimerRunningAndPaused) {
		uint32 currentTime = g_engine->getTotalPlayTime();
		uint32 pauseDuration = currentTime - _pauseStartTime;
		_startTime += pauseDuration;
		_pauseStartTime = 0;
		setupNextScriptResponseTimer();
		return;
	}

	// Restart a timer that was stopped while paused.
	bool shouldRestartStoppedTimer = (shouldRestart && _startTime == 0 && _pauseStartTime > 0);
	if (shouldRestartStoppedTimer) {
		start();
	}
}

void TimerActor::timerEvent(const TimerEvent &event) {
	// The timer actor is subtly different from other actors that can have timer events called on them,
	// which is why the default call to process timer events doesn't work here.
	ScriptResponse *nextTimeScriptResponse = findNextTimeScriptResponseAfter(_lastProcessedTime);
	double eventTimeInSeconds = nextTimeScriptResponse->_argumentValue.asFloat();
	uint32 eventTimeInMilliseconds = eventTimeInSeconds * 1000;
	// Increment by 1 to prevent re-triggering the same event. This works because in the original,
	// timer events are at least 10 ms apart anyway.
	_lastProcessedTime = eventTimeInMilliseconds + 1;

	// Unlike the other timer handlers, schedule the next script response FIRST,
	// before executing the current one. This seems to be because any time event
	// can re-start the timer.
	g_engine->getTimerService()->stopTimer(_timer);
	if (!setupNextScriptResponseTimer()) {
		// No more events, stop the timer.
		stop();
	}

	// Actually execute the script response we found.
	nextTimeScriptResponse->execute(_id);
}

} // End of namespace MediaStation
