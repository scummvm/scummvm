/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_RCUTYPES_H
#define ICB_RCUTYPES_H

#include "engines/icb/common/px_rccommon.h" // This should be included in every file before anything else

#include "common/scummsys.h"

namespace ICB {

typedef const char *cstr;

// Definition of a boolean value that can be used across the PC and PSX.  I stopped true being 0xff because
// C++ weak typing allows you to assign a bool8 to an int8 without warning, whereupon '==' fails for TRUE8 because
// one is signed and one isn't.
typedef unsigned char bool8;
#define TRUE8 ((unsigned char)0x01)
#define FALSE8 ((unsigned char)0x00)

// end of file

} // End of namespace ICB

#endif //_px_rcUTYPES_H
