/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_POOL_H
#define GRIM_POOL_H

#include "common/hashmap.h"
#include "common/list.h"

#include "engines/grim/savegame.h"

namespace Grim {

template<class T> class Pool;

class PoolObjectBase {
public:
	virtual ~PoolObjectBase() { };
	virtual int getId() const = 0;
	virtual int32 getTag() const = 0;
};

template<class T>
class PoolObject : public PoolObjectBase {
public:
	class Pool {
	public:
		template<class it, class Type>
		class Iterator {
		public:
			Iterator(const Iterator &i) : _i(i._i) { }
			Iterator(const it &i) : _i(i) { }

			int32 getId() const { return _i->_key; }
			Type &getValue() const { return _i->_value; }

			Type &operator*() const { return _i->_value; }

			Iterator &operator=(const Iterator &i) { _i = i._i; return *this; }

			bool operator==(const Iterator i) const { return _i == i._i; }
			bool operator!=(const Iterator i) const { return _i != i._i; }

			Iterator &operator++() { ++_i; return *this; }
			Iterator operator++(int) { Iterator iter = *this; ++_i; return iter; }

			Iterator &operator--() { --_i; return *this; }
			Iterator operator--(int) { Iterator iter = *this; --_i; return iter; }

		private:
			it _i;
		};

		typedef Iterator<typename Common::HashMap<int32, T *>::iterator, T *> iterator;
		typedef Iterator<typename Common::HashMap<int32, T *>::const_iterator, T *const> const_iterator;

		Pool();
		~Pool();

		void addObject(T *obj);
		void removeObject(int32 id);

		T *getObject(int32 id);
		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;
		int getSize() const;
		void deleteObjects();
		void saveObjects(SaveGame *save);
		void restoreObjects(SaveGame *save);

	private:
		bool _restoring;
		Common::HashMap<int32, T*> _map;
	};

	/**
	 * @short Smart pointer class
	 * This class wraps a C pointer to T, subclass of PoolObject, which gets reset to NULL as soon as
	 * the object is deleted, e.g by Pool::restoreObjects().
	 * Its operator overloads allows the Ptr class to be used as if it was a raw C pointer.
	 */
	class Ptr {
	public:
		Ptr() : _obj(NULL) { }
		Ptr(T *obj) : _obj(obj) {
			if (_obj)
				_obj->addPointer(this);
		}
		Ptr(const Ptr &ptr) : _obj(ptr._obj) {
			if (_obj)
				_obj->addPointer(this);
		}
		~Ptr() {
			if (_obj)
				_obj->removePointer(this);
		}

		Ptr &operator=(T *obj);
		Ptr &operator=(const Ptr &ptr);

		inline operator bool() const { return _obj; }
		inline bool operator!() const { return !_obj; }
		inline bool operator==(T *obj) const { return _obj == obj; }
		inline bool operator!=(T *obj) const { return _obj != obj; }

		inline T *operator->() const { return _obj; }
		inline T &operator*() const { return *_obj; }
		inline operator T*() const { return _obj; }

	private:
		inline void reset() { _obj = NULL; }

		T *_obj;

		friend class PoolObject;
	};

	virtual ~PoolObject();

	void setId(int id);

	int getId() const override;
	virtual int32 getTag() const override { return T::getStaticTag(); }

	static Pool &getPool();

protected:
	PoolObject();

	static void saveStaticState(SaveGame *state) {}
	static void restoreStaticState(SaveGame *state) {}

private:
	void addPointer(Ptr *pointer) { _pointers.push_back(pointer); }
	void removePointer(Ptr *pointer) { _pointers.remove(pointer); }

	int _id;
	static int s_id;
	static Pool *s_pool;

	Common::List<Ptr *> _pointers;

