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
#include "BPersistMgr.h"

namespace WinterMute {

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
BOOL DCCompareElements(const TYPE *pElement1, const ARG_TYPE *pElement2) {
	return *pElement1 == *pElement2;
}

//class CBPersistMgr;

/////////////////////////////////////////////////////////////////////////////
// CBArray<TYPE, ARG_TYPE>
/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
class CBArray {
public:
// Construction
	CBArray();

// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();
	HRESULT Persist(CBPersistMgr *PersistMgr);

	// Accessing elements
	TYPE GetAt(int nIndex) const;
	void SetAt(int nIndex, ARG_TYPE newElement);
	TYPE &ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const TYPE *GetData() const;
	TYPE *GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, ARG_TYPE newElement);
	int Add(ARG_TYPE newElement);
	int Append(const CBArray &src);
	void Copy(const CBArray &src);

	// overloaded operator helpers
	TYPE operator[](int nIndex) const;
	TYPE &operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, ARG_TYPE newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CBArray *pNewArray);

// Implementation
protected:
	TYPE *m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CBArray();
};

/////////////////////////////////////////////////////////////////////////////
// CBArray<TYPE, ARG_TYPE> inline functions
/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
inline int CBArray<TYPE, ARG_TYPE>::GetSize() const {
	return m_nSize;
}
template<class TYPE, class ARG_TYPE>
inline int CBArray<TYPE, ARG_TYPE>::GetUpperBound() const {
	return m_nSize - 1;
}
template<class TYPE, class ARG_TYPE>
inline void CBArray<TYPE, ARG_TYPE>::RemoveAll() {
	SetSize(0, -1);
}
template<class TYPE, class ARG_TYPE>
inline TYPE CBArray<TYPE, ARG_TYPE>::GetAt(int nIndex) const {
	return m_pData[nIndex];
}
template<class TYPE, class ARG_TYPE>
inline void CBArray<TYPE, ARG_TYPE>::SetAt(int nIndex, ARG_TYPE newElement) {
	m_pData[nIndex] = newElement;
}
template<class TYPE, class ARG_TYPE>
inline TYPE &CBArray<TYPE, ARG_TYPE>::ElementAt(int nIndex) {
	return m_pData[nIndex];
}
template<class TYPE, class ARG_TYPE>
inline const TYPE *CBArray<TYPE, ARG_TYPE>::GetData() const {
	return (const TYPE *)m_pData;
}
template<class TYPE, class ARG_TYPE>
inline TYPE *CBArray<TYPE, ARG_TYPE>::GetData() {
	return (TYPE *)m_pData;
}
template<class TYPE, class ARG_TYPE>
inline int CBArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement) {
	int nIndex = m_nSize;
	SetAtGrow(nIndex, newElement);
	return nIndex;
}
template<class TYPE, class ARG_TYPE>
inline TYPE CBArray<TYPE, ARG_TYPE>::operator[](int nIndex) const {
	return GetAt(nIndex);
}
template<class TYPE, class ARG_TYPE>
inline TYPE &CBArray<TYPE, ARG_TYPE>::operator[](int nIndex) {
	return ElementAt(nIndex);
}

