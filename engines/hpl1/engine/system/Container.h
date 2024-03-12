/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_CONTAINER_H
#define HPL_CONTAINER_H

#include "common/array.h"
#include "common/list.h"
#include "hpl1/engine/system/MemoryManager.h"
#include "common/stablemap.h"

namespace hpl {

//---------------------------------

class iContainerIterator {
	friend class cSerializeClass;

public:
	virtual ~iContainerIterator() {}

protected:
	virtual bool HasNext() = 0;

	virtual void *NextPtr() = 0;
};

//---------------------------------

class iContainer {
	friend class cSerializeClass;

public:
	virtual ~iContainer() = default;
	virtual size_t Size() = 0;
	virtual void Clear() = 0;

protected:
	virtual void AddVoidPtr(void **apPtr) = 0;
	virtual void AddVoidClass(void *apClass) = 0;

	virtual iContainerIterator *CreateIteratorPtr() = 0;
};

//---------------------------------

class iContainerKeyPair {
public:
	virtual ~iContainerKeyPair() = default;
	virtual size_t Size() = 0;

	virtual void AddVoidPtr(void *apKey, void **apClass) = 0;
	virtual void AddVoidClass(void *apKey, void *apClass) = 0;
};

//---------------------------------

template<class T>
class cContainerVecIterator : public iContainerIterator {
	void *NextPtr() {
		return &Next();
	}

public:
	cContainerVecIterator(Common::Array<T> *apVec) {
		mpVec = apVec;
		mIt = apVec->begin();
	}

	bool HasNext() {
		return mIt != mpVec->end();
	}

	T &Next() {
		T &val = *mIt;
		mIt++;
		return val;
	}

	T &PeekNext() {
		return *mIt;
	}

	void Erase() {
		if (mIt != mpVec->end())
			mIt = mpVec->erase(mIt);
	}

private:
	Common::Array<T> *mpVec;
	typename Common::Array<T>::iterator mIt;
};

////////////////////////////

template<class T>
class cContainerVec : public iContainer {
private:
	void AddVoidPtr(void **apPtr) {
		mvVector.push_back(*((T *)apPtr));
	}
	void AddVoidClass(void *apClass) {
		mvVector.push_back(*((T *)apClass));
	}
	iContainerIterator *CreateIteratorPtr() {
		return hplNew(cContainerVecIterator<T>, (&mvVector));
	}

public:
	cContainerVec() {}

	//////////////////////
	size_t Size() {
		return mvVector.size();
	}

	void Clear() {
		mvVector.clear();
	}

	//////////////////////

	void Reserve(size_t alSize) {
		mvVector.reserve(alSize);
	}

	void Resize(size_t alSize) {
		mvVector.resize(alSize);
	}

	void Add(T aVal) {
		mvVector.push_back(aVal);
	}

	//////////////////////

	cContainerVecIterator<T> GetIterator() {
		return cContainerVecIterator<T>(&mvVector);
	}

	//////////////////////

	T &operator[](size_t alX) {
		return mvVector[alX];
	}

	//////////////////////

	Common::Array<T> mvVector;
};

//---------------------------------

template<class T>
class cContainerListIterator : public iContainerIterator {
	void *NextPtr() {
		return &Next();
	}

public:
	cContainerListIterator(Common::List<T> *apVec) {
		mpVec = apVec;
		mIt = apVec->begin();
	}

	virtual ~cContainerListIterator() {}

	bool HasNext() {
		return mIt != mpVec->end();
	}

	T &Next() {
		T &val = *mIt;
		mIt++;
		return val;
	}

	T &PeekNext() {
		return *mIt;
	}

	void Erase() {
		if (mIt != mpVec->end())
			mIt = mpVec->erase(mIt);
	}

private:
	Common::List<T> *mpVec;
	typename Common::List<T>::iterator mIt;
};

////////////////////////////

template<class T>
class cContainerList : public iContainer {
private:
	void AddVoidPtr(void **apPtr) {
		mvVector.push_back(*((T *)apPtr));
	}
	void AddVoidClass(void *apClass) {
		mvVector.push_back(*((T *)apClass));
	}
	iContainerIterator *CreateIteratorPtr() {
		return hplNew(cContainerListIterator<T>, (&mvVector));
	}

public:
	cContainerList() {}
	virtual ~cContainerList() {}

	//////////////////////
	size_t Size() {
		return mvVector.size();
	}

	void Clear() {
		mvVector.clear();
	}
	//////////////////////

	void Add(T aVal) {
		mvVector.push_back(aVal);
	}

	//////////////////////

	cContainerListIterator<T> GetIterator() {
		return cContainerListIterator<T>(&mvVector);
	}

	//////////////////////

	Common::List<T> mvVector;
};

//---------------------------------

} // namespace hpl

#endif // HPL_CONTAINER_H
