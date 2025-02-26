
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

#include "bagel/boflib/stdinc.h"

namespace Bagel {

extern int mapWindowsPointSize(int pointSize);

/**
 * Starts a Millisecond timer
 */
extern void timerStart();

/**
 * Stops the timer started by timerStart, returns time elapsed.
 * @return      Number of MilliSeconds elapsed since call to timerStart
 */
extern uint32 timerStop();

/**
 * Returns the current elapsed time in milliseconds
*/
extern uint32 getTimer();

/**
 * Pauses the computer for specified number of MilliSeconds
 * @param milli     Number of milliseconds
 */
extern void bofSleep(uint32 milli);

extern Fixed fixedDivide(Fixed Dividend, Fixed Divisor);
extern Fixed fixedMultiply(Fixed Multiplicand, Fixed Multiplier);

#define intToFixed(i) (Fixed)(((long)(i)) << 16)
#define fixedToInt(f) (int)(((long)(f)) >> 16)

//////////////////////////////////////////////////////////////////////////////
//
// Memory allocation routines (Should be put into their own MEMORY module)
//
//////////////////////////////////////////////////////////////////////////////


/**
 * Allocates a memory block of specified size
 * @param nSize         Number of bytes to allocate
 * @param pFile         Source file name
 * @param nLine         Source file line number
 * @param bClear        true if buffer should be cleared
 * @return              Pointer to new buffer
 */
extern void *bofMemAlloc(uint32 nSize, const char *pFile, int nLine, bool bClear);

/**
 * Frees specified memory block
 * @param pBuf          Buffer to de-allocate
 **/
extern void bofMemFree(void *pBuf);

#define bofAlloc(n) bofMemAlloc((n), __FILE__, __LINE__, false)
#define bofCleanAlloc(n) bofMemAlloc((n), __FILE__, __LINE__, true)
#define bofFree(p) bofMemFree((p))

inline uint32 getFreePhysMem() {
	return 999999;
}

/**
 * Encrypts specified buffer
 * @param pBuf          Buffer to encrypt
 * @param lSize         Number of bytes in buffer
 * @param pszPassword   Optional password to encrypt with
 */
void encrypt(void *pBuf, int32 lSize, const char *pszPassword = nullptr);
#define decrypt encrypt

extern void encryptPartial(void *, int32, int32, const char *pPassword = nullptr);
#define decryptPartial encryptPartial

} // namespace Bagel

#endif
