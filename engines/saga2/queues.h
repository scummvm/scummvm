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

#ifndef SAGA2_QUEUES_H
#define SAGA2_QUEUES_H

namespace Saga2 {

//
// Queue Types:
//   SimpleQueue : queue of items
//   PublicQueue : queue of items - fully public
//   SimplePtrQueue : queue of pointers to items
//   PublicPtrQueue : queue of pointers to items - fully public
//

/* ===================================================================== *
    Memory management
 * ===================================================================== */

#ifndef QUEUES_EXTERNAL_ALLOCATION
#define queueAlloc( s ) malloc( s )
#define queueFree( s ) free( s )
#else
extern void *queueAlloc(size_t s);
extern void queueFree(void *mem);
#endif


//
//  to make a queue of ints do
//    SimpleQueue<int> foo;
//  this template uses pass-by-value so I'd
//  recommend using
//    SimpleQueue<HugeStruct *>
//  instead of
//    SimpleQueue<HugeStruct>
//
//  be sure you check the return value on
//  insert() calls
//

template <class BASETYPE> class SimpleQueue {
	// Embedded type for member elements
	struct SimpleQueueElement {
		BASETYPE data;
		SimpleQueueElement *next;

	public:
		SimpleQueueElement(BASETYPE b) {
			next = NULL;
			data = b;
		}
		~SimpleQueueElement() { }
		void *operator new (size_t s) {
			return queueAlloc(s);
		}
		void operator delete (void *m) {
			queueFree(m);
		}
	private:
		SimpleQueueElement();
	} *head, *tail;
public:

	SimpleQueue() {
		head = NULL;
		tail = NULL;
	}
	~SimpleQueue() {
		while (head) get();
	}
	void *operator new (size_t s) {
		return queueAlloc(s);
	}
	void operator delete (void *m) {
		queueFree(m);
	}

	bool isEmpty(void) {
		return head == NULL;
	}

	BASETYPE first(void) {
		if (isEmpty()) return 0;
		else return head->data;
	}

	BASETYPE get(void) {
		SimpleQueueElement *oldHead = head;
		BASETYPE r = first();
		if (isEmpty())  return r;
		if (NULL == (head = head->next)) tail = NULL;
		delete oldHead;
		return r;
	}

	bool insert(const BASETYPE b) {
		SimpleQueueElement *newElt = new SimpleQueueElement(b);
		if (newElt == NULL)  return false;
		if (tail)
			tail->next = newElt;
		else
			head = newElt;
		tail = newElt;
		return true;
	}
};

//
//
// This is a variation where everything is public
//
//
//
//


template <class BASETYPE> class PublicQueue {
	// Embedded type for member elements
public:
	struct PublicQueueElement {
		BASETYPE data;
		PublicQueueElement *next;

	public:
		PublicQueueElement(BASETYPE b) {
			next = NULL;
			data = b;
		}
		~PublicQueueElement() { }
		void *operator new (size_t s) {
			return queueAlloc(s);
		}
		void operator delete (void *m) {
			queueFree(m);
		}
	private:
		PublicQueueElement();
	} *head, *tail;

	PublicQueue() {
		head = NULL;
		tail = NULL;
	}
	~PublicQueue() {
		while (head) get();
	}
	void *operator new (size_t s) {
		return queueAlloc(s);
	}
	void operator delete (void *m) {
		queueFree(m);
	}

	bool isEmpty(void) {
		return head == NULL;
	}

	BASETYPE first(void) {
		if (isEmpty()) return 0;
		else return head->data;
	}

	BASETYPE get(void) {
		PublicQueueElement *oldHead = head;
		BASETYPE r = first();
		if (isEmpty())  return r;
		if (NULL == (head = head->next)) tail = NULL;
		delete oldHead;
		return r;
	}

	bool insert(const BASETYPE b) {
		PublicQueueElement *newElt = new PublicQueueElement(b);
		if (newElt == NULL)  return false;
		if (tail)
			tail->next = newElt;
		else
			head = newElt;
		tail = newElt;
		return true;
	}
};

//
//  to make a queue of ints do
//    SimplePtrQueue<int> foo;
//  this template uses pass-by-value so I'd
//  recommend using
//    SimplePtrQueue<HugeStruct *>
//  instead of
//    SimplePtrQueue<HugeStruct>
//
//  be sure you check the return value on
//  insert() calls
//

template <class BASETYPE> class SimplePtrQueue {
	// Embedded type for member elements
	struct SimplePtrQueueElement {
		BASETYPE *data;
		SimplePtrQueueElement *next;

	public:
		SimplePtrQueueElement(BASETYPE *b) {
			next = NULL;
			data = b;
		}
		~SimplePtrQueueElement() {
			if (data != NULL) delete data;
			data = NULL;
		}
		void *operator new (size_t s) {
			return queueAlloc(s);
		}
		void operator delete (void *m) {
			queueFree(m);
		}
	private:
		SimplePtrQueueElement();
	} *head, *tail;
public:

	SimplePtrQueue() {
		head = NULL;
		tail = NULL;
	}
	~SimplePtrQueue() {
		while (head) get();
	}
	void *operator new (size_t s) {
		return queueAlloc(s);
	}
	void operator delete (void *m) {
		queueFree(m);
	}

	bool isEmpty(void) {
		return head == NULL;
	}

	BASETYPE *first(void) {
		if (isEmpty()) return 0;
		else return head->data;
	}

	BASETYPE *get(void) {
		SimplePtrQueueElement *oldHead = head;
		BASETYPE *r = first();
		if (isEmpty())  return r;
		if (NULL == (head = head->next)) tail = NULL;
		delete oldHead;
		return r;
	}

	bool insert(BASETYPE *b) {
		SimplePtrQueueElement *newElt = new SimplePtrQueueElement(b);
		if (newElt == NULL)  return false;
		if (tail)
			tail->next = newElt;
		else
			head = newElt;
		tail = newElt;
		return true;
	}
};

//
//
// This is a variation where everything is public
//
//
//
//


template <class BASETYPE> class PublicPtrQueue {
	// Embedded type for member elements
public:
	struct PublicPtrQueueElement {
		BASETYPE *data;
		PublicPtrQueueElement *next;

	public:
		PublicPtrQueueElement(BASETYPE *b) {
			next = NULL;
			data = b;
		}
		~PublicPtrQueueElement() {
			if (data != NULL) delete data;
			data = NULL;
		}
		void *operator new (size_t s) {
			return queueAlloc(s);
		}
		void operator delete (void *m) {
			queueFree(m);
		}
	private:
		PublicPtrQueueElement();
	} *head, *tail;

	PublicPtrQueue() {
		head = NULL;
		tail = NULL;
	}
	~PublicPtrQueue() {
		while (head) get();
	}
	void *operator new (size_t s) {
		return queueAlloc(s);
	}
	void operator delete (void *m) {
		queueFree(m);
	}

	bool isEmpty(void) {
		return head == NULL;
	}

	BASETYPE *first(void) {
		if (isEmpty()) return 0;
		else return head->data;
	}

	BASETYPE *get(void) {
		PublicPtrQueueElement *oldHead = head;
		BASETYPE *r = first();
		if (isEmpty())  return r;
		if (NULL == (head = head->next)) tail = NULL;
		delete oldHead;
		return r;
	}

	bool insert(BASETYPE *b) {
		PublicPtrQueueElement *newElt = new PublicPtrQueueElement(b);
		if (newElt == NULL)  return false;
		if (tail)
			tail->next = newElt;
		else
			head = newElt;
		tail = newElt;
		return true;
	}
};

} // end of namespace Saga2

#endif
