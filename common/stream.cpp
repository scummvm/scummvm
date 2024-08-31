/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/ptr.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/str.h"

namespace Common {

enum {
	kTempBufSize = 65536
};

uint32 WriteStream::writeStream(ReadStream *stream, uint32 dataSize) {
	void *buf = malloc(dataSize);
	dataSize = stream->read(buf, dataSize);
	assert(dataSize > 0);
	dataSize = write(buf, dataSize);
	free(buf);
	return dataSize;
}

uint32 WriteStream::writeStream(ReadStream *stream) {
	uint32 ret = 0;

	void *buf = malloc(kTempBufSize);
	assert(buf);

	uint32 readSize, writeSize;
	do {
		readSize = stream->read(buf, kTempBufSize);
		writeSize = write(buf, readSize);
		ret += writeSize;
	} while(readSize == kTempBufSize && writeSize == readSize);

	free(buf);
	return ret;
}

void WriteStream::writeString(const String &str) {
	write(str.c_str(), str.size());
}

SeekableReadStream *ReadStream::readStream(uint32 dataSize) {
	void *buf = malloc(dataSize);
	dataSize = read(buf, dataSize);
	assert(dataSize > 0);
	return new MemoryReadStream((byte *)buf, dataSize, DisposeAfterUse::YES);
}

Common::String ReadStream::readString(char terminator, size_t len) {
	Common::String result;
	char c;
	bool end = false;
	size_t bytesRead = 0;

	while (bytesRead < len && !(end && len == Common::String::npos)) {
		c = (char)readByte();
		if (eos())
			break;
		if (c == terminator)
			end = true;
		if (!end)
			result += c;
		bytesRead++;
	}

	return result;
}

Common::String ReadStream::readPascalString(bool transformCR) {
	Common::String s;
	char *buf;
	int len;
	int i;

	len = readByte();
	buf = (char *)malloc(len + 1);
	for (i = 0; i < len; i++) {
		buf[i] = readByte();
		if (transformCR && buf[i] == 0x0d)
			buf[i] = '\n';
	}

	buf[i] = 0;

	s = buf;
	free(buf);

	return s;
}

uint32 MemoryReadStream::read(void *dataPtr, uint32 dataSize) {
	// Read at most as many bytes as are still available...
	if (dataSize > _size - _pos) {
		dataSize = _size - _pos;
		_eos = true;
	}
	memcpy(dataPtr, _ptr, dataSize);

	_ptr += dataSize;
	_pos += dataSize;

	return dataSize;
}

bool MemoryReadStream::seek(int64 offs, int whence) {
	// Pre-Condition
	assert(_pos <= _size);
	switch (whence) {
	case SEEK_END:
		// SEEK_END works just like SEEK_SET, only 'reversed',
		// i.e. from the end.
		offs = _size + offs;
		// Fall through
	case SEEK_SET:
		// Fall through
	default:
		_ptr = _ptrOrig.get() + offs;
		_pos = offs;
		break;

	case SEEK_CUR:
		_ptr += offs;
		_pos += offs;
		break;
	}
	
	if (!(_pos <= _size)) {
		_pos = _size;
	}

	// Reset end-of-stream flag on a successful seek
	_eos = false;
	return true; // FIXME: STREAM REWRITE
}

#pragma mark -

enum {
	LF = 0x0A,
	CR = 0x0D
};

char *SeekableReadStream::readLine(char *buf, size_t bufSize, bool handleCR) {
	assert(buf != nullptr && bufSize > 1);
	char *p = buf;
	size_t len = 0;
	char c = 0;

	// If end-of-file occurs before any characters are read, return NULL
	// and the buffer contents remain unchanged.
	if (eos() || err()) {
		return nullptr;
	}

	// Loop as long as there is still free space in the buffer,
	// and the line has not ended
	while (len + 1 < bufSize && c != LF) {
		c = readByte();

		if (eos()) {
			// If end-of-file occurs before any characters are read, return
			// NULL and the buffer contents remain unchanged.
			if (len == 0)
				return nullptr;

			break;
		}

		// If an error occurs, return NULL and the buffer contents
		// are indeterminate.
		if (err())
			return nullptr;

		// Check for CR or CR/LF
		// * DOS and Windows use CRLF line breaks
		// * Unix and macOS use LF line breaks
		// * Macintosh before macOS used CR line breaks
		if (c == CR && handleCR) {
			// Look at the next char -- is it LF? If not, seek back
			c = readByte();

			if (err()) {
				return nullptr; // error: the buffer contents are indeterminate
			}
			if (eos()) {
				// The CR was the last character in the file.
				// Reset the eos() flag since we successfully finished a line
				clearErr();
			} else if (c != LF) {
				seek(-1, SEEK_CUR);
			}

			// Treat CR & CR/LF as plain LF
			c = LF;
		}

		*p++ = c;
		len++;
	}

	// We always terminate the buffer if no error occurred
	*p = 0;
	return buf;
}

String SeekableReadStream::readLine(bool handleCR) {
	// Read a line
	String line;
	while (line.lastChar() != '\n') {
		char buf[256];
		if (!readLine(buf, 256, handleCR))
			break;
		line += buf;
	}

	if (line.lastChar() == '\n')
		line.deleteLastChar();

	return line;
}

uint32 SubReadStream::read(void *dataPtr, uint32 dataSize) {
	if (dataSize > _end - _pos) {
		dataSize = _end - _pos;
		_eos = true;
	}

	dataSize = _parentStream->read(dataPtr, dataSize);
	_pos += dataSize;

	return dataSize;
}

SeekableSubReadStream::SeekableSubReadStream(SeekableReadStream *parentStream, uint32 begin, uint32 end, DisposeAfterUse::Flag disposeParentStream)
	: SubReadStream(parentStream, end, disposeParentStream),
	_parentStream(parentStream),
	_begin(begin) {
	assert(_begin <= _end);
	_pos = _begin;
	_parentStream->seek(_pos);
	_eos = false;
}

bool SeekableSubReadStream::seek(int64 offset, int whence) {
	assert(_pos >= _begin);
	assert(_pos <= _end);

	switch (whence) {
	case SEEK_END:
		offset = size() + offset;
		// fallthrough
	case SEEK_SET:
		// Fall through
	default:
		_pos = _begin + offset;
		break;
	case SEEK_CUR:
		_pos += offset;
	}

	assert(_pos >= _begin);
	assert(_pos <= _end);

	bool ret = _parentStream->seek(_pos);
	if (ret) _eos = false; // reset eos on successful seek

	return ret;
}

uint32 SafeSeekableSubReadStream::read(void *dataPtr, uint32 dataSize) {
	// Make sure the parent stream is at the right position
	seek(0, SEEK_CUR);

	return SeekableSubReadStream::read(dataPtr, dataSize);
}

void SeekableReadStream::hexdump(int len, int bytesPerLine, int startOffset) {
	uint pos_ = pos();
	uint size_ = size();
	uint toRead = MIN<uint>(len + startOffset, size_ - pos_);
	byte *data = (byte *)calloc(toRead, 1);

	read(data, toRead);
	Common::hexdump(data, toRead, bytesPerLine, startOffset);

	free(data);

	seek(pos_);
}

#pragma mark -

namespace {

/**
 * Wrapper class which adds buffering to any given ReadStream.
 * Users can specify how big the buffer should be, and whether the
 * wrapped stream should be disposed when the wrapper is disposed.
 */
class BufferedReadStream : virtual public ReadStream {
protected:
	DisposablePtr<ReadStream> _parentStream;
	byte *_buf;
	uint32 _pos;
	bool _eos; // end of stream
	uint32 _bufSize;
	uint32 _realBufSize;

public:
	BufferedReadStream(ReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream);
	virtual ~BufferedReadStream();

