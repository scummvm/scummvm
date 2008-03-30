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

#include "common/memorypool.h"
#include "common/util.h"

namespace Common {

static const size_t CHUNK_PAGE_SIZE = 32;

void* MemoryPool::allocPage() {
	void* result = ::malloc(CHUNK_PAGE_SIZE * _chunkSize);
	_pages.push_back(result);
	void* current = result;
	for (size_t i = 1; i < CHUNK_PAGE_SIZE; ++i) {
		void* next    = ((char*)current + _chunkSize);
		*(void**)current = next;

		current = next;
	}
	*(void**)current = NULL;
	return result;
}

MemoryPool::MemoryPool(size_t chunkSize) {
	// You must at least fit the pointer in the node (technically unneeded considering the next rounding statement)
	_chunkSize = MAX(chunkSize, sizeof(void*));
	// There might be an alignment problem on some platforms when trying to load a void* on a non natural boundary
	// so we round to the next sizeof(void*)
	_chunkSize = (_chunkSize + sizeof(void*) - 1) & (~(sizeof(void*) - 1));

	_next = NULL;
}

MemoryPool::~MemoryPool() {
	for (size_t i = 0; i<_pages.size(); ++i)
	::free(_pages[i]);
}

void* MemoryPool::malloc() {
#if 1
	if (!_next)
		_next = allocPage();

	void* result = _next;
	_next = *(void**)result;
	return result;
#else
	return ::malloc(_chunkSize);
#endif
}

void MemoryPool::free(void* ptr) {
#if 1
	*(void**)ptr = _next;
	_next = ptr;
#else
	::free(ptr);
#endif
}

// Technically not compliant C++ to compare unrelated pointers. In practice...
bool MemoryPool::isPointerInPage(void* ptr, void* page) {
	return (ptr >= page) && (ptr < (char*)page + CHUNK_PAGE_SIZE * _chunkSize);
}

void MemoryPool::freeUnusedPages() {
	//std::sort(_pages.begin(), _pages.end());
	Array<size_t> numberOfFreeChunksPerPage;
	numberOfFreeChunksPerPage.resize(_pages.size());
	for (size_t i = 0; i < numberOfFreeChunksPerPage.size(); ++i) {
		numberOfFreeChunksPerPage[i] = 0;
	}

	void* iterator = _next;
	while (iterator) {
		// This should be a binary search
		for (size_t i = 0; i < _pages.size(); ++i) {
			if (isPointerInPage(iterator, _pages[i])) {
				++numberOfFreeChunksPerPage[i];
				break;
			}
		}
		iterator = *(void**)iterator;
	}

	size_t freedPagesCount = 0;
	for (size_t i = 0; i < _pages.size(); ++i) {
		if (numberOfFreeChunksPerPage[i] == CHUNK_PAGE_SIZE) {
			::free(_pages[i]);
			_pages[i] = NULL; // TODO : Remove NULL values
			++freedPagesCount;
		}
	}

	//printf("%d freed pages\n", freedPagesCount); 
}

}	// End of namespace Common
