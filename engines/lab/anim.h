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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_DIFF_H
#define LAB_DIFF_H

#include "common/file.h"

namespace Lab {

class LabEngine;
#define CONTINUOUS      0xFFFF

struct DIFFHeader {
	uint16 _version;    // unused
	uint16 _width;
	uint16 _height;
	char _depth;        // unused
	char _fps;
	uint32 _bufferSize; // unused
	uint16 _machine;    // unused
	uint32 _flags;
};

struct BitMap {
	uint16 _bytesPerRow;
	uint16 _rows;       // unused
	byte _flags;
	byte *_planes[16];
};

#define BITMAPF_NONE  0
#define BITMAPF_VIDEO (1<<7)

class Anim {
private:
	LabEngine *_vm;

	uint32 header;
	uint16 CurBit;
	uint16 numchunks;
	uint32 WaitSec;
	uint32 WaitMicros;
	uint32 DelayMicros;
	bool continuous;
	bool IsPlaying;
	bool IsAnim;
	bool IsPal;
	bool donepal;
	uint16 framenumber;
	bool PlayOnce;
	byte *Buffer;
	byte *storagefordifffile;
	byte **difffile;
	uint32 size;
	bool StopPlayingEnd;
	uint16 samplespeed;
	byte *start;
	uint32 diffwidth;
	uint32 diffheight;
	bool stopsound;
	uint16 _dataBytesPerRow;

	void unDIFFByteByte(byte *dest, byte *diff);
	void unDIFFByteWord(uint16 *dest, uint16 *diff);
	void VUnDIFFByteByte(byte *Dest, byte *diff, uint16 bytesperrow);
	void VUnDIFFByteWord(uint16 *Dest, uint16 *diff, uint16 bytesperrow);
	void VUnDIFFByteLong(uint32 *Dest, uint32 *diff, uint16 bytesperrow);

public:
	Anim(LabEngine *vm);

	DIFFHeader headerdata;
	char diffcmap[256 * 3];
	bool IsBM;          /* Just fill in the RawDIFFBM structure */
	bool waitForEffect; /* Wait for each sound effect to finish before continuing. */
	bool DoBlack;       /* Black the screen before new picture  */
	bool nopalchange;   /* Don't change the palette.            */
	BitMap RawDiffBM;

	void unDiff(byte *NewBuf, byte *OldBuf, byte *DiffData, uint16 bytesperrow, bool IsV);
	bool unDIFFMemory(byte *dest,           /* Where to Un-DIFF */
                  byte *diff,           /* The DIFFed code. */
                  uint16 headerSize,    /* Size of header (1, 2 or 4 bytes) (only supports 1 currently */
                  uint16 copySize);     /* Size of minimum copy or skip. (1, 2 or 4 bytes) */

	bool VUnDIFFMemory(byte *dest, byte *diff, uint16 headerSize, uint16 copySize, uint16 bytesPerRow);
	void runLengthDecode(byte *dest, byte *source);
	void VRunLengthDecode(byte *dest, byte *source, uint16 bytesPerRow);
	bool readDiff(bool playonce);
	void diffNextFrame();
	void readSound(bool waitTillFinished, Common::File *file);
	void stopDiff();
	void stopDiffEnd();
	void stopSound();
	void playDiff();

};

} // End of namespace Lab

#endif /* LAB_DIFF_H */


