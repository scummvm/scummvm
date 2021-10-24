//
//  Object.h
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef FREESCAPE_OBJECT_H
#define FREESCAPE_OBJECT_H

#include "common/system.h"
#include "math/vector3d.h"

#include "freescape/gfx.h"

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
	Math::Vector3d getOrigin();
	Math::Vector3d getSize();

	virtual void draw(Freescape::Renderer *gfx) = 0;

	virtual bool isDrawable();
	virtual bool isPlanar();
	bool isInvisible() { return _flags & 0x4; }

	virtual ~Object();

	uint16 _flags;
	Type _type;
	uint16 _objectID;
	Math::Vector3d _origin, _size, _rotation;
};

#endif
