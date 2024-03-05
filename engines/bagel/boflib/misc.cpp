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

#include "common/file.h"
#include "common/textconsole.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/cache.h"
#include "bagel/bagel.h"

namespace Bagel {

#define MAX_LINE_LEN 100

STATIC DWORD lStart;

// Analysis code, haven't decided if this is staying or not...
// #define BOF_MEM_ANALYZE BOF_DEBUG

#if BOF_MEM_ANALYZE
VOID BofUpdateAnalyzeInfo(ULONG lSize, const CHAR *pszFile, INT nLine);
VOID BofDumpAnalyzeInfo();

typedef struct MEMANALYZEBLOCK {
	CHAR m_pszFileName[16];
	INT m_nLineNo;
	INT m_nTotalAllocations;
	INT m_nTotalBytes;
	INT m_nTotalFrees;
} MemAnalyzeBlock;

#define NUM_ANALYZE_SLOTS 500

static MemAnalyzeBlock gAnalyzeMem[NUM_ANALYZE_SLOTS];
static BOOL gAnalyzeIndex = 0;
#endif

// Local prototypes
//
static BOOL ReadLine(Common::SeekableReadStream *fp, CHAR *pszBuf);

#if BOF_MAC
VOID PathNameFromDirID(LONG dirID, SHORT vRefNum, CHAR *pFullPathName);
VOID PathNameFromWD(LONG vRefNum, CHAR *pPathName);
VOID PathNameFromWD2(LONG vRefNum, LONG lDirID, CHAR *pszPathName);
VOID pstrcat(CHAR *dst, CHAR *src);
VOID pstrinsert(CHAR *dst, CHAR *src);
LONG GetMemAllocated();

#define USEMALLOC TRUE
#if BOF_DEBUG
#define USEPOOLMEMORY FALSE
#else
#define USEPOOLMEMORY FALSE
#endif
#endif

#if USEPOOLMEMORY

// the workhorses...
VOID *BofGetPoolMem(ULONG lSize);
VOID BofFreePoolMem(VOID *);
#endif

// Cache the current directory information
#if BOF_MAC
static CHAR g_szCurDir[256] = "";
#endif

/*
 * Filename that ErrorLog writes to
 */
static const CHAR g_szErrorLogFile[MAX_FNAME] = {"ERROR.LOG"};

VOID ErrorLog(const CHAR *format, ...) {
	va_list argptr;

	va_start(argptr, format);
	Common::String str = Common::String::vformat(format, argptr);
	va_end(argptr);

	error("%s", str.c_str());
}

BOOL ProbableTrue(INT nProbability) {
	// Probability must be between 0 and 100 percent
	Assert((nProbability >= 0) && (nProbability <= 100));

	return (int)g_engine->getRandomNumber(99) < nProbability;
}

VOID TimerStart() {
	lStart = g_system->getMillis();
}

DWORD TimerStop() {
	return g_system->getMillis() - lStart;
}

DWORD GetTimer() {
	return g_system->getMillis();
}

VOID Sleep(DWORD milli) {
	g_system->delayMillis(milli);
}

#define ALLOC_FAIL_RETRIES 2

#if BOF_DEBUG

#define MEM_PRE_CODE 0x1234
#define MEM_POST_CODE 0x5678

#if BOF_WIN16
#define MAX_MEM_BLOCKS 1000
#else
#define MAX_MEM_BLOCKS 100000
#endif

#define BM_UNUSED 0 // block has not been allocated
#define BM_USED 1   // block has been allocated
#define BM_MARKED 2 // block has been marked (MARKED implies used)

typedef struct tagMEMBLOCK {
	const CHAR *m_pFile;
	UBYTE *m_pBuf;
	ULONG m_lSize;
	INT m_nLine;
	UINT m_nUsed;
#if BOF_MEM_ANALYZE
	INT m_nAnalyzeIndex;
#endif
} MEMBLOCK;

/*
 * local functions
 */
MEMBLOCK *GetMemBlock(UBYTE *pPtr);

static MEMBLOCK g_cMemBlockList[MAX_MEM_BLOCKS];
static LONG g_lTotalAllocated = 0;
static LONG g_lMaxAllocated = 0;
static LONG g_lLowFreeBlock = 0;
static LONG g_lHighBlock = 0;
static LONG g_lNumUsedBlocks = 0;

/*****************************************************************************
 *
 *  BofMemDefrag     - Re-arranges memory blocks for optimal allocations
 *
 *  DESCRIPTION:     This function will move all mem blocks up to create more
 *                   contiguous free blocks (Only used in BOF_DEBUG mode)
 *
 *  SAMPLE USAGE:
 *  MemOptimize();
 *
 *  RETURNS:  nothing
 *
 *****************************************************************************/
VOID BofMemDefrag() {
	MEMBLOCK *pFree, *pCur, *pStart;

	pCur = &g_cMemBlockList[MAX_MEM_BLOCKS - 1];
	pFree = pStart = &g_cMemBlockList[0];

	while (pFree < pCur) {

		// Find next free block
		//
		while (pFree < pCur) {

			if (pFree->m_nUsed == BM_UNUSED) {
				break;
			}
			pFree++;
		}

		// Find next used block
		//
		while (pFree < pCur) {

			if (pCur->m_nUsed != BM_UNUSED) {
				break;
			}
			pCur--;
		}

		// if there was a free block
		//
		if (pFree < pCur) {

			// then move this block into that slot
			//
			BofMemCopy(pFree, pCur, sizeof(MEMBLOCK));
			*(ULONG *)pFree->m_pBuf = pFree - pStart;
			pCur->m_nUsed = BM_UNUSED;
			pFree++;
			pCur--;
		}
	}

	// Take note of the highest used block (We know that the next block
	// is the next free block.
	//
	g_lHighBlock = g_lNumUsedBlocks - 1;
	if (g_lHighBlock < 0) {
		g_lHighBlock = 0;
	}
}

/*****************************************************************************
 *
 *  MarkMemBlocks    - Marks allocated memory blocks
 *
 *  DESCRIPTION:     This function is used to mark all currently allocated
 *                   memory blocks so a later call to VerifyAllBlocksDeleted()
 *                   will ignore marked blocks, and not report those as undeleted.
 *
 *  SAMPLE USAGE:
 *  MarkMemBlocks();
 *
 *  RETURNS:  nothing
 *
 *****************************************************************************/
VOID MarkMemBlocks() {
	LONG i;

	for (i = 0; i < MAX_MEM_BLOCKS; i++) {

		// mark all used blocks
		//
		if (g_cMemBlockList[i].m_nUsed == BM_USED)
			g_cMemBlockList[i].m_nUsed = BM_MARKED;
	}
}

/*****************************************************************************
 *
 *  GetMemAllocated  - Retrieves the total ammount of currently allocated memory
 *
 *  DESCRIPTION:     This funtion is used internally by the Boffo Memory
 *                   routines.
 *
 *
 *  SAMPLE USAGE:
 *  lBytes = GetMemAllocated();
 *
 *
 *  RETURNS:  LONG = Number of bytes currently allocated
 *
 *****************************************************************************/
LONG GetMemAllocated() {
	return (g_lTotalAllocated);
}

/*****************************************************************************
 *
 *  GetMemBlock      - Retrieves MEMBLOCK struct associated with specified buffer
 *
 *  DESCRIPTION:     This routine is used internally by the Boffo Memory
 *                   routines.
 *
 *
 *  SAMPLE USAGE:
 *  pBlock = GetMemBlock(pPtr);
 *  UBYTE *pPtr;                         buffer to get blok for
 *
 *  RETURNS:  MEMBLOCK * = pointer to memory allocation block
 *
 *****************************************************************************/
MEMBLOCK *GetMemBlock(UBYTE *pPtr) {
	MEMBLOCK *p;
	LONG i;

	// assume we won't find it
	p = nullptr;

	if (pPtr != nullptr) {

		pPtr -= sizeof(USHORT) + sizeof(ULONG);

		i = *((ULONG *)pPtr);

		if ((i >= 0) && (i < MAX_MEM_BLOCKS) && (g_cMemBlockList[i].m_nUsed != BM_UNUSED) && g_cMemBlockList[i].m_pBuf == pPtr) {
			p = &g_cMemBlockList[i];

		} else {

			// run thru the list of allocated blocks to find the specified block
			//
			for (i = 0; i < MAX_MEM_BLOCKS; i++) {
				if ((g_cMemBlockList[i].m_nUsed != BM_UNUSED) && g_cMemBlockList[i].m_pBuf == pPtr) {
					p = &g_cMemBlockList[i];
					LogError(BuildString("Pointer underrun detected in MemBlock: %08lx allocated on line %d in %s", pPtr, p->m_nLine, p->m_pFile));
					break;
				}
			}
			if (p == nullptr) {
				LogError(BuildString("Block not found: %08lx", pPtr))
			}
		}
	}

	return (p);
}

/*****************************************************************************
 *
 *  IsValidPointer   - Determines if specified pointer is a valid memory block
 *
 *  DESCRIPTION:
 *
 *
 *
 *  SAMPLE USAGE:
 *  bValid = IsValidPointer(pPtr);
 *  UBYTE *pPtr;                         pointer to check
 *
 *  RETURNS:  BOOL = TRUE if pointer is a valid memory block
 *
 *****************************************************************************/
BOOL IsValidPointer(UBYTE *pPtr) {
	return (GetMemBlock(pPtr) != nullptr);
}

/*****************************************************************************
 *
 *  VerifyPointer    - Provides error checking during pointer validation
 *
 *  DESCRIPTION:
 *
 *
 *
 *  SAMPLE USAGE:
 *  VerifyPointer(pPtr);
 *  UBYTE *pPtr;                         pointer to check
 *
 *  RETURNS:  nothing
 *
 *****************************************************************************/
VOID VerifyPointer(UBYTE *pPtr) {
	MEMBLOCK *p;

	Assert(pPtr != nullptr);

	if ((p = GetMemBlock(pPtr)) != nullptr) {

		Assert(p->m_pBuf != nullptr);

		UBYTE *pBuf;
		pBuf = p->m_pBuf;
		pBuf += sizeof(ULONG);

		if (*(USHORT *)pBuf != MEM_PRE_CODE) {
			LogError(BuildString("Pointer underrun (File %s, Line %d)", p->m_pFile, p->m_nLine));
		}

		if (*(USHORT *)(pBuf + p->m_lSize + sizeof(USHORT)) != MEM_POST_CODE) {
			LogError(BuildString("Pointer overrun (File %s, Line %d)", p->m_pFile, p->m_nLine));
		}
	} else {
		LogError(BuildString("%08lx is not a valid memory block", pPtr));
	}
}

/*****************************************************************************
 *
 *  VerifyMemoryBlocks - Validates all currently allocated memory blocks
 *
 *  DESCRIPTION:     Checks all allocated memory blocks for pointer overrun
 *                   and underrun, and for buffer overlaps.
 *
 *
 *  SAMPLE USAGE:
 *  VerifyMemoryBlocks();
 *
 *  RETURNS:  nothing
 *
 *****************************************************************************/
VOID VerifyMemoryBlocks() {
	UBYTE *pi, *pjStart, *pjEnd;
	LONG i, j;

	for (i = 0; i < MAX_MEM_BLOCKS; i++) {

		if (g_cMemBlockList[i].m_nUsed != BM_UNUSED) {
			VerifyPointer(g_cMemBlockList[i].m_pBuf + sizeof(USHORT) + sizeof(ULONG));

			// make sure that no memory blocks overlap
			//
			pi = g_cMemBlockList[i].m_pBuf;
			for (j = 0; j < MAX_MEM_BLOCKS; j++) {

				if (g_cMemBlockList[j].m_nUsed != BM_UNUSED) {

					if (i != j) {
						pjStart = g_cMemBlockList[j].m_pBuf;
						pjEnd = pjStart + g_cMemBlockList[j].m_lSize - 1;

						// Does these blocks overlap?
						//
						if (pi >= pjStart && pi <= pjEnd) {
							LogError(BuildString("MEMBLOCK Overlap %d with %d", i, j));
						}
					}
				}
			}
		}
	}
}

/*****************************************************************************
 *
 *  VerifyAllBlocksDeleted - Ensures that all memory blocks have been deleted
 *
 *  DESCRIPTION:     Checks all allocated memory blocks for overrun and
 *                   underrun.
 *
 *
 *  SAMPLE USAGE:
 *  VerifyMemoryBlocks();
 *
 *  RETURNS:  nothing
 *
 *****************************************************************************/
VOID VerifyAllBlocksDeleted() {
	MEMBLOCK *p;
	LONG i, lNumBytes;

	LogInfo(BuildString("Max memory footprint: %ld", g_lMaxAllocated));

	lNumBytes = 0;
	for (i = 0; i < MAX_MEM_BLOCKS; i++) {

		p = &g_cMemBlockList[i];

		// ignoring MARKED blocks, show all blocks that have not yet
		// been deleted
		//
		if (p->m_nUsed == BM_USED) {
			lNumBytes += p->m_lSize;
			LogError(BuildString("Memory Block not freed: Size: %ld, File %s, Line %d", p->m_lSize, p->m_pFile, p->m_nLine));
		}
	}

	if (lNumBytes != 0) {
		LogError(BuildString("Total unfreed memory: %ld bytes", lNumBytes));
	}
}

#endif

VOID *BofMemAlloc(ULONG lSize, const CHAR *pszFile, INT nLine, BOOL bClear) {
	// for now, until I fix it, pszFile MUST be valid.
	Assert(pszFile != nullptr);
	Assert(lSize != 0);

	VOID *pNewBlock;
	INT nRetries;

	// assume failure
	pNewBlock = nullptr;

	// Try a few times to allocate the desired ammount of memory.
	// Flush objects from Cache is neccessary.
	//
	for (nRetries = 0; nRetries < ALLOC_FAIL_RETRIES; nRetries++) {

#if BOF_DEBUG

		MEMBLOCK *p;
		ULONG lSafeSize;
		LONG i, iNextFree;
		BOOL bFound;

		bFound = FALSE;
		if ((g_lLowFreeBlock < MAX_MEM_BLOCKS) && g_cMemBlockList[g_lLowFreeBlock].m_nUsed == BM_UNUSED) {
			bFound = TRUE;
			iNextFree = g_lLowFreeBlock;

		} else if (((iNextFree = g_lHighBlock + 1) < MAX_MEM_BLOCKS) && (g_cMemBlockList[iNextFree].m_nUsed == BM_UNUSED)) {
			bFound = TRUE;

		} else {

			/*
			 * add this memory block to the list of valid blocks
			 */
			for (i = 0; i < MAX_MEM_BLOCKS; i++) {
				if (g_cMemBlockList[i].m_nUsed == BM_UNUSED) {
					iNextFree = i;
					bFound = TRUE;
					break;
				}
			}
		}

		g_lHighBlock = max(g_lHighBlock, iNextFree);

		p = &g_cMemBlockList[iNextFree];

		/* if this fails, then we must increase MAX_MEM_BLOCKS */
		Assert(bFound);

		// One more used memory block
		g_lNumUsedBlocks++;

		/*
		 * allocate a block of memory large enough to hold the original size
		 * plus 4 bytes for pointer overrun tests.  NOTE: In future, we can
		 * do our own memory pooling here.
		 */
		lSafeSize = lSize + 2 * sizeof(USHORT) + sizeof(ULONG);

#if BOF_WIN16
		p->m_pBuf = (UBYTE *)GlobalAllocPtr(GMEM_FIXED | (bClear ? 0 : GMEM_ZEROINIT), lSafeSize);
#else
#if BOF_MAC
#if USEMALLOC
		p->m_pBuf = (UBYTE *)malloc(lSafeSize);
#else
		p->m_pBuf = (UBYTE *)NewPtr(lSafeSize);
#endif
#else
		p->m_pBuf = (UBYTE *)malloc(lSafeSize);
#endif
#endif

		if (p->m_pBuf != nullptr) {

			if (bClear) {
				BofMemSet(p->m_pBuf, 0, lSafeSize);
			}

			/*
			 * fill in our debug info
			 */
			p->m_nLine = nLine;
			p->m_pFile = pszFile;
			p->m_lSize = lSize;
			p->m_nUsed = BM_USED;

			// Keep track of # bytes allocated
			g_lTotalAllocated += p->m_lSize;
			g_lMaxAllocated = max(g_lMaxAllocated, g_lTotalAllocated);

			/*
			 * surround this buffer with check codes
			 */
			*(ULONG *)p->m_pBuf = iNextFree;
			*(USHORT *)(p->m_pBuf + sizeof(ULONG)) = MEM_PRE_CODE;
			*(USHORT *)(p->m_pBuf + lSize + sizeof(USHORT) + sizeof(ULONG)) = MEM_POST_CODE;

#if BOF_MEM_ANALYZE
			BofUpdateAnalyzeInfo(lSize, pszFile, nLines);
#endif
			pNewBlock = (VOID *)(p->m_pBuf + sizeof(USHORT) + sizeof(ULONG));
		}

#else

#if USEPOOLMEMORY
		pNewBlock = BofGetPoolMem(lSize);
#else
#if BOF_WIN16
		pNewBlock = (UBYTE *)GlobalAllocPtr(GMEM_FIXED | (bClear ? 0 : GMEM_ZEROINIT), lSize);
#else
#if BOF_MAC
#if USEMALLOC
		pNewBlock = (VOID *)malloc(lSize);
#else
		pNewBlock = (VOID *)NewPtr(lSize);
#endif
#else
		pNewBlock = malloc(lSize);
		if (bClear)
			BofMemSet((UBYTE *)pNewBlock, 0, lSize);
#endif
#endif
#endif
#endif // !BOF_DEBUG

		// If allocation was successfull, then we're outta here
		//
		if (pNewBlock != nullptr) {
			break;
		}

		// Otherwise, we need to free up some memory by flushing old
		// objects from the Cache.
		//

		CCache::Optimize(lSize + 2 * sizeof(USHORT) + sizeof(ULONG));
	}

	if (pNewBlock == nullptr) {
		LogError(BuildString("Could not allocate %ld bytes, file %s, line %d", lSize, pszFile, nLine));
	}

	return pNewBlock;
}

VOID *BofMemReAlloc(VOID *pOldPtr, ULONG lNewSize, const CHAR *pszFile, INT nLine) {
	// for now, until I fix it, pszFile MUST be valid.
	Assert(pszFile != nullptr);

#if BOF_DEBUG

	MEMBLOCK *p;
	ULONG lSafeSize;

	/*
	 * if not using realloc for 1st time allocation
	 */
	if (pOldPtr != nullptr) {

		/*
		 * pointer must belong to our pool
		 */
		if (IsValidPointer((UBYTE *)pOldPtr)) {

			/*
			 * if not de-allocating this pointer
			 */
			if (lNewSize != 0) {

				/*
				 * get the MEMBLOCK that corresponds to this pointer
				 */
				if ((p = GetMemBlock((UBYTE *)pOldPtr)) != nullptr) {

					/* add boundary */
					lSafeSize = lNewSize + 2 * sizeof(USHORT) + sizeof(ULONG);

					/*
					 * reallocate a new buffer
					 */
#if BOF_WIN16
					if ((p->m_pBuf = (UBYTE *)GlobalReAllocPtr(p->m_pBuf, lSafeSize, 0)) != nullptr) {
#else
					if ((p->m_pBuf = (UBYTE *)realloc(p->m_pBuf, lSafeSize)) != nullptr) {
#endif
						g_lTotalAllocated -= p->m_lSize;

						/*
						 * record new debug info
						 */
						p->m_nLine = nLine;
						p->m_pFile = pszFile;
						p->m_lSize = lSafeSize;

						g_lTotalAllocated += p->m_lSize;
						g_lMaxAllocated = max(g_lMaxAllocated, g_lTotalAllocated);

						/*
						 * surround this buffer with check codes
						 */
						*(USHORT *)(p->m_pBuf + sizeof(ULONG)) = MEM_PRE_CODE;
						*(USHORT *)(p->m_pBuf + lNewSize + sizeof(USHORT) + sizeof(ULONG)) = MEM_POST_CODE;

						return ((VOID *)(p->m_pBuf + sizeof(USHORT) + sizeof(ULONG)));

					} else {
						LogError(BuildString("Could not reallocate %ld bytes. File %s, Line %d", lSafeSize, pszFile, nLine));
					}

				} else {
					LogError(BuildString("Unable to find memory block for %08lx. File %s, Line %d", pOldPtr, pszFile, nLine));
				}

			} else {

				BofMemFree(pOldPtr, pszFile, nLine);
				return (nullptr);
			}
		} else {
			LogError(BuildString("Attempt to reallocate invalid pointer in File %s, line %d", pszFile, nLine));
		}

	} else {
		return (BofMemAlloc(lNewSize, pszFile, nLine, FALSE));
	}
	return (nullptr);

#else
	VOID *pNewBlock;

#if BOF_WIN16
	pNewBlock = (UBYTE *)GlobalReAllocPtr(pOldPtr, lNewSize, 0);
#else
	pNewBlock = realloc(pOldPtr, lNewSize);
#endif

	return (pNewBlock);
#endif
}

VOID BofMemFree(VOID *pBuf, const CHAR *pszFile, INT nLine) {
	Assert(pszFile != nullptr);

#if BOF_DEBUG

	MEMBLOCK *p;

	if (pBuf != nullptr) {

		if ((p = GetMemBlock((UBYTE *)pBuf)) != nullptr) {
			LONG i;

#if BOF_MEM_ANALYZE
			gAnalyzeMem[p->m_nAnalyzeIndex].m_nTotalFrees++;
			gAnalyzeMem[p->m_nAnalyzeIndex].m_nTotalBytes -= p->m_lSize;
#endif

			VerifyPointer((UBYTE *)pBuf);

			i = *(ULONG *)p->m_pBuf;

			// we now have a free block to use
			//
			g_lLowFreeBlock = 0;
			if (i >= 0 && i < MAX_MEM_BLOCKS) {
				g_lLowFreeBlock = i;
			}

			// One less used memory block
			g_lNumUsedBlocks--;

			// Fill freed block with a "Delete" token (0xDD)

			BofMemSet(p->m_pBuf, 0xDD, p->m_lSize);

#if BOF_WIN16

			GlobalFreePtr(p->m_pBuf);
#else
#if BOF_MAC
#if USEMALLOC
			free(p->m_pBuf);
#else
			DisposePtr((Ptr)p->m_pBuf);
#endif
#else
			free(p->m_pBuf);
#endif
#endif

			// Keep track of # bytes allocated and deallocated
			g_lTotalAllocated -= p->m_lSize;

			// Mark this block as now unused. (Also, get rid of the rest of
			// the MEMBLOCK info just in case we try to use it somewhere else)
			//
			p->m_nUsed = BM_UNUSED;
			p->m_pBuf = nullptr;
			p->m_nLine = 0;
			p->m_pFile = nullptr;
			p->m_lSize = 0;

		} else {

			// pBuf is not one of the pointers that we allocated
			//
			LogError(BuildString("Attempt to free invalid pointer in File %s, line %d", pszFile, nLine));
		}

	} else {
		LogError(BuildString("Attempt to free nullptr pointer in file %s, line %d", pszFile, nLine));
	}
#else
#if USEPOOLMEMORY
	BofFreePoolMem(pBuf);
#else

#if BOF_WIN16
	GlobalFreePtr(pBuf);
#else
#if BOF_MAC
#if USEMALLOC
	free(pBuf);
#else
	DisposePtr((CHAR *)pBuf);
#endif
#else
	free(pBuf);
#endif
#endif
#endif

#endif // !BOF_DEBUG
}

Fixed FixedMultiply(Fixed Multiplicand, Fixed Multiplier) {
	Fixed fixResult;
	int64 nTmpNum;

	nTmpNum = (int64)Multiplicand * Multiplier;
	fixResult = (Fixed)(nTmpNum >> 16);

	return fixResult;
}

Fixed FixedDivide(Fixed Dividend, Fixed Divisor) {
	Fixed fixResult;
	int64 nBigNum;

	nBigNum = (int64)Dividend << 16;
	fixResult = (Fixed)(nBigNum / Divisor);

	return fixResult;
}

VOID BofMemSet(VOID *pSrc, UBYTE chByte, LONG lBytes) {
	Assert(pSrc != nullptr);

	UBYTE *pBuf;

	pBuf = (UBYTE *)pSrc;

	while (lBytes-- != 0)
		*pBuf++ = chByte;
}

VOID BofMemCopy(VOID *pDst, const VOID *pSrc, LONG lLength) {
	Assert(pDst != nullptr);
	Assert(pSrc != nullptr);
	Assert(lLength >= 0);
	UBYTE *p1, *p2;

	p1 = (UBYTE *)pDst;
	p2 = (UBYTE *)pSrc;

	while (lLength-- != 0) {
		*p1++ = *p2++;
	}
}

ERROR_CODE WriteIniSetting(const CHAR *pszFileName, const CHAR *pszSection, const CHAR *pszVar, const CHAR *pszNewValue) {
	error("TODO: WriteIniSetting");

#if 0
	// can't acess nullptr pointers
	//
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pszNewValue != nullptr);

	FILE *pInFile, *pOutFile;
	CHAR szTmpFile[MAX_FNAME];
	CHAR szBuf[MAX_LINE_LEN];
	CHAR szSectionBuf[MAX_LINE_LEN];
	CHAR szOldDir[MAX_DIRPATH];
	INT len;
	BOOL bDone;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	GetCurrentDir(szOldDir);
	GotoSystemDir();

	FileTempName(szTmpFile);

	sprintf(szSectionBuf, "[%s]", pszSection);

	if (FileExists(pszFileName)) {

		if ((pInFile = fopen(pszFileName, "r")) != nullptr) {

			if ((pOutFile = fopen(szTmpFile, "w")) != nullptr) {

				len = strlen(szSectionBuf);

				bDone = FALSE;
				do {
					if (!ReadLine(pInFile, szBuf)) {
						fprintf(pOutFile, "\n%s\n", szSectionBuf);
						fprintf(pOutFile, "%s=%s\n", pszVar, pszNewValue);
						bDone = TRUE;
						break;
					}

					fprintf(pOutFile, "%s\n", szBuf);

				} while (strncmp(szBuf, szSectionBuf, len));

				len = strlen(pszVar);

				while (!bDone) {
					if (!ReadLine(pInFile, szBuf)) {
						fprintf(pOutFile, "%s=%s\n", pszVar, pszNewValue);
						bDone = TRUE;
						break;
					}
					if (!strncmp(szBuf, pszVar, len) || (szBuf[0] == '\0'))
						break;

					fprintf(pOutFile, "%s\n", szBuf);
				}

				if (!bDone) {
					if (szBuf[0] == '\0') {
						fprintf(pOutFile, "%s=%s\n", pszVar, pszNewValue);

						do {
							fprintf(pOutFile, "%s\n", szBuf);
						} while (ReadLine(pInFile, szBuf));

					} else {

						fprintf(pOutFile, "%s=%s\n", pszVar, pszNewValue);
						while (ReadLine(pInFile, szBuf)) {
							fprintf(pOutFile, "%s\n", szBuf);
						}
					}
				}

				fclose(pOutFile);
				fclose(pInFile);
				FileDelete(pszFileName);
				FileRename(szTmpFile, pszFileName);

			} else {
				fclose(pInFile);
				errCode = ERR_FOPEN;
			}

		} else {
			errCode = ERR_FOPEN;
		}

	} else {

		if ((pOutFile = fopen(pszFileName, "w")) != nullptr) {
			fprintf(pOutFile, "%s\n", szSectionBuf);
			fprintf(pOutFile, "%s=%s\n", pszVar, pszNewValue);

			fclose(pOutFile);

		} else {
			errCode = ERR_FOPEN;
		}
	}

	SetCurrentDir(szOldDir);

	return (errCode);
#endif
}

ERROR_CODE ReadIniSetting(const CHAR *pszFileName, const CHAR *pszSection, const CHAR *pszVar, CHAR *pszValue, const CHAR *pszDefault, UINT nMaxLen) {
	//  can't acess nullptr pointers
	//
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pszValue != nullptr);
	Assert(pszDefault != nullptr);

