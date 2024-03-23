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

#ifndef TWINE_SCENE_BUGGY_H
#define TWINE_SCENE_BUGGY_H

#include "common/scummsys.h"
#include "twine/input.h"
#include "twine/scene/actor.h"

namespace TwinE {

class Buggy {
private:
	TwinEEngine *_engine;

public:
	Buggy(TwinEEngine *engine) : _engine(engine) {}
	void initBuggy(uint8 numobj, uint32 flaginit);
	void resetBuggy();
	void takeBuggy();
	void leaveBuggy(uint8 newcomportement);
	void doAnimBuggy(ActorStruct *ptrobj);
	void moveBuggy(ActorStruct *ptrobj);
};

} // namespace TwinE

#endif
