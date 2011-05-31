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

// Disable symbol overrides so that we can use zlib.h
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/zlib.h"
#include "common/util.h"
#include "common/stream.h"

#if defined(USE_ZLIB)
  #ifdef __SYMBIAN32__
    #include <zlib\zlib.h>
  #else
    #include <zlib.h>
  #endif

  #if ZLIB_VERNUM < 0x1204
  #error Version 1.2.0.4 or newer of zlib is required for this code
  #endif
#endif


namespace Common {

#if defined(USE_ZLIB)

bool uncompress(byte *dst, unsigned long *dstLen, const byte *src, unsigned long srcLen) {
	return Z_OK == ::uncompress(dst, dstLen, src, srcLen);
}

/**
 * A simple wrapper class which can be used to wrap around an arbitrary
 * other SeekableReadStream and will then provide on-the-fly decompression support.
 * Assumes the compressed data to be in gzip format.
 */
class GZipReadStream : public Common::SeekableReadStream {
protected:
	enum {
		BUFSIZE = 16384		// 1 << MAX_WBITS
	};

	byte	_buf[BUFSIZE];

	Common::SeekableReadStream *_wrapped;
	z_stream _stream;
	int _zlibErr;
	uint32 _pos;
	uint32 _origSize;
	bool _eos;

public:

	GZipReadStream(Common::SeekableReadStream *w) : _wrapped(w) {
		assert(w != 0);

		_stream.zalloc = Z_NULL;
		_stream.zfree = Z_NULL;
		_stream.opaque = Z_NULL;

		// Verify file header is correct
		w->seek(0, SEEK_SET);
		uint16 header = w->readUint16BE();
		assert(header == 0x1F8B ||
		       ((header & 0x0F00) == 0x0800 && header % 31 == 0));

		if (header == 0x1F8B) {
			// Retrieve the original file size
			w->seek(-4, SEEK_END);
			_origSize = w->readUint32LE();
		} else {
			// Original size not available in zlib format
			_origSize = 0;
		}
		_pos = 0;
		w->seek(0, SEEK_SET);
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

	~GZipReadStream() {
		inflateEnd(&_stream);
		delete _wrapped;
	}

	bool err() const { return (_zlibErr != Z_OK) && (_zlibErr != Z_STREAM_END); }
	void clearErr() {
		// only reset _eos; I/O errors are not recoverable
		_eos = false;
	}

	uint32 read(void *dataPtr, uint32 dataSize) {
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

	bool eos() const {
		return _eos;
	}
	int32 pos() const {
		return _pos;
	}
	int32 size() const {
		return _origSize;
	}
	bool seek(int32 offset, int whence = SEEK_SET) {
		int32 newPos = 0;
		assert(whence != SEEK_END);	// SEEK_END not supported
		switch (whence) {
		case SEEK_SET:
			newPos = offset;
			break;
		case SEEK_CUR:
			newPos = _pos + offset;
		}

		assert(newPos >= 0);

		if ((uint32)newPos < _pos) {
			// To search backward, we have to restart the whole decompression
			// from the start of the file. A rather wasteful operation, best
			// to avoid it. :/
#if DEBUG
			warning("Backward seeking in GZipReadStream detected");
#endif
			_pos = 0;
			_wrapped->seek(0, SEEK_SET);
			_zlibErr = inflateReset(&_stream);
			if (_zlibErr != Z_OK)
				return false;	// FIXME: STREAM REWRITE
			_stream.next_in = _buf;
			_stream.avail_in = 0;
		}

		offset = newPos - _pos;

		// Skip the given amount of data (very inefficient if one tries to skip
		// huge amounts of data, but usually client code will only skip a few
		// bytes, so this should be fine.
		byte tmpBuf[1024];
		while (!err() && offset > 0) {
			offset -= read(tmpBuf, MIN((int32)sizeof(tmpBuf), offset));
		}

		_eos = false;
		return true;	// FIXME: STREAM REWRITE
	}
};

/**
 * A simple wrapper class which can be used to wrap around an arbitrary
 * other WriteStream and will then provide on-the-fly compression support.
 * The compressed data is written in the gzip format.
 */
class GZipWriteStream : public Common::WriteStream {
protected:
	enum {
		BUFSIZE = 16384		// 1 << MAX_WBITS
	};

	byte	_buf[BUFSIZE];
	Common::WriteStream *_wrapped;
	z_stream _stream;
	int _zlibErr;

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
	GZipWriteStream(Common::WriteStream *w) : _wrapped(w) {
		assert(w != 0);
		_stream.zalloc = Z_NULL;
		_stream.zfree = Z_NULL;
		_stream.opaque = Z_NULL;

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
		_stream.next_in = 0;
	}

	~GZipWriteStream() {
		finalize();
		deflateEnd(&_stream);
		delete _wrapped;
	}

	bool err() const {
		// CHECKME: does Z_STREAM_END make sense here?
		return (_zlibErr != Z_OK && _zlibErr != Z_STREAM_END) || _wrapped->err();
	}

	void clearErr() {
		// Note: we don't reset the _zlibErr here, as it is not
		// clear in general how
		_wrapped->clearErr();
	}

	void finalize() {
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

	uint32 write(const void *dataPtr, uint32 dataSize) {
		if (err())
			return 0;

		// Hook in the new data ...
		// Note: We need to make a const_cast here, as zlib is not aware
		// of the const keyword.
		_stream.next_in = const_cast<byte *>((const byte *)dataPtr);
		_stream.avail_in = dataSize;

		// ... and flush it to disk
		processData(Z_NO_FLUSH);

		return dataSize - _stream.avail_in;
	}
};

#endif	// USE_ZLIB

Common::SeekableReadStream *wrapCompressedReadStream(Common::SeekableReadStream *toBeWrapped) {
#if defined(USE_ZLIB)
	if (toBeWrapped) {
		uint16 header = toBeWrapped->readUint16BE();
		bool isCompressed = (header == 0x1F8B ||
				     ((header & 0x0F00) == 0x0800 &&
				      header % 31 == 0));
		toBeWrapped->seek(-2, SEEK_CUR);
		if (isCompressed)
			return new GZipReadStream(toBeWrapped);
	}
#endif
	return toBeWrapped;
}

Common::WriteStream *wrapCompressedWriteStream(Common::WriteStream *toBeWrapped) {
#if defined(USE_ZLIB)
	if (toBeWrapped)
		return new GZipWriteStream(toBeWrapped);
#endif
	return toBeWrapped;
}


}	// End of namespace Common
