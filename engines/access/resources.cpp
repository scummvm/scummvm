/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, 0xwhose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, 0xor (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, 0xwrite to the Free Software
 * Foundation, 0xInc., 0x51 Franklin Street, 0xFifth Floor, 0xBoston, 0xMA 02110-1301, 0xUSA.
 *
 */

#include "access/resources.h"
#include "access/access.h"

namespace Access {

const byte INITIAL_PALETTE[18 * 3] = {
	0x00, 0x00, 0x00, 
	0xff, 0xff, 0xff,
	0xf0, 0xf0, 0xf0,
	0xe0, 0xe0, 0xe0,
	0xd0, 0xd0, 0xd0,
	0xc0, 0xc0, 0xc0,
	0xb0, 0xb0, 0xb0,
	0xa0, 0xa0, 0xa0,
	0x90, 0x90, 0x90,
	0x80, 0x80, 0x80,
	0x70, 0x70, 0x70,
	0x60, 0x60, 0x60,
	0x50, 0x50, 0x50,
	0x40, 0x40, 0x40,
	0x30, 0x30, 0x30,
	0x20, 0x20, 0x20,
	0x10, 0x10, 0x10,
	0x00, 0x00, 0x00
};

} // End of namespace Access