	CHAR szOldDir[MAX_DIRPATH];
	CHAR szBuf[MAX_LINE_LEN];
	CHAR szSectionBuf[MAX_LINE_LEN];
	CHAR *p;
	Common::File fp;
	INT len;
	BOOL bEof;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	GetCurrentDir(szOldDir);
	GotoSystemDir();

	// assume we will need to use the default setting
	//
	strncpy(pszValue, pszDefault, nMaxLen - 1);
	pszValue[nMaxLen] = '\0';

	// Open the .INI file
	//
	if (fp.open(pszFileName)) {
		Common::sprintf_s(szSectionBuf, "[%s]", pszSection);
		len = strlen(szSectionBuf);

		bEof = FALSE;
		do {
			if (!ReadLine(&fp, szBuf)) {
				bEof = TRUE;
				break;
			}
		} while (strncmp(szBuf, szSectionBuf, len));

		if (!bEof) {

			len = strlen(pszVar);

			do {
				if (!ReadLine(&fp, szBuf) || (szBuf[0] == '\0')) {
					bEof = TRUE;
					break;
				}

			} while (strncmp(szBuf, pszVar, len));

			if (!bEof) {

				// strip out any comments
				StrReplaceChar(szBuf, ';', '\0');

				// find 1st equal sign
				p = strchr(szBuf, '=');

				// error in .INI file if can't find the equal sign
				//
				if (p != nullptr) {

					p++;

					if (strlen(p) > 0) {
						strncpy(pszValue, p, nMaxLen - 1);
						pszValue[nMaxLen] = '\0';
					}
				} else {
					LogError(BuildString("Error in %s, section: %s, entry: %s", pszFileName, pszSection, pszVar));
					errCode = ERR_FTYPE;
				}
			}
		}

		// we are done with the file
		//
		fp.close();
	}

