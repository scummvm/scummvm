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

namespace Freescape {

Object::Type Object::getType() { return _type; }
uint16 Object::getObjectID() { return _objectID; }
uint16 Object::getObjectFlags() { return _flags; }
void Object::setObjectFlags(uint32 flags) { _flags = flags; }
Math::Vector3d Object::getOrigin() { return origin; }
void Object::setOrigin(Math::Vector3d _origin) { origin = _origin; };
Math::Vector3d Object::getSize() { return size; }

bool Object::isDrawable() { return false; }
bool Object::isPlanar() { return false; }

bool Object::isInvisible() { return _flags & 0x80; }
void Object::makeInvisible() { _flags = _flags | 0x80; }
void Object::makeVisible() { _flags = _flags & ~0x80; }
bool Object::isDestroyed() { return _flags & 0x20; }
void Object::destroy() { _flags = _flags | 0x20; }
void Object::toggleVisibility() { _flags = _flags ^ 0x80; }

Object::~Object() {}

} // End of namespace Freescape
