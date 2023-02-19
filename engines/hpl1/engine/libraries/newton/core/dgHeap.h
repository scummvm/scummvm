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
#ifndef __dgHeapBase__
#define __dgHeapBase__

#include "dgStdafx.h"
#include "dgMemory.h"

#include "common/util.h"


//#define DG_HEAP_SANITY_CHECK

template <class OBJECT, class KEY>
class dgHeapBase {
protected:
	struct RECORD {
		KEY m_key;
		OBJECT m_obj;

		RECORD(KEY key, const OBJECT &obj)
			: m_key(key), m_obj(obj) {
		}
	};

	dgHeapBase(dgInt32 maxElements, dgMemoryAllocator *const allocator);
	dgHeapBase(const void *const buffer, dgInt32 sizeInBytes);
	~dgHeapBase();

public:
	DG_CLASS_ALLOCATOR(allocator)

	void Flush();
	KEY MaxValue() const;
	KEY Value(dgInt32 i = 0) const;
	dgInt32 GetCount() const;
	dgInt32 GetMaxCount() const;
	const OBJECT &operator[](dgInt32 i) const;
	OBJECT &operator[](dgInt32 i);
	dgInt32 Find(OBJECT &obj);
	dgInt32 Find(KEY key);

	dgInt32 m_curCount;
	dgInt32 m_maxCount;
//	bool m_allocated;
	dgMemoryAllocator *m_allocator;
	RECORD *m_pool;
};

template <class OBJECT, class KEY>
class dgDownHeap: public dgHeapBase<OBJECT, KEY> {
public:
	dgDownHeap(dgInt32 maxElements, dgMemoryAllocator *const allocator);
	dgDownHeap(const void *const buffer, dgInt32 sizeInBytes);

	void Pop();
	void Push(OBJECT &obj, KEY key);
	void Sort();
	void Remove(dgInt32 Index);

#ifdef DG_HEAP_SANITY_CHECK
	bool SanityCheck();
#endif
};

template <class OBJECT, class KEY>
class dgUpHeap: public dgHeapBase<OBJECT, KEY> {
public:
	dgUpHeap(dgInt32 maxElements, dgMemoryAllocator *const allocator);
	dgUpHeap(const void *const buffer, dgInt32 sizeInBytes);

	void Pop();
	void Push(OBJECT &obj, KEY key);
	void Sort();
	void Remove(dgInt32 Index);

#ifdef DG_HEAP_SANITY_CHECK
	bool SanityCheck();
#endif
};



template <class OBJECT, class KEY>
dgHeapBase<OBJECT, KEY>::dgHeapBase(dgInt32 maxElements, dgMemoryAllocator *const allocator) {
//	m_allocated = true;
	m_allocator = allocator;
	m_pool = (RECORD *)m_allocator->Malloc(maxElements * sizeof(RECORD));
	m_maxCount = maxElements;
	Flush();
}

template <class OBJECT, class KEY>
dgHeapBase<OBJECT, KEY>::dgHeapBase(const void *const buffer, dgInt32 sizeInBytes) {
//	NEWTON_ASSERT (0);
//	m_allocated = false;
	m_allocator = NULL;
	m_pool = const_cast<RECORD *>((const RECORD *)buffer);
	m_maxCount = dgInt32(sizeInBytes / sizeof(RECORD));
	Flush();
}

template <class OBJECT, class KEY>
dgHeapBase<OBJECT, KEY>::~dgHeapBase() {
	if (m_allocator) {
		m_allocator->Free(m_pool);
	}
}


template <class OBJECT, class KEY>
KEY dgHeapBase<OBJECT, KEY>::Value(dgInt32 i) const {
	return m_pool[i].m_key;
}


template <class OBJECT, class KEY>
dgInt32 dgHeapBase<OBJECT, KEY>::GetCount() const {
	return m_curCount;
}


template <class OBJECT, class KEY>
void dgHeapBase<OBJECT, KEY>::Flush() {
	m_curCount = 0;

#ifdef _DEBUG
//	dgHeapBase<OBJECT,KEY>::m_pool[dgHeapBase<OBJECT,KEY>::m_curCount].m_key = KEY (0);
#endif
}


template <class OBJECT, class KEY>
KEY dgHeapBase<OBJECT, KEY>::MaxValue() const {
	return m_pool[0].m_key;
}


