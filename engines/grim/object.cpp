/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "engines/grim/lua/lobject.h"

#include "engines/grim/savegame.h"
#include "engines/grim/font.h"
#include "engines/grim/object.h"


DECLARE_SINGLETON(Grim::ObjectManager);

namespace Grim {

Object::Object() : _refCount(0) {

}

Object::~Object() {
	if (lua_isopen()) {
		luaO_resetObject(this);	//after climbing the ties rope an ObjectState gets deleted but not removed
	}							//from the lua's userdata list, resulting in a dangling pointer
								//that breaks the saving. We need to reset to NULL the pointer manually.
	for (Common::List<Pointer *>::iterator i = _pointers.begin(); i != _pointers.end(); ++i) {
		(*i)->resetPointer();
	}
}

void Object::saveState(SaveGame *) const {

}

bool Object::restoreState(SaveGame *) {
	return false;
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


void ObjectManager::saveObject(SaveGame *state, Object *object) {
	const char *str = object->typeName();
	int32 len = strlen(str);

	state->writeLEUint32(len);
	state->write(str, len);

	object->saveState(state);
}

ObjectPtr<Object> ObjectManager::restoreObject(SaveGame *state) {
	const char *str = state->readCharString();

	ObjectPtr<Object> ptr;
	Common::String type = str;
	delete[] str;
	if (_creators.contains(type)) {
		CreatorFunc func = _creators.getVal(type);
		ptr = (func)(state);
	} else {
		error("Type name \"%s\" not registered", type.c_str());
	}

	return ptr;
}

}
