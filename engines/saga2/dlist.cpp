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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/dlist.h"

namespace Saga2 {

DNode *DNode::remove(void) {
	succ->pred = pred;
	pred->succ = succ;
	return this;
}

void DList::addHead(DNode &d) {
	head->pred = &d;
	d.succ = head;
	head = &d;
	d.pred = (DNode *)&head;
}

void DList::addTail(DNode &d) {
	tail->succ = &d;
	d.pred = tail;
	tail = &d;
	d.succ = (DNode *)&overlap;
}

DNode *DList::remHead(void) {
	DNode       *n = head;
	if (n == nullptr)
		return nullptr;

	if (n->succ) {
		head = n->succ;
		head->pred = (DNode *)&head;
		return n;
	}
	return 0;
}

DNode *DList::remTail(void) {
	DNode       *n = tail;

	if (n->pred) {
		tail = n->pred;
		tail->succ = (DNode *)&overlap;
		return n;
	}
	return 0;
}


//  Insert a node before another one
void DList::insert(DNode &d, DNode &before) {
	d.succ = &before;
	d.pred = before.pred;
	before.pred = &d;
	d.pred->succ = &d;
}

void DList::insert(DNode &d, void *extra, int (*compare)(DNode &d1, DNode &d2, void *extra)) {
	DNode           *search;

	for (search = head;
	        search->succ != NULL && (*compare)(*search, d, extra) < 0;
	        search = search->succ)
		;

	insert(d, *search);
}


int DList::count(void) const {
	int     ct;                                         // count
	DNode   *d;

	for (d = head, ct = -1;                          // scan list
	        d;
	        d = d->succ, ++ct) ;                          // bunp count

	return ct;
}

DNode *DList::select(int number) const {
	if (number < 0) {
		for (DNode *d = tail; d->succ; d = d->succ, ++number) {
			if (number >= -1) return d;
		}
		return 0;
	}

	for (DNode *d = head; d->succ; d = d->succ, --number) {
		if (number <= 0) return d;
	}
	return 0;
}

} // end of namespace Saga2
