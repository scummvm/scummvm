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

#ifndef HPL_COLLIDE_SHAPE_NEWTON_H
#define HPL_COLLIDE_SHAPE_NEWTON_H

#include "hpl1/engine/libraries/newton/Newton.h"
#include "hpl1/engine/physics/CollideShape.h"

#include "hpl1/engine/math/MathTypes.h"

namespace hpl {

class iVertexBuffer;
class iCollideShape;

typedef Common::Array<iCollideShape *> tCollideShapeVec;
typedef tCollideShapeVec::iterator tCollideShapeVecIt;

class cCollideShapeNewton : public iCollideShape {
public:
	cCollideShapeNewton(eCollideShapeType aType, const cVector3f &avSize,
						cMatrixf *apOffsetMtx, NewtonWorld *apNewtonWorld,
						iPhysicsWorld *apWorld);
	~cCollideShapeNewton();

	iCollideShape *GetSubShape(int alIdx);
	int GetSubShapeNum();

	cVector3f GetInertia(float afMass);

	void CreateFromShapeVec(tCollideShapeVec &avShapes);
	void CreateFromVertices(const unsigned int *apIndexArray, int alIndexNum,
							const float *apVertexArray, int alVtxStride, int alVtxNum);

	NewtonCollision *GetNewtonCollision() { return mpNewtonCollision; }

private:
	NewtonCollision *mpNewtonCollision;
	NewtonWorld *mpNewtonWorld;

	tCollideShapeVec mvSubShapes;
};

} // namespace hpl

#endif // HPL_COLLIDE_SHAPE_NEWTON_H
