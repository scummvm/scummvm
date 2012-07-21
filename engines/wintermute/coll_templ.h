/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_COLL_TEMPL_H
#define WINTERMUTE_COLL_TEMPL_H


#include <new>
#include "engines/wintermute/base/base_persistence_manager.h"

namespace WinterMute {

// Quite the same as Common::Array, but with persistence-support.

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void DCConstructElements(TYPE *pElements, int nCount) {
	// first do bit-wise zero initialization
	memset((void *)pElements, 0, nCount * sizeof(TYPE));

	// then call the constructor(s)
	for (; nCount--; pElements++)
		::new((void *)pElements) TYPE;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void DCDestructElements(TYPE *pElements, int nCount) {
	// call the destructor(s)
	for (; nCount--; pElements++)
		pElements->~TYPE();
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE>
inline void DCCopyElements(TYPE *pDest, const TYPE *pSrc, int nCount) {
	// default is element-copy using assignment
	while (nCount--)
		*pDest++ = *pSrc++;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
bool DCCompareElements(const TYPE *pElement1, const ARG_TYPE *pElement2) {
	return *pElement1 == *pElement2;
}

//class BasePersistenceManager;

/////////////////////////////////////////////////////////////////////////////
// BaseArray<TYPE, ARG_TYPE>
/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
class BaseArray {
public:
// Construction
	BaseArray();

// Attributes
	int getSize() const;
	int getUpperBound() const;
	void setSize(int nNewSize, int nGrowBy = -1);

// Operations
	// Clean up
	void freeExtra();
	void removeAll();
	bool persist(BasePersistenceManager *persistMgr);

	// Accessing elements
	TYPE getAt(int nIndex) const;
	void setAt(int nIndex, ARG_TYPE newElement);
	TYPE &elementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const TYPE *getData() const;
	TYPE *getData();

	// Potentially growing the array
	void setAtGrow(int nIndex, ARG_TYPE newElement);
	int add(ARG_TYPE newElement);
	int append(const BaseArray &src);
	void copy(const BaseArray &src);

	// overloaded operator helpers
	TYPE operator[](int nIndex) const;
	TYPE &operator[](int nIndex);

	// Operations that move elements around
	void insertAt(int nIndex, ARG_TYPE newElement, int nCount = 1);
	void removeAt(int nIndex, int nCount = 1);
	void insertAt(int nStartIndex, BaseArray *pNewArray);

// Implementation
protected:
	TYPE *_pData;   // the actual array of data
	int _nSize;     // # of elements (upperBound - 1)
	int _nMaxSize;  // max allocated
	int _nGrowBy;   // grow amount

public:
	~BaseArray();
};

/////////////////////////////////////////////////////////////////////////////
// BaseArray<TYPE, ARG_TYPE> inline functions
/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
inline int BaseArray<TYPE, ARG_TYPE>::getSize() const {
	return _nSize;
}
template<class TYPE, class ARG_TYPE>
inline int BaseArray<TYPE, ARG_TYPE>::getUpperBound() const {
	return _nSize - 1;
}
template<class TYPE, class ARG_TYPE>
inline void BaseArray<TYPE, ARG_TYPE>::removeAll() {
	setSize(0, -1);
}
template<class TYPE, class ARG_TYPE>
inline TYPE BaseArray<TYPE, ARG_TYPE>::getAt(int nIndex) const {
	return _pData[nIndex];
}
template<class TYPE, class ARG_TYPE>
inline void BaseArray<TYPE, ARG_TYPE>::setAt(int nIndex, ARG_TYPE newElement) {
	_pData[nIndex] = newElement;
}
template<class TYPE, class ARG_TYPE>
inline TYPE &BaseArray<TYPE, ARG_TYPE>::elementAt(int nIndex) {
	return _pData[nIndex];
}
template<class TYPE, class ARG_TYPE>
inline const TYPE *BaseArray<TYPE, ARG_TYPE>::getData() const {
	return (const TYPE *)_pData;
}
template<class TYPE, class ARG_TYPE>
inline TYPE *BaseArray<TYPE, ARG_TYPE>::getData() {
	return (TYPE *)_pData;
}
template<class TYPE, class ARG_TYPE>
inline int BaseArray<TYPE, ARG_TYPE>::add(ARG_TYPE newElement) {
	int nIndex = _nSize;
	setAtGrow(nIndex, newElement);
	return nIndex;
}
template<class TYPE, class ARG_TYPE>
inline TYPE BaseArray<TYPE, ARG_TYPE>::operator[](int nIndex) const {
	return getAt(nIndex);
}
template<class TYPE, class ARG_TYPE>
inline TYPE &BaseArray<TYPE, ARG_TYPE>::operator[](int nIndex) {
	return elementAt(nIndex);
}

/////////////////////////////////////////////////////////////////////////////
// BaseArray<TYPE, ARG_TYPE> out-of-line functions
/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
BaseArray<TYPE, ARG_TYPE>::BaseArray() {
	_pData = NULL;
	_nSize = _nMaxSize = _nGrowBy = 0;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
BaseArray<TYPE, ARG_TYPE>::~BaseArray() {
	if (_pData != NULL) {
		DCDestructElements<TYPE>(_pData, _nSize);
		delete[](byte *)_pData;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void BaseArray<TYPE, ARG_TYPE>::setSize(int nNewSize, int nGrowBy) {
	if (nGrowBy != -1)
		_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0) {
		// shrink to nothing
		if (_pData != NULL) {
			DCDestructElements<TYPE>(_pData, _nSize);
			delete[](byte *)_pData;
			_pData = NULL;
		}
		_nSize = _nMaxSize = 0;
	} else if (_pData == NULL) {
		// create one with exact size
		_pData = (TYPE *) new byte[nNewSize * sizeof(TYPE)];
		DCConstructElements<TYPE>(_pData, nNewSize);
		_nSize = _nMaxSize = nNewSize;
	} else if (nNewSize <= _nMaxSize) {
		// it fits
		if (nNewSize > _nSize) {
			// initialize the new elements
			DCConstructElements<TYPE>(&_pData[_nSize], nNewSize - _nSize);
		} else if (_nSize > nNewSize) {
			// destroy the old elements
			DCDestructElements<TYPE>(&_pData[nNewSize], _nSize - nNewSize);
		}
		_nSize = nNewSize;
	} else {
		// otherwise, grow array
		nGrowBy = _nGrowBy;
		if (nGrowBy == 0) {
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = _nSize / 8;
			nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
		}
		int nNewMax;
		if (nNewSize < _nMaxSize + nGrowBy)
			nNewMax = _nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		TYPE *pNewData = (TYPE *) new byte[nNewMax * sizeof(TYPE)];

		// copy new data from old
		memcpy(pNewData, _pData, _nSize * sizeof(TYPE));

		// construct remaining elements
		DCConstructElements<TYPE>(&pNewData[_nSize], nNewSize - _nSize);

		// get rid of old stuff (note: no destructors called)
		delete[](byte *)_pData;
		_pData = pNewData;
		_nSize = nNewSize;
		_nMaxSize = nNewMax;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
int BaseArray<TYPE, ARG_TYPE>::append(const BaseArray &src) {
	int nOldSize = _nSize;
	setSize(_nSize + src._nSize);
	DCCopyElements<TYPE>(_pData + nOldSize, src._pData, src._nSize);
	return nOldSize;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void BaseArray<TYPE, ARG_TYPE>::copy(const BaseArray &src) {
	setSize(src._nSize);
	DCCopyElements<TYPE>(_pData, src._pData, src._nSize);
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void BaseArray<TYPE, ARG_TYPE>::freeExtra() {
	if (_nSize != _nMaxSize) {
		// shrink to desired size
		TYPE *pNewData = NULL;
		if (_nSize != 0) {
			pNewData = (TYPE *) new byte[_nSize * sizeof(TYPE)];
			// copy new data from old
			memcpy(pNewData, _pData, _nSize * sizeof(TYPE));
		}

		// get rid of old stuff (note: no destructors called)
		delete[](byte *)_pData;
		_pData = pNewData;
		_nMaxSize = _nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void BaseArray<TYPE, ARG_TYPE>::setAtGrow(int nIndex, ARG_TYPE newElement) {
	if (nIndex >= _nSize)
		setSize(nIndex + 1, -1);
	_pData[nIndex] = newElement;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void BaseArray<TYPE, ARG_TYPE>::insertAt(int nIndex, ARG_TYPE newElement, int nCount /*=1*/) {
	if (nIndex >= _nSize) {
		// adding after the end of the array
		setSize(nIndex + nCount, -1);   // grow so nIndex is valid
	} else {
		// inserting in the middle of the array
		int nOldSize = _nSize;
		setSize(_nSize + nCount, -1);  // grow it to new size
		// destroy intial data before copying over it
		DCDestructElements<TYPE>(&_pData[nOldSize], nCount);
		// shift old data up to fill gap
		memmove(&_pData[nIndex + nCount], &_pData[nIndex],
		        (nOldSize - nIndex) * sizeof(TYPE));

		// re-init slots we copied from
		DCConstructElements<TYPE>(&_pData[nIndex], nCount);
	}

	// insert new value in the gap
	while (nCount--)
		_pData[nIndex++] = newElement;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void BaseArray<TYPE, ARG_TYPE>::removeAt(int nIndex, int nCount) {
	// just remove a range
	int nMoveCount = _nSize - (nIndex + nCount);
	DCDestructElements<TYPE>(&_pData[nIndex], nCount);
	if (nMoveCount)
		memcpy(&_pData[nIndex], &_pData[nIndex + nCount],
		       nMoveCount * sizeof(TYPE));
	_nSize -= nCount;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void BaseArray<TYPE, ARG_TYPE>::insertAt(int nStartIndex, BaseArray *pNewArray) {
	if (pNewArray->GetSize() > 0) {
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			setAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}


/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
bool BaseArray<TYPE, ARG_TYPE>::persist(BasePersistenceManager *persistMgr) {
	int i, j;
	if (persistMgr->_saving) {
		j = getSize();
		persistMgr->transfer("ArraySize", &j);
		for (i = 0; i < j; i++) {
			ARG_TYPE obj = getAt(i);
			persistMgr->transfer("", &obj);
		}
	} else {
		setSize(0, -1);
		persistMgr->transfer("ArraySize", &j);
		for (i = 0; i < j; i++) {
			ARG_TYPE obj;
			persistMgr->transfer("", &obj);
			add(obj);
		}
	}
	return STATUS_OK;
}

} // end of namespace WinterMute

#endif // COLL_TEMPL_H
