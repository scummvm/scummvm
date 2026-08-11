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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_NODE_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_NODE_H

#ifdef ENABLE_HE

#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"

namespace Scumm {

#define NUM_CHILDREN_NODES 4

enum EChildID {
	kChild1 = 0,
	kChild2 = 1,
	kChild3 = 2,
	kChild4 = 3
};

class CCollisionNode {
public:
	CCollisionNode();
	CCollisionNode(const CCollisionObjectVector &initObjects);
	~CCollisionNode();

	friend class CCollisionObjectTree;

private:
	void searchTree(const U32BoundingBox &searchRange, CCollisionObjectVector *targetList) const;
	static U32BoundingBox getChildQuadrant(const U32BoundingBox &parentQuadrant, EChildID childID);

	// Children nodes to this node
	CCollisionNode *_child[NUM_CHILDREN_NODES];

	// A list of object pointers that are contained within this node
	CCollisionObjectVector _objectList;

	// The area that is handled by this node
	U32BoundingBox _quadrant;

	// Whether or not this is a leaf node
	bool _isExternal;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_NODE_H
