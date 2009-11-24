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

#ifndef GRAPHICS_VIDEO_DXA_PLAYER_H
#define GRAPHICS_VIDEO_DXA_PLAYER_H

#include "graphics/video/video_player.h"

namespace Graphics {

/**
 * Decoder for DXA videos.
 *
 * Video decoder used in engines:
 *  - agos
 *  - sword1
 *  - sword2
 */
class DXADecoder : public VideoDecoder {
public:
	DXADecoder();
	virtual ~DXADecoder();

	/**
	 * Load a DXA encoded video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const char *fileName);

	/**
	 * Close a DXA encoded video file
	 */
	void closeFile();

	bool decodeNextFrame();

	/**
	 * Get the sound chunk tag of the loaded DXA file
	 */
	uint32 getSoundTag() { return _soundTag; }

private:
	void decodeZlib(byte *data, int size, int totalSize);
	void decode12(int size);
	void decode13(int size);

	enum ScaleMode {
		S_NONE,
		S_INTERLACED,
		S_DOUBLE
	};

	byte *_frameBuffer1;
	byte *_frameBuffer2;
	byte *_scaledBuffer;
	byte *_inBuffer;
	uint32 _inBufferSize;
	byte *_decompBuffer;
	uint32 _decompBufferSize;
	uint16 _curHeight;
	uint32 _frameSize;
	ScaleMode _scaleMode;
	uint32 _soundTag;
};

} // End of namespace Graphics

#endif
