
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

#include "common/scummsys.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

CHAR *StrReplaceChar(CHAR *, CHAR, CHAR);
inline ULONG GetFreePhysMem() { return 999999; }

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

#define BofAlloc(n) BofMemAlloc((n), __FILE__, __LINE__, false)
#define BofCAlloc(n, m) BofMemAlloc((uint32)(n) * (m), __FILE__, __LINE__, true)
#define BofReAlloc(p, n) BofMemReAlloc((p), (n), __FILE__, __LINE__)
#define BofFree(p) BofMemFree((p), __FILE__, __LINE__)

} // namespace Bagel

#endif
