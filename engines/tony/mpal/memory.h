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
 *
 */

#ifndef TONY_MPAL_MEMORY
#define TONY_MPAL_MEMORY

#include "common/scummsys.h"
#include "common/list.h"

namespace Tony {

namespace MPAL {

typedef void *HANDLE;
typedef HANDLE HGLOBAL;

class MemoryItem {
protected:
    byte *_buffer;
	uint32 _size;
public:
	MemoryItem(uint32 size);
	virtual ~MemoryItem();
	
	uint32 Size() { return _size; }
	void *DataPointer() { return (void *)_buffer; }
	bool IsValid() { return _buffer != NULL; }

    // Casting for access to data
    operator void *();
};

class MemoryManager {
private:
	Common::List<MemoryItem *> _memoryBlocks;
public:
	MemoryManager();
	virtual ~MemoryManager();

	MemoryItem &allocate(uint32 size);
	HGLOBAL alloc(uint32 size);
	MemoryItem &getItem(HGLOBAL handle);
	MemoryItem &operator[](HGLOBAL handle);
	void erase(MemoryItem *item);
	void erase(HGLOBAL handle);

	uint32 getSize(HANDLE handle);
};

// defines
#define GlobalAlloc(flags, size)	_vm->_memoryManager.alloc(size)
#define GlobalAllocate(size)		_vm->_memoryManager.allocate(size)
#define GlobalFree(handle)			_vm->_memoryManager.erase(handle)
#define GlobalLock(handle)			(_vm->_memoryManager.getItem(handle).DataPointer())
#define GlobalUnlock(handle)		{}
#define GlobalSize(handle)			(_vm->_memoryManager.getItem(handle).Size())

#define GMEM_FIXED 1
#define GMEM_MOVEABLE 2
#define GMEM_ZEROINIT 4

} // end of namespace MPAL

} // end of namespace Tony

#endif
