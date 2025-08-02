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

#ifndef MEDIASTATION_PATH_H
#define MEDIASTATION_PATH_H

#include "mediastation/actor.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class PathActor : public Actor {
public:
	PathActor() : Actor(kActorTypePath) {};

	virtual void process() override;

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

private:
	double _percentComplete = 0.0;
	uint _totalSteps = 0;
	uint _currentStep = 0;
	uint _nextPathStepTime = 0;
	uint _stepDurationInMilliseconds = 0;
	bool _isPlaying = false;

	Common::Point _startPoint;
	Common::Point _endPoint;
	uint32 _stepRate = 0;
	uint32 _duration = 0;

	// Method implementations.
	void timePlay();
	void setDuration(uint durationInMilliseconds);
	double percentComplete();
};

} // End of namespace MediaStation

#endif
