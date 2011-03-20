
#include "object.h"
#include "engines/grim/savegame.h"

#include "engines/grim/lua/lobject.h"

namespace Grim {

typedef Object *(*CreatorFunc)();
Common::HashMap<Common::String, CreatorFunc> ObjectManager::_creators;

Object::Object() : _refCount(0) {

}

Object::~Object() {
	luaO_resetObject(this);	//after climbing the ties rope an ObjectState gets deleted but not removed
							//from the lua's userdata list, resulting in a dangling pointer
							//that breaks the saving. We need to reset to NULL the pointer manually.
	for (Common::List<Pointer *>::iterator i = _pointers.begin(); i != _pointers.end(); ++i) {
		(*i)->resetPointer();
	}
}

void Object::save(SaveGame *state) const {
	state->writeLEUint32(_refCount);

	saveState(state);
}

bool Object::restore(SaveGame *state) {
	_refCount = state->readLEUint32();

	return restoreState(state);
}

void Object::saveState(SaveGame */*state*/) const {

}

bool Object::restoreState(SaveGame */*state*/) {
	return false;
}

void Object::ref() {
	++_refCount;
}

void Object::deref() {
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

	object->save(state);
}

Object *ObjectManager::restoreObject(SaveGame *state) {
	int32 len = state->readLEUint32();
	char *str = new char[len + 1];
	state->read(str, len);
	str[len] = '\0';

	Object *o = newObject(str);
	delete[] str;
	o->restore(state);
	return o;
}

Object *ObjectManager::newObject(const char *typeName) {
	Common::String type = typeName;
	if (_creators.contains(type)) {
		CreatorFunc func = _creators.getVal(type);
		Object *o = (func)();
		return o;
	} else {
		warning("Type name \"%s\" not registered", typeName);
	}

	return 0;
}

}
