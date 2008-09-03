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

#include "common/stream.h"
#include "common/str.h"
#include "common/util.h"

namespace Common {

void WriteStream::writeString(const String &str) {
	write(str.c_str(), str.size());
}

MemoryReadStream *ReadStream::readStream(uint32 dataSize) {
	void *buf = malloc(dataSize);
	dataSize = read(buf, dataSize);
	assert(dataSize > 0);
	return new MemoryReadStream((byte *)buf, dataSize, true);
}


uint32 MemoryReadStream::read(void *dataPtr, uint32 dataSize) {
	// Read at most as many bytes as are still available...
	if (dataSize > _size - _pos)
		dataSize = _size - _pos;
	memcpy(dataPtr, _ptr, dataSize);

	if (_encbyte) {
		byte *p = (byte *)dataPtr;
		byte *end = p + dataSize;
		while (p < end)
			*p++ ^= _encbyte;
	}

	_ptr += dataSize;
	_pos += dataSize;

	return dataSize;
}

void MemoryReadStream::seek(int32 offs, int whence) {
	// Pre-Condition
	assert(_pos <= _size);
	switch (whence) {
	case SEEK_END:
		// SEEK_END works just like SEEK_SET, only 'reversed',
		// i.e. from the end.
		offs = _size - offs;
		// Fall through
	case SEEK_SET:
		_ptr = _ptrOrig + offs;
		_pos = offs;
		break;

	case SEEK_CUR:
		_ptr += offs;
		_pos += offs;
		break;
	}
	// Post-Condition
	assert(_pos <= _size);
}

#define LF 0x0A
#define CR 0x0D

char *SeekableReadStream::readLine_OLD(char *buf, size_t bufSize) {
	assert(buf && bufSize > 0);
	char *p = buf;
	size_t len = 0;
	char c;

	if (buf == 0 || bufSize == 0 || eos()) {
		return 0;
	}

	// We don't include the newline character(s) in the buffer, and we
	// always terminate it - we never read more than len-1 characters.

	// EOF is treated as a line break, unless it was the first character
	// that was read.

	// 0 is treated as a line break, even though it should never occur in
	// a text file.

	// DOS and Windows use CRLF line breaks
	// Unix and OS X use LF line breaks
	// Macintosh before OS X uses CR line breaks


	c = readByte();
	if (eos() || ioFailed()) {
		return 0;
	}

	while (!eos() && len + 1 < bufSize) {

		if (ioFailed())
			return 0;

		if (c == 0 || c == LF)
			break;

		if (c == CR) {
			c = readByte();
			if (c != LF && !eos())
				seek(-1, SEEK_CUR);
			break;
		}

		*p++ = c;
		len++;

		c = readByte();
	}

	// This should fix a bug while using readLine with Common::File
	// it seems that it sets the eos flag after an invalid read
	// and at the same time the ioFailed flag
	// the config file parser fails out of that reason for the new themes
	if (eos()) {
		clearIOFailed();
	}

	*p = 0;
	return buf;
}

char *SeekableReadStream::readLine_NEW(char *buf, size_t bufSize) {
	assert(buf != 0 && bufSize > 1);
	char *p = buf;
	size_t len = 0;
	char c = 0;

	// If end-of-file occurs before any characters are read, return NULL
	// and the buffer contents remain unchanged. 
	if (eos() || ioFailed()) {
		return 0;
	}

	// Loop as long as the stream has not ended, there is still free
	// space in the buffer, and the line has not ended
	while (!eos() && len + 1 < bufSize && c != LF) {
		c = readByte();
		
		// If end-of-file occurs before any characters are read, return
		// NULL and the buffer contents remain unchanged. If an error
		/// occurs, return NULL and the buffer contents are indeterminate.
		if (ioFailed() || (len == 0 && eos()))
			return 0;

		// Check for CR or CR/LF
		// * DOS and Windows use CRLF line breaks
		// * Unix and OS X use LF line breaks
		// * Macintosh before OS X used CR line breaks
		if (c == CR) {
			// Look at the next char -- is it LF? If not, seek back
			c = readByte();
			if (c != LF && !eos())
				seek(-1, SEEK_CUR);
			// Treat CR & CR/LF as plain LF
			c = LF;
		}
		
		*p++ = c;
		len++;
	}

	// FIXME:
	// This should fix a bug while using readLine with Common::File
	// it seems that it sets the eos flag after an invalid read
	// and at the same time the ioFailed flag
	// the config file parser fails out of that reason for the new themes
	if (eos()) {
		clearIOFailed();
	}

	// We always terminate the buffer if no error occured
	*p = 0;
	return buf;
}

String SeekableReadStream::readLine() {
	// Read a line
	String line;
	while (line.lastChar() != '\n') {
		char buf[256];
		if (!readLine_NEW(buf, 256))
			break;
		line += buf;
	}
	
	if (line.lastChar() == '\n')
		line.deleteLastChar();

	return line;
}



uint32 SubReadStream::read(void *dataPtr, uint32 dataSize) {
	dataSize = MIN(dataSize, _end - _pos);

	dataSize = _parentStream->read(dataPtr, dataSize);
	_pos += dataSize;

	return dataSize;
}

SeekableSubReadStream::SeekableSubReadStream(SeekableReadStream *parentStream, uint32 begin, uint32 end, bool disposeParentStream)
	: SubReadStream(parentStream, end, disposeParentStream),
	_parentStream(parentStream),
	_begin(begin) {
	assert(_begin <= _end);
	_pos = _begin;
	_parentStream->seek(_pos);
}

void SeekableSubReadStream::seek(int32 offset, int whence) {
	assert(_pos >= _begin);
	assert(_pos <= _end);

	switch(whence) {
	case SEEK_END:
		offset = size() - offset;
		// fallthrough
	case SEEK_SET:
		_pos = _begin + offset;
		break;
	case SEEK_CUR:
		_pos += offset;
	}

	assert(_pos >= _begin);
	assert(_pos <= _end);

	_parentStream->seek(_pos);
}

BufferedReadStream::BufferedReadStream(ReadStream *parentStream, uint32 bufSize, bool disposeParentStream)
	: _parentStream(parentStream),
	_disposeParentStream(disposeParentStream),
	_pos(0),
	_bufSize(0),
	_realBufSize(bufSize) {

	assert(parentStream);
	_buf = new byte[bufSize];
	assert(_buf);
}

BufferedReadStream::~BufferedReadStream() {
	if (_disposeParentStream)
		delete _parentStream;
	delete _buf;
}

uint32 BufferedReadStream::read(void *dataPtr, uint32 dataSize) {
	uint32 alreadyRead = 0;
	const uint32 bufBytesLeft = _bufSize - _pos;

	// Check whether the data left in the buffer suffices....
	if (dataSize > bufBytesLeft) {
		// Nope, we need to read more data

		// First, flush the buffer, if it is non-empty
		if (0 < bufBytesLeft) {
			memcpy(dataPtr, _buf + _pos, bufBytesLeft);
			_pos = _bufSize;
			alreadyRead += bufBytesLeft;
			dataPtr = (byte *)dataPtr + bufBytesLeft;
			dataSize -= bufBytesLeft;
		}
			
		// At this point the buffer is empty. Now if the read request
		// exceeds the buffer size, just satisfy it directly.
		if (dataSize > _bufSize)
			return alreadyRead + _parentStream->read(dataPtr, dataSize);

		// Refill the buffer.
		// If we didn't read as many bytes as requested, the reason
		// is EOF or an error. In that case we truncate the buffer
		// size, as well as the number of  bytes we are going to
		// return to the caller.
		_bufSize = _parentStream->read(_buf, _realBufSize);
		_pos = 0;
		if (dataSize > _bufSize)
			dataSize = _bufSize;
	}

	// Satisfy the request from the buffer
	memcpy(dataPtr, _buf + _pos, dataSize);
	_pos += dataSize;
	return alreadyRead + dataSize;
}

BufferedSeekableReadStream::BufferedSeekableReadStream(SeekableReadStream *parentStream, uint32 bufSize, bool disposeParentStream)
	: BufferedReadStream(parentStream, bufSize, disposeParentStream),
	_parentStream(parentStream) {
}

void BufferedSeekableReadStream::seek(int32 offset, int whence) {
	// If it is a "local" seek, we may get away with "seeking" around
	// in the buffer only.
	// Note: We could try to handle SEEK_END and SEEK_SET, too, but
	// since they are rarely used, it seems not worth the effort.
	if (whence == SEEK_CUR && (int)_pos + offset >= 0 && _pos + offset <= _bufSize) {
		_pos += offset;
	} else {
		// Seek was not local enough, so we reset the buffer and
		// just seeks normally in the parent stream.
		if (whence == SEEK_CUR)
			offset -= (_bufSize - _pos);
		_pos = _bufSize;
		_parentStream->seek(offset, whence);
	}
}

}	// End of namespace Common
