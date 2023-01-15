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

#ifndef TWINE_SCRIPTLIFEV1_H
#define TWINE_SCRIPTLIFEV1_H

#include "common/scummsys.h"

namespace TwinE {

// SCENE_SIZE_MAX
#define MAX_TARGET_ACTOR_DISTANCE 0x7D00

class TwinEEngine;

class ScriptLifeV1 {
private:
	TwinEEngine *_engine;

public:
	ScriptLifeV1(TwinEEngine *engine);

	/**
	 * Process actor life script
	 * @param actorIdx Current processed actor index
	 */
	void doLife(int32 actorIdx);
};

} // namespace TwinE

#endif
