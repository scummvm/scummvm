/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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

#ifndef GRAPHICS_ANIMATION_H
#define GRAPHICS_ANIMATION_H

// Uncomment this if you are using libmpeg2 0.3.1.
// #define USE_MPEG2_0_3_1

#ifdef _MSC_VER
typedef int8 int8_t;
typedef int16 int16_t;
typedef int32 int32_t;

typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t; 
#else
#include <inttypes.h>
#endif

#ifdef USE_MPEG2
extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

#ifdef USE_MPEG2_0_3_1
typedef int mpeg2_state_t;
typedef sequence_t mpeg2_sequence_t;
#define STATE_BUFFER -1
#endif

#endif




#endif