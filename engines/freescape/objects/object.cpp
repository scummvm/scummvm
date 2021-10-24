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

Object::Type Object::getType() { return _type; }
uint16 Object::getObjectID() { return _objectID; }
uint16 Object::getObjectFlags() { return _flags; }
Math::Vector3d Object::getOrigin() { return _origin; }
Math::Vector3d Object::getSize() { return _size; }

//void Object::draw(Freescape::Renderer *gfx) {
//	gfx;
//}
bool Object::isDrawable() { return false; }
bool Object::isPlanar() { return false; }

Object::~Object() {}
