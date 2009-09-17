/* Scumm Tools
 * Copyright (C) 2002-2006 The ScummVM project
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
typedef uint8 bool;
#define false 0
#define true 1

/* If your C compiler doesn't support 'inline', please add a check for it. */
#if defined(_MSC_VER)
#define inline __inline
#endif

#endif


/*
 * Various utility macros
 */

#if defined(_MSC_VER)

	#define scumm_stricmp stricmp
	#define scumm_strnicmp _strnicmp
	#define snprintf _snprintf

	#define SCUMM_LITTLE_ENDIAN

	#define START_PACK_STRUCTS pack(push, 1)
	#define END_PACK_STRUCTS   pack(pop)


#elif defined(__MINGW32__)

	#define scumm_stricmp stricmp
	#define scumm_strnicmp strnicmp

	#define SCUMM_LITTLE_ENDIAN

	#define START_PACK_STRUCTS pack(push, 1)
	#define END_PACK_STRUCTS   pack(pop)


	#ifndef _HEAPOK
	#define _HEAPOK	(-2)
	#endif

#elif defined(UNIX)

	#define scumm_stricmp strcasecmp
	#define scumm_strnicmp strncasecmp

	#if defined(__DECCXX) /* Assume alpha architecture */
	#define INVERSE_MKID
	#define SCUMM_NEED_ALIGNMENT
	#endif

	#if !defined(__GNUC__)
	#define START_PACK_STRUCTS pack (1)
	#define END_PACK_STRUCTS   pack ()
	#endif

#else

	#error No system type defined

#endif


/*
 * GCC specific stuff
 */
#if defined(__GNUC__)
        #define GCC_PACK __attribute__((packed))
        #define NORETURN __attribute__((__noreturn__))
        #define GCC_PRINTF(x,y) __attribute__((format(printf, x, y)))
#else
        #define GCC_PACK
        #define GCC_PRINTF(x,y)
#endif

#define READ_UINT16(a) READ_LE_UINT16(a)
#define READ_UINT32(a) READ_LE_UINT32(a)

#define WRITE_UINT16(a, v) WRITE_LE_UINT16(a, v)
#define WRITE_UINT32(a, v) WRITE_LE_UINT32(a, v)

#define FROM_LE_32(a) ((uint32)(a))
#define FROM_LE_16(a) ((uint16)(a))

#define TO_LE_32(a) ((uint32)(a))
#define TO_LE_16(a) ((uint16)(a))

#define TO_BE_32(a) SWAP_BYTES_32(a)
#define TO_BE_16(a) SWAP_BYTES_16(a)

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

static inline uint32 SWAP_32(uint32 a) {
	return ((a >> 24) & 0xFF) | ((a >> 8) & 0xFF00) | ((a << 8) & 0xFF0000) |
		((a << 24) & 0xFF000000);
}

static inline uint16 SWAP_16(uint16 a) {
	return ((a >> 8) & 0xFF) | ((a << 8) & 0xFF00);
}

static inline uint16 READ_LE_UINT16(const void *ptr) {
	const byte *b = (const byte *)ptr;
	return (b[1] << 8) + b[0];
}
static inline uint32 READ_LE_UINT32(const void *ptr) {
	const byte *b = (const byte *)ptr;
	return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + (b[0]);
}
static inline void WRITE_LE_UINT16(void *ptr, uint16 value) {
	byte *b = (byte *)ptr;
	b[0] = (byte)(value >> 0);
	b[1] = (byte)(value >> 8);
}
static inline void WRITE_LE_UINT32(void *ptr, uint32 value) {
	byte *b = (byte *)ptr;
	b[0] = (byte)(value >>  0);
	b[1] = (byte)(value >>  8);
	b[2] = (byte)(value >> 16);
	b[3] = (byte)(value >> 24);
}

static inline uint16 READ_BE_UINT16(const void *ptr) {
	const byte *b = (const byte *)ptr;
	return (b[0] << 8) + b[1];
}
static inline uint32 READ_BE_UINT32(const void *ptr) {
	const byte *b = (const byte*)ptr;
	return (b[0] << 24) + (b[1] << 16) + (b[2] << 8) + (b[3]);
}
static inline void WRITE_BE_UINT16(void *ptr, uint16 value) {
	byte *b = (byte *)ptr;
	b[0] = (byte)(value >> 8);
	b[1] = (byte)(value >> 0);
}
static inline void WRITE_BE_UINT32(void *ptr, uint32 value) {
	byte *b = (byte *)ptr;
	b[0] = (byte)(value >> 24);
	b[1] = (byte)(value >> 16);
	b[2] = (byte)(value >>  8);
	b[3] = (byte)(value >>  0);
}

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
void debug(int level, const char *s, ...);

enum Platform {
	kPlatformPC,
	kPlatformAmiga,
	kPlatformAtariST,
	kPlatformMacintosh,
	kPlatformFMTowns,
	kPlatformWindows,
	kPlatformNES,
	kPlatformC64,
	kPlatformCoCo3,
	kPlatformLinux,
	kPlatformAcorn,
	kPlatformSegaCD,
	kPlatform3DO,
	kPlatformPCEngine,

	kPlatformApple2GS,
	kPlatformPC98,
	kPlatformWii,
	kPlatformPSX,

	kPlatformUnknown = -1
};

enum {
	EN_ANY,     // Generic English (when only one game version exist)
	EN_USA,
	EN_GRB,

	DE_DEU,
	FR_FRA,
	IT_ITA,
	PT_BRA,
	ES_ESP,
	JA_JPN,
	ZH_TWN,
	KO_KOR,
	SE_SWE,
	HB_ISR,
	RU_RUS,
	CZ_CZE,
	NL_NLD,
	NB_NOR,
	PL_POL,

	UNK_LANG = -1	// Use default language (i.e. none specified)
};

#if defined(__cplusplus)
}
#endif

#endif
