//
//  Object.h
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Object__
#define __Phantasma__Object__

#include "common/system.h"

//#include <vector>
//#include "freescape/language/instruction.h"

typedef struct Vector3d {
	uint16 x, y, z;
	uint16 &operator[](int index) {
		switch (index) {
		default:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}
	};
} Vector3d;

class VertexBuffer;
class DrawElementsBuffer;
class BatchDrawer;
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

	Type getType();
	uint16 getObjectID();
	Vector3d getOrigin();
	Vector3d getSize();

	/*
		virtual void setupOpenGL(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer);
		virtual void draw(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer, BatchDrawer *batchDrawer, bool allowPolygonOffset);
		*/
	virtual bool isDrawable();
	virtual bool isPlanar();

	virtual ~Object();

protected:
	Type type;
	uint16 objectID;
	Vector3d origin, size;
};

/*
#include "Entrance.h"
#include "GeometricObject.h"
#include "Group.h"
#include "Sensor.h"
*/

#endif /* defined(__Phantasma__Object__) */
