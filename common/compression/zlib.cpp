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

// Disable symbol overrides so that we can use zlib.h
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/compression/zlib.h"
#include "common/ptr.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/textconsole.h"

#if defined(USE_ZLIB)
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
#endif


namespace Common {

#if defined(USE_ZLIB)

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

#endif	// USE_ZLIB

WriteStream *wrapCompressedWriteStream(WriteStream *toBeWrapped) {
#if defined(USE_ZLIB)
	if (toBeWrapped)
		return new GZipWriteStream(toBeWrapped);
#endif
	return toBeWrapped;
}


} // End of namespace Common
