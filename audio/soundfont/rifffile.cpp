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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
/*
 * VGMTrans (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included VGMTrans_LICENSE.txt file
 */

#include "rifffile.h"

using namespace std;

uint32 Chunk::GetSize() {
	return 8 + GetPaddedSize(_size);
}

void Chunk::SetData(const void *src, uint32 datasize) {
	_size = datasize;

	// set the size and copy from the data source
	datasize = GetPaddedSize(_size);
	if (_data != NULL) {
		delete[] _data;
		_data = NULL;
	}
	_data = new uint8[datasize];
	memcpy(_data, src, _size);

	// Add pad byte
	uint32 padsize = datasize - _size;
	if (padsize != 0) {
		memset(_data + _size, 0, padsize);
	}
}

void Chunk::Write(uint8 *buffer) {
	uint32 padsize = GetPaddedSize(_size) - _size;
	memcpy(buffer, _id, 4);
	*(uint32 * )(buffer + 4) =
			_size + padsize;  // Microsoft says the chunkSize doesn't contain padding size, but many
	// software cannot handle the alignment.
	memcpy(buffer + 8, _data, GetPaddedSize(_size));
}

Chunk *ListTypeChunk::AddChildChunk(Chunk *ck) {
	_childChunks.push_back(ck);
	return ck;
}

uint32 ListTypeChunk::GetSize() {
	uint32 listChunkSize = 12;  // id + size + "LIST"
	for (Common::List<Chunk *>::iterator iter = this->_childChunks.begin(); iter != _childChunks.end(); iter++)
		listChunkSize += (*iter)->GetSize();
	return GetPaddedSize(listChunkSize);
}

void ListTypeChunk::Write(uint8 *buffer) {
	memcpy(buffer, this->_id, 4);
	memcpy(buffer + 8, this->_type, 4);

	uint32 bufOffset = 12;
	for (Common::List<Chunk *>::iterator iter = this->_childChunks.begin(); iter != _childChunks.end(); iter++) {
		(*iter)->Write(buffer + bufOffset);
		bufOffset += (*iter)->GetSize();
	}

	uint32 unpaddedSize = bufOffset;
	uint32 padsize = GetPaddedSize(unpaddedSize) - unpaddedSize;
	*(uint32 *) (buffer + 4) =
			unpaddedSize + padsize - 8;  // Microsoft says the chunkSize doesn't contain padding size, but many
	// software cannot handle the alignment.

	// Add pad byte
	if (padsize != 0) {
		memset(_data + unpaddedSize, 0, padsize);
	}
}

RiffFile::RiffFile(const Common::String &file_name, const Common::String &form) : RIFFChunk(form), _name(file_name) {}
