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

#ifndef HPL_MESH_TYPES_H
#define HPL_MESH_TYPES_H

#include "hpl1/engine/math/BoundingVolume.h"
#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

//----------------------------------------------

class cTriangleData {
public:
	cVector3f normal;
	bool facingLight;

	cTriangleData() {}
	cTriangleData(const cVector3f &avNormal) {
		normal = avNormal;
	}
};

typedef Common::Array<cTriangleData> tTriangleDataVec;
typedef tTriangleDataVec::iterator tTriangleDataVecIt;

//----------------------------------------------

class cTriEdge {
public:
	int point1, point2;
	mutable int tri1, tri2;
	bool invert_tri2;

	cTriEdge() = default;
	constexpr cTriEdge(int alPoint1, int alPoint2, int alTri1, int alTri2) : point1(alPoint1), point2(alPoint2), tri1(alTri1), tri2(alTri2), invert_tri2(false) {
	}
};

typedef Common::Array<cTriEdge> tTriEdgeVec;
typedef tTriEdgeVec::iterator tTriEdgeVecIt;

//----------------------------------------------

} // namespace hpl

#endif // HPL_MESH_TYPES_H
