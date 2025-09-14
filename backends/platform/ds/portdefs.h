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

#ifndef _PORTDEFS_H_
#define _PORTDEFS_H_

/**
 * Include ndstypes.h for uint16 etc. typedefs
 * These are deprecated so they clash with our typedefs
 * Mask them by renaming them when including the header
 */
#define uint8  __nds_uint8
#define uint16 __nds_uint16
#define uint32 __nds_uint32
#define uint64 __nds_uint64
#define int8  __nds_int8
#define int16 __nds_int16
#define int32 __nds_int32
#define int64 __nds_int64

#include "nds/ndstypes.h"

#undef uint8
#undef uint16
#undef uint32
#undef uint64
#undef int8
#undef int16
#undef int32
#undef int64

// Include required headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <new>
#include <limits>

#include <nds/arm9/sassert.h>
#undef assert
#define assert(e) sassert(e, " ")

#ifndef DISABLE_COMMAND_LINE
#define DISABLE_COMMAND_LINE
#endif

#ifndef STREAM_AUDIO_FROM_DISK
#define STREAM_AUDIO_FROM_DISK
#endif

#endif
