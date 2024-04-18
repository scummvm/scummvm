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
#include "bagel/boflib/string_functions.h"
#include "bagel/boflib/file_functions.h"

namespace Bagel {

#define MAX_LINE_LEN 100

static uint32 lStart;

// Local prototypes
//
static bool ReadLine(Common::SeekableReadStream *fp, char *pszBuf);

void ErrorLog(const char *format, ...) {
	va_list argptr;

	va_start(argptr, format);
	Common::String str = Common::String::vformat(format, argptr);
	va_end(argptr);

	error("%s", str.c_str());
}

bool ProbableTrue(int nProbability) {
	// Probability must be between 0 and 100 percent
	Assert((nProbability >= 0) && (nProbability <= 100));

	return (int)g_engine->getRandomNumber(99) < nProbability;
}

void TimerStart() {
	lStart = g_system->getMillis();
}

uint32 TimerStop() {
	return g_system->getMillis() - lStart;
}

uint32 GetTimer() {
	return g_system->getMillis();
}

void Sleep(uint32 milli) {

	g_system->delayMillis(milli);
}

#define ALLOC_FAIL_RETRIES 2

void *BofMemAlloc(uint32 lSize, const char *pszFile, int nLine, bool bClear) {
	// for now, until I fix it, pszFile MUST be valid.
	Assert(pszFile != nullptr);
	Assert(lSize != 0);

	void *pNewBlock;
	int nRetries;

	// assume failure
	pNewBlock = nullptr;

	// Try a few times to allocate the desired ammount of memory.
	// Flush objects from Cache is neccessary.
	//
	for (nRetries = 0; nRetries < ALLOC_FAIL_RETRIES; nRetries++) {
		pNewBlock = (void *)malloc(lSize);

		if (bClear)
			BofMemSet((byte *)pNewBlock, 0, lSize);

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

void *BofMemReAlloc(void *pOldPtr, uint32 lNewSize, const char *pszFile, int nLine) {
	// for now, until I fix it, pszFile MUST be valid.
	Assert(pszFile != nullptr);

	void *pNewBlock;

	pNewBlock = realloc(pOldPtr, lNewSize);

	return pNewBlock;
}

void BofMemFree(void *pBuf, const char *pszFile, int nLine) {
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

void BofMemSet(void *pSrc, byte chByte, int32 lBytes) {
	Assert(pSrc != nullptr);

	byte *pBuf;

	pBuf = (byte *)pSrc;

	while (lBytes-- != 0)
		*pBuf++ = chByte;
}

void BofMemCopy(void *pDst, const void *pSrc, int32 lLength) {
	Assert(pDst != nullptr);
	Assert(pSrc != nullptr);
	Assert(lLength >= 0);
	byte *p1;
	const byte *p2;

	p1 = (byte *)pDst;
	p2 = (const byte *)pSrc;

	while (lLength-- != 0) {
		*p1++ = *p2++;
	}
}

ErrorCode WriteIniSetting(const char *pszFileName, const char *pszSection, const char *pszVar, const char *pszNewValue) {
	error("TODO: WriteIniSetting");

#if 0
	// can't acess nullptr pointers
	//
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pszNewValue != nullptr);

	FILE *pInFile, *pOutFile;
	char szTmpFile[MAX_FNAME];
	char szBuf[MAX_LINE_LEN];
	char szSectionBuf[MAX_LINE_LEN];
	char szOldDir[MAX_DIRPATH];
	int len;
	bool bDone;
	ErrorCode errCode;

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

				bDone = false;
				do {
					if (!ReadLine(pInFile, szBuf)) {
						fprintf(pOutFile, "\n%s\n", szSectionBuf);
						fprintf(pOutFile, "%s=%s\n", pszVar, pszNewValue);
						bDone = true;
						break;
					}

					fprintf(pOutFile, "%s\n", szBuf);

				} while (strncmp(szBuf, szSectionBuf, len));

				len = strlen(pszVar);

				while (!bDone) {
					if (!ReadLine(pInFile, szBuf)) {
						fprintf(pOutFile, "%s=%s\n", pszVar, pszNewValue);
						bDone = true;
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

ErrorCode ReadIniSetting(const char *pszFileName, const char *pszSection, const char *pszVar, char *pszValue, const char *pszDefault, uint32 nMaxLen) {
	//  can't acess nullptr pointers
	//
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pszValue != nullptr);
	Assert(pszDefault != nullptr);

	char szOldDir[MAX_DIRPATH];
	char szBuf[MAX_LINE_LEN];
	char szSectionBuf[MAX_LINE_LEN];
	char *p;
	Common::File fp;
	int len;
	bool bEof;
	ErrorCode errCode;

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

		bEof = false;
		do {
			if (!ReadLine(&fp, szBuf)) {
				bEof = true;
				break;
			}
		} while (strncmp(szBuf, szSectionBuf, len));

		if (!bEof) {

			len = strlen(pszVar);

			do {
				if (!ReadLine(&fp, szBuf) || (szBuf[0] == '\0')) {
					bEof = true;
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

ErrorCode WriteIniSetting(const char *pszFileName, const char *pszSection, const char *pszVar, int nNewValue) {
	// Can't acess nullptr pointers
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);

	char szBuf[20];
	ErrorCode errCode;

	// assume no error
	errCode = ERR_NONE;

	Common::sprintf_s(szBuf, "%d", nNewValue);
	errCode = WriteIniSetting(pszFileName, pszSection, pszVar, szBuf);

	return errCode;
}

ErrorCode ReadIniSetting(const char *pszFileName, const char *pszSection, const char *pszVar, int *pValue, int nDefault) {
	// can't acess nullptr pointers
	//
	Assert(pszFileName != nullptr);
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pValue != nullptr);

	char szBuf[MAX_LINE_LEN];
	char szSectionBuf[MAX_LINE_LEN];
	char szOldDir[MAX_DIRPATH];
	char *p;
	Common::File fp;
	int len;
	int nTmpVal;
	bool bEof;
	ErrorCode errCode;

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

		bEof = false;
		do {
			if (!ReadLine(&fp, szBuf)) {
				bEof = true;
				break;
			}
		} while (strncmp(szBuf, szSectionBuf, len));

		if (!bEof) {
			len = strlen(pszVar);

			do {
				if (!ReadLine(&fp, szBuf) || (szBuf[0] == '\0')) {
					bEof = true;
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

bool ReadLine(Common::SeekableReadStream *fp, char *pszBuf) {
	if (fp->eos())
		return false;

	Common::String line = fp->readLine();
	Common::strcpy_s(pszBuf, MAX_LINE_LEN, line.c_str());
	StrReplaceChar(pszBuf, '\n', '\0');

	return true;
}

void Encrypt(void *pBuf, int32 size, const char *pszPassword) {
	byte *p;
	const char *pPW, *pStart;

	Assert(pBuf != nullptr);

	pStart = pszPassword;
	if (!pszPassword || strlen(pszPassword) == 0) {
		pStart = "\0\0";
	}

	p = (byte *)pBuf;

	pPW = pStart;
	while (--size >= 0) {
		*p ^= (byte)(0xD2 + size + *pPW);
		p++;
		if (*pPW++ == '\0') {
			pPW = pStart;
		}
	}
}

void EncryptPartial(void *pBuf, int32 fullSize, int32 lBytes, const char *pszPassword) {
	byte *p;
	const char *pPW, *pStart;

	Assert(pBuf != nullptr);

	pStart = pszPassword;
	if (!pszPassword || strlen(pszPassword) == 0) {
		pStart = "\0\0";
	}

	p = (byte *)pBuf;

	pPW = pStart;
	while (--lBytes >= 0) {
		fullSize--;
		*p ^= (byte)(0xD2 + fullSize + *pPW);
		p++;
		if (*pPW++ == '\0') {
			pPW = pStart;
		}
	}
}


bool IsKeyDown(uint32 lKeyCode) {
	bool bIsDown;

	// assume key is not down
	bIsDown = false;

	switch (lKeyCode) {
#if BOF_WINDOWS
	case BKEY_ESC:
		if (::GetAsyncKeyState(VK_ESCAPE)) {
			bIsDown = true;
		}
		break;

	case BKEY_RIGHT:
		if (::GetAsyncKeyState(VK_RIGHT) & 0xf000)
			bIsDown = true;
		break;

	case BKEY_LEFT:
		if (::GetAsyncKeyState(VK_LEFT) & 0xf000)
			bIsDown = true;
		break;

	case BKEY_UP:
		if (::GetAsyncKeyState(VK_UP) & 0xf000)
			bIsDown = true;
		break;

	case BKEY_DOWN:
		if (::GetAsyncKeyState(VK_DOWN) & 0xf000)
			bIsDown = true;
		break;

	/*case BKEY_NUM_LOCK:
	    if (::GetAsyncKeyState(VK_NUMLOCK) & 0xf000)
	        bIsDown = true;
	    break;*/

	case BKEY_ALT:
		if (::GetAsyncKeyState(VK_MENU) & 0xf000)
			bIsDown = true;
		break;

	case BKEY_CTRL:
		if (::GetAsyncKeyState(VK_CONTROL) & 0xf000)
			bIsDown = true;
		break;

	case BKEY_SHIFT:
		if (::GetAsyncKeyState(VK_SHIFT) & 0xf000)
			bIsDown = true;
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
void FSSpecFromPath(char *pszPath, FSSpec *fspec) {
	OSErr oserr = noErr;
	Str255 szFullPath;

	strcpy((char *)szFullPath, pszPath);
	StrCToPascal((char *)szFullPath);

	FSMakeFSSpec(0, 0, szFullPath, fspec);
}
#endif

int MapWindowsPointSize(int pointSize) {
	int mappedPointSize = pointSize;
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

void LIVEDEBUGGING(char *pMessage1, char *pMessage2) {
#if BOF_MAC
	if (gLiveDebugging == true) {
		MacMessageBox(pMessage1, pMessage2);
	}
#endif
	return;
}

} // namespace Bagel
