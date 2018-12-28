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
 */

#ifndef GLK_ALAN2_UTIL
#define GLK_ALAN2_UTIL

#include "glk/alan2/acode.h"
#include "glk/alan2/alan2.h"
#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

// TODO: Move these
extern LitElem *litValues;
extern uint32 litCount;	// for LITMAX - defined in parse.cpp
extern ActElem *acts;	// Actor table pointer
extern ObjElem *objs;	// Object table pointer

// Type checks 
inline bool isObj(Aword x) {
	return x >= OBJMIN && x <= OBJMAX;
}

inline bool isAct(Aword x) {
	return x >= ACTMIN && x <= ACTMAX;
}

inline bool isCnt(Aword x) {
	return (x >= CNTMIN && x <= CNTMAX) ||
		(isObj(x) && objs[x - OBJMIN].cont != 0) ||
		(isAct(x) && acts[x - ACTMIN].cont != 0);
}

inline bool isLoc(Aword x) {
	return x >= LOCMIN && x <= LOCMAX;
}

inline bool isNum(Aword x) {
	return x >= LITMIN && x <= LITMAX && litValues[x - LITMIN].type == TYPNUM;
}

inline bool isStr(Aword x) {
	return x >= LITMIN && x <= LITMAX && litValues[x - LITMIN].type == TYPSTR;
}

inline bool isLit(Aword x) {
	return x >= LITMIN && x <= LITMAX;
}

inline bool endOfTable(LimElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

inline bool endOfTable(ScrElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

inline bool endOfTable(ExtElem *addr) {
	Aword *x = (Aword *)addr;
	return *x == EOF;
}

} // End of namespace Alan2
} // End of namespace Glk

#endif
