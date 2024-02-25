
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

#ifndef BAGEL_BOFLIB_BOF_LIST_H
#define BAGEL_BOFLIB_BOF_LIST_H

#include "common/scummsys.h"

namespace Bagel {


#define MIN_NODES 5 // Minimum # of pre-allocated nodes in node array

template<class T>
class CBofListNode {
protected:
	T m_cItem;				// data contained at this node

public:
	CBofListNode() { m_pNext = m_pPrev = nullptr; }
	CBofListNode(T cItem) {
		m_pNext = m_pPrev = nullptr;
		m_cItem = cItem;
	}

	T GetNodeItem() { return (m_cItem); }
	void SetNodeItem(T cItem) { m_cItem = cItem; }

	CBofListNode *m_pNext; // Next node in list
	CBofListNode *m_pPrev; // previous node in list
};

template<class T>
class CBofList {
private:
	void NewItemList();
	void KillItemList();
	void RecalcItemList();

	/**
	 * Allocates a new CBofListNode with specified data
	 * @param cItem		Data to store in new node
	 * @returns			Pointer to new node
	 */
	CBofListNode<T> *NewNode(T cItem);

	/**
	 * Calculates the actual head of this linked list
	 * @remarks		This function is used for debugging to verify that m_pHead
	 *				is still pointing to the 1st node in the list.
	 * @returns		Pointer to head of list
	 */
	CBofListNode<T> *GetActualHead();

	/**
	 * Calculates the actual tail of this linked list
	 * @remarks		This function is used for debugging to verify that m_pTail
	 *				is still pointing to the last node in the list.
	 * @returns		Pointer to tail of list
	 */
	CBofListNode<T> *GetActualTail();

protected:
	size_t m_nNumItems;
	size_t m_nItemsAllocated;
	CBofListNode<T> *m_pHead;		// pointer to head of list
	CBofListNode<T> *m_pTail;		// pointer to tail of list

	uint32 *m_pItemList;			// pointer to secondary node list

public:
	/*
	 * Constructor
	 */
	CBofList() {
		m_nNumItems = 0;
		m_nItemsAllocated = 0;
		m_pHead = m_pTail = nullptr;
		m_pItemList = nullptr;
	}

	virtual ~CBofList() {
		RemoveAll();
		KillItemList();
		Assert(m_nNumItems == 0);
	}

	int GetCount() const { return m_nNumItems; }

	/**
	 * Retrieves the number of items in this list
	 * @returns		Returns the number of linked items in this linked list.
	 */
	int GetActualCount() const;

	/**
	 * Returns true if the list is empty
	 * @return 
	*/
	bool IsEmpty() const { return m_pHead == nullptr; }

	/**
	 * Retrieves the item at the specified location
	 * @returns		Returns the item located at the node with given index.
	 * @param nNodeIndex		Index of node to retrieve
	 */
	T GetNodeItem(int nNodeIndex);


	void SetNodeItem(int nNodeIndex, T t);

	T operator[](int nIndex) { return GetNodeItem(nIndex); }

	/**
	 * Retrieves the node at the specified location
	 * @returns		Returns the node located at the given index.
	 * @param nIndex		Index of node to retrieve
	 */
	CBofListNode<T> *GetNode(int nNodeIndex);

	/**
	 * Inserts a new node as the previous node to the one specified
	 * @param nNodeIndex	Index of node to insert before
	 * @param cNewItem		Data to store at new node
	 */
	void InsertBefore(int nNodeIndex, T cNewItem);

	/**
	 * Inserts a new node as the previous node to the one specified
	 * @param pNode			Node to insert before
	 * @param cNewItem		Data to store at new node
	 */
	void InsertBefore(CBofListNode<T> *pNode, T cNewItem);

	/**
	 * Inserts a new node as the next node to the one specified
	 * @param nNodeIndex		Index of node to insert after
	 * @param cNewItem			Data to store at new node
	 */
	void InsertAfter(int nNodeIndex, T cNewItem);

	/**
	 * Inserts a new node as the next node to the one specified
	 * @param pNode				Node to insert after
	 * @param cNewItem			Data to store at new node
	 */
	void InsertAfter(CBofListNode<T> *pNode, T cNewItem);

	/**
	 * Removes specfied node from the list
	 * @param pNode				Node to remove
	 * @returns					Item stored at specified location
	 */
	T Remove(CBofListNode<T> *pNode);

	/**
	 * Removes specfied node (by index) from the list
	 * @param nNodeIndex		Index of node to remove
	 * @returns					Item stored at specified location
	 */
	T Remove(int nNodeIndex);

	/**
	 * Removes all nodes from this list
	 * @remarks		Deletes all memory used by the nodes in this list
	 */
	void RemoveAll();

	/**
	 * Removes specfied node (by index) from the list
	 * @returns		Item stored at specified location
	 */
	T RemoveHead();

	/**
	 * Removes specfied node (by index) from the list
	 * @returns		Item stored at specified location
	 */
	T RemoveTail();

	/**
	 * Adds specified node as the new head of this list
	 * @param pNode		Pointer to node to add to the list
	 */
	void AddToHead(CBofListNode<T> *pNewNode);

	/**
	 * Adds specified item as the new head of this list
	 * @param cItem		Item to add to the list
	 */
	void AddToHead(T cItem);

	/**
	 * Adds specified node as the new tail of this list
	 * @param pNode		Pointer to node to add to the list
	 */
	void AddToTail(CBofListNode<T> *pNewNode);

	/**
	 * Adds specified item as the new tail of this list
	 * @param cItem		Item to add to the list
	 */
	void AddToTail(T cItem);

	CBofListNode<T> *GetHead() const { return m_pHead; }
	CBofListNode<T> *GetTail() const { return m_pTail; }
};

} // namespace Bagel

#endif
