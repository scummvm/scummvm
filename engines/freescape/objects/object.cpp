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

#include "freescape/objects/object.h"

namespace Freescape {

Object::Type Object::getType() { return type; }
uint16 Object::getObjectID() { return objectID; }
uint16 Object::getObjectFlags() { return flags; }
void Object::setObjectFlags(uint32 _flags) { flags = _flags; }
Math::Vector3d Object::getOrigin() { return origin; }
void Object::setOrigin(Math::Vector3d _origin) { origin = _origin; };
Math::Vector3d Object::getSize() { return size; }

bool Object::isDrawable() { return false; }
bool Object::isPlanar() { return false; }

bool Object::isInvisible() { return flags & 0x80; }
void Object::makeInvisible() { flags = flags | 0x80; }
void Object::makeVisible() { flags = flags & ~0x80; }
bool Object::isDestroyed() { return flags & 0x20; }
void Object::destroy() { flags = flags | 0x20; }
void Object::toggleVisibility() { flags = flags ^ 0x80; }

Object::~Object() {}

} // End of namespace Freescape
