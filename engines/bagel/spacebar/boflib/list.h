
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

#ifndef BAGEL_BOFLIB_LIST_H
#define BAGEL_BOFLIB_LIST_H

#include "common/scummsys.h"
#include "bagel/boflib/misc.h"

namespace Bagel {
namespace SpaceBar {

#define MIN_NODES 5 // Minimum # of pre-allocated nodes in node array

template<class T>
class CBofListNode {
protected:
	T _cItem;              // Data contained at this node

public:
	CBofListNode() {
		_pNext = _pPrev = nullptr;
	}
	CBofListNode(T cItem) {
		_pNext = _pPrev = nullptr;
		_cItem = cItem;
	}

	T getNodeItem() {
		return _cItem;
	}
	void setNodeItem(T cItem) {
		_cItem = cItem;
	}

	CBofListNode *_pNext; // Next node in list
	CBofListNode *_pPrev; // Previous node in list
};

template<class T>
class CBofList {
private:
	void newItemList() {
		if (_pItemList != nullptr) {
			bofFree(_pItemList);
			_pItemList = nullptr;
		}

		if (_nNumItems != 0) {
			_pItemList = (void **)bofAlloc(_nNumItems * sizeof(void *));
		}
	}

	void killItemList() {
		if (_pItemList != nullptr) {
			bofFree(_pItemList);
			_pItemList = nullptr;
		}
	}

	void recalcItemList() {
		// We only want to recalc if we're about to overflow what we have
		if (_nNumItems >= _nItemsAllocated) {
			if (_pItemList != nullptr) {
				bofFree(_pItemList);
				_pItemList = nullptr;
			}

			if (_nNumItems != 0) {
				assert(_nItemsAllocated < 0x8000);
				_nItemsAllocated *= 2;
				if (_nItemsAllocated == 0)
					_nItemsAllocated = MIN_NODES;

				_pItemList = (void **)bofAlloc(_nItemsAllocated * sizeof(void *));
			}
		}

		if (_nNumItems != 0) {
			assert(_pItemList != nullptr);

			int i = 0;
			CBofListNode<T> *pNode = _pHead;

			while (pNode != nullptr) {
				*(_pItemList + i++) = pNode;
				pNode = pNode->_pNext;
			}
		}
	}

	/**
	 * Allocates a new CBofListNode with specified data
	 * @param cItem     Data to store in new node
	 * @returns         Pointer to new node
	 */
	CBofListNode<T> *newNode(T cItem) {
		CBofListNode<T> *pNewNode = new CBofListNode<T>(cItem);
		return pNewNode;
	}

	/**
	 * Calculates the actual head of this linked list
	 * @remarks     This function is used for debugging to verify that _pHead
	 *              is still pointing to the 1st node in the list.
	 * @returns     Pointer to head of list
	 */
	CBofListNode<T> *getActualHead() {
		CBofListNode<T> *pNode;
		CBofListNode<T> *pLast = pNode = _pHead;

		while (pNode != nullptr) {
			pLast = pNode;
			pNode = pNode->_pPrev;
		}

		return pLast;
	}

	/**
	 * Calculates the actual tail of this linked list
	 * @remarks     This function is used for debugging to verify that _pTail
	 *              is still pointing to the last node in the list.
	 * @returns     Pointer to tail of list
	 */
	CBofListNode<T> *getActualTail() {
		CBofListNode<T> *pNode;
		CBofListNode<T> *pLast = pNode = _pTail;

		while (pNode != nullptr) {
			pLast = pNode;
			pNode = pLast->_pNext;
		}

		return pLast;
	}

protected:
	uint32 _nNumItems;
	uint32 _nItemsAllocated;
	CBofListNode<T> *_pHead;       // pointer to head of list
	CBofListNode<T> *_pTail;       // pointer to tail of list

	void **_pItemList;         // pointer to secondary node list

public:
	/*
	 * Constructor
	 */
	CBofList() {
		_nNumItems = 0;
		_nItemsAllocated = 0;
		_pHead = _pTail = nullptr;
		_pItemList = nullptr;
	}

