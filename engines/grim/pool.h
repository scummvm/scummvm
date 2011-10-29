
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

template<class T, int32 tag>
class PoolObject : public PoolObjectBase {
public:
	class Pool {
	public:
		typedef typename Common::HashMap<int32, T *>::iterator Iterator;

		Pool();
		~Pool();

		void addObject(T *obj);
		void removeObject(int32 id);

		T *getObject(int32 id);
		Iterator getBegin();
		Iterator getEnd();
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

	int getId() const;
	int32 getTag() const;
	static int32 getTagStatic();

	static Pool *getPool();

protected:
	PoolObject();

private:
	void setId(int id);
	void addPointer(Ptr *pointer) { _pointers.push_back(pointer); }
	void removePointer(Ptr *pointer) { _pointers.remove(pointer); }

	int _id;
	static int s_id;
	static Pool *s_pool;

	Common::List<Ptr *> _pointers;

	friend class Pool;
	friend class Ptr;
};

template <class T, int32 tag>
bool operator==(T *obj, const typename PoolObject<T, tag>::Ptr &ptr) {
	return obj == ptr._obj;
}

template <class T, int32 tag>
bool operator!=(T *obj, const typename PoolObject<T, tag>::Ptr &ptr) {
	return obj != ptr._obj;
}

template <class T, int32 tag>
int PoolObject<T, tag>::s_id = 0;

template <class T, int32 tag>
typename PoolObject<T, tag>::Pool *PoolObject<T, tag>::s_pool = NULL;

template <class T, int32 tag>
PoolObject<T, tag>::PoolObject() {
	++s_id;
	_id = s_id;

	if (!s_pool) {
		s_pool = new Pool();
	}
	s_pool->addObject(static_cast<T *>(this));
}

template <class T, int32 tag>
PoolObject<T, tag>::~PoolObject() {
	s_pool->removeObject(_id);

	for (typename Common::List<Ptr *>::iterator i = _pointers.begin(); i != _pointers.end(); ++i) {
		(*i)->reset();
	}
}

template <class T, int32 tag>
void PoolObject<T, tag>::setId(int id) {
	_id = id;
	if (id > s_id) {
		s_id = id;
	}
}

template <class T, int32 tag>
int PoolObject<T, tag>::getId() const {
	return _id;
}

template <class T, int32 tag>
typename PoolObject<T, tag>::Pool *PoolObject<T, tag>::getPool() {
	if (!s_pool) {
		s_pool = new Pool();
	}
	return s_pool;
}

template <class T, int32 tag>
int32 PoolObject<T, tag>::getTag() const {
	return tag;
}

template <class T, int32 tag>
int32 PoolObject<T, tag>::getTagStatic() {
	return tag;
}

/**
 * @class Pool
 */

template <class T, int32 tag>
PoolObject<T, tag>::Pool::Pool() :
	_restoring(false) {

}

template <class T, int32 tag>
PoolObject<T, tag>::Pool::~Pool() {
	PoolObject<T, tag>::s_pool = NULL;
}

template <class T, int32 tag>
void PoolObject<T, tag>::Pool::addObject(T *obj) {
	if (!_restoring) {
		_map.setVal(obj->_id, obj);
	}
}

template <class T, int32 tag>
void PoolObject<T, tag>::Pool::removeObject(int32 id) {
	_map.erase(id);
}

template <class T, int32 tag>
T *PoolObject<T, tag>::Pool::getObject(int32 id) {
	return _map.getVal(id, NULL);
}

template <class T, int32 tag>
typename PoolObject<T, tag>::Pool::Iterator PoolObject<T, tag>::Pool::getBegin() {
	return _map.begin();
}

template <class T, int32 tag>
typename PoolObject<T, tag>::Pool::Iterator PoolObject<T, tag>::Pool::getEnd() {
	return _map.end();
}

template <class T, int32 tag>
int PoolObject<T, tag>::Pool::getSize() const {
	return _map.size();
}

template <class T, int32 tag>
void PoolObject<T, tag>::Pool::deleteObjects() {
	while (!_map.empty()) {
		delete getBegin()->_value;
	}
	delete this;
}

template <class T, int32 tag>
void PoolObject<T, tag>::Pool::saveObjects(SaveGame *state) {
	state->beginSection(tag);

	state->writeLEUint32(_map.size());
	for (Iterator i = getBegin(); i != getEnd(); ++i) {
		T *a = i->_value;
		state->writeLESint32(i->_key);

		a->saveState(state);
	}

	state->endSection();
}

template <class T, int32 tag>
void PoolObject<T, tag>::Pool::restoreObjects(SaveGame *state) {
	state->beginSection(tag);

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
	for (Iterator i = getBegin(); i != getEnd(); i++) {
		delete i->_value;
	}
	_map = tempMap;
	_restoring = false;

	state->endSection();
}

template<class T, int32 tag>
typename PoolObject<T, tag>::Ptr &PoolObject<T, tag>::Ptr::operator=(T *obj) {
	if (_obj) {
		_obj->removePointer(this);
	}
	_obj = obj;
	if (obj) {
		obj->addPointer(this);
	}

	return *this;
}

template<class T, int32 tag>
typename PoolObject<T, tag>::Ptr &PoolObject<T, tag>::Ptr::operator=(const Ptr &ptr) {
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
