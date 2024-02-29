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

namespace Bagel {

CQueue::CQueue() {
}

CQueue::CQueue(VOID *pObject) {
	// Validate input
	Assert(pObject != nullptr);

	// Start queue with this object
	AddItem(pObject);
}

CQueue::CQueue(CQueue *pQueue) {
	CLList *pList;

	// validate input queue
	Assert(pQueue != nullptr);

	pList = pQueue->m_pQueueList;
	while (pList != nullptr) {
		AddItem(pList->GetData());
		pList = pList->GetNext();
	}
}

CQueue::CQueue(const CQueue &cQueue) {
	CLList *pList;

	pList = cQueue.m_pQueueList;
	while (pList != nullptr) {
		AddItem(pList->GetData());
		pList = pList->GetNext();
	}
}

CQueue::~CQueue() {
	// Can't destruct if we don't exist
	Assert(IsValidObject(this));

	while (RemoveItem() != nullptr) {
	}

	// Remove() must set m_pQueueList to nullptr when it removes the last item
	Assert(m_pQueueList == nullptr);
}

VOID CQueue::AddItem(VOID *pObject) {
	// make sure this object exists
	Assert(IsValidObject(this));

	CLList *pNewItem;

	if ((pNewItem = new CLList(pObject)) != nullptr) {
		if (m_pQueueList != nullptr) {
			m_pQueueList->AddToTail(pNewItem);
		} else {
			m_pQueueList = pNewItem;
		}

		Assert(pNewItem->GetHead() == m_pQueueList);
	}
}

VOID *CQueue::RemoveItem() {
	// make sure this object exists
	Assert(IsValidObject(this));

	CLList *pList;
	VOID *pObject;

	// assume empty list
	pObject = nullptr;

	if ((pList = m_pQueueList) != nullptr) {

		pObject = pList->GetData();

		m_pQueueList = pList->GetNext();
		delete pList;
	}

	return (pObject);
}

VOID CQueue::DeleteItem(VOID *pItem) {
	// make sure this object exists
	Assert(IsValidObject(this));
	Assert(pItem != nullptr);

	CLList *pList, *pNext;

	pList = m_pQueueList;
	while (pList != nullptr) {
		pNext = pList->GetNext();

		if (pItem == pList->GetData()) {

			// If this is the 1st item in the Queue, then move head
			if (pList == m_pQueueList)
				m_pQueueList = m_pQueueList->GetNext();

			delete pList;
			break;
		}

		pList = pNext;
	}
}

VOID *CQueue::GetQItem() {
	Assert(IsValidObject(this));

	VOID *pItem;

	pItem = nullptr;
	if (m_pQueueList != nullptr) {
		pItem = m_pQueueList->GetData();
	}

	return pItem;
}

VOID CQueue::Flush() {
	Assert(IsValidObject(this));

	while (RemoveItem() != nullptr) {
	}
}

} // namespace Bagel
