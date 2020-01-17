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

#include "ultima/shared/engine/resources.h"

namespace Ultima {
namespace Shared {

ResourceFile::ResourceFile(Common::ReadStream *in) : _inStream(in), _outStream(nullptr), _bufferP(_buffer) {
	Common::fill(_buffer, _buffer + STRING_BUFFER_SIZE, 0);
}

ResourceFile::ResourceFile(Common::WriteStream *out) : _inStream(nullptr), _outStream(out), _bufferP(_buffer) {
	Common::fill(_buffer, _buffer + STRING_BUFFER_SIZE, 0);
}

void ResourceFile::syncString(const char *&str) {
	if (_inStream) {
		str = _bufferP;
		while ((*_bufferP = _inStream->readByte()) != '\0')
			++_bufferP;

		assert(_bufferP < (_buffer + STRING_BUFFER_SIZE));
	} else {
		_outStream->writeString(str);
	}
}

void ResourceFile::syncStrings(const char **str, int count) {
	if (_inStream) {
		uint tag = _inStream->readUint32LE();
		assert(tag == MKTAG(count, 0, 0, 0));
	} else {
		_outStream->writeUint32LE(MKTAG(count, 0, 0, 0));
	}

	for (int idx = 0; idx < count; ++idx)
		syncString(str[idx]);
}

void ResourceFile::syncStrings2D(const char **str, int count1, int count2) {
	if (_inStream) {
		uint tag = _inStream->readUint32LE();
		assert(tag == MKTAG(count1, count2, 0, 0));
	} else {
		_outStream->writeUint32LE(MKTAG(count1, count2, 0, 0));
	}

	for (int idx = 0; idx < count1 * count2; ++idx)
		syncString(str[idx]);
}

void ResourceFile::syncNumber(int &val) {
	if (_inStream)
		val = _inStream->readSint32LE();
	else
		_outStream->writeUint32LE(val);
}

void ResourceFile::syncNumbers(int *vals, int count) {
	if (_inStream) {
		uint tag = _inStream->readUint32LE();
		assert(tag == MKTAG(count, 0, 0, 0));
		for (int idx = 0; idx < count; ++idx)
			vals[idx] = _inStream->readSint32LE();
	} else {
		_outStream->writeUint32LE(MKTAG(count, 0, 0, 0));
		for (int idx = 0; idx < count; ++idx)
			_outStream->writeUint32LE(vals[idx]);
	}
}

void ResourceFile::syncNumbers2D(int *vals, int count1, int count2) {
	if (_inStream) {
		uint tag = _inStream->readUint32LE();
		assert(tag == MKTAG(count1, count2, 0, 0));
		for (int idx = 0; idx < count1 * count2; ++idx)
			vals[idx] = _inStream->readSint32LE();
	} else {
		_outStream->writeUint32LE(MKTAG(count1, count2, 0, 0));
		for (int idx = 0; idx < count1 * count2; ++idx)
			_outStream->writeUint32LE(vals[idx]);
	}
}

void ResourceFile::syncNumbers3D(int *vals, int count1, int count2, int count3) {
	if (_inStream) {
		uint tag = _inStream->readUint32LE();
		assert(tag == MKTAG(count1, count2, count3, 0));
		for (int idx = 0; idx < count1 * count2 * count3; ++idx)
			vals[idx] = _inStream->readSint32LE();
	} else {
		_outStream->writeUint32LE(MKTAG(count1, count2, count3, 0));
		for (int idx = 0; idx < count1 * count2 * count3; ++idx)
			_outStream->writeUint32LE(vals[idx]);
	}
}

void ResourceFile::syncBytes2D(byte *vals, int count1, int count2) {
	if (_inStream) {
		uint tag = _inStream->readUint32LE();
		assert(tag == MKTAG(count1, count2, 0, 0));
		_inStream->read(vals, count1 * count2);
	} else {
		_outStream->writeUint32LE(MKTAG(count1, count2, 0, 0));
		_outStream->write(vals, count1 * count2);
	}
}

/*-------------------------------------------------------------------*/

bool Resources::setup() {
	return false;
}

} // End of namespace Shared
} // End of namespace Ultima
