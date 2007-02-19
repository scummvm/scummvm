/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/stream.h"
#include "common/str.h"
#include "common/util.h"

namespace Common {

void WriteStream::writeString(const String &str) {
	write(str.c_str(), str.size());
}

void MemoryReadStream::seek(int32 offs, int whence) {
	// Pre-Condition
	assert(_pos <= _bufSize);
	switch (whence) {
	case SEEK_END:
		// SEEK_END works just like SEEK_SET, only 'reversed',
		// i.e. from the end.
		offs = _bufSize - offs;
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
	assert(_pos <= _bufSize);
}

#define LF 0x0A
#define CR 0x0D

char *SeekableReadStream::readLine(char *buf, size_t bufSize) {
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

}	// End of namespace Common
