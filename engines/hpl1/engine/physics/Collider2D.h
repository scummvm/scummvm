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

#ifndef HPL_COLLIDER2D_H
#define HPL_COLLIDER2D_H

#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class cMesh2D;
class cWorld2D;
class cBody2D;
class cCollideData2D;
class cCollisionMesh2D;

class cCollider2DDebug {
public:
	cVector2f mvPushVec;
	cVector2f mvPushPos;
};

class cCollider2D {
public:
	cCollider2D();
	~cCollider2D();

	void SetWorld(cWorld2D *apWorld) { mpWorld = apWorld; }

	/**
	 * Collides a body with the world and returns a flag of what object has collided.
	 * \param *apBody
	 * \return 0= no collision, eFlagBit0 = Tiles, all other bits are custom.
	 */
	tFlag CollideBody(cBody2D *apBody, cCollideData2D *apData);

	/**
	 * Collides a custom rect with the world
	 * \param aRect
	 * \param alCollideFlags Specifies what things to check for collision with.
	 * \return
	 */
	tFlag CollideRect(cRect2f &aRect, tFlag alCollideFlags, cCollideData2D *apData);

	tFlag CollideLine(const cVector2f &avStart, const cVector2f &avEnd, tFlag alCollideFlags,
					  cCollideData2D *apData);

	cCollider2DDebug mDebug;

private:
	cWorld2D *mpWorld;

	void SetCollideMesh(cCollisionMesh2D *apMesh, cRect2f &aRect);

	bool Collide(cCollisionMesh2D *apMeshA, cCollisionMesh2D *apMeshB, cVector2f &avMTD);
	bool AxisSeparateMeshes(cVector2f &avAxis, cCollisionMesh2D *apMeshA,
							cCollisionMesh2D *apMeshB);
	void CalculateInterval(const cVector2f &avAxis, cCollisionMesh2D *apMesh,
						   float &afMin, float &afMax);
};

} // namespace hpl

#endif // HPL_COLLIDER2D_H
