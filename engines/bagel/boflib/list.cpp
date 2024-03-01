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

#include "bagel/boflib/list.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

template<class T>
inline int CBofList<T>::GetActualCount() const {
#ifndef RELEASE_BUILD
	CBofListNode<T> *pNode;
	int nCount;

	nCount = 0;
	pNode = m_pHead;
	while (pNode != nullptr) {
		nCount++;
		pNode = pNode->m_pNext;
	}

	// There should be no discrepency
	assert(m_nNumItems == nCount);
#endif

	return m_nNumItems;
}

template<class T>
inline T CBofList<T>::GetNodeItem(int nNodeIndex) {
	CBofListNode<T> *pNode = GetNode(nNodeIndex);

	assert(pNode != nullptr);

	return pNode->GetNodeItem();
}

template<class T>
inline void CBofList<T>::SetNodeItem(int nNodeIndex, T tNewItem) {
	CBofListNode<T> *pNode = GetNode(nNodeIndex);

	assert(pNode != nullptr);

	pNode->SetNodeItem(tNewItem);
}

template<class T>
inline CBofListNode<T> *CBofList<T>::GetNode(int nNodeIndex) {
	assert(nNodeIndex >= 0 && nNodeIndex < GetCount());

	CBofListNode<T> *pNode;

	if (m_pItemList == nullptr) {

		pNode = m_pHead;
		while (pNode != nullptr) {
			if (nNodeIndex-- == 0)
				break;
			pNode = pNode->m_pNext;
		}

	} else {
		pNode = (CBofListNode<T> *)(*(m_pItemList + nNodeIndex));
	}

	return pNode;
}

template<class T>
inline void CBofList<T>::InsertAfter(int nNodeIndex, T cNewItem) {
	assert(!IsEmpty());
	InsertAfter(GetNode(nNodeIndex), cNewItem);
}

template<class T>
inline void CBofList<T>::InsertAfter(CBofListNode<T> *pNode, T cNewItem) {
	assert(pNode != nullptr);
	assert(!IsEmpty());

	CBofListNode<T> *pNewNode;

	if (pNode == m_pTail) {
		AddToTail(cNewItem);
	} else {

		pNewNode = NewNode(cNewItem);
		pNewNode->m_pPrev = pNode;
		pNewNode->m_pNext = pNode->m_pNext;

		if (pNode->m_pNext != nullptr)
			pNode->m_pNext->m_pPrev = pNewNode;

		pNode->m_pNext = pNewNode;
	}

	// one more item in list
	assert(m_nNumItems != 0xFFFF);
	m_nNumItems++;

	RecalcItemList();
}

template<class T>
inline void CBofList<T>::NewItemList() {
	if (m_pItemList != nullptr) {
		BofFree(m_pItemList);
		m_pItemList = nullptr;
	}

	if (m_nNumItems != 0) {
		m_pItemList = (uint32 *)BofAlloc(m_nNumItems * sizeof(uint32));
	}
}

template<class T>
inline void CBofList<T>::KillItemList() {
	if (m_pItemList != nullptr) {
		BofFree(m_pItemList);
		m_pItemList = nullptr;
	}
}

template<class T>
inline void CBofList<T>::RecalcItemList() {
	CBofListNode<T> *pNode;
	int i;

	// we only want to recalc if we're about to overflow what we
	// have.

	if (m_nNumItems >= m_nItemsAllocated) {

		if (m_pItemList != nullptr) {
			BofFree(m_pItemList);
			m_pItemList = nullptr;
		}

		if (m_nNumItems != 0) {

			assert(m_nItemsAllocated < 0x8000);
			m_nItemsAllocated *= 2;
			if (m_nItemsAllocated == 0)
				m_nItemsAllocated = MIN_NODES;

			m_pItemList = (uint32 *)BofAlloc(m_nItemsAllocated * sizeof(uint32));
		}
	}

	if (m_nNumItems != 0) {

		assert(m_pItemList != nullptr);

		i = 0;
		pNode = m_pHead;
		while (pNode != nullptr) {
			*(m_pItemList + i++) = (uint32)pNode;
			pNode = pNode->m_pNext;
		}
	}
}

template<class T>
inline void CBofList<T>::InsertBefore(int nNodeIndex, T cNewItem) {
	assert(!IsEmpty());
	InsertBefore(GetNode(nNodeIndex), cNewItem);
}