	SetCurrentDir(szOldDir);

	return errCode;
}

ERROR_CODE WriteIniSetting(const CHAR *pszFileName, const CHAR *pszSection, const CHAR *pszVar, INT nNewValue) {
	// Can't acess nullptr pointers
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);

	CHAR szBuf[20];
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	Common::sprintf_s(szBuf, "%d", nNewValue);
	errCode = WriteIniSetting(pszFileName, pszSection, pszVar, szBuf);

	return (errCode);
}

ERROR_CODE ReadIniSetting(const CHAR *pszFileName, const CHAR *pszSection, const CHAR *pszVar, INT *pValue, INT nDefault) {
	// can't acess nullptr pointers
	//
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pValue != nullptr);

	CHAR szBuf[MAX_LINE_LEN];
	CHAR szSectionBuf[MAX_LINE_LEN];
	CHAR szOldDir[MAX_DIRPATH];
	CHAR *p;
	Common::File fp;
	INT len;
	INT nTmpVal;
	BOOL bEof;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	GetCurrentDir(szOldDir);

	GotoSystemDir();

	// assume we will need to use the default setting
	nTmpVal = nDefault;

	// open the .INI file
	//
	if (fp.open(pszFileName)) {
		Common::sprintf_s(szSectionBuf, "[%s]", pszSection);
		len = strlen(szSectionBuf);

		bEof = FALSE;
		do {
			if (!ReadLine(&fp, szBuf)) {
				bEof = TRUE;
				break;
			}
		} while (strncmp(szBuf, szSectionBuf, len));

		if (!bEof) {
			len = strlen(pszVar);

			do {
				if (!ReadLine(&fp, szBuf) || (szBuf[0] == '\0')) {
					bEof = TRUE;
					break;
				}

			} while (strncmp(szBuf, pszVar, len));

			if (!bEof) {
				// strip out any comments
				StrReplaceChar(szBuf, ';', '\0');

				// find 1st equal sign
				p = strchr(szBuf, '=');

				// error in .INI file if can't find the equal sign
				//
				if (p != nullptr) {

					p++;
					if (strlen(p) > 0) {
						nTmpVal = atoi(p);
					}
				} else {
					LogError(BuildString("Error in %s, section: %s, entry: %s", pszFileName, pszSection, pszVar));
					errCode = ERR_FTYPE;
				}
			}
		}

		// We are done with the file
		fp.close();
	}

	SetCurrentDir(szOldDir);

	if (pValue != nullptr)
		*pValue = nTmpVal;

	return errCode;
}

