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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_TREE_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_TREE_H

#ifdef ENABLE_HE

#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"

namespace Scumm {

#define INIT_MAX_HEIGHT  10
#define INIT_MAX_OBJECTS 5

class CCollisionObjectTree {
public:
	CCollisionObjectTree() : _maxHeight(INIT_MAX_HEIGHT),
							 _maxObjectsInNode(INIT_MAX_OBJECTS),
							 _root(nullptr),
							 _errorFlag(false) {}

	CCollisionObjectTree(const CCollisionObjectVector &inputObjects);
	~CCollisionObjectTree();

	void initialize(const CCollisionObjectVector &inputObjects);
	void selectObjectsInBound(const U32BoundingBox &bound, CCollisionObjectVector *targetVector);

	bool checkErrors();

private:
	CCollisionNode *buildSelectionStructure(const CCollisionObjectVector &inputObjects, int currentLevel, const U32BoundingBox &nodeRange);

	int _maxHeight;
	size_t _maxObjectsInNode;
	CCollisionNode *_root;
	bool _errorFlag;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_TREE_H
