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

#include "mediastation/assets/path.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Path::~Path() {
	_percentComplete = 0;
}

Operand Path::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.size() == 0);
		timePlay();
		return Operand();
	}

	case kSetDurationMethod: {
		assert(args.size() == 1);
		uint durationInMilliseconds = static_cast<uint>(args[0].getDouble() * 1000);
		setDuration(durationInMilliseconds);
		return Operand();
	}

	case kPercentCompleteMethod: {
		assert(args.size() == 0);
		Operand returnValue(kOperandTypeFloat1);
		returnValue.putDouble(percentComplete());
		return returnValue;
	}

	case kSetDissolveFactorMethod: {
		assert(args.size() == 1);
		warning("Path::callMethod(): setDissolveFactor not implemented yet");
		return Operand();
	}

	case kIsPlayingMethod: {
		assert(args.empty());
		Operand returnValue(kOperandTypeLiteral1);
		returnValue.putInteger(_isActive);
		return returnValue;
	}

	default:
		error("Path::callMethod(): Got unimplemented method ID %s (%d)", builtInMethodToStr(methodId), static_cast<uint>(methodId));
	}
}

void Path::timePlay() {
	if (_isActive) {
		warning("Path::timePlay(): Attempted to play a path that is already playing");
		return;
	}

	if (_header->_duration == 0) {
		warning("Path::timePlay(): Got zero duration");
	} else if (_header->_stepRate == 0) {
		error("Path::timePlay(): Got zero step rate");
	}

	setActive();
	_percentComplete = 0;
	_nextPathStepTime = 0;
	_currentStep = 0;
	_totalSteps = (_header->_duration * _header->_stepRate) / 1000;
	_stepDurationInMilliseconds = 1000 / _header->_stepRate;

	// TODO: Run the path start event. Haven't seen one the wild yet, don't know its ID.
	debugC(5, kDebugScript, "Path::timePlay(): No PathStart event handler");
}

void Path::process() {
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

		// We donʻt run a step event for the last step.
		runEventHandlerIfExists(kStepEvent);
		_nextPathStepTime = ++_currentStep * _stepDurationInMilliseconds;
	} else {
		setInactive();
		_percentComplete = 0;
		_nextPathStepTime = 0;
		_currentStep = 0;
		_totalSteps = 0;
		_stepDurationInMilliseconds = 0;

		runEventHandlerIfExists(kPathEndEvent);
	}
}

void Path::setDuration(uint durationInMilliseconds) {
	// TODO: Do we need to save the original duration?
	debugC(5, kDebugScript, "Path::setDuration(): Setting duration to %d ms", durationInMilliseconds);
	_header->_duration = durationInMilliseconds;
}


double Path::percentComplete() {
	debugC(5, kDebugScript, "Path::percentComplete(): Returning percent complete %f%%", _percentComplete * 100);
	return _percentComplete;
}

} // End of namespace MediaStation

