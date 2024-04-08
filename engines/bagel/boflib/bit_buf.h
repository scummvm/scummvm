
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

#ifndef BAGEL_BOFLIB_BIT_BUF_H
#define BAGEL_BOFLIB_BIT_BUF_H

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/file.h"

namespace Bagel {

#define MAXFILELENGTH 80

#define ERR_ENDFILE 14
#define ERR_SKIP 15

/****************************************
 * definitions/enumerations             *
 ****************************************/

// I/O buffer for reading and writing
#define MAXFILEBUF 8192

#define BITS_IN_CHAR 8

// Bitwise Finction info
#define USHRT_BITS (sizeof(uint16) * BITS_IN_CHAR)

/*
 * Huffman binary tree - used by implode and deflate
 */
struct HBINTREE {
	struct HBINTREE *left; /* left child for huffman coded tree */
	union {
		struct HBINTREE *right; /* right child for huffman coded tree */
		int16 value;            /* value for current node in huffman tree */
	};
};

struct DFSTRUCT {
	/* fixed S-F literal tree */
	struct HBINTREE *fhLitTree;

	/* dynamic S-F literal tree */
	struct HBINTREE *dhLitTree;

	/* fixed S-F distance tree */
	struct HBINTREE *fhDisTree;

	/* dynamic S-F distance tree */
	struct HBINTREE *dhDisTree;
};

/*
 * Global Variables used by Zip routines
 */
struct ZIPGLOBAL {
	uint32 zfCrcValue; /* Current file's Uncompressed CRC value */

	int32 fileSize;
	int32 dataSize;

	uint16 zfgpFlags; /* General Purpose Flags */

	CBofFile *zipFile; /* handle of ZIP file */
	CBofFile *auxFile; /* handle of file being written */

	/*
	 * bit operations for BitWrite, BitRead, BitFlush and BitSync
	 */
	struct BITVAR {
		int32 fileSize; /* file's compressed size */
		byte *bufPtr; /* Pointer to bitBuffer */
		byte *bufEnd; /* Pointer to end bitBuffer */
		uint16 last;   /* last word shifted down */
		byte bitCnt;  /* Bit offset in bitBuffer */
	} bitVar;

	/*
	 * buffered I/O operations
	 */
	struct BUFVAR {
		byte *bufPtr; /* pos within buffer */
		byte *bufEnd; /* end of buffer */
	} bufVar;

	/*
	 * compression method globals - these should always be reset to NULLs
	 */

	/*
	 * for deflate and inflate
	 */
	struct DFSTRUCT deflate;

	char zfFileName[MAXFILELENGTH + 1];

	union {
		/* double buffer used for Store, Extract, and Ecnrypt */
		byte bigBuf[MAXFILEBUF * 2];

		struct {
			/* input Buffer to be used for Buffered I/O */
			byte inBuffer[MAXFILEBUF];

			/* output Buffer to be used for Buffered I/O */
			byte outBuffer[MAXFILEBUF];
		} s;
	} u;
};

/*
 * prototypes
 */
extern void BitReadInit(byte *pInBuf, int nBufSize);
extern ErrorCode BitRead(uint16 *bitPtr, int16 bitCnt);
extern ErrorCode BitReadQuick(uint16 *bitPtr, byte codeSize);
extern ErrorCode BitReadQuick1(uint16 *bitPtr);
extern ErrorCode BitReadBytes(byte *buffer, int16 size, byte *pInBuf, int nBufSize);

extern void BitWriteInit();
extern int32 BitWriteSize();
extern ErrorCode BitWrite(uint16 *bitPtr, int16 bitCnt);
extern ErrorCode BitAltFlush();
extern ErrorCode BitWriteFlush(int32 *rFileSize);
extern ErrorCode BitWriteBytes(byte *buffer, int16 size);
extern ErrorCode BitWriteQuick(uint16 *bitPtr, byte codeSize);

extern void BufReadInit(byte *pInBuf, int nBufSize);
extern ErrorCode BufRead(byte *buffer, int16 size, int16 *rSize);
extern ErrorCode BufReadQuick(byte *data);
extern ErrorCode BufReadStrQuick(byte *data, int16 len, int16 *rLen);

#define zg g_engine->_zg

} // namespace Bagel

#endif
