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

#include "bagel/boflib/queue.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

CQueue::CQueue() {
}

CQueue::~CQueue() {
	// Can't destruct if we don't exist
	assert(isValidObject(this));

	while (removeItem() != nullptr) {
	}

	// remove() must set _pQueueList to nullptr when it removes the last item
	assert(_pQueueList == nullptr);
}

void CQueue::addItem(void *pObject) {
	// Make sure this object exists
	assert(isValidObject(this));

	CLList *pNewItem = new CLList(pObject);
	if (_pQueueList != nullptr) {
		_pQueueList->addToTail(pNewItem);
	} else {
		_pQueueList = pNewItem;
	}

	assert(pNewItem->getHead() == _pQueueList);
}

void *CQueue::removeItem() {
	// Make sure this object exists
	assert(isValidObject(this));

	// Assume empty list
	void *pObject = nullptr;

	CLList *pList = _pQueueList;
	if (pList != nullptr) {
		pObject = pList->getData();

		_pQueueList = pList->getNext();
		delete pList;
	}

	return pObject;
}

void CQueue::deleteItem(void *pItem) {
	// Make sure this object exists
	assert(isValidObject(this));
	assert(pItem != nullptr);

	CLList *pList = _pQueueList;
	while (pList != nullptr) {
		CLList *pNext = pList->getNext();

		if (pItem == pList->getData()) {
			// If this is the 1st item in the Queue, then move head
			if (pList == _pQueueList)
				_pQueueList = _pQueueList->getNext();

			delete pList;
			break;
		}

		pList = pNext;
	}
}

void *CQueue::getQItem() {
	assert(isValidObject(this));

	void *pItem = nullptr;
	if (_pQueueList != nullptr) {
		pItem = _pQueueList->getData();
	}

	return pItem;
}

void CQueue::flush() {
	assert(isValidObject(this));

	while (removeItem() != nullptr) {
	}
}

} // namespace Bagel
