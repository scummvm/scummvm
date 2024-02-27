
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

#ifndef BAGEL_BOFLIB_MISC_H
#define BAGEL_BOFLIB_MISC_H

#include "bagel/boflib/boffo.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/bof_error.h"

namespace Bagel {

VOID TimeUnpack(USHORT num, USHORT *hour, USHORT *min, USHORT *sec);
USHORT TimePack(USHORT hour, USHORT min, USHORT sec);
VOID DateUnpack(USHORT num, USHORT *year, USHORT *month, USHORT *day);
USHORT DatePack(USHORT year, USHORT month, USHORT day);

VOID MessageLog(const CHAR *, const CHAR *, ...);
VOID ErrorLog(const CHAR *, ...);
VOID SetErrLogFile(const CHAR *pszFileName);

LONG FileLength(const CHAR *);
CHAR *FileGetTempDir();
CHAR *FileTempName(CHAR *);
ERROR_CODE FileRename(const CHAR *pszOldName, const CHAR *pszNewName);
ERROR_CODE FileDelete(const CHAR *);
BOOL FileExists(const CHAR *);
BOOL FileIsDirectory(const CHAR *);
CHAR *FileGetFullPath(CHAR *pszDstBuf, const CHAR *pszSrcBuf);
BOOL IsDriveWriteLocked();
LONG GetFreeDiskSpace(const CHAR *pszDrive);
INT MapWindowsPointSize(INT pointSize);

BOOL ProbableTrue(INT);

VOID TimerStart();
DWORD TimerStop();
DWORD GetTimer();

Fixed FixedDivide(Fixed Dividend, Fixed Divisor);
Fixed FixedMultiply(Fixed Multiplicand, Fixed Multiplier);

#define IntToFixed(i) (Fixed)(((long)(i)) << 16)
#define FixedToInt(f) (int)(((long)f) >> 16)

//////////////////////////////////////////////////////////////////////////////
//
// Memory allocation routines (Should be put into their own MEMORY module)
//
//////////////////////////////////////////////////////////////////////////////


/**
 * Allocates a memory block of specified size
 * @param nSize			Number of bytes to allocate
 * @param pFile			Source file name
 * @param nLine			Source file line number
 * @param bClear		true if buffer should be cleared
 * @return				Pointer to new buffer
 */
extern void *BofMemAlloc(uint32 nSize, const char *pFile, int nLine, bool bClear);

/**
 * Re-Allocates a memory block to the specified size
 * @param pOldPointer	Old memory pointer
 * @param nNewSize		New buffer size
 * @param pFile			Source file name
 * @param nLine			Source file line number
 * @return				Pointer to new buffer
 */
extern void *BofMemReAlloc(void *pOldPtr, uint32 nNewSize, const char *pFile, int nLine);

/**
 * Frees specified memory block
 * @param pBuf			Buffer to de-allocate
 * @param pFile			Source file name
 * @param nLine			Source file line number
 **/
extern void BofMemFree(void *pBuf, const char *pszFile, int nLine);

#define BofAlloc(n) BofMemAlloc((n), __FILE__, __LINE__, FALSE)
#define BofCAlloc(n, m) BofMemAlloc((ULONG)(n) * (m), __FILE__, __LINE__, TRUE)
#define BofReAlloc(p, n) BofMemReAlloc((p), (n), __FILE__, __LINE__)
#define BofFree(p) BofMemFree((p), __FILE__, __LINE__)

#if BOF_DEBUG
extern VOID BofMemDefrag();
extern VOID MarkMemBlocks();
extern VOID VerifyMemoryBlocks();
extern VOID VerifyAllBlocksDeleted();
extern VOID VerifyPointer(UBYTE HUGE *pPtr);
extern BOOL IsValidPointer(UBYTE HUGE *pPtr);
#if BOF_MEMTEST
extern VOID BofMemTest();
#endif
#endif

#define BofMemSet memset
#define BofMemCopy memcpy
#define BofMemMove memmove

extern ULONG GetFreePhysMem();
extern ULONG GetFreeMem();

//////////////////////////////////////////////////////////////////////////////
//
// String routines (Should be put into their own STRING module)
//
//////////////////////////////////////////////////////////////////////////////

VOID StrWordCaps(CHAR *);
INT StrFreqMatch(const CHAR *, const CHAR *);
BOOL StrCompare(const CHAR *, const CHAR *);
INT StrCharCount(const CHAR *, CHAR);
CHAR *StriStr(const CHAR *, const CHAR *);
VOID StrUprStr(CHAR *, const CHAR *);
VOID StrLwrStr(CHAR *, const CHAR *);
VOID StrCpyStripChar(CHAR *, const CHAR *, CHAR);
CHAR *StrStripChar(CHAR *, CHAR);
CHAR *StrReplaceChar(CHAR *, CHAR, CHAR);
VOID MemReplaceChar(UBYTE *pBuf, UBYTE chOld, UBYTE chNew, LONG lSize);
CHAR *StrReplaceStr(CHAR *pszBuf, const CHAR *, const CHAR *);
VOID StrInvertCase(CHAR *pszBuf);
CHAR *StrCToPascal(CHAR *pszBuffer);
CHAR *StrPascalToC(CHAR *pszBuffer);

ERROR_CODE WriteIniSetting(const CHAR *, const CHAR *, const CHAR *, const CHAR *);
ERROR_CODE ReadIniSetting(const CHAR *, const CHAR *, const CHAR *, CHAR *, const CHAR *, UINT);
ERROR_CODE WriteIniSetting(const CHAR *, const CHAR *, const CHAR *, INT);
ERROR_CODE ReadIniSetting(const CHAR *, const CHAR *, const CHAR *, INT *, INT);

VOID Encrypt(VOID *, LONG, const CHAR *pPassword = NULL);
#define Decrypt Encrypt

// jwl 1.6.97 encrypt/decrypt just part of a buffer
VOID EncryptPartial(VOID *, LONG, LONG, const CHAR *pPassword = NULL);
#define DecryptPartial EncryptPartial

ERROR_CODE GetCurrentDir(CHAR *pszDirectory);
ERROR_CODE SetCurrentDir(CHAR *pszDirectory);
ERROR_CODE GotoSystemDir();
ERROR_CODE GetSystemDir(CHAR *pszDirectory);
VOID GetInstallPath(CHAR *pszDirectory);
VOID SetInstallPath(CHAR *pszDirectory);

#define ABS(x) ((x) > 0 ? (x) : -(x))

BOOL IsKeyDown(ULONG lKeyCode);

// Debugging
void LIVEDEBUGGING(CHAR *pMessage1, CHAR *pMessage2);

// Globals
extern BOOL gLiveDebugging;

} // namespace Bagel

#endif
