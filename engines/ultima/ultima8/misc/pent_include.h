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

#ifndef ULTIMA8_MISC_PENT_INCLUDE_H
#define ULTIMA8_MISC_PENT_INCLUDE_H

//
// Common/base types
//
#include "ultima/ultima8/misc/common_types.h"


//
// p_dynamic_cast support
//
#include "ultima/ultima8/misc/p_dynamic_cast.h"


//
// Strings
//
#include "ultima/ultima8/misc/istring.h"


//
// The Debugger
//
#include "ultima/ultima8/misc/debugger.h"


// Two very useful macros that one should use instead of pure delete; they
// will additionally set the old object pointer to 0, thus helping prevent
// double deletes (note that "delete 0" is a no-op).
#define FORGET_OBJECT(x) do { delete x; x = 0; } while(0)

//
// Can't happen.
// For things that really can't happen. Or shouldn't anyway.
//
#define CANT_HAPPEN() do { assert(false); } while(0)

//
// Can't happen with a message
//
// Allows a message to be supplied.
// May not work on all compilers or runtimes as expected
//
#define CANT_HAPPEN_MSG(msg) do { assert(msg && false); } while(0)

#endif
