
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

/**
 * Log an error message
*/
extern void ErrorLog(const char *format, ...);
inline void SetErrLogFile(const char *pszFileName) {}

extern int MapWindowsPointSize(int pointSize);

/**
 * Return a true / false based on the probability given
 * @param nProbability      The probability of returning a true
 * @return                  A true <nProbability> of the tine
 */
extern bool ProbableTrue(int);

/**
 * Starts a Millisecond timer
 */
extern void TimerStart();

/**
 * Stops the timer started by TimerStart, returns time elapsed.
 * @return      Number of MilliSeconds elapsed since call to TimerStart
 */
extern uint32 TimerStop();

/**
 * Returns the current elapsed time in milliseconds
*/
extern uint32 GetTimer();

/**
 * Pauses the computer for specified number of MilliSeconds
 * @param milli     Number of milliseconds
 */
extern void Sleep(uint32 milli);

extern Fixed FixedDivide(Fixed Dividend, Fixed Divisor);
extern Fixed FixedMultiply(Fixed Multiplicand, Fixed Multiplier);

#define IntToFixed(i) (Fixed)(((long)(i)) << 16)
#define FixedToInt(f) (int)(((long)f) >> 16)

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
extern void *BofMemAlloc(uint32 nSize, const char *pFile, int nLine, bool bClear);

/**
 * Re-Allocates a memory block to the specified size
 * @param pOldPointer   Old memory pointer
 * @param nNewSize      New buffer size
 * @param pFile         Source file name
 * @param nLine         Source file line number
 * @return              Pointer to new buffer
 */
extern void *BofMemReAlloc(void *pOldPtr, uint32 nNewSize, const char *pFile, int nLine);

/**
 * Frees specified memory block
 * @param pBuf          Buffer to de-allocate
 * @param pFile         Source file name
 * @param nLine         Source file line number
 **/
extern void BofMemFree(void *pBuf, const char *pszFile, int nLine);

#define BofAlloc(n) BofMemAlloc((n), __FILE__, __LINE__, false)
#define BofCAlloc(n, m) BofMemAlloc((uint32)(n) * (m), __FILE__, __LINE__, true)
#define BofReAlloc(p, n) BofMemReAlloc((p), (n), __FILE__, __LINE__)
#define BofFree(p) BofMemFree((p), __FILE__, __LINE__)

#define BofMemSet memset
#define BofMemCopy memcpy
#define BofMemMove memmove

inline uint32 GetFreePhysMem() {
	return 999999;
}

/**
 * Encrypts specified buffer
 * @param pBuf          Buffer to encrypt
 * @param lSize         Number of bytes in buffer
 * @param pszPassword   Optional password to encrypt with
 */
void Encrypt(void *, int32, const char *pPassword = nullptr);
#define Decrypt Encrypt

extern void EncryptPartial(void *, int32, int32, const char *pPassword = nullptr);
#define DecryptPartial EncryptPartial

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : -(x))
#endif

extern bool IsKeyDown(uint32 lKeyCode);

} // namespace Bagel

#endif
