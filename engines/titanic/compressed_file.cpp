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

#include "titanic/compressed_file.h"

namespace Titanic {

#define BUFFER_SIZE 1024

CompressedFile::CompressedFile() : SimpleFile() {
	_fileMode = COMPMODE_NONE;
	Common::fill(&_writeBuffer[0], &_writeBuffer[516], 0);
	_dataStartPtr = nullptr;
	_dataPtr = nullptr;
	_dataRemaining = 0;
	_dataMaxSize = 0;
	_dataCount = 0;
}

CompressedFile::~CompressedFile() {
}

void CompressedFile::open(const Common::String &name) {
	SimpleFile::open(name);

	_compression.initDecompress();
	_fileMode = COMPMODE_READ;
	_dataPtr = _dataStartPtr = new byte[BUFFER_SIZE];
	_dataMaxSize = BUFFER_SIZE;
	_dataRemaining = 0;
	_dataCount = 0;
}

void CompressedFile::open(Common::SeekableReadStream *stream) {
	SimpleFile::open(stream);

	_compression.initDecompress();
	_fileMode = COMPMODE_READ;
	_dataPtr = _dataStartPtr = new byte[BUFFER_SIZE];
	_dataMaxSize = BUFFER_SIZE;
	_dataRemaining = 0;
	_dataCount = 0;
}

void CompressedFile::open(Common::OutSaveFile *stream) {
	SimpleFile::open(stream);

	_compression.initCompress();
	_fileMode = COMPMODE_WRITE;
}

void CompressedFile::close() {
	int result;

	switch (_fileMode) {
	case COMPMODE_WRITE:
		do {
			_compression._destPtr = _writeBuffer;
			_compression._destCount = 512;
			result = _compression.compress(4);
			int count = 512 - _compression._destCount;

			if (count)
				write(_writeBuffer, count);
		} while (!result);
		break;
	case COMPMODE_READ:
		_compression.close();
		delete[] _dataStartPtr;
		_dataStartPtr = _dataPtr = nullptr;
		_dataRemaining = _dataMaxSize = 0;

		SimpleFile::close();
		break;
	default:
		break;
	}
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
		if (!_dataRemaining) {
			decompress();
			if (!_dataRemaining)
				break;
		}

		*dataPtr++ = *_dataPtr++;
		--_dataRemaining;
	}

	return bytesRead;
}

void CompressedFile::decompress() {
	const size_t COUNT = 1;
	byte fileByte;
	int count;

	_dataPtr = _dataStartPtr;
	_compression._destPtr = _dataStartPtr;
	_compression._destCount = _dataMaxSize;

	if (_dataMaxSize < 0x100)
		return;

	// Loop to get data from the file as needed and decompress
	do {
		if (!_compression._srcCount) {
			// Read in next byte from the source file
			if (!SimpleFile::unsafeRead(&fileByte, 1))
				break;

			// Set up the decompressor to process the data
			_compression._srcCount = COUNT;
			_compression._srcPtr = &fileByte;
		}

		int count = _compression.decompress(COUNT);
		_dataRemaining = _dataMaxSize - _compression._destCount;

		if (count == COUNT) {
			_dataCount = COUNT;
			break;
		}
	} while (!count && _compression._destCount > 0x100);
}

} // End of namespace Titanic
