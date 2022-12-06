/* gzio.c - decompression support for gzip */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2005,2006,2007,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Most of this file was originally the source file "inflate.c", written
 * by Mark Adler.  It has been very heavily modified.  In particular, the
 * original would run through the whole file at once, and this version can
 * be stopped and restarted on any boundary during the decompression process.
 *
 * The license and header comments that file are included here.
 */

/* inflate.c -- Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 */

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.
 */

#ifndef COMMON_GZIO_H
#define COMMON_GZIO_H 1

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/ptr.h"

namespace Common {

/* The state stored in filesystem-specific data.  */
class GzioReadStream : public Common::SeekableReadStream
{
public:
	static GzioReadStream* openClickteam(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);
	static GzioReadStream* openDeflate(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);
	static GzioReadStream* openDeflateWithDict(Common::SeekableReadStream *parent, uint64 uncompressed_size, const byte *dict, uint32 dict_size,
						   DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);
	static GzioReadStream* openVise(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);
	static GzioReadStream* openZlib(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);
	static int32 clickteamDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off = 0);
	static int32 deflateDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off = 0);
	static int32 deflateDecompressWithDict (byte *outbuf, uint32 outsize, const byte *inbuf, uint32 insize, const byte *dict, uint32 dict_size, int64 off = 0);
	static int32 viseDecompress (byte *outbuf, uint32 outsize, const byte *inbuf, uint32 insize, int64 off = 0);
	static int32 zlibDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off = 0);
	static GzioReadStream* openGzip(Common::SeekableReadStream *parent, DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);
	static GzioReadStream* openZlibOrGzip(Common::SeekableReadStream *parent, uint64 uncompressed_size = kUnknownSize, DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);

        /**
	 * Wrapper around gzio's deflate functions. This function is used by Glk to
	 * decompress TAF 4.0 files, which are headerless Zlib compressed streams with a
	 * custom header
	 *
	 * @param dst       the destination stream to write decompressed data out to
	 * @param src       the Source stream
	 *
	 * @return true on success, false otherwise.
	 */
	static bool inflateZlibHeaderless(Common::WriteStream *dst, Common::SeekableReadStream *src);

	/**
	 * Take an arbitrary SeekableReadStream and wrap it in a custom stream which
	 * provides transparent on-the-fly decompression. Assumes the data it
	 * retrieves from the wrapped stream to be either uncompressed or in gzip
	 * format. In the former case, the original stream is returned unmodified
	 * (and in particular, not wrapped). In the latter case the stream is
	 * returned wrapped
	 *
	 * Certain GZip-formats don't supply an easily readable length, if you
	 * still need the length carried along with the stream, and you know
	 * the decompressed length at wrap-time, then it can be supplied as knownSize
	 * here. knownSize will be ignored if the GZip-stream DOES include a length.
	 * The created stream also becomes responsible for freeing the passed stream.
	 *
	 * It is safe to call this with a NULL parameter (in this case, NULL is
	 * returned).
	 *
	 * @param toBeWrapped	the stream to be wrapped (if it is in gzip-format)
	 * @param knownSize		a supplied length of the compressed data (if not available directly)
	 */
	static SeekableReadStream *wrapCompressedReadStream(SeekableReadStream *toBeWrapped, uint64 knownSize = kUnknownSize);

	int32 readAtOffset(int64 offset, byte *buf, uint32 len);
	bool readWhole (Common::WriteStream *dst);

	uint32 read(void *dataPtr, uint32 dataSize) override;

	bool eos() const override { return _eos; }
	bool err() const override { return _err; }
	void clearErr() override { _eos = false; _err = false; }

	int64 pos() const override { return _streamPos; }
	int64 size() const override { return _uncompressedSize; }

	bool seek(int64 offs, int whence = SEEK_SET) override;

	static const uint64 kUnknownSize = 0x7fffffffffffffff;

private:
  /*
   *  Window Size
   *
   *  This must be a power of two, and at least 32K for zip's deflate method
   */

	static const int WSIZE = 0x8000;
	static const int INBUFSIZ = 0x2000;

	/* If input is in memory following fields are used instead of file.  */
	Common::DisposablePtr<Common::SeekableReadStream> _input;
	/* The offset at which the data starts in the underlying file.  */
	int64 _dataOffset;
	/* The type of current block.  */
	int _blockType;
	/* The length of current block.  */
	int _blockLen;
	/* The flag of the last block.  */
	int _lastBlock;
	/* The flag of codes.  */
	int _codeState;
	/* The length of a copy.  */
	unsigned _inflateN;
	/* The index of a copy.  */
	unsigned _inflateD;
	/* The bit buffer.  */
	unsigned long _bb;
	/* The bits in the bit buffer.  */
	unsigned _bk;
	/* The sliding window in uncompressed data.  */
	uint8 _slide[WSIZE];
	/* Current position in the slide.  */
	unsigned _wp;
	/* The literal/length code table.  */
	struct huft *_tl;
	/* The distance code table.  */
	struct huft *_td;
	/* The lookup bits for the literal/length code table. */
	int _bl;
	/* The lookup bits for the distance code table.  */
	int _bd;
	/* The original offset value.  */
	int64 _savedOffset;
	/* The supplied dictionary.  */
	uint8 _dict[WSIZE];
	uint32 _dict_size;
	uint32 _orig_crc32;

	bool _err;

	/* The input buffer.  */
	byte _inbuf[INBUFSIZ];
	int _inbufD;
	int _inbufSize;
	uint64 _uncompressedSize;
	uint64 _streamPos;
	bool _eos;

	enum class Mode { ZLIB, CLICKTEAM, VISE } _mode;

        GzioReadStream(Common::SeekableReadStream *parent, DisposeAfterUse::Flag disposeParent, uint64 uncompressedSize, Mode mode, const byte *dict = nullptr, uint32 dict_size = 0) :
	  _dataOffset(0), _blockType(0), _blockLen(0), _dict_size(MIN<uint32>(dict_size, sizeof(_dict))),
	  _lastBlock(0), _codeState (0), _inflateN(0),
	  _inflateD(0), _bb(0), _bk(0), _wp(0), _tl(nullptr),
	  _td(nullptr), _bl(0),
	  _bd(0), _savedOffset(0), _err(false), _mode(mode), _input(parent, disposeParent),
	  _inbufD(0), _inbufSize(0), _uncompressedSize(uncompressedSize), _streamPos(0), _eos(false) {
		memcpy(_dict, dict + (_dict_size - dict_size), dict_size);
	}

	void inflate_window();
	void initialize_tables();
	static bool test_zlib_header(Common::SeekableReadStream *stream);
	void init_zlib();
	static bool test_gzip_header (Common::SeekableReadStream *stream);
	void init_gzip();
	void get_new_block();
	byte parentGetByte();
	uint16 parentGetLE16 ();
	void parentSeek(int64 off);
	uint64 parentTell();
	void init_fixed_block();
	int inflate_codes_in_window();
	void init_dynamic_block ();
	void init_stored_block ();
	void eat_bytes (int len);
	void eat_string ();
};

}

#endif
