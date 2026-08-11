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

CCollisionObjectTree::CCollisionObjectTree(const CCollisionObjectVector &inputObjects) : _maxHeight(INIT_MAX_HEIGHT),
																						 _maxObjectsInNode(INIT_MAX_OBJECTS),
																						 _root(nullptr),
																						 _errorFlag(false) {
	initialize(inputObjects);
}

CCollisionObjectTree::~CCollisionObjectTree() {
	if (_root) {
		delete _root;
		_root = nullptr;
	}
}

void CCollisionObjectTree::initialize(const CCollisionObjectVector &inputObjects) {
	if (_root) {
		delete _root;
		_root = nullptr;
	}

	U32BoundingBox buildRange;
	buildRange.setMinPoint(inputObjects.getMinPoint(X_INDEX), inputObjects.getMinPoint(Y_INDEX));
	buildRange.setMaxPoint(inputObjects.getMaxPoint(X_INDEX), inputObjects.getMaxPoint(Y_INDEX));
	_root = buildSelectionStructure(inputObjects, 0, buildRange);
}

void CCollisionObjectTree::selectObjectsInBound(const U32BoundingBox &bound, CCollisionObjectVector *targetVector) {
	_root->searchTree(bound, targetVector);

	if (targetVector->size() <= 0) {
		warning("CCollisionObjectTree::selectObjectsInBound(): (targetVector->size() <= 0) Something went really wrong with a collision, ignore and U32 will attempt to correct.");
		_errorFlag = true;
		return;
	}

	// Here we make sure that we don't have any duplicate objects in our target stack...
	Std::sort(targetVector->begin(), targetVector->end());

	CCollisionObjectVector::iterator newEnd = Std::unique(targetVector->begin(), targetVector->end());
	targetVector->erase(newEnd, targetVector->end());
}

bool CCollisionObjectTree::checkErrors() {
	if (_errorFlag) {
		_errorFlag = false;
		return true;
	} else {
		return false;
	}
}

CCollisionNode *CCollisionObjectTree::buildSelectionStructure(const CCollisionObjectVector &inputObjects, int currentLevel, const U32BoundingBox &nodeRange) {
	// Create a new node, containing all of the input points...
	CCollisionNode *newNode = new CCollisionNode(inputObjects);
	newNode->_quadrant = nodeRange;

	// See if we are at the final level, or if we have reached our target occupancy...
	if ((currentLevel == _maxHeight) ||
		(inputObjects.size() <= _maxObjectsInNode)) {
		newNode->_isExternal = true;
	} else {
		newNode->_isExternal = false;

		// Otherwise, break the inputPoints into 4 new point lists...
		CCollisionObjectVector newList[NUM_CHILDREN_NODES];
		U32BoundingBox newRange[NUM_CHILDREN_NODES];
		int childID;

		for (childID = 0; childID < NUM_CHILDREN_NODES; childID++) {
			newRange[childID] = CCollisionNode::getChildQuadrant(nodeRange, (EChildID)childID);
		}

		// Go through each point in the list...
		for (size_t i = 0; i < inputObjects.size(); i++) {
			const ICollisionObject *currentObject = inputObjects[i];

			// Figure out which child each point belongs to...
			for (childID = 0; childID < NUM_CHILDREN_NODES; ++childID) {
				if (newRange[childID].intersect(currentObject->getBoundingBox())) {
					newList[childID].push_back(currentObject);
				}
			}
		}

		// Make recursive calls...
		for (childID = 0; childID < NUM_CHILDREN_NODES; ++childID) {
			newNode->_child[childID] = buildSelectionStructure(newList[childID],
															   currentLevel + 1,
															   newRange[childID]);
		}
	}

	return newNode;
}

} // End of namespace Scumm
