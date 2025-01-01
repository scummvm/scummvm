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
	case BuiltInMethod::timePlay: {
		assert(args.size() == 0);
		timePlay();
		return Operand();
	}

	case BuiltInMethod::setDuration: {
		assert(args.size() == 1);
		uint durationInMilliseconds = (uint)(args[0].getDouble() * 1000);
		setDuration(durationInMilliseconds);
		return Operand();
	}

	case BuiltInMethod::percentComplete: {
		assert(args.size() == 0);
		Operand returnValue(Operand::Type::Float1);
		returnValue.putDouble(percentComplete());
		return returnValue;
	}

	default: {
		error("Got unimplemented method ID %d", methodId);
	}
	}
}

void Path::timePlay() {
	// TODO: Check that itʻs zero before we reset it, since this function isn't re-entrant!
	_percentComplete = 0.0;

	if (_header->_duration == 0) {
		warning("Path::timePlay(): Got zero duration");
	} else if (_header->_stepRate == 0) {
		error("Path::timePlay(): Got zero step rate");
	}
	debugC(5, kDebugScript, "Path::timePlay(): Path playback started");
	uint totalSteps = (_header->_duration * _header->_stepRate) / 1000;
	//uint stepDurationInMilliseconds = 1000 / _header->_stepRate;

	// RUN THE START EVENT HANDLER.
	EventHandler *startEventHandler = nullptr; // TODO: Haven't seen a path start event in the wild yet, don't know its ID.
	if (startEventHandler != nullptr) {
		debugC(5, kDebugScript, "Path::timePlay(): Running PathStart event handler");
		startEventHandler->execute(_header->_id);
	} else {
		debugC(5, kDebugScript, "Path::timePlay(): No PathStart event handler");
	}

	// STEP THE PATH.
	EventHandler *pathStepHandler = _header->_eventHandlers[EventHandler::Type::Step];
	for (uint i = 0; i < totalSteps; i++) {
		_percentComplete = (double)(i + 1) / totalSteps;
		debugC(5, kDebugScript, "Path::timePlay(): Step %d of %d", i, totalSteps);
		// TODO: Actually step the path. It seems they mostly just use this for
		// palette animation in the On Step event handler, so nothing is actually drawn on the screen now.

		// RUN THE ON STEP EVENT HANDLER.
		// TODO: Is this supposed to come after or before we step the path?
		if (pathStepHandler != nullptr) {
			debugC(5, kDebugScript, "Path::timePlay(): Running PathStep event handler");
			pathStepHandler->execute(_header->_id);
		} else {
			debugC(5, kDebugScript, "Path::timePlay(): No PathStep event handler");
		}
	}

	// RUN THE END EVENT HANDLER.
	EventHandler *endEventHandler = _header->_eventHandlers[EventHandler::Type::PathEnd];
	if (endEventHandler != nullptr) {
		debugC(5, kDebugScript, "Path::timePlay(): Running PathEnd event handler");
		endEventHandler->execute(_header->_id);
	} else {
		debugC(5, kDebugScript, "Path::timePlay(): No PathEnd event handler");
	}

	// CLEAN UP.
	_percentComplete = 0;
}

void Path::process() {
	// TODO: Handle this case.
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

