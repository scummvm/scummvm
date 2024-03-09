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

#include "bagel/boflib/deflate.h"
#include "bagel/boflib/bit_buf.h"
#include "bagel/boflib/crc.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/misc.h"
#include "bagel/bagel.h"

namespace Bagel {

/*
 * Deflate constants
 */
#define DF_MAXCODES 288
/* maximum # of all codes */
#define DF_LITCODES 288
/* maximum # of literal codes */
#define DF_LITCODESUSED (DF_LITCODES - 2)
#define DF_DISCODES 32 /* maximum # of distance codes */
#define DF_DISCODESUSED (DF_DISCODES - 2)
#define DF_BITCODES 19   /* maximum # of bit length codes */
#define DF_ENDBLOCK 256  /* end of block marker for deflate */
#define DF_MAXLENGTH 258 /* maximum copy length */
#define DF_STORED 0      /* block is stored */
#define DF_FIXED 1       /* block uses fixed codes */
#define DF_DYNAMIC 2     /* block uses dynamic codes */
#define DF_RESERVED 3    /* reserved - Error */

/* local constants */
#define WINDSIZE 32768U
#define WINDMASK (WINDSIZE - 1)
#define HASHSIZE 32768U
#define HASHMASK (HASHSIZE - 1)
#define REPFLAG (1U << 15)
#define STACKSIZE (15 * 1024)
#define FIRSTREAD (DF_MAXLENGTH + (DF_MAXLENGTH - 2))
#define LASTPASS (pass == 1)

/* hash function */
#define HASHFUNC(a, b, c) ((USHORT)(((USHORT)c << 7) ^ ((USHORT)b << 4) ^ a))

/*
 * values + lengths before Shannon-Fanno code generation
 */
struct CODELENS {
	SHORT val;  /* value that this entry represents */
	UBYTE bits; /* Bit size of this entry */
};

struct CODETABLE {
	USHORT code; /* Code for this entry in the code table */
	UBYTE bits;  /* number of bits needed for 'code' */
};

struct PACKTABLE {
	UBYTE bits;
	UBYTE extraBits;
	UBYTE extraLen;
};

/*
 * local structures for code stacking
 */
struct CODESTACK {
	USHORT offset;
	UBYTE idxLen;
	UBYTE idxOff;
};
struct ALTCODESTACK {
	UBYTE lit1;
	UBYTE lit2;
	UBYTE lit3;
	UBYTE length;
};

/*
 * table of extra bits needed for lengths
 */
struct CODETABLE lenExtraTable[] = {
	{3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0},
	{11, 1}, {13, 1}, {15, 1}, {17, 1}, {19, 2}, {23, 2}, {27, 2}, {31, 2},
	{35, 3}, {43, 3}, {51, 3}, {59, 3}, {67, 4}, {83, 4}, {99, 4}, {115, 4},
	{131, 5}, {163, 5}, {195, 5}, {227, 5}, {258, 0},
};

/*
 * table of extra bits needed for distances
 */
struct CODETABLE disExtraTable[] = {
	{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 1}, {7, 1}, {9, 2}, {13, 2}, {17, 3},
	{25, 3}, {33, 4}, {49, 4}, {65, 5}, {97, 5}, {129, 6}, {193, 6}, {257, 7},
	{385, 7}, {513, 8}, {769, 8}, {1025, 9}, {1537, 9}, {2049, 10}, {3073, 10},
	{4097, 11}, {6145, 11}, {8193, 12}, {12289, 12}, {16385, 13}, {24577, 13},
};

/*
 * fixed bit lengths for Shannon-Fano codes
 * <val, bits>
 */
struct CODELENS bitTreeTable[] = {
	{16, 0}, {17, 0}, {18, 0}, {0, 0}, {8, 0},  {7, 0}, {9, 0},  {6, 0}, {10, 0}, {5, 0},
	{11, 0},  {4, 0}, {12, 0}, {3, 0}, {13, 0}, {2, 0}, {14, 0}, {1, 0}, {15, 0}
};

/*
 * table of indexs in lenExtraTable[] for lengths 3 - 258
 */
STATIC UBYTE lenExtraIndex[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 9, 9, 10, 10, 11, 11,
	12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15,
	16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17,
	18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28,
};

/*
 * table of indexs in disExtraTable[] for distances 1 - 32768
 */
STATIC UBYTE disExtraIndex[] = {
	/* distance 1 - 256 */
	0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,

	/* distance 257 - 32768 */
	0, 0, 16, 17, 18, 18, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21,
	22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
};

/*
 * fixed huffman codes for literals/lengths
 * <code, bits>
 */
STATIC struct CODETABLE flTable[] = {
	/* literals */
	{12, 8}, {140, 8}, {76, 8}, {204, 8}, {44, 8}, {172, 8}, {108, 8}, {236, 8},
	{28, 8}, {156, 8}, {92, 8}, {220, 8}, {60, 8}, {188, 8}, {124, 8}, {252, 8},
	{2, 8},  {130, 8}, {66, 8}, {194, 8}, {34, 8}, {162, 8}, {98, 8},  {226, 8},
	{18, 8}, {146, 8}, {82, 8}, {210, 8}, {50, 8}, {178, 8}, {114, 8}, {242, 8},
	{10, 8}, {138, 8}, {74, 8}, {202, 8}, {42, 8}, {170, 8}, {106, 8}, {234, 8},
	{26, 8}, {154, 8}, {90, 8}, {218, 8}, {58, 8}, {186, 8}, {122, 8}, {250, 8},
	{6, 8},  {134, 8}, {70, 8}, {198, 8}, {38, 8}, {166, 8}, {102, 8}, {230, 8},
	{22, 8}, {150, 8}, {86, 8}, {214, 8}, {54, 8}, {182, 8}, {118, 8}, {246, 8},
	{14, 8}, {142, 8}, {78, 8}, {206, 8}, {46, 8}, {174, 8}, {110, 8}, {238, 8},
	{30, 8}, {158, 8}, {94, 8}, {222, 8}, {62, 8}, {190, 8}, {126, 8}, {254, 8},
	{1, 8},  {129, 8}, {65, 8}, {193, 8}, {33, 8}, {161, 8}, {97, 8},  {225, 8},
	{17, 8}, {145, 8}, {81, 8}, {209, 8}, {49, 8}, {177, 8}, {113, 8}, {241, 8},
	{9, 8},  {137, 8}, {73, 8}, {201, 8}, {41, 8}, {169, 8}, {105, 8}, {233, 8},
	{25, 8}, {153, 8}, {89, 8}, {217, 8}, {57, 8}, {185, 8}, {121, 8}, {249, 8},
	{5, 8},  {133, 8}, {69, 8}, {197, 8}, {37, 8}, {165, 8}, {101, 8}, {229, 8},
	{21, 8}, {149, 8}, {85, 8}, {213, 8}, {53, 8}, {181, 8}, {117, 8}, {245, 8},
	{13, 8}, {141, 8}, {77, 8}, {205, 8}, {45, 8}, {173, 8}, {109, 8}, {237, 8},
	{29, 8}, {157, 8}, {93, 8}, {221, 8}, {61, 8}, {189, 8}, {125, 8}, {253, 8},
	{19, 9}, {275, 9}, {147, 9}, {403, 9}, {83, 9}, {339, 9}, {211, 9}, {467, 9},
	{51, 9}, {307, 9}, {179, 9}, {435, 9}, {115, 9}, {371, 9}, {243, 9}, {499, 9},
	{11, 9}, {267, 9}, {139, 9}, {395, 9}, {75, 9}, {331, 9}, {203, 9}, {459, 9},
	{43, 9}, {299, 9}, {171, 9}, {427, 9}, {107, 9}, {363, 9}, {235, 9}, {491, 9},
	{27, 9}, {283, 9}, {155, 9}, {411, 9}, {91, 9}, {347, 9}, {219, 9}, {475, 9},
	{59, 9}, {315, 9}, {187, 9}, {443, 9}, {123, 9}, {379, 9}, {251, 9}, {507, 9},
	{7, 9}, {263, 9}, {135, 9}, {391, 9}, {71, 9}, {327, 9}, {199, 9}, {455, 9},
	{39, 9}, {295, 9}, {167, 9}, {423, 9}, {103, 9}, {359, 9}, {231, 9}, {487, 9},
	{23, 9}, {279, 9}, {151, 9}, {407, 9}, {87, 9}, {343, 9}, {215, 9}, {471, 9},
	{55, 9}, {311, 9}, {183, 9}, {439, 9}, {119, 9}, {375, 9}, {247, 9}, {503, 9},
	{15, 9}, {271, 9}, {143, 9}, {399, 9}, {79, 9}, {335, 9}, {207, 9}, {463, 9},
	{47, 9}, {303, 9}, {175, 9}, {431, 9}, {111, 9}, {367, 9}, {239, 9}, {495, 9},
	{31, 9}, {287, 9}, {159, 9}, {415, 9}, {95, 9}, {351, 9}, {223, 9}, {479, 9},
	{63, 9}, {319, 9}, {191, 9}, {447, 9}, {127, 9}, {383, 9}, {255, 9}, {511, 9},

	/* EOB */
	{0, 7},

	/* lengths */
	{64, 7}, {32, 7}, {96, 7}, {16, 7}, {80, 7}, {48, 7}, {112, 7}, {8, 7},
	{72, 7}, {40, 7}, {104, 7}, {24, 7}, {88, 7}, {56, 7}, {120, 7}, {4, 7},
	{68, 7}, {36, 7}, {100, 7}, {20, 7}, {84, 7}, {52, 7}, {116, 7}, {3, 8},
	{131, 8}, {67, 8}, {195, 8}, {35, 8}, {163, 8},
};

/*
 * fixed huffman codes for distances
 * <code, bits>
 */
STATIC struct CODETABLE fdTable[] = {
	{0, 5}, {16, 5}, {8, 5}, {24, 5}, {4, 5}, {20, 5}, {12, 5}, {28, 5},
	{2, 5}, {18, 5}, {10, 5}, {26, 5}, {6, 5}, {22, 5}, {14, 5}, {30, 5},
	{1, 5}, {17, 5}, {9, 5}, {25, 5}, {5, 5}, {21, 5}, {13, 5}, {29, 5},
	{3, 5}, {19, 5}, {11, 5}, {27, 5}, {7, 5}, {23, 5},
};

//
// local function prototypes
//
LONG UnStackCodes(struct CODESTACK *, struct ALTCODESTACK *, SHORT, SHORT *, SHORT *, SHORT *, SHORT *);
LONG StackCodes(struct CODESTACK *, struct ALTCODESTACK *, SHORT, SHORT, SHORT *, SHORT *, SHORT *, SHORT *, LONG *, SHORT *, SHORT *, LONG *);
SHORT HuffCalcHead(struct PACKTABLE *, struct CODETABLE *, SHORT, SHORT);
LONG HuffCalcSize(struct CODETABLE *, struct CODETABLE *, struct CODESTACK *, SHORT);
SHORT PackLengths(struct CODETABLE *, struct PACKTABLE *, SHORT);
ERROR_CODE HuffBuildCodes(struct CODETABLE *, SHORT *, SHORT, SHORT, SHORT);
VOID SFBuildCodes(struct CODETABLE *, struct CODELENS *, SHORT);
LONG ComputeVar(SHORT, SHORT, LONG);
struct HBINTREE *HuffBuildTree(struct CODELENS *, SHORT);
INT CDECL SortCmp(struct CODELENS *, struct CODELENS *);
ERROR_CODE HuffBuildFixedTrees();
ERROR_CODE HuffBuildDynamicTrees();
VOID HuffFreeDynamic();
VOID HuffFreeFixed();
VOID HuffDecode(struct HBINTREE *, USHORT *);

ERROR_CODE Deflate(CBofFile *pDestFile, UBYTE *pSrcBuf, INT nSrcBufSize, USHORT factor, ULONG *pCrc, ULONG *pCompressedSize) {
	SHORT lenNum, lenSum, disNum, disSum;
	LONG lenSqr, disSqr;
	SHORT lastLength, len;
	USHORT tmpVal;

	struct CODETABLE *dlTable, *ddTable, *blTable;
	struct CODETABLE *curLitTable, *curDisTable;
	struct PACKTABLE *pkTable;

	LONG fileSize, blockSize, oldSize;
	LONG compSize = 0, altCompSize = 0;
	LONG dynSize, fixSize;
	LONG disVar, oldDisVar, lenVar, oldLenVar;
	USHORT bufSize, bufUsed, offset, lastOff, off;
	USHORT code, u, ru, ou, xu;
	USHORT curOffset, curCode;
	SHORT pkSize, curLength, curRep;
	SHORT nLitCodes, nDisCodes, nLenCodes;
	SHORT length, bufLength, bufLen, n;
	SHORT rep, maxLen, sSize, dStack, lStack, sSave;
	SHORT depth;
	SHORT *lenProbCount, *disProbCount;
	UBYTE *lookBackBuf, *endBuf, *rdPtr, *oldPtr;
	UBYTE *nextPtr, *curNextPtr, *curRepPtr;
	USHORT *hashTable, *colTable;
	ERROR_CODE altErrCode, errCode;
	UBYTE method;

	UBYTE *toPtr, *tp, *hp, *p;
	struct CODESTACK *stack;
	struct CODESTACK *sPtr;
	struct ALTCODESTACK *altStack;
	struct ALTCODESTACK *altPtr;
	struct CODETABLE *ct, *bt;
	struct CODETABLE *cte, *bte;
	struct PACKTABLE *pk;
	SHORT i;

	/* assume no error */
	errCode = ERR_NONE;

	/* nothing allocated */
	colTable = hashTable = nullptr;
	stack = nullptr;
	altStack = nullptr;
	lookBackBuf = nullptr;
	dlTable = nullptr;
	pkTable = nullptr;
	lenProbCount = nullptr;

	// Init Zip info
	//
	memset(&zg.deflate, 0, sizeof(DFSTRUCT));
	zg.zipFile = pDestFile;
	zg.auxFile = nullptr;
	zg.fileSize = nSrcBufSize;
	zg.dataSize = 0L;
	zg.zfCrcValue = ~0UL;

	// these  were not initialized before
	sSize = 0;
	curRep = 0;
	nextPtr = nullptr;
	offset = 0;
	curOffset = 0;
	curRepPtr = nullptr;
	code = 0;
	curNextPtr = nullptr;
	rep = 0;
	curCode = 0;

	/*
	 * look back buffer need not be larger than the file
	 */
	if ((fileSize = zg.fileSize) < (LONG)(bufSize = WINDSIZE))
		bufSize = (USHORT)fileSize;

	/*
	 * allocate the hash/collision tables, code stack and lookback
	 * allocate the literal, distance, and bit length code tables
	 */
	if ((hashTable = (USHORT *)BofCAlloc(HASHSIZE, sizeof(USHORT))) == nullptr ||
	        (colTable = (USHORT *)BofCAlloc(bufSize, sizeof(USHORT))) == nullptr ||
	        (stack = (CODESTACK *)BofCAlloc(STACKSIZE, sizeof(struct CODESTACK))) == nullptr ||
	        (altStack = (ALTCODESTACK *)BofCAlloc(STACKSIZE, sizeof(struct ALTCODESTACK))) == nullptr ||
	        (lookBackBuf = (UBYTE *)BofAlloc(bufSize + (DF_MAXLENGTH - 1))) == nullptr ||
	        (dlTable = (CODETABLE *)BofAlloc((DF_LITCODESUSED + DF_DISCODESUSED + DF_BITCODES) * sizeof(struct CODETABLE))) == nullptr ||
	        (pkTable = (PACKTABLE *)BofAlloc((DF_LITCODESUSED + DF_DISCODESUSED) * sizeof(struct PACKTABLE))) == nullptr ||
	        (lenProbCount = (SHORT *)BofAlloc((DF_LITCODESUSED + DF_DISCODESUSED) * sizeof(SHORT))) == nullptr) {
		errCode = ERR_MEMORY;

		/*
		 * Queue up the initial inBuffer
		 */
	} else {

		BufReadInit(pSrcBuf, nSrcBufSize);

		/* initialize the output buffer */
		BitWriteInit();

		/* initialize lookback pointers */
		rdPtr = toPtr = lookBackBuf;
		endBuf = lookBackBuf + bufSize;

		/* initialize probability tables */
		disProbCount = lenProbCount + DF_LITCODESUSED;

		/*
		 * initial read must be at least DF_MAXLENGTH * 2
		 */
		if ((USHORT)(length = FIRSTREAD) > bufSize)
			length = bufSize;

		/* nothing in lookback */
		bufUsed = 0;

		/* nothing queued */
		bufLength = 0;

		/* nothing unstacked */
		sSave = 0;

		/*
		 * while not EOF - compress in STACKSIZE blocks
		 */
		do {
			/*************************************************
			 * Phase 1 - pop, compress, tally, push          *
			 *************************************************/

			/* reset the dynamic counts */
			memset(lenProbCount, 0, DF_LITCODESUSED * sizeof(SHORT));
			memset(disProbCount, 0, DF_DISCODESUSED * sizeof(SHORT));

			/* set the end-of-block count (only happens once) */
			lenProbCount[DF_ENDBLOCK] = 1;
			lenSqr = oldLenVar = lenNum = lenSum = lStack = 1;

			/* reset count totals */
			disSqr = oldDisVar = disNum = disSum = dStack = 0;

			/*
			 * stack saved codes if any
			 */
			if (sSave) {
				blockSize = StackCodes(stack, altStack, sSize, n = (sSave - sSize), lenProbCount, disProbCount, &lenSum, &lenNum, &lenSqr, &disSum, &disNum, &disSqr);
				oldPtr = lookBackBuf + (((USHORT)(toPtr - lookBackBuf) - blockSize) & WINDMASK);
				oldSize = blockSize;
				fileSize -= oldSize;
				sSave = 0;
			} else {
				oldPtr = toPtr;
				oldSize = 0;
				n = 0;
			}

			/* initialize stack pointers */
			sPtr = stack + (sSize = n);
			altPtr = altStack + sSize;

			/* compressed file size */
			compSize = BitWriteSize();

			/* no current string */
			lastLength = curLength = -1;

			/*
			 * while there is still data to read from the inFile, keep going
			 */
			do {
				/*
				 * if the last read was 0 bytes then we are at EOF
				 */
				if (lastLength != 0) {

					/*
					 * wrap if needed
					 */
					if (rdPtr >= endBuf)
						rdPtr = lookBackBuf;

					/* distance from window start */
					tmpVal = (USHORT)(rdPtr - lookBackBuf);

					/*
					 * large read
					 */
					if ((lastLength = length) != 1) {

						/* read string */
						errCode = BufReadStrQuick(rdPtr, lastLength, &lastLength);

						/*
						 * if not at EOF
						 */
						if (lastLength != 0) {

							/*
							 * copy shadow to first FIRSTREAD bytes
							 */
							if (tmpVal + lastLength > bufSize) {

								/* no more than what was read to shadow */
								memcpy(lookBackBuf, endBuf, length = (SHORT)((tmpVal + lastLength) - bufSize));
								rdPtr = lookBackBuf + length;

								/*
								 * copy FIRSTREAD bytes to shadow
								 */
							} else {
								/*
								 * if within first FIRSTREAD bytes
								 */
								if (tmpVal < (DF_MAXLENGTH - 1)) {

									/*
									 * no more than what was read
									 */
									if ((length = (DF_MAXLENGTH - 1) - (SHORT)tmpVal) > lastLength)
										length = lastLength;

									/* copy to shadow */
									memcpy(endBuf + tmpVal, rdPtr, length);
								}

								/* next */
								rdPtr += lastLength;
							}

							/* next */
							bufLength = (SHORT)(bufLength + lastLength);
						}

						/*
						 * small read
						 */
					} else if (!(errCode = BufReadQuick(rdPtr))) {

						/*
						 * copy to shadow if within FIRSTREAD bytes
						 */
						if (tmpVal < (DF_MAXLENGTH - 1))
							*(endBuf + tmpVal) = *rdPtr;

						/* next */
						rdPtr++;
						bufLength++;
					}

					/*
					 * handle errors/EOF
					 */
					if (errCode == ERR_ENDFILE) {
						errCode = ERR_NONE;
						lastLength = 0;
					}
					if (errCode)
						break;
				}

				/*
				 * need at least 3 for string lookback
				 */
				if (bufLength >= 3) {

					/* local copies */
					bufLen = bufLength;
					tp = toPtr;

					for (;;) {

						/* assume nothing */
						length = -1;

						/*
						 * if current string complete do next
						 */
						if (curLength != -1) {

							/*
							 * if not already there (REP advance)
							 */
							ou = hashTable[curCode];
							if ((u = (USHORT)(tp - lookBackBuf)) != (ou & ~REPFLAG)) {

								colTable[u] = ou;
								hashTable[curCode] = u;
							}

							/* last character hashed */
							curRepPtr = tp;

							/*
							 * if no repeated prefix
							 */
							if ((curRep -= 2) <= 0) {

								/* prefix 1 character */
								curRep = 1;

							} else {
								/* have a repeat */
								hashTable[curCode] |= REPFLAG;

								/*
								 * first code already computed
								 */
								i = curRep;
								while (--i > 0) {

									if (++curRepPtr >= endBuf)
										curRepPtr = lookBackBuf;

									hashTable[curCode] = (u = (USHORT)(curRepPtr - lookBackBuf)) | REPFLAG;

									if (u != (ou & ~REPFLAG))
										colTable[u] = ou;
									else
										ou = colTable[u];
								}
							}

							/*
							 * need at least 3 for a string
							 * need at least 3 for another
							 */
							if (curLength < 3 || --bufLen < curLength)
								break;

							/*
							 * bump to next string
							 */
							if (++tp >= endBuf)
								tp = lookBackBuf;
						}

						/*
						 * determine maximum string length
						 */
						if ((maxLen = bufLen) > DF_MAXLENGTH)
							maxLen = DF_MAXLENGTH;

						/*
						 * don't look so hard if we have a good string
						 */
						depth = factor;
						if (curLength >= 32)
							depth >>= 2;

						/*
						 * check for repeated prefix
						 */
						ru = rep = 0;
						if (tp[1] == tp[0] && tp[2] == tp[0]) {
							ru = REPFLAG;
							rep = 3 - 1;
							while (++rep < maxLen && tp[rep] == tp[0])
								;
						}

						/* determine offset for end of new stuff */
						tmpVal = (USHORT)(rdPtr - tp) & WINDMASK;

						/* determine offset from buffer start */
						xu = (USHORT)(tp - lookBackBuf);

						/* previous location of current string */
						u = hashTable[code = HASHFUNC(tp[0], tp[1], tp[2])];

						/* no last string */
						off = USHRT_MAX;

						/*
						 * scan lookback buffer
						 */
						for (;;) {

							/* determine hash pointer offset */
							lastOff = off;
							off = (USHORT)(((ou = u) & ~REPFLAG) - xu) & WINDMASK;

							/*
							 * if hash and string have repeated prefix
							 */
							if (ou & ru) {

								/*
								 * backup to match string REP if we
								 * end at current string then previous
								 * character was probably repeated
								 */
								if (!(off -= (rep - 3)))
									off = WINDSIZE - 1;
								if ((u = WINDSIZE - bufUsed) > off)
									off = u;

								/* determine hash pointer offset */
								u = (xu + off) & WINDMASK;
							}

							/* nothing yet */
							len = 0;

							/*
							 * stop looking if has entry invalid
							 */
							if (off < tmpVal)
								break;

							/*
							 * if repeated prefix
							 */
							if (ou & ru) {

								/*
								 * if a repeated prefix crosses string
								 * then we need to move prefix start
								 * just prior to string
								 */
								if (((off + rep) & WINDMASK) < tmpVal) {
									off = WINDSIZE - 1;
									u = (xu - 1) & WINDMASK;
								}
							}

							/*
							 * stop looking if not further away
							 */
							if (off >= lastOff)
								break;

							/*
							 * if xth and 1st character matches
							 */
							hp = lookBackBuf + (u & ~REPFLAG);
							if ((length < 3 || *(USHORT *)(hp + length - 1) == *(USHORT *)(tp + length - 1)) && *(USHORT *)(hp + 1) == *(USHORT *)(tp + 1) && *hp == *tp) {

								/* have a length of 3 */
								len = 3;

								/*
								 * match in steps of 4
								 */
								while (len < (SHORT)(maxLen - (sizeof(ULONG) - 1)) && *(ULONG *)(hp + len) == *(ULONG *)(tp + len))
									len += sizeof(ULONG);

								/*
								 * attempt 2 more
								 */
								if (len < (SHORT)(maxLen - (sizeof(USHORT) - 1)) && *(USHORT *)(hp + len) == *(USHORT *)(tp + len))
									len += sizeof(USHORT);

								/*
								 * attempt 1 more
								 */
								if (len < maxLen && hp[len] == tp[len])
									len++;
							}

							/*
							 * better string found
							 */
							if (len > length) {
								offset = WINDSIZE - off;
								length = len;
								if ((nextPtr = tp + len) >= endBuf)
									nextPtr -= bufSize;
							}

							/*
							 * stop looking if depth reached or no more or at max length
							 */
							if (!--depth || length == maxLen)
								break;

							/* next */
							u = colTable[ou & ~REPFLAG];
						}

						/*
						 * if we did two passes
						 */
						if (curLength != -1)
							break;

						/*
						 * use anything found
						 */
						curCode = code;
						curNextPtr = nextPtr;
						curRep = rep;
						curOffset = offset;
						if ((curLength = len) < length)
							curLength = length;
					}

					/*
					 * need at least three for a string
					 */
					if (curLength >= 3 && (curLength > length || (curLength == length && curOffset <= offset))) {

						/* stop at buffer length minus already done */
						i = bufLength - curRep - 2;

						/* stop at string length minus already done */
						n = i - curLength + curRep;

						/* start without repeats */
						curRep = DF_MAXLENGTH;

						/*
						 * generate hash codes for remaining string
						 */
						while (--i >= 0) {

							if (++curRepPtr >= endBuf)
								curRepPtr = lookBackBuf;

							/*
							 * prefix repeat
							 */
							ru = (USHORT)(curRepPtr - lookBackBuf);
							if (curRep < (DF_MAXLENGTH - 2) && curRepPtr[1] == curRepPtr[0] && curRepPtr[2] == curRepPtr[0]) {

								hashTable[code] = (u = ru) | REPFLAG;

								if (u != (ou & ~REPFLAG))
									colTable[u] = ou;
								else
									ou = colTable[u];

								curRep++;

								/*
								 * normal code
								 */
							} else if (i >= n) {

								colTable[u = ru] = (ou = hashTable[code = HASHFUNC(curRepPtr[0], curRepPtr[1], curRepPtr[2])]);
								hashTable[code] = u;

								curRep = DF_MAXLENGTH;
								if (curRepPtr[1] == curRepPtr[0] && curRepPtr[2] == curRepPtr[0]) {
									hashTable[code] |= REPFLAG;
									curRep = 1;
								}

								/*
								 * end of string
								 */
							} else
								break;
						}

						/*
						 * push length and short string on stack
						 */
						if (!(altPtr->length = (UBYTE)(curLength - 3))) {
							altPtr->lit1 = toPtr[0];
							altPtr->lit2 = toPtr[1];
							altPtr->lit3 = toPtr[2];
						}
						altPtr++;

						/* look up the index for this length */
						i = lenExtraIndex[curLength - 3];

						/* update length count */
						lenNum += !lenProbCount[i + 257];
						lenSqr += ((USHORT)lenProbCount[i + 257]++ << 1) + 1;
						lenSum++;

						/* save length index */
						sPtr->idxLen = (UBYTE)i;

						/*
						 * look up the index for this offset
						 */
						if ((u = (USHORT)(curOffset - 1)) >= 256)
							u = (u >> 7) + 256;
						i = disExtraIndex[u];

						/* update offset count */
						disNum += !disProbCount[i];
						disSqr += ((USHORT)disProbCount[i]++ << 1) + 1;
						disSum++;

						/* save offset index */
						sPtr->idxOff = (UBYTE)i;

						/*
						 * push offset on stack
						 */
						sPtr->offset = curOffset;
						sPtr++;
						sSize++;

						/* next position */
						toPtr = curNextPtr;

						/* less in buffer/file */
						bufLength -= curLength;
						fileSize -= curLength;
						oldSize += curLength;

						/*
						 * more in lookback buffer
						 */
						if (bufUsed < WINDSIZE && (bufUsed += curLength) > bufSize)
							bufUsed = WINDSIZE;

						/* string length */
						length = curLength;
						curLength = -1;
						compSize += 3;

						/* compute variance for distances */
						disVar = ComputeVar(disNum, disSum, disSqr);

						/*
						 * if variance wider
						 */
						if (disVar > oldDisVar || disVar < 1000) {
							oldDisVar = disVar;
							dStack = sSize;

							/*
							 * if variance within 2% of last and not at EOF
							 */
						} else if (disVar + (disVar / 50) < oldDisVar && fileSize > 0) {

							/* unstack to widest variance */
							blockSize = UnStackCodes(sPtr, altPtr, sSize - dStack, lenProbCount, disProbCount, &lenSum, &disSum);

							/* correct sizes */
							fileSize += blockSize;
							oldSize -= blockSize;
							sSave = sSize;
							sSize = dStack;
							break;
						}

						/* skip literal push */
						goto skipliteral;
					}
				}

				/*
				 * push literal onto the stack
				 */
				sPtr->offset = 0;
				sPtr->idxLen = *toPtr;
				sPtr++;
				altPtr++;
				sSize++;

				/* update literal count */
				lenNum += !lenProbCount[*toPtr];
				lenSqr += ((USHORT)lenProbCount[*toPtr]++ << 1) + 1;
				lenSum++;

				/*
				 * next position
				 */
				if (++toPtr >= endBuf)
					toPtr = lookBackBuf;

				/* less in buffer/file */
				bufLength--;
				fileSize--;
				oldSize++;
				compSize++;

				/*
				 * more in lookBack buffer
				 */
				if (bufUsed < WINDSIZE)
					bufUsed++;

				/* current string is next string */
				curLength = length;
				curRep = rep;
				curOffset = offset;
				curNextPtr = nextPtr;
				curCode = code;

				/* one literal character */
				length = 1;

				/****************************
				 * string match enters here *
				 ****************************/

skipliteral:

				/* compute variance for literals/lengths */
				lenVar = ComputeVar(lenNum, lenSum, lenSqr);

				/*
				 * if variance wider
				 */
				if (lenVar > oldLenVar || lenVar < 1000) {
					oldLenVar = lenVar;
					lStack = sSize;

					/*
					 * if variance withing 5% of last and not at EOF
					 */
				} else if (lenVar + (lenVar / 20) < oldLenVar && fileSize > 0) {

					/* unstack to widest variance */
					blockSize = UnStackCodes(sPtr, altPtr, sSize - lStack, lenProbCount, disProbCount, &lenSum, &disSum);

					/* correct sizes */
					fileSize += blockSize;
					oldSize -= blockSize;
					sSave = sSize;
					sSize = lStack;
					break;
				}

				/*
				 * stop at end of stack
				 */
			} while (fileSize > 0 && sSize < STACKSIZE);

			if (errCode)
				break;

			/*************************************************
			 * Phase 2 - write headers, generate, pop, write *
			 *************************************************/

			/* calculate how big the block is using fixed codes */
			blockSize = fixSize = HuffCalcSize(flTable, fdTable, stack, sSize);

			/* default to fixed codes */
			curLitTable = flTable;
			curDisTable = fdTable;
			method = DF_FIXED;

			/* build dynamic literal huffman codes */
			memset(dlTable, 0, DF_LITCODESUSED * sizeof(struct CODETABLE));
			if ((errCode = HuffBuildCodes(dlTable, lenProbCount, lenSum, DF_LITCODESUSED, 15)) != ERR_NONE)
				break;

			/*
			 * calculate the number of literal/length codes used
			 */
			nLitCodes = DF_LITCODESUSED;
			while (nLitCodes > 257 && !dlTable[nLitCodes - 1].bits)
				nLitCodes--;

			/* build dynamic distance huffman codes */
			memset(ddTable = dlTable + nLitCodes, 0, DF_DISCODESUSED * sizeof(struct CODETABLE));
			if ((errCode = HuffBuildCodes(ddTable, disProbCount, disSum, DF_DISCODESUSED, 15)) != ERR_NONE)
				break;

			/*
			 * calculate the number of distance codes used
			 */
			nDisCodes = DF_DISCODESUSED;
			while (nDisCodes > 1 && !ddTable[nDisCodes - 1].bits)
				nDisCodes--;

			/* pack literal/length and distance tables at once */
			pkSize = PackLengths(dlTable, pkTable, nLitCodes + nDisCodes);

			/* re-use for bit-length counts */
			memset(lenProbCount, 0, DF_BITCODES * sizeof(SHORT));

			/*
			 * count all the bit lengths (0-18)
			 */
			pk = pkTable;
			i = pkSize;
			while (--i >= 0) {
				lenProbCount[pk->bits]++;
				pk++;
			}

			/* build the huffman codes for the packed lengths of the huffman codes */
			memset(blTable = ddTable + nDisCodes, 0, DF_BITCODES * sizeof(struct CODETABLE));
			if ((errCode = HuffBuildCodes(blTable, lenProbCount, pkSize, DF_BITCODES, 7)) != ERR_NONE)
				break;

			/*
			 * calculate the number of bit length codes used
			 */
			nLenCodes = DF_BITCODES;
			while (nLenCodes > 4 && !(blTable + bitTreeTable[nLenCodes - 1].val)->bits)
				nLenCodes--;

			/* calculate the compressed size to see if the file got larger */
			dynSize = HuffCalcHead(pkTable, blTable, pkSize, nLenCodes) + HuffCalcSize(dlTable, ddTable, stack, sSize);

			/*
			 * if it did not get larger, then use dynamic codes
			 */
			if (dynSize < fixSize) {
				blockSize = dynSize;
				curLitTable = dlTable;
				curDisTable = ddTable;
				method = DF_DYNAMIC;
			}

			/*
			 * if this block got bigger then store it
			 */
			if (blockSize > oldSize)
				method = DF_STORED;

			/*
			 * write out the 3 bit block header:
			 *
			 *   *--2--*--1--*--0--*
			 *      |     |     |
			 *      +-----+     last block if set
			 *         |
			 *         0 = stored
			 *         1 = fixed
			 *         2 = dynamic
			 *         3 = reserved
			 */

			tmpVal = (method << 1);
			if (!fileSize)
				tmpVal |= 1;
			if ((errCode = BitWriteQuick(&tmpVal, 3)) != ERR_NONE)
				break;

			/* compressed size */
			altCompSize = BitWriteSize();

			/*
			 * if the block got bigger then store it
			 */
			if (method == DF_STORED) {

				len = (SHORT)oldSize;
				tmpVal = ~len;

				/*
				 * write out the block length and the one's compliment of the block length
				 */
				if (((errCode = BitWriteBytes((UBYTE *)&len, sizeof(SHORT))) != ERR_NONE) || ((errCode = BitWriteBytes((UBYTE *)&tmpVal, sizeof(SHORT))) != ERR_NONE))
					break;

				/* last position */
				p = oldPtr;

				/*
				 * while there is still data to write
				 */
				while (len) {

					/*
					 * not more than we can do without wrap-around
					 */
					if ((USHORT)(n = len) > (tmpVal = endBuf - p))
						n = (SHORT)tmpVal;

					/*
					 * write data
					 */
					if ((errCode = BitWriteBytes(p, n)) != ERR_NONE)
						break;

					/* assume wrap around */
					p = lookBackBuf;

					/* less to do */
					len -= n;

					/*
					 * update progress
					 */
					altCompSize += n;
					oldSize -= n;
				}

				if (errCode)
					break;

				/*
				 * block is compressed
				 */
			} else {
				/*
				 * if we are using dynamic codes then write out the huffman
				 * codes header and the bit length codes
				 */
				if (method == DF_DYNAMIC) {

					/*
					 * write out the number of literal codes to be sent (5 bits)
					 * write out the number of distance codes to be sent (5 bits)
					 * write out the number of bit length codes (4 bits)
					 */
					tmpVal = (nLitCodes - 257) | ((nDisCodes - 1) << 5) | ((nLenCodes - 4) << 10);
					if ((errCode = BitWriteQuick(&tmpVal, 5 + 5 + 4)) != ERR_NONE)
						break;

					/*
					 * write out that many 3 bit bit length codes
					 */
					i = -1;
					while (++i < nLenCodes) {

						/*
						 * codes must be written in a certain order:
						 * 16 17 18 0 8 7 9 6 10 5 11 4 12 3 13 2 14 1 15
						 */
						tmpVal = (blTable + bitTreeTable[i].val)->bits;
						if ((errCode = BitWriteQuick(&tmpVal, 3)) != ERR_NONE)
							break;
					}

					if (errCode)
						break;

					/*
					 * write out all of the packed bit lengths
					 */
					pk = pkTable;
					i = pkSize;
					while (--i >= 0) {

						/*
						 * write out bit lengths
						 */
						bt = blTable + pk->bits;
						if ((errCode = BitWriteQuick(&bt->code, bt->bits)) != ERR_NONE)
							break;

						/*
						 * write out any extra bits needed
						 */
						if (pk->extraBits) {
							tmpVal = pk->extraLen;
							if ((errCode = BitWriteQuick(&tmpVal, pk->extraBits)) != ERR_NONE)
								break;
						}

						/* next */
						pk++;
					}

					if (errCode)
						break;
				}

				/* reset the stack pointer */
				altPtr = altStack;
				sPtr = stack;
				i = sSize;

				/*
				 * while the stack has info in it
				 */
				while (--i >= 0) {

					/* assume literal */
					ct = curLitTable + sPtr->idxLen;

					/*
					 * stacked record is offset/distance
					 */
					if (sPtr->offset) {

						UBYTE bits1, bits2, bits3;

						ct += 257;
						cte = lenExtraTable + sPtr->idxLen;

						bt = curDisTable + sPtr->idxOff;
						bte = disExtraTable + sPtr->idxOff;

						altCompSize += 3;
						len = altPtr->length + 3;

						/*
						 * three characters costs less than lookback
						 */
						if ((len == 3) &&
						        ((bits1 = curLitTable[altPtr->lit1].bits) != 0) &&
						        ((bits2 = curLitTable[altPtr->lit2].bits) != 0) &&
						        ((bits3 = curLitTable[altPtr->lit3].bits) != 0) &&
						        (bits1 + bits2 + bits3) <= (ct->bits + cte->bits + bt->bits + bte->bits)) {

							/*
							 * write out each code in this string
							 */
							ct = curLitTable + altPtr->lit1;
							if ((errCode = BitWriteQuick(&ct->code, ct->bits)) != ERR_NONE)
								break;
							ct = curLitTable + altPtr->lit2;
							if ((errCode = BitWriteQuick(&ct->code, ct->bits)) != ERR_NONE)
								break;
							ct = curLitTable + altPtr->lit3;
							if ((errCode = BitWriteQuick(&ct->code, ct->bits)) != ERR_NONE)
								break;

							/*
							 * lookback costs less than three characters
							 */
						} else {
							/*
							 * write out length code
							 */
							if ((errCode = BitWriteQuick(&ct->code, ct->bits)) != ERR_NONE)
								break;

							/*
							 * write out any extra bits needed to encode this length
							 */
							if (cte->bits) {
								tmpVal = len - cte->code;
								if ((errCode = BitWriteQuick(&tmpVal, cte->bits)) != ERR_NONE)
									break;
							}

							/*
							 * write out distance code
							 */
							if ((errCode = BitWriteQuick(&bt->code, bt->bits)) != ERR_NONE)
								break;

							/*
							 * write out any extra bits needed to encode this offset
							 */
							if (bte->bits) {
								tmpVal = sPtr->offset - bte->code;
								if ((errCode = BitWriteQuick(&tmpVal, bte->bits)) != ERR_NONE)
									break;
							}
						}

						/*
						 * stacked record is a literal
						 */
					} else {
						altCompSize++;
						len = 1;

						/*
						 * write out each code in this string
						 */
						if ((errCode = BitWriteQuick(&ct->code, ct->bits)) != ERR_NONE)
							break;
					}

					/*
					 * update progress
					 */
					oldSize -= len;

					sPtr++;
					altPtr++;
				}

				/*
				 * write out the End-Of-Block Code
				 */
				if (!errCode)
					errCode = BitWriteQuick(&curLitTable[DF_ENDBLOCK].code, curLitTable[DF_ENDBLOCK].bits);
				if (errCode)
					break;
			}

		} while (fileSize > 0);

		/*
		 * Flush the Bit Buffer out to disk
		 */
		if (((altErrCode = BitWriteFlush(&zg.dataSize)) != ERR_NONE) && !errCode)
			errCode = altErrCode;
	}

	/*
	 * free allocations
	 */
	if (lenProbCount != nullptr)
		BofFree(lenProbCount);
	if (pkTable != nullptr)
		BofFree(pkTable);
	if (dlTable != nullptr)
		BofFree(dlTable);
	if (lookBackBuf != nullptr)
		BofFree(lookBackBuf);
	if (altStack != nullptr)
		BofFree((VOID *)altStack);
	if (stack != nullptr)
		BofFree((VOID *)stack);
	if (colTable != nullptr)
		BofFree((VOID *)colTable);
	if (hashTable != nullptr)
		BofFree((VOID *)hashTable);

	// return the CRC and compressed size back to the caller
	//
	if (pCrc != nullptr)
		*pCrc = zg.zfCrcValue;

	if (pCompressedSize != nullptr)
		*pCompressedSize = zg.dataSize;

	debug(1, "compSize: %d altCompSize: %d", compSize, altCompSize);

	return (errCode);
}

/**
 *
 * name      UnStackCodes - removes specified number of codes from stack
 *
 * synopsis  size = UnStackCodes(sPtr, altPtr, count, lenProbCount, disProbCount, lenSum, disSum)
 *           struct CODESTACK *sPtr;             top of stack
 *           struct ALTCODESTACK *altPtr;        top of alternate stack
 *           SHORT count;                        number of codes
 *           SHORT *lenProbCount;                literal/length probabilities
 *           SHORT *disProbCount;                distance probabilities
 *           SHORT *lenSum;                      literal/length sum
 *           SHORT *disSum;                      distance sum
 *
 * purpose   Unstacks codes which would be better used in the next block
 *
 * returns   size = raw data size of unstacked codes
 *
 **/

LONG UnStackCodes(struct CODESTACK *sPtr, struct ALTCODESTACK *altPtr, SHORT count, SHORT *lenProbCount, SHORT *disProbCount, SHORT *lenSum, SHORT *disSum) {
	SHORT i;
	LONG stSize;

	/* unstacked size */
	stSize = 0L;

	/*
	 * unstack specified items
	 */
	while (--count >= 0) {

		/* previous */
		altPtr--;
		sPtr--;

		/* assume literal index */
		i = sPtr->idxLen;

		/*
		 * if length/offset pair
		 */
		if (sPtr->offset) {

			/* correct offset count */
			disProbCount[sPtr->idxOff]--;
			(*disSum)--;

			/* correct size */
			stSize += altPtr->length + (3 - 1);

			/* get length index */
			i += 257;
		}

		/* correct literal/length count */
		lenProbCount[i]--;
		(*lenSum)--;

		/* correct size */
		stSize++;
	}

	return (stSize);
}

/**
 *
 * name      StackCodes - addes specified number of codes removed from stack
 *
 * synopsis  size = StackCodes(sPtr, altPtr, offset, count, lenProbCount, disProbCount, lenSum, lenNum, lenSqr, disSum, disNum, disSqr)
 *           struct CODESTACK *sPtr;             base of stack
 *           struct ALTCODESTACK *altPtr;        base of alternate stack
 *           SHORT offset;                       offset to top of stack
 *           SHORT count;                        number of codes
 *           SHORT *lenProbCount;                literal/length probabilities
 *           SHORT *disProbCount;                distance probabilities
 *           SHORT *lenSum;                      literal/length sum
 *           SHORT *lenNum;                      literal/length count
 *           LONG *lenSqr;                       literal/length squares
 *           SHORT *disSum;                      distance sum
 *           SHORT *disNum;                      distance count
 *           LONG *disSqr;                       distance squares
 *
 * purpose   Unstacks codes which would be better used in the next block
 *
 * returns   size = raw data size of unstacked codes
 *
 **/

LONG StackCodes(struct CODESTACK *sPtr, struct ALTCODESTACK *altPtr, SHORT offset, SHORT count, SHORT *lenProbCount, SHORT *disProbCount, SHORT *lenSum, SHORT *lenNum, LONG *lenSqr, SHORT *disSum, SHORT *disNum, LONG *disSqr) {
	SHORT i;
	LONG size;

	/* stacked size */
	size = 0;

	/*
	 * stack specified items
	 */
	while (--count >= 0) {

		/* copy to stack bottom */
		*altPtr = *(altPtr + offset);
		*sPtr = *(sPtr + offset);

		/* assume literal index */
		i = sPtr->idxLen;

		/*
		 * if length/offset pair
		 */
		if (sPtr->offset) {

			/* correct offset amount */
			*disNum += !disProbCount[sPtr->idxOff];
			*disSqr += ((USHORT)disProbCount[sPtr->idxOff]++ << 1) + 1;
			(*disSum)++;

			/* correct size */
			size += altPtr->length + (3 - 1);

			/* get length index */
			i += 257;
		}

		*lenNum += !lenProbCount[i];
		*lenSqr += ((USHORT)lenProbCount[i]++ << 1) + 1;
		(*lenSum)++;

		/* correct size */
		size++;

		/* next */
		sPtr++;
		altPtr++;
	}

	return (size);
}

/**
 *
 * name      HuffCalcHead - calculate dynamic code header size
 *
 * synopsis  size = HuffCalcHead()
 *
 * purpose   Determines size of Huffman codes + bit lengths header
 *
 * returns   size = total size of header in bytes
 *
 **/

SHORT HuffCalcHead(struct PACKTABLE *pkTable, struct CODETABLE *blTable, SHORT pkSize, SHORT nLenCodes) {
	LONG size;
	struct PACKTABLE *pk;

	/* literal codes + distance codes + bit length codes + 3 bits per */
	size = (5 + 5 + 4) + nLenCodes * 3;

	/*
	 * total the packed bit lengths
	 */
	pk = pkTable;
	while (--pkSize >= 0) {
		size += blTable[pk->bits].bits + pk->extraBits;
		pk++;
	}

	return ((SHORT)((size + 7) >> 3));
}

/**
 *
 * name      HuffCalcSize - calculate compressed data size
 *
 *
 * synopsis  newSize = HuffCalcSize(table, block, size)
 *           struct CODETABLE *lenTable;     table of literal/length huffman codes
 *           struct CODETABLE *disTable      table of distance huffman codes
 *           struct CODESTACK *block     block of compressed data
 *           SHORT size                      size of the data block
 *
 * purpose   Determines size of a data block after huffman compression
 *
 * returns   newSize = total data size after compression
 *
 **/

LONG HuffCalcSize(struct CODETABLE *lenTable, struct CODETABLE *disTable, struct CODESTACK *sPtr, SHORT sSize) {
	LONG size;

	/* start with nothing */
	size = 0;

	while (--sSize >= 0) {

		/*
		 * if it was a literal
		 */
		if (!sPtr->offset)
			size += lenTable[sPtr->idxLen].bits;

		/*
		 * otherwise it was a length/offset
		 */
		else {
			/* update the number of bits needed for this length */
			size += lenTable[sPtr->idxLen + 257].bits + lenExtraTable[sPtr->idxLen].bits;

			/* update the number of bits needed for this offset */
			size += disTable[sPtr->idxOff].bits + disExtraTable[sPtr->idxOff].bits;
		}

		sPtr++;
	}

	return ((size + 7) >> 3);
}

/**
 *
 * name      PackLengths - compresses an array of bit lengths;
 *
 *
 * synopsis  pkSize = PackLengths(srcTable, pkTable, srcSize)
 *           struct CODETABLE *srcTable      table to read from
 *           struct PACKTABLE *pkTable       table to write to
 *           SHORT srcSize                   size of source table
 *
 * purpose   to compress an array of bit lengths with RLE
 *
 * returns   pkSize = packed table size
 *
 **/

SHORT PackLengths(struct CODETABLE *srcTable, struct PACKTABLE *pkTable, SHORT srcSize) {
	SHORT pkSize, length;
	UBYTE last;

	/* start with nothing */
	last = UCHAR_MAX;
	pkSize = 0;

	/* zero table */
	memset(pkTable, 0, srcSize * sizeof(struct PACKTABLE));

	/*
	 * until end-of-table
	 */
	do {
		/*
		 * add the new one to the table
		 */
		length = 1;
		if (srcTable->bits != last) {
			if ((last = srcTable->bits) != 0) {
				pkTable->bits = last;
				pkTable++;
				pkSize++;
				length = 0;
			}
		}

		/*
		 * continue while we keep getting the same characters
		 * if the run is not zeros then it can only have a length of 6 or less
		 * if the run is zeros then it can have a length up to 138
		 */
		while (--srcSize && (++srcTable)->bits == last && (last != 0 ? (length < 6) : (length < 138)))
			length++;

		/*
		 * at least 3 repeats needed
		 */
		if ((length -= 3) < 0) {

			/*
			 * add to table
			 */
			while (--length >= -3) {
				pkTable->bits = last;
				pkTable++;
				pkSize++;
			}

			/*
			 * 3 repeats for RLE
			 */
		} else {
			/*
			 * RLE for non-zeros use code 16
			 */
			if (last != 0) {
				pkTable->extraBits = 2;
				pkTable->extraLen = (UBYTE)length;
				pkTable->bits = 16;

				/*
				 * if the RLE under 11 use code 17
				 */
			} else if (length < (11 - 3)) {
				pkTable->extraBits = 3;
				pkTable->extraLen = (UBYTE)length;
				pkTable->bits = 17;

				/*
				 * otherwise RLE is 11 to 138 so use code 18
				 */
			} else {
				pkTable->extraBits = 7;
				pkTable->extraLen = (UBYTE)length - (UBYTE)(11 - 3);
				pkTable->bits = 18;
			}

			pkTable++;
			pkSize++;
		}

	} while (srcSize);

	return (pkSize);
}

/**
 *
 * name      HuffBuildCodes - build Huffman codes given the frequencies
 *
 * synopsis  HuffBuildCodes(codeTable, probCounts, sum, size, bits)
 *           struct CODETABLE *codeTable         table of huffman codes that is built
 *           SHORT *probCounts                   probability counts
 *           SHORT sum;                          count total
 *           SHORT size                          number of nodes
 *           SHORT bits                          maximum bit length
 *
 * purpose   to build Huffman codes for nodes based on their frequency
 *
 **/

ERROR_CODE HuffBuildCodes(struct CODETABLE *codeTable, SHORT *probCounts, SHORT sum, SHORT size, SHORT bits) {
	/* modified Fibonacci sequence for minimum variability Huffman */
	STATIC SHORT fib[] = {3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364, 2207, 3571};

	SHORT count[DF_MAXCODES * 2];
	SHORT heap[DF_MAXCODES * 2];
	SHORT dad[DF_MAXCODES * 2];
	struct CODELENS *codeLens;
	SHORT heapsize;
	SHORT scale;
	ERROR_CODE errCode;

	SHORT i, j, k, v, t;

	/* assume no errors */
	errCode = ERR_NONE;

	/*
	 * allocate memory for tree descriptor
	 */
	if ((codeLens = (CODELENS *)BofAlloc(sizeof(struct CODELENS) * DF_MAXCODES)) == nullptr)
		errCode = ERR_MEMORY;

	else {
		/*
		 * scale counts if necessary
		 * 1,1,1,3,4,7,11,18,29,47,76,123,199,322,521,843,1364,2207,3571
		 */
		if ((scale = fib[bits] - size - 2) > sum)
			scale = sum;

		/*
		 * initialize heap and scale counts
		 */
		heap[0] = heapsize = 0;
		i = -1;
		while (++i < size) {
			count[i] = 0;
			if (probCounts[i]) {
				count[i] = (USHORT)(((LONG)probCounts[i] * scale + (sum - 1)) / sum);
				heap[++heapsize] = i;
			}
		}

		/*
		 * if something to do
		 */
		if (heapsize) {

			/*
			 * need at least two entries
			 */
			if (heapsize == 1)
				codeTable[heap[1]].bits = 1;

			else {
				/*
				 * make priority queue
				 */
				i = (heapsize / 2) + 1;
				while (--i > 0) {
					v = heap[k = i];
					while ((j = k * 2) <= heapsize) {
						if (j < heapsize && count[heap[j]] > count[heap[j + 1]])
							j++;
						if (count[v] <= count[heap[j]])
							break;
						heap[k] = heap[j];
						k = j;
					}
					heap[k] = v;
				}

				/*
				 * while queue has at least two entries
				 */
				i = size - 1;
				do {
					/*
					 * take out least-freq entry
					 */
					t = heap[1];
					v = heap[1] = heap[heapsize--];
					k = 1;
					while ((j = k * 2) <= heapsize) {
						if (j < heapsize && count[heap[j]] > count[heap[j + 1]])
							j++;
						if (count[v] <= count[heap[j]])
							break;
						heap[k] = heap[j];
						k = j;
					}
					heap[k] = v;

					/* next least-freq entry */
					j = heap[1];

					/* generate new node */
					count[++i] = (SHORT)(count[t] + count[j]);
					dad[t] = dad[j] = i;

					/*
					 * put into queue at furthest point
					 */
					v = heap[1] = i;
					k = 1;
					while ((j = k * 2) <= heapsize) {
						if (j < heapsize && count[heap[j]] >= count[heap[j + 1]])
							j++;
						if (count[v] <= count[heap[j]])
							break;
						heap[k] = heap[j];
						k = j;
					}
					heap[k] = v;

				} while (heapsize > 1);
				dad[i] = 0;

				/*
				 * generate code lengths for values
				 */
				k = -1;
				while (++k < size) {
					if (!count[k])
						codeLens[k].val = codeLens[k].bits = 0;
					else {
						i = 0;
						t = dad[k];
						do {
							i++;
						} while ((t = dad[t]) != 0);
						codeLens[k].val = k;
						codeLens[k].bits = (UBYTE)i;
					}
				}

				/* generate Shannon-Fanno codes from lengths */
				SFBuildCodes(codeTable, codeLens, size);
			}
		}

		/* free tree descriptor */
		BofFree(codeLens);
	}

	return (errCode);
}

/**
 *
 * name      HuffBuildTree - Build a fixed or dynamic huffman tree
 *
 *
 * synopsis  binTree = HuffBuildTree(tempTree, treeSize)
 *           struct CODELENS *tempTree   table that the binary tree is build from
 *           SHORT treeSize              size of that table
 *
 * purpose   to create a huffman binary tree from code lengths
 *
 *
 * returns   binTree = the binary tree representing either fixed or dynamic codes
 *
 **/

struct HBINTREE *HuffBuildTree(struct CODELENS *codeLens, SHORT size) {
	struct HBINTREE *binTree;
	struct HBINTREE *next, *nextFree, *parents;
	struct HBINTREE *limit;
	USHORT bitLength;

