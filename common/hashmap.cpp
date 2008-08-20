/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// The hash map (associative array) implementation in this file is
// based on code by Andrew Y. Ng, 1996:

/* 
 * Copyright (c) 1998-2003 Massachusetts Institute of Technology. 
 * This code was developed as part of the Haystack research project 
 * (http://haystack.lcs.mit.edu/). Permission is hereby granted, 
 * free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to 
 * the following conditions: 
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
 * OTHER DEALINGS IN THE SOFTWARE. 
 */

#include "common/hashmap.h"

namespace Common {

// const char *:
uint hashit(const char *p) {
	uint hash = 0;
	byte c;
	while ((c = *p++))
		hash = (hash * 31 + c);
	return hash;
}

uint hashit_lower(const char *p) {
	uint hash = 0;
	byte c;
	while ((c = *p++))
		hash = (hash * 31 + tolower(c));
	return hash;
}

// The following table is taken from the GNU ISO C++ Library's hashtable.h file.
static const uint primes[] = {
	53ul,         97ul,         193ul,       389ul,       769ul,
	1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
	49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
	1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
	50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
	1610612741ul, 3221225473ul, 4294967291ul
};

uint nextTableSize(uint x) {
	int i = 0;
	while (x >= primes[i])
		i++;
	return primes[i];
}

#ifdef DEBUG_HASH_COLLISIONS
static double
	g_collisions = 0,
	g_lookups = 0,
	g_collPerLook = 0,
	g_arrsize = 0,
	g_nele = 0;
static int g_max_arrsize = 0, g_max_nele = 0;
static int g_totalHashmaps = 0;

void updateHashCollisionStats(int collisions, int lookups, int arrsize, int nele) {
	g_collisions += collisions;
	g_lookups += lookups;
	if (lookups)
		g_collPerLook += (double)collisions / (double)lookups;
	g_arrsize += arrsize;
	g_nele += nele;
	g_totalHashmaps++;
	
	g_max_arrsize = MAX(g_max_arrsize, arrsize);
	g_max_nele = MAX(g_max_nele, nele);

	fprintf(stdout, "%d hashmaps: colls %.1f; lookups %.1f; ratio %.3f%%; size %f (max: %d); capacity %f (max: %d)\n",
		g_totalHashmaps,
		g_collisions / g_totalHashmaps,
		g_lookups / g_totalHashmaps,
		100 * g_collPerLook / g_totalHashmaps,
		g_nele / g_totalHashmaps, g_max_nele,
		g_arrsize / g_totalHashmaps, g_max_arrsize);
}
#endif

}	// End of namespace Common
