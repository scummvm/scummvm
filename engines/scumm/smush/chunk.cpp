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


#include "scumm/smush/chunk.h"
#include "scumm/scumm.h"
#include "scumm/file.h"

#include "common/file.h"
#include "common/str.h"
#include "common/util.h"

namespace Scumm {

BaseChunk::BaseChunk() :
	_type(0),
	_size(0),
	_curPos(0),
	_name("") {
}

bool BaseChunk::eos() const {
	return _curPos >= _size;
}

int32 BaseChunk::pos() const {
	return _curPos;
}

Chunk::type BaseChunk::getType() const {
	return _type;
}

int32 BaseChunk::size() const {
	return _size;
}

bool BaseChunk::seek(int32 delta, int dir) {
	switch (dir) {
	case SEEK_CUR:
		_curPos += delta;
		break;
	case SEEK_SET:
		if (delta < 0)
			error("invalid seek request");
		_curPos = (uint32)delta;
		break;
	case SEEK_END:
		if (delta > 0 || _size < (uint32)-delta)
			error("invalid seek request");
		_curPos = (uint32)(_size + delta);
		break;
	default:
		break;
	}

	if (_curPos > _size) {
		// It may happen that user misused our SAN compression tool
		// and ignored FLU index for videos which are used by INSANE.
		// This will lead to incorrect seek requests
		//
		// In fact it may happen only within INSANE, so do not even check for it
		warning("Looks like you compressed file %s in wrong way. It has FLU index which was not updated", _name.c_str());
		error("invalid seek request : %d > %d (delta == %d)", _curPos, _size, delta);
	}
	
	return true;
}

FileChunk::FileChunk(BaseScummFile *data, int offset) {
	_data = data;
	_deleteData = false;

	_data->seek(offset, SEEK_SET);
	_type = _data->readUint32BE();
	_size = _data->readUint32BE();
	_offset = _data->pos();
	_curPos = 0;
}

FileChunk::FileChunk(const Common::String &name, int offset) {
	_data = new ScummFile();
	_deleteData = true;
	if (!g_scumm->openFile(*_data, name))
		error("FileChunk: Unable to open file %s", name.c_str());

	_data->seek(offset, SEEK_SET);
	_type = _data->readUint32BE();
	_size = _data->readUint32BE();
	_offset = _data->pos();
	_curPos = 0;
	_name = name;
}

FileChunk::~FileChunk() {
	if (_deleteData)
		delete _data;
}

Chunk *FileChunk::subBlock() {
	FileChunk *ptr = new FileChunk(_data, _offset + _curPos);
	skip(sizeof(Chunk::type) + sizeof(uint32) + ptr->size());
	return ptr;
}

void FileChunk::reseek() {
	_data->seek(_offset + _curPos, SEEK_SET);
}

uint32 FileChunk::read(void *buffer, uint32 dataSize) {
	if (dataSize <= 0 || (_curPos + dataSize) > _size)
		error("invalid buffer read request");

	dataSize = _data->read(buffer, dataSize);
	_curPos += dataSize;

	return dataSize;
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
	skip(sizeof(Chunk::type) + sizeof(uint32) + ptr->size());
	return ptr;
}

void MemoryChunk::reseek() {
}

uint32 MemoryChunk::read(void *buffer, uint32 dataSize) {
	if (dataSize <= 0 || (_curPos + dataSize) > _size)
		error("invalid buffer read request");

	memcpy(buffer, _data + _curPos, dataSize);
	_curPos += dataSize;
	return dataSize;
}

} // End of namespace Scumm
