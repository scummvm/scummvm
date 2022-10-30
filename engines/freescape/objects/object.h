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

// Based on Phantasma code by Thomas Harte (2013)

#ifndef FREESCAPE_OBJECT_H
#define FREESCAPE_OBJECT_H

#include "math/aabb.h"
#include "math/vector3d.h"

#include "freescape/gfx.h"

namespace Freescape {

class Object {
public:
	typedef enum {
		Entrance = 0,
		Cube = 1,
		Sensor = 2,
		Rectangle = 3,

		EastPyramid = 4,
		WestPyramid = 5,
		UpPyramid = 6,
		DownPyramid = 7,
		NorthPyramid = 8,
		SouthPyramid = 9,

		Line = 10,
		Triangle = 11,
		Quadrilateral = 12,
		Pentagon = 13,
		Hexagon = 14,

		Group = 15
	} Type;

	virtual Type getType();
	uint16 getObjectID();
	uint16 getObjectFlags();
	void setObjectFlags(uint32 flags);
	Math::Vector3d getOrigin();
	virtual void setOrigin(Math::Vector3d origin);
	Math::Vector3d getSize();

	virtual void draw(Freescape::Renderer *gfx) = 0;

	virtual bool isDrawable();
	virtual bool isPlanar();
	bool isInvisible();
	void makeInvisible();
	void makeVisible();
	void toggleVisibility();
	bool isDestroyed();
	void destroy();

	virtual ~Object();

	uint16 _flags;
	Type _type;
	uint16 _objectID;
	Math::Vector3d _origin, _size, _rotation;
	Math::AABB _boundingBox;
};

} // End of namespace Freescape

#endif
