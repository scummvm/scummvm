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

#include "engines/icb/p4.h"

namespace ICB {

// version & owner details

// So version string is 18 bytes long :
// Version String =  <8 byte header,5 character version, \0, INT32 time>
#define HEAD_LEN 8
uint8 version_string[HEAD_LEN + 10] = {1, 255, 37, 22, 45, 128, 34, 67};

uint8 unencoded_name[HEAD_LEN + 48] = {9,   11,  73,  18,  120, 209, 211, 237, 'R', 'e', 'v', 'o', 'l', 'u', 't', 'i',
									   'o', 'n', ' ', 'S', 'o', 'f', 't', 'w', 'a', 'r', 'e', ' ', 'L', 't', 'd', 0};

uint8 encoded_name[HEAD_LEN + 48] = {24,  52,  90,  122, 223, 165, 33,  199, 179, 209, 225, 157, 222, 238, 219, 209, 143, 224, 133, 190, 232, 209, 162, 177, 198, 228, 202, 146,
									 180, 232, 214, 65,  65,  65,  116, 104, 116, 114, 107, 104, 32,  49,  64,  35,  123, 125, 61,  45,  41,  40,  163, 36,  49,  123, 125, 10};

} // End of namespace ICB