	/**
	 * Destructor
	 */
	virtual ~CBofList() {
		removeAll();
		killItemList();
		assert(_nNumItems == 0);
	}

	int getCount() const {
		return _nNumItems;
	}

	/**
	 * Retrieves the number of items in this list
	 * @returns     Returns the number of linked items in this linked list.
	 */
	int getActualCount() const {
		uint32 nCount = 0;
		CBofListNode<T> *pNode = _pHead;
		while (pNode != nullptr) {
			nCount++;
			pNode = pNode->_pNext;
		}

		// There should be no discrepancy
		assert(_nNumItems == nCount);

		return _nNumItems;
	}

	/**
	 * Returns true if the list is empty
	 * @return
	*/
	bool isEmpty() const {
		return _pHead == nullptr;
	}

	/**
	 * Retrieves the item at the specified location
	 * @returns     Returns the item located at the node with given index.
	 * @param nNodeIndex        Index of node to retrieve
	 */
	inline T getNodeItem(int nNodeIndex) {
		CBofListNode<T> *pNode = getNode(nNodeIndex);

		assert(pNode != nullptr);

		return pNode->getNodeItem();
	}

	void setNodeItem(int nNodeIndex, T tNewItem) {
		CBofListNode<T> *pNode = getNode(nNodeIndex);

		assert(pNode != nullptr);

		pNode->setNodeItem(tNewItem);
	}

	T operator[](int nIndex) {
		return getNodeItem(nIndex);
	}

	/**
	 * Retrieves the node at the specified location
	 * @returns     Returns the node located at the given index.
	 * @param nNodeIndex        Index of node to retrieve
	 */
	CBofListNode<T> *getNode(int nNodeIndex) {
		assert(nNodeIndex >= 0 && nNodeIndex < getCount());

		CBofListNode<T> *pNode;

		if (_pItemList == nullptr) {

			pNode = _pHead;
			while (pNode != nullptr) {
				if (nNodeIndex-- == 0)
					break;
				pNode = pNode->_pNext;
			}

		} else {
			pNode = (CBofListNode<T> *)(*(_pItemList + nNodeIndex));
		}

		return pNode;
	}

	/**
	 * Inserts a new node as the previous node to the one specified
	 * @param nNodeIndex    Index of node to insert before
	 * @param cNewItem      Data to store at new node
	 */
	void insertBefore(int nNodeIndex, T cNewItem) {
		assert(!isEmpty());
		insertBefore(getNode(nNodeIndex), cNewItem);
	}

	/**
	 * Inserts a new node as the previous node to the one specified
	 * @param pNode         Node to insert before
	 * @param cNewItem      Data to store at new node
	 */
	void insertBefore(CBofListNode<T> *pNode, T cNewItem) {
		assert(pNode != nullptr);
		assert(!isEmpty());

		if (pNode == _pHead) {
			addToHead(cNewItem);
		} else {

			CBofListNode<T> *pNewNode = newNode(cNewItem);

			pNewNode->_pPrev = pNode->_pPrev;
			pNewNode->_pNext = pNode;

			if (pNode->_pPrev != nullptr)
				pNode->_pPrev->_pNext = pNewNode;

			pNode->_pPrev = pNewNode;
		}

		// one more item in list
		assert(_nNumItems != 0xFFFF);
		_nNumItems++;

		recalcItemList();
	}

	/**
	 * Inserts a new node as the next node to the one specified
	 * @param nNodeIndex        Index of node to insert after
	 * @param cNewItem          Data to store at new node
	 */
	void insertAfter(int nNodeIndex, T cNewItem) {
		assert(!isEmpty());
		insertAfter(getNode(nNodeIndex), cNewItem);
	}

