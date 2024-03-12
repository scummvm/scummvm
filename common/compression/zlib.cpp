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

#ifdef __MORPHOS__
  #define _NO_PPCINLINE
  #include <zlib.h>
  #undef _NO_PPCINLINE
#else
  #include <zlib.h>
#endif

#if ZLIB_VERNUM < 0x1204
#error Version 1.2.0.4 or newer of zlib is required for this code
#endif

#include "common/compression/deflate.h"

#include "common/ptr.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/textconsole.h"


namespace Common {

bool inflateZlib(byte *dst, unsigned long *dstLen, const byte *src, unsigned long srcLen) {
	return Z_OK == uncompress(dst, dstLen, src, srcLen);
}

bool inflateZlibHeaderless(byte *dst, uint *dstLen, const byte *src, uint srcLen, const byte *dict, uint dictLen) {
	if (!dst || !dstLen || !*dstLen || !src || !srcLen)
		return false;

	// Initialize zlib
	z_stream stream;
	stream.next_in = const_cast<byte *>(src);
	stream.avail_in = srcLen;
	stream.next_out = dst;
	stream.avail_out = *dstLen;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	// Negative MAX_WBITS tells zlib there's no zlib header
	int err = inflateInit2(&stream, -MAX_WBITS);
	if (err != Z_OK)
		return false;

	// Set the dictionary, if provided
	if (dict != nullptr) {
		err = inflateSetDictionary(&stream, const_cast<byte *>(dict), dictLen);
		if (err != Z_OK)
			return false;
	}

	err = inflate(&stream, Z_SYNC_FLUSH);
	if (err != Z_OK && err != Z_STREAM_END) {
		inflateEnd(&stream);
		return false;
	}

	inflateEnd(&stream);
	*dstLen = *dstLen - stream.avail_out;
	return true;
}

#ifndef RELEASE_BUILD
static bool _shownBackwardSeekingWarning = false;
#endif

/**
 * A simple wrapper class which can be used to wrap around an arbitrary
 * other SeekableReadStream and will then provide on-the-fly decompression support.
 * Assumes the compressed data to be in gzip format.
 */
class GZipReadStream : public SeekableReadStream {
protected:
	enum {
		BUFSIZE = 16384		// 1 << MAX_WBITS
	};

	byte	_buf[BUFSIZE];

	DisposablePtr<SeekableReadStream> _wrapped;
	z_stream _stream;
	int _zlibErr;
	uint64 _parentPos;
	uint32 _pos;
	uint32 _origSize;
	bool _eos;

public:

	GZipReadStream(SeekableReadStream *w, DisposeAfterUse::Flag disposeParent, uint32 knownSize) : _wrapped(w, disposeParent), _stream() {
		assert(w != nullptr);

		_parentPos = w->pos();
		// Verify file header is correct
		uint16 header = w->readUint16BE();
		assert(header == 0x1F8B ||
		       ((header & 0x0F00) == 0x0800 && header % 31 == 0));

		if (header == 0x1F8B) {
			// Retrieve the original file size
			w->seek(-4, SEEK_END);
			_origSize = w->readUint32LE();
		} else {
			// Original size not available in zlib format
			// use an otherwise known size if supplied.
			_origSize = knownSize;
		}
		w->seek(_parentPos, SEEK_SET);
		_pos = 0;
		_eos = false;

		// Adding 32 to windowBits indicates to zlib that it is supposed to
		// automatically detect whether gzip or zlib headers are used for
		// the compressed file. This feature was added in zlib 1.2.0.4,
		// released 10 August 2003.
		// Note: This is *crucial* for savegame compatibility, do *not* remove!
		_zlibErr = inflateInit2(&_stream, MAX_WBITS + 32);
		if (_zlibErr != Z_OK)
			return;

		// Setup input buffer
		_stream.next_in = _buf;
		_stream.avail_in = 0;
	}

