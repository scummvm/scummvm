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

#ifndef ULTIMA8_MISC_COMMON_TYPES_H
#define ULTIMA8_MISC_COMMON_TYPES_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima8 {


enum DebugLevel {
	// General info debug information
	MM_INFO = 1,

	// Minor warnings not important enough to show to the user normally
	MM_MINOR_WARN = 1
};

// ObjId and ProcId

//! 16-Bit ID of an Object
typedef uint16 ObjId;

//! 16-Bit ID of a Process
typedef uint16 ProcId;

} // End of namespace Ultima8
} // End of namespace Ultima

#endif

