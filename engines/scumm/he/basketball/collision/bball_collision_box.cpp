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

U32FltPoint3D CCollisionBox::findNearestPoint(const U32FltPoint3D &testPoint) const {
	U32FltPoint3D boxPoint;

	for (int i = X_INDEX; i <= Z_INDEX; i++) {
		EDimension dimension = (EDimension)i;

		if (testPoint[dimension] < minPoint[dimension]) {
			boxPoint[dimension] = minPoint[dimension];
		} else if (testPoint[dimension] > maxPoint[dimension]) {
			boxPoint[dimension] = maxPoint[dimension];
		} else {
			boxPoint[dimension] = testPoint[dimension];
		}
	}

	return boxPoint;
}

U32BoundingBox CCollisionBox::getBoundingBox() const {
	return *this;
}

U32BoundingBox CCollisionBox::getBigBoundingBox() const {
	return *this;
}

} // End of namespace Scumm
