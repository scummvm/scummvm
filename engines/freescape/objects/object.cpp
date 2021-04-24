//
//  Object.cpp
//  Phantasma
//
//  Created by Thomas Harte on 18/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "freescape/objects/object.h"
#include "freescape/freescape.h"
#include "freescape/gfx.h"

Object::Type Object::getType() { return type; }
uint16 Object::getObjectID() { return objectID; }
//Vector3d Object::getOrigin()	{	return origin;		}
//Vector3d Object::getSize()		{	return size;		}

//void Object::setupOpenGL(VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer)					{}
void Object::draw(Freescape::Renderer *gfx/*VertexBuffer *areaVertexBuffer, DrawElementsBuffer *areaDrawElementsBuffer, BatchDrawer *batchDrawer, bool allowPolygonOffset*/) {
    gfx;
}
bool Object::isDrawable() { return false; }
bool Object::isPlanar() { return false; }

Object::~Object() {}
