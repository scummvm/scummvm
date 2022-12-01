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
	static GzioReadStream* openZlib(Common::SeekableReadStream *parent, uint64 uncompressed_size, DisposeAfterUse::Flag disposeParent = DisposeAfterUse::NO);
	static int32 clickteamDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off = 0);
	static int32 deflateDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off = 0);
	static int32 zlibDecompress (byte *outbuf, uint32 outsize, byte *inbuf, uint32 insize, int64 off = 0);
	int32 readAtOffset(int64 offset, byte *buf, uint32 len);

	uint32 read(void *dataPtr, uint32 dataSize) override;

	bool eos() const override { return _eos; }
	bool err() const override { return _err; }
	void clearErr() override { _eos = false; _err = false; }

	int64 pos() const override { return _streamPos; }
	int64 size() const override { return _uncompressedSize; }

	bool seek(int64 offs, int whence = SEEK_SET) override;

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

	bool _err;

	/* The input buffer.  */
	byte _inbuf[INBUFSIZ];
	int _inbufD;
	int _inbufSize;
	uint64 _uncompressedSize;
	uint64 _streamPos;
	bool _eos;

	enum class Mode { ZLIB, CLICKTEAM } _mode;

        GzioReadStream(Common::SeekableReadStream *parent, DisposeAfterUse::Flag disposeParent, uint64 uncompressedSize, Mode mode) :
	  _dataOffset(0), _blockType(0), _blockLen(0),
	  _lastBlock(0), _codeState (0), _inflateN(0),
	  _inflateD(0), _bb(0), _bk(0), _wp(0), _tl(nullptr),
	  _td(nullptr), _bl(0),
	  _bd(0), _savedOffset(0), _err(false), _mode(mode), _input(parent, disposeParent),
	  _inbufD(0), _inbufSize(0), _uncompressedSize(uncompressedSize), _streamPos(0), _eos(false) {}

	void inflate_window();
	void initialize_tables();
	bool test_zlib_header();
	void get_new_block();
	byte parentGetByte();
	void parentSeek(int64 off);
	void init_fixed_block();
	int inflate_codes_in_window();
	void init_dynamic_block ();
	void init_stored_block ();
};

}