BOOL ReadLine(Common::SeekableReadStream *fp, CHAR *pszBuf) {
	if (fp->eos())
		return FALSE;

	Common::String line = fp->readLine();
	Common::strcpy_s(pszBuf, MAX_LINE_LEN, line.c_str());
	StrReplaceChar(pszBuf, '\n', '\0');

	return TRUE;
}

VOID Encrypt(VOID *pBuf, LONG size, const CHAR *pszPassword) {
	UBYTE *p;
	const CHAR *pPW, *pStart;

	Assert(pBuf != nullptr);

	pStart = pszPassword;
	if (pszPassword == nullptr) {
		pStart = "\0\0";
	}

	p = (UBYTE *)pBuf;

	pPW = pStart;
	while (--size >= 0) {
		*p ^= (UBYTE)(0xD2 + size + *pPW);
		p++;
		if (*pPW++ == '\0') {
			pPW = pStart;
		}
	}
}

VOID EncryptPartial(VOID *pBuf, LONG fullSize, LONG lBytes, const CHAR *pszPassword) {
	UBYTE *p;
	const CHAR *pPW, *pStart;

	Assert(pBuf != nullptr);

	pStart = pszPassword;
	if (pszPassword == nullptr) {
		pStart = "\0\0";
	}

	p = (UBYTE *)pBuf;

	pPW = pStart;
	while (--lBytes >= 0) {
		fullSize--;
		*p ^= (UBYTE)(0xD2 + fullSize + *pPW);
		p++;
		if (*pPW++ == '\0') {
			pPW = pStart;
		}
	}
}


