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
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"

#include "sci/sci.h"
#include "sci/gui/gui_helpers.h"
#include "sci/gui/gui_memmgr.h"
#include "sci/gui/gui_dbllist.h"

namespace Sci {

DblList::DblList() {
	_hFirst = 0;
	_hLast = 0;
}

DblList::DblList(HEAPHANDLE heap) {
	byte *ptr = heap2Ptr(heap);
	_hFirst = READ_UINT16(ptr);
	_hLast = READ_UINT16(ptr + 2);
}

DblList::~DblList(void) {
}
//--------------------------------------
// Prints all list contents
void DblList::Dump(char*caption) {
	debug("DumpList %s:", caption);
	debug("  First: %04X Last: %04X", _hFirst, _hLast);
	HEAPHANDLE node = _hFirst;
	while (node) {
		GUINode *pNode = (GUINode *)heap2Ptr(node);
		debug("  %04X key=%04X prev=%04X next=%04X add.data=%db", node,
				pNode->key, pNode->prev, pNode->next, heapGetDataSize(node) - 6);
		node = pNode->next;
	}
	debug("End of list");
}
//--------------------------------------
// Add a new node to front of the list
HEAPHANDLE DblList::AddToFront(HEAPHANDLE node, uint16 key) {
	if (!node) {
		warning("Bad node handler (%04X) passed to DblList::AddToFront !",
				node);
		return node;
	}
	GUINode *pNode = (GUINode *)heap2Ptr(node);
	pNode->key = key;
	if (_hFirst) { // we already have  a 1st node
		GUINode *pnext = (GUINode *)heap2Ptr(_hFirst);
		pnext->prev = node;
		pNode->next = _hFirst;
	} else { // list is empty, to passed node becames 1st one
		_hLast = node;
		pNode->next = 0;
	}
	_hFirst = node;
	pNode->prev = 0;
	return node;
}

//-------------------------------------
//
HEAPHANDLE DblList::AddToEnd(HEAPHANDLE node, uint16 key) {
	if (!node) {
		warning("Bad node handler (%04X) passed to DblList::AddToEnd !", node);
		return node;
	}
	GUINode *pNode = (GUINode *)heap2Ptr(node);
	if (_hFirst) { // list is not empty
		GUINode *plast = (GUINode *)heap2Ptr(_hLast);
		plast->next = node;
		pNode->prev = _hLast;
	} else { // list is empty, so the node becames 1st one
		_hFirst = node;
		pNode->prev = 0;
	}
	_hLast = node;
	pNode->next = 0;
	pNode->key = key;

	return node;
}

//------------------------------------------------
// returns node that contains the key
HEAPHANDLE DblList::FindKey(uint16 key) {
	HEAPHANDLE node = _hFirst;
	while (node) {
		GUINode *pNode = (GUINode *)heap2Ptr(node);
		if (pNode->key == key)
			break;
		node = pNode->next;
	}
	return node;
}
//------------------------------------------------
// detaches node with specified key and returning the node
HEAPHANDLE DblList::DeleteKey(uint16 key) {
	HEAPHANDLE node = FindKey(key);
	if (node)
		DeleteNode(node);
	return node;
}
//------------------------------------------------
// detaches specified node from list
byte DblList::DeleteNode(HEAPHANDLE node) {
	if (!node) {
		warning("Bad node handler (%04X) passed to DblList::AddToEnd !", node);
		return node;
	}
	// updating the links
	GUINode *pNode = (GUINode *)heap2Ptr(node);
	if (pNode->prev) {
		GUINode *pprev = (GUINode *)heap2Ptr(pNode->prev);
		pprev->next = pNode->next;
	}
	if (pNode->next) {
		GUINode *pnext = (GUINode *)heap2Ptr(pNode->next);
		pnext->prev = pNode->prev;
	}
	// updating list head if needed
	if (_hFirst == node)
		_hFirst = pNode->next;
	if (_hLast == node)
		_hLast = pNode->prev;
	pNode->prev = 0;
	pNode->next = 0;
	return 1;
}
//------------------------------------------------
// Moves node to the end of the list
HEAPHANDLE DblList::MoveToEnd(HEAPHANDLE node) {
	if (!node) {
		warning("Bad node handler (%04X) passed to DblList::MoveToEnd !", node);
		return node;
	}
	GUINode *pNode = (GUINode *)heap2Ptr(node);
	if (pNode->next) { // node is not the last one in list
		DeleteNode(node);
		AddToEnd(node, pNode->key);
	}
	return node;
}
//------------------------------------------------
// Moves node to the front of the list
HEAPHANDLE DblList::MoveToFront(HEAPHANDLE node) {
	if (!node) {
		warning("Bad node handler (%04X) passed to DblList::MoveToFront !",
				node);
		return node;
	}
	GUINode *pNode = (GUINode *)heap2Ptr(node);
	if (pNode->prev) { // node is not 1st one in list
		DeleteNode(node);
		AddToFront(node, pNode->key);
	}
	return node;
}
//------------------------------------------------
HEAPHANDLE DblList::AddAfter(HEAPHANDLE ref, HEAPHANDLE node, uint16 key) {
	if (!node) {
		warning("Bad node handler (%04X) passed to DblList::AddAfter !", node);
		return node;
	}
	GUINode *pNode = (GUINode *)heap2Ptr(node);
	GUINode *pref = (GUINode *)heap2Ptr(ref);
	pNode->key = key;
	if (pref->next == 0) { // ref node is the last one
		pNode->next = 0;
		_hLast = node;
	} else {
		GUINode *pnext = (GUINode *)heap2Ptr(pref->next);
		pNode->next = pref->next;
		pnext->prev = node;
	}
	pref->next = node;
	pNode->prev = ref;
	return node;
}
//------------------------------------------------
//
HEAPHANDLE DblList::AddBefore(HEAPHANDLE ref, HEAPHANDLE node, uint16 key) {
	if (!node) {
		warning("Bad node handler (%04X) passed to DblList::AddBefore !", node);
		return node;
	}
	GUINode *pNode = (GUINode *)heap2Ptr(node);
	GUINode *pref = (GUINode *)heap2Ptr(ref);
	pNode->key = key;
	if (pref->prev == 0) { // ref node is the 1st one
		pNode->prev = 0;
		_hFirst = node;
	} else {
		GUINode*pprev = (GUINode *)heap2Ptr(pref->prev);
		pNode->prev = pref->prev;
		pprev->next = node;
	}
	pref->prev = node;
	pNode->next = ref;
	return node;
}
//------------------------------------------------
void DblList::toHeap(HEAPHANDLE heap) {
	byte *ptr = heap2Ptr(heap);
	WRITE_UINT16(ptr, _hFirst);
	WRITE_UINT16(ptr + 2, _hLast);
}
//------------------------------------------------
void DblList::DeleteList() {
	HEAPHANDLE node = getFirst(), next;
	GUINode *pNode;
	while (node) {
		pNode = (GUINode *)heap2Ptr(node);
		next = pNode->next;
		heapDisposePtr(node);
		node = next;
	}
	_hFirst = _hLast = 0;
}
//------------------------------------------------
uint16 DblList::getSize() {
	uint16 cnt = 0;
	HEAPHANDLE node = getFirst();
	GUINode *pNode;
	while (node) {
		pNode = (GUINode *)heap2Ptr(node);
		node = pNode->next;
		cnt++;
	}
	return cnt;
}
//------------------------------------------------
} // end of namespace
