
#include "object.h"
#include "engines/grim/savegame.h"

#include "engines/grim/lua/lobject.h"

#include "engines/grim/font.h"

DECLARE_SINGLETON(Grim::ObjectManager)

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
