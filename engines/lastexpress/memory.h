/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LASTEXPRESS_MEMORY_H
#define LASTEXPRESS_MEMORY_H

#include "common/scummsys.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

class LastExpressEngine;

struct MemPage {
	void *memPagePtr;
	uint32 memPageSize;
	int32 character;
	char pageName[81];
	bool allocatedFlag;

	MemPage() {
		memPagePtr = nullptr;
		memPageSize = 0;
		character = 0;
		memset(pageName, 0, sizeof(pageName));
		allocatedFlag = false;
	}

	void copyFrom(MemPage other) {
		memPagePtr = other.memPagePtr;
		memPageSize = other.memPageSize;
		character = other.character;
		memcpy(pageName, other.pageName, sizeof(pageName));
		allocatedFlag = other.allocatedFlag;
	}
};

enum MemoryFlags {
	kMemoryFlagInit    = 0x1,
	kMemoryFlagFXFree  = 0x2,
	kMemoryFlagSeqFree = 0x4,
};

class MemoryManager {
public:
	MemoryManager(LastExpressEngine *engine);
	~MemoryManager() {}

	void initMem();
	void *allocMem(uint32 size, const char *name, int character);
	void freeMem(void *data);
	void releaseMemory();
	void freeFX();
	void lockFX();
	void lockSeqMem(uint32 size);
	void freeSeqMem();
	Seq *copySeq(Seq *sequenceToCopy);

private:
	LastExpressEngine *_engine = nullptr;

	int32 _nisSeqMemFlag = 0;
	MemPage _memoryPages[128];
	int _numAllocatedMemPages = 1;
	int32 _nisSeqMemAvailForLocking = 0;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_MEMORY_H
