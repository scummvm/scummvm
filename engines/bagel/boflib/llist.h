
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

#ifndef BAGEL_BOFLIB_LLIST_H
#define BAGEL_BOFLIB_LLIST_H

namespace Bagel {

/**
 * Linked list class
 */
class CLList {
protected:
	CLList *_pPrev; // Pointer to next link in chain
	CLList *_pNext; // Pointer to previous link in chain
	void *_pData;   // Pointer to owning CData

public:
	/**
	 * Default constructor
	 */
	CLList();

	/**
	 * Alternate constructor that initializes with specified data
	 */
	CLList(void *pObj);

	/**
	 * Destructor
	 */
	virtual ~CLList();

	/**
	 * Links specified node to current list after current node
	 * @remarks         This will link a new list into the current right after the current node
	 * @param pNewList  Pointer to list to be inserted
	 */
	void Insert(CLList *pNewList);

	/**
	 * Deletes current node
	 */
	void Delete();

	/**
	 * Links specified node to head of list
	 * @remarks         This can be used to link 2 lists together by Linking the
	 *                  tail of one list to the head of another
	 * @param pNewList  New list to link to head of current list
	 */
	void addToHead(CLList *pNewList);

	/**
	 * Links specified node to tail of current list
	 * @remarks         This can be used to link 2 lists together by Linking the
	 *                  head of one list to the tail of another
	 * @param pNewList  Pointer to new list
	 */
	void addToTail(CLList *pNewList);

	/**
	 * Moves this item to the head of the linked list
	 */
	void MoveToHead();

	/**
	 * Moves this item to the tail of the linked list
	 */
	void MoveToTail();

	/**
	 * Moves this item 1 item to the left
	 */
	void MoveLeft();

	/**
	 * Moves this item 1 item to the right
	 */
	void MoveRight();

	/**
	 * Returns head of current list
	 * @return      Pointer to head of list
	*/
	CLList *getHead();

	/**
	 * Returns tail of current list
	 * @return      Pointer to tail of list
	*/
	CLList *getTail();
	CLList *getPrev() const {
		return _pPrev;
	}

	CLList *getNext() const {
		return _pNext;
	}

	void *getData() const {
		return _pData;
	}

	void PutData(void *pObj) {
		_pData = pObj;
	}

	/**
	 * Flushes entire list
	 */
	void FlushList();
};

} // namespace Bagel

#endif
