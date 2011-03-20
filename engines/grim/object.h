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

#ifndef GRIM_OBJECT_H
#define GRIM_OBJECT_H

#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/func.h"
#include "common/list.h"
#include <iostream>
using namespace std;
namespace Grim {

class SaveGame;

class Pointer;
class Object {
public:
	Object();
	virtual ~Object();

	virtual const char *typeName() const { return ""; }

	void save(SaveGame *state) const;
	bool restore(SaveGame *state);

	void ref();
	void deref();

protected:
	virtual void saveState(SaveGame *state) const;
	virtual bool restoreState(SaveGame *state);

private:
	int _refCount;
	Common::List<Pointer *> _pointers;

	friend class Pointer;
};

class Pointer {
protected:
	virtual ~Pointer() {}

	void addPointer(Object *obj) {
		obj->_pointers.push_back(this);
	}
	void rmPointer(Object *obj) {
		obj->_pointers.remove(this);
	}

	virtual void resetPointer() {}

	friend class Object;
};

template<class T> class ObjectPtr : public Pointer {
public:
	ObjectPtr() :
		_obj(NULL) {

	}
	ObjectPtr(T *obj) :
		_obj(obj) {
		if (obj) {
			_obj->ref();
			addPointer(obj);
		}
	}
	ObjectPtr(const ObjectPtr<T> &ptr) {
		_obj = NULL;
		*this = ptr;
	}
	~ObjectPtr() {
		if (_obj) {
			rmPointer(_obj);
			_obj->deref();
		}
	}

	ObjectPtr &operator=(T *obj) {
		if (obj != _obj) {
			if (_obj) {
				rmPointer(_obj);
				_obj->deref();
				_obj = NULL;

			}

			if (obj) {
				_obj = obj;
				_obj->ref();
				addPointer(obj);
			}
		}

		return *this;
	}
	ObjectPtr &operator=(const ObjectPtr<T> &ptr) {
		if (_obj != ptr._obj) {
			if (_obj) {
				rmPointer(_obj);
				_obj->deref();
				_obj = NULL;

			}

			if (ptr._obj) {
				_obj = ptr._obj;
				_obj->ref();
				addPointer(_obj);
			}
		}

		return *this;
	}
	bool operator==(const ObjectPtr &ptr) const {
		return (_obj == ptr._obj);
	}
	bool operator==(Object *obj) const {
		return (_obj == obj);
	}
	operator bool() const {
		return (_obj);
	}
	bool operator!() const {
		return (!_obj);
	}

	T *object() const {
		return _obj;
	}
	T *operator->() const {
		return _obj;
	}
	T &operator*() const {
		return *_obj;
	}
	operator T*() const {
		return _obj;
	}

protected:
	void resetPointer() {
		_obj = NULL;
	}

private:
	T *_obj;
};

class ObjectManager {
public:
	static void saveObject(SaveGame *state, Object *object);
	static Object *restoreObject(SaveGame *state);
	static Object *newObject(const char *typeName);

	template<class T> static bool registerType() {
		T obj;
		Common::String type = obj.typeName();
		if (_creators.contains(type)) {
			warning("Type name %s already registered", type.c_str());
			return false;
		}
		_creators.setVal(type, &createObj<T>);

		return true;
	}

private:
	template<class T> static Object *createObj() {
		return new T();
	}

	typedef Object *(*CreatorFunc)();
	static Common::HashMap<Common::String, CreatorFunc> _creators;

};

} // end of namespace Grim

#define GRIM_OBJECT(class) 				\
	public:								\
		const char *typeName() const {	\
			return #class;				\
		}

#endif