	bool eos() const override { return _eos; }
	bool err() const override { return _parentStream->err(); }
	void clearErr() override { _eos = false; _parentStream->clearErr(); }

	uint32 read(void *dataPtr, uint32 dataSize) override;
};

BufferedReadStream::BufferedReadStream(ReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream)
	: _parentStream(parentStream, disposeParentStream),
	_pos(0),
	_eos(false),
	_bufSize(0),
	_realBufSize(bufSize) {

	assert(parentStream);
	_buf = new byte[bufSize];
	assert(_buf);
}

BufferedReadStream::~BufferedReadStream() {
	delete[] _buf;
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
		if (dataSize > _realBufSize) {
			uint32 n = _parentStream->read(dataPtr, dataSize);
			if (_parentStream->eos())
				_eos = true;

			// Fill the buffer from the user buffer so a seek back in
			// the stream into the buffered area brings consistent data.
			_bufSize = MIN(n, _realBufSize);
			_pos = _bufSize;
			memcpy(_buf, (byte *)dataPtr + n - _bufSize, _bufSize);

			return alreadyRead + n;
		}

		// Refill the buffer.
		// If we didn't read as many bytes as requested, the reason
		// is EOF or an error. In that case we truncate the buffer
		// size, as well as the number of  bytes we are going to
		// return to the caller.
		_bufSize = _parentStream->read(_buf, _realBufSize);
		_pos = 0;
		if (_bufSize < dataSize) {
			// we didn't get enough data from parent
			if (_parentStream->eos())
				_eos = true;
			dataSize = _bufSize;
		}
	}

