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

#ifndef TWINE_SCENE_DART_H
#define TWINE_SCENE_DART_H

#include "twine/scene/actor.h"
#include "twine/twine.h"

#define MAX_DARTS 3
#define BODY_3D_DART 61
// dart flags
#define DART_TAKEN (1 << 0)

namespace TwinE {

class Dart {
private:
	TwinEEngine *_engine;

public:
	struct T_DART {
		int32 PosX = 0;
		int32 PosY = 0;
		int32 PosZ = 0;
		int32 Alpha = 0;
		int32 Beta = 0;
		int32 Body = 0;
		int32 NumCube = 0; // Number of the cube in which the dart is located
		uint32 Flags = 0u;

		int32 XMin = 0; // ZV of the darts
		int32 YMin = 0;
		int32 ZMin = 0;
		int32 XMax = 0;
		int32 YMax = 0;
		int32 ZMax = 0;
	};
	T_DART ListDart[MAX_DARTS];

	Dart(TwinEEngine *engine) : _engine(engine) {}

	void InitDarts();
	int32 GetDart();
	void TakeAllDarts();
	void CheckDartCol(ActorStruct *ptrobj);
};

} // namespace TwinE

#endif
