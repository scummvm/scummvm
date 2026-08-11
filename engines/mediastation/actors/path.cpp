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

#include "mediastation/actors/path.h"
#include "mediastation/mediastation.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

void PathActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderStartPoint:
		_startPoint = chunk.readTypedPoint();
		break;

	case kActorHeaderEndPoint:
		_endPoint = chunk.readTypedPoint();
		break;

	case kActorHeaderPathTotalSteps:
		_totalSteps = chunk.readTypedUint16();
		break;

	case kActorHeaderStepRate:
		_stepRate = chunk.readTypedDouble();
		break;

	case kActorHeaderDuration: {
		// These are stored in the file as fractional seconds,
		// but we want milliseconds.
		const uint MILLISECONDS_IN_ONE_SECOND = 1000;
		_duration = chunk.readTypedTime() * MILLISECONDS_IN_ONE_SECOND;
		_useTimeForCompletion = true;
		break;
	}

	default:
		Actor::readParameter(chunk, paramType);
	}
}

ScriptValue PathActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kTimePlayMethod:
		ARGCOUNTCHECK(0);
		startPath();
		break;

	case kTimeStopMethod:
		ARGCOUNTCHECK(0);
		stopPath();
		break;

	case kTimePauseMethod:
		ARGCOUNTCHECK(0);
		pausePath();
		break;

	case kTimeResumeMethod: {
		ARGCOUNTRANGE(0, 1);
		bool shouldRestart = false;
		if (args.size() == 1) {
			shouldRestart = args[0].asBool();
		}
		resumePath(shouldRestart);
		break;
	}

	case kGetLeftXMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_currentPoint.x);
		break;

	case kGetTopYMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToFloat(_currentPoint.y);
		break;

	case kPathSetStartPointMethod:
		ARGCOUNTCHECK(2);
		_startPoint.x = static_cast<int16>(args[0].asFloat());
		_startPoint.y = static_cast<int16>(args[1].asFloat());
		break;

	case kPathSetEndPointMethod:
		ARGCOUNTCHECK(2);
		_endPoint.x = static_cast<int16>(args[0].asFloat());
		_endPoint.y = static_cast<int16>(args[1].asFloat());
		break;

	case kPathSetTotalStepsMethod:
		ARGCOUNTCHECK(1);
		_totalSteps = static_cast<uint>(args[0].asFloat());
		_useTimeForCompletion = false;
		break;

	case kPathSetStepRateMethod:
		ARGCOUNTCHECK(1);
		_stepRate = static_cast<uint>(args[0].asFloat());
		break;

	case kPathSetDurationMethod: {
		ARGCOUNTCHECK(1);
		// Convert from seconds to milliseconds.
		const uint MILLISECONDS_IN_ONE_SECOND = 1000;
		double durationAsFractionalSeconds = args[0].asFloatOrTime();
		_duration = durationAsFractionalSeconds * MILLISECONDS_IN_ONE_SECOND;
		_useTimeForCompletion = true;
		break;
	}

	case kPathGetPercentCompleteMethod:
		ARGCOUNTCHECK(0);
		if (_playState == kPathPlaying) {
			returnValue.setToFloat(getPercentComplete());
		} else {
			returnValue.setToFloat(1.0);
		}
		break;

	case kIsPlayingMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_playState == kPathPlaying || _playState == kPathPaused);
		break;

	case kIsPausedMethod:
		ARGCOUNTCHECK(0);
		returnValue.setToBool(_playState == kPathPaused);
		break;

	default:
		returnValue = Actor::callMethod(methodId, args);
	}
	return returnValue;
}

void PathActor::onEvent(const ActorEvent &event) {
	// The original has other logic here, but I like the way I track things better.
	runScriptResponseIfExists(event.type);
}

void PathActor::startPath() {
	_currentPoint = _startPoint;
	if (_stepRate <= 0.0) {
		error("[%s] %s: Got zero or negative step rate", debugName(), __func__);
	}
	_stepDurationInMilliseconds = static_cast<uint>((1.0 / _stepRate) * 1000);
	_startTime = g_engine->getTotalPlayTime();
	_playState = kPathPlaying;
	scheduleNextTimerEvent();
	_currentStep = 0;

	// There is no path start script response.
}

