
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
#include "bagel/boflib/bof_error.h"
#include "bagel/boflib/bof_file.h"

namespace Bagel {

#define MAXFILELENGTH 80

#define REGISTER
#ifndef USHRT_MAX
#define USHRT_MAX 0xffff
#endif
#ifndef SHRT_MAX
#define SHRT_MAX 0x7fff
#endif
#ifndef UCHAR_MAX
#define UCHAR_MAX 0xff
#endif

#define ERR_ENDFILE 14
#define ERR_SKIP 15

/****************************************
 * definitions/enumerations             *
 ****************************************/

// I/O buffer for reading and writing
#define MAXFILEBUF 8192

#define CHAR_BIT 8

// Bitwise Finction info
#define USHRT_BITS (sizeof(USHORT) * CHAR_BIT)

/*
 * Huffman binary tree - used by implode and deflate
 */
struct HBINTREE {
	struct HBINTREE *left; /* left child for huffman coded tree */
	union {
		struct HBINTREE *right; /* right child for huffman coded tree */
		SHORT value;            /* value for current node in huffman tree */
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
	ULONG zfCrcValue; /* Current file's Uncompressed CRC value */

	LONG fileSize;
	LONG dataSize;

	USHORT zfgpFlags; /* General Purpose Flags */

	CBofFile *zipFile; /* handle of ZIP file */
	CBofFile *auxFile; /* handle of file being written */

	/*
	 * bit operations for BitWrite, BitRead, BitFlush and BitSync
	 */
	struct BITVAR {
		LONG fileSize; /* file's compressed size */
		UBYTE *bufPtr; /* Pointer to bitBuffer */
		UBYTE *bufEnd; /* Pointer to end bitBuffer */
		USHORT last;   /* last word shifted down */
		UBYTE bitCnt;  /* Bit offset in bitBuffer */
	} bitVar;

	/*
	 * buffered I/O operations
	 */
	struct BUFVAR {
		UBYTE *bufPtr; /* pos within buffer */
		UBYTE *bufEnd; /* end of buffer */
	} bufVar;

	/*
	 * compression method globals - these should always be reset to NULLs
	 */

	/*
	 * for deflate and inflate
	 */
	struct DFSTRUCT deflate;

	CHAR zfFileName[MAXFILELENGTH + 1];

	union {
		/* double buffer used for Store, Extract, and Ecnrypt */
		UBYTE bigBuf[MAXFILEBUF * 2];

		struct {
			/* input Buffer to be used for Buffered I/O */
			UBYTE inBuffer[MAXFILEBUF];

			/* output Buffer to be used for Buffered I/O */
			UBYTE outBuffer[MAXFILEBUF];
		} s;
	} u;
};

/*
 * prototypes
 */
extern VOID BitReadInit(UBYTE *pInBuf, INT nBufSize);
extern ERROR_CODE BitRead(USHORT *bitPtr, SHORT bitCnt);
extern ERROR_CODE BitReadQuick(USHORT *bitPtr, UBYTE codeSize);
extern ERROR_CODE BitReadQuick1(USHORT *bitPtr);
extern ERROR_CODE BitReadBytes(UBYTE *buffer, SHORT size, UBYTE *pInBuf, INT nBufSize);

extern VOID BitWriteInit(VOID);
extern LONG BitWriteSize(VOID);
extern ERROR_CODE BitWrite(USHORT *bitPtr, SHORT bitCnt);
extern ERROR_CODE BitAltFlush(VOID);
extern ERROR_CODE BitWriteFlush(LONG *rFileSize);
extern ERROR_CODE BitWriteBytes(UBYTE *buffer, SHORT size);
extern ERROR_CODE BitWriteQuick(USHORT *bitPtr, UBYTE codeSize);

extern VOID BufReadInit(UBYTE *pInBuf, INT nBufSize);
extern ERROR_CODE BufRead(UBYTE *buffer, SHORT size, SHORT *rSize);
extern ERROR_CODE BufReadQuick(UBYTE *data);
extern ERROR_CODE BufReadStrQuick(UBYTE *data, SHORT len, SHORT *rLen);

#define zg g_engine->_zg

} // namespace Bagel

#endif
