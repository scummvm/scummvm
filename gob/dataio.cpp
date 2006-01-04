/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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

DataIO::DataIO(GobEngine *vm) : _vm(vm) {
}

Common::File *DataIO::file_getHandle(int16 handle) {
	return &_vm->_global->_filesHandles[handle];
}

int16 DataIO::file_open(const char *path, Common::File::AccessMode mode) {
	int16 i;

	for (i = 0; i < MAX_FILES; i++) {
		if (!file_getHandle(i)->isOpen())
			break;
	}
	if (i == MAX_FILES)
		return -1;

	file_getHandle(i)->open(path, mode);

	if (file_getHandle(i)->isOpen())
		return i;

	return -1;
}

int16 DataIO::getChunk(const char *chunkName) {
	int16 file;
	int16 slot;
	int16 chunk;
	struct ChunkDesc *dataDesc;

	for (file = 0; file < MAX_DATA_FILES; file++) {
		if (_vm->_global->_dataFiles[file] == 0)
			return -1;

		for (slot = 0; slot < MAX_SLOT_COUNT; slot++)
			if (_vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot] == -1)
				break;

		if (slot == MAX_SLOT_COUNT)
			return -1;

		dataDesc = _vm->_global->_dataFiles[file];
		for (chunk = 0; chunk < _vm->_global->_numDataChunks[file];
		    chunk++, dataDesc++) {
			if (scumm_stricmp(chunkName, dataDesc->chunkName) != 0)
				continue;

			_vm->_global->_isCurrentSlot[file * MAX_SLOT_COUNT + slot] = 0;
			_vm->_global->_chunkSize[file * MAX_SLOT_COUNT + slot] =
			    dataDesc->size;
			_vm->_global->_chunkOffset[file * MAX_SLOT_COUNT + slot] =
			    dataDesc->offset;
			_vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot] = 0;
			return file * 10 + slot + 50;
		}
	}
	return -1;
}

char DataIO::freeChunk(int16 handle) {
	if (handle >= 50 && handle < 100) {
		handle -= 50;
		_vm->_global->_chunkPos[(handle / 10) * MAX_SLOT_COUNT + (handle % 10)] = -1;
		return 0;
	}
	return 1;
}

int32 DataIO::readChunk(int16 handle, char *buf, int16 size) {
	int16 file;
	int16 slot;
	int16 i;
	int32 offset;

	if (handle < 50 || handle >= 100)
		return -2;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	if (_vm->_global->_isCurrentSlot[file * MAX_SLOT_COUNT + slot] == 0) {
		for (i = 0; i < MAX_SLOT_COUNT; i++)
			_vm->_global->_isCurrentSlot[file * MAX_SLOT_COUNT + i] = 0;

		offset =
		    _vm->_global->_chunkOffset[file * MAX_SLOT_COUNT + slot] +
		    _vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot];
		debug(7, "seek: %ld, %ld", _vm->_global->_chunkOffset[file * MAX_SLOT_COUNT + slot], _vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot]);
		file_getHandle(_vm->_global->_dataFileHandles[file])->seek(offset, SEEK_SET);
	}

	_vm->_global->_isCurrentSlot[file * MAX_SLOT_COUNT + slot] = 1;
	if (_vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot] + size >
	    _vm->_global->_chunkSize[file * MAX_SLOT_COUNT + slot])
		size =
		    _vm->_global->_chunkSize[file * MAX_SLOT_COUNT + slot] -
		    _vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot];

	file_getHandle(_vm->_global->_dataFileHandles[file])->read(buf, size);
	_vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot] += size;
	return size;
}

int16 DataIO::seekChunk(int16 handle, int32 pos, int16 from) {
	int16 file;
	int16 slot;

	if (handle < 50 || handle >= 100)
		return -1;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	_vm->_global->_isCurrentSlot[file * MAX_SLOT_COUNT + slot] = 0;
	if (from == SEEK_SET)
		_vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot] = pos;
	else
		_vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot] += pos;

	return _vm->_global->_chunkPos[file * MAX_SLOT_COUNT + slot];
}

int32 DataIO::getChunkSize(const char *chunkName) {
	int16 file;
	int16 chunk;
	struct ChunkDesc *dataDesc;
	int16 slot;
	int32 realSize;

	for (file = 0; file < MAX_DATA_FILES; file++) {
		if (_vm->_global->_dataFiles[file] == 0)
			return -1;

		dataDesc = _vm->_global->_dataFiles[file];
		for (chunk = 0; chunk < _vm->_global->_numDataChunks[file];
		    chunk++, dataDesc++) {
			if (scumm_stricmp(chunkName, dataDesc->chunkName) != 0)
				continue;

			if (dataDesc->packed == 0) {
				_vm->_global->_packedSize = -1;
				return dataDesc->size;
			}

			for (slot = 0; slot < MAX_SLOT_COUNT; slot++)
				_vm->_global->_isCurrentSlot[slot] = 0;

			file_getHandle(_vm->_global->_dataFileHandles[file])->seek(dataDesc->offset, SEEK_SET);
			realSize = file_getHandle(_vm->_global->_dataFileHandles[file])->readUint32LE();
			_vm->_global->_packedSize = dataDesc->size;
			return realSize;
		}
	}
	return -1;
}

