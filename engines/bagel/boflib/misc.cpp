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
#include "bagel/boflib/cache.h"
#include "bagel/bagel.h"
#include "bagel/boflib/string_functions.h"

namespace Bagel {

#define MAX_LINE_LEN 100

static uint32 lStart;

void timerStart() {
	lStart = g_system->getMillis();
}

uint32 timerStop() {
	return g_system->getMillis() - lStart;
}

uint32 getTimer() {
	return g_system->getMillis();
}

void bofSleep(uint32 milli) {

	g_system->delayMillis(milli);
}

#define ALLOC_FAIL_RETRIES 2

void *bofMemAlloc(uint32 nSize, const char *pFile, int nLine, bool bClear) {
	// For now, until I fix it, pFile MUST be valid.
	assert(pFile != nullptr);
	assert(nSize != 0);

	// Assume failure
	void *pNewBlock = nullptr;

	// Try a few times to allocate the desired amount of memory.
	// Flush objects from Cache is necessary.
	for (int nRetries = 0; nRetries < ALLOC_FAIL_RETRIES; nRetries++) {
		pNewBlock = (void *)malloc(nSize);

		// If allocation was successful, then we're outta here
		if (pNewBlock != nullptr) {
			if (bClear)
				memset((byte *)pNewBlock, 0, nSize);

			break;
		}
		// Otherwise, we need to free up some memory by flushing old
		// objects from the Cache.
		CCache::optimize(nSize + 2 * sizeof(uint16) + sizeof(uint32));
	}

	if (pNewBlock == nullptr)
		CBofError::fatalError(ERR_MEMORY, "Could not allocate %u bytes, file %s, line %d", nSize, pFile, nLine);

	return pNewBlock;
}

void bofMemFree(void *pBuf) {
	free(pBuf);
}

Fixed fixedMultiply(Fixed Multiplicand, Fixed Multiplier) {
	int64 nTmpNum = (int64)Multiplicand * Multiplier;
	Fixed fixResult = (Fixed)(nTmpNum >> 16);

	return fixResult;
}

Fixed fixedDivide(Fixed Dividend, Fixed Divisor) {
	int64 nBigNum = (int64)Dividend << 16;
	Fixed fixResult = (Fixed)(nBigNum / Divisor);

	return fixResult;
}

void bofMemSet(void *pSrc, byte chByte, int32 lBytes) {
	assert(pSrc != nullptr);

	byte *pBuf = (byte *)pSrc;

	while (lBytes-- != 0)
		*pBuf++ = chByte;
}

void bofMemCopy(void *pDst, const void *pSrc, int32 lLength) {
	assert(pDst != nullptr);
	assert(pSrc != nullptr);
	assert(lLength >= 0);

	byte *p1 = (byte *)pDst;
	const byte *p2 = (const byte *)pSrc;

	while (lLength-- != 0) {
		*p1++ = *p2++;
	}
}

bool readLine(Common::SeekableReadStream *fp, char *pszBuf) {
	if (fp->eos())
		return false;

	Common::String line = fp->readLine();
	Common::strcpy_s(pszBuf, MAX_LINE_LEN, line.c_str());
	strreplaceChar(pszBuf, '\n', '\0');

	return true;
}

void encrypt(void *pBuf, int32 size, const char *pszPassword) {
	assert(pBuf != nullptr);

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

void encryptPartial(void *pBuf, int32 fullSize, int32 lBytes, const char *pszPassword) {
	assert(pBuf != nullptr);

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

int mapWindowsPointSize(int pointSize) {
	return pointSize;
}

} // namespace Bagel
