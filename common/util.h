/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "common/sys.h"
#include "common/debug.h"

#if defined(WIN32)
#include <windows.h>
#endif

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

template<typename T> inline T ABS (T x)			{ return (x >= 0) ? x : -x; }
template<typename T> inline T MIN (T a, T b)	{ return (a < b) ? a : b; }
template<typename T> inline T MAX (T a, T b)	{ return (a > b) ? a : b; }
template<typename T> inline T CLIP (T v, T amin, T amax)
		{ if (v < amin) return amin; else if (v > amax) return amax; else return v; }

/**
 * Template method which swaps the vaulues of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

#if defined(ARRAYSIZE)
// VS2005beta2 introduces new stuff in winnt.h
#undef ARRAYSIZE
#endif
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

#ifndef round
#define round(x) ((x > 0.0) ? floor((x) + 0.5) : ceil((x) - 0.5))
#endif

namespace Common {

/**
 * Print a hexdump of the data passed in. The number of bytes per line is
 * customizable.
 * @param data	the data to be dumped
 * @param len	the lenght of that data
 * @param bytesPerLine	number of bytes to print per line (default: 16)
 */
extern void hexdump(const byte *data, int len, int bytesPerLine = 16);

/**
 * Simple random number generator. Although it is definitely not suitable for
 * cryptographic purposes, it serves our purposes just fine.
 */
class RandomSource {
private:
	uint32 _randSeed;

public:
	RandomSource();
	void setSeed(uint32 seed);

	uint32 getSeed() {
		return _randSeed;
	}

	/**
	 * Generates a random unsigned integer in the interval [0, max].
	 * @param max	the upper bound
	 * @return	a random number in the interval [0, max]
	 */
	uint getRandomNumber(uint max);
	/**
	 * Generates a random bit, i.e. either 0 or 1.
	 * Identical to getRandomNumber(1), but faster, hopefully.
	 * @return	a random bit, either 0 or 1
	 */
	uint getRandomBit(void);
	/**
	 * Generates a random unsigned integer in the interval [min, max].
	 * @param min	the lower bound
	 * @param max	the upper bound
	 * @return	a random number in the interval [min, max]
	 */
	uint getRandomNumberRng(uint min, uint max);
};

Common::String tag2string(uint32 tag);

}	// End of namespace Common

#endif
