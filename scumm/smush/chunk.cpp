/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include <stdafx.h>
#include "chunk.h"

#include <stdio.h> // for FILE, fopen, fclose, fseek and ftell
#include <string.h> // for memcpy

/*!	@brief very small and fast wrapper for a ifstream.

	implements reference counting, so that ::file_Chunk does not leak memory !
*/
class FilePtr {
	char * _filename;
	FILE * _ifs;
	int _refcount;
	int _curPos;
public:
	FilePtr(const char * fname) : _refcount(1), _curPos(0) {
		debug(9, "FilePtr created for %s", fname);
		_filename = strdup(fname);
		_ifs  = fopen(fname, "rb");
		if(_ifs == NULL) error("FilePtr unable to read file \"%s\"", fname);
	}
	~FilePtr() {
		debug(9, "FilePtr destroyed for %s", _filename);
		free(_filename);
		fclose(_ifs);
	}
	int tell() {
		return _curPos;
	}
	bool seek(int pos) {
		if(pos != _curPos) {
			fseek(_ifs, pos, SEEK_SET);
			_curPos = pos;
		}
		return true;
	}
	bool read(void * ptr, int size) {
		fread(ptr, size, 1, _ifs);
		_curPos += size;
		return true;
	}
	void incRef() {
		_refcount++;
	}
	void decRef() {
		if(--_refcount == 0)
			delete this;
	}
};

const char * Chunk::ChunkString(Chunk::type t) {
	static char data[5];
	data[0] = (char)((t >> 24) & 0xFF);
	data[1] = (char)((t >> 16) & 0xFF);
	data[2] = (char)((t >> 8) & 0xFF);
	data[3] = (char)((t >> 0) & 0xFF);
	data[4] = 0;
	return data;
}

FileChunk::FileChunk() : _data(0), _type(0), _size(0), _curPos(0) {
}

FileChunk::~FileChunk() {
	if(_data) _data->decRef();
}

FileChunk::FileChunk(const char * fname) {
	_data = new FilePtr(fname);
	_data->read(&_type, 4);
	_type = TO_BE_32(_type);
	_data->read(&_size, 4);
	_size = TO_BE_32(_size);
	_offset = _data->tell();
	_curPos = 0;
}

Chunk::type FileChunk::getType() const { 
	return _type; 
}

unsigned int FileChunk::getSize() const { 
	return _size; 
}

Chunk * FileChunk::subBlock() {
	FileChunk * ptr = new FileChunk;
	ptr->_data = _data;
	_data->incRef();
	_data->seek(_offset + _curPos);
	unsigned int temp;
	_data->read(&temp, 4);
	ptr->_type = TO_BE_32(temp);
	_data->read(&temp, 4);
	ptr->_size = TO_BE_32(temp);
	ptr->_offset = _offset + _curPos + 8;
	ptr->_curPos = 0;
	seek(8 + ptr->getSize());
	return ptr;
}

bool FileChunk::eof() const { 
	return _curPos >= _size; 
}

unsigned int FileChunk::tell() const { 
	return _curPos; 
}

bool FileChunk::seek(int delta, seek_type dir) {
	switch(dir) {
		case seek_cur:
			_curPos += delta;
			break;
		case seek_start:
			if(delta < 0) error("invalid seek request");
			_curPos = (unsigned int)delta;
			break;
		case seek_end:
			if(delta > 0 || (_size + delta) < 0) error("invalid seek request");
			_curPos = (unsigned int)(_size + delta);
			break;
	}
	if(_curPos > _size) {
		error("invalid seek request : %d > %d (delta == %d)", _curPos, _size, delta);
	}
	return true;
}

bool FileChunk::read(void * buffer, unsigned int size) {
	if(size <= 0 || (_curPos + size) > _size) error("invalid buffer read request");
	_data->seek(_offset + _curPos);
	_data->read(buffer, size);
	_curPos += size;
	return true;
}

char FileChunk::getChar() {
	if(_curPos >= _size) error("invalid char read request");
	_data->seek(_offset + _curPos);
	char buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return buffer;
}

unsigned char FileChunk::getByte() {
	if(_curPos >= _size) error("invalid byte read request");
	_data->seek(_offset + _curPos);
	unsigned char buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return buffer;
}

short FileChunk::getShort() {
	unsigned short buffer = getWord();
	return *((short*)&buffer);
}

unsigned short FileChunk::getWord() {
	if(_curPos >= _size - 1) error("invalid word read request");
	_data->seek(_offset + _curPos);
	unsigned short buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return TO_LE_16(buffer);
}

unsigned int FileChunk::getDword() {
	if(_curPos >= _size - 3) error("invalid dword read request");
	_data->seek(_offset + _curPos);
	unsigned int buffer;
	_data->read(&buffer, sizeof(buffer));
	_curPos+= sizeof(buffer);
	return TO_LE_32(buffer);
}

ContChunk::ContChunk(char * data) {
	if(data == 0) error("Chunk() called with NULL pointer");
	_type = (Chunk::type)READ_BE_UINT32(data);
	_size = READ_BE_UINT32(data+4);
	_data = data + sizeof(Chunk::type) + sizeof(unsigned int);
	_curPos = 0;
}

Chunk::type ContChunk::getType() const { 
	return _type; 
}

unsigned int ContChunk::getSize() const { 
	return _size; 
}

Chunk * ContChunk::subBlock() {
	ContChunk * ptr = new ContChunk(_data + _curPos);
	seek(sizeof(Chunk::type) + sizeof(unsigned int) + ptr->getSize());
	return ptr;
}

bool ContChunk::eof() const { 
	return _curPos >= _size; 
}

unsigned int ContChunk::tell() const { 
	return _curPos; 
}

bool ContChunk::seek(int delta, seek_type dir) {
	switch(dir) {
		case seek_cur:
			_curPos += delta;
			break;
		case seek_start:
			if(delta < 0) error("invalid seek request");
			_curPos = (unsigned int)delta;
			break;
		case seek_end:
			if(delta > 0 || (_size + delta) < 0) error("invalid seek request");
			_curPos = (unsigned int)(_size + delta);
			break;
	}
	if(_curPos > _size) {
		error("invalid seek request : %d > %d (delta == %d)", _curPos, _size, delta);
	}
	return true;
}

bool ContChunk::read(void * buffer, unsigned int size) {
	if(size <= 0 || (_curPos + size) > _size) error("invalid buffer read request");
	memcpy(buffer, _data + _curPos, size);
	_curPos += size;
	return true;
}

char ContChunk::getChar() {
	if(_curPos >= _size) error("invalid char read request");
	return _data[_curPos++];
}

unsigned char ContChunk::getByte() {
	if(_curPos >= _size) error("invalid byte read request");
	unsigned char * ptr = (unsigned char *)(_data + _curPos);
	_curPos += 1;
	return *ptr;
}

short ContChunk::getShort() {
	if(_curPos >= _size - 1) error("invalid short read request");
	unsigned short buffer = getWord();
	return *((short*)&buffer);
}

unsigned short ContChunk::getWord() {
	if(_curPos >= _size - 1) error("invalid word read request");
	unsigned short * ptr = (unsigned short *)(_data + _curPos);
	_curPos += 2;
	return READ_LE_UINT16(ptr);
}

unsigned int ContChunk::getDword() {
	if(_curPos >= _size - 3) error("invalid dword read request");
	unsigned int * ptr = (unsigned int *)(_data + _curPos);
	_curPos += 4;
	return READ_LE_UINT32(ptr);
}
