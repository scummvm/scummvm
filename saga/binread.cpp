/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "binread.h"

BinReader::BinReader() {
	_buf    = NULL;
	_bufPtr = NULL;
	_bufEnd = NULL;
	_bufLen = 0;
}

BinReader::BinReader(const byte *buf, size_t buflen) {
	_buf    = buf;
	_bufPtr = buf;
	_bufEnd = buf + buflen;
	_bufLen = buflen;
}

BinReader::~BinReader() {
}

void BinReader::setBuf(const byte *buf, size_t buflen) {
	_buf    = buf;
	_bufPtr = buf;
	_bufEnd = buf + buflen;
	_bufLen = buflen;
}

void BinReader::skip(size_t skip_ct) {
	assert((_bufPtr + skip_ct) <= _bufEnd );

	_bufPtr += skip_ct;
}

unsigned int BinReader::readUint16LE() {
	assert((_bufPtr + 2) <= _bufEnd);
	
	unsigned int u16_le = ((unsigned int)_bufPtr[1] << 8) | _bufPtr[0];

	_bufPtr += 2;

	return u16_le;
}

unsigned int BinReader::readUint16BE() {
	assert((_bufPtr + 2) <= _bufEnd);

	unsigned int u16_be = ((unsigned int)_bufPtr[0] << 8) | _bufPtr[1];
 
	_bufPtr += 2;
	    
	return u16_be;
}

int BinReader::readSint16LE() {
	assert((_bufPtr + 2) <= _bufEnd);

	unsigned int u16_le = ((unsigned int)_bufPtr[1] << 8) | _bufPtr[0];

	_bufPtr += 2;

	return u16_le;
}

int BinReader::readSint16BE() {
	assert((_bufPtr + 2) <= _bufEnd);

	unsigned int u16_be = ((unsigned int)_bufPtr[0] << 8) | _bufPtr[1];
 
	_bufPtr += 2;
	    
	return u16_be;
}


uint32 BinReader::readUint32LE() {
	assert((_bufPtr + 4) <= _bufEnd);

	unsigned long u32_le = ((unsigned long)_bufPtr[3] << 24) | 
	                       ((unsigned long)_bufPtr[2] << 16) |
	                       ((unsigned long)_bufPtr[1] << 8 ) |
	                       _bufPtr[0];

	_bufPtr += 4;

	return u32_le;
}

uint32 BinReader::readUint32BE() {
	assert((_bufPtr + 4) <= _bufEnd);

	unsigned long u32_be = ((unsigned long)_bufPtr[0] << 24) | 
	                       ((unsigned long)_bufPtr[1] << 16) |
	                       ((unsigned long)_bufPtr[2] << 8 ) |
	                       _bufPtr[3];

	_bufPtr += 4;

	return u32_be;
}

int32 BinReader::readSint32LE() {
	assert((_bufPtr + 4) <= _bufEnd);

	unsigned long u32_le = ((unsigned long)_bufPtr[3] << 24) | 
	                       ((unsigned long)_bufPtr[2] << 16) |
	                       ((unsigned long)_bufPtr[1] << 8 ) |
	                       _bufPtr[0];

	_bufPtr += 4;

	return u32_le;
}

int32 BinReader::readSint32BE() {
	assert((_bufPtr + 4) <= _bufEnd);

	unsigned long u32_be = ((unsigned long)_bufPtr[0] << 24) | 
	                       ((unsigned long)_bufPtr[1] << 16) |
	                       ((unsigned long)_bufPtr[2] << 8 ) |
	                       _bufPtr[3];

	_bufPtr += 4;

	return u32_be;
}


