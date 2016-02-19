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

#include "common/util.h"
#include "titanic/compressed_file.h"

namespace Titanic {

DecompressorData::DecompressorData() {
	_field0 = 0;
	_field4 = 0;
	_field8 = 0;
	_fieldC = 0;
	_field10 = 0;
	_field14 = 0;
}

/*------------------------------------------------------------------------*/

Decompressor::Decompressor() {
	_createFn = nullptr;
	_destroyFn = nullptr;
	_field18 = 0;
	_dataPtr = nullptr;
	_field28 = 0;
}

void Decompressor::load(const char *version, int v) {
	if (!version || *version != '1')
		error("Bad version");
	
	_field18 = 0;
	if (!_createFn) {
		_createFn = &Decompressor::createMethod;
		_field28 = 0;
	}

	if (!_destroyFn) {
		_destroyFn = &Decompressor::destroyMethod;
	}

	_dataPtr = (this->*_createFn)(_field28, 1, 24);
	_dataPtr->_field14 = 0;
	_dataPtr->_fieldC = 0;
	if (v < 0) {
		v = -v;
		_dataPtr->_fieldC = 1;
	}

	if (v < 8 || v > 15)
		error("Bad parameter");

	_dataPtr->_field10 = v;
	_dataPtr->_field14 = sub1(_dataPtr->_fieldC ? nullptr : &Decompressor::method3, 1 << v);

	if (_dataPtr->_field14)
		sub2();
	else
		close();
}

int Decompressor::sub1(Method3Fn fn, int v) {
	return 0;
}

void Decompressor::close() {

}

DecompressorData *Decompressor::createMethod(int v1, int v2, int v3) {
	return new DecompressorData();
}

void Decompressor::destroyMethod(DecompressorData *ptr) {
	delete ptr;
}

/*------------------------------------------------------------------------*/

CompressedFile::CompressedFile() : SimpleFile() {
	_fileMode = 0;
	_isReading = 0;
	_field260 = 0;
	_mode = 0;
}

CompressedFile::~CompressedFile() {
}

void CompressedFile::open(const Common::String &name, FileMode mode) {
	SimpleFile::open(name, mode);

	if (mode == FILE_READ) {
		_decompressor.load();
		_fileMode = 2;
	} else if (mode == FILE_WRITE) {
		_decompressor.load();
		_fileMode = 1;
	}
}

void CompressedFile::open(Common::SeekableReadStream *stream, FileMode mode) {
	SimpleFile::open(stream, mode);

	if (mode == FILE_READ) {
		_decompressor.load();
		_fileMode = 2;
	} else if (mode == FILE_WRITE) {
		_decompressor.load();
		_fileMode = 1;
	}
}

void CompressedFile::close() {
	_queue.clear();
	SimpleFile::close();
}

size_t CompressedFile::unsafeRead(void *dst, size_t count) {
	assert(_file.isOpen());
	if (count == 0)
		return 0;

	// Ensure there's enough data queued in the buffer
	decompress();

	// Pass the data to the output buffer
	size_t bytesRead = 0;
	byte *dataPtr = (byte *)dst;

	while (count > 0) {
		if (_queue.empty()) {
			decompress();
			if (_queue.empty())
				break;
		}

		*dataPtr++ = _queue.pop();
		++bytesRead;
		--count;
	}

	return bytesRead;
}

void CompressedFile::decompress() {

}

} // End of namespace Titanic
