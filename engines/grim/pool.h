
#ifndef GRIM_POOL_H
#define GRIM_POOL_H

#include "common/hashmap.h"

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

	virtual ~PoolObject();

	int getId() const;
	int32 getTag() const;
	static int32 getTagStatic();

	static Pool *getPool();

protected:
	PoolObject();

	virtual void resetInternalData();

private:
	void setId(int id);

	int _id;
	static int s_id;
	static Pool *s_pool;

	friend class Pool;
};

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
void PoolObject<T, tag>::resetInternalData() {

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
		// Can't use T directly here, since resetInternalData() is protected and
		// Pool is friend of PoolObject, not of T.
		PoolObject<T, tag> *t = i->_value;
		t->resetInternalData();
		delete t;
	}
	_map = tempMap;
	_restoring = false;

	state->endSection();
}

}

#endif
