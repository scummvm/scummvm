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
#include "scumm/he/basketball/collision/bball_collision_player.h"
#include "scumm/he/basketball/collision/bball_collision_support_obj.h"
#include "scumm/he/basketball/collision/bball_collision_object.h"
#include "scumm/he/basketball/collision/bball_collision_sphere.h"
#include "scumm/he/basketball/collision/bball_collision_box.h"
#include "scumm/he/basketball/collision/bball_collision_cylinder.h"
#include "scumm/he/basketball/collision/bball_collision_stack.h"
#include "scumm/he/basketball/collision/bball_collision_node.h"
#include "scumm/he/basketball/collision/bball_collision_tree.h"

namespace Scumm {

void CCollisionPlayer::startBlocking(int blockHeight, int blockTime) {
	assert(blockTime != 0);

	_maxBlockHeight = blockHeight;
	_blockTime = blockTime;

	int blockIncrement = _maxBlockHeight / _blockTime;

	_blockHeight += blockIncrement;
	height += blockIncrement;
}

void CCollisionPlayer::holdBlocking() {
	int blockIncrement = _maxBlockHeight / _blockTime;

	if (_velocity.z > 0) {
		if ((_blockHeight + blockIncrement) <= _maxBlockHeight) {
			_blockHeight += blockIncrement;
			height += blockIncrement;
		} else {
			blockIncrement = _maxBlockHeight - _blockHeight;
			_blockHeight += blockIncrement;
			height += blockIncrement;

			assert(_blockHeight == _maxBlockHeight);
		}
	} else if (_velocity.z < 0) {
		if ((_blockHeight - blockIncrement) >= 0) {
			_blockHeight -= blockIncrement;
			height -= blockIncrement;
		} else {
			blockIncrement = _blockHeight;
			_blockHeight -= blockIncrement;
			height -= blockIncrement;

			assert(_blockHeight == 0);
		}
	}
}

void CCollisionPlayer::endBlocking() {
	height -= _blockHeight;
	_blockHeight = 0;
}

bool CCollisionPlayer::testCatch(const ICollisionObject &targetObject, U32Distance3D *distance, CBBallCourt *court) {
	if (&targetObject == (ICollisionObject *)&court->_virtualBall) {
		int oldHeight = height;
		height += _catchHeight;

		bool retVal = testObjectIntersection(targetObject, distance);

		height = oldHeight;

		return retVal;
	} else {
		return false;
	}
}

} // End of namespace Scumm
