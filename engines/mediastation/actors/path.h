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

enum PathPlayState {
	kPathStopped = 1,
	kPathPlaying = 2,
	kPathPaused = 3,
};

class PathActor : public Actor {
public:
	PathActor() : Actor(kActorTypePath) {};

	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

	virtual void onEvent(const ActorEvent &event) override;
	virtual void timerEvent(const TimerEvent &event) override;

private:
	PathPlayState _playState = kPathStopped;
	bool _useTimeForCompletion = false;
	double _duration = 0.0;
	double _stepRate = 0.0;
	uint _stepDurationInMilliseconds = 0;
	uint _currentStep = 0;
	uint _startTime = 0;
	uint _pauseTime = 0;
	uint _totalSteps = 0;
	uint _nextPathStepTime = 0;

	Common::Point _startPoint;
	Common::Point _endPoint;
	Common::Point _currentPoint;

	void startPath();
	void stopPath();
	void pausePath();
	void resumePath(bool shouldRestart);

	double getPercentComplete();
	bool step();
	void scheduleNextTimerEvent();
};

} // End of namespace MediaStation

#endif
