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

#ifndef ICB_PROTOCOL_H
#define ICB_PROTOCOL_H

#include "engines/icb/common/px_common.h"

namespace ICB {

#pragma pack(push)
#pragma pack(1)

// old sword 2 stuff required for console font printing

#define NAME_LEN 34

typedef struct {
	uint32 compSize; // compressed size of frame - NB. compression type is now in Anim Header
	uint16 width;    // dimensions of frame
	uint16 height;
} _frameHeader;

typedef struct {
	uint8 fileType;    // byte to define file type (see below)
	uint8 compType;    // type of file compression used ie. on whole file (see below)
	uint32 compSize;   // length of compressed file (ie. length on disk)
	uint32 decompSize; // length of decompressed file held in memory (NB. frames still held compressed)
	uint8 name[NAME_LEN]; // name of object
} _standardHeader;

typedef struct {
	int16 x;            // sprite x-coord OR offset to add to mega's feet x-coord to calc sprite y-coord
	int16 y;            // sprite y-coord OR offset to add to mega's feet y-coord to calc sprite y-coord
	uint32 frameOffset; // points to start of frame header (from start of file header)
	uint8 frameType;    // 0=print sprite normally with top-left corner at (x,y), otherwise see below...
} _cdtEntry;

typedef struct {
	uint8 runTimeComp;   // type of runtime compression used for the frame data (see below)
	uint16 noAnimFrames; // number of frames in the anim (ie. no. of CDT entries)
	uint16 feetStartX;   // start coords for mega to walk to, before running anim
	uint16 feetStartY;
	uint8 feetStartDir; // direction to start in before running anim
	uint16 feetEndX;    // end coords for mega to stand at after running anim (vital if anim starts from an off-screen position, or ends in a different place from the start)
	uint16 feetEndY;
	uint8 feetEndDir; // direction to start in after running anim
	uint16 blend;
} _animHeader;

_frameHeader *FetchFrameHeader(uint8 *animFile, uint16 frameNo); // (25sep96JEL)

} // End of namespace ICB

#endif