BOOL IsKeyDown(ULONG lKeyCode) {
	BOOL bIsDown;

	// assume key is not down
	bIsDown = FALSE;

	switch (lKeyCode) {
#if BOF_WINDOWS
	case BKEY_ESC:
		if (::GetAsyncKeyState(VK_ESCAPE)) {
			bIsDown = TRUE;
		}
		break;

	case BKEY_RIGHT:
		if (::GetAsyncKeyState(VK_RIGHT) & 0xf000)
			bIsDown = TRUE;
		break;

	case BKEY_LEFT:
		if (::GetAsyncKeyState(VK_LEFT) & 0xf000)
			bIsDown = TRUE;
		break;

	case BKEY_UP:
		if (::GetAsyncKeyState(VK_UP) & 0xf000)
			bIsDown = TRUE;
		break;

	case BKEY_DOWN:
		if (::GetAsyncKeyState(VK_DOWN) & 0xf000)
			bIsDown = TRUE;
		break;

	/*case BKEY_NUM_LOCK:
	    if (::GetAsyncKeyState(VK_NUMLOCK) & 0xf000)
	        bIsDown = TRUE;
	    break;*/

	case BKEY_ALT:
		if (::GetAsyncKeyState(VK_MENU) & 0xf000)
			bIsDown = TRUE;
		break;

	case BKEY_CTRL:
		if (::GetAsyncKeyState(VK_CONTROL) & 0xf000)
			bIsDown = TRUE;
		break;

	case BKEY_SHIFT:
		if (::GetAsyncKeyState(VK_SHIFT) & 0xf000)
			bIsDown = TRUE;
		break;
#endif
	default:
		LogWarning(BuildString("IsKeyDown() - Invalid key: %08lx\n", lKeyCode));
		break;
	}

	return (bIsDown);
}