void DataIO::openDataFile(const char *src) {
	char path[128];
	int16 i;
	int16 file;
	struct ChunkDesc *dataDesc;

	strcpy(path, src);
	for (i = 0; path[i] != '.' && path[i] != 0; i++);
	if (path[i] == 0)
		strcat(path, ".stk");

	for (file = 0; file < MAX_DATA_FILES; file++)
		if (_vm->_global->_dataFiles[file] == 0)
			break;

	if (file == MAX_DATA_FILES)
		error("dataFileOpen: Data file slots are full\n");
	_vm->_global->_dataFileHandles[file] = file_open(path);

	if (_vm->_global->_dataFileHandles[file] == -1)
		error("dataFileOpen: Can't open %s data file\n", path);

	_vm->_global->_numDataChunks[file] = file_getHandle(_vm->_global->_dataFileHandles[file])->readUint16LE();

	debug(7, "DataChunks: %d [for %s]", _vm->_global->_numDataChunks[file], path);

	_vm->_global->_dataFiles[file] = dataDesc =
	    (struct ChunkDesc *)malloc(sizeof(struct ChunkDesc) *
	    _vm->_global->_numDataChunks[file]);

	for (i = 0; i < _vm->_global->_numDataChunks[file]; i++) {
		file_getHandle(_vm->_global->_dataFileHandles[file])->read(dataDesc[i].chunkName, 13);
		dataDesc[i].size = file_getHandle(_vm->_global->_dataFileHandles[file])->readUint32LE();
		dataDesc[i].offset = file_getHandle(_vm->_global->_dataFileHandles[file])->readUint32LE();
		dataDesc[i].packed = file_getHandle(_vm->_global->_dataFileHandles[file])->readByte();
	}

	for (i = 0; i < _vm->_global->_numDataChunks[file]; i++)
		debug(7, "%d: %s %d", i, dataDesc[i].chunkName, dataDesc[i].size);

	for (i = 0; i < MAX_SLOT_COUNT; i++)
		_vm->_global->_chunkPos[file * MAX_SLOT_COUNT + i] = -1;

}

void DataIO::closeDataFile() {
	int16 file;
	for (file = MAX_DATA_FILES - 1; file >= 0; file--) {
		if (_vm->_global->_dataFiles[file] != 0) {
			free(_vm->_global->_dataFiles[file]);
			_vm->_global->_dataFiles[file] = 0;
			file_getHandle(_vm->_global->_dataFileHandles[file])->close();
			return;
		}
	}
}

char *DataIO::getUnpackedData(const char *name) {
	int32 realSize;
	int16 chunk;
	char *unpackBuf;
	char *packBuf;
	char *ptr;
	int32 sizeLeft;

	realSize = getChunkSize(name);
	if (_vm->_global->_packedSize == -1 || realSize == -1)
		return 0;

	chunk = getChunk(name);
	if (chunk == -1)
		return 0;

	unpackBuf = (char *)malloc(realSize);
	if (unpackBuf == 0)
		return 0;

	packBuf = (char *)malloc(_vm->_global->_packedSize);
	if (packBuf == 0) {
		free(unpackBuf);
		return 0;
	}

	sizeLeft = _vm->_global->_packedSize;
	ptr = packBuf;
	while (sizeLeft > 0x4000) {
		readChunk(chunk, ptr, 0x4000);
		sizeLeft -= 0x4000;
		ptr += 0x4000;
	}
	readChunk(chunk, ptr, sizeLeft);
	freeChunk(chunk);
	_vm->_pack->unpackData(packBuf, unpackBuf);
	free(packBuf);
	return unpackBuf;
}

void DataIO::closeData(int16 handle) {
	if (freeChunk(handle) != 0)
		file_getHandle(handle)->close();
}

int16 DataIO::openData(const char *path, Common::File::AccessMode mode) {
	int16 handle;

	if (mode != Common::File::kFileReadMode)
		return file_open(path, mode);

	handle = getChunk(path);
	if (handle >= 0)
		return handle;

	return file_open(path, mode);
}

int32 DataIO::readData(int16 handle, char *buf, int16 size) {
	int32 res;

	res = readChunk(handle, buf, size);
	if (res >= 0)
		return res;

	return file_getHandle(handle)->read(buf, size);
}

void DataIO::seekData(int16 handle, int32 pos, int16 from) {
	int32 resPos;

	resPos = seekChunk(handle, pos, from);
	if (resPos != -1)
		return;

	file_getHandle(handle)->seek(pos, from);
}

int32 DataIO::getDataSize(const char *name) {
	char buf[128];
	int32 chunkSz;
	Common::File file;

	strcpy(buf, name);
	chunkSz = getChunkSize(buf);
	if (chunkSz >= 0)
		return chunkSz;

	if (!file.open(buf))
		error("getDataSize: Can't find data(%s)", name);

	chunkSz = file.size();
	file.close();

	return chunkSz;
}

char *DataIO::getData(const char *path) {
	char *data;
	char *ptr;
	int32 size;
	int16 handle;

	data = getUnpackedData(path);
	if (data != 0)
		return data;

	size = getDataSize(path);
	data = (char *)malloc(size);
	if (data == 0)
		return 0;

	handle = openData(path);

	ptr = data;
	while (size > 0x4000) {
		readData(handle, ptr, 0x4000);
		size -= 0x4000;
		ptr += 0x4000;
	}
	readData(handle, ptr, size);
	closeData(handle);
	return data;
}

}				// End of namespace Gob
