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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_COLL_TEMPL_H
#define WINTERMUTE_COLL_TEMPL_H

#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

#include <new>

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void dcConstructElements(TYPE *pElements, int32 nCount) {
	// first do bit-wise zero initialization
	memset((void *)pElements, 0, nCount * sizeof(TYPE));

	// then call the constructor(s)
	for (; nCount--; pElements++)
		::new((void *)pElements) TYPE;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void dcDestructElements(TYPE *pElements, int32 nCount) {
	// call the destructor(s)
	for (; nCount--; pElements++)
		pElements->~TYPE();
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void dcCopyElements(TYPE *pDest, const TYPE *pSrc, int32 nCount) {
	// default is element-copy using assignment
	while (nCount--)
		*pDest++ = *pSrc++;
}

/////////////////////////////////////////////////////////////////////////////
// BaseArray<TYPE>
/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
class BaseArrayBase {
public:
	// Construction
	BaseArrayBase();

	// Attributes
	int32 getSize() const;
	int32 getUpperBound() const;
	void setSize(int32 nNewSize, int32 nGrowBy = -1);

	// Operations
	// Clean up
	void freeExtra();
	void removeAll();

	// Accessing elements
	TYPE getAt(int32 nIndex) const;
	void setAt(int32 nIndex, TYPE newElement);
	TYPE& elementAt(int32 nIndex);

	// Direct Access to the element data (may return NULL)
	const TYPE *getData() const;
	TYPE *getData();

	// Potentially growing the array
	void setAtGrow(int32 nIndex, TYPE newElement);
	int32 add(TYPE newElement);
	int32 append(const BaseArrayBase &src);
	void copy(const BaseArrayBase &src);

	// overloaded operator helpers
	TYPE operator[](int32 nIndex) const;
	TYPE &operator[](int32 nIndex);

	// Operations that move elements around
	void insertAt(int32 nIndex, TYPE newElement, int32 nCount = 1);
	void removeAt(int32 nIndex, int32 nCount = 1);
	void insertAt(int32 nStartIndex, BaseArrayBase *pNewArray);

	// Implementation
protected:
	TYPE *_pData;   // the actual array of data
	int32 _nSize;     // # of elements (upperBound - 1)
	int32 _nMaxSize;  // max allocated
	int32 _nGrowBy;   // grow amount

public:
	~BaseArrayBase();
};

/////////////////////////////////////////////////////////////////////////////
// CBArray<TYPE> inline functions
/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline int32 BaseArrayBase<TYPE>::getSize() const {
	return _nSize;
}

template<class TYPE>
inline int32 BaseArrayBase<TYPE>::getUpperBound() const {
	return _nSize - 1;
}

template<class TYPE>
inline void BaseArrayBase<TYPE>::removeAll() {
	setSize(0, -1);
}

template<class TYPE>
inline TYPE BaseArrayBase<TYPE>::getAt(int32 nIndex) const {
	return _pData[nIndex];
}

template<class TYPE>
inline void BaseArrayBase<TYPE>::setAt(int32 nIndex, TYPE newElement) {
	_pData[nIndex] = newElement;
}

template<class TYPE>
inline TYPE &BaseArrayBase<TYPE>::elementAt(int32 nIndex) {
	return _pData[nIndex];
}

template<class TYPE>
inline const TYPE *BaseArrayBase<TYPE>::getData() const {
	return (const TYPE *)_pData;
}

template<class TYPE>
inline TYPE *BaseArrayBase<TYPE>::getData() {
	return (TYPE *)_pData;
}

template<class TYPE>
inline int32 BaseArrayBase<TYPE>::add(TYPE newElement) {
	int32 nIndex = _nSize;
	setAtGrow(nIndex, newElement);
	return nIndex;
}

template<class TYPE>
inline TYPE BaseArrayBase<TYPE>::operator[](int32 nIndex) const {
	return getAt(nIndex);
}

template<class TYPE>
inline TYPE &BaseArrayBase<TYPE>::operator[](int32 nIndex) {
	return elementAt(nIndex);
}

/////////////////////////////////////////////////////////////////////////////
// BaseArray<TYPE, ARG_TYPE> out-of-line functions
/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
BaseArrayBase<TYPE>::BaseArrayBase() {
	_pData = nullptr;
	_nSize = _nMaxSize = _nGrowBy = 0;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
BaseArrayBase<TYPE>::~BaseArrayBase() {
	if (_pData != nullptr) {
		dcDestructElements<TYPE>(_pData, _nSize);
		delete[] (byte *)_pData;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
void BaseArrayBase<TYPE>::setSize(int32 nNewSize, int32 nGrowBy) {
	if (nGrowBy != -1)
		_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0) {
		// shrink to nothing
		if (_pData != nullptr) {
			dcDestructElements<TYPE>(_pData, _nSize);
			delete[] (byte *)_pData;
			_pData = nullptr;
		}
		_nSize = _nMaxSize = 0;
	} else if (_pData == nullptr) {
		// create one with exact size
		_pData = (TYPE *) new byte[nNewSize * sizeof(TYPE)];
		dcConstructElements<TYPE>(_pData, nNewSize);
		_nSize = _nMaxSize = nNewSize;
	} else if (nNewSize <= _nMaxSize) {
		// it fits
		if (nNewSize > _nSize) {
			// initialize the new elements
			dcConstructElements<TYPE>(&_pData[_nSize], nNewSize - _nSize);
		} else if (_nSize > nNewSize) {
			// destroy the old elements
			dcDestructElements<TYPE>(&_pData[nNewSize], _nSize - nNewSize);
		}
		_nSize = nNewSize;
	} else {
		// otherwise, grow array
		int32 numGrowBy = _nGrowBy;
		if (numGrowBy == 0) {
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			numGrowBy = _nSize / 8;
			numGrowBy = (numGrowBy < 4) ? 4 : ((numGrowBy > 1024) ? 1024 : numGrowBy);
		}
		int nNewMax;
		if (nNewSize < _nMaxSize + numGrowBy)
			nNewMax = _nMaxSize + numGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		TYPE *pNewData = (TYPE *) new byte[nNewMax * sizeof(TYPE)];

		// copy new data from old
		memcpy(pNewData, _pData, _nSize * sizeof(TYPE));

		// construct remaining elements
		dcConstructElements<TYPE>(&pNewData[_nSize], nNewSize - _nSize);

		// get rid of old stuff (note: no destructors called)
		delete[] (byte *)_pData;
		_pData = pNewData;
		_nSize = nNewSize;
		_nMaxSize = nNewMax;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
int32 BaseArrayBase<TYPE>::append(const BaseArrayBase &src) {
	int32 nOldSize = _nSize;
	setSize(_nSize + src._nSize);
	dcCopyElements<TYPE>(_pData + nOldSize, src._pData, src._nSize);
	return nOldSize;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
void BaseArrayBase<TYPE>::copy(const BaseArrayBase &src) {
	setSize(src._nSize);
	dcCopyElements<TYPE>(_pData, src._pData, src._nSize);
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
void BaseArrayBase<TYPE>::freeExtra() {
	if (_nSize != _nMaxSize) {
		// shrink to desired size
		TYPE *pNewData = nullptr;
		if (_nSize != 0) {
			pNewData = (TYPE *) new byte[_nSize * sizeof(TYPE)];
			// copy new data from old
			memcpy(pNewData, _pData, _nSize * sizeof(TYPE));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] (byte *)_pData;
		_pData = pNewData;
		_nMaxSize = _nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
void BaseArrayBase<TYPE>::setAtGrow(int32 nIndex, TYPE newElement) {
	if (nIndex >= _nSize)
		setSize(nIndex + 1, -1);
	_pData[nIndex] = newElement;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
void BaseArrayBase<TYPE>::insertAt(int32 nIndex, TYPE newElement, int32 nCount /*=1*/) {
	if (nIndex >= _nSize) {
		// adding after the end of the array
		setSize(nIndex + nCount, -1);   // grow so nIndex is valid
	} else {
		// inserting in the middle of the array
		int32 nOldSize = _nSize;
		setSize(_nSize + nCount, -1);  // grow it to new size
		// destroy intial data before copying over it
		dcDestructElements<TYPE>(&_pData[nOldSize], nCount);
		// shift old data up to fill gap
		memmove(&_pData[nIndex + nCount], &_pData[nIndex],
				(nOldSize - nIndex) * sizeof(TYPE));

		// re-init slots we copied from
		dcConstructElements<TYPE>(&_pData[nIndex], nCount);
	}

	// insert new value in the gap
	while (nCount--)
		_pData[nIndex++] = newElement;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
void BaseArrayBase<TYPE>::removeAt(int32 nIndex, int32 nCount) {
	// just remove a range
	int32 nMoveCount = _nSize - (nIndex + nCount);
	dcDestructElements<TYPE>(&_pData[nIndex], nCount);
	if (nMoveCount)
		memmove(&_pData[nIndex], &_pData[nIndex + nCount], nMoveCount * sizeof(TYPE));
	_nSize -= nCount;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
void BaseArrayBase<TYPE>::insertAt(int32 nStartIndex, BaseArrayBase *pNewArray) {
	if (pNewArray->getSize() > 0) {
		insertAt(nStartIndex, pNewArray->getAt(0), pNewArray->getSize());
		for (int32 i = 0; i < pNewArray->getSize(); i++)
			setAt(nStartIndex + i, pNewArray->getAt(i));
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
class BaseArray : public BaseArrayBase<TYPE> {
public:
	bool persist(BasePersistenceManager *persistMgr) {
		int32 i, j;
		if (persistMgr->getIsSaving()) {
			j = BaseArray::getSize();
			persistMgr->transferSint32("ArraySize", &j);
			for (i = 0; i < j; i++) {
				TYPE obj = BaseArray::getAt(i);
				persistMgr->transferPtr("", &obj);
			}
		} else {
			BaseArray::setSize(0, -1);
			persistMgr->transferSint32("ArraySize", &j);
			for (i = 0; i < j; i++) {
				TYPE obj = nullptr;
				persistMgr->transferPtr("", &obj);
				BaseArray::add(obj);
			}
		}
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////
template <>
class BaseArray<char *> : public BaseArrayBase<char *> {
public:
	bool persist(BasePersistenceManager *persistMgr) {
		int32 i, j;
		if (persistMgr->getIsSaving()) {
			j = getSize();
			persistMgr->transferSint32("ArraySize", &j);
			for (i = 0; i < j; i++) {
				char *obj = getAt(i);
				persistMgr->transferCharPtr("", &obj);
			}
		} else {
			setSize(0, -1);
			persistMgr->transferSint32("ArraySize", &j);
			for (i = 0; i < j; i++) {
				char *obj = nullptr;
				persistMgr->transferCharPtr("", &obj);
				add(obj);
			}
		}
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////
template <>
class BaseArray<const char *> : public BaseArrayBase<const char *> {
public:
	bool persist(BasePersistenceManager *persistMgr) {
		int32 i, j;
		if (persistMgr->getIsSaving()) {
			j = getSize();
			persistMgr->transferSint32("ArraySize", &j);
			for (i = 0; i < j; i++) {
				const char * obj = getAt(i);
				persistMgr->transferConstChar("", &obj);
			}
		} else {
			setSize(0, -1);
			persistMgr->transferSint32("ArraySize", &j);
			for (i = 0; i < j; i++) {
				const char * obj;
				persistMgr->transferConstChar("", &obj);
				add(obj);
			}
		}
		return true;
	}
};

} // End of namespace Wintermute

#endif
