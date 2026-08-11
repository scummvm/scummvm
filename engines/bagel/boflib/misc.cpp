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

void ErrorLog(const char *logFile, const char *format, ...) {
	// No implementation
}

int StrFreqMatch(const char *mask, const char *test) {
	static int nCount[256];
	int i, divergence;

	/* can't access null pointers */
	assert(mask != nullptr);
	assert(test != nullptr);

	/* assume no match by making the divergence really high */
	divergence = 100;

	/*
	* the first letter has to match before we even think about continuing
	*/
	if (*mask == *test) {

		/* reset the frequency count */
		memset(nCount, 0, sizeof(int) * 256);

		/* count the frequency of the chars in 'mask' */
		while (*mask != '\0')
			nCount[(int)*mask++] += 1;

		/* subtract off the frequency of the chars in 'test' */
		while (*test != '\0')
			nCount[(int)*test++] -= 1;

		/*
		* total all of the frequencies
		*/
		divergence = 0;
		for (i = 0; i <= 255; i++)
			divergence += abs(nCount[i]);
	}

	/* returning a 0 or 1 means that the strings are almost identical */
	return (divergence);
}


bool StrCompare(const char *pszStr1, const char *pszStr2, unsigned int nSize) {
	char *s1, *p, string1[256];   /* replace this stack hog with malloc */
	char *s2, *sp, string2[256];       /* replace this stack hog with malloc */
	int i, n, inc;
	bool bMatch;

	/* can't access null pointers */
	assert(pszStr1 != nullptr);
	assert(pszStr2 != nullptr);

	/* make sure that we are not going to blow the stack */
	assert(strlen(pszStr1) < 256);
	assert(strlen(pszStr2) < 256);

	p = s1 = string1;
	s2 = string2;

	/*
	* Reset these buffers to garuntee the strings will terminate
	*/
	memset(s1, 0, 256);
	memset(s2, 0, 256);

	/* make local copies of the strings */
	Common::sprintf_s(s1, 256, " %s", pszStr1);
	Common::sprintf_s(s2, 256, " %s", pszStr2);

	/*
	* make "THE", "AN", and "A" be special tokens - make them uppercase while
	* all other words are lowercase.
	*/
	strLower(s1);
	StrUprStr(s1, " the ");
	StrUprStr(s1, " an ");
	StrUprStr(s1, " a ");

	if ((sp = strstr(s2, " THE ")) != nullptr) {
		memmove(sp, sp + 5, strlen(sp + 5) + 1);
	}

	/*
	* strip out all unwanted characters
	*/

	/* strip out all spaces
	*/
	StrStripChar(s1, ' ');
	StrStripChar(s2, ' ');

	/* strip out any apostrophes
	*/
	StrStripChar(s1, 39);
	StrStripChar(s2, 39);

	/* strip out any commas
	*/
	StrStripChar(s1, ',');
	StrStripChar(s2, ',');

	/* strip out any dashes
	*/
	StrStripChar(s1, '-');
	StrStripChar(s2, '-');

	/*
	*  add a buffer zone to the end of both strings
	* (this might now be obsolete)
	*/
	Common::strcat_s(s1, 256, "     ");
	Common::strcat_s(s2, 256, "     ");

	/*
	* compare the 2 strings, if we get a "THE", "A", or "AN" (case sensative)
	* then do a special compare
	*/

	/* assume they match */
	bMatch = true;

	i = 0;
	n = strlen(s1);
	while (i++ < n) {

		/*
		* handle special tokens
		*/
		if ((*s1 == 'T') || (*s1 == 'A')) {
			inc = 3;
			if (*s1 == 'A') {
				inc = 1;
				if (*(s1 + 1) == 'N') {
					inc = 2;
				}
			}
			p = s1;

			if (toupper(*s1) != toupper(*s2)) {
				s1 += inc;
				i += inc;
			} else {
				p = s1 + inc;
				i += inc;
			}
		}

		if ((toupper(*s1) != toupper(*s2)) && (toupper(*p) != toupper(*s2))) {
			bMatch = false;
			break;
		}
		s1++;
		s2++;
		p++;
	}

	return (bMatch);
}


