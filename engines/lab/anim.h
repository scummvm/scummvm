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

	uint32 _header;
	uint16 _curBit;
	uint16 _numChunks;
	uint32 _waitSec;
	uint32 _waitMicros;
	uint32 _delayMicros;
	bool _continuous;
	bool _isPlaying;
	bool _isAnim;
	bool _isPal;
	bool _donePal;
	uint16 _frameNum;
	bool _playOnce;
	byte *_buffer;
	byte *_diffFile;
	uint32 _size;
	bool _stopPlayingEnd;
	uint16 _sampleSpeed;
	uint32 _diffWidth;
	uint32 _diffHeight;
	BitMap *DrawBitMap;

	void readBlock(void *Buffer, uint32 Size, byte **File);

public:
	Anim(LabEngine *vm);

	DIFFHeader _headerdata;
	char _diffPalette[256 * 3];
	bool _waitForEffect; // Wait for each sound effect to finish before continuing.
	bool _doBlack;       // Black the screen before new picture
	bool _noPalChange;   // Don't change the palette.
	BitMap _rawDiffBM;

	void readDiff(byte *buffer, bool playOnce, bool onlyDiffData = false);
	void diffNextFrame(bool onlyDiffData = false);
	void stopDiff();
	void stopDiffEnd();
};

} // End of namespace Lab

#endif // LAB_DIFF_H
