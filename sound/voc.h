/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SOUND_VOC_H
#define SOUND_VOC_H

#include "stdafx.h"
#include "common/scummsys.h"

class AudioStream;
class File;

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct VocHeader {
	uint8 desc[20];
	uint16 datablock_offset;
	uint16 version;
	uint16 id;
} GCC_PACK;

struct VocBlockHeader {
	uint8 blocktype;
	uint8 size[3];
	uint8 sr;
	uint8 pack;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif

/**
 * Take a sample rate parameter as it occurs in a VOC sound header, and
 * return the corresponding sample frequency.
 */
extern int getSampleRateFromVOCRate(int vocSR);

extern byte *readVOCFromMemory(byte *ptr, int &size, int &rate, int &loops, int &begin_loop, int &end_loop);
extern byte *loadVOCFile(File *file, int &size, int &rate);

AudioStream *makeVOCStream(byte *ptr);
AudioStream *makeVOCStream(File *file);

#endif
