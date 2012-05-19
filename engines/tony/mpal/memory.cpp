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

#include "common/algorithm.h"
#include "common/textconsole.h"
#include "tony/mpal/memory.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       MemoryItem methods
\****************************************************************************/

/**
 * Constructor
 * @param					Data sizee
 */
MemoryItem::MemoryItem(uint32 size) {
	_size = size;
	_buffer = (size == 0) ? NULL : new byte[size];
}

/**
 * Destructor
 */
MemoryItem::~MemoryItem() {
	delete[] _buffer;
}

/**
 * Returns a pointer to the resource
 */
MemoryItem::operator void *() {
	return DataPointer();
}

/****************************************************************************\
*       MemoryManager methods
\****************************************************************************/

MemoryManager::MemoryManager() {
}

/**
 * Destructor
 */
MemoryManager::~MemoryManager() {
	Common::List<MemoryItem *>::iterator i;
	for (i = _memoryBlocks.begin(); i != _memoryBlocks.end(); ++i) {
		MemoryItem *item = *i;
		delete item;
	}
}

/**
 * Allocates a new memory block
 * @returns					Returns a MemoryItem instance for the new block
 */
MemoryItem &MemoryManager::allocate(uint32 size, uint flags) {
	MemoryItem *newItem = new MemoryItem(size);
	if ((flags & GMEM_ZEROINIT) != 0) {
		byte *dataP = (byte *)newItem->DataPointer();
		Common::fill(dataP, dataP + size, 0);
	}

	_memoryBlocks.push_back(newItem);

	return *newItem;
}

/**
 * Allocates a new memory block and returns its data pointer
 * @returns					Data pointer to allocated block
 */
HGLOBAL MemoryManager::alloc(uint32 size, uint flags) {
	MemoryItem &newItem = allocate(size, flags);
	return (HGLOBAL)newItem.DataPointer();
}

/**
 * Returns a reference to the MemoryItem for a gien byte pointer
 * @param block				Byte pointer
 */
MemoryItem &MemoryManager::getItem(HGLOBAL handle) {
	Common::List<MemoryItem *>::iterator i;
	for (i = _memoryBlocks.begin(); i != _memoryBlocks.end(); ++i) {
		MemoryItem *item = *i;
		if (item->DataPointer() == handle)
			return *item;
	}

	error("Could not locate a memory block");
}

/**
 * Square bracketes operator 
 * @param block				Byte pointer
 */
MemoryItem &MemoryManager::operator[](HGLOBAL handle) {
	return getItem(handle);
}

/**
 * Returns a size of a memory block given its pointer
 */
uint32 MemoryManager::getSize(HGLOBAL handle) {
	MemoryItem &item = getItem(handle);
	return item.Size();
}

/**
 * Erases a given item
 */
void MemoryManager::erase(MemoryItem *item) {
	delete item;
	_memoryBlocks.remove(item);
}

/**
 * Erases a given item
 */
void MemoryManager::erase(HGLOBAL handle) {
	MemoryItem &item = getItem(handle);
	erase(&item);
}

/****************************************************************************\
*       Stand-alone methods
\****************************************************************************/

void CopyMemory(void *dst, const void *first, int size) {
	Common::copy((const byte *)first, (const byte *)first + size, (byte *)dst);
}


} // end of namespace MPAL

} // end of namespace Tony
