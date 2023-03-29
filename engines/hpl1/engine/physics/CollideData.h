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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_COLLIDE_DATA_H
#define HPL_COLLIDE_DATA_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cCollidePoint {
public:
	cVector3f mvPoint;
	cVector3f mvNormal;
	float mfDepth;
};

typedef Common::Array<cCollidePoint> tCollidePointVec;
typedef tCollidePointVec::iterator tCollidePointVecIt;

class cCollideData {
public:
	tCollidePointVec mvContactPoints;
	int mlNumOfPoints;

	void SetMaxSize(int alSize) {
		mvContactPoints.resize(alSize);
	}
};

} // namespace hpl

#endif // HPL_COLLIDE_DATA_H
