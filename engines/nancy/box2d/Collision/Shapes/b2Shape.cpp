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

/*
 * Altered for ScummVM: uses the system headers through ScummVM.
 */

#include "b2Shape.h"
#include "b2CircleShape.h"
#include "b2PolygonShape.h"
#include "../b2Collision.h"
#include "../b2BroadPhase.h"
#include "../../Common/b2BlockAllocator.h"


b2Shape* b2Shape::Create(const b2ShapeDef* def, b2BlockAllocator* allocator)
{
	switch (def->type)
	{
	case e_circleShape:
		{
			void* mem = allocator->Allocate(sizeof(b2CircleShape));
			return new (mem) b2CircleShape(def);
		}

	case e_polygonShape:
		{
			void* mem = allocator->Allocate(sizeof(b2PolygonShape));
			return new (mem) b2PolygonShape(def);
		}

	default:
		b2Assert(false);
		return NULL;
	}
}

void b2Shape::Destroy(b2Shape* s, b2BlockAllocator* allocator)
{
	switch (s->GetType())
	{
	case e_circleShape:
		s->~b2Shape();
		allocator->Free(s, sizeof(b2CircleShape));
		break;

	case e_polygonShape:
		s->~b2Shape();
		allocator->Free(s, sizeof(b2PolygonShape));
		break;

	default:
		b2Assert(false);
	}
}

b2Shape::b2Shape(const b2ShapeDef* def)
{
	m_userData = def->userData;
	m_friction = def->friction;
	m_restitution = def->restitution;
	m_density = def->density;
	m_body = NULL;
	m_sweepRadius = 0.0f;

	m_next = NULL;

	m_proxyId = b2_nullProxy;

	m_filter = def->filter;

	m_isSensor = def->isSensor;
}

b2Shape::~b2Shape()
{
	b2Assert(m_proxyId == b2_nullProxy);
}

void b2Shape::CreateProxy(b2BroadPhase* broadPhase, const b2XForm& transform)
{
	b2Assert(m_proxyId == b2_nullProxy);

	b2AABB aabb;
	ComputeAABB(&aabb, transform);

	bool inRange = broadPhase->InRange(aabb);

	// You are creating a shape outside the world box.
	b2Assert(inRange);

	if (inRange)
	{
		m_proxyId = broadPhase->CreateProxy(aabb, this);
	}
	else
	{
		m_proxyId = b2_nullProxy;
	}
}

void b2Shape::DestroyProxy(b2BroadPhase* broadPhase)
{
	if (m_proxyId != b2_nullProxy)
	{
		broadPhase->DestroyProxy(m_proxyId);
		m_proxyId = b2_nullProxy;
	}
}

bool b2Shape::Synchronize(b2BroadPhase* broadPhase, const b2XForm& transform1, const b2XForm& transform2)
{
	if (m_proxyId == b2_nullProxy)
	{	
		return false;
	}

	// Compute an AABB that covers the swept shape (may miss some rotation effect).
	b2AABB aabb;
	ComputeSweptAABB(&aabb, transform1, transform2);

	if (broadPhase->InRange(aabb))
	{
		broadPhase->MoveProxy(m_proxyId, aabb);
		return true;
	}
	else
	{
		return false;
	}
}

void b2Shape::RefilterProxy(b2BroadPhase* broadPhase, const b2XForm& transform)
{
	if (m_proxyId == b2_nullProxy)
	{	
		return;
	}

	broadPhase->DestroyProxy(m_proxyId);

	b2AABB aabb;
	ComputeAABB(&aabb, transform);

	bool inRange = broadPhase->InRange(aabb);

	if (inRange)
	{
		m_proxyId = broadPhase->CreateProxy(aabb, this);
	}
	else
	{
		m_proxyId = b2_nullProxy;
	}
}