#if BOF_MAC
//	Takes a fully qualified directory specification and returns the
//  FSSpec record.  jwl 07.03.96
VOID FSSpecFromPath(CHAR *pszPath, FSSpec *fspec) {
	OSErr oserr = noErr;
	Str255 szFullPath;

	strcpy((CHAR *)szFullPath, pszPath);
	StrCToPascal((CHAR *)szFullPath);

	FSMakeFSSpec(0, 0, szFullPath, fspec);
}
#endif

#define NUM_OPT_SIZES 56
static BOOL gPreAllocInit = FALSE;
typedef struct {
	INT nSize;
	INT nAlignedSize;
	INT nOccurrences;
	VOID *pMemBuff;
	VOID *pEndMemBuff;
	USHORT *pMemUsed;
	USHORT nIndex;
} MEMOPTTABLE;
static MEMOPTTABLE gMemOptTable[NUM_OPT_SIZES] = {
	2, 4, 2291, nullptr, nullptr, nullptr, 0,
	3, 4, 15801, nullptr, nullptr, nullptr, 0,
	4, 4, 1960, nullptr, nullptr, nullptr, 0,
	5, 8, 600, nullptr, nullptr, nullptr, 0,
	6, 8, 751, nullptr, nullptr, nullptr, 0,
	7, 8, 527, nullptr, nullptr, nullptr, 0,
	8, 8, 1364, nullptr, nullptr, nullptr, 0,
	9, 12, 1093, nullptr, nullptr, nullptr, 0,
	10, 12, 1827, nullptr, nullptr, nullptr, 0,
	11, 12, 2180, nullptr, nullptr, nullptr, 0,
	12, 12, 39000, nullptr, nullptr, nullptr, 0,
	13, 16, 523, nullptr, nullptr, nullptr, 0,
	14, 16, 538, nullptr, nullptr, nullptr, 0,
	15, 16, 527, nullptr, nullptr, nullptr, 0,
	16, 16, 343, nullptr, nullptr, nullptr, 0,
	17, 20, 650, nullptr, nullptr, nullptr, 0,
	18, 20, 820, nullptr, nullptr, nullptr, 0,
	19, 20, 486, nullptr, nullptr, nullptr, 0,
	20, 20, 18455, nullptr, nullptr, nullptr, 0,
	21, 24, 1845, nullptr, nullptr, nullptr, 0,
	25, 28, 100, nullptr, nullptr, nullptr, 0,
	26, 28, 320, nullptr, nullptr, nullptr, 0,
	27, 28, 200, nullptr, nullptr, nullptr, 0,
	28, 28, 450, nullptr, nullptr, nullptr, 0,
	29, 32, 100, nullptr, nullptr, nullptr, 0,
	30, 32, 440, nullptr, nullptr, nullptr, 0,
	31, 32, 190, nullptr, nullptr, nullptr, 0,
	32, 32, 70, nullptr, nullptr, nullptr, 0,
	34, 36, 160, nullptr, nullptr, nullptr, 0,
	35, 36, 250, nullptr, nullptr, nullptr, 0,
	36, 36, 1265, nullptr, nullptr, nullptr, 0,
	37, 40, 649, nullptr, nullptr, nullptr, 0,
	38, 40, 370, nullptr, nullptr, nullptr, 0,
	39, 40, 425, nullptr, nullptr, nullptr, 0,
	40, 40, 1130, nullptr, nullptr, nullptr, 0,
	56, 56, 6780, nullptr, nullptr, nullptr, 0,
	60, 60, 4380, nullptr, nullptr, nullptr, 0,
	64, 64, 950, nullptr, nullptr, nullptr, 0,
	68, 68, 970, nullptr, nullptr, nullptr, 0,
	76, 76, 1640, nullptr, nullptr, nullptr, 0,
	80, 80, 320, nullptr, nullptr, nullptr, 0,
	81, 84, 165, nullptr, nullptr, nullptr, 0,
	82, 84, 340, nullptr, nullptr, nullptr, 0,
	83, 84, 600, nullptr, nullptr, nullptr, 0,
	84, 84, 2270, nullptr, nullptr, nullptr, 0,
	85, 88, 240, nullptr, nullptr, nullptr, 0,
	86, 88, 66, nullptr, nullptr, nullptr, 0,
	88, 88, 1230, nullptr, nullptr, nullptr, 0,
	92, 92, 90, nullptr, nullptr, nullptr, 0,
	100, 100, 230, nullptr, nullptr, nullptr, 0,
	112, 112, 2540, nullptr, nullptr, nullptr, 0,
	124, 124, 210, nullptr, nullptr, nullptr, 0,
	132, 132, 120, nullptr, nullptr, nullptr, 0,
	160, 160, 70, nullptr, nullptr, nullptr, 0,
	272, 272, 84, nullptr, nullptr, nullptr, 0,
	444, 444, 100, nullptr, nullptr, nullptr, 0
};

