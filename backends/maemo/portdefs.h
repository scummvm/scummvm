/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#ifndef PORTDEFS_H
#define PORTDEFS_H

#define SCUMM_LITTLE_ENDIAN
#define SCUMM_NEED_ALIGNMENT

#undef HAVE_X86

#undef LINUPY

/* Data types */
typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

/* Libs */
#undef USE_VORBIS
#undef USE_TREMOR
#undef USE_FLAC
#undef USE_MAD
#define USE_ZLIB
#undef USE_MPEG2
#undef USE_MT32EMU

/* Whether we should use i386 assembly routines */
#undef USE_NASM

#undef main

#ifndef REAL_MAIN
#define main scummvm_main
#endif

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

void set_doubling(unsigned char enable);

#endif /* PORTDEFS_H */


