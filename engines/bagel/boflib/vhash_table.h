
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

#ifndef BAGEL_BOFLIB_VHASH_TABLE_H
#define BAGEL_BOFLIB_VHASH_TABLE_H

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/list.h"

namespace Bagel {

template<class T, int S>
class CBofVHashTable {
public:
	CBofVHashTable(unsigned(*hashFun)(const T &));
	virtual ~CBofVHashTable();
	BOOL contains(const T &val);
	VOID insert(const T &val);

private:
	// Default constructor is not allowed.  A declaration is provided,
	// but an implementation is not.  A link error will result if an
	// attempt is made to use this constructor.  The location which
	// is attempting to reference the default constructor should be changed
	// so that it calls the class constructor which takes a pointer to a
	// function and a table size.
	CBofVHashTable();

	// Member which holds the hash table itself.
	CBofList<T *> m_xHashTable[S];

	// Member which holds a pointer to the table's hashing
	// function.
	unsigned(*m_pHashFunction)(const T &);

	// Member which holds the count of buckets in the hash table.
	INT m_nHashTableSize;

	// Boolean which affords a light-weight test of whether the hash
	// table is empty.
	//
	BOOL m_bIsEmpty;
};

// CBofVHashTable::CBofVHashTable - class constructor.
//
template<class T, int S>
CBofVHashTable<T, S>::CBofVHashTable(unsigned(*hashFun)(const T &)) : m_nHashTableSize(S),
	m_pHashFunction(hashFun), m_bIsEmpty(TRUE) {
}

// CBofVHashTable::~CBofVHashTable - class destructor.
template<class T, int S>
CBofVHashTable<T, S>::~CBofVHashTable() {
	INT x, i;

	for (x = 0; x < m_nHashTableSize; x++) {
		CBofList<T *> *pHashBucket = &m_xHashTable[x];
		INT nListEntries = pHashBucket->GetActualCount();

		for (i = 0; i < nListEntries; i++) {
			T *pListItem = pHashBucket->GetNodeItem(i);
			delete pListItem;
			pHashBucket->SetNodeItem(i, (T *)NULL);
		}
	}
}

// CBofVHashTable<T, S>::insert - add a value to the hash table.
template<class T, int S>
VOID CBofVHashTable<T, S>::insert(const T &val) {
	T *pNodeValue = new T(val);
	CBofListNode<T *> *pNode = new CBofListNode<T *>(pNodeValue);
	Assert(pNode != NULL);

	INT nHashBucketIndex = ((*m_pHashFunction)(val)) % m_nHashTableSize;
	Assert(nHashBucketIndex < m_nHashTableSize);

	CBofList<T *> *pHashBucket = &m_xHashTable[nHashBucketIndex];
	Assert(pHashBucket != NULL);
	pHashBucket->AddToTail(pNode);
}

// CBofVHashTable<T, S>contains - predicate to test whether a value is stored in the hash table.
template<class T, int S>
BOOL CBofVHashTable<T, S>::contains(const T &val) {
	BOOL returnValue = FALSE;
	INT nHashBucketIndex = ((*m_pHashFunction)(val)) % m_nHashTableSize;
	Assert(nHashBucketIndex < m_nHashTableSize);

	CBofVHashTable<T, S> *const fakeThis = (CBofVHashTable<T, S> *const)this;
	CBofList<T *> *pHashBucket = &(fakeThis->m_xHashTable[nHashBucketIndex]);
	Assert(pHashBucket != NULL);
	int nItemsInBucket = pHashBucket->GetCount();
	for (int i = 0; i < nItemsInBucket; i++) {
		T *TableEntry = pHashBucket->GetNodeItem(i);
		if (TableEntry->CompareNoCase((const char *)val) == 0) {
			returnValue = TRUE;
			break;
		}
	}
	return returnValue;
}

} // namespace Bagel

#endif
