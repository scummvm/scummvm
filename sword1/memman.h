/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef MEMMAN_H
#define MEMMAN_H

#include "scummsys.h"

struct BsMemHandle {
	void *data;
	uint32 size;
	uint32 refCount;
	uint16 cond;
	BsMemHandle *next, *prev;
};
// mem conditions:
#define MEM_FREED		0
#define MEM_CAN_FREE	1
#define MEM_DONT_FREE	2

#define MAX_ALLOC (6*1024*1024) // max amount of mem we want to alloc().

class MemMan {
public:
	MemMan(void);
	~MemMan(void);
	void alloc(BsMemHandle *bsMem, uint32 pSize, uint16 pCond = MEM_DONT_FREE);
	void setCondition(BsMemHandle *bsMem, uint16 pCond);
	void freeNow(BsMemHandle *bsMem);
	void initHandle(BsMemHandle *bsMem);
	void flush(void);
private:
	void addToFreeList(BsMemHandle *bsMem);
	void removeFromFreeList(BsMemHandle *bsMem);
	void checkMemoryUsage(void);
	uint32 _alloced;  //currently allocated memory
	BsMemHandle *_memListFree;
	BsMemHandle *_memListFreeEnd;
};

#endif //MEMMAN_H