	if (dataSize) {
		// Satisfy the request from the buffer
		memcpy(dataPtr, _buf + _pos, dataSize);
		_pos += dataSize;
	}
	return alreadyRead + dataSize;
}

} // End of anonymous namespace


ReadStream *wrapBufferedReadStream(ReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream) {
	if (parentStream)
		return new BufferedReadStream(parentStream, bufSize, disposeParentStream);
	return nullptr;
}

#pragma mark -

namespace {

/**
 * Wrapper class which adds buffering to any given SeekableReadStream.
 * @see BufferedReadStream
 */
class BufferedSeekableReadStream : public BufferedReadStream, public SeekableReadStream {
protected:
	SeekableReadStream *_parentStream;
public:
	BufferedSeekableReadStream(SeekableReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::NO);

	int64 pos() const override { return _parentStream->pos() - (_bufSize - _pos); }
	int64 size() const override { return _parentStream->size(); }

	bool seek(int64 offset, int whence = SEEK_SET) override;
};

BufferedSeekableReadStream::BufferedSeekableReadStream(SeekableReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream)
	: BufferedReadStream(parentStream, bufSize, disposeParentStream),
	_parentStream(parentStream) {
}

bool BufferedSeekableReadStream::seek(int64 offset, int whence) {
	// If it is a "local" seek, we may get away with "seeking" around
	// in the buffer only.
	_eos = false; // seeking always cancels EOS

	int relOffset = 0;
	switch (whence) {
	case SEEK_SET:
		relOffset = offset - pos();
		break;
	case SEEK_CUR:
		relOffset = offset;
		break;
	case SEEK_END:
		relOffset = (size() + offset) - pos();
		break;
	default:
		break;
	}

	if ((int)_pos + relOffset >= 0 && _pos + relOffset <= _bufSize) {
		_pos += relOffset;

		// Note: we do not need to reset parent's eos flag here. It is
		// sufficient that it is reset when actually seeking in the parent.
	} else {
		// Seek was not local enough, so we reset the buffer and
		// just seek normally in the parent stream.
		if (whence == SEEK_CUR)
			offset -= (_bufSize - _pos);
		// We invalidate the buffer here. This assures that successive seeks
		// do not have the chance to incorrectly think they seek'ed back into
		// the buffer.
		// Note: This does not take full advantage of the buffer. But it is
		// a simple way to prevent nasty errors. It would be possible to take
		// full advantage of the buffer by saving its actual start position.
		// This seems not worth the effort for this seemingly uncommon use.
		_pos = _bufSize = 0;
		_parentStream->seek(offset, whence);
	}

	return true;
}

} // End of anonymous namespace

