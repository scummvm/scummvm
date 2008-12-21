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

#ifndef GRAPHICS_DXA_PLAYER_H
#define GRAPHICS_DXA_PLAYER_H

#include "common/scummsys.h"
#include "common/stream.h"

namespace Graphics {

enum ScaleMode {
	S_NONE,
	S_INTERLACED,
	S_DOUBLE
};

class DXAPlayer {
protected:
	byte *_frameBuffer1;
	byte *_frameBuffer2;
	byte *_scaledBuffer;
	byte *_drawBuffer;
	byte *_inBuffer;
	uint32 _inBufferSize;
	byte *_decompBuffer;
	uint32 _decompBufferSize;
	uint16 _width;
	uint16 _height, _curHeight;
	uint16 _framesCount;
	uint32 _framesPerSec;
	uint16 _frameNum;
	uint32 _frameSize;
	uint32 _frameTicks;
	ScaleMode _scaleMode;

public:
	DXAPlayer();
	virtual ~DXAPlayer();

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	int getWidth();

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	int getHeight();

	/**
	 * Returns the current frame number of the video
	 * @return the current frame number of the video
	 */
	int getCurFrame();

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	int getFrameCount();

	/**
	 * Load a DXA encoded video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const char *fileName);

	/**
	 * Close a DXA encoded video file
	 */
	void closeFile();

protected:
	/**
	 * Set palette, based on current frame
	 * @param pal		the palette data
	 */
	virtual void setPalette(byte *pal) = 0;

	/**
	 * Copy current frame into the specified position of the destination
	 * buffer.
	 * @param dst		the buffer
	 * @param x		the x position of the buffer
	 * @param y		the y position of the buffer
	 * @param pitch		the pitch of buffer
	 */
	void copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch);

	/**
	 * Decode the next frame
	 */
	void decodeNextFrame();

	void decodeZlib(byte *data, int size, int totalSize);
	void decode12(int size);
	void decode13(int size);

	Common::SeekableReadStream *_fileStream;
};

} // End of namespace Graphics

#endif
