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
		int32 PosX;
		int32 PosY;
		int32 PosZ;
		int32 Alpha;
		int32 Beta;
		int32 Body;
		int32 NumCube; // Number of the cube in which the dart is located
		uint32 Flags;

		int32 XMin; // ZV of the darts
		int32 YMin;
		int32 ZMin;
		int32 XMax;
		int32 YMax;
		int32 ZMax;
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
