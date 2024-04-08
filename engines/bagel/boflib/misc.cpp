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

// Local prototypes
//
static BOOL ReadLine(Common::SeekableReadStream *fp, CHAR *pszBuf);

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

VOID *BofMemAlloc(uint32 lSize, const CHAR *pszFile, INT nLine, BOOL bClear) {
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
		pNewBlock = (VOID *)malloc(lSize);

		if (bClear)
			BofMemSet((UBYTE *)pNewBlock, 0, lSize);

		// If allocation was successfull, then we're outta here
		//
		if (pNewBlock != nullptr) {
			break;
		}

		// Otherwise, we need to free up some memory by flushing old
		// objects from the Cache.
		//

		CCache::Optimize(lSize + 2 * sizeof(uint16) + sizeof(uint32));
	}

	if (pNewBlock == nullptr) {
		LogError(BuildString("Could not allocate %ld bytes, file %s, line %d", lSize, pszFile, nLine));
	}

	return pNewBlock;
}

VOID *BofMemReAlloc(VOID *pOldPtr, uint32 lNewSize, const CHAR *pszFile, INT nLine) {
	// for now, until I fix it, pszFile MUST be valid.
	Assert(pszFile != nullptr);

	VOID *pNewBlock;

	pNewBlock = realloc(pOldPtr, lNewSize);

	return pNewBlock;
}

VOID BofMemFree(VOID *pBuf, const CHAR *pszFile, INT nLine) {
	Assert(pszFile != nullptr);

	free(pBuf);
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
	UBYTE *p1;
	const UBYTE *p2;

	p1 = (UBYTE *)pDst;
	p2 = (const UBYTE *)pSrc;

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

	return errCode;
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

	return errCode;
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


BOOL IsKeyDown(uint32 lKeyCode) {
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

	return bIsDown;
}

#if BOF_MAC
//	Takes a fully qualified directory specification and returns the
//  FSSpec record.
VOID FSSpecFromPath(CHAR *pszPath, FSSpec *fspec) {
	OSErr oserr = noErr;
	Str255 szFullPath;

	strcpy((CHAR *)szFullPath, pszPath);
	StrCToPascal((CHAR *)szFullPath);

	FSMakeFSSpec(0, 0, szFullPath, fspec);
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
