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
#include "bagel/boflib/misc.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/cache.h"
#include "bagel/bagel.h"
#include "bagel/boflib/string_functions.h"

namespace Bagel {

#define MAX_LINE_LEN 100

static uint32 lStart;

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
	// For now, until I fix it, pszFile MUST be valid.
	Assert(pszFile != nullptr);
	Assert(lSize != 0);

	// Assume failure
	void *pNewBlock = nullptr;

	// Try a few times to allocate the desired amount of memory.
	// Flush objects from Cache is necessary.
	for (int nRetries = 0; nRetries < ALLOC_FAIL_RETRIES; nRetries++) {
		pNewBlock = (void *)malloc(lSize);

		// If allocation was successful, then we're outta here
		if (pNewBlock != nullptr) {
			if (bClear)
				BofMemSet((byte *)pNewBlock, 0, lSize);

			break;
		}
		// Otherwise, we need to free up some memory by flushing old
		// objects from the Cache.
		CCache::Optimize(lSize + 2 * sizeof(uint16) + sizeof(uint32));
	}

	if (pNewBlock == nullptr) {
		LogError(BuildString("Could not allocate %ld bytes, file %s, line %d", lSize, pszFile, nLine));
	}

	return pNewBlock;
}

void BofMemFree(void *pBuf, const char *pszFile, int nLine) {
	Assert(pszFile != nullptr);

	free(pBuf);
}

Fixed FixedMultiply(Fixed Multiplicand, Fixed Multiplier) {
	int64 nTmpNum = (int64)Multiplicand * Multiplier;
	Fixed fixResult = (Fixed)(nTmpNum >> 16);

	return fixResult;
}

Fixed FixedDivide(Fixed Dividend, Fixed Divisor) {
	int64 nBigNum = (int64)Dividend << 16;
	Fixed fixResult = (Fixed)(nBigNum / Divisor);

	return fixResult;
}

void BofMemSet(void *pSrc, byte chByte, int32 lBytes) {
	Assert(pSrc != nullptr);

	byte *pBuf = (byte *)pSrc;

	while (lBytes-- != 0)
		*pBuf++ = chByte;
}

void BofMemCopy(void *pDst, const void *pSrc, int32 lLength) {
	Assert(pDst != nullptr);
	Assert(pSrc != nullptr);
	Assert(lLength >= 0);

	byte *p1 = (byte *)pDst;
	const byte *p2 = (const byte *)pSrc;

	while (lLength-- != 0) {
		*p1++ = *p2++;
	}
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
	Assert(pBuf != nullptr);

	const char *pStart = pszPassword;
	if (!pszPassword || strlen(pszPassword) == 0) {
		pStart = "\0\0";
	}

	byte *p = (byte *)pBuf;

	const char *pPW = pStart;
	while (--size >= 0) {
		*p ^= (byte)(0xD2 + size + *pPW);
		p++;
		if (*pPW++ == '\0') {
			pPW = pStart;
		}
	}
}

void EncryptPartial(void *pBuf, int32 fullSize, int32 lBytes, const char *pszPassword) {
	Assert(pBuf != nullptr);

	const char *pStart = pszPassword;
	if (!pszPassword || strlen(pszPassword) == 0) {
		pStart = "\0\0";
	}

	byte *p = (byte *)pBuf;

	const char *pPW = pStart;
	while (--lBytes >= 0) {
		fullSize--;
		*p ^= (byte)(0xD2 + fullSize + *pPW);
		p++;
		if (*pPW++ == '\0') {
			pPW = pStart;
		}
	}
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

} // namespace Bagel
