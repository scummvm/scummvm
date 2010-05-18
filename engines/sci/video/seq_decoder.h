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

#ifndef SEQ_DECODER_H
#define SEQ_DECODER_H

#include "graphics/video/video_decoder.h"

namespace Sci {

/**
 * Implementation of the Sierra SEQ decoder, used in KQ6 DOS floppy/CD and GK1 DOS
 */
class SeqDecoder : public Graphics::FixedRateVideoDecoder {
public:
	SeqDecoder();
	virtual ~SeqDecoder();

	bool load(Common::SeekableReadStream &stream);
	void close();

	void setFrameDelay(int frameDelay) { _frameDelay = frameDelay; }

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return SEQ_SCREEN_WIDTH; }
	uint16 getHeight() const { return SEQ_SCREEN_HEIGHT; }
	uint32 getFrameCount() const { return _frameCount; }
	Graphics::Surface *decodeNextFrame();
	Graphics::PixelFormat getPixelFormat() const { return Graphics::PixelFormat::createFormatCLUT8(); }
	byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }
  
protected:
	Common::Rational getFrameRate() const { assert(_frameDelay); return Common::Rational(60, _frameDelay); }

private:
	enum {
		SEQ_SCREEN_WIDTH = 320,
		SEQ_SCREEN_HEIGHT = 200
	};

	bool decodeFrame(byte *rleData, int rleSize, byte *litData, int litSize, byte *dest, int left, int width, int height, int colorKey);

	uint16 _width, _height;
	uint16 _frameDelay;
	Common::SeekableReadStream *_fileStream;
	byte _palette[256 * 3];
	bool _dirtyPalette;
	uint32 _frameCount;
	Graphics::Surface *_surface;
};

} // End of namespace Sci

#endif
