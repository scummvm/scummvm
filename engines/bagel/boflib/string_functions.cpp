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

#include "bagel/boflib/string_functions.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

VOID StrWordCaps(CHAR *pszText) {
	CHAR *p;

	// Can't access a nullptr pointer
	Assert(pszText != nullptr);

	// Make entire string lower case
	strlwr(pszText);

	// Make 1st letter upper case
	*pszText = (CHAR)toupper(*pszText);

	// Make 1st letter after every space be upper-case
	while ((p = strchr(p, ' ')) != nullptr) {
		++p;
		*p = (CHAR)toupper(*p);
	}
}

INT StrFreqMatch(const CHAR *mask, const CHAR *test) {
	static INT nCount[256];
	INT i, divergence;

	// Can't access nullptr pointers
	Assert(mask != nullptr);
	Assert(test != nullptr);

	// Assume no match by making the divergence really high
	divergence = 100;

	// The first letter has to match before we even think about continuing
	if (*mask == *test) {
		// Reset the frequency count
		BofMemSet(nCount, 0, sizeof(INT) * 256);

		// Count the frequency of the chars in 'mask'
		while (*mask != '\0')
			nCount[*mask++] += 1;

		// Subtract off the frequency of the chars in 'test'
		while (*test != '\0')
			nCount[*test++] -= 1;

		// Total all of the frequencies
		divergence = 0;
		for (i = 0; i <= 255; i++)
			divergence += ABS(nCount[i]);
	}

	// Returning a 0 or 1 means that the strings are almost identical
	return divergence;
}

BOOL StrCompare(const CHAR *pszStr1, const CHAR *pszStr2) {
	CHAR *s1, *p, string1[256];		// replace this stack hog with malloc
	CHAR *s2, *sp, string2[256];	// replace this stack hog with malloc
	INT i, n, inc;
	BOOL bMatch;

	// Can't access nullptr pointers
	Assert(pszStr1 != nullptr);
	Assert(pszStr2 != nullptr);

	// Make sure that we are not going to blow the stack
	Assert(strlen(pszStr1) < 256);
	Assert(strlen(pszStr2) < 256);

	p = s1 = string1;
	s2 = string2;

	// Reset these buffers to garuntee the strings will terminate
	BofMemSet(s1, 0, 256);
	BofMemSet(s2, 0, 256);

	// Make local copies of the strings
	Common::sprintf_s(s1, 256, " %s", pszStr1);
	Common::sprintf_s(s2, 256, " %s", pszStr2);

	// Make "THE", "AN", and "A" be special tokens - make them uppercase while
	// all other words are lowercase.
	strlwr(s1);
	StrUprStr(s1, " the ");
	StrUprStr(s1, " an ");
	StrUprStr(s1, " a ");

	if ((sp = strstr(s2, " THE ")) != nullptr) {
		memmove(sp, sp + 5, strlen(sp + 5) + 1);
	}

	// Strip out all unwanted characters
	// Strip out all spaces
	StrStripChar(s1, ' ');
	StrStripChar(s2, ' ');

	// Strip out any apostrophes
	StrStripChar(s1, 39);
	StrStripChar(s2, 39);

	// Strip out any commas
	StrStripChar(s1, ',');
	StrStripChar(s2, ',');

	// Strip out any dashes
	StrStripChar(s1, '-');
	StrStripChar(s2, '-');

	// Add a buffer zone to the end of both strings (this might now be obsolete)
	Common::strcat_s(s1, 256, "     ");
	Common::strcat_s(s2, 256, "     ");

	// Make sure that we are not going to blow the stack */
	Assert(strlen(s1) < 256);
	Assert(strlen(s2) < 256);

	// Compare the 2 strings, if we get a "THE", "A", or "AN" (case sensative)
	// then do a special compare	
	bMatch = TRUE;	

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
			bMatch = FALSE;
			break;
		}
		s1++;
		s2++;
		p++;
	}

	return (bMatch);
}

INT StrCharCount(const CHAR *str, CHAR c) {
	INT n;

	Assert(str != nullptr);

	n = 0;
	while (*str != '\0') {
		if (*str++ == c)
			n++;
	}

	return n;
}

CHAR *StriStr(const CHAR *s1, const CHAR *s2) {
	CHAR *p, *pszStr1, *pszStr2;
	CHAR szLocalBuff1[512];
	CHAR szLocalBuff2[512];

	// Can't access nullptr pointers
	Assert(s1 != nullptr);
	Assert(s2 != nullptr);

	// Assume string not found
	p = nullptr;

	// Allocate buffers big enough to hold copies of these strings
	// Don't bother allocating unless we don't have big enough buffers.
	INT s1Len = strlen(s1);
	INT s2Len = strlen(s2);

	if (s1Len < 512) {
		pszStr1 = szLocalBuff1;
	} else {
		pszStr1 = (CHAR *)BofAlloc(s1Len + 1);
	}

	if (s2Len < 512) {
		pszStr2 = szLocalBuff2;
	} else {
		pszStr2 = (CHAR *)BofAlloc(s2Len + 1);
	}

	if (pszStr1 != nullptr) {
		if (pszStr2 != nullptr) {
			// Copy to local buffers
			Common::strcpy_s(pszStr1, 9999, s1);
			Common::strcpy_s(pszStr2, 9999, s2);

			// Convert to upper case
			strupr(pszStr1);
			strupr(pszStr2);

			// Find pszStr2 in pszStr1
			p = strstr(pszStr1, pszStr2);

			// Re-point to original string s1
			if (p != nullptr) {
				p = (CHAR *)s1 + (p - pszStr1);
			}
			if (pszStr2 != szLocalBuff2) {
				BofFree(pszStr2);
			}
		}
		if (pszStr1 != szLocalBuff1) {
			BofFree(pszStr1);
		}
	}

	return p;
}

