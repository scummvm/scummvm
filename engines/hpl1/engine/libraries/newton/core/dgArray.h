/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
*/

/****************************************************************************
*
*  Visual C++ 6.0 created by: Julio Jerez
*
****************************************************************************/
#ifndef __dgArray__
#define __dgArray__

#include "dgStdafx.h"

template<class T>
class dgArray {
public:
	dgArray(dgInt32 granulatitySize, dgMemoryAllocator *const allocator);
	~dgArray();

	DG_CLASS_ALLOCATOR(allocator)

	T &operator[](dgInt32 i);
	const T &operator[](dgInt32 i) const;
	void Resize(dgInt32 size) const;

private:
	dgInt32 m_granulatity;
	mutable dgInt32 m_maxSize;
	mutable T *m_array;
	dgMemoryAllocator *m_allocator;
};


template<class T>
dgArray<T>::dgArray(dgInt32 granulatitySize, dgMemoryAllocator *const allocator) {
	m_maxSize   = 0;
	m_granulatity   = granulatitySize;
	m_array = NULL;
	m_allocator = allocator;
}

template<class T>
dgArray<T>::~dgArray() {
	if (m_array) {
		m_allocator->FreeLow(m_array);
	}
}


template<class T>
const T &dgArray<T>::operator[](dgInt32 i) const {
	NEWTON_ASSERT(i >= 0);
	while (i >= m_maxSize) {
		Resize(i);
	}
	return m_array[i];
}


template<class T>
T &dgArray<T>::operator[](dgInt32 i) {
	NEWTON_ASSERT(i >= 0);
	while (i >= m_maxSize) {
		Resize(i);
	}
	return m_array[i];
}

template<class T>
void dgArray<T>::Resize(dgInt32 size) const {
	if (size >= m_maxSize) {
		size = size + m_granulatity - (size + m_granulatity) % m_granulatity;
		T *const newArray = (T *) m_allocator->MallocLow(dgInt32(sizeof(T) * size));
		if (m_array) {
			for (dgInt32 i = 0; i < m_maxSize; i ++) {
				newArray[i] = m_array[i];
			}
			m_allocator->FreeLow(m_array);
		}
		m_array = newArray;
		m_maxSize = size;
	} else if (size < m_maxSize) {
		size = size + m_granulatity - (size + m_granulatity) % m_granulatity;
		T *const newArray = (T *) m_allocator->MallocLow(dgInt32(sizeof(T) * size));
		if (m_array) {
			for (dgInt32 i = 0; i < size; i ++) {
				newArray[i] = m_array[i];
			}
			m_allocator->FreeLow(m_array);
		}
		m_array = newArray;
		m_maxSize = size;
	}
}



#endif




