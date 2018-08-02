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
 */

#include "startrek/filestream.h"

namespace StarTrek {

FileStream::FileStream(Common::SeekableReadStream *stream, bool bigEndian) {
	_bigEndian = bigEndian;

	_pos = 0;
	_size = stream->size();
	_data = new byte[_size];
	stream->read(_data, _size);
	delete stream;
}

FileStream::~FileStream() {
	delete[] _data;
}

// ReadStream functions

uint32 FileStream::read(void *dataPtr, uint32 dataSize) {
	if (_pos + dataSize > (uint32)size())
		dataSize = size() - _pos;
	memcpy(dataPtr, _data + _pos, dataSize);
	_pos += dataSize;
	return dataSize;
}

byte FileStream::readByte() {
	assert(_pos + 1 <= size());
	return _data[_pos++];
}

uint16 FileStream::readUint16() {
	assert(_pos + 2 <= size());
	uint16 w;
	if (_bigEndian)
		w = _data[_pos + 1] | (_data[_pos] << 8);
	else
		w = _data[_pos] | (_data[_pos + 1] << 8);
	_pos += 2;
	return w;
}

uint32 FileStream::readUint32() {
	assert(_pos + 4 <= size());
	uint32 w;
	if (_bigEndian)
		w = _data[_pos + 3] | (_data[_pos + 2] << 8) | (_data[_pos + 1] << 16) | (_data[_pos] << 24);
	else
		w = _data[_pos] | (_data[_pos + 1] << 8) | (_data[_pos + 2] << 16) | (_data[_pos + 3] << 24);
	_pos += 4;
	return w;
}

int16 FileStream::readSint16() {
	return (int16)readUint16();
}

int32 FileStream::readSint32() {
	return (int32)readUint32();
}

// SeekableReadStream functions

int32 FileStream::pos() const {
	return _pos;
}

int32 FileStream::size() const {
	return _size;
}

bool FileStream::seek(int32 offset, int whence) {
	assert(whence == SEEK_SET);
	_pos = offset;
	return true;
}

} // End of namespace StarTrek
