/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/lua/lobject.h"

#include "engines/grim/savegame.h"
#include "engines/grim/font.h"
#include "engines/grim/object.h"


namespace Grim {

int32 Object::s_id = 0;

Object::Object() :
	_refCount(0) {

	++s_id;
	_id = s_id;
}

Object::~Object() {
	for (Common::List<Pointer *>::iterator i = _pointers.begin(); i != _pointers.end(); ++i) {
		(*i)->resetPointer();
	}
}

void Object::reference() {
	++_refCount;
}

void Object::dereference() {
	if (_refCount > 0) {
		--_refCount;
	}

	if (_refCount == 0) {
		_refCount = -1;
		delete this;
	}
}

int32 Object::getId() {
	return _id;
}

}
