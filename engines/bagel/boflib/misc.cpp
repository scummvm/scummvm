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

#include "common/textconsole.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

CHAR *StrReplaceChar(CHAR *str, CHAR cOld, CHAR cNew) {
	CHAR *p = str;

	/* can't acces a null pointer */
	Assert(str != NULL);

	/* if cOld was '\0' then this function would do nothing */
	Assert(cOld != '\0');

	while (*p != '\0') {
		if (*p == cOld) {
			*p = cNew;
		}
		p++;
	}
	return (str);
}

void *BofMemAlloc(uint32 lSize, const char *pszFile, int nLine, bool bClear) {
	error("TODO: BofMemAlloc");
}

void *BofMemReAlloc(void *pOldPtr, uint32 lNewSize, const char *pszFile, int nLine) {
	error("TODO: BofMemReAlloc");
}

void BofMemFree(void *pBuf, const char *pszFile, int nLine) {
	error("TODO: BofMemFree");
}

} // namespace Bagel
