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
 * Change Log:
 * $Log$
 * Revision 1.8  2001/11/06 10:34:48  cmatsuoka
 * Added missing missing/sys files.
 *
 * Revision 1.7  2001/11/05 19:21:49  strigeus
 * bug fixes,
 * speech in dott
 *
 * Revision 1.6  2001/11/03 06:33:29  cmatsuoka
 * Protecting VC++-specific pragmas with ifdef _MSC_VER to allow
 * a clean Cygwin build.
 *
 * Revision 1.5  2001/10/23 19:51:50  strigeus
 * recompile not needed when switching games
 * debugger skeleton implemented
 *
 * Revision 1.4  2001/10/16 10:01:47  strigeus
 * preliminary DOTT support
 *
 * Revision 1.3  2001/10/09 18:35:02  strigeus
 * fixed object parent bug
 * fixed some signed/unsigned comparisons
 *
 * Revision 1.2  2001/10/09 17:38:20  strigeus
 * Autodetection of endianness.
 *
 * Revision 1.1.1.1  2001/10/09 14:30:14  strigeus
 * initial revision
 *
 *
 */

#if defined(WIN32)

/* Pragmas are VC++-specific */
#if defined(_MSC_VER)
#pragma warning (disable: 4244)
#pragma warning (disable: 4101)
#endif

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

#elif defined(UNIX)

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
#define NORETURN 
#define CDECL 

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

#define START_PACK_STRUCTS pack (1)
#define END_PACK_STRUCTS   pack ()

#else
#error No system type defined
#endif


#if defined(SCUMM_LITTLE_ENDIAN)

//#if defined(SCUMM_NEED_ALIGNMENT)
//#error Little endian processors that need alignment is not implemented
//#endif

#define MKID(a) ((((a)>>24)&0xFF) | (((a)>>8)&0xFF00) | (((a)<<8)&0xFF0000) | (((a)<<24)&0xFF000000))

#if defined(SCUMM_NEED_ALIGNMENT)
	int FORCEINLINE READ_LE_UINT16(void *ptr) {
		return (((byte*)ptr)[1]<<8)|((byte*)ptr)[0];
	}
#else
	int FORCEINLINE READ_LE_UINT16(void *ptr) {
		return *(uint16*)(ptr);
	}
#endif

int FORCEINLINE READ_BE_UINT16(void *ptr) {
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

int FORCEINLINE READ_LE_UINT16(void *ptr) {
	byte *b = (byte*)ptr;
	return (b[1]<<8) + b[0];
}

int FORCEINLINE READ_BE_UINT16(void *ptr) {
	return *(uint16*)(ptr);
}

int FORCEINLINE READ_BE_UINT16_UNALIGNED(void *ptr) {
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

