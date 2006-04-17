/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "common/file.h"

struct MoviePlayer {
	bool _playing;
	bool _leftButtonDown;
	bool _rightButtonDown;
	Common::File _fd;
	uint8 *_frameBuffer1;
	uint8 *_frameBuffer2;
	uint16 _width;
	uint16 _height;
	uint32 _frameSize;
	uint16 _framesCount;
	uint16 _currentFrame;
	uint32 _frameTicks;
	
	bool open(const char *filename);
	void close();
	void play();
	void delay(uint amount);

	void handleNextFrame();
	void decodeZlib(uint8 *data, int size, int totalSize);
	void decode0(uint8 *data, int size);
	void decode2(uint8 *data, int size, int totalSize);
};

#endif
