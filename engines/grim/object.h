/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRIM_OBJECT_H
#define GRIM_OBJECT_H

#include "common/list.h"

namespace Grim {

class SaveGame;

class Pointer;
class Object {
public:
	Object();
	virtual ~Object();

	void reset() { };
	void reference();
	void dereference();

	int32 getId() const;

private:
	void setId(int32 id);
	int _refCount;
	Common::List<Pointer *> _pointers;
	int32 _id;
	static int32 s_id;

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
		_obj(nullptr) {

	}
	ObjectPtr(T *obj) :
		_obj(obj) {
		if (obj) {
			Object *o = (Object *)_obj;
			o->reference();
			addPointer(o);
		}
	}
	ObjectPtr(const ObjectPtr<T> &ptr) : Pointer() {
		_obj = nullptr;
		*this = ptr;
	}
	~ObjectPtr() {
		if (_obj) {
			Object *o = (Object *)_obj;
			rmPointer(o);
			o->dereference();
		}
	}

	ObjectPtr &operator=(T *obj) {
		if (obj != _obj) {
			if (_obj) {
				rmPointer(_obj);
				_obj->dereference();
				_obj = nullptr;

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
				_obj = nullptr;

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
	void resetPointer() override {
		_obj = nullptr;
	}

private:
	T *_obj;
};

}

#endif
