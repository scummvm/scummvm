/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#ifndef WSAMOVIES_H
#define WSAMOVIES_H

#include "kyra/resource.h"

namespace Kyra {
class KyraEngine;

class Movie {
public:
	Movie(KyraEngine *vm) : _x(-1), _y(-1), _drawPage(-1), _vm(vm), _opened(false) {}
	virtual ~Movie() {}

	virtual bool opened() { return _opened; }

	virtual void open(const char *filename, int offscreen, uint8 *palette) = 0;
	virtual void close() = 0;

	virtual int frames() = 0;

	virtual void displayFrame(int frameNum) = 0;

	int _x, _y;
	int _drawPage;
protected:
	KyraEngine *_vm;
	bool _opened;
};

class WSAMovieV1 : public Movie {
public:
	WSAMovieV1(KyraEngine *vm);
	virtual ~WSAMovieV1();

	virtual void open(const char *filename, int offscreen, uint8 *palette);
	virtual void close();

	virtual int frames() { return _opened ? _numFrames : -1; }

	virtual void displayFrame(int frameNum);
protected:
	virtual void processFrame(int frameNum, uint8 *dst);

	enum WSAFlags {
		WF_OFFSCREEN_DECODE = 0x10,
		WF_NO_FIRST_FRAME = 0x40,
		WF_HAS_PALETTE = 0x100
	};

	uint16 _currentFrame;
	uint16 _numFrames;
	uint16 _width;
	uint16 _height;
	uint16 _flags;
	uint8 *_deltaBuffer;
	uint32 _deltaBufferSize;
	uint8 *_offscreenBuffer;
	uint32 *_frameOffsTable;
	uint8 *_frameData;
};
} // end of namespace Kyra

#endif