SeekableReadStream *wrapBufferedSeekableReadStream(SeekableReadStream *parentStream, uint32 bufSize, DisposeAfterUse::Flag disposeParentStream) {
	if (parentStream)
		return new BufferedSeekableReadStream(parentStream, bufSize, disposeParentStream);
	return nullptr;
}

#pragma mark -

namespace {

/**
 * Wrapper class which adds buffering to any WriteStream.
 */
class BufferedWriteStream : public SeekableWriteStream {
protected:
	WriteStream *_parentStream;
	byte *_buf;
	uint32 _pos;
	const uint32 _bufSize;

	/**
	 * Write out the data in the buffer.
	 *
	 * @note This method is identical to flush() (which actually is
	 * implemented by calling this method), except that it is not
	 * virtual, hence there is less overhead calling it.
	 */
	bool flushBuffer() {
		const uint32 bytesToWrite = _pos;

		if (bytesToWrite) {
			_pos = 0;
			if (_parentStream->write(_buf, bytesToWrite) != bytesToWrite)
				return false;
		}
		return true;
	}

public:
	BufferedWriteStream(WriteStream *parentStream, uint32 bufSize)
		: _parentStream(parentStream),
		_pos(0),
		_bufSize(bufSize) {

		assert(parentStream);
		_buf = new byte[bufSize];
		assert(_buf);
	}

	virtual ~BufferedWriteStream() {
		const bool flushResult = flushBuffer();
		assert(flushResult);

		delete _parentStream;

		delete[] _buf;
	}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		// check if we have enough space for writing to the buffer
		if (_bufSize - _pos >= dataSize) {
			memcpy(_buf + _pos, dataPtr, dataSize);
			_pos += dataSize;
		} else if (_bufSize >= dataSize) {	// check if we can flush the buffer and load the data
			const bool flushResult = flushBuffer();
			assert(flushResult);
			memcpy(_buf, dataPtr, dataSize);
			_pos += dataSize;
		} else	{	// too big for our buffer
			const bool flushResult = flushBuffer();
			assert(flushResult);
			return _parentStream->write(dataPtr, dataSize);
		}
		return dataSize;
	}

	bool flush() override { return flushBuffer(); }

	int64 pos() const override { return _pos + _parentStream->pos(); }

	bool seek(int64 offset, int whence) override {
		flush();

		Common::SeekableWriteStream *sws =
			dynamic_cast<Common::SeekableWriteStream *>(_parentStream);
		return sws ? sws->seek(offset, whence) : false;
	}

	int64 size() const override {
		Common::SeekableWriteStream *sws =
			dynamic_cast<Common::SeekableWriteStream *>(_parentStream);
		return sws ? MAX(sws->pos() + _pos, sws->size()) : -1;
	}
};

} // End of anonymous namespace

SeekableWriteStream *wrapBufferedWriteStream(SeekableWriteStream *parentStream, uint32 bufSize) {
	if (parentStream)
		return new BufferedWriteStream(parentStream, bufSize);
	return nullptr;
}

WriteStream *wrapBufferedWriteStream(WriteStream *parentStream, uint32 bufSize) {
	if (parentStream)
		return new BufferedWriteStream(parentStream, bufSize);
	return nullptr;
}

uint32 SafeMutexedSeekableSubReadStream::read(void *dataPtr, uint32 dataSize) {
	Common::StackLock lock(_mutex);
	return Common::SafeSeekableSubReadStream::read(dataPtr, dataSize);
}

} // End of namespace Common
