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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/dataio.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/util.h"

namespace Gob {

DataStream::DataStream(DataIO &io, int16 handle, uint32 dSize, bool dispose) {
	_io = &io;
	_handle = handle;
	_size = dSize;
	_dispose = dispose;

	_data = 0;
	_stream = 0;
}

DataStream::DataStream(byte *buf, uint32 dSize, bool dispose) {
	_data = buf;
	_size = dSize;
	_stream = new Common::MemoryReadStream(_data, _size);
	_dispose = dispose;

	_io = 0;
	_handle = -1;
}

DataStream::~DataStream() {
	delete _stream;

	if (_dispose) {
		delete[] _data;
		if ((_handle >= 0) && _io)
			_io->closeData(_handle);
	}
}

int32 DataStream::pos() const {
	if (_stream)
		return _stream->pos();

	int32 resPos = _io->getChunkPos(_handle);
	if (resPos != -1)
		return resPos;

	return _io->file_getHandle(_handle)->pos();
}

int32 DataStream::size() const {
	if (_stream)
		return _stream->size();

	return _size;
}

bool DataStream::seek(int32 offset, int whence) {
	if (_stream)
		return _stream->seek(offset, whence);
	else if ((_handle < 50) || (_handle >= 128))
		return _io->file_getHandle(_handle)->seek(offset, whence);
	else {
		_io->seekChunk(_handle, offset, whence);
		return true;
	}
}

bool DataStream::eos() const {
	if (_stream)
		return _stream->eos();

	return pos() >= size(); // FIXME (eos definition change)
}

uint32 DataStream::read(void *dataPtr, uint32 dataSize) {
	if (_stream)
		return _stream->read(dataPtr, dataSize);

	if ((_handle < 50) || (_handle >= 128))
		return _io->file_getHandle(_handle)->read((byte *) dataPtr, dataSize);

	byte *data = (byte *) dataPtr;
	uint32 haveRead = 0;
	while (dataSize > 0x3FFF) {
		_io->readChunk(_handle, (byte *) data, 0x3FFF);
		dataSize -= 0x3FFF;
		data += 0x3FFF;
		haveRead += 0x3FFF;
	}
	_io->readChunk(_handle, (byte *) data, dataSize);

	return haveRead + dataSize;
}

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

int32 DataIO::unpackData(byte *src, byte *dest) {
	uint32 realSize;
	uint32 counter;
	uint16 cmd;
	byte *tmpBuf;
	int16 off;
	byte len;
	uint16 tmpIndex;

	tmpBuf = new byte[4114];
	assert(tmpBuf);

	counter = realSize = READ_LE_UINT32(src);

	for (int i = 0; i < 4078; i++)
		tmpBuf[i] = 0x20;
	tmpIndex = 4078;

	src += 4;

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

const Common::File *DataIO::file_getHandle(int16 handle) const {
	return &_filesHandles[handle];
}

int16 DataIO::file_open(const char *path) {
	int16 i;

	for (i = 0; i < MAX_FILES; i++) {
		if (!file_getHandle(i)->isOpen())
			break;
	}
	if (i == MAX_FILES)
		return -1;

	if (file_getHandle(i)->open(path))
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

		if (slot == MAX_SLOT_COUNT) {
			warning("Chunk slots full");
			return -1;
		}

		dataDesc = _dataFiles[file];
		for (uint16 chunk = 0; chunk < _numDataChunks[file]; chunk++, dataDesc++) {
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

int32 DataIO::readChunk(int16 handle, byte *buf, uint16 size) {
	int16 file;
	int16 slot;
	int16 i;
	int32 offset;

	if ((handle < 50) || (handle >= 128))
		return -2;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	int index = file * MAX_SLOT_COUNT + slot;

	_chunkPos[index] = CLIP<int32>(_chunkPos[index], 0, _chunkSize[index]);

	if (!_isCurrentSlot[index]) {
		for (i = 0; i < MAX_SLOT_COUNT; i++)
			_isCurrentSlot[file * MAX_SLOT_COUNT + i] = false;

		offset = _chunkOffset[index] + _chunkPos[index];

		debugC(7, kDebugFileIO, "seek: %d, %d", _chunkOffset[index], _chunkPos[index]);

		file_getHandle(_dataFileHandles[file])->seek(offset, SEEK_SET);
	}

	_isCurrentSlot[index] = true;
	if ((_chunkPos[index] + size) > (_chunkSize[index]))
		size = _chunkSize[index] - _chunkPos[index];

	file_getHandle(_dataFileHandles[file])->read(buf, size);
	_chunkPos[index] += size;
	return size;
}

int16 DataIO::seekChunk(int16 handle, int32 pos, int16 from) {
	int16 file;
	int16 slot;

	if ((handle < 50) || (handle >= 128))
		return -1;

	file = (handle - 50) / 10;
	slot = (handle - 50) % 10;
	int index = file * MAX_SLOT_COUNT + slot;

	_isCurrentSlot[index] = false;
	if (from == SEEK_SET)
		_chunkPos[index] = pos;
	else if (from == SEEK_CUR)
		_chunkPos[index] += pos;
	else if (from == SEEK_END)
		_chunkPos[index] = _chunkSize[index] - pos;

	return _chunkPos[index];
}

uint32 DataIO::getChunkPos(int16 handle) const {
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
	struct ChunkDesc *dataDesc;
	int16 slot;
	int32 realSize;

	for (file = 0; file < MAX_DATA_FILES; file++) {
		if (_dataFiles[file] == 0)
			return -1;

		dataDesc = _dataFiles[file];
		for (uint16 chunk = 0; chunk < _numDataChunks[file]; chunk++, dataDesc++) {
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
	char *fakeTotPtr;

	strncpy0(path, src, 127);
	if (!strchr(path, '.')) {
		path[123] = 0;
		strcat(path, ".stk");
	}

	for (file = 0; file < MAX_DATA_FILES; file++)
		if (_dataFiles[file] == 0)
			break;

	if (file == MAX_DATA_FILES)
		error("DataIO::openDataFile(): Data file slots are full");

	_dataFileHandles[file] = file_open(path);

	if (_dataFileHandles[file] == -1)
		error("DataIO::openDataFile(): Can't open data file \"%s\"", path);

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

		// Replacing cyrillic characters
		Util::replaceChar(dataDesc[i].chunkName, (char) 0x85, 'E');
		Util::replaceChar(dataDesc[i].chunkName, (char) 0x8A, 'K');
		Util::replaceChar(dataDesc[i].chunkName, (char) 0x8E, 'O');
		Util::replaceChar(dataDesc[i].chunkName, (char) 0x91, 'C');
		Util::replaceChar(dataDesc[i].chunkName, (char) 0x92, 'T');

		// Geisha use 0ot files, which are compressed TOT files without the packed byte set.
		fakeTotPtr = strstr(dataDesc[i].chunkName, "0OT");
		if (fakeTotPtr != 0) {
			strncpy(fakeTotPtr, "TOT", 3);
			dataDesc[i].packed = 1;
		}
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

byte *DataIO::getUnpackedData(const char *name) {
	int32 realSize;
	int16 chunk;
	byte *unpackBuf;
	byte *packBuf;
	byte *ptr;
	int32 sizeLeft;

	realSize = getChunkSize(name);
	if ((_packedSize == -1) || (realSize == -1))
		return 0;

	chunk = getChunk(name);
	if (chunk == -1)
		return 0;

	unpackBuf = new byte[realSize];
	assert(unpackBuf);

	packBuf = new byte[_packedSize];
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

int16 DataIO::openData(const char *path) {
	int16 handle;

	handle = getChunk(path);
	if (handle >= 0)
		return handle;

	return file_open(path);
}

bool DataIO::existData(const char *path) {
	if (!path || (path[0] == '\0'))
		return false;

	int16 handle = openData(path);

	if (handle < 0)
		return false;

	closeData(handle);
	return true;
}

DataStream *DataIO::openAsStream(int16 handle, bool dispose) {
	uint32 curPos = getPos(handle);
	seekData(handle, 0, SEEK_END);
	uint32 size = getPos(handle);
	seekData(handle, curPos, SEEK_SET);

	return new DataStream(*this, handle, size, dispose);
}

uint32 DataIO::getPos(int16 handle) {
	uint32 resPos;

	resPos = getChunkPos(handle);
	if (resPos != 0xFFFFFFFF)
		return resPos;

	return file_getHandle(handle)->pos();
}

void DataIO::seekData(int16 handle, int32 pos, int16 from) {
	int32 resPos;

	resPos = seekChunk(handle, pos, from);
	if (resPos != -1)
		return;

	file_getHandle(handle)->seek(pos, from);
}

int32 DataIO::readData(int16 handle, byte *buf, uint16 size) {
	int32 res;

	res = readChunk(handle, buf, size);
	if (res >= 0)
		return res;

	return file_getHandle(handle)->read(buf, size);
}

int32 DataIO::getDataSize(const char *name) {
	char buf[128];
	int32 chunkSz;
	Common::File file;

	strncpy0(buf, name, 127);

	chunkSz = getChunkSize(buf);
	if (chunkSz >= 0)
		return chunkSz;

	if (!file.open(buf))
		error("DataIO::getDataSize(): Can't find data \"%s\"", name);

	chunkSz = file.size();
	file.close();

	return chunkSz;
}

byte *DataIO::getData(const char *path) {
	byte *data;
	byte *ptr;
	int32 size;
	int16 handle;

	data = getUnpackedData(path);
	if (data)
		return data;

	size = getDataSize(path);
	data = new byte[size];
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

DataStream *DataIO::getDataStream(const char *path) {
	if (!existData(path))
		return 0;

	uint32 size = getDataSize(path);
	byte *data = getData(path);

	return new DataStream(data, size);
}

} // End of namespace Gob