	friend class Pool;
	friend class Ptr;
};

template <class T>
bool operator==(T *obj, const typename PoolObject<T>::Ptr &ptr) {
	return obj == ptr._obj;
}

template <class T>
bool operator!=(T *obj, const typename PoolObject<T>::Ptr &ptr) {
	return obj != ptr._obj;
}

template <class T>
int PoolObject<T>::s_id = 0;

template <class T>
typename PoolObject<T>::Pool *PoolObject<T>::s_pool = NULL;

template <class T>
PoolObject<T>::PoolObject() {
	++s_id;
	_id = s_id;

	if (!s_pool) {
		s_pool = new Pool();
	}
	s_pool->addObject(static_cast<T *>(this));
}

template <class T>
PoolObject<T>::~PoolObject() {
	s_pool->removeObject(_id);

	for (typename Common::List<Ptr *>::iterator i = _pointers.begin(); i != _pointers.end(); ++i) {
		(*i)->reset();
	}
}

template <class T>
void PoolObject<T>::setId(int id) {
	_id = id;
	if (id > s_id) {
		s_id = id;
	}
}

template <class T>
int PoolObject<T>::getId() const {
	return _id;
}

template <class T>
typename PoolObject<T>::Pool &PoolObject<T>::getPool() {
	if (!s_pool) {
		s_pool = new Pool();
	}
	return *s_pool;
}

/**
 * @class Pool
 */

template <class T>
PoolObject<T>::Pool::Pool() :
	_restoring(false) {
}

template <class T>
PoolObject<T>::Pool::~Pool() {
	PoolObject<T>::s_pool = NULL;
}

template <class T>
void PoolObject<T>::Pool::addObject(T *obj) {
	if (!_restoring) {
		_map.setVal(obj->_id, obj);
	}
}

template <class T>
void PoolObject<T>::Pool::removeObject(int32 id) {
	_map.erase(id);
}

template <class T>
T *PoolObject<T>::Pool::getObject(int32 id) {
	return _map.getVal(id, NULL);
}

template <class T>
typename PoolObject<T>::Pool::iterator PoolObject<T>::Pool::begin() {
	return iterator(_map.begin());
}

template <class T>
typename PoolObject<T>::Pool::const_iterator PoolObject<T>::Pool::begin() const {
	return const_iterator(_map.begin());
}

template <class T>
typename PoolObject<T>::Pool::iterator PoolObject<T>::Pool::end() {
	return iterator(_map.end());
}

template <class T>
typename PoolObject<T>::Pool::const_iterator PoolObject<T>::Pool::end() const {
	return const_iterator(_map.end());
}

template <class T>
int PoolObject<T>::Pool::getSize() const {
	return _map.size();
}

template <class T>
void PoolObject<T>::Pool::deleteObjects() {
	while (!_map.empty()) {
		delete *begin();
	}
	delete this;
}

template <class T>
void PoolObject<T>::Pool::saveObjects(SaveGame *state) {
	state->beginSection(T::getStaticTag());

	T::saveStaticState(state);

	state->writeLEUint32(_map.size());
	for (iterator i = begin(); i != end(); ++i) {
		T *a = *i;
		state->writeLESint32(i.getId());

		a->saveState(state);
	}

	state->endSection();
}

template <class T>
void PoolObject<T>::Pool::restoreObjects(SaveGame *state) {
	state->beginSection(T::getStaticTag());

	T::restoreStaticState(state);

	int32 size = state->readLEUint32();
	_restoring = true;
	Common::HashMap<int32, T *> tempMap;
	for (int32 i = 0; i < size; ++i) {
		int32 id = state->readLESint32();
		T *t = _map.getVal(id);
		_map.erase(id);
		if (!t) {
			t = new T();
			t->setId(id);
		}
		tempMap[id] = t;
		t->restoreState(state);
	}
	for (iterator i = begin(); i != end(); ++i) {
		delete *i;
	}
	_map = tempMap;
	_restoring = false;

	state->endSection();
}

template<class T>
typename PoolObject<T>::Ptr &PoolObject<T>::Ptr::operator=(T *obj) {
	if (_obj) {
		_obj->removePointer(this);
	}
	_obj = obj;
	if (obj) {
		obj->addPointer(this);
	}

	return *this;
}

template<class T>
typename PoolObject<T>::Ptr &PoolObject<T>::Ptr::operator=(const Ptr &ptr) {
	if (_obj) {
		_obj->removePointer(this);
	}
	_obj = ptr._obj;
	if (_obj) {
		_obj->addPointer(this);
	}

	return *this;
}

}

#endif