	/**
	 * Inserts a new node as the next node to the one specified
	 * @param pNode             Node to insert after
	 * @param cNewItem          Data to store at new node
	 */
	void insertAfter(CBofListNode<T> *pNode, T cNewItem) {
		assert(pNode != nullptr);
		assert(!isEmpty());

		if (pNode == _pTail) {
			addToTail(cNewItem);
		} else {

			CBofListNode<T> *pNewNode = newNode(cNewItem);
			pNewNode->_pPrev = pNode;
			pNewNode->_pNext = pNode->_pNext;

			if (pNode->_pNext != nullptr)
				pNode->_pNext->_pPrev = pNewNode;

			pNode->_pNext = pNewNode;
		}

		// one more item in list
		assert(_nNumItems != 0xFFFF);
		_nNumItems++;

		recalcItemList();
	}

	/**
	 * Removes specified node from the list
	 * @param pNode             Node to remove
	 * @returns                 Item stored at specified location
	 */
	T remove(CBofListNode<T> *pNode) {
		assert(pNode != nullptr);

		// One less item in list
		_nNumItems--;

		//assert(_nNumItems >= 0);

		if (pNode != nullptr) {

			T retVal = pNode->getNodeItem();

			if (_pHead == pNode)
				_pHead = _pHead->_pNext;

			if (_pTail == pNode)
				_pTail = _pTail->_pPrev;

			if (pNode->_pPrev != nullptr)
				pNode->_pPrev->_pNext = pNode->_pNext;

			if (pNode->_pNext != nullptr)
				pNode->_pNext->_pPrev = pNode->_pPrev;

			delete pNode;

			recalcItemList();
			return retVal;
		} else {
			return T();
		}
	}

	/**
	 * Removes specified node (by index) from the list
	 * @param nNodeIndex        Index of node to remove
	 * @returns                 Item stored at specified location
	 */
	T remove(int nNodeIndex) {
		return remove(getNode(nNodeIndex));
	}

	/**
	 * Removes all nodes from this list
	 * @remarks     Deletes all memory used by the nodes in this list
	 */
	void removeAll() {
		int i = getCount();

		while (i-- != 0)
			remove(0);
	}

	/**
	 * Removes specified node (by index) from the list
	 * @returns     Item stored at specified location
	 */
	inline T removeHead() {
		assert(_pHead != nullptr);

		return remove(_pHead);
	}

	/**
	 * Removes specified node (by index) from the list
	 * @returns     Item stored at specified location
	 */
	inline T removeTail() {
		assert(_pTail != nullptr);
		return remove(_pTail);
	}

	/**
	 * Adds specified node as the new head of this list
	 * @param pNewNode     Pointer to node to add to the list
	 */
	inline void addToHead(CBofListNode<T> *pNewNode) {
		assert(pNewNode != nullptr);

		pNewNode->_pNext = _pHead;
		pNewNode->_pPrev = nullptr;
		if (_pHead != nullptr)
			_pHead->_pPrev = pNewNode;
		_pHead = pNewNode;

		if (_pTail == nullptr)
			_pTail = _pHead;

		// one less item in list
		assert(_nNumItems != 0xFFFF);
		_nNumItems++;

		recalcItemList();
	}

	/**
	 * Adds specified item as the new head of this list
	 * @param cItem     Item to add to the list
	 */
	inline void addToHead(T cItem) {
		addToHead(newNode(cItem));
	}

	/**
	 * Adds specified node as the new tail of this list
	 * @param pNewNode     Pointer to node to add to the list
	 */
	void addToTail(CBofListNode<T> *pNewNode) {
		assert(pNewNode != nullptr);

		pNewNode->_pPrev = _pTail;
		pNewNode->_pNext = nullptr;
		if (_pTail != nullptr)
			_pTail->_pNext = pNewNode;
		_pTail = pNewNode;

		if (_pHead == nullptr)
			_pHead = _pTail;

		// One more item in list
		assert(_nNumItems != 0xFFFF);
		_nNumItems++;
		recalcItemList();
	}

	/**
	 * Adds specified item as the new tail of this list
	 * @param cItem     Item to add to the list
	 */
	void addToTail(T cItem) {
		addToTail(newNode(cItem));
	}

	CBofListNode<T> *getHead() const {
		return _pHead;
	}
	CBofListNode<T> *getTail() const {
		return _pTail;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
