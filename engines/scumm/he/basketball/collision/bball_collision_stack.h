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

#ifndef SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_STACK_H
#define SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_STACK_H

#ifdef ENABLE_HE

#include "scumm/he/basketball/collision/bball_collision_support_obj.h"

#include "common/array.h"

namespace Scumm {

class ICollisionObject;

class CCollisionObjectStack : public Common::Array<const ICollisionObject *> {
public:
	void clear();
};

class CCollisionObjectVector : public Common::Array<const ICollisionObject *> {
public:
	int getMinPoint(EDimension dimension) const;
	int getMaxPoint(EDimension dimension) const;
	bool contains(const ICollisionObject &object) const;
};

} // End of namespace Scumm

#endif // ENABLE_HE

#endif // SCUMM_HE_BASKETBALL_COLLISION_BBALL_COLLISION_STACK_H
