/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 Neil Millstone
 * Copyright (C) 2006 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#ifndef _PORTDEFS_H_
#define _PORTDEFS_H_


typedef unsigned char u8;
typedef signed char s8;

typedef unsigned short u16;
typedef signed short s16;

typedef unsigned int u32;
typedef signed int s32;

//#define double float

#define CT_NO_TRANSPARENCY

#undef assert
#define assert(expr) consolePrintf("Asserted!")
//#define NO_DEBUG_MSGS
#include "ds-fs.h"

//#define debug(fmt, ...) consolePrintf(fmt, ##__VA_ARGS__)
//#define debug(fmt, ...) debug(0, fmt, ##__VA_ARGS__)
#define time(t) DS_time(t)
//#define memcpy(dest, src, size) DS_memcpy(dest, src, size)

time_t DS_time(time_t* t);
time_t DS_time(long* t);
void* DS_memcpy(void* s1, void const* s2, size_t n);

#endif