/*****************************************************************************
*
*  StrCharCount     - Counts number of occurences of a specified char in String
*
*  DESCRIPTION:     Counts the number of times the specified character occurs
*                   in the given string.
*
*  SAMPLE USAGE:
*  n = StrCharCount(str, c);
*  const char *str;                         pointer to string to parse
*  char c;                                  character to count in str
*
*  RETURNS:  int  = number of times character c occurs in string str
*
*****************************************************************************/
int StrCharCount(const char *str, char c) {
	int n;

	assert(str != nullptr);
	assert(strlen(str) <= 255);

	n = 0;
	while (*str != '\0') {
		if (*str++ == c)
			n++;
	}

	return (n);
}


/**
*  name      StriStr - same as strstr() except ignores case
*
*  synopsis  s = StriStr(s1, s2)
*            const char *s1                 string to parse
*            const char *s2                 substring to find in s1
*
*  purpose   To search for a string inside another string while ignoring case
*
*
*  returns   pointer to substring s2 in s1 or nullptr if not found
*
**/

char *StriStr(char *s1, const char *s2) {
	char *p, str1[80];
	char str2[80];

	/* can't access null pointers */
	assert(s1 != nullptr);
	assert(s2 != nullptr);

	/* make sure we don't blow the stack */
	assert(strlen(s1) < 80);
	assert(strlen(s2) < 80);

	/* copy to local buffers */
	Common::strcpy_s(str1, s1);
	Common::strcpy_s(str2, s2);

	/* convert to upper case */
	strUpper(str1);
	strUpper(str2);

	/* find str2 in str1 */
	p = strstr(str1, str2);

	/* re-point to original string s1 */
	if (p != nullptr) {
		p = s1 + (p - str1);
	}

	return (p);
}

void StrUprStr(char *s1, const char *s2) {
	char *p;
	int i, n;

	/* can't access null pointers */
	assert(s1 != nullptr);
	assert(s2 != nullptr);

	p = s1;
	while ((p = StriStr(p, s2)) != nullptr) {
		n = strlen(s2);
		for (i = 0; i < n; i++) {
			*p = (char)toupper(*p);
			p++;
		}
	}
}


/**
*  name      StrCpyStripChar - Same as Common::strcpy_s() except specified character
*                              will be stripped from the destination string.
*
*  synopsis  StrCpyStripChar(dest, source, c)
*            char *dest                     destinaton string
*            const char *source             source string
*            char c                         character to strip from dest
*
*  purpose   To strip out a specified character while copying 1 string to another
*
*
*  returns   Nothing
*
**/
void StrCpyStripChar(char *dest, const char *source, char c) {
	/* can't access null pointers */
	assert(dest != nullptr);
	assert(source != nullptr);

	/* source and destination cannot be the same */
	assert(dest != source);

	do {
		if ((*dest = *source) != c)
			dest++;
	} while (*source++ != '\0');
}


char *StrStripChar(char *str, char c) {
	char *p = str;

	/* can't acces a null pointer */
	assert(str != nullptr);

	/* if c was '\0' then this function would do nothing */
	assert(c != '\0');

	while (*p != '\0') {
		if (*p == c) {

			/* move the string left 1 byte (including the terminator) */
			memmove(p, p + 1, strlen(p + 1) + 1);
		}
		p++;
	}
	return (str);
}


char *StrReplaceChar(char *str, char cOld, char cNew) {
	char *p = str;

	/* can't acces a null pointer */
	assert(str != nullptr);

	/* if cOld was '\0' then this function would do nothing */
	assert(cOld != '\0');

	while (*p != '\0') {
		if (*p == cOld) {
			*p = cNew;
		}
		p++;
	}
	return (str);
}

/**
*  name      ProbableTrue
*  synopsis  - return a true / false based on the probability given
*            int nProbability          the probability of returning a true
*
*  purpose   to return a true <nProbability> of the tine
*
*
*  returns   bool
*
**/
bool ProbableTrue(int nProbability) {
	int nRand = brand() % 100;

	return (nRand < nProbability);
}

} // namespace Bagel