/////////////////////////////////////////////////////////////////////////////
// CBArray<TYPE, ARG_TYPE> out-of-line functions
/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
CBArray<TYPE, ARG_TYPE>::CBArray() {
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
CBArray<TYPE, ARG_TYPE>::~CBArray() {
	if (m_pData != NULL) {
		DCDestructElements<TYPE>(m_pData, m_nSize);
		delete[](byte  *)m_pData;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CBArray<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowBy) {
	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0) {
		// shrink to nothing
		if (m_pData != NULL) {
			DCDestructElements<TYPE>(m_pData, m_nSize);
			delete[](byte  *)m_pData;
			m_pData = NULL;
		}
		m_nSize = m_nMaxSize = 0;
	} else if (m_pData == NULL) {
		// create one with exact size
		m_pData = (TYPE *) new byte[nNewSize * sizeof(TYPE)];
		DCConstructElements<TYPE>(m_pData, nNewSize);
		m_nSize = m_nMaxSize = nNewSize;
	} else if (nNewSize <= m_nMaxSize) {
		// it fits
		if (nNewSize > m_nSize) {
			// initialize the new elements
			DCConstructElements<TYPE>(&m_pData[m_nSize], nNewSize - m_nSize);
		} else if (m_nSize > nNewSize) {
			// destroy the old elements
			DCDestructElements<TYPE>(&m_pData[nNewSize], m_nSize - nNewSize);
		}
		m_nSize = nNewSize;
	} else {
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0) {
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = m_nSize / 8;
			nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		TYPE *pNewData = (TYPE *) new byte[nNewMax * sizeof(TYPE)];

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));

		// construct remaining elements
		DCConstructElements<TYPE>(&pNewData[m_nSize], nNewSize - m_nSize);

		// get rid of old stuff (note: no destructors called)
		delete[](byte  *)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
int CBArray<TYPE, ARG_TYPE>::Append(const CBArray &src) {
	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);
	DCCopyElements<TYPE>(m_pData + nOldSize, src.m_pData, src.m_nSize);
	return nOldSize;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CBArray<TYPE, ARG_TYPE>::Copy(const CBArray &src) {
	SetSize(src.m_nSize);
	DCCopyElements<TYPE>(m_pData, src.m_pData, src.m_nSize);
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CBArray<TYPE, ARG_TYPE>::FreeExtra() {
	if (m_nSize != m_nMaxSize) {
		// shrink to desired size
		TYPE *pNewData = NULL;
		if (m_nSize != 0) {
			pNewData = (TYPE *) new byte[m_nSize * sizeof(TYPE)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));
		}

		// get rid of old stuff (note: no destructors called)
		delete[](byte  *)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CBArray<TYPE, ARG_TYPE>::SetAtGrow(int nIndex, ARG_TYPE newElement) {
	if (nIndex >= m_nSize)
		SetSize(nIndex + 1, -1);
	m_pData[nIndex] = newElement;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CBArray<TYPE, ARG_TYPE>::InsertAt(int nIndex, ARG_TYPE newElement, int nCount /*=1*/) {
	if (nIndex >= m_nSize) {
		// adding after the end of the array
		SetSize(nIndex + nCount, -1);   // grow so nIndex is valid
	} else {
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount, -1);  // grow it to new size
		// destroy intial data before copying over it
		DCDestructElements<TYPE>(&m_pData[nOldSize], nCount);
		// shift old data up to fill gap
		memmove(&m_pData[nIndex + nCount], &m_pData[nIndex],
		        (nOldSize - nIndex) * sizeof(TYPE));

		// re-init slots we copied from
		DCConstructElements<TYPE>(&m_pData[nIndex], nCount);
	}

	// insert new value in the gap
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CBArray<TYPE, ARG_TYPE>::RemoveAt(int nIndex, int nCount) {
	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);
	DCDestructElements<TYPE>(&m_pData[nIndex], nCount);
	if (nMoveCount)
		memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
		       nMoveCount * sizeof(TYPE));
	m_nSize -= nCount;
}

/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
void CBArray<TYPE, ARG_TYPE>::InsertAt(int nStartIndex, CBArray *pNewArray) {
	if (pNewArray->GetSize() > 0) {
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}


/////////////////////////////////////////////////////////////////////////////
template<class TYPE, class ARG_TYPE>
HRESULT CBArray<TYPE, ARG_TYPE>::Persist(CBPersistMgr *PersistMgr) {
	int i, j;
	if (PersistMgr->m_Saving) {
		j = GetSize();
		PersistMgr->Transfer("ArraySize", &j);
		for (i = 0; i < j; i++) {
			ARG_TYPE obj = GetAt(i);
			PersistMgr->Transfer("", &obj);
		}
	} else {
		SetSize(0, -1);
		PersistMgr->Transfer("ArraySize", &j);
		for (i = 0; i < j; i++) {
			ARG_TYPE obj;
			PersistMgr->Transfer("", &obj);
			Add(obj);
		}
	}
	return S_OK;
}

} // end of namespace WinterMute

#endif // COLL_TEMPL_H
