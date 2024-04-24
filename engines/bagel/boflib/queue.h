
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

#ifndef BAGEL_BOFLIB_QUEUE_H
#define BAGEL_BOFLIB_QUEUE_H

#include "bagel/boflib/object.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

class CQueue : public CBofObject {
protected:
	CLList *m_pQueueList = nullptr;

public:
	/**
	 * Default constructor
	 */
	CQueue();

	/**
	 * Constructor that adds a passed object onto the queue
	 * @param pObject       First object
	 */
	CQueue(void *pObject);

	/**
	 * Constructor that copies an existing queue
	 * @param pQueue        Source queue
	 */
	CQueue(CQueue *pQueue);

	/**
	 * Constructor that copies an existing queue
	 * @param cQueue        Source queue
	*/
	CQueue(const CQueue &cQueue);

	/**
	 * Destructor
	*/
	virtual ~CQueue();

	/**
	 * Pushes object onto stack
	 * @param pObject       Pointer to object to be pushed
	 */
	void AddItem(void *pObject);

	/**
	 * Removes 1 object off stack
	 * @return      Pointer to top object on stack
	 */
	void *RemoveItem();

	/**
	 * Retrieves the 1st item (FIFO order) without removing it.
	 * @return      1st item in Queue (or nullptr is Queue is empty)
	 */
	void *GetQItem();

	/**
	 * Deletes specified item from the QUEUE (Illegally)
	 * @param pObject       Object to remove
	 */
	void DeleteItem(void *pObject);

	/**
	 * Removes all items from the queue
	 */
	void Flush();
};

} // namespace Bagel

#endif
