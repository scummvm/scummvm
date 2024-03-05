
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

#ifndef BAGEL_BOFLIB_STDINC_H
#define BAGEL_BOFLIB_STDINC_H

#include "common/scummsys.h"
#include "bagel/boflib/boffo.h"

namespace Bagel {

#define MAKEDWORD(a, b) MAKELPARAM(a, b)
#define MAKERGB(R, G, B) ((DWORD)(((DWORD)((BYTE)(R))) | (((DWORD)((BYTE)(G))) << 8) | (((DWORD)((BYTE)(B))) << 16)))

/*
 * Math Function Macros
 */
#define isodd(x) ((x)&1)   /* Returns 1 if number is odd  */
#define iseven(x) !((x)&1) /* Returns 1 if number is even */
#ifndef min
#define min(x, y) (x < y ? x : y) /* returns the min of x and y */
#endif
#ifndef max
#define max(x, y) (x > y ? x : y) /* returns the max of x and y */
#endif

/*
 * normal types
 */
#define VIRTUAL virtual
#define STATIC static

#ifndef CDECL
#define CDECL
#endif

#define INLINE inline
#define ASM __asm
#ifndef PASCAL
#define PASCAL pascal
#endif
#define WCHAR wchar_t
#define CHAR char
typedef unsigned char UCHAR;
typedef byte UBYTE;
#define INT int
typedef int16 SHORT;
typedef uint16 USHORT;
#define WORD USHORT
#define LONG int32
typedef uint32 ULONG;
#define FLOAT float
#define DOUBLE double
#define VOID void
#define CONST const

#define INT16 int16
#define INT32 int32
#define UINT16 USHORT
#define UINT32 ULONG

typedef const char *LPCTSTR;

typedef VOID *(*BOFCALLBACK)(INT, VOID *);

typedef long Fixed;
#define FIXED Fixed

/*
 * extended types
 */
typedef struct bofSIZE {
	INT cx;
	INT cy;
} ST_SIZE;

typedef struct bofPOINT {
	INT x;
	INT y;
} ST_POINT;

typedef struct bofRECT {
	INT left;
	INT top;
	INT right;
	INT bottom;
} ST_RECT;

typedef struct bofVECTOR {
	DOUBLE x;
	DOUBLE y;
	DOUBLE z;
} VECTOR;

struct WINDOWPOS {
	void *hwnd = nullptr;
	void *hwndInsertAfter = nullptr;
	int x = 0;
	int y = 0;
	int cx = 0;
	int cy = 0;
	uint32 flags = 0;
};


typedef byte BYTE;
typedef uint32 UINT;
typedef uint32 DWORD;
typedef bool BOOL;

#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)(w))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)(((UINT)(w) >> 8) & 0xFF))
#endif
#ifndef LOWORD
#define LOWORD(l) ((WORD)(DWORD)(l))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#endif

#define MAKE_WORD(a, b) ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKE_LONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

/* for big-endian platforms (i.e. MAC) */
#define SWAPWORD(x) MAKE_WORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKE_LONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

/*
 * Boolean types
 */
enum {
	YES = 1,
	NO = 0
};
#ifndef TRUE
#define TRUE true
#define FALSE false
#endif
typedef bool BOOLEAN;

#ifndef MAX_FNAME
#define MAX_FNAME 256
#endif

#define MAX_DIRPATH 256

/**
 * Displays assertion failure
 * @param bExpression		Expression to evaluate (fail if false)
 * @param nLine				Source code line number of assertion
 * @param pszSourceFile		Source code file of assertion
 * @param pszTimeStamp		Time and date version of source file
 */
extern VOID BofAssert(BOOL bExpression, INT nLine, const CHAR *pszSourceFile, const CHAR *pszTimeStamp);

#ifdef __TIMESTAMP__
#define Assert(f) BofAssert(f, __LINE__, __FILE__, __TIMESTAMP__)
#else
#define Assert(f) BofAssert(f, __LINE__, __FILE__, NULL)
#endif /* __TIMESTAMP__ */

#define Abort(p) BofAbort(p, __FILE__, __LINE__);

#define BOF_MALLOC(nSize) BofAlloc(nSize)
#define BOF_ALLOC(nSize) BofAlloc(nSize)
#define BOF_CALLOC(nElem, nSize) BofCAlloc(nElem, nSize)
#define BOF_FREE(pvPointer) BofFree(pvPointer)
#define BOF_REALLOC(pvPointer, nSize) BofReAlloc(pvPointer, nSize)

} // namespace Bagel

#endif
