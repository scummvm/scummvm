/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 * $Header$
 *
 */

#if defined(_MSC_VER)

#pragma warning (disable: 4244)
#pragma warning (disable: 4101)

#define scumm_stricmp stricmp

#if defined(CHECK_HEAP)
#undef CHECK_HEAP
#define CHECK_HEAP checkHeap();
#else
#define CHECK_HEAP
#endif

#define SCUMM_LITTLE_ENDIAN

#define FORCEINLINE __forceinline
#define NORETURN _declspec(noreturn)

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#define START_PACK_STRUCTS pack (push,1)
#define END_PACK_STRUCTS   pack(pop)
#define GCC_PACK

#elif defined(__CYGWIN__)

#define scumm_stricmp stricmp
#define CHECK_HEAP
#define SCUMM_LITTLE_ENDIAN

#define FORCEINLINE inline
#define NORETURN __attribute__((__noreturn__))
#define GCC_PACK __attribute__((packed))
#define CDECL

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#define START_PACK_STRUCTS pack (push,1)
#define END_PACK_STRUCTS   pack(pop)

#elif (defined(UNIX) || defined(__APPLE__))

#define scumm_stricmp strcasecmp

#define CHECK_HEAP

/* need this for the SDL_BYTEORDER define */
#include <SDL_byteorder.h>

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define SCUMM_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#define SCUMM_BIG_ENDIAN
#define SCUMM_NEED_ALIGNMENT
#else
#error Neither SDL_BIG_ENDIAN nor SDL_LITTLE_ENDIAN is set.
#endif

#define FORCEINLINE inline
#define CDECL 

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#if defined(__GNUC__)
#define START_PACK_STRUCTS
#define END_PACK_STRUCTS
#define GCC_PACK __attribute__((packed))
#define NORETURN __attribute__((__noreturn__)) 
#else
#define START_PACK_STRUCTS pack (1)
#define END_PACK_STRUCTS   pack ()
#define GCC_PACK
#define NORETURN
#endif

#elif defined(__DC__)

#define scumm_stricmp strcasecmp
#define CHECK_HEAP
#define SCUMM_LITTLE_ENDIAN
#define SCUMM_NEED_ALIGNMENT

#define FORCEINLINE inline
#define NORETURN __attribute__((__noreturn__))
#define GCC_PACK __attribute__((packed))
#define CDECL

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#define START_PACK_STRUCTS pack (push,1)
#define END_PACK_STRUCTS   pack(pop)

#else
#error No system type defined
#endif


#if defined(SCUMM_LITTLE_ENDIAN)

//#if defined(SCUMM_NEED_ALIGNMENT)
//#error Little endian processors that need alignment is not implemented
//#endif

#define MKID(a) ((((a)>>24)&0xFF) | (((a)>>8)&0xFF00) | (((a)<<8)&0xFF0000) | (((a)<<24)&0xFF000000))

#if defined(SCUMM_NEED_ALIGNMENT)
	uint FORCEINLINE READ_LE_UINT16(void *ptr) {
		return (((byte*)ptr)[1]<<8)|((byte*)ptr)[0];
	}
#else
	uint FORCEINLINE READ_LE_UINT16(void *ptr) {
		return *(uint16*)(ptr);
	}
#endif

uint FORCEINLINE READ_BE_UINT16(void *ptr) {
	return (((byte*)ptr)[0]<<8)|((byte*)ptr)[1];
}

#if defined(SCUMM_NEED_ALIGNMENT)
	uint32 FORCEINLINE READ_LE_UINT32(void *ptr) {
		byte *b = (byte*)ptr;
		return (b[3]<<24)+(b[2]<<16)+(b[1]<<8)+(b[0]);
	}
#else
	uint32 FORCEINLINE READ_LE_UINT32(void *ptr) {
		return *(uint32*)(ptr);
	}
#endif

uint32 FORCEINLINE READ_BE_UINT32(void *ptr) {
	byte *b = (byte*)ptr;
	return (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
}

#define READ_BE_UINT32_UNALIGNED READ_BE_UINT32
#define READ_BE_UINT16_UNALIGNED READ_BE_UINT16

#define READ_UINT32_UNALIGNED(a) READ_LE_UINT32(a)

#define FROM_LE_32(__a__) __a__
#define FROM_LE_16(__a__) __a__

#define TO_LE_32(__a__) __a__
#define TO_LE_16(__a__) __a__

#define TO_BE_32(a) ((((a)>>24)&0xFF) | (((a)>>8)&0xFF00) | (((a)<<8)&0xFF0000) | (((a)<<24)&0xFF000000))

#elif defined(SCUMM_BIG_ENDIAN)

#define MKID(a) (a)

uint32 FORCEINLINE FROM_LE_32(uint32 a) {
	return ((a>>24)&0xFF) + ((a>>8)&0xFF00) + ((a<<8)&0xFF0000) + ((a<<24)&0xFF000000);
}

uint16 FORCEINLINE FROM_LE_16(uint16 a) {
	return ((a>>8)&0xFF) + ((a<<8)&0xFF00);
}

#define TO_LE_32 FROM_LE_32
#define TO_LE_16 FROM_LE_16

uint32 FORCEINLINE READ_LE_UINT32(void *ptr) {
	byte *b = (byte*)ptr;
	return (b[3]<<24)+(b[2]<<16)+(b[1]<<8)+(b[0]);
}

uint32 FORCEINLINE READ_BE_UINT32(void *ptr) {
	return *(uint32*)(ptr);
}

uint FORCEINLINE READ_LE_UINT16(void *ptr) {
	byte *b = (byte*)ptr;
	return (b[1]<<8) + b[0];
}

uint FORCEINLINE READ_BE_UINT16(void *ptr) {
	return *(uint16*)(ptr);
}

uint FORCEINLINE READ_BE_UINT16_UNALIGNED(void *ptr) {
	return (((byte*)ptr)[0]<<8)|((byte*)ptr)[1];
}

uint32 FORCEINLINE READ_BE_UINT32_UNALIGNED(void *ptr) {
	byte *b = (byte*)ptr;
	return (b[0]<<24)+(b[1]<<16)+(b[2]<<8)+(b[3]);
}

#define READ_UINT32_UNALIGNED READ_BE_UINT32_UNALIGNED

#define TO_BE_32(a) (a)

#else

#error No endianness defined

#endif


#ifdef NEED_STRDUP
char *strdup(const char *s);
#endif
