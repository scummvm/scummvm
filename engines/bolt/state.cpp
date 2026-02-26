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

#include "bolt/bolt.h"

namespace Bolt {

bool BoltEngine::initVRam(int16 poolSize) {
	g_vramRecordCount = 0;
	g_vramUsedBytes = 0;

	g_allocatedMemPool = (byte *)_xp->allocMem(poolSize);
	if (!g_allocatedMemPool) {
		g_curErrorCode = 1;
		return false;
	}

	g_allocatedMemPoolSize = poolSize;
	g_curErrorCode = 0;
	return true;
}

void BoltEngine::freeVRam() {
	if (g_allocatedMemPool) {
		_xp->freeMem(g_allocatedMemPool);
		g_allocatedMemPool = 0;
	}
}

bool BoltEngine::vLoad(void *dest, const char *name) {
	char localName[82];
	int16 recordOffset;

	strcpy_s(localName, name);
	uint32 nameLen = strlen(name);
	if (!(nameLen & 1)) {
		localName[nameLen] = ' ';
		nameLen++;
		localName[nameLen] = '\0';
	}

	if (!findRecord(localName, &recordOffset)) {
		g_curErrorCode = 3;
		return false;
	}

	int16 size = dataSize(recordOffset);
	byte *addr = dataAddress(recordOffset);

	memcpy(dest, addr, size);

	g_curErrorCode = 0;
	return true;
}

bool BoltEngine::vSave(void *src, uint16 srcSize, const char *name) {
	char localName[82];

	strcpy_s(localName, name);
	uint32 nameLen = strlen(name);
	if (!(nameLen & 1)) {
		localName[nameLen] = ' ';
		nameLen++;
		localName[nameLen] = '\0';
	}

	uint32 nameStorageLen = nameLen + 1;
	uint32 entrySize = nameStorageLen + srcSize + 4;

	vDelete(localName);

	if (entrySize + g_vramUsedBytes > g_allocatedMemPoolSize) {
		g_curErrorCode = 4;
		return 0;
	}

	byte *writePtr = &g_allocatedMemPool[g_vramUsedBytes];

	WRITE_UINT16(writePtr, (uint16)entrySize);
	WRITE_UINT16(writePtr + 2, (uint16)nameStorageLen);
	writePtr += 4;

	memcpy(writePtr, localName, nameStorageLen);
	writePtr += nameStorageLen;

	memcpy(writePtr, src, srcSize);

	g_vramUsedBytes += entrySize;
	g_vramRecordCount++;
	g_curErrorCode = 0;
	return true;
}

bool BoltEngine::vDelete(const char *name) {
	char localName[82];
	int16 recordOffset;

	strcpy_s(localName, name);
	uint32 nameLen = strlen(name);
	if (!(nameLen & 1)) {
		localName[nameLen] = ' ';
		nameLen++;
		localName[nameLen] = '\0';
	}

	g_curErrorCode = 3;

	if (g_vramRecordCount == 0)
		return 0;

	if (!findRecord(localName, &recordOffset))
		return 0;

	int16 entrySize = (int16)READ_UINT16(&g_allocatedMemPool[recordOffset]);

	if (recordOffset + entrySize < g_vramUsedBytes) {
		memmove(g_allocatedMemPool + recordOffset,
				g_allocatedMemPool + recordOffset + entrySize,
				g_vramUsedBytes - (recordOffset + entrySize));
	}

	g_vramUsedBytes -= entrySize;
	g_vramRecordCount--;
	g_curErrorCode = 0;
	return true;
}

byte *BoltEngine::dataAddress(int16 recordOffset) {
	byte *entry = &g_allocatedMemPool[recordOffset];

	int16 nameStorageLen = (int16)READ_UINT16(entry + 2);
	return &g_allocatedMemPool[recordOffset + 4 + nameStorageLen];
}

uint16 BoltEngine::dataSize(int16 recordOffset) {
	byte *entry = g_allocatedMemPool + recordOffset;

	int16 entrySize = (int16)READ_UINT16(entry);
	int16 nameStorageLen = (int16)READ_UINT16(entry + 2);
	return entrySize - 4 - nameStorageLen;
}

bool BoltEngine::findRecord(const char *name, int16 *outOffset) {
	int16 offset = 0;
	byte *ptr = g_allocatedMemPool;

	for (int16 i = 0; i < g_vramRecordCount; i++) {
		char *entryName = (char *)&ptr[4];
		if (strcmp(name, entryName) == 0) {
			*outOffset = offset;
			return true;
		}

		offset += (int16)READ_UINT16(ptr);
		ptr = &g_allocatedMemPool[offset];
	}

	return false;
}

} // End of namespace Bolt
