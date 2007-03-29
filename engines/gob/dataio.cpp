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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/dataio.h"
#include "gob/global.h"

namespace Gob {

DataIO::DataIO(GobEngine *vm) : _vm(vm) {
	for (int i = 0; i < MAX_DATA_FILES; i++) {
		_dataFiles[i] = 0;
		_numDataChunks[i] = 0;
		_dataFileHandles[i] = -1;
	}
	_packedSize = 0;
}

DataIO::~DataIO() {
	for (int i = 0; i < MAX_DATA_FILES; i++) {
		if (_dataFiles[i])
			file_getHandle(_dataFileHandles[i])->close();
		delete[] _dataFiles[i];
	}
}

int32 DataIO::unpackData(char *sourceBuf, char *destBuf) {
	uint32 realSize;
	uint32 counter;
	uint16 cmd;
	byte *src;
	byte *dest;
	byte *tmpBuf;
	int16 off;
	byte len;
	uint16 tmpIndex;

	tmpBuf = new byte[4114];
	assert(tmpBuf);

	counter = realSize = READ_LE_UINT32(sourceBuf);

	for (int i = 0; i < 4078; i++)
		tmpBuf[i] = 0x20;
	tmpIndex = 4078;

	src = (byte *) (sourceBuf + 4);
	dest = (byte *) destBuf;

	cmd = 0;
	while (1) {
		cmd >>= 1;
		if ((cmd & 0x0100) == 0) {
			cmd = *src | 0xFF00;
			src++;
		}
		if ((cmd & 1) != 0) { /* copy */
			*dest++ = *src;
			tmpBuf[tmpIndex] = *src;
			src++;
			tmpIndex++;
			tmpIndex %= 4096;
			counter--;
			if (counter == 0)
				break;
		} else { /* copy string */

			off = *src++;
			off |= (*src & 0xF0) << 4;
			len = (*src & 0x0F) + 3;
			src++;

			for (int i = 0; i < len; i++) {
				*dest++ = tmpBuf[(off + i) % 4096];
				counter--;
				if (counter == 0) {
					delete[] tmpBuf;
					return realSize;
				}
				tmpBuf[tmpIndex] = tmpBuf[(off + i) % 4096];
				tmpIndex++;
				tmpIndex %= 4096;
			}

		}
	}
	delete[] tmpBuf;
	return realSize;
}

Common::File *DataIO::file_getHandle(int16 handle) {
	return &_filesHandles[handle];
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
	int16 slot;
	struct ChunkDesc *dataDesc;

	for (int16 file = 0; file < MAX_DATA_FILES; file++) {
		if (_dataFiles[file] == 0)
			return -1;

		for (slot = 0; slot < MAX_SLOT_COUNT; slot++)
			if (_chunkPos[file * MAX_SLOT_COUNT + slot] == -1)
				break;

		if (slot == MAX_SLOT_COUNT)
			return -1;

		dataDesc = _dataFiles[file];
		for (int16 chunk = 0; chunk < _numDataChunks[file]; chunk++, dataDesc++) {
			if (scumm_stricmp(chunkName, dataDesc->chunkName) != 0)
				continue;

			_isCurrentSlot[file * MAX_SLOT_COUNT + slot] = false;
			_chunkSize[file * MAX_SLOT_COUNT + slot] = dataDesc->size;
			_chunkOffset[file * MAX_SLOT_COUNT + slot] = dataDesc->offset;
			_chunkPos[file * MAX_SLOT_COUNT + slot] = 0;
			return file * 10 + slot + 50;
		}
	}
	return -1;
}

char DataIO::freeChunk(int16 handle) {
	if ((handle >= 50) && (handle < 128)) {
		handle -= 50;
		_chunkPos[(handle / 10) * MAX_SLOT_COUNT + (handle % 10)] = -1;
		return 0;
	}
	return 1;
}

int32 DataIO::readChunk(int16 handle, char *buf, uint16 size) {
	int16 file;
	int16 slot;
	int16 i;
	int32 offset;

	if ((handle < 50) || (handle >= 128))
		return -2;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	if (!_isCurrentSlot[file * MAX_SLOT_COUNT + slot]) {
		for (i = 0; i < MAX_SLOT_COUNT; i++)
			_isCurrentSlot[file * MAX_SLOT_COUNT + i] = false;

		offset = _chunkOffset[file * MAX_SLOT_COUNT + slot] +
			_chunkPos[file * MAX_SLOT_COUNT + slot];

		debugC(7, kDebugFileIO, "seek: %d, %d",
				_chunkOffset[file * MAX_SLOT_COUNT + slot],
				_chunkPos[file * MAX_SLOT_COUNT + slot]);

		file_getHandle(_dataFileHandles[file])->seek(offset, SEEK_SET);
	}

	_isCurrentSlot[file * MAX_SLOT_COUNT + slot] = true;
	if ((_chunkPos[file * MAX_SLOT_COUNT + slot] + size) >
	    (_chunkSize[file * MAX_SLOT_COUNT + slot]))
		size = _chunkSize[file * MAX_SLOT_COUNT + slot] -
			_chunkPos[file * MAX_SLOT_COUNT + slot];

	file_getHandle(_dataFileHandles[file])->read(buf, size);
	_chunkPos[file * MAX_SLOT_COUNT + slot] += size;
	return size;
}

int16 DataIO::seekChunk(int16 handle, int32 pos, int16 from) {
	int16 file;
	int16 slot;

	if ((handle < 50) || (handle >= 128))
		return -1;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	_isCurrentSlot[file * MAX_SLOT_COUNT + slot] = false;
	if (from == SEEK_SET)
		_chunkPos[file * MAX_SLOT_COUNT + slot] = pos;
	else
		_chunkPos[file * MAX_SLOT_COUNT + slot] += pos;

	return _chunkPos[file * MAX_SLOT_COUNT + slot];
}

uint32 DataIO::getChunkPos(int16 handle) {
	int16 file;
	int16 slot;

	if ((handle < 50) || (handle >= 128))
		return 0xFFFFFFFF;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;

	return _chunkPos[file * MAX_SLOT_COUNT + slot];
}

int32 DataIO::getChunkSize(const char *chunkName) {
	int16 file;
	int16 chunk;
	struct ChunkDesc *dataDesc;
	int16 slot;
	int32 realSize;

	for (file = 0; file < MAX_DATA_FILES; file++) {
		if (_dataFiles[file] == 0)
			return -1;

		dataDesc = _dataFiles[file];
		for (chunk = 0; chunk < _numDataChunks[file]; chunk++, dataDesc++) {
			if (scumm_stricmp(chunkName, dataDesc->chunkName) != 0)
				continue;

			if (dataDesc->packed == 0) {
				_packedSize = -1;
				return dataDesc->size;
			}

			for (slot = 0; slot < MAX_SLOT_COUNT; slot++)
				_isCurrentSlot[slot] = false;

			file_getHandle(_dataFileHandles[file])->seek(dataDesc->offset, SEEK_SET);
			realSize = file_getHandle(_dataFileHandles[file])->readUint32LE();
			_packedSize = dataDesc->size;
			return realSize;
		}
	}
	return -1;
}

void DataIO::openDataFile(const char *src, bool itk) {
	ChunkDesc *dataDesc;
	char path[128];
	int16 file;

	strcpy(path, src);
	if (!strchr(path, '.'))
		strcat(path, ".stk");

	for (file = 0; file < MAX_DATA_FILES; file++)
		if (_dataFiles[file] == 0)
			break;

	if (file == MAX_DATA_FILES)
		error("openDataFile: Data file slots are full");

	_dataFileHandles[file] = file_open(path);

	if (_dataFileHandles[file] == -1)
		error("openDataFile: Can't open %s data file", path);

	_dataFileItk[file] = itk;
	_numDataChunks[file] = file_getHandle(_dataFileHandles[file])->readUint16LE();

	debugC(7, kDebugFileIO, "DataChunks: %d [for %s]", _numDataChunks[file], path);

	dataDesc = new ChunkDesc[_numDataChunks[file]];
	_dataFiles[file] = dataDesc;

	for (int i = 0; i < _numDataChunks[file]; i++) {
		file_getHandle(_dataFileHandles[file])->read(dataDesc[i].chunkName, 13);
		dataDesc[i].size = file_getHandle(_dataFileHandles[file])->readUint32LE();
		dataDesc[i].offset = file_getHandle(_dataFileHandles[file])->readUint32LE();
		dataDesc[i].packed = file_getHandle(_dataFileHandles[file])->readByte();
	}

	for (int i = 0; i < _numDataChunks[file]; i++)
		debugC(7, kDebugFileIO, "%d: %s %d", i, dataDesc[i].chunkName, dataDesc[i].size);

	for (int i = 0; i < MAX_SLOT_COUNT; i++)
		_chunkPos[file * MAX_SLOT_COUNT + i] = -1;
}

void DataIO::closeDataFile(bool itk) {
	for (int file = MAX_DATA_FILES - 1; file >= 0; file--) {
		if (_dataFiles[file] && (_dataFileItk[file] == itk)) {
			delete[] _dataFiles[file];
			_dataFiles[file] = 0;
			file_getHandle(_dataFileHandles[file])->close();
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
	if ((_packedSize == -1) || (realSize == -1))
		return 0;

	chunk = getChunk(name);
	if (chunk == -1)
		return 0;

	unpackBuf = new char[realSize];
	assert(unpackBuf);

	packBuf = new char[_packedSize];
	assert(packBuf);

	sizeLeft = _packedSize;
	ptr = packBuf;
	while (sizeLeft > 0x4000) {
		readChunk(chunk, ptr, 0x4000);
		sizeLeft -= 0x4000;
		ptr += 0x4000;
	}
	readChunk(chunk, ptr, sizeLeft);
	freeChunk(chunk);
	unpackData(packBuf, unpackBuf);

	delete[] packBuf;
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

int32 DataIO::readData(int16 handle, char *buf, uint16 size) {
	int32 res;

	res = readChunk(handle, buf, size);
	if (res >= 0)
		return res;

	return file_getHandle(handle)->read(buf, size);
}

byte DataIO::readByte(int16 handle) {
	char buf;

	readData(handle, &buf, 1);
	return ((byte) buf);
}

uint16 DataIO::readUint16(int16 handle) {
	char buf[2];

	readData(handle, buf, 2);
	return READ_LE_UINT16(buf);
}

uint32 DataIO::readUint32(int16 handle) {
	char buf[4];

	readData(handle, buf, 4);
	return READ_LE_UINT32(buf);
}

int32 DataIO::writeData(int16 handle, char *buf, uint16 size) {
	return file_getHandle(handle)->write(buf, size);
}

void DataIO::seekData(int16 handle, int32 pos, int16 from) {
	int32 resPos;

	resPos = seekChunk(handle, pos, from);
	if (resPos != -1)
		return;

	file_getHandle(handle)->seek(pos, from);
}

uint32 DataIO::getPos(int16 handle) {
	uint32 resPos;

	resPos = getChunkPos(handle);
	if (resPos != 0xFFFFFFFF)
		return resPos;

	return file_getHandle(handle)->pos();
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
	if (data)
		return data;

	size = getDataSize(path);
	data = new char[size];
	assert(data);

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

} // End of namespace Gob
