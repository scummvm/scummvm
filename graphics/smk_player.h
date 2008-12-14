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

// Based on http://wiki.multimedia.cx/index.php?title=Smacker
// and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 15884
// http://svn.ffmpeg.org/ffmpeg/trunk/libavcodec/smacker.c?revision=15884&view=markups

#ifndef GRAPHICS_SMK_PLAYER_H
#define GRAPHICS_SMK_PLAYER_H

#include "common/scummsys.h"
#include "common/file.h"

namespace Common {
	class File;
}

class OSystem;

namespace Graphics {

class BigHuffmanTree;

/*
 * Implementation of a Smacker v2/v4 video decoder
 */

class SMKPlayer {
public:
	SMKPlayer();
	virtual ~SMKPlayer();

	Common::SeekableReadStream *_fileStream;

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	uint getWidth() { return _header.width; }

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	uint getHeight() { return (_header.flags ? 2 : 1) * _header.height; }

	/**
	 * Returns the current frame number of the video
	 * @return the current frame number of the video
	 */
	uint32 getCurFrame() { return _currentSMKFrame; }

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	uint32 getFrameCount() { return _framesCount; }

	/**
	 * Returns the frame rate of the video
	 * @return the frame rate of the video
	 */
	int32 getFrameRate() { return _header.frameRate; }

	/**
	 * Load an SMK encoded video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const char *filename);

	/**
	 * Close an SMK encoded video file
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
	bool decodeNextFrame();

	byte *getCurSMKImage() { return _image; }
	bool paletteDidChange() { return _paletteDidChange; }
	byte *palette() { return _palette; }

	uint16 _framesCount;

private:
	void unpackPalette();

	uint32 _currentSMKFrame;

	struct {
		uint32 signature;
		uint32 width;
		uint32 height;
		uint32 frames;
		int32 frameRate;
		uint32 flags;
		uint32 audioSize[7];
		uint32 treesSize;
		uint32 mMapSize;
		uint32 mClrSize;
		uint32 fullSize;
		uint32 typeSize;
		uint32 audioRate[7];
		uint32 dummy;
	} _header;

	uint32 *_frameSizes;
	uint32 *_frameTypes;

	BigHuffmanTree *_MMapTree;
	BigHuffmanTree *_MClrTree;
	BigHuffmanTree *_FullTree;
	BigHuffmanTree *_TypeTree;

	byte *_frameData;

	byte *_image;
	bool _paletteDidChange;
	byte *_palette;
	// Possible runs of blocks
	int getBlockRun(int index) { return (index <= 58) ? index + 1 : (2 ^ (59 - index)) * 128; }
};

} // End of namespace Graphics

#endif