#if USEPOOLMEMORY
#define LONGSIZE 4
VOID *BofGetPoolMem(ULONG lSize) {
	VOID *pNewBlock = nullptr;
	INT i;

	// jwl preallocate all our frequently accessed objects
	if (gPreAllocInit == FALSE) {
		for (i = 0; i < NUM_OPT_SIZES; i++) {
			INT nBytesNeeded = gMemOptTable[i].nOccurrences;
#if USEMALLOC
			gMemOptTable[i].pMemBuff = malloc((gMemOptTable[i].nAlignedSize + LONGSIZE) * gMemOptTable[i].nOccurrences);
			gMemOptTable[i].pMemUsed = (USHORT *)malloc(nBytesNeeded * sizeof(SHORT));
#else
			gMemOptTable[i].pMemBuff = NewPtr((gMemOptTable[i].nAlignedSize + LONGSIZE) * gMemOptTable[i].nOccurrences);
			gMemOptTable[i].pMemUsed = (USHORT *)NewPtr(nBytesNeeded * sizeof(SHORT));
#endif
			gMemOptTable[i].pEndMemBuff = (CHAR *)gMemOptTable[i].pMemBuff + (gMemOptTable[i].nAlignedSize + LONGSIZE) * gMemOptTable[i].nOccurrences;
			gMemOptTable[i].nIndex = gMemOptTable[i].nOccurrences - 1;

			if (gMemOptTable[i].pMemBuff == nullptr || gMemOptTable[i].pMemUsed == nullptr) {
				LogError(BuildString("BofGetPoolMem failed to allocate %d bytes, prepare to crash and burn!", (gMemOptTable[i].nAlignedSize + LONGSIZE) * gMemOptTable[i].nOccurrences));
			}

			// fill in the indices
			for (USHORT j = 0; j < nBytesNeeded; j++) {
				gMemOptTable[i].pMemUsed[j] = j;
			}
		}
		gPreAllocInit = TRUE;
	}

	// Search the table, see if we have a size that we are able to use from the prealloc pool
	for (i = 0; i < NUM_OPT_SIZES; i++) {
		if (lSize == gMemOptTable[i].nSize) {
			if (gMemOptTable[i].nIndex != 0) {
				USHORT nNextAvail = gMemOptTable[i].pMemUsed[gMemOptTable[i].nIndex];
				pNewBlock = (CHAR *)gMemOptTable[i].pMemBuff + nNextAvail * (gMemOptTable[i].nAlignedSize + LONGSIZE);

				Assert(pNewBlock >= gMemOptTable[i].pMemBuff && pNewBlock < gMemOptTable[i].pEndMemBuff);

				*(USHORT *)pNewBlock = (USHORT)i;
				(CHAR *)pNewBlock += 2;

				*(USHORT *)pNewBlock = (USHORT)gMemOptTable[i].nIndex;
				(CHAR *)pNewBlock += 2;

				Assert(gMemOptTable[i].pMemUsed[gMemOptTable[i].nIndex] == nNextAvail);

				gMemOptTable[i].nIndex--;
			}

			Assert(gMemOptTable[i].nIndex != 0);
		}
	}

	// if we didn't get it from the prealloc area, then get it from the heap.
	if (pNewBlock == nullptr) {
#if USEMALLOC
		pNewBlock = malloc(lSize + LONGSIZE);
#else
		pNewBlock = NewPtr(lSize + LONGSIZE);
#endif
		if (pNewBlock) {
			*(LONG *)pNewBlock = (LONG) - 1;
			(CHAR *)pNewBlock += LONGSIZE;
		}
	}

	Assert(pNewBlock != nullptr);
	if (pNewBlock == nullptr) {
		LogError(BuildString("BofGetPoolMem failed to allocate %d bytes, prepare to crash and burn!", lSize));
	}

	return pNewBlock;
}

