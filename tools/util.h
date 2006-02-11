/* Scumm Tools
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef UTIL_H
#define UTIL_H

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#ifdef WIN32
#include <io.h>
#include <process.h>
#endif


/*
 * Some useful types
 */

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

#if !defined(__cplusplus)
typedef enum { false = 0, true = 1 } bool;

/* If your C compiler doesn't support 'inline', please add a check for it. */
#if defined(_MSC_VER)
#define inline __inline
#endif

#endif


/*
 * Various utility macros
 */

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

static inline uint32 SWAP_32(uint32 a) {
	return ((a >> 24) & 0xFF) | ((a >> 8) & 0xFF00) | ((a << 8) & 0xFF0000) |
		((a << 24) & 0xFF000000);
}

static inline uint16 SWAP_16(uint16 a) {
	return ((a >> 8) & 0xFF) | ((a << 8) & 0xFF00);
}

#if defined(SCUMM_BIG_ENDIAN)
#define TO_BE_32(a) (a)
#define TO_BE_16(a) (a)
#define TO_LE_32(a) SWAP_32(a)
#define TO_LE_16(a) SWAP_16(a)
#else
#define TO_BE_32(a) SWAP_32(a)
#define TO_BE_16(a) SWAP_16(a)
#define TO_LE_32(a) (a)
#define TO_LE_16(a) (a)
#endif

#define MKID(a) (((a&0xff) << 8) | ((a >> 8)&0xff))

#if defined(__GNUC__)
#define NORETURN_PRE
#define NORETURN_POST	__attribute__((__noreturn__))
#elif defined(_MSC_VER)
#define NORETURN_PRE	_declspec(noreturn)
#define NORETURN_POST
#else
#define NORETURN_PRE
#define NORETURN_POST
#endif


#if defined(__cplusplus)
extern "C" {
#endif

/* File I/O */
uint8 readByte(FILE *fp);
uint16 readUint16BE(FILE *fp);
uint16 readUint16LE(FILE *fp);
uint32 readUint32BE(FILE *fp);
uint32 readUint32LE(FILE *fp);
void writeByte(FILE *fp, uint8 b);
void writeUint16BE(FILE *fp, uint16 value);
void writeUint16LE(FILE *fp, uint16 value);
void writeUint32BE(FILE *fp, uint32 value);
void writeUint32LE(FILE *fp, uint32 value);
uint32 fileSize(FILE *fp);

/* Misc stuff */
void NORETURN_PRE error(const char *s, ...) NORETURN_POST;
void warning(const char *s, ...);

#if defined(__cplusplus)
}
#endif

#endif
