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

// Include ndstypes.h for uint16 etc. typedefs
#include "nds/ndstypes.h"

typedef unsigned int uint;

// Define SCUMMVM_DONT_DEFINE_TYPES to prevent scummsys.h from trying to
// re-define those data types.
#define SCUMMVM_DONT_DEFINE_TYPES

// Include required headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <new>

#include <nds/arm9/sassert.h>
#undef assert
#define assert(e) sassert(e, " ")

#ifndef DISABLE_COMMAND_LINE
#define DISABLE_COMMAND_LINE
#endif

#ifndef STREAM_AUDIO_FROM_DISK
#define STREAM_AUDIO_FROM_DISK
#endif

// FIXME: Since I can't change the engine at the moment (post lockdown) this define can go here.
// This define changes the mouse-relative motion which doesn't make sense on a touch screen to
// a more conventional form of input where the menus can be clicked on.
#define LURE_CLICKABLE_MENUS

#endif
