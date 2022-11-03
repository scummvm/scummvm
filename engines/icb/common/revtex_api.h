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

#ifndef ICB_REVTEX_API_H
#define ICB_REVTEX_API_H

namespace ICB {

#define REVTEX_API_ID "RTX"
#define REVTEX_API_SCHEMA_ICB 1
#define REVTEX_API_SCHEMA_ELDORADO 2

#define MAX_PAL_ENTRIES (256)

typedef struct revtex_API_header {
	char id[4];
	uint32 schema;
} revtex_API_header;

// The level pointers within the RevTexture structure in this structure
// are relative addresses which get converted to absolute pointer by
// calling the Map function
typedef struct revtex_API_v1 {
	char id[4];
	uint32 schema;

	// RevTexture revtex;
	uint32 palette[256];   // windows 32-bit RGB with 1 byte of padding
	uint32 width;          // must be power of 2
	uint32 height;         // must be power of 2
	uint32 levelOffset[9]; // width/1 * height/1 -> width/256 * height/256

} revtex_API_v1;

typedef struct revtex_API_v2 {
	char id[4];
	uint32 schema;
	uint32 transparent;

	// RevTexture revtex;
	uint32 palette[256];   // windows 32-bit RGB with 1 byte of padding
	uint32 width;          // must be power of 2
	uint32 height;         // must be power of 2
	uint32 levelOffset[9]; // width/1 * height/1 -> width/256 * height/256

} revtex_API_v2;

} // End of namespace ICB

#endif // #ifndef REVTEX_API_H
