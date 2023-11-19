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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#ifndef FREESCAPE_OBJECT_H
#define FREESCAPE_OBJECT_H

#include "math/aabb.h"

#include "freescape/gfx.h"

namespace Freescape {

enum ObjectType {
	kEntranceType = 0,
	kCubeType = 1,
	kSensorType = 2,
	kRectangleType = 3,

	kEastPyramidType = 4,
	kWestPyramidType = 5,
	kUpPyramidType = 6,
	kDownPyramidType = 7,
	kNorthPyramidType = 8,
	kSouthPyramidType = 9,

	kLineType = 10,
	kTriangleType = 11,
	kQuadrilateralType = 12,
	kPentagonType = 13,
	kHexagonType = 14,

	kGroupType = 15
};

class Object {
public:
	virtual ObjectType getType() { return _type; }
	uint16 getObjectID() { return _objectID; }
	uint16 getObjectFlags() { return _flags; }
	void setObjectFlags(uint32 flags_) { _flags = flags_; }
	Math::Vector3d getOrigin() { return _origin; }
	virtual void setOrigin(Math::Vector3d origin_) { _origin = origin_; };
	Math::Vector3d getSize() { return _size; }

	virtual bool isDrawable() { return false; }
	virtual bool isPlanar() { return false; }
	virtual void scale(int factor) = 0;

	bool isInvisible() { return _flags & 0x40; }
	void makeInvisible() { _flags = _flags | 0x40; }
	void makeVisible() { _flags = _flags & ~0x40; }
	bool isInitiallyInvisible() { return _flags & 0x80; }
	void makeInitiallyInvisible() { _flags = _flags | 0x80; }
	void makeInitiallyVisible() { _flags = _flags & ~0x80; }
	bool isDestroyed() { return _flags & 0x20; }
	void destroy() { _flags = _flags | 0x20; }
	void restore() { _flags = _flags & ~0x20; }
	void toggleVisibility() { _flags = _flags ^ 0x40; }

	virtual ~Object() {}
	virtual Object *duplicate() = 0;

	virtual void draw(Freescape::Renderer *gfx) = 0;

	uint16 _flags;
	ObjectType _type;
	uint16 _objectID;
	Math::Vector3d _origin, _size, _rotation;
	Math::AABB _boundingBox;
	Object *_partOfGroup = nullptr;
};

} // End of namespace Freescape

#endif // FREESCAPE_OBJECT_H
