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
#include "common/debug.h"
#include "common/singleton.h"
#include "common/textconsole.h"

namespace Grim {

class SaveGame;

class Pointer;
class Object {
public:
	Object();
	virtual ~Object();

	void reference();
	void dereference();

	int32 getId();

private:
	int _refCount;
	Common::List<Pointer *> _pointers;
	int32 _id;
	static int32 s_id;

	friend class GrimEngine;
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
			_obj->reference();
			addPointer(obj);
		}
	}
	ObjectPtr(const ObjectPtr<T> &ptr) : Pointer() {
		_obj = NULL;
		*this = ptr;
	}
	~ObjectPtr() {
		if (_obj) {
			rmPointer(_obj);
			_obj->dereference();
		}
	}

	ObjectPtr &operator=(T *obj) {
		if (obj != _obj) {
			if (_obj) {
				rmPointer(_obj);
				_obj->dereference();
				_obj = NULL;

			}

			if (obj) {
				_obj = obj;
				_obj->reference();
				addPointer(obj);
			}
		}

		return *this;
	}
	ObjectPtr &operator=(const ObjectPtr<T> &ptr) {
		if (_obj != ptr._obj) {
			if (_obj) {
				rmPointer(_obj);
				_obj->dereference();
				_obj = NULL;

			}

			if (ptr._obj) {
				_obj = ptr._obj;
				_obj->reference();
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

}

#endif
