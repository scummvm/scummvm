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

#ifndef COMMON_MEMORYPOOL_H
#define COMMON_MEMORYPOOL_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Common {

class MemoryPool {
protected:
	MemoryPool(const MemoryPool&);
	MemoryPool& operator=(const MemoryPool&);
	
	struct Page {
		void *start;
		size_t numChunks;
	};

	size_t			_chunkSize;
	Array<Page>		_pages;
	void			*_next;
	size_t			_chunksPerPage;

	void	allocPage();
	void	addPageToPool(const Page &page);
	bool	isPointerInPage(void *ptr, const Page &page);

public:
	MemoryPool(size_t chunkSize);
	~MemoryPool();

	void 	*malloc();
	void 	free(void *ptr);

	void	freeUnusedPages();
};

template<size_t CHUNK_SIZE, size_t NUM_INTERNAL_CHUNKS = 32>
class FixedSizeMemoryPool : public MemoryPool {
private:
	enum {
		REAL_CHUNK_SIZE = (CHUNK_SIZE + sizeof(void*) - 1) & (~(sizeof(void*) - 1))
	};

	byte	_storage[NUM_INTERNAL_CHUNKS * REAL_CHUNK_SIZE];
public:
	FixedSizeMemoryPool() : MemoryPool(CHUNK_SIZE) {
		assert(REAL_CHUNK_SIZE == _chunkSize);
		// Insert some static storage
		Page internalPage = { _storage, NUM_INTERNAL_CHUNKS };
		addPageToPool(internalPage);
	}
};

template<size_t CHUNK_SIZE>
class FixedSizeMemoryPool<CHUNK_SIZE,0> : public MemoryPool {
public:
	FixedSizeMemoryPool() : MemoryPool(CHUNK_SIZE) {}
};

}	// End of namespace Common

#endif
