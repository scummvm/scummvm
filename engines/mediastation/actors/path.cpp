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

	case kActorHeaderStepRate: {
		double _stepRateFloat = chunk.readTypedDouble();
		// This should always be an integer anyway,
		// so we'll cast away any fractional part.
		_stepRate = static_cast<uint32>(_stepRateFloat);
		break;
	}

	case kActorHeaderDuration:
		// These are stored in the file as fractional seconds,
		// but we want milliseconds.
		_duration = static_cast<uint32>(chunk.readTypedTime() * 1000);
		break;

	case kActorHeaderPathTotalSteps:
		_totalSteps = chunk.readTypedUint16();
		break;

	default:
		Actor::readParameter(chunk, paramType);
	}
}

ScriptValue PathActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.size() == 0);
		timePlay();
		return returnValue;
	}

	case kSetDurationMethod: {
		assert(args.size() == 1);
		uint durationInMilliseconds = static_cast<uint>(args[0].asTime() * 1000);
		setDuration(durationInMilliseconds);
		return returnValue;
	}

	case kPercentCompleteMethod: {
		assert(args.size() == 0);
		returnValue.setToFloat(percentComplete());
		return returnValue;
	}

	case kIsPlayingMethod: {
		assert(args.empty());
		returnValue.setToBool(_isPlaying);
		return returnValue;
	}

	default:
		return Actor::callMethod(methodId, args);
	}
}

void PathActor::timePlay() {
	if (_isPlaying) {
		return;
	}

	if (_duration == 0) {
		warning("%s: Got zero duration", __func__);
	} else if (_stepRate == 0) {
		error("%s: Got zero step rate", __func__);
	}

	_isPlaying = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	_percentComplete = 0;
	_nextPathStepTime = 0;
	_currentStep = 0;
	_totalSteps = (_duration * _stepRate) / 1000;
	_stepDurationInMilliseconds = 1000 / _stepRate;

	// TODO: Run the path start event. Haven't seen one the wild yet, don't know its ID.
	debugC(5, kDebugScript, "Path::timePlay(): No PathStart event handler");
}

void PathActor::process() {
	if (!_isPlaying) {
		return;
	}

	uint currentTime = g_system->getMillis();
	uint pathTime = currentTime - _startTime;

	bool doNextStep = pathTime >= _nextPathStepTime;
	if (!doNextStep) {
		return;
	}

	_percentComplete = static_cast<double>(_currentStep + 1) / _totalSteps;
	debugC(2, kDebugScript, "Path::timePlay(): Step %d of %d", _currentStep, _totalSteps);

	if (_currentStep < _totalSteps) {
		// TODO: Actually step the path. It seems they mostly just use this for
		// palette animation in the On Step event handler, so nothing is actually drawn on the screen now.

		// We don't run a step event for the last step.
		runEventHandlerIfExists(kStepEvent);
		_nextPathStepTime = ++_currentStep * _stepDurationInMilliseconds;
	} else {
		_isPlaying = false;
		_percentComplete = 0;
		_nextPathStepTime = 0;
		_currentStep = 0;
		_totalSteps = 0;
		_stepDurationInMilliseconds = 0;

		runEventHandlerIfExists(kPathEndEvent);
	}
}

void PathActor::setDuration(uint durationInMilliseconds) {
	// TODO: Do we need to save the original duration?
	debugC(5, kDebugScript, "Path::setDuration(): Setting duration to %d ms", durationInMilliseconds);
	_duration = durationInMilliseconds;
}

double PathActor::percentComplete() {
	debugC(5, kDebugScript, "Path::percentComplete(): Returning percent complete %f%%", _percentComplete * 100);
	return _percentComplete;
}

} // End of namespace MediaStation
