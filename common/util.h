/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "scummsys.h"
#include "system.h"

template<typename T> inline T ABS (T x)			{ return (x>=0) ? x : -x; }
template<typename T> inline T MIN (T a, T b)	{ return (a<b) ? a : b; }
template<typename T> inline T MAX (T a, T b)	{ return (a>b) ? a : b; }

/**
 * Template method which swaps the vaulues of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

int RGBMatch(byte *palette, int r, int g, int b);
int Blend(int src, int dst, byte *palette);
void ClearBlendCache();

/**
 * Print a hexdump of the data passed in. The number of bytes per line
 * is customizable.
 * @param data	the data to be dumped
 * @param len	the lenght of that data
 * @param bytes_per_line	number of bytes to print per line (default: 16)
 */
void hexdump(const byte * data, int len, int bytes_per_line = 16);

/**
 * A simple random number generator. Although it is definitely not suitable
 * for cryptographic purposes, it serves our purposes just fine.
 */
class RandomSource {
private:
	uint32 _randSeed;

public:
	RandomSource(uint32 seed = 0xA943DE33);
	void setSeed(uint32 seed);
	
	/**
	 * Generates a random unsigned integer in the interval [0, max].
	 * @param max	the upper bound
	 * @return	a random number in the interval [0, max].
	 */
	uint getRandomNumber(uint max);
	/**
	 * Generates a random unsigned integer in the interval [min, max].
	 * @param min	the lower bound
	 * @param max	the upper bound
	 * @return	a random number in the interval [min, max].
	 */
	uint getRandomNumberRng(uint min, uint max);
};

/**
 * Auxillary class to (un)lock a mutex on the stack.
 */
class StackLock {
	OSystem::MutexRef _mutex;
	OSystem *_syst;
	void lock();
	void unlock();
public:
	StackLock(OSystem::MutexRef mutex, OSystem *syst = 0);
	~StackLock();
};


#endif
