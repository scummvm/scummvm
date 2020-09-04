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

#include "common/memstream.h"
#include "rawfile.h"

uint32 RawFile::GetBytes(size_t offset, uint32 nCount, void *pBuffer) const {
	memcpy(pBuffer, data() + offset, nCount);
	return nCount;
}

const char *MemFile::data() const {
	return (const char *) _data;
}

uint8 MemFile::GetByte(size_t offset) const {
	_seekableReadStream->seek(offset);
	return _seekableReadStream->readByte();
}

uint16 MemFile::GetShort(size_t offset) const {
	_seekableReadStream->seek(offset);
	return _seekableReadStream->readUint16LE();
}

uint32 MemFile::GetWord(size_t offset) const {
	_seekableReadStream->seek(offset);
	return _seekableReadStream->readUint32LE();
}

size_t MemFile::size() const {
	return _seekableReadStream->size();
}

MemFile::~MemFile() {
	delete _seekableReadStream;
}

MemFile::MemFile(const byte *data, uint32 size) : _data(data) {
	_seekableReadStream = new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}
