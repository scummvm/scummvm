/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	MEMORY_H
#define	MEMORY_H

namespace Sword2 {

struct MemBlock {
	int16 id;
	int16 uid;
	byte *ptr;
	uint32 size;
};

class MemoryManager {
private:
	Sword2Engine *_vm;

	MemBlock *_memBlocks;
	MemBlock **_memBlockIndex;
	int16 _numBlocks;

	uint32 _totAlloc;

	int16 *_idStack;
	int16 _idStackPtr;

	int16 findExactPointerInIndex(byte *ptr);
	int16 findPointerInIndex(byte *ptr);
	int16 findInsertionPointInIndex(byte *ptr);

public:
	MemoryManager(Sword2Engine *vm);
	~MemoryManager();

	int32 encodePtr(byte *ptr);
	byte *decodePtr(int32 n);

	byte *memAlloc(uint32 size, int16 uid);
	void memFree(byte *ptr);

	void memDisplay();
	void memStatusStr(char *buf);
};

} // End of namespace Sword2

#endif
