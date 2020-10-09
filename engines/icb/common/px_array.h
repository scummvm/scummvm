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

#ifndef ICB_LIBRARY_CMYACTARRAY
#define ICB_LIBRARY_CMYACTARRAY

#include <cstring>
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_rcutypes.h"

namespace ICB {

#define MY_TEMPLATE template <class Type>
#define T_MYACTARRAY rcActArray<Type>
#define T_MYPTRARRAY rcAutoPtrArray<Type>

MY_TEMPLATE class rcActArray {
public:
	rcActArray() { // Construct an empty array
		m_userPosition = m_allocatedSize = 0;
	}

	rcActArray(rcActArray &a) {
		m_userPosition = m_allocatedSize = 0;
		(*this) = a;
	}

	~rcActArray(); // Destruct the array

	void operator=(const rcActArray &);

	// Member access functions
	uint GetNoItems() const { return (m_userPosition); }

	uint Add(const Type &f); // Add an item.

	Type &operator[](uint); // Give access to an entry
	const Type &operator[](uint i) const;

	void SetSize(uint n) { ResizeArray(n); }
	void Reset();

private:
	uint m_userPosition;  // Next place to add an item to
	uint m_allocatedSize; // How many items have been allocated

	Type **m_contents; // A pointer to pointers to the objects

	void ResizeArray(uint); // Change the size of the array
};

MY_TEMPLATE
void T_MYACTARRAY::operator=(const rcActArray &a) {
	if (m_allocatedSize)
		delete[] m_contents;
	m_userPosition = a.m_userPosition;
	m_allocatedSize = a.m_allocatedSize;

	if (m_allocatedSize)

	{
		m_contents = new Type *[m_allocatedSize];
		for (uint count = 0; count < m_allocatedSize; count++)
			m_contents[count] = new Type(*(a.m_contents[count]));
	}
}

MY_TEMPLATE
Type &T_MYACTARRAY::operator[](uint n) {
	if (n >= m_userPosition) {
		ResizeArray(n);
		m_userPosition = n + 1;
	}
	return (*(m_contents[n]));
}

MY_TEMPLATE
const Type &T_MYACTARRAY::operator[](uint n) const {
	// It is permissable to look at an element that has not been defined, as the constructor assures
	// that the contents are valid
	if (n >= m_userPosition) {
		// We must cast this to a type that can change
		((const rcActArray<Type> *)this)->ResizeArray(n);
		((const rcActArray<Type> *)this)->m_userPosition = n + 1;
	}

	return (*(m_contents[n]));
}

MY_TEMPLATE T_MYACTARRAY::~rcActArray() { Reset(); }

MY_TEMPLATE void T_MYACTARRAY::Reset() {
	for (uint count = 0; count < m_allocatedSize; count++)
		delete m_contents[count];

	if (m_allocatedSize)
		delete[] m_contents;
	m_allocatedSize = 0;
	m_userPosition = 0;
}

MY_TEMPLATE void T_MYACTARRAY::ResizeArray(uint n2) {
	// if n is still within the allocated area then just set the last position
	if (n2 >= m_allocatedSize) {
		// Make sure we are going to make the thing big enough
		uint nextSize = m_allocatedSize ? m_allocatedSize + m_allocatedSize : 1; // Double, or 1 if now 0
		while (nextSize <= n2)
			nextSize += nextSize;

		// Get a New pointer array of the correct size
		Type **newArray = new Type *[nextSize];
		if (m_allocatedSize > 0) {
			// Copy in the old stuff
			memcpy((unsigned char *)newArray, (unsigned char *)m_contents, m_allocatedSize * sizeof(Type *));
		}
		// Put empty objects in the newly allocated space
		for (uint newObjects = m_allocatedSize; newObjects < nextSize; newObjects++)
			newArray[newObjects] = new Type;
		// Remove any old stuff
		if (m_allocatedSize)
			delete[] m_contents;
		m_contents = newArray;
		m_allocatedSize = nextSize;
	}
}

MY_TEMPLATE uint T_MYACTARRAY::Add(const Type &f) {
	operator[](m_userPosition) = f;
	return (m_userPosition - 1);
}

MY_TEMPLATE class rcAutoPtrArray {
	uint m_noContents;   // How many entries have been allocated
	uint m_userPosition; // Next position for the Add command

	Type **m_contents; // A pointer to pointers to the objects

	void ResizeArray(uint); // Change the size of the array
public:
	explicit rcAutoPtrArray() { // Construct an empty array
		m_noContents = m_userPosition = 0;
	}
	~rcAutoPtrArray(); // Destruct the array

	// Member access functions
	uint GetNoItems() const { return (m_userPosition); }

	uint Add(Type *f) {
		operator[](m_userPosition) = f;
		return (m_userPosition - 1);
	}

	Type *&operator[](uint);             // Give access to an entry
	const Type *&operator[](uint) const; // Give access to an entry

	void Reset();
	void RemoveAndShuffle(uint); // Remove an object from the array
	void SetSize(uint n) { ResizeArray(n); }

	// Super dangerous, but faster, access to the array
	Type *GetRawArray() { return (*m_contents); }

private: // Prevent use of the PtrArray copy constructor
	// The default copy constructor should never be called
	rcAutoPtrArray(const rcAutoPtrArray &) {}
	void operator=(const rcAutoPtrArray &) {}
};

MY_TEMPLATE
Type *&T_MYPTRARRAY::operator[](uint n) {
	if (n >= m_userPosition) {
		ResizeArray(n);
		m_userPosition = n + 1;
	}
	return (m_contents[n]);
}

MY_TEMPLATE
const Type *&T_MYPTRARRAY::operator[](uint n) const {
	// It is permissable to look at an element that has not been defined, as it will be defined as NULL
	if (n >= m_userPosition) {
		(const_cast<rcAutoPtrArray<Type> *>(this))->ResizeArray(n);
		(const_cast<rcAutoPtrArray<Type> *>(this))->m_userPosition = n + 1;
	}

	return const_cast<const Type *&>(m_contents[n]);
}

MY_TEMPLATE T_MYPTRARRAY::~rcAutoPtrArray() { Reset(); }

MY_TEMPLATE void T_MYPTRARRAY::Reset() {
	// The pointer array maintains responsibility for deleting any contents
	for (uint count = 0; count < m_userPosition; count++)
		if (m_contents[count])
			delete m_contents[count];
	if (m_noContents)
		delete[] m_contents;
	m_noContents = m_userPosition = 0;
}

MY_TEMPLATE void T_MYPTRARRAY::ResizeArray(uint n2) {
	if (n2 >= m_noContents) {
		// Double the allocation value
		uint nextSize = m_noContents > 0 ? m_noContents + m_noContents : 1;
		while (n2 >= nextSize)
			nextSize = nextSize + nextSize;
		// Get a New pointer array of the correct size
		Type **newArray = new Type *[nextSize];
		// Copy in the old stuff, if there is any
		if (m_noContents > 0)
			memcpy((unsigned char *)newArray, (unsigned char *)m_contents, m_noContents * sizeof(Type *));
		// Reset the New entries
		memset((unsigned char *)(newArray + m_noContents), 0, (nextSize - m_noContents) * sizeof(Type *));
		// Remove any old stuff
		if (m_noContents)
			delete[] m_contents;
		m_contents = newArray;
		m_noContents = nextSize;
	}
}

MY_TEMPLATE void T_MYPTRARRAY::RemoveAndShuffle(uint n) {
	// Remove an object from the array

	// First delete it
	if (m_contents[n])
		delete m_contents[n];
	// and shuffle the array
	memcpy(m_contents + n, m_contents + n + 1, (m_noContents - n - 1) * sizeof(Type *));
}

template <class Type> class rcIntArray {
	uint m_noContents;   // How many entries there are
	uint m_userPosition; // Where the next add position goes
	Type *m_contents;

	void ResizeArray(uint); // Change the size of the array

public:
	explicit rcIntArray() { // Construct an empty array
		m_noContents = m_userPosition = 0;
	}
	~rcIntArray() { // Destruct the array
		if (m_noContents)
			delete[] m_contents;
	}

	// Copy constructor
	rcIntArray(const rcIntArray &a) {
		m_noContents = m_userPosition = 0;
		(*this) = a;
	}

	// Constructor with an initial size
	rcIntArray(uint initialSize) { ResizeArray(initialSize); }

	const rcIntArray &operator=(const rcIntArray &);

	// Member access functions
	uint GetNoItems() const { return (m_userPosition); }

	uint Add(Type f); // Add an integer. Only makes sense if the resize step is one

	Type &operator[](uint);            // Give access to an entry
	const Type operator[](uint) const; // Give access to an entry

	void Reset();
	void SetSize(uint n) { ResizeArray(n); }

	Type *GetRawArray() { return (m_contents); }
};

template <class Type> Type &rcIntArray<Type>::operator[](uint index) {
	if (index >= m_userPosition) {
		ResizeArray(index);
		m_userPosition = index + 1;
	}
	return m_contents[index];
}

// This version of [] allows the array to be part of a const function
template <class Type> const Type rcIntArray<Type>::operator[](uint index) const {
	// It is permissable to look at an element that has not been defined, as it will have been set to 0
	if (index >= m_userPosition) {
		// Remove any 'constness' for a resize
		((const rcIntArray<Type> *)this)->ResizeArray(index);
		((const rcIntArray<Type> *)this)->m_userPosition = index + 1;
	}

	return m_contents[index];
}

template <class IntType> void rcIntArray<IntType>::ResizeArray(uint accessedSize) {
	// Check if we need to do any reallocating
	if (accessedSize >= m_noContents) {
		uint newSize = m_noContents > 0 ? m_noContents * 2 : 1;
		while (newSize <= accessedSize)
			newSize = newSize + newSize;

		IntType *newArray = new IntType[newSize];
		if (m_noContents)
			memcpy(newArray, m_contents, m_noContents * sizeof(IntType));
		// Call me a fool, but I like my integers initialised to 0
		memset(newArray + m_noContents, 0, (newSize - m_noContents) * sizeof(IntType));

		if (m_noContents)
			delete[] m_contents;
		m_contents = newArray;
		m_noContents = newSize;
	}
}

template <class IntType> const rcIntArray<IntType> &rcIntArray<IntType>::operator=(const rcIntArray<IntType> &obOpB) {
	uint nCount;

	if (m_noContents)
		delete[] m_contents;
	m_userPosition = obOpB.m_userPosition;
	m_noContents = obOpB.m_noContents;

	if (m_noContents) {
		m_contents = new IntType[m_noContents];
		for (nCount = 0; nCount < m_noContents; nCount++)
			m_contents[nCount] = obOpB.m_contents[nCount];
	}

	return *this;
}

template <class Type> void rcIntArray<Type>::Reset() {
	// CLear out the array
	if (m_noContents) {
		delete[] m_contents;
		m_noContents = m_userPosition = 0;
	}
}

template <class Type> uint rcIntArray<Type>::Add(Type f) {
	// Add an integer. Only makes sense if the resize step is one
	operator[](m_userPosition) = f;
	return (m_userPosition - 1);
}

} // End of namespace ICB

#endif // ndef _LIBRARY_CMYACTARRAY
