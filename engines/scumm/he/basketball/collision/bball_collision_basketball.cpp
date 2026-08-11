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

#include "scumm/he/basketball/court.h"
#include "scumm/he/basketball/collision/bball_collision_basketball.h"
#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_sphere.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"
#include "scumm/he/basketball/collision/bball_collision_tree.h"

namespace Scumm {

bool CCollisionBasketball::testCatch(const ICollisionObject &targetObject, U32Distance3D *distance, CBBallCourt *court) {
	if (targetObject._objectType == kPlayer) {
		ICollisionObject *object = const_cast<ICollisionObject *>(&targetObject);
		CCollisionPlayer *player = static_cast<CCollisionPlayer *>(object);
		return (player->testCatch(*this, distance, court));
	} else {
		return false;
	}
}

} // End of namespace Scumm
