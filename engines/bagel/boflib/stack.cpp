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

#include "bagel/boflib/stack.h"

namespace Bagel {

CStack::CStack() {
}

CStack::CStack(VOID *pObject) {
	// validate input
	Assert(pObject != NULL);

	// start stack with this object
	Push(pObject);
}

CStack::CStack(CStack *pStack) {
	CLList *pList;

	// validate input stack
	Assert(pStack != NULL);

	pList = pStack->m_pStackList;
	while (pList != NULL) {
		Push(pList->GetData());
		pList = pList->GetNext();
	}
}

CStack::CStack(const CStack &cStack) {
	CLList *pList;

	pList = cStack.m_pStackList;
	while (pList != NULL) {
		Push(pList->GetData());
		pList = pList->GetNext();
	}
}

CStack::~CStack() {
	// make sure we are not destructing an invalid object
	Assert(IsValidObject(this));

	// remove all items from the stack
	//
	while (Pop() != NULL) {
	}

	// Pop() must set m_pStackList to NULL when it removes the last item
	// from the stack.
	Assert(m_pStackList == NULL);
}

VOID CStack::Push(VOID *pObject) {
	// make sure this object exists
	Assert(IsValidObject(this));

	CLList *pNewItem;

	if ((pNewItem = new CLList(pObject)) != NULL) {

		if (m_pStackList != NULL) {
			m_pStackList->AddToTail(pNewItem);
		} else {
			m_pStackList = pNewItem;
		}

		Assert(pNewItem->GetHead() == m_pStackList);
	}
}

VOID *CStack::Pop() {
	// make sure this object exists
	Assert(IsValidObject(this));

	CLList *pTop;
	VOID *pObject;

	pObject = NULL;

	if (m_pStackList != NULL) {
		pTop = m_pStackList->GetTail();

		// m_pStackList must always be the head of the linked list
		Assert(pTop->GetHead() == m_pStackList);

		pObject = pTop->GetData();

		if (pTop == m_pStackList)
			m_pStackList = NULL;

		delete pTop;
	}

	return pObject;
}

} // namespace Bagel
