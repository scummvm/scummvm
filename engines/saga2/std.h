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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_STD_H
#define SAGA2_STD_H

#include "common/system.h"

typedef uint32      ChunkID;
#define MakeID(a,b,c,d) ((d<<24L)|(c<<16L)|(b<<8L)|a)

#include "saga2/saga2.h"

#define FTA

// #define LEAVE        goto exitit         // bail out of function
#define unless(x)   if((x)==NULL)       // an inverted if statement

#ifndef elementsof
#define elementsof(x) (sizeof(x)/sizeof(x[0]))
#endif

#ifndef offsetof
#define offsetof(type,field) (uint32)&(((type *)0)->field)
#endif

#ifndef FALSE
#define FALSE       0
#endif
#ifndef TRUE
#define TRUE        1
#endif
#define OK                      0
#define EMPTY           -1

typedef bool BOOL;

#define maxuint8 0xff
#define minuint8 0
#define maxint16 0x7fff
#define minint16 0x8000
#define maxuint16 0xffff
#define minuint16 0
#define maxint32 0x7fffffff
#define maxuint32 0xffffffffu
#define minuint32 0u

//  Plaftorm metrics
const int           platformWidth = 8,      // width and height of platform
                    platMask = platformWidth - 1,
                    platShift = 3;


#endif  //SAGA2_STD_H