template <class OBJECT, class KEY>
dgInt32 dgHeapBase<OBJECT, KEY>::GetMaxCount() const {
	return m_maxCount;
}


template <class OBJECT, class KEY>
dgInt32 dgHeapBase<OBJECT, KEY>::Find(OBJECT &obj) {
	// For now let perform a linear search
	// this is efficient if the size of the heap is small
	// ex: m_curCount < 32
	// this will be change to a binary search in the heap should the
	// the size of the heap get larger than 32
	//  NEWTON_ASSERT (m_curCount <= 32);
	for (dgInt32 i = 0; i < m_curCount; i ++) {
		if (m_pool[i].obj == obj) {
			return i;
		}
	}
	return - 1;
}


template <class OBJECT, class KEY>
dgInt32 dgHeapBase<OBJECT, KEY>::Find(KEY key) {
	// ex: m_curCount < 32
	// this will be change to a binary search in the heap shoud the
	// the size of the heap get larger than 32
	NEWTON_ASSERT(m_curCount <= 32);
	for (dgInt32 i = 0; i < m_curCount; i ++)   {
		if (m_pool[i].m_key == key) {
			return i;
		}
	}
	return - 1;
}


template <class OBJECT, class KEY>
const OBJECT &dgHeapBase<OBJECT, KEY>::operator[](dgInt32 i) const {
	NEWTON_ASSERT(i <= m_curCount);
	return m_pool[i].m_obj;
}

template <class OBJECT, class KEY>
OBJECT &dgHeapBase<OBJECT, KEY>::operator[](dgInt32 i) {
	NEWTON_ASSERT(i <= m_curCount);
	return m_pool[i].m_obj;
}


// **************************************************************************
//
// down Heap
//
// **************************************************************************
template <class OBJECT, class KEY>
dgDownHeap<OBJECT, KEY>::dgDownHeap(dgInt32 maxElements, dgMemoryAllocator *const allocator)
	: dgHeapBase<OBJECT, KEY> (maxElements, allocator) {
}

template <class OBJECT, class KEY>
dgDownHeap<OBJECT, KEY>::dgDownHeap(const void *const buffer, dgInt32 sizeInBytes)
	: dgHeapBase<OBJECT, KEY> (buffer, sizeInBytes) {
}