	struct CODELENS *p;

	/*
	 * allocate the huffman binary tree
	 */
	if ((binTree = (HBINTREE *)BofAlloc(bitLength = (sizeof(struct HBINTREE) * 2) * (size + 1))) != nullptr) {

		/* set the entire tree to NULLs */
		memset(binTree, 0, bitLength);

		/* initialize pointers */
		nextFree = next = binTree;
		limit = binTree + size * 2;

		/* sort it by it's bitlength in ascending order */
		qsort(codeLens, size, sizeof(struct CODELENS), (INT(CDECL *)(CONST VOID *, CONST VOID *))SortCmp);

		p = codeLens + size - 1;
		bitLength = p->bits + 1;
		while (--bitLength) {

			/*
			 * create leaf nodes and make sure we don't overflow memory
			 */
			while (p >= codeLens && p->bits == (UBYTE)bitLength && nextFree < limit) {
				(++nextFree)->value = p->val;
				--p;
			}

			/*
			 * create the parents of the nodes just created
			 * and make sure we don't overflow memory
			 */
			if (bitLength > 1) {
				parents = nextFree;
				while (++next <= parents && nextFree < limit) {
					(++nextFree)->left = next;
					nextFree->right = ++next;
				}
				next = parents;
			}
		}

		/*
		 * create the root node
		 */
		binTree->right = nextFree;
		if (--nextFree != binTree)
			binTree->left = nextFree;
	}

