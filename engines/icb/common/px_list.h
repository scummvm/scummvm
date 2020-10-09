/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
 */

#ifndef ICB_PX_LIBRARY_LIST_TEMPLATE
#define ICB_PX_LIBRARY_LIST_TEMPLATE

namespace ICB {

template <class Type> class pxList {
protected:
	Type m_item;          // The item at this node
	pxList<Type> *m_tail; // The tail of the list

public:
	pxList() {
		m_tail = NULL; // Construct an empty list
	}
	~pxList() {
		if (!IsEmpty())
			delete Tail(); // Destruct the array
	}

	Type &Head() {
		return m_item; // The lists' head
	}
	pxList<Type> *Tail() {
		return m_tail; // The lists' tail
	}
	bool IsEmpty() {
		return (m_tail == NULL); // Is the list empty ?
	}
	void Append(const Type &);        // Append item to the end of the list
	void Insert(const Type &);        // Inserts an item after the current item
	pxList<Type> *Find(const Type &); // Returns the list whose head == entry (NULL if not found)
	int Length();                     // Returns the length of the list
};

template <class Type> class rcSortedList {
protected:
	Type m_item;                // The item at this node
	rcSortedList<Type> *m_tail; // The tail of the list

public:
	rcSortedList() {
		m_tail = NULL; // Construct an empty list
	}
	~rcSortedList() {
		if (!IsEmpty())
			delete Tail(); // Destruct the array
	}

public:
	Type &Head();               // The lists' head
	rcSortedList<Type> *Tail(); // The lists' tail
	bool IsEmpty();             // Is the list empty ?
	int Length();               // Returns the length of the list

public:
	rcSortedList<Type> *Insert(const Type &); // Inserts an item in the correct place
	rcSortedList<Type> *Find(const Type &);   // Returns the list whose head == entry (NULL if not found)
};

template <class Type> void pxList<Type>::Append(const Type &entry) {
	if (IsEmpty()) {
		m_item = entry;
		m_tail = new pxList<Type>;
	} else {
		Tail()->Append(entry);
	}
}

template <class Type> void pxList<Type>::Insert(const Type &entry) {
	pxList<Type> *newNode = new pxList<Type>;

	if (IsEmpty()) { // Is the list is empty, insert the item at the start of the list
		m_item = entry;
	} else { // Else put the item before this node
		newNode->m_item = m_item;
		m_item = entry;
		newNode->m_tail = Tail();
	}
	m_tail = newNode;
}

template <class Type> pxList<Type> *pxList<Type>::Find(const Type &entry) {
	// If this is the end of list marker we haven't found it
	if (IsEmpty())
		return NULL;

	if (Head() == entry)
		return this;

	return (Tail()->Find(entry));
}

template <class Type> int pxList<Type>::Length() {
	if (IsEmpty())
		return 0;

	return (1 + Tail()->Length());
}

template <class Type> Type &rcSortedList<Type>::Head() { // The lists' head
	return m_item;
}

template <class Type> rcSortedList<Type> *rcSortedList<Type>::Tail() { // The lists' tail
	return m_tail;
}

template <class Type> bool rcSortedList<Type>::IsEmpty() { // Is the list empty ?
	return (m_tail == NULL);
}

template <class Type> rcSortedList<Type> *rcSortedList<Type>::Insert(const Type &entry) {
	if (IsEmpty()) {
		// End of the list so add the entry here
		m_item = entry;
		m_tail = new rcSortedList<Type>;
		return this;
	}
	// The class being listed must have a '>' Operator defined
	else if (m_item > entry) {
		// The new item comes before the current one
		rcSortedList<Type> *newNode = new rcSortedList<Type>;
		newNode->m_tail = m_tail;
		newNode->m_item = m_item;
		m_item = entry;
		m_tail = newNode;
		return this;
	} else {
		// Keep going
		return Tail()->Insert(entry);
	}
}

template <class Type> rcSortedList<Type> *rcSortedList<Type>::Find(const Type &entry) {
	// If this is the end of list marker we haven't found it
	if (IsEmpty())
		return NULL;

	// this list is sorted, so we can stop when we have a higher value than entry
	if (Head() > entry)
		return NULL;

	if (Head() == entry)
		return this;

	return (Tail()->Find(entry));
}

template <class Type> int rcSortedList<Type>::Length() {
	if (IsEmpty())
		return 0;

	return (1 + Tail()->Length());
}

} // End of namespace ICB

#endif // #ifndef _PX_LIBRARY_LIST_TEMPLATE
