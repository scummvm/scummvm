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

#include "common/savefile.h"
#include "common/stream.h"

#include "sci/sci.h"
#include "sci/engine/file.h"

namespace Sci {

#ifdef ENABLE_SCI32

VirtualIndexFile::VirtualIndexFile(Common::String fileName) : _fileName(fileName), _changed(false) {
	Common::SeekableReadStream *inFile = g_sci->getSaveFileManager()->openForLoading(fileName);

	_bufferSize = inFile->size();
	_buffer = new char[_bufferSize];
	inFile->read(_buffer, _bufferSize);
	_ptr = _buffer;
	delete inFile;
}

VirtualIndexFile::~VirtualIndexFile() {
	close();

	_bufferSize = 0;
	delete[] _buffer;
	_buffer = 0;
}

uint32 VirtualIndexFile::read(char *buffer, uint32 size) {
	uint32 curPos = _ptr - _buffer;
	uint32 finalSize = MIN<uint32>(size, _bufferSize - curPos);
	char *localPtr = buffer;

	for (uint32 i = 0; i < finalSize; i++)
		*localPtr++ = *_ptr++;

	return finalSize;
}

uint32 VirtualIndexFile::write(const char *buffer, uint32 size) {
	_changed = true;
	uint32 curPos = _ptr - _buffer;
	
	// Check if the buffer needs to be resized
	if (curPos + size >= _bufferSize) {
		_bufferSize = curPos + size + 1;
		char *tmp = _buffer;
		_buffer = new char[_bufferSize];
		_ptr = _buffer + curPos;
		memcpy(_buffer, tmp, _bufferSize);
		delete[] tmp;
	}

	for (uint32 i = 0; i < size; i++)
		*_ptr++ = *buffer++;

	return size;
}

uint32 VirtualIndexFile::readLine(char *buffer, uint32 size) {
	uint32 startPos = _ptr - _buffer;
	uint32 bytesRead = 0;
	char *localPtr = buffer;

	// This is not a full-blown implementation of readLine, but it
	// suffices for Phantasmagoria
	while (startPos + bytesRead < size) {
		bytesRead++;

		if (*_ptr == 0 || *_ptr == 0x0A) {
			_ptr++;
			*localPtr = 0;
			return bytesRead;
		} else {
			*localPtr++ = *_ptr++;
		}
	}

	return bytesRead;
}

bool VirtualIndexFile::seek(int32 offset, int whence) {
	uint32 startPos = _ptr - _buffer;
	assert(offset >= 0);

	switch (whence) {
	case SEEK_CUR:
		assert(startPos + offset < _bufferSize);
		_ptr += offset;
		break;
	case SEEK_SET:
		assert(offset < _bufferSize);
		_ptr = _buffer + offset;
		break;
	case SEEK_END:
		assert(_bufferSize - offset >= 0);
		_ptr = _buffer + (_bufferSize - offset);
		break;
	}

	return true;
}

void VirtualIndexFile::close() {
	if (_changed) {
		Common::WriteStream *outFile = g_sci->getSaveFileManager()->openForSaving(_fileName);
		outFile->write(_buffer, _bufferSize);
		delete outFile;
	}

	// Maintain the buffer, and seek to the beginning of it
	_ptr = _buffer;
}

#endif

} // End of namespace Sci
