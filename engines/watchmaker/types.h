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

#ifndef WATCHMAKER_TYPES_H
#define WATCHMAKER_TYPES_H

// Temporary includes to make porting easier
#include "common/array.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/str.h"

namespace Watchmaker {

//************************************************************************************************************************
//	typedefs
//************************************************************************************************************************

typedef unsigned char   uint8;
typedef signed char     int8;
typedef unsigned short  uint16;
typedef signed short    int16;
typedef unsigned int    uint32;
typedef signed int      int32;
typedef float           t3dF32;
typedef double          t3dF64;

} // End of namespace Watchmaker

#endif // WATCHMAKER_TYPES_H