template<class T>
inline void CBofList<T>::InsertBefore(CBofListNode<T> *pNode, T cNewItem) {
	assert(pNode != nullptr);
	assert(!IsEmpty());

	CBofListNode<T> *pNewNode;

	if (pNode == m_pHead) {
		AddToHead(cNewItem);
	} else {

		pNewNode = NewNode(cNewItem);

		pNewNode->m_pPrev = pNode->m_pPrev;
		pNewNode->m_pNext = pNode;

		if (pNode->m_pPrev != nullptr)
			pNode->m_pPrev->m_pNext = pNewNode;

		pNode->m_pPrev = pNewNode;
	}

	// one more item in list
	assert(m_nNumItems != 0xFFFF);
	m_nNumItems++;

	RecalcItemList();
}

template<class T>
inline T CBofList<T>::RemoveHead() {
	assert(m_pHead != nullptr);

	return Remove(m_pHead);
}

template<class T>
inline T CBofList<T>::RemoveTail() {
	assert(m_pTail != nullptr);
	return Remove(m_pTail);
}

template<class T>
inline T CBofList<T>::Remove(int nNodeIndex) {
	return Remove(GetNode(nNodeIndex));
}

template<class T>
inline T CBofList<T>::Remove(CBofListNode<T> *pNode) {
	assert(pNode != nullptr);

	T retVal;

	// One less item in list
	m_nNumItems--;

	assert(m_nNumItems >= 0);

	if (pNode != nullptr) {

		retVal = pNode->GetNodeItem();

		if (m_pHead == pNode)
			m_pHead = m_pHead->m_pNext;

		if (m_pTail == pNode)
			m_pTail = m_pTail->m_pPrev;

		if (pNode->m_pPrev != nullptr)
			pNode->m_pPrev->m_pNext = pNode->m_pNext;

		if (pNode->m_pNext != nullptr)
			pNode->m_pNext->m_pPrev = pNode->m_pPrev;

		delete pNode;
	}

	RecalcItemList();

	return retVal;
}

template<class T>
inline void CBofList<T>::RemoveAll() {
	int i = GetCount();

	while (i-- != 0)
		Remove(0);
}

template<class T>
inline void CBofList<T>::AddToHead(T cItem) {
	AddToHead(NewNode(cItem));
}

template<class T>
inline void CBofList<T>::AddToHead(CBofListNode<T> *pNewNode) {
	assert(pNewNode != nullptr);

	pNewNode->m_pNext = m_pHead;
	pNewNode->m_pPrev = nullptr;
	if (m_pHead != nullptr)
		m_pHead->m_pPrev = pNewNode;
	m_pHead = pNewNode;

	if (m_pTail == nullptr)
		m_pTail = m_pHead;

	// one less item in list
	assert(m_nNumItems != 0xFFFF);
	m_nNumItems++;

	RecalcItemList();
}

template<class T>
inline void CBofList<T>::AddToTail(T cItem) {
	AddToTail(NewNode(cItem));
}

template<class T>
inline void CBofList<T>::AddToTail(CBofListNode<T> *pNewNode) {
	assert(pNewNode != nullptr);

	pNewNode->m_pPrev = m_pTail;
	pNewNode->m_pNext = nullptr;
	if (m_pTail != nullptr)
		m_pTail->m_pNext = pNewNode;
	m_pTail = pNewNode;

	if (m_pHead == nullptr)
		m_pHead = m_pTail;

	// One more item in list
	assert(m_nNumItems != 0xFFFF);
	m_nNumItems++;
	RecalcItemList();
}

template<class T>
inline CBofListNode<T> *CBofList<T>::NewNode(T cItem) {
	CBofListNode<T> *pNewNode = new CBofListNode<T>(cItem);
	return pNewNode;
}

template<class T>
inline CBofListNode<T> *CBofList<T>::GetActualHead() {
	CBofListNode<T> *pNode, *pLast;

	pLast = pNode = m_pHead;
	while (pNode != nullptr) {
		pLast = pNode;
		pNode = pNode->m_pPrev;
	}

	return pLast;
}

template<class T>
inline CBofListNode<T> *CBofList<T>::GetActualTail() {
	CBofListNode<T> *pNode, *pLast;

	pLast = pNode = m_pTail;
	while (pNode != nullptr) {
		pLast = pNode;
		pNode = pLast->m_pNext;
	}

	return pLast;
}

} // namespace Bagel