	GZipReadStream(SeekableReadStream *w, DisposeAfterUse::Flag disposeParent, uint32 knownSize, const byte *dict, uint dictLen) : _wrapped(w, disposeParent), _stream() {
		assert(w != nullptr);

		_parentPos = w->pos();
		// This is headerless deflate
		// Original size not available
		// use an otherwise known size if supplied.
		_origSize = knownSize;
		_pos = 0;
		_eos = false;

		_zlibErr = inflateInit2(&_stream, -MAX_WBITS);
		if (_zlibErr != Z_OK)
			return;

		// Set the dictionary, if provided
		if (dict != nullptr && dictLen > 0) {
			_zlibErr = inflateSetDictionary(&_stream, const_cast<byte *>(dict), dictLen);
			if (_zlibErr != Z_OK)
				return;
		}

		// Setup input buffer
		_stream.next_in = _buf;
		_stream.avail_in = 0;
	}

	~GZipReadStream() {
		inflateEnd(&_stream);
	}

	bool err() const override { return (_zlibErr != Z_OK) && (_zlibErr != Z_STREAM_END); }
	void clearErr() override {
		// only reset _eos; I/O errors are not recoverable
		_eos = false;
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		_stream.next_out = (byte *)dataPtr;
		_stream.avail_out = dataSize;

		// Keep going while we get no error
		while (_zlibErr == Z_OK && _stream.avail_out) {
			if (_stream.avail_in == 0 && !_wrapped->eos()) {
				// If we are out of input data: Read more data, if available.
				_stream.next_in = _buf;
				_stream.avail_in = _wrapped->read(_buf, BUFSIZE);
			}
			_zlibErr = inflate(&_stream, Z_NO_FLUSH);
		}

		// Update the position counter
		_pos += dataSize - _stream.avail_out;

		if (_zlibErr == Z_STREAM_END && _stream.avail_out > 0)
			_eos = true;

		return dataSize - _stream.avail_out;
	}

	bool eos() const override {
		return _eos;
	}
	int64 pos() const override {
		return _pos;
	}
	int64 size() const override {
		return _origSize;
	}
	bool seek(int64 offset, int whence = SEEK_SET) override {
		int32 newPos = 0;
		switch (whence) {
		default:
			// fallthrough intended
		case SEEK_SET:
			newPos = offset;
			break;
		case SEEK_CUR:
			newPos = _pos + offset;
			break;
		case SEEK_END:
			// NOTE: This can be an expensive operation (see below).
			newPos = size() + offset;
			break;
		}

		assert(newPos >= 0);

		if ((uint32)newPos < _pos) {
			// To search backward, we have to restart the whole decompression
			// from the start of the file. A rather wasteful operation, best
			// to avoid it. :/

#ifndef RELEASE_BUILD
			if (!_shownBackwardSeekingWarning) {
				// We only throw this warning once per stream, to avoid
				// getting the console swarmed with warnings when consecutive
				// seeks are made.
				debug(1, "Backward seeking in GZipReadStream detected");
				_shownBackwardSeekingWarning = true;
			}
#endif

			_pos = 0;
			_wrapped->seek(_parentPos, SEEK_SET);
			_zlibErr = inflateReset(&_stream);
			if (_zlibErr != Z_OK)
				return false; // FIXME: STREAM REWRITE
			_stream.next_in = _buf;
			_stream.avail_in = 0;
		}

		offset = newPos - _pos;

		// Skip the given amount of data (very inefficient if one tries to skip
		// huge amounts of data, but usually client code will only skip a few
		// bytes, so this should be fine.
		byte tmpBuf[1024];
		while (!err() && offset > 0) {
			offset -= read(tmpBuf, MIN((int64)sizeof(tmpBuf), offset));
		}

		_eos = false;
		return true; // FIXME: STREAM REWRITE
	}
};

/**
 * A simple wrapper class which can be used to wrap around an arbitrary
 * other WriteStream and will then provide on-the-fly compression support.
 * The compressed data is written in the gzip format.
 */
class GZipWriteStream : public WriteStream {
protected:
	enum {
		BUFSIZE = 16384		// 1 << MAX_WBITS
	};

	byte	_buf[BUFSIZE];
	ScopedPtr<WriteStream> _wrapped;
	z_stream _stream;
	int _zlibErr;
	uint32 _pos;

