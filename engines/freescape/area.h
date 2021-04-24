//
//  Area.h
//  Phantasma
//
//  Created by Thomas Harte on 25/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#ifndef __Phantasma__Area__
#define __Phantasma__Area__

#include "common/hashmap.h"
#include "freescape/objects/object.h"
//#include "VertexBuffer.h"
//#include "DrawElementsBuffer.h"

typedef Common::HashMap<uint16, Object *> ObjectMap;

class BatchDrawer;
class Area {
public:
	Area(
		uint16 areaID,
		ObjectMap *objectsByID,
		ObjectMap *entrancesByID);
	virtual ~Area();

	Object *objectWithID(uint16 objectID);
	Object *entranceWithID(uint16 objectID);
	uint16 getAreaID();
	void draw(/*bool allowPolygonOffset, BatchDrawer *batchDrawer*/);

private:
	uint16 areaID;
	ObjectMap *objectsByID;
	ObjectMap *entrancesByID;
	Common::Array<Object *> drawableObjects;

	Object *objectWithIDFromMap(ObjectMap *map, uint16 objectID);

	VertexBuffer *vertexBuffer;
	DrawElementsBuffer *drawElementsBuffer;
};

#endif /* defined(__Phantasma__Area__) */
