/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef CINE_PART_H_
#define CINE_PART_H_

namespace Cine {
#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct PartBuffer {
	char partName[14];
	uint32 offset;
	uint32 packedSize;
	uint32 unpackedSize;
} GCC_PACK;

struct AnimData {
	uint16 width;
	uint16 var1;
	uint16 bpp;
	uint16 height;

	byte *ptr1;
	byte *ptr2;
	int16 fileIdx;
	int16 frameIdx;
	char name[10];
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

#define NUM_MAX_PARTDATA 255
#define NUM_MAX_ANIMDATA 255

extern AnimData *animDataTable;
extern PartBuffer *partBuffer;

void loadPart(const char *partName);
void freePartRange(byte startIdx, byte numIdx);
void closePart(void);

int16 findFileInBundle(const char *fileName);

void readFromPart(int16 idx, byte *dataPtr);

byte *readBundleFile(int16 foundFileIdx);
byte *readFile(const char *filename);

} // End of namespace Cine

#endif
