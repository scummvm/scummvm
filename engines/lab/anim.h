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

#ifndef LAB_ANIM_H
#define LAB_ANIM_H

namespace Lab {

class LabEngine;
#define CONTINUOUS      0xFFFF

struct DIFFHeader {
	uint16 _width;
	uint16 _height;
	char _fps;
	uint32 _flags;
};

class Anim {
private:
	LabEngine *_vm;

	uint32 _lastBlockHeader;
	uint16 _numChunks;
	uint32 _delayMicros;
	bool _continuous;
	bool _isPlaying;
	bool _isAnim;
	bool _isPal;
	bool _donePal;
	uint16 _frameNum;
	bool _playOnce;
	Common::File *_diffFile;
	uint32 _diffFileStart;
	uint32 _size;
	bool _stopPlayingEnd;
	uint16 _sampleSpeed;

	byte *_outputBuffer;
	DIFFHeader _headerdata;

public:
	Anim(LabEngine *vm);
	~Anim();

	char _diffPalette[256 * 3];
	bool _waitForEffect; // Wait for each sound effect to finish before continuing.
	bool _doBlack;       // Black the screen before new picture
	bool _noPalChange;   // Don't change the palette.
	byte *_scrollScreenBuffer;

	/**
	 * Reads in a DIFF file.
	 */
	void setOutputBuffer(byte *memoryBuffer); // nullptr for output to screen
	void readDiff(Common::File *diffFile, bool playOnce, bool onlyDiffData);
	void diffNextFrame(bool onlyDiffData = false);

	/**
	 * Stops an animation from running.
	 */
	void stopDiff();

	/**
	 * Stops an animation from running.
	 */
	void stopDiffEnd();

	uint16 getDIFFHeight();

	bool isPlaying() const { return _isPlaying;  }
};

} // End of namespace Lab

#endif // LAB_ANIM_H
