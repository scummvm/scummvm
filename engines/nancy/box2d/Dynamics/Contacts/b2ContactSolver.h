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
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef CONTACT_SOLVER_H
#define CONTACT_SOLVER_H

#include "../../Common/b2Math.h"
#include "../../Collision/b2Collision.h"
#include "../b2World.h"

class b2Contact;
class b2Body;
class b2Island;
class b2StackAllocator;

struct b2ContactConstraintPoint
{
	b2Vec2 localAnchor1;
	b2Vec2 localAnchor2;
	b2Vec2 r1;
	b2Vec2 r2;
	float32 normalImpulse;
	float32 tangentImpulse;
	float32 positionImpulse;
	float32 normalMass;
	float32 tangentMass;
	float32 equalizedMass;
	float32 separation;
	float32 velocityBias;
};

struct b2ContactConstraint
{
	b2ContactConstraintPoint points[b2_maxManifoldPoints];
	b2Vec2 normal;
	b2Manifold* manifold;
	b2Body* body1;
	b2Body* body2;
	float32 friction;
	float32 restitution;
	int32 pointCount;
};

class b2ContactSolver
{
public:
	b2ContactSolver(const b2TimeStep& step, b2Contact** contacts, int32 contactCount, b2StackAllocator* allocator);
	~b2ContactSolver();

	void InitVelocityConstraints(const b2TimeStep& step);
	void SolveVelocityConstraints();
	void FinalizeVelocityConstraints();

	bool SolvePositionConstraints(float32 baumgarte);

	b2TimeStep m_step;
	b2StackAllocator* m_allocator;
	b2ContactConstraint* m_constraints;
	int m_constraintCount;
};

#endif