	void processData(int flushType) {
		// This function is called by both write() and finalize().
		while (_zlibErr == Z_OK && (_stream.avail_in || flushType == Z_FINISH)) {
			if (_stream.avail_out == 0) {
				if (_wrapped->write(_buf, BUFSIZE) != BUFSIZE) {
					_zlibErr = Z_ERRNO;
					break;
				}
				_stream.next_out = _buf;
				_stream.avail_out = BUFSIZE;
			}
			_zlibErr = deflate(&_stream, flushType);
		}
	}

public:
	GZipWriteStream(WriteStream *w) : _wrapped(w), _stream(), _pos(0) {
		assert(w != nullptr);

		// Adding 16 to windowBits indicates to zlib that it is supposed to
		// write gzip headers. This feature was added in zlib 1.2.0.4,
		// released 10 August 2003.
		// Note: This is *crucial* for savegame compatibility, do *not* remove!
		_zlibErr = deflateInit2(&_stream,
		                 Z_DEFAULT_COMPRESSION,
		                 Z_DEFLATED,
		                 MAX_WBITS + 16,
		                 8,
				 Z_DEFAULT_STRATEGY);
		assert(_zlibErr == Z_OK);

		_stream.next_out = _buf;
		_stream.avail_out = BUFSIZE;
		_stream.avail_in = 0;
		_stream.next_in = nullptr;
	}

	~GZipWriteStream() {
		finalize();
		deflateEnd(&_stream);
	}

	bool err() const override {
		// CHECKME: does Z_STREAM_END make sense here?
		return (_zlibErr != Z_OK && _zlibErr != Z_STREAM_END) || _wrapped->err();
	}

	void clearErr() override {
		// Note: we don't reset the _zlibErr here, as it is not
		// clear in general how
		_wrapped->clearErr();
	}

	void finalize() override {
		if (_zlibErr != Z_OK)
			return;

		// Process whatever remaining data there is.
		processData(Z_FINISH);

		// Since processData only writes out blocks of size BUFSIZE,
		// we may have to flush some stragglers.
		uint remainder = BUFSIZE - _stream.avail_out;
		if (remainder > 0) {
			if (_wrapped->write(_buf, remainder) != remainder) {
				_zlibErr = Z_ERRNO;
			}
		}

		// Finalize the wrapped savefile, too
		_wrapped->finalize();
	}

	uint32 write(const void *dataPtr, uint32 dataSize) override {
		if (err())
			return 0;

		// Hook in the new data ...
		// Note: We need to make a const_cast here, as zlib is not aware
		// of the const keyword.
		_stream.next_in = const_cast<byte *>((const byte *)dataPtr);
		_stream.avail_in = dataSize;

		// ... and flush it to disk
		processData(Z_NO_FLUSH);

		_pos += dataSize - _stream.avail_in;
		return dataSize - _stream.avail_in;
	}

	int64 pos() const override { return _pos; }
};

SeekableReadStream *wrapCompressedReadStream(SeekableReadStream *toBeWrapped, DisposeAfterUse::Flag disposeParent, uint64 knownSize) {
	if (!toBeWrapped) {
		return nullptr;
	}

	if (toBeWrapped->eos() || toBeWrapped->err() || toBeWrapped->size() < 2) {
		if (disposeParent == DisposeAfterUse::YES) {
			delete toBeWrapped;
		}
		return nullptr;
	}

	uint16 header = toBeWrapped->readUint16BE();
	bool isCompressed = (header == 0x1F8B ||
			     ((header & 0x0F00) == 0x0800 &&
			      header % 31 == 0));
	toBeWrapped->seek(-2, SEEK_CUR);
	if (isCompressed) {
		return new GZipReadStream(toBeWrapped, disposeParent, knownSize);
	}
	return toBeWrapped;
}

SeekableReadStream *wrapDeflateReadStream(SeekableReadStream *toBeWrapped, DisposeAfterUse::Flag disposeParent, uint64 knownSize, const byte *dict, uint dictLen) {
	if (!toBeWrapped) {
		return nullptr;
	}

	if (toBeWrapped->eos() || toBeWrapped->err()) {
		if (disposeParent == DisposeAfterUse::YES) {
			delete toBeWrapped;
		}
		return nullptr;
	}
	return new GZipReadStream(toBeWrapped, disposeParent, knownSize, dict, dictLen);
}

WriteStream *wrapCompressedWriteStream(WriteStream *toBeWrapped) {
	if (!toBeWrapped)
		return nullptr;
	return new GZipWriteStream(toBeWrapped);
}

} // End of namespace Common
