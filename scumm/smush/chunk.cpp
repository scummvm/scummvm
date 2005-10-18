/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "scumm/smush/chunk.h"
#include "scumm/scumm.h"

#include "common/file.h"
#include "common/str.h"
#include "common/util.h"

namespace Scumm {

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

FileChunk::FileChunk(ScummFile *data, int offset) {
	_data = data;
	_deleteData = false;

	_data->seek(offset, seek_start);
	_type = _data->readUint32BE();
	_size = _data->readUint32BE();
	_offset = _data->pos();
	_curPos = 0;
}

FileChunk::FileChunk(const Common::String &name, int offset) {
	_data = new ScummFile();
	_deleteData = true;
	if (!g_scumm->openFile(*_data, name.c_str()))
		error("FileChunk: Unable to open file %s", name.c_str());

	_data->seek(offset, seek_start);
	_type = _data->readUint32BE();
	_size = _data->readUint32BE();
	_offset = _data->pos();
	_curPos = 0;
}

FileChunk::~FileChunk() {
	if (_deleteData)
		delete _data;
}

Chunk *FileChunk::subBlock() {
	FileChunk *ptr = new FileChunk(_data, _offset + _curPos);
	seek(sizeof(Chunk::type) + sizeof(uint32) + ptr->getSize());
	return ptr;
}

void FileChunk::reseek() {
	_data->seek(_offset + _curPos);
}

bool FileChunk::read(void *buffer, uint32 size) {
	if (size <= 0 || (_curPos + size) > _size)
		error("invalid buffer read request");

	_data->read(buffer, size);
	_curPos += size;
	return true;
}

int8 FileChunk::getChar() {
	return (int8)getByte();
}

byte FileChunk::getByte() {
	_curPos++;

	if (_curPos > _size)
		error("invalid byte read request");

	return _data->readByte();
}

int16 FileChunk::getShort() {
	return (int16)getWord();
}

uint16 FileChunk::getWord() {
	_curPos += 2;

	if (_curPos > _size)
		error("invalid word read request");

	return _data->readUint16LE();
}

uint32 FileChunk::getDword() {
	_curPos += 4;

	if (_curPos > _size)
		error("invalid dword read request");

	return _data->readUint32LE();
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

void MemoryChunk::reseek() {
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

} // End of namespace Scumm
