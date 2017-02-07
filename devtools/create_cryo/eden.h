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

#pragma once

typedef unsigned char byte;
typedef short int16;
typedef unsigned short uint16;

struct icon_t {
	int16	sx;
	int16	sy;
	int16	ex;
	int16	ey;
	uint16	cursor_id;          // & 0x8000 - inactive/hidden
	unsigned int	action_id;
	unsigned int	object_id;
};
#define END_ICONS {-1, -1, -1, -1, 0, 0, 0}

struct room_t {
	byte   ff_0;
	byte   exits[4];
	byte   flags;
	uint16  bank;
	uint16  party;
	byte   level;
	byte   video;
	byte   location;
	byte   background;
};
#define END_ROOMS {0xFF, {0xFF, 0xFF, 0xFF, 0xFF}, 0xFF, 0xFFFF, 0xFFFF, 0xFF, 0xFF, 0xFF, 0xFF}
