//
//  Object.cpp
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "freescape/objects/object.h"

Object::Type Object::getType() { return type; }
uint16 Object::getObjectID() { return objectID; }
//Vector3d Object::getOrigin()	{	return origin;		}
//Vector3d Object::getSize()		{	return size;		}

//void Object::setupOpenGL(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer)					{}
//void Object::draw(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer, BatchDrawer *batchDrawer, bool allowPolygonOffset)	{}
bool Object::isDrawable() { return false; }
bool Object::isPlanar() { return false; }

Object::~Object() {}
