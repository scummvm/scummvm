/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_BONES_H
#define ICB_PX_BONES_H

#include "engines/icb/common/px_common.h"

namespace ICB {

// maximum number of deformations
#define MAX_DEFORMABLE_BONES 4

// the deformations being used (so standard across tools/engine...
#define JAW_DEFORMATION 0
#define NECK_DEFORMATION 1
#define LOOK_DEFORMATION 2
#define SPARE_DEFORMATION 3

class BoneDeformation {
public:
	BoneDeformation() {
		boneTarget.vx = boneTarget.vy = boneTarget.vz = boneValue.vx = boneValue.vy = boneValue.vz = 0;
		boneNumber = -1;
	}

	int16 boneNumber;
	int16 boneSpeed;
	SVECTOR boneValue;
	SVECTOR boneTarget;

	void UpdateBoneValue(int16 &v, int16 t);
	void Target0();
	void Update();
};

} // End of namespace ICB

#endif // _PX_BONES_H
