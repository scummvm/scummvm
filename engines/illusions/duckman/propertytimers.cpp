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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/duckman/propertytimers.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"
#include "engines/util.h"

namespace Illusions {

// PropertyTimers

PropertyTimers::PropertyTimers(IllusionsEngine_Duckman *vm) {
	_vm = vm;
	_propertyTimersActive = false;
	_propertyTimersPaused = false;
}

PropertyTimers::~PropertyTimers() {
}

void PropertyTimers::addPropertyTimer(uint32 propertyId) {
	PropertyTimer *propertyTimer;
	if (findPropertyTimer(propertyId, propertyTimer) || findPropertyTimer(0, propertyTimer)) {
		propertyTimer->_propertyId = propertyId;
		propertyTimer->_startTime = 0;
		propertyTimer->_duration = 0;
		propertyTimer->_endTime = 0;
	}
}

void PropertyTimers::setPropertyTimer(uint32 propertyId, uint32 duration) {
	PropertyTimer *propertyTimer;
	if (findPropertyTimer(propertyId, propertyTimer)) {
		propertyTimer->_startTime = getCurrentTime();
		propertyTimer->_duration = duration;
		propertyTimer->_endTime = duration + propertyTimer->_startTime;
	}
	_vm->_scriptResource->_properties.set(propertyId, false);
	if (!_propertyTimersActive) {
		_vm->_updateFunctions->add(29, _vm->getCurrentScene(), new Common::Functor1Mem<uint, int, PropertyTimers>
			(this, &PropertyTimers::updatePropertyTimers));
		_propertyTimersActive = true;
	}
}

void PropertyTimers::removePropertyTimer(uint32 propertyId) {
	PropertyTimer *propertyTimer;
	if (findPropertyTimer(propertyId, propertyTimer))
		propertyTimer->_propertyId = 0;
	_vm->_scriptResource->_properties.set(propertyId, true);
}

bool PropertyTimers::findPropertyTimer(uint32 propertyId, PropertyTimer *&propertyTimer) {
	for (uint i = 0; i < kPropertyTimersCount; ++i) {
		if (_propertyTimers[i]._propertyId == propertyId) {
			propertyTimer = &_propertyTimers[i];
			return true;
		}
	}
	return false;
}

int PropertyTimers::updatePropertyTimers(uint flags) {
	int result = kUFNext;
	uint32 currTime = getCurrentTime();
	if (_vm->_pauseCtr <= 0) {
		if (_propertyTimersPaused) {
			for (uint i = 0; i < kPropertyTimersCount; ++i) {
				PropertyTimer &propertyTimer = _propertyTimers[i];
				propertyTimer._startTime = currTime;
				propertyTimer._endTime = currTime + propertyTimer._duration;
			}
			_propertyTimersPaused = false;
		}
		if (flags & 1) {
			_propertyTimersActive = false;
			_propertyTimersPaused = false;
			result = kUFTerminate;
		} else {
			bool timersActive = false;
			for (uint i = 0; i < kPropertyTimersCount; ++i) {
				PropertyTimer &propertyTimer = _propertyTimers[i];
				if (propertyTimer._propertyId) {
					timersActive = true;
					if (!_vm->_scriptResource->_properties.get(propertyTimer._propertyId) &&
						isTimerExpired(propertyTimer._startTime, propertyTimer._endTime))
						_vm->_scriptResource->_properties.set(propertyTimer._propertyId, true);
				}
			}
			if (!timersActive) {
				_propertyTimersActive = false;
				_propertyTimersPaused = false;
				result = kUFTerminate;
			}
		}
	} else {
		if (!_propertyTimersPaused) {
			for (uint i = 0; i < kPropertyTimersCount; ++i) {
				PropertyTimer &propertyTimer = _propertyTimers[i];
				propertyTimer._duration -= getDurationElapsed(propertyTimer._startTime, propertyTimer._endTime);
			}
			_propertyTimersPaused = true;
		}
		result = kUFNext;
	}
	return result;
}

} // End of namespace Illusions