VOID StrUprStr(CHAR *s1, const CHAR *s2) {
	CHAR *p;
	INT i, n;

	/* can't access nullptr pointers */
	Assert(s1 != nullptr);
	Assert(s2 != nullptr);

	p = s1;
	while ((p = StriStr(p, s2)) != nullptr) {
		n = strlen(s2);
		for (i = 0; i < n; i++) {
			*p = (CHAR)toupper(*p);
			p++;
		}
	}
}

VOID StrLwrStr(CHAR *s1, const CHAR *s2) {
	CHAR *p;
	INT i, n;

	/* can't access nullptr pointers */
	Assert(s1 != nullptr);
	Assert(s2 != nullptr);

	p = s1;
	while ((p = StriStr(p, s2)) != nullptr) {
		n = strlen(s2);
		for (i = 0; i < n; i++) {
			*p = (CHAR)tolower(*p);
			p++;
		}
	}
}

VOID StrCpyStripChar(CHAR *dest, const CHAR *source, CHAR c) {
	// Can't access nullptr pointers
	Assert(dest != nullptr);
	Assert(source != nullptr);

	// Source and destination cannot be the same
	Assert(dest != source);

	do {
		if ((*dest = *source) != c)
			dest++;
	} while (*source++ != '\0');
}

CHAR *StrStripChar(CHAR *str, CHAR c) {
	CHAR *p = str;

	// Can't acces a nullptr pointer
	Assert(str != nullptr);

	// If c was '\0' then this function would do nothing
	Assert(c != '\0');

	while (*p != '\0') {
		if (*p == c) {
			// Move the string left 1 byte (including the terminator)
			memmove(p, p + 1, strlen(p + 1) + 1);
		}

		p++;
	}

	return str;
}

CHAR *StrReplaceChar(CHAR *str, CHAR cOld, CHAR cNew) {
	CHAR *p = str;

	// Can't acces a nullptr pointer
	Assert(str != nullptr);

	// If cOld was '\0' then this function would do nothing
	Assert(cOld != '\0');

	while (*p != '\0') {
		if (*p == cOld) {
			*p = cNew;
		}
		p++;
	}

	return str;
}

CHAR *StrReplaceStr(CHAR *pszBuf, const CHAR *pszTok, const CHAR *pszNewTok) {
	CHAR *p, *pszSearch, *pszEndTok;
	INT nTok, nNewTok, nDiff;

	// Can't access nullptr pointers
	Assert(pszBuf != nullptr);
	Assert(pszTok != nullptr);
	Assert(pszNewTok != nullptr);

	// Search token, and new token can't be the same
	Assert(strcmp(pszTok, pszNewTok) != 0);

	nTok = strlen(pszTok);
	nNewTok = strlen(pszNewTok);
	nDiff = nTok - nNewTok;

	p = pszBuf;
	while ((pszSearch = strstr(p, pszTok)) != nullptr) {
		pszEndTok = pszSearch + nTok;

		if (nDiff != 0) {
			memmove(pszEndTok - nDiff, pszEndTok, strlen(pszEndTok) + 1);
		}
		BofMemCopy(pszSearch, pszNewTok, nNewTok);
	}

	return pszBuf;
}

VOID MemReplaceChar(UBYTE *pBuf, UBYTE chOld, UBYTE chNew, LONG lSize) {
	Assert(pBuf != nullptr);
	Assert(chOld != chNew);

	while (lSize-- > 0) {

		if (*pBuf == chOld) {
			*pBuf = chNew;
		}
		pBuf++;
	}
}

VOID StrInvertCase(CHAR *pszBuf) {
	// Can't acces a nullptr pointer
	Assert(pszBuf != nullptr);

	while (*pszBuf != '\0') {
		// If character is lower-case, then make it upper case
		if (Common::isLower(*pszBuf)) {
			*pszBuf = (CHAR)toupper(*pszBuf);

		} else {
			// Ootherwise character is upper-case, make it lower case
			*pszBuf = (CHAR)tolower(*pszBuf);
		}
		pszBuf++;
	}
}

CHAR *StrCToPascal(CHAR *pszBuffer) {
	Assert(pszBuffer != nullptr);

	INT n;

	n = strlen(pszBuffer);
	Assert(n < 256);

	memmove(pszBuffer + 1, pszBuffer, n);
	*pszBuffer = (CHAR)n;

	return pszBuffer;
}

CHAR *StrPascalToC(CHAR *pszBuffer) {
	Assert(pszBuffer != nullptr);

	INT n;

	n = *pszBuffer;

	Assert(n < 256);

	memmove(pszBuffer, pszBuffer + 1, n);

	// null-terminate it
	*(pszBuffer + n) = '\0';

	return pszBuffer;
}

} // namespace Bagel