VOID BofFreePoolMem(VOID *pBuf) {
	INT i;
	// Go through each of our preallocated buffers and see if this came from one of them.
	// If we got here, then we malloc'd it

	BOOL bFound = FALSE;
	CHAR *pActual = (CHAR *)pBuf;
	pActual -= LONGSIZE;

	USHORT nExpectedGroup = *(USHORT *)pActual;
	pActual += 2;
	USHORT nExpectedIndex = *(USHORT *)pActual;
	pActual += 2;
	pActual -= LONGSIZE;

	if (nExpectedGroup >= 0 && nExpectedGroup < NUM_OPT_SIZES) {
		i = nExpectedGroup;
		// for (i = 0; i < NUM_OPT_SIZES; i++) {
		if (pActual >= gMemOptTable[i].pMemBuff && pActual < gMemOptTable[i].pEndMemBuff) {
			INT nReturnIndex = ((CHAR *)pActual - gMemOptTable[i].pMemBuff) / (gMemOptTable[i].nAlignedSize + LONGSIZE);

			Assert(((pActual - gMemOptTable[i].pMemBuff) % (gMemOptTable[i].nAlignedSize + LONGSIZE)) == 0);
			Assert(nExpectedGroup == i);

			// This one will now be marked as available.
			gMemOptTable[i].nIndex++;

			Assert(gMemOptTable[i].nIndex < gMemOptTable[i].nOccurrences);

			USHORT nNextAvail = gMemOptTable[i].nIndex;
			USHORT nHoldNext = gMemOptTable[i].pMemUsed[nNextAvail];

			Assert(nExpectedIndex != 0xFFFF);

			gMemOptTable[i].pMemUsed[nNextAvail] = nReturnIndex;

			CHAR *p = (CHAR *)gMemOptTable[i].pMemBuff + nHoldNext * (gMemOptTable[i].nAlignedSize + LONGSIZE);
			*(USHORT *)p = i;
			p += 2;

			*(USHORT *)p = nExpectedIndex;
			p += 2;

			gMemOptTable[i].pMemUsed[nExpectedIndex] = nHoldNext;

			bFound = TRUE;
		}
	}

	// Get rid of it the old fashioned way if it was not prealloc'd
	if (bFound == FALSE) {
#if BOF_MAC
#if USEMALLOC
		free(pActual);
#else
		DisposePtr((CHAR *)pActual);
#endif
#else
		free(pActual);
#endif
	}
}
#endif

#if BOF_MEM_ANALYZE

// jwl 10.10.96 keep track of who is calling us to get memory...
VOID BofUpdateAnalyzeInfo(ULONG lSize, const CHAR *pszFile, INT nLine) {

	if (gAnalyzeIndex == 0) {
		for (INT i = 0; i < NUM_ANALYZE_SLOTS; i++) {
			gAnalyzeMem[i].m_nLineNo = 0;
			gAnalyzeMem[i].m_nTotalAllocations = 0;
			gAnalyzeMem[i].m_nTotalBytes = 0;
			gAnalyzeMem[i].m_nTotalFrees = 0;

			memset(gAnalyzeMem[i].m_pszFileName, 0, 16);
		}
	}

	// Find if we already have an index for this guy...
	INT j;
	for (j = 0; j < NUM_ANALYZE_SLOTS; j++) {
		if (gAnalyzeMem[j].m_nLineNo == nLine &&
		        (strcmp(gAnalyzeMem[j].m_pszFileName, pszFile) == 0)) {
			gAnalyzeMem[j].m_nTotalAllocations++;
			gAnalyzeMem[j].m_nTotalBytes += lSize;
			p->m_nAnalyzeIndex = j;
			break;
		}
	}

	// If not, then enter one!
	if (j == NUM_ANALYZE_SLOTS) {
		gAnalyzeMem[gAnalyzeIndex].m_nLineNo = nLine;
		gAnalyzeMem[gAnalyzeIndex].m_nTotalAllocations = 1;
		gAnalyzeMem[gAnalyzeIndex].m_nTotalBytes = lSize;
		strcpy(gAnalyzeMem[gAnalyzeIndex].m_pszFileName, pszFile);
		p->m_nAnalyzeIndex = gAnalyzeIndex;
		gAnalyzeIndex++;
		if (gAnalyzeIndex >= NUM_ANALYZE_SLOTS) {
			LogError("gAnalyzeIndex >= NUM_ANALYZE_SLOTS");
		}
	}
}

VOID BofDumpAnalyzeInfo() {

	// Dump the analysis data
	for (j = 0; j < gAnalyzeIndex; j++) {
		LogError(BuildString("Alloc called from %s; line %d; %d Times; %d Freed; %d in Memory; TotalBytes = %d",
		                     gAnalyzeMem[j].m_pszFileName,
		                     gAnalyzeMem[j].m_nLineNo,
		                     gAnalyzeMem[j].m_nTotalAllocations,
		                     gAnalyzeMem[j].m_nTotalFrees,
		                     gAnalyzeMem[j].m_nTotalAllocations - gAnalyzeMem[j].m_nTotalFrees,
		                     gAnalyzeMem[j].m_nTotalBytes));
	}
}
#endif

INT MapWindowsPointSize(INT pointSize) {
	INT mappedPointSize = pointSize;
#if BOF_MAC
	switch (pointSize) {
	case 8:
		mappedPointSize = 6;
		break;
	case 10:
		mappedPointSize = 8;
		break;
	case 12:
		mappedPointSize = 10;
		break;
	case 14:
	case 15:
	case 16:
		mappedPointSize = 12;
		break;
	case 18:
		mappedPointSize = 14;
		break;
	case 20:
		mappedPointSize = 16;
		break;
	case 24:
		mappedPointSize = 18;
		break;
	case 28:
		mappedPointSize = 18;
		break;
	case 36:
		mappedPointSize = 24;
		break;
	default:
#if DEVELOPMENT
		DebugStr("\pMapWindowsPointSize invalid size");
#endif
		break;
	}
#endif
	return mappedPointSize;
}

void LIVEDEBUGGING(CHAR *pMessage1, CHAR *pMessage2) {
#if BOF_MAC
	if (gLiveDebugging == TRUE) {
		MacMessageBox(pMessage1, pMessage2);
	}
#endif
	return;
}

} // namespace Bagel
