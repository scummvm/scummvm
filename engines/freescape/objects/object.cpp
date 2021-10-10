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
uint16 Object::getObjectFlags() { return flags; }
Vector3d Object::getOrigin() { return origin; }
Vector3d Object::getSize() { return size; }

//void Object::draw(Freescape::Renderer *gfx) {
//	gfx;
//}
bool Object::isDrawable() { return false; }
bool Object::isPlanar() { return false; }

Object::~Object() {}