void PathActor::stopPath() {
	if (_playState == kPathPlaying || _playState == kPathPaused) {
		g_engine->getTimerService()->stopTimer(_timer);
		_playState = kPathStopped;
		ActorEvent actorEvent(_id, kPathStoppedEvent);
		g_engine->getEventLoop()->queueEvent(actorEvent);
	}
}

void PathActor::pausePath() {
	if (_playState == kPathPlaying) {
		_playState = kPathPaused;
		_pauseTime = g_engine->getTotalPlayTime();
	}
}

void PathActor::resumePath(bool shouldRestart) {
	if (_playState == kPathPaused) {
		// Calculate how long we were paused, to make sure we resume at the right point.
		uint currentTime = g_engine->getTotalPlayTime();
		uint pauseDuration = currentTime - _pauseTime;
		_startTime += pauseDuration;
		_playState = kPathPlaying;
		scheduleNextTimerEvent();
	} else if (_playState != kPathPlaying && shouldRestart) {
		startPath();
	}
}

double PathActor::getPercentComplete() {
	double percentComplete = 1.0;
	if (!_useTimeForCompletion) {
		if (_totalSteps > 0) {
			percentComplete = static_cast<double>(_currentStep) / _totalSteps;
		}
	} else {
		uint currentTime = g_engine->getTotalPlayTime();
		if (currentTime > _startTime && _duration > 0) {
			double timeElapsed = currentTime - _startTime;
			percentComplete = timeElapsed / _duration;
			if (percentComplete > 1.0) {
				percentComplete = 1.0;
			}
		}
	}

	return percentComplete;
}

bool PathActor::step() {
	double percentComplete = getPercentComplete();
	if (percentComplete < 1.0) {
		double nextX = _startPoint.x + (_endPoint.x - _startPoint.x) * percentComplete;
		double nextY = _startPoint.y + (_endPoint.y - _startPoint.y) * percentComplete;
		_currentPoint = Common::Point(static_cast<int16>(nextX), static_cast<int16>(nextY));
		debugC(4, kDebugEvents, "[%s] %s: %f%% complete (startPoint: (%d, %d)) (endPoint: (%d, %d)) (currentPoint: (%d, %d))",
			debugName(), __func__, percentComplete,
			_endPoint.x, _endPoint.y, _startPoint.x, _startPoint.y, _currentPoint.x, _currentPoint.y);

		// We don't run a step event for the last step.
		ActorEvent actorEvent(_id, kPathStepEvent);
		g_engine->getEventLoop()->queueEvent(actorEvent);
		return false;
	}
	return true;
}

void PathActor::scheduleNextTimerEvent() {
	// Catch up if we are behind.
	_nextPathStepTime += _stepDurationInMilliseconds;
	uint32 currentTime = g_engine->getTotalPlayTime();
	if (_nextPathStepTime < currentTime) {
		_nextPathStepTime = currentTime;
	}
	uint32 delayUntilNextStepInMilliseconds = _nextPathStepTime - currentTime;
	debugC(5, kDebugEvents, "[%s] %s: next step in %d ms", debugName(), __func__, delayUntilNextStepInMilliseconds);
	g_engine->getTimerService()->startTimer(_timer, delayUntilNextStepInMilliseconds);
}

void PathActor::timerEvent(const TimerEvent &event) {
	_currentStep += 1;
	bool finishedPlaying = step();
	if (!finishedPlaying) {
		scheduleNextTimerEvent();
	} else {
		g_engine->getTimerService()->stopTimer(_timer);
		_playState = kPathStopped;
		_nextPathStepTime = 0;
		_currentStep = 0;
		_stepDurationInMilliseconds = 0;

		ActorEvent actorEvent(_id, kPathEndEvent);
		g_engine->getEventLoop()->queueEvent(actorEvent);
	}
}

} // End of namespace MediaStation
