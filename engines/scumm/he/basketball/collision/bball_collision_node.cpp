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

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_sphere.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"
#include "scumm/he/basketball/collision/bball_collision_tree.h"

namespace Scumm {

CCollisionNode::CCollisionNode() {
	for (int i = 0; i < NUM_CHILDREN_NODES; i++) {
		_child[i] = nullptr;
	}

	_quadrant.setMinPoint(0, 0);
	_quadrant.setMaxPoint(0, 0);

	_isExternal = false;
}

CCollisionNode::CCollisionNode(const CCollisionObjectVector &initObjects) : _objectList(initObjects) {
	for (int i = 0; i < NUM_CHILDREN_NODES; i++) {
		_child[i] = nullptr;
	}

	_quadrant.setMinPoint(0, 0);
	_quadrant.setMaxPoint(0, 0);

	_isExternal = false;
}

CCollisionNode::~CCollisionNode() {
	if (!_isExternal) {
		for (int i = 0; i < NUM_CHILDREN_NODES; i++) {
			assert(_child[i]);
			delete _child[i];
			_child[i] = nullptr;
		}
	}
}

U32BoundingBox CCollisionNode::getChildQuadrant(const U32BoundingBox &parentQuadrant, EChildID childID) {
	int minX, minY;
	int maxX, maxY;

	int centerX = ((parentQuadrant.getMinPoint().x + parentQuadrant.getMaxPoint().x) / 2);
	int centerY = ((parentQuadrant.getMinPoint().y + parentQuadrant.getMaxPoint().y) / 2);

	switch (childID) {

	case kChild1:
		minX = centerX;
		minY = parentQuadrant.getMinPoint().y;
		maxX = parentQuadrant.getMaxPoint().x;
		maxY = centerY;
		break;

	case kChild2:
		minX = centerX;
		minY = centerY;
		maxX = parentQuadrant.getMaxPoint().x;
		maxY = parentQuadrant.getMaxPoint().y;
		break;

	case kChild3:
		minX = parentQuadrant.getMinPoint().x;
		minY = centerY;
		maxX = centerX;
		maxY = parentQuadrant.getMaxPoint().y;
		break;

	case kChild4:
		minX = parentQuadrant.getMinPoint().x;
		minY = parentQuadrant.getMinPoint().y;
		maxX = centerX;
		maxY = centerY;
		break;

	default:
		warning("CCollisionNode::getChildQuadrant(): Invalid childID passed to getChildQuadrant");
		minX = 0;
		minY = 0;
		maxX = 0;
		maxY = 0;
	}

	U32BoundingBox childQuadrant;
	childQuadrant.setMinPoint(minX, minY);
	childQuadrant.setMaxPoint(maxX, maxY);
	return childQuadrant;
}

void CCollisionNode::searchTree(const U32BoundingBox &searchRange, CCollisionObjectVector *targetList) const {
	if (searchRange.intersect(_quadrant)) {
		if (_isExternal) {
			// Search through and add points that are within bounds...
			for (CCollisionObjectVector::size_type objectIndex = 0; objectIndex < _objectList.size(); ++objectIndex) {
				if (!_objectList[objectIndex]->_ignore) {
					const ICollisionObject *currentObject = _objectList[objectIndex];
					targetList->push_back(currentObject);
				}
			}
		} else {
			// Search all of the children...
			for (int childID = 0; childID < NUM_CHILDREN_NODES; childID++) {
				_child[childID]->searchTree(searchRange, targetList);
			}
		}
	}
}

} // End of namespace Scumm
