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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/system.h"
#include "common/debug.h"
#include "common/textconsole.h"

#ifndef LAB_STDDEFINES_H
#define LAB_STDDEFINES_H

namespace Lab {

#define IS_MACOSX 1
#define USE_SWAP 1


#define INCL(BITSET,BIT) ((BITSET) |= (BIT))

#define EXCL(BITSET,BIT) ((BITSET) &= (~(BIT)))



#define SETBIT(BITSET,BITNUM)   INCL(BITSET, (1 << (BITNUM)))

#define UNSETBIT(BITSET,BITNUM) EXCL(BITSET, (1 << (BITNUM)))

#define INBIT(BITSET,BITNUM)    ( ((1 << (BITNUM)) & (BITSET)) > 0 )

#if !defined(WIN32)
#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif
#endif

#if defined(IS_MACOSX)
#define getTime Lab_GetTime
#define delay Lab_Delay
#endif

#if !defined(DOSCODE)
#if defined(USE_NOSWAP)
#define swapUShort(value) (value)
#define swapUShortPtr(ptr,count) (ptr)
#define swapShort(value) (value)
#define swapShortPtr(ptr,count) (ptr)
#define swapUInt(value) (value)
#define swapUIntPtr(ptr,count) (ptr)
#define swapInt(value) (value)
#define swapIntPtr(ptr,count) (ptr)
#define swapULong(value) (value)
#define swapULongPtr(ptr,count) (ptr)
#define swapLong(value) (value)
#define swapLongPtr(ptr,count) (ptr)
#elif defined(USE_SWAP)
uint16 swapUShort(uint16 value);
uint16 *swapUShortPtr(uint16 *ptr, int count);
int16 swapShort(int16 value);
int16 *swapShortPtr(int16 *ptr, int count);
unsigned int swapUInt(unsigned int value);
unsigned int *swapUIntPtr(unsigned int *ptr, int count);
int swapInt(int value);
int *swapIntPtr(int *ptr, int count);
uint32 swapULong(uint32 value);
uint32 *swapULongPtr(uint32 *ptr, int count);
int32 swapLong(int32 value);
int32 *swapLongPtr(int32 *ptr, int count);
#else
#error Please tell me about swapping bytes!
#endif

#endif

} // End of namespace Lab

#endif /* LAB_STDDEFINES_H */
