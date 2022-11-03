/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_BITMAP_PC_H_INCLUDED
#define ICB_PX_BITMAP_PC_H_INCLUDED

#include "engines/icb/common/px_common.h"

namespace ICB {

// These define the extension for the finding the bitmap files
#ifndef PX_BITMAP_PC_EXT
#define PX_BITMAP_PC_EXT "bitmap_pc"
#endif

// Replaced pxHeader with id and added schema control to datafiles
#define PC_BITMAP_SCHEMA 1
#define PC_BITMAP_ID "PCB"

// Data structure to overlay the sprite data.
typedef struct {
	uint32 x, y;           // X and Y position of sprite.
	uint32 width, height;  // Width and height of the sprite in pixels.
	uint8 data[1]; // Sprite data.

} _pxPCSprite;

typedef struct _pxPCBitmap {
	char id[4];    // "PCB" Pc bitmap
	uint32 schema; // The current schema number

	uint8 palette[4 * 256];         // RGB but padded with 0 to 32-bits.
	uint32 num_sprites;             // Number of sprites in this file.
	uint32 sprite_offsets[1];       // Offsets to sprite data for each sprite.

} _pxPCBitmap;

} // End of namespace ICB

#endif // #ifndef _PX_BITMAP_PC_H_INCLUDED
