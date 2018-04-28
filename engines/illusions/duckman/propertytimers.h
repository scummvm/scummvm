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

#ifndef ILLUSIONS_DUCKMAN_PROPERTYTIMERS_H
#define ILLUSIONS_DUCKMAN_PROPERTYTIMERS_H

#include "illusions/illusions.h"
#include "common/algorithm.h"
#include "common/stack.h"

namespace Illusions {

class IllusionsEngine_Duckman;

struct PropertyTimer {
	uint32 _propertyId;
	uint32 _startTime;
	uint32 _duration;
	uint32 _endTime;
	PropertyTimer() : _propertyId(0) {}
};

const uint kPropertyTimersCount = 6;

class PropertyTimers {
public:
	PropertyTimers(IllusionsEngine_Duckman *vm);
	~PropertyTimers();
private:
	IllusionsEngine_Duckman *_vm;
	PropertyTimer _propertyTimers[kPropertyTimersCount];
	bool _propertyTimersActive;
	bool _propertyTimersPaused;
public:
	void addPropertyTimer(uint32 propertyId);
	void setPropertyTimer(uint32 propertyId, uint32 duration);
	void removePropertyTimer(uint32 propertyId);
	bool findPropertyTimer(uint32 propertyId, PropertyTimer *&propertyTimer);
	int updatePropertyTimers(uint flags);
};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
