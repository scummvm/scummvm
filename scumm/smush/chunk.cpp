/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "chunk.h"

#include "common/engine.h"
#include "common/file.h"
#include "common/str.h"

class FilePtr {
	ScummVM::String _filename;
	File _ifs;
	int32 _refcount;
	int32 _curPos;
public:
	FilePtr(const char *fname, const char *directory) :
		_filename(fname),
		_refcount(1),
		_curPos(0) {
			debug(9, "FilePtr created for %s", fname);
			_ifs.open(fname, directory);
			if (_ifs.isOpen() == false) error("FilePtr unable to read file %s", fname);
		}

	~FilePtr() {
		debug(9, "FilePtr destroyed for %s", _filename.c_str());
		_ifs.close();
	}

	int32 tell() {
		return _curPos;
	}

	bool seek(int32 pos) {
		if (pos != _curPos) {
			_ifs.seek(pos, SEEK_SET);
			_curPos = pos;
		}
		return true;
	}

	bool read(void *ptr, int32 size) {
		_ifs.read(ptr, size);
		_curPos += size;
		return true;
	}

	void incRef() {
		_refcount++;
	}

	void decRef() {
		if (--_refcount == 0)
			delete this;
	}
};

const char *Chunk::ChunkString(Chunk::type t) {
	static char data[5];
	data[0] = (char)((t >> 24) & 0xFF);
	data[1] = (char)((t >> 16) & 0xFF);
	data[2] = (char)((t >> 8) & 0xFF);
	data[3] = (char)((t >> 0) & 0xFF);
	data[4] = 0;
	return data;
}

BaseChunk::BaseChunk() : 
	_type(0),
	_size(0),
	_curPos(0) {
}

bool BaseChunk::eof() const {
	return _curPos >= _size; 
}

uint32 BaseChunk::tell() const {
	return _curPos; 
}

Chunk::type BaseChunk::getType() const { 
	return _type; 
}

uint32 BaseChunk::getSize() const { 
	return _size; 
}

bool BaseChunk::seek(int32 delta, seek_type dir) {
	switch(dir) {
		case seek_cur:
			_curPos += delta;
			break;
		case seek_start:
			if (delta < 0)
				error("invalid seek request");

			_curPos = (uint32)delta;
			break;
		case seek_end:
			if (delta > 0 || _size < (uint32)-delta)
				error("invalid seek request");

			_curPos = (uint32)(_size + delta);
			break;
	}
	if (_curPos > _size) {
		error("invalid seek request : %d > %d (delta == %d)", _curPos, _size, delta);
	}
	return true;
}

FileChunk::FileChunk() : 
	_data(0) {
}

FileChunk::FileChunk(const char *fname, const char *directory) {
	_data = new FilePtr(fname, directory);
	_data->read(&_type, 4);
	_type = TO_BE_32(_type);
	_data->read(&_size, 4);
	_size = TO_BE_32(_size);
	_offset = _data->tell();
	_curPos = 0;
}

FileChunk::~FileChunk() {
	if (_data)
		_data->decRef();
}

Chunk *FileChunk::subBlock() {
	FileChunk *ptr = new FileChunk();
	ptr->_data = _data;
	_data->incRef();
	_data->seek(_offset + _curPos);
	uint32 temp;
	_data->read(&temp, 4);
	ptr->_type = TO_BE_32(temp);
	_data->read(&temp, 4);
	ptr->_size = TO_BE_32(temp);
	ptr->_offset = _offset + _curPos + 8;
	ptr->_curPos = 0;
	seek(8 + ptr->getSize());
	return ptr;
}

bool FileChunk::read(void *buffer, uint32 size) {
	if (size <= 0 || (_curPos + size) > _size)
		error("invalid buffer read request");

	_data->seek(_offset + _curPos);
	_data->read(buffer, size);
	_curPos += size;
	return true;
}

int8 FileChunk::getChar() {
	if (_curPos >= _size)
		error("invalid char read request");

	_data->seek(_offset + _curPos);
	int8 buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return buffer;
}

byte FileChunk::getByte() {
	if (_curPos >= _size)
		error("invalid byte read request");

	_data->seek(_offset + _curPos);
	byte buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return buffer;
}

int16 FileChunk::getShort() {
	int16 buffer = getWord();
	return *((int16*)&buffer);
}

uint16 FileChunk::getWord() {
	if (_curPos >= _size - 1)
		error("invalid word read request");

	_data->seek(_offset + _curPos);
	uint16 buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return TO_LE_16(buffer);
}

uint32 FileChunk::getDword() {
	if (_curPos >= _size - 3)
		error("invalid dword read request");

	_data->seek(_offset + _curPos);
	uint32 buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return TO_LE_32(buffer);
}

MemoryChunk::MemoryChunk(byte *data) {
	if (data == 0)
		error("Chunk() called with NULL pointer");

	_type = (Chunk::type)READ_BE_UINT32(data);
	_size = READ_BE_UINT32(data + 4);
	_data = data + sizeof(Chunk::type) + sizeof(uint32);
	_curPos = 0;
}

Chunk *MemoryChunk::subBlock() {
	MemoryChunk *ptr = new MemoryChunk(_data + _curPos);
	seek(sizeof(Chunk::type) + sizeof(uint32) + ptr->getSize());
	return ptr;
}

bool MemoryChunk::read(void *buffer, uint32 size) {
	if (size <= 0 || (_curPos + size) > _size)
		error("invalid buffer read request");

	memcpy(buffer, _data + _curPos, size);
	_curPos += size;
	return true;
}

int8 MemoryChunk::getChar() {
	if (_curPos >= _size)
		error("invalid char read request");

	return _data[_curPos++];
}

byte MemoryChunk::getByte() {
	if (_curPos >= _size)
		error("invalid byte read request");

	byte *ptr = (byte *)(_data + _curPos);
	_curPos += 1;
	return *ptr;
}

int16 MemoryChunk::getShort() {
	if (_curPos >= _size - 1)
		error("invalid int16 read request");

	int16 buffer = getWord();
	return *((int16 *)&buffer);
}

uint16 MemoryChunk::getWord() {
	if (_curPos >= _size - 1)
		error("invalid word read request");

	uint16 *ptr = (uint16 *)(_data + _curPos);
	_curPos += 2;
	return READ_LE_UINT16(ptr);
}

uint32 MemoryChunk::getDword() {
	if (_curPos >= _size - 3)
		error("invalid dword read request");

	uint32 *ptr = (uint32 *)(_data + _curPos);
	_curPos += 4;
	return READ_LE_UINT32(ptr);
}
