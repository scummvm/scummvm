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
	uint32 varA;		/* unused */
	uint32 offset;
	uint32 packed_size;
	uint32 unpacked_size;
	uint32 var1A;		/* unused */
} partBufferStruct;

typedef struct {
	uint16 width;
	uint16 var1;
	uint16 field_4;
	uint16 var2;

	uint8 *ptr1;
	uint8 *ptr2;
	int16 fileIdx;
	int16 frameIdx;
	char name[10];
} animDataStruct;

#define NUM_MAX_PARTDATA 255
#define NUM_MAX_ANIMDATA 255

extern animDataStruct animDataTable[NUM_MAX_ANIMDATA];
extern partBufferStruct *partBuffer;

void loadPart(const char *partName);
void freePartRange(uint8 startIdx, uint8 numIdx);
void closePart(void);

int16 findFileInBundle(const char *fileName);

void readFromPart(int16 idx, uint8 *dataPtr);

uint8 *readBundleFile(int16 foundFileIdx);

#endif