	return (binTree);
}

/**
 *
 * name      SortCmp - Support routine for qsort() - used in Explode & Inflate
 *
 * synopsis  dif = SortCmp(e1, e2)
 *           struct CODELENS *e1      Element 1
 *           struct CODELENS *e2      Element 2
 *
 * purpose   To determine if e1 is larger than e2 for sorting purposes
 *           primary key is codeSize, secondary key is 'val'
 *
 *
 * returns   (-1): if e1 < e2, (0): if e1 == e2, (1): if e1 > e2
 *
 **/

INT CDECL SortCmp(struct CODELENS *e1, struct CODELENS *e2) {
	INT n;

	/*
	 * Sort the tree entries by codeSize (primary) and val (secondary)
	 */
	if ((n = (INT)e1->bits - (INT)e2->bits) == 0)
		n = e1->val - e2->val;

	return (n);
}

/**
 * name      SFBuildCodes - build Shannon/Fanno codes from Huffman bit lengths
 *
 * synopsis  SFBuildCodes(codeTable, codeLens, size)
 *           struct CODETABLE *codeTable         table of shannon-fano codes to be built
 *           struct CODELENS *codeLens           bit lengths of the huffman codes
 *           SHORT size                          number of nodes
 *
 * purpose   to calculate the lengths of the Huffman codes
 *
 **/

VOID SFBuildCodes(struct CODETABLE *codeTable, struct CODELENS *codeLens, SHORT size) {
	USHORT inc, code1, code2;
	USHORT mask1, mask2;
	UBYTE bitLen, bits;
	SHORT val;
	SHORT i;

	/* sort it by it's bitlength in ascending order */
	qsort(codeLens, size, sizeof(struct CODELENS), (INT(CDECL *)(CONST VOID *, CONST VOID *))SortCmp);

	inc = code1 = 0;
	bitLen = 0;

	/*
	 * all non-zero lengths
	 */
	while (--size >= 0 && (bits = codeLens[size].bits) != 0) {

		code1 += inc;

		if (bits != bitLen)
			inc = (USHORT)1 << (16 - (bitLen = bits));

		code2 = ((USHORT)1 << bitLen) - 1;
		mask1 = 0x8000;
		mask2 = 0x0001;

		i = bits;
		while (--i >= 0) {
			if (code1 & mask1)
				code2 ^= mask2;
			mask1 >>= 1;
			mask2 <<= 1;
		}

		val = codeLens[size].val;
		codeTable[val].code = code2;
		codeTable[val].bits = bitLen;
	}
}

/*
 * ComputeVar - compute variance for number sequence
 */
LONG ComputeVar(SHORT num, SHORT sum, LONG sqr) {
	return ((sqr - (LONG)sum * sum / num) / num);
}

ERROR_CODE Inflate(UBYTE *pDstBuf, INT nDstBufSize, UBYTE *pSrcBuf, INT nSrcBufSize, ULONG lOriginalCrc) {
	USHORT header, length;
	USHORT tmpBits;
	USHORT literal;

	struct HBINTREE *lTree, *dTree;
	LONG fileSize;
	USHORT offset;
	UBYTE *lookBackBuf, *endBuf, *pDst;
	ERROR_CODE errCode;

	UBYTE *toPtr, *fromPtr;
	struct CODETABLE *cte;
	SHORT i;

	/* assume no error */
	errCode = ERR_NONE;

	zg.dataSize = nSrcBufSize;
	zg.fileSize = nDstBufSize;
	zg.zipFile = nullptr;
	zg.auxFile = nullptr;
	zg.zfCrcValue = ~0UL;

	memset(&zg.deflate, 0, sizeof(DFSTRUCT));

	fileSize = zg.fileSize;

	/*
	 * build fixed Huffman trees
	 */
	if ((errCode = HuffBuildFixedTrees()) == ERR_NONE) {

		/*
		 * allocate the Sliding Window (also known as the Look-Back Buffer)
		 */
		if ((lookBackBuf = (UBYTE *)BofAlloc(WINDSIZE)) != nullptr) {

			BitReadInit(pSrcBuf, nSrcBufSize);

			/* initialize output */
			pDst = pDstBuf;

			/*
			 * preset sliding window to all zeros so that any seek
			 * past beginning of the file will yield zeros
			 */
			memset(toPtr = lookBackBuf, '\0', WINDSIZE);
			endBuf = lookBackBuf + WINDSIZE;

			do {
				/*
				 * read header from input stream
				 */
				BitReadQuick(&header, 3);

				/*
				 * check for unknown block type
				 */
				if ((tmpBits = (header >> 1)) == DF_RESERVED) {
					errCode = ERR_CRC;
					break;
				}

				/*
				 * if this is a stored block then do a flat copy
				 */
				if (tmpBits == DF_STORED) {

					/*
					 * read length and 1's compliment of length
					 */
					BitReadBytes((UBYTE *)&length, sizeof(USHORT), pSrcBuf, nSrcBufSize);
					BitReadBytes((UBYTE *)&tmpBits, sizeof(USHORT), pSrcBuf, nSrcBufSize);

					/*
					 * verify that the stored data is ok
					 */
					if (length != ~tmpBits) {
						errCode = ERR_CRC;
						break;
					}

					/*
					 * can't go past EOF so there must have been an error
					 */
					if ((fileSize -= length) < 0) {
						errCode = ERR_CRC;
						break;
					}

					/*
					 * until copy complete
					 */
					while (length) {

						/*
						 * not more than SHRT_MAX
						 */
						if ((tmpBits = endBuf - toPtr) > SHRT_MAX)
							tmpBits = SHRT_MAX;

						/*
						 * not more than we can do without wrap-around
						 */
						if ((offset = length) > tmpBits)
							offset = tmpBits;

						/*
						 * read/write data
						 */
						BitReadBytes(toPtr, (SHORT)offset, pSrcBuf, nSrcBufSize);

						memcpy(pDst, toPtr, (SHORT)offset);
						pDst += offset;

						if ((toPtr += offset) >= endBuf)
							toPtr = lookBackBuf;

						/* less to do */
						length -= offset;
					}

					if (errCode)
						break;

					/*
					 * otherwise loop until end of block code is sent
					 */
				} else {

					/*
					 * if using fixed codes
					 */
					if (tmpBits == DF_FIXED) {

						/* set the correct fixed tree pointers */
						lTree = zg.deflate.fhLitTree;
						dTree = zg.deflate.fhDisTree;

						/*
						 * otherwise build the dynamic codes
						 */
					} else {
						/*
						 * build dynamic trees
						 */
						if ((errCode = HuffBuildDynamicTrees()) != ERR_NONE) {
							HuffFreeDynamic();
							break;
						}

						/* set the correct dynamic tree pointers */
						lTree = zg.deflate.dhLitTree;
						dTree = zg.deflate.dhDisTree;
					}

					/*
					 * loop until we get the DF_ENDBLOCK code (256)
					 */
					for (;;) {
						HuffDecode(lTree, &literal);

						if (literal == DF_ENDBLOCK)
							break;

						/*
						 * if literal is the actual character then it's length is 1 byte
						 */
						if (literal < 256) {

							/*
							 * Write out 1 byte
							 */
							*pDst++ = (UBYTE)literal;

							/*
							 * copy literal to output stream
							 */
							*toPtr = (UBYTE)literal;
							if (++toPtr >= endBuf)
								toPtr = lookBackBuf;

							/* one literal character */
							length = 1;

							/*
							 * otherwise we must determine the length based on the Length Codes
							 */
						} else {
							/* get length */
							length = (cte = lenExtraTable + literal - 257)->code;

							/*
							 * read the required number of extra bits
							 * and get the correct length from the table
							 */
							if (cte->bits) {

								BitRead(&tmpBits, cte->bits);

								/* correct length */
								length += tmpBits;
							}

							/*
							 * decode offset from input stream
							 */
							HuffDecode(dTree, &tmpBits);

							/* get offset */
							offset = (cte = disExtraTable + tmpBits)->code;

							/*
							 * read the required number of extra bits
							 * and get the correct offset from the table
							 */
							if (cte->bits) {

								BitRead(&tmpBits, cte->bits);

								/* correct offset */
								offset += tmpBits;
							}

							/*
							 * Go back 'offset' bytes in Look Back Buffer and copy 'length'
							 * bytes into a temporary buffer, and push them back onto queue
							 */
							fromPtr = toPtr - offset;
							if ((USHORT)(toPtr - lookBackBuf) < offset)
								fromPtr += WINDSIZE;

							/*
							 * do fast copy/write if no overlap or wrap-around
							 */
							if ((fromPtr < toPtr) ? ((USHORT)(endBuf - toPtr) > length && fromPtr + length <= toPtr) : ((USHORT)(endBuf - fromPtr) >= length && toPtr + length <= fromPtr)) {

								/*
								 * write string
								 */
								memcpy(pDst, fromPtr, length);
								pDst += length;

								/* add string to look-back buffer */
								memcpy(toPtr, fromPtr, length);
								toPtr += length;

								/*
								 * must copy/write 1 byte at a time
								 */
							} else {

								/*
								 * copy/write 1 byte at a time
								 */
								i = length;
								while (--i >= 0) {

									/*
									 * write out byte
									 */
									*pDst++ = *fromPtr;

									/*
									 * add byte to the look-Back Buffer
									 */
									*toPtr = *fromPtr;
									if (++toPtr >= endBuf)
										toPtr = lookBackBuf;

									/*
									 * next byte
									 */
									if (++fromPtr >= endBuf)
										fromPtr = lookBackBuf;
								}
							}
						}

						/*
						 * can't go past EOF so there must have been an error
						 */
						if ((fileSize -= length) < 0) {
							errCode = ERR_CRC;
							break;
						}
					}

					/* don't need the dynamic trees anymore */
					HuffFreeDynamic();

					/*
					 * stop on errors
					 */
					if (errCode)
						break;
				}

				/*
				 * continue while 1st bit of the block header is Zero
				 */
			} while (!(header & 1));

			/* we are finished with this buffer */
			BofFree(lookBackBuf);

		} else {
			errCode = ERR_MEMORY;
		}
	}

	/* Free the fixed huffman trees */
	HuffFreeFixed();

	// calculate the crc
	//
	if (!errCode && (CalculateCRC(pDstBuf, nDstBufSize) != lOriginalCrc)) {
		errCode = ERR_CRC;
	}

	return (errCode);
}

/**
 *
 * name      HuffBuildFixedTrees - Build fixed Huffman literal, and distance trees
 *
 *
 * synopsis  errCode = HuffBuildFixedTrees()
 *
 * purpose   to create the fixed Huffman codes for deflate/inflate
 *
 *
 * returns   errCode = Error return code
 *
 **/

ERROR_CODE HuffBuildFixedTrees() {
	struct CODELENS *tempTree;
	USHORT bitLength;
	ERROR_CODE errCode;

	struct CODELENS *p;
	SHORT i;

	/* assume no error */
	errCode = ERR_NONE;

	/*
	 * allocate tree descriptor
	 */
	if ((tempTree = (CODELENS *)BofAlloc(sizeof(struct CODELENS) * DF_LITCODES)) == nullptr)
		errCode = ERR_MEMORY;

	else {
		/* assume 8 bits */
		bitLength = 8;

		/*
		 * prepare the fixed huffman literal tree
		 */
		p = tempTree + DF_LITCODES;
		i = DF_LITCODES;
		while (--i >= 0) {

			/*
			 * codes 256 to 279 are 7 bits
			 */
			if (i == 279)
				bitLength = 7;

			/*
			 * codes 144 to 255 are 9 bits
			 */
			else if (i == 255)
				bitLength = 9;

			/*
			 * codes 0 to 143 and 280 to 287 are 8 bits
			 */
			else if (i == 143)
				bitLength = 8;

			p--;
			p->val = i;
			p->bits = (UBYTE)bitLength;
		}

		/*
		 * build the fixed huffman literal/length tree
		 */
		if ((zg.deflate.fhLitTree = HuffBuildTree(tempTree, DF_LITCODES)) == nullptr)
			errCode = ERR_MEMORY;

		else {
			/*
			 * prepare the fixed huffman distance tree - all codes are 5 bits
			 */
			p = tempTree + DF_DISCODES;
			i = DF_DISCODES;
			while (--i >= 0) {
				p--;
				p->val = i;
				p->bits = 5;
			}

			/*
			 * build the fixed huffman distance tree
			 */
			if ((zg.deflate.fhDisTree = HuffBuildTree(tempTree, DF_DISCODES)) == nullptr)
				errCode = ERR_MEMORY;
		}

		/* free tree descriptor */
		BofFree(tempTree);
	}

	return (errCode);
}

/**
 *
 * name      HuffBuildDynamicTrees - Build dynamic Huffman literal, and distance trees
 *
 *
 * synopsis  errCode = HuffBuildDynamicTrees()
 *
 * purpose   to create the dynamic Huffman codes for deflate/inflate
 *
 *
 * returns   errCode = Error return code
 *
 **/

ERROR_CODE HuffBuildDynamicTrees() {
	SHORT temp;

	struct CODELENS *tree;
	struct HBINTREE *bitBinTree;
	SHORT nLitCodes, nDisCodes, nLenCodes;
	SHORT val, n;
	UBYTE bitLength, lastLength;
	ERROR_CODE errCode;

	struct CODELENS *p;
	SHORT i;

	/* assume no error */
	errCode = ERR_NONE;

	/*
	 * allocate tree descriptor
	 */
	if ((tree = (CODELENS *)BofAlloc(sizeof(struct CODELENS) * (DF_LITCODES + DF_DISCODES))) != nullptr) {

		/*
		 * read 5 bits for # of literal codes
		 * read 5 bits for # of distance code
		 * read 4 bits for # of bit length codes
		 */
		BitRead((USHORT *)&temp, (5 + 5 + 4));

		/* ranges from 257 to 288 */
		nLitCodes = (temp & 0x001F) + 257;

		/* ranges from 1 to 32 */
		nDisCodes = ((temp & 0x03E0) >> 5) + 1;

		/* ranges from 4 to 19 */
		nLenCodes = ((temp & 0x3C00) >> 10) + 4;

		/* setup tree for bit length codes */
		memcpy(tree, bitTreeTable, DF_BITCODES * sizeof(struct CODELENS));

		/*
		 * read the bit lengths for the bit length codes
		 */
		p = tree;
		i = nLenCodes;
		while (--i >= 0) {
			BitReadQuick((USHORT *)&temp, 3);
			p->bits = (UBYTE)temp;
			p++;
		}

		/*
		 * build the bit length code tree to decode the lengths for the other trees
		 */
		if ((bitBinTree = HuffBuildTree(tree, nLenCodes)) == nullptr) {
			errCode = ERR_MEMORY;

			/*
			 * build the temporary trees
			 */
		} else {

			/*
			 * trees must be pre-set to zeros so that the HuffBuild function will create
			 * the correct codes given that some entries have zero bit lengths (unused)
			 */
			memset(tree, 0, (DF_LITCODES + DF_DISCODES) * sizeof(struct CODELENS));

			/*
			 * must decode both trees at once since Phil
			 * packs it that way -- we think its a bug
			 */
			n = (SHORT)(nLitCodes + nDisCodes);
			p = tree;

			bitLength = 0;
			i = 0;

			do {
				/*
				 * decode the bit lengths from the input file
				 */
				HuffDecode(bitBinTree, (USHORT *)&temp);

				/* save last length */
				lastLength = bitLength;

				/*
				 * handle special codes 16, 17, and 18
				 */
				switch (temp) {

				/*
				 * copy the last bit length 3 to 6 times
				 */
				case 16:
					/* read another 2 bits */
					BitReadQuick((USHORT *)&temp, 2);
					temp += 3;
					bitLength = lastLength;
					break;

				/*
				 * copy a bit length of Zero 3 to 10 times
				 */
				case 17:
					/* need to read another 3 bits */
					BitReadQuick((USHORT *)&temp, 3);
					temp += 3;
					bitLength = 0;
					break;

				/*
				 * copy a bit length of Zero 11 to 138 times
				 */
				case 18:
					/* need to read another 7 bits */
					BitReadQuick((USHORT *)&temp, 7);
					temp += 11;
					bitLength = 0;
					break;

				/*
				 * all others
				 */
				default:
					if (temp > 18)
						errCode = ERR_CRC;
					else {
						bitLength = (UBYTE)temp;
						temp = 1;
					}
					break;
				}

				if (errCode)
					break;

				/*
				 * for temp codes insert the info for our temparary tables
				 */
				while (--temp >= 0) {

					/*
					 * error if we went to far
					 */
					if (i >= n) {
						errCode = ERR_CRC;
						break;
					}

					/*
					 * correct for distance table
					 */
					if ((val = i) >= nLitCodes)
						val -= nLitCodes;

					/* insert the bit lengths */
					p->val = val;
					p->bits = bitLength;
					p++;
					i++;
				}

				if (errCode)
					break;

			} while (i < n);

			/* don't need the bit length tree any more */
			BofFree(bitBinTree);
		}

		/*
		 * if no errors thus far
		 */
		if (!errCode) {

			/*
			 * build the literal dynamic tree
			 * and build the distance dynamic tree
			 */
			if ((zg.deflate.dhLitTree = HuffBuildTree(tree, nLitCodes)) == nullptr || (zg.deflate.dhDisTree = HuffBuildTree(tree + nLitCodes, nDisCodes)) == nullptr)
				errCode = ERR_MEMORY;
		}

		/* free tree descriptor */
		BofFree(tree);

	} else {
		errCode = ERR_MEMORY;
	}

	return (errCode);
}

/**
 *
 * name      HuffFreeDynamic - Frees dynamic Huffman trees
 *
 *
 * synopsis  HuffFreeDynamic()
 *
 * purpose   to free the HuffBuilt dynamic Huffman binary trees
 *
 **/

VOID HuffFreeDynamic() {
	/*
	 * free the dynamic huffman distance tree
	 */
	if (zg.deflate.dhDisTree != nullptr) {
		BofFree(zg.deflate.dhDisTree);
		zg.deflate.dhDisTree = nullptr;
	}

	/*
	 * free the dynamic huffman literal/length tree
	 */
	if (zg.deflate.dhLitTree != nullptr) {
		BofFree(zg.deflate.dhLitTree);
		zg.deflate.dhLitTree = nullptr;
	}
}

/**
 *
 * name      HuffFreeFixed - Frees fixed Huffman trees
 *
 *
 * synopsis  HuffFreeFixed()
 *
 * purpose   to free the HuffBuilt fixed Huffman binary trees
 *
 **/

VOID HuffFreeFixed() {
	/*
	 * free the fixed huffman distance tree
	 */
	if (zg.deflate.fhDisTree != nullptr) {
		BofFree(zg.deflate.fhDisTree);
		zg.deflate.fhDisTree = nullptr;
	}

	/*
	 * free the fixed huffman literal/length tree
	 */
	if (zg.deflate.fhLitTree != nullptr) {
		BofFree(zg.deflate.fhLitTree);
		zg.deflate.fhLitTree = nullptr;
	}
}

/**
 *
 * name      HuffDecode - Decodes a huffman/shannon-fano binary tree
 *
 *
 * synopsis  binTree = HuffDecode(binTree, rValue)
 *
 * purpose   to decode a Huffman/S-F binary tree by reading bits from the input
 *           stream
 *
 * returns   nothing
 *
 **/
VOID HuffDecode(struct HBINTREE *binTree, USHORT *rValue) {
	USHORT bit;

	/*
	 * keep going until we find the code
	 */
	for (;;) {

		/*
		 * read in 1 bit at a time
		 */
		BitReadQuick1(&bit);

		/*
		 * note: 'right' must come after 'left'
		 */
		if ((binTree = *(&binTree->left + (bit ^ 1)))->left == nullptr) {
			*rValue = binTree->value;
			break;
		}
	}
}

} // namespace Bagel