template <class OBJECT, class KEY>
void dgDownHeap<OBJECT, KEY>::Push(OBJECT &obj, KEY key) {
	dgInt32 i;
	dgInt32 j;
#ifdef _DEBUG
//	NEWTON_ASSERT (m_curCount < m_maxCount);
	dgInt32 cc = dgHeapBase<OBJECT, KEY>::m_curCount;
	dgInt32 cm = dgHeapBase<OBJECT, KEY>::m_maxCount;
	NEWTON_ASSERT(cc < cm);
#endif

	dgHeapBase<OBJECT, KEY>::m_curCount ++;

	for (i = dgHeapBase<OBJECT, KEY>::m_curCount; i; i = j) {
		j = i >> 1;
		if (!j || (dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key > key)) {
			break;
		}
		dgHeapBase<OBJECT, KEY>::m_pool[i - 1] = dgHeapBase<OBJECT, KEY>::m_pool[j - 1];
	}
	NEWTON_ASSERT(i);
	dgHeapBase<OBJECT, KEY>::m_pool[i - 1].m_key = key;
	dgHeapBase<OBJECT, KEY>::m_pool[i - 1].m_obj = obj;

#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}


template <class OBJECT, class KEY>
void dgDownHeap<OBJECT, KEY>::Remove(dgInt32 index) {
	dgInt32 j;
	dgInt32 k;

	dgHeapBase<OBJECT, KEY>::m_curCount--;
	KEY key(dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_key);
	for (k = index + 1; k <= (dgHeapBase<OBJECT, KEY>::m_curCount >> 1); k = j) {
		j = k + k;
		if ((j < dgHeapBase<OBJECT, KEY>::m_curCount) &&
		        (dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key < dgHeapBase<OBJECT, KEY>::m_pool[j].m_key)) {
			j ++;
		}
		if (key >= dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key) {
			break;
		}
		dgHeapBase<OBJECT, KEY>::m_pool[k - 1] = dgHeapBase<OBJECT, KEY>::m_pool[j - 1];
	}
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_key = key;
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_obj = dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_obj;

#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif

}

template <class OBJECT, class KEY>
void dgDownHeap<OBJECT, KEY>::Pop() {
	dgInt32 j;
	dgInt32 k;

	dgHeapBase<OBJECT, KEY>::m_curCount--;
	KEY key(dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_key);
	for (k = 1; k <= (dgHeapBase<OBJECT, KEY>::m_curCount >> 1); k = j) {
		j = k + k;
		if ((j < dgHeapBase<OBJECT, KEY>::m_curCount) &&
		        (dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key < dgHeapBase<OBJECT, KEY>::m_pool[j].m_key)) {
			j ++;
		}
		if (key >= dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key) {
			break;
		}
		dgHeapBase<OBJECT, KEY>::m_pool[k - 1] = dgHeapBase<OBJECT, KEY>::m_pool[j - 1];
	}
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_key = key;
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_obj = dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_obj;

#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}



template <class OBJECT, class KEY>
void dgDownHeap<OBJECT, KEY>::Sort() {

	dgInt32 count = dgHeapBase<OBJECT, KEY>::m_curCount;
	for (dgInt32 i = 1; i < count; i ++) {
		KEY key(dgHeapBase<OBJECT, KEY>::m_pool[0].m_key);
		OBJECT obj(dgHeapBase<OBJECT, KEY>::m_pool[0].m_obj);

		Pop();

		dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_key = key;
		dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_obj = obj;
	}

	dgHeapBase<OBJECT, KEY>::m_curCount = count;
	for (dgInt32 i = 0; i < count / 2; i ++) {
		KEY key(dgHeapBase<OBJECT, KEY>::m_pool[i].m_key);
		OBJECT obj(dgHeapBase<OBJECT, KEY>::m_pool[i].m_obj);

		dgHeapBase<OBJECT, KEY>::m_pool[i].m_key = dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_key;
		dgHeapBase<OBJECT, KEY>::m_pool[i].m_obj = dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_obj;

		dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_key = key;
		dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_obj = obj;
	}
#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}

#ifdef DG_HEAP_SANITY_CHECK
template <class OBJECT, class KEY>
bool dgDownHeap<OBJECT, KEY>::SanityCheck() {
	for (dgInt32 i = 0; i < dgHeapBase<OBJECT, KEY>::m_curCount / 2; i ++) {
		if (dgHeapBase<OBJECT, KEY>::m_pool[i].m_key < dgHeapBase<OBJECT, KEY>::m_pool[i * 2 + 1].m_key) {
			return false;
		}
		if ((i * 2 + 2) < dgHeapBase<OBJECT, KEY>::m_curCount) {
			if (dgHeapBase<OBJECT, KEY>::m_pool[i].m_key < dgHeapBase<OBJECT, KEY>::m_pool[i * 2 + 2].m_key) {
				return false;
			}
		}
	}

	return true;
}
#endif




// **************************************************************************
//
// down Heap
//
// **************************************************************************
template <class OBJECT, class KEY>
dgUpHeap<OBJECT, KEY>::dgUpHeap(dgInt32 maxElements, dgMemoryAllocator *const allocator)
	: dgHeapBase<OBJECT, KEY> (maxElements, allocator) {
}

template <class OBJECT, class KEY>
dgUpHeap<OBJECT, KEY>::dgUpHeap(const void *const buffer, dgInt32 sizeInBytes)
	: dgHeapBase<OBJECT, KEY> (buffer, sizeInBytes) {
}

#ifdef DG_HEAP_SANITY_CHECK
template <class OBJECT, class KEY>
bool dgUpHeap<OBJECT, KEY>::SanityCheck() {
	for (dgInt32 i = 0; i < dgHeapBase<OBJECT, KEY>::m_curCount / 2; i ++) {
		if (dgHeapBase<OBJECT, KEY>::m_pool[i].m_key > dgHeapBase<OBJECT, KEY>::m_pool[i * 2 + 1].m_key) {
			return false;
		}
		if ((i * 2 + 2) < dgHeapBase<OBJECT, KEY>::m_curCount) {
			if (dgHeapBase<OBJECT, KEY>::m_pool[i].m_key > dgHeapBase<OBJECT, KEY>::m_pool[i * 2 + 2].m_key) {
				return false;
			}
		}
	}

	return true;
}
#endif

template <class OBJECT, class KEY>
void dgUpHeap<OBJECT, KEY>::Push(OBJECT &obj, KEY key) {
	dgInt32 i;
	dgInt32 j;

#ifdef _DEBUG
	//  NEWTON_ASSERT (m_curCount < m_maxCount);
	dgInt32 cc = dgHeapBase<OBJECT, KEY>::m_curCount;
	dgInt32 cm = dgHeapBase<OBJECT, KEY>::m_maxCount;
	NEWTON_ASSERT(cc < cm);
#endif
	dgHeapBase<OBJECT, KEY>::m_curCount ++;

	for (i = dgHeapBase<OBJECT, KEY>::m_curCount; i; i = j) {
		j = i >> 1;
		if (!j || (dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key < key)) {
			break;
		}
		dgHeapBase<OBJECT, KEY>::m_pool[i - 1] = dgHeapBase<OBJECT, KEY>::m_pool[j - 1];
	}
	NEWTON_ASSERT(i);
	dgHeapBase<OBJECT, KEY>::m_pool[i - 1].m_key = key;
	dgHeapBase<OBJECT, KEY>::m_pool[i - 1].m_obj = obj;

#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}


template <class OBJECT, class KEY>
void dgUpHeap<OBJECT, KEY>::Sort() {
	dgInt32 count = dgHeapBase<OBJECT, KEY>::m_curCount;
	for (dgInt32 i = 1; i < count; i ++) {
		KEY key(dgHeapBase<OBJECT, KEY>::m_pool[0].m_key);
		OBJECT obj(dgHeapBase<OBJECT, KEY>::m_pool[0].m_obj);

		Pop();

		dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_key = key;
		dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_obj = obj;
	}

	dgHeapBase<OBJECT, KEY>::m_curCount = count;
	for (dgInt32 i = 0; i < count / 2; i ++) {
		KEY key(dgHeapBase<OBJECT, KEY>::m_pool[i].m_key);
		OBJECT obj(dgHeapBase<OBJECT, KEY>::m_pool[i].m_obj);

		dgHeapBase<OBJECT, KEY>::m_pool[i].m_key = dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_key;
		dgHeapBase<OBJECT, KEY>::m_pool[i].m_obj = dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_obj;

		dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_key = key;
		dgHeapBase<OBJECT, KEY>::m_pool[count - i - 1].m_obj = obj;
	}
#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}


template <class OBJECT, class KEY>
void dgUpHeap<OBJECT, KEY>::Remove(dgInt32 index) {
	dgInt32 j;
	dgInt32 k;

	dgHeapBase<OBJECT, KEY>::m_curCount--;
	KEY key(dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_key);
	for (k = index + 1; k <= (dgHeapBase<OBJECT, KEY>::m_curCount >> 1); k = j) {
		j = k + k;
		if ((j < dgHeapBase<OBJECT, KEY>::m_curCount) &&
		        (dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key > dgHeapBase<OBJECT, KEY>::m_pool[j].m_key)) {
			j ++;
		}
		if (key <= dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key) {
			break;
		}
		dgHeapBase<OBJECT, KEY>::m_pool[k - 1] = dgHeapBase<OBJECT, KEY>::m_pool[j - 1];
	}
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_key = key;
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_obj = dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_obj;

#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}


template <class OBJECT, class KEY>
void dgUpHeap<OBJECT, KEY>::Pop() {
	dgInt32 j;
	dgInt32 k;

	dgHeapBase<OBJECT, KEY>::m_curCount--;
	KEY key(dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_key);
	for (k = 1; k <= (dgHeapBase<OBJECT, KEY>::m_curCount >> 1); k = j) {
		j = k + k;
		if ((j < dgHeapBase<OBJECT, KEY>::m_curCount) &&
		        (dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key > dgHeapBase<OBJECT, KEY>::m_pool[j].m_key)) {
			j ++;
		}
		if (key <= dgHeapBase<OBJECT, KEY>::m_pool[j - 1].m_key) {
			break;
		}
		dgHeapBase<OBJECT, KEY>::m_pool[k - 1] = dgHeapBase<OBJECT, KEY>::m_pool[j - 1];
	}
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_key = key;
	dgHeapBase<OBJECT, KEY>::m_pool[k - 1].m_obj = dgHeapBase<OBJECT, KEY>::m_pool[dgHeapBase<OBJECT, KEY>::m_curCount].m_obj;

#ifdef DG_HEAP_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}


#endif
