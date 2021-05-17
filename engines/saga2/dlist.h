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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */


#ifndef SAGA2_DLIST_H
#define SAGA2_DLIST_H

namespace Saga2 {

//  A doubly-linked list node

class DNode {
	DNode               *succ,              // ptr to next in list
	                    *pred;              // ptr to prev in list

	friend class        DList;
	friend class        DList_iter;

public:
	DNode *next(void) const {
		return succ->succ ? succ : 0;    // Next in list
	}
	DNode *prev(void) const {
		return pred->pred ? pred : 0;    // Previous in list
	}
	DNode *remove(void);
};

//  A doubly-linked list (with dummy pointer to resolve end cases)

class DList {
	DNode               *head,                  // ptr to next in list
	                    *overlap,               // dummy pointer
	                    *tail;                  // ptr to prev in list

	friend class        DList_iter;
public:

	//  constructor
	DList(void) {
		head = (DNode *)&overlap;
		tail = (DNode *)&head;
		overlap = 0;
	}

	//  Return pointer to first/last node in list
	DNode *first(void) const {
		return head->succ ? head : NULL;
	}
	DNode *last(void) const {
		return tail->pred ? tail : NULL;
	}

	//  Add node to head/tail of list
	void addHead(DNode &d);
	void addTail(DNode &d);

	//  Remove head/tail
	DNode *remHead(void);
	DNode *remTail(void);

	//  Test if list is empty
	bool empty(void) {
		return head == (DNode *)&overlap;
	}

	//  Insert a node before another one
	void insert(DNode &d, DNode &before);

	//  Insert a node in a specific order
	void insert(DNode &d, void *extra, int (*compare)(DNode &d1, DNode &d2, void *extra));

	//  Count number of nodes in the list
	int count(void) const;

	//  Select the Nth node in the list
	DNode *select(int number) const;
};

class DList_iter {
	DNode               *current;

public:
	//  Constructor, initializes the iterator to the head of the list
	DList_iter(DList &dl) {
		current = dl.head;
	}

	DNode *next(void) {
		return current->succ ? (current = current->succ) : 0;
	}
	DNode *prev(void) {
		return current->pred ? (current = current->pred) : 0;
	}
#if 0
	DNode *remove(void) {
		if (current->succ && current->pred) {
			current.remove();
			current = current->succ;
		}
	}
#endif
};

} // end of namespace Saga2

#endif
