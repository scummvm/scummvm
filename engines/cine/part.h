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

typedef struct {
	char part_name[10];
	u32 varA;		/* unused */
	u32 offset;
	u32 packed_size;
	u32 unpacked_size;
	u32 var1A;		/* unused */
} partBufferStruct;

typedef struct {
	u16 width;
	u16 var1;
	u16 field_4;
	u16 var2;

	u8 *ptr1;
	u8 *ptr2;
	s16 fileIdx;
	s16 frameIdx;
	char name[10];
} animDataStruct;

#define NUM_MAX_PARTDATA 255
#define NUM_MAX_ANIMDATA 255

extern animDataStruct animDataTable[NUM_MAX_ANIMDATA];
extern partBufferStruct *partBuffer;

void loadPart(const char *partName);
void freePartRange(u8 startIdx, u8 numIdx);
void closePart(void);

s16 findFileInBundle(const char *fileName);

void readFromPart(s16 idx, u8 *dataPtr);

u8 *readBundleFile(s16 foundFileIdx);

#endif
