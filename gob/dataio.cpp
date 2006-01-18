/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/pack.h"

namespace Gob {

int16 file_write(int16 handle, char *buf, int16 size) {
	return filesHandles[handle].write(buf, size);
}

int16 file_open(const char *path, Common::File::AccessMode mode) {
	int16 i;

	for (i = 0; i < MAX_FILES; i++) {
		if (!filesHandles[i].isOpen())
			break;
	}
	if (i == MAX_FILES)
		return -1;

	filesHandles[i].open(path, mode);

	if (filesHandles[i].isOpen())
		return i;

	return -1;
}

Common::File *file_getHandle(int16 handle) {
	return &filesHandles[handle];
}

int16 data_getChunk(const char *chunkName) {
	int16 file;
	int16 slot;
	int16 chunk;
	struct ChunkDesc *dataDesc;

	for (file = 0; file < MAX_DATA_FILES; file++) {
		if (dataFiles[file] == 0)
			return -1;

		for (slot = 0; slot < MAX_SLOT_COUNT; slot++)
			if (chunkPos[file * MAX_SLOT_COUNT + slot] == -1)
				break;

		if (slot == MAX_SLOT_COUNT)
			return -1;

		dataDesc = dataFiles[file];
		for (chunk = 0; chunk < numDataChunks[file];
		    chunk++, dataDesc++) {
			if (scumm_stricmp(chunkName, dataDesc->chunkName) != 0)
				continue;

			isCurrentSlot[file * MAX_SLOT_COUNT + slot] = 0;
			chunkSize[file * MAX_SLOT_COUNT + slot] =
			    dataDesc->size;
			chunkOffset[file * MAX_SLOT_COUNT + slot] =
			    dataDesc->offset;
			chunkPos[file * MAX_SLOT_COUNT + slot] = 0;
			return file * 10 + slot + 50;
		}
	}
	return -1;
}

char data_freeChunk(int16 handle) {
	if (handle >= 50 && handle < 100) {
		handle -= 50;
		chunkPos[(handle / 10) * MAX_SLOT_COUNT + (handle % 10)] = -1;
		return 0;
	}
	return 1;
}

int32 data_readChunk(int16 handle, char *buf, int16 size) {
	int16 file;
	int16 slot;
	int16 i;
	int32 offset;

	if (handle < 50 || handle >= 100)
		return -2;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	if (isCurrentSlot[file * MAX_SLOT_COUNT + slot] == 0) {
		for (i = 0; i < MAX_SLOT_COUNT; i++)
			isCurrentSlot[file * MAX_SLOT_COUNT + i] = 0;

		offset =
		    chunkOffset[file * MAX_SLOT_COUNT + slot] +
		    chunkPos[file * MAX_SLOT_COUNT + slot];
		debug(7, "seek: %ld, %ld", chunkOffset[file * MAX_SLOT_COUNT + slot], chunkPos[file * MAX_SLOT_COUNT + slot]);
		file_getHandle(dataFileHandles[file])->seek(offset, SEEK_SET);
	}

	isCurrentSlot[file * MAX_SLOT_COUNT + slot] = 1;
	if (chunkPos[file * MAX_SLOT_COUNT + slot] + size >
	    chunkSize[file * MAX_SLOT_COUNT + slot])
		size =
		    chunkSize[file * MAX_SLOT_COUNT + slot] -
		    chunkPos[file * MAX_SLOT_COUNT + slot];

	file_getHandle(dataFileHandles[file])->read(buf, size);
	chunkPos[file * MAX_SLOT_COUNT + slot] += size;
	return size;
}

int16 data_seekChunk(int16 handle, int32 pos, int16 from) {
	int16 file;
	int16 slot;

	if (handle < 50 || handle >= 100)
		return -1;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	isCurrentSlot[file * MAX_SLOT_COUNT + slot] = 0;
	if (from == SEEK_SET)
		chunkPos[file * MAX_SLOT_COUNT + slot] = pos;
	else
		chunkPos[file * MAX_SLOT_COUNT + slot] += pos;

	return chunkPos[file * MAX_SLOT_COUNT + slot];
}

int32 data_getChunkSize(const char *chunkName) {
	int16 file;
	int16 chunk;
	struct ChunkDesc *dataDesc;
	int16 slot;
	int32 realSize;

	for (file = 0; file < MAX_DATA_FILES; file++) {
		if (dataFiles[file] == 0)
			return -1;

		dataDesc = dataFiles[file];
		for (chunk = 0; chunk < numDataChunks[file];
		    chunk++, dataDesc++) {
			if (scumm_stricmp(chunkName, dataDesc->chunkName) != 0)
				continue;

			if (dataDesc->packed == 0) {
				packedSize = -1;
				return dataDesc->size;
			}

			for (slot = 0; slot < MAX_SLOT_COUNT; slot++)
				isCurrentSlot[slot] = 0;

			file_getHandle(dataFileHandles[file])->seek(dataDesc->offset, SEEK_SET);
			realSize = file_getHandle(dataFileHandles[file])->readUint32LE();
			packedSize = dataDesc->size;
			return realSize;
		}
	}
	return -1;
}

void data_openDataFile(const char *src) {
	char path[128];
	int16 i;
	int16 file;
	struct ChunkDesc *dataDesc;

	strcpy(path, src);
	for (i = 0; path[i] != '.' && path[i] != 0; i++);
	if (path[i] == 0)
		strcat(path, ".stk");

	for (file = 0; file < MAX_DATA_FILES; file++)
		if (dataFiles[file] == 0)
			break;

	if (file == MAX_DATA_FILES)
		error("data_dataFileOpen: Data file slots are full\n");
	dataFileHandles[file] = file_open(path);

	if (dataFileHandles[file] == -1)
		error("data_dataFileOpen: Can't open %s data file\n", path);

	numDataChunks[file] = file_getHandle(dataFileHandles[file])->readUint16LE();

	debug(7, "DataChunks: %d [for %s]", numDataChunks[file], path);

	dataFiles[file] = dataDesc =
	    (struct ChunkDesc *)malloc(sizeof(struct ChunkDesc) *
	    numDataChunks[file]);

	for (i = 0; i < numDataChunks[file]; i++) {
		file_getHandle(dataFileHandles[file])->read(dataDesc[i].chunkName, 13);
		dataDesc[i].size = file_getHandle(dataFileHandles[file])->readUint32LE();
		dataDesc[i].offset = file_getHandle(dataFileHandles[file])->readUint32LE();
		dataDesc[i].packed = file_getHandle(dataFileHandles[file])->readByte();
	}

	for (i = 0; i < numDataChunks[file]; i++)
		debug(7, "%d: %s %d", i, dataDesc[i].chunkName, dataDesc[i].size);

	for (i = 0; i < MAX_SLOT_COUNT; i++)
		chunkPos[file * MAX_SLOT_COUNT + i] = -1;

}

void data_closeDataFile() {
	int16 file;
	for (file = MAX_DATA_FILES - 1; file >= 0; file--) {
		if (dataFiles[file] != 0) {
			free(dataFiles[file]);
			dataFiles[file] = 0;
			file_getHandle(dataFileHandles[file])->close();
			return;
		}
	}
}

char *data_getUnpackedData(const char *name) {
	int32 realSize;
	int16 chunk;
	char *unpackBuf;
	char *packBuf;
	char *ptr;
	int32 sizeLeft;

	realSize = data_getChunkSize(name);
	if (packedSize == -1 || realSize == -1)
		return 0;

	chunk = data_getChunk(name);
	if (chunk == -1)
		return 0;

	unpackBuf = (char *)malloc(realSize);
	if (unpackBuf == 0)
		return 0;

	packBuf = (char *)malloc(packedSize);
	if (packBuf == 0) {
		free(unpackBuf);
		return 0;
	}

	sizeLeft = packedSize;
	ptr = packBuf;
	while (sizeLeft > 0x4000) {
		data_readChunk(chunk, ptr, 0x4000);
		sizeLeft -= 0x4000;
		ptr += 0x4000;
	}
	data_readChunk(chunk, ptr, sizeLeft);
	data_freeChunk(chunk);
	unpackData(packBuf, unpackBuf);
	free(packBuf);
	return unpackBuf;
}

void data_closeData(int16 handle) {
	if (data_freeChunk(handle) != 0)
		file_getHandle(handle)->close();
}

int16 data_openData(const char *path, Common::File::AccessMode mode) {
	int16 handle;

	if (mode != Common::File::kFileReadMode)
		return file_open(path, mode);

	handle = data_getChunk(path);
	if (handle >= 0)
		return handle;

	return file_open(path, mode);
}

int32 data_readData(int16 handle, char *buf, int16 size) {
	int32 res;

	res = data_readChunk(handle, buf, size);
	if (res >= 0)
		return res;

	return file_getHandle(handle)->read(buf, size);
}

void data_seekData(int16 handle, int32 pos, int16 from) {
	int32 resPos;

	resPos = data_seekChunk(handle, pos, from);
	if (resPos != -1)
		return;

	file_getHandle(handle)->seek(pos, from);
}

int32 data_getDataSize(const char *name) {
	char buf[128];
	int32 chunkSz;
	Common::File file;

	strcpy(buf, name);
	chunkSz = data_getChunkSize(buf);
	if (chunkSz >= 0)
		return chunkSz;

	if (!file.open(buf))
		error("data_getDataSize: Can't find data(%s)", name);

	chunkSz = file.size();
	file.close();

	return chunkSz;
}

char *data_getData(const char *path) {
	char *data;
	char *ptr;
	int32 size;
	int16 handle;

	data = data_getUnpackedData(path);
	if (data != 0)
		return data;

	size = data_getDataSize(path);
	data = (char *)malloc(size);
	if (data == 0)
		return 0;

	handle = data_openData(path);

	ptr = data;
	while (size > 0x4000) {
		data_readData(handle, ptr, 0x4000);
		size -= 0x4000;
		ptr += 0x4000;
	}
	data_readData(handle, ptr, size);
	data_closeData(handle);
	return data;
}

char *data_getSmallData(const char *path) {
	return data_getData(path);
}

}				// End of namespace Gob
