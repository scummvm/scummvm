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

#include "common/str.h"

#include "bagel/boflib/string_functions.h"

namespace Bagel {

int strCharCount(const char *str, char c) {
	assert(str != nullptr);

	int n = 0;
	while (*str != '\0') {
		if (*str++ == c)
			n++;
	}

	return n;
}

char *strreplaceChar(char *str, char cOld, char cNew) {
	char *p = str;

	// Can't access a nullptr pointer
	assert(str != nullptr);

	// If cOld was '\0' then this function would do nothing
	assert(cOld != '\0');

	while (*p != '\0') {
		if (*p == cOld) {
			*p = cNew;
		}
		p++;
	}

	return str;
}

char *strreplaceStr(char *pszBuf, const char *pszTok, const char *pszNewTok) {
	// Can't access nullptr pointers
	assert(pszBuf != nullptr);
	assert(pszTok != nullptr);
	assert(pszNewTok != nullptr);

	// Search token, and new token can't be the same
	assert(strcmp(pszTok, pszNewTok) != 0);

	int nTok = strlen(pszTok);
	int nNewTok = strlen(pszNewTok);
	int nDiff = nTok - nNewTok;

	char *p = pszBuf;
	for (char *pszSearch = strstr(p, pszTok); pszSearch; pszSearch = strstr(p, pszTok)) {
		char *pszEndTok = pszSearch + nTok;

		if (nDiff != 0) {
			memmove(pszEndTok - nDiff, pszEndTok, strlen(pszEndTok) + 1);
		}
		memcpy(pszSearch, pszNewTok, nNewTok);
	}

	return pszBuf;
}

void memreplaceChar(byte *pBuf, byte chOld, byte chNew, int32 lSize) {
	assert(pBuf != nullptr);
	assert(chOld != chNew);

	while (lSize-- > 0) {

		if (*pBuf == chOld) {
			*pBuf = chNew;
		}
		pBuf++;
	}
}

} // namespace Bagel
