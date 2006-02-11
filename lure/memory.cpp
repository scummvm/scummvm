/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/memory.h"
#include "common/file.h"

namespace Lure {

MemoryBlock *Memory::allocate(uint32 size) {
	MemoryBlock *block = new MemoryBlock(size);
	return block;
}

MemoryBlock *Memory::duplicate(MemoryBlock *src) {
	MemoryBlock *block = new MemoryBlock(src);
	return block;
}

uint8 *Memory::alloc(uint32 size) {
	return (uint8 *) malloc(size);
}

void Memory::dealloc(uint8 *block) {
	free(block);
}

/*--------------------------------------------------------------------------*/

MemoryBlock::MemoryBlock(uint32 size1) {
	_data = (uint8 *) malloc(size1);
	if (!_data) error ("Failed allocating memory block");
	_size = size1;
}

MemoryBlock::MemoryBlock(MemoryBlock *src) {
	_size = src->size();
	_data = (uint8 *) malloc(_size);
	if (!_data) error ("Failed allocating memory block");
	memcpy(_data, src->data(), _size);
}

MemoryBlock::~MemoryBlock() {
	free(_data);
}

void MemoryBlock::empty() {
	::memset(_data, 0, _size);
}

void MemoryBlock::memorySet(int c, size_t startIndex, size_t num) {
	byte *p = _data + startIndex;
	::memset(p, c, num);
}

void MemoryBlock::copyFrom(MemoryBlock *src) {
	copyFrom(src, 0, 0, src->size());
}

void MemoryBlock::copyFrom(MemoryBlock *src, uint32 srcPos, uint32 destPos, uint32 srcLen) {
	if ((srcPos + srcLen > src->size()) || (destPos + srcLen > size()))
		error("Memory block overrun in block copy");

	uint8 *pDest = _data + destPos;
	uint8 *pSrc = src->data() + srcPos;
	memcpy(pDest, pSrc, srcLen);
}

void MemoryBlock::copyFrom(const byte *src, uint32 srcPos, uint32 destPos, uint32 srcLen) {
	byte *pDest = _data + destPos;
	const byte *pSrc = src + srcPos;
	memcpy(pDest, pSrc, srcLen);
}

void MemoryBlock::reallocate(uint32 size1) {
	_size = size1;
	_data = (byte *) realloc(_data, size1);
	if (!_data) error ("Failed reallocating memory block");
}

void MemoryBlock::saveToFile(const Common::String &filename) {
	Common::File *f = new Common::File();
	f->open(filename.c_str(), Common::File::kFileWriteMode);
	f->write(_data, _size);
	f->close();
	delete f;
}

} // end of namespace Lure
