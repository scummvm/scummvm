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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/mono.h"

namespace Saga2 {

bool debugOn = FALSE;

void debugf(char *format, ...) {
	if (debugOn) {
		char scratchBuffer[512];

		va_list val;

		va_start(val, format);
		vsprintf(scratchBuffer, format, val);
		va_end(val);

		monoprint(scratchBuffer);
	}
}

void monoscroll(int lines) {
	unsigned int col;
	short *pd;
	short *ps;

	for (col = lines, pd = (short *)0xB0000, ps = (short *)(160 * lines + 0xB0000);
	        col < 24;
	        col++) {
		int row;

		for (row = 0; row < 80; row++) {
			*(pd++) = *(ps++);
		}
	}
}

void monocopy(int lines, char *p, int length) {
	int column = 24 - lines;
	if (column < 0) column = 0;
	if (length > 80 * 24) length = 80 * 24;

	int length2 = (80 - length) % 80;

	char *pDest = column * 160 + (char *)0xB0000;

	while (length > 0) {
		*(pDest++) = *(p++);
		*(pDest++) = 0x40;
		length--;
	}
	while (length2 > 0) {
		*(pDest++) = ' ';
		*(pDest++) = 0x40;
		length2--;
	}
}

void monoprint(char *p) {
	char *p2;
	while (*p != 0) {
		p2 = p;
		while (*p2 != 0 && *p2 != '\n') {
			p2++;
		}
		bool foundCR = *p2 == '\n';
		if (foundCR) *p2 = 0;

		int length = strlen(p);

		int lines = 1 + length / 80;

		monoscroll(lines);

		monocopy(lines, p, length);

		if (foundCR) {
			*p2 = '\n';
			p = p2 + 1;
		} else {
			p = p2;
		}
	}
}


#if 0

#ifndef __MYASSERT
#define __MYASSERT



/*
 *  myassert.h
 *
 *  Copyright by The Dreamers Guild, Inc. 1995.  All rights reserved.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif
#undef assert



#ifdef NDEBUG
#define assert(__ignore) ((void)0)
#else
extern void __assert(int, char *, char *, int);
extern void __myassert(long, char *, char *, int);
#define assert(expr) __myassert((long)(expr),#expr,__FILE__,__LINE__);
#endif
#ifdef __cplusplus
};
#endif

#endif




void __myassert(long expr, char *pError, char *pFile, int line) {
	static bool inAssert = FALSE;

	if (inAssert) return;

	inAssert = TRUE;

	if (!expr) {
		debug("Assert failed: %s, file = %s, line = %d", pError, pFile, line);
		if (connection != connectionNone) {
			error("Assert failed: %s, file = %s, line = %d",
			      pError, pFile, line);
		} else {
#undef __assert
			__assert(expr, pError, pFile, line);
		}
	}

	inAssert = FALSE;
}


#endif

#if 0   // __WATCOMC__

void debugf(char *, ...) {
}

void monoprint(char *) {
}

#endif

} // end if namespace Saga2
